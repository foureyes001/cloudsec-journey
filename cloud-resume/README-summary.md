# Cloud Resume — Serverless Visitor Counter on AWS

A static résumé site hosted **privately** on Amazon S3, served globally over **HTTPS** via CloudFront, with a **serverless visitor counter** built on API Gateway → Lambda → DynamoDB. Built as part of the [Cloud Resume Challenge](https://cloudresumechallenge.dev/).

- **Live:** https://djt4zmhvif6vg.cloudfront.net
- **Region:** `us-east-1`

---

## Architecture

```
  Visitor's browser
        │
        │ 1) HTTPS GET — load the résumé page
        ▼
  CloudFront ── HTTPS · global edge cache · *.cloudfront.net certificate
        │
        │ OAC: only THIS distribution may read the bucket
        ▼
  Amazon S3 (PRIVATE) ── static site files; direct S3 access → 403 Forbidden

  ── once the page loads, its JavaScript runs the visitor counter ──

  Visitor's browser
        │
        │ 2) fetch() — HTTPS GET /
        ▼
  API Gateway (HTTP API) ── CORS (scoped to the CloudFront origin) · throttling
        │
        │ proxy integration · resource policy allows ONLY this API to invoke
        ▼
  AWS Lambda (Python) ── atomic +1
        │
        │ IAM: dynamodb:UpdateItem on ONE table only
        ▼
  DynamoDB ── cloud-resume-visitor-count
        │
        └─ returns {"count": N} ──► browser writes it into the page
```

## Tech stack

| Layer | Service / Tool |
|---|---|
| Static hosting (origin) | Amazon S3 (private) |
| CDN / HTTPS | Amazon CloudFront + Origin Access Control (OAC) |
| API | Amazon API Gateway (HTTP API, v2) |
| Compute | AWS Lambda (Python) |
| Database | Amazon DynamoDB (on-demand) |
| Access control | IAM (least-privilege inline policy) |
| Frontend | HTML / CSS / vanilla JavaScript (`fetch`) |

---

## How it was built — *what* and *why*, step by step

### 1. Private S3 bucket (the origin)
**What:** Created an S3 bucket to hold the static site files (`index.html`, etc.) with **Block Public Access enabled** (all four settings on).
**Why:** The bucket stores the website but must never be reachable directly from the internet. Keeping it private means the *only* path to the files is through CloudFront — a direct S3 URL returns **403 Forbidden**. This is the foundation of the "private origin, public CDN" pattern.

### 2. CloudFront distribution + Origin Access Control (OAC)
**What:** Put a CloudFront distribution in front of the bucket and attached **OAC**, which supplies an S3 bucket policy granting read access *only* to this specific distribution. The viewer protocol policy redirects **HTTP → HTTPS (301)**. HTTPS uses CloudFront's default `*.cloudfront.net` certificate — **no ACM certificate**, since the site uses the cloudfront.net domain rather than a custom one.
**Why:**
- **OAC** enforces that only CloudFront — not the public, not any other distribution — can read the private bucket. It's the mechanism that makes step 1's private bucket actually serveable.
- **CloudFront** adds HTTPS, a global edge cache (low latency for visitors anywhere), and a stable public URL while the origin stays locked down.

### 3. DynamoDB table (the counter store)
**What:** Created table `cloud-resume-visitor-count` with partition key **`id` (String)** and **on-demand** capacity. Seeded one item: `id = "visitors"` (String) and `count = 0` (Number) — two separate attributes.
**Why:**
- **On-demand** capacity = pay per request, nothing to provision or tune — ideal for low, unpredictable traffic.
- A single item keyed by `id = "visitors"` holds the running total; the table needs nothing more.

### 4. Lambda function (the increment logic)
**What:** Created `cloud-resume-visitor-counter` (a current Python runtime) that:
- calls `update_item` with `UpdateExpression="ADD #c :inc"`, where `:inc = 1`;
- uses an **expression-attribute-name alias `#c` → `count`**, because `count` is a DynamoDB **reserved word**;
- sets `ReturnValues="UPDATED_NEW"` to read back the new total;
- casts the returned value with **`int()`** (DynamoDB returns a `Decimal`, which JSON can't serialize);
- returns the **API Gateway proxy shape**: `statusCode`, `headers`, and `body` (where `body = json.dumps({"count": ...})`).

Tested in the console → `statusCode 200`, body `{"count": N}`, incrementing on each invocation.
**Why:**
- **`ADD`** is an **atomic** server-side increment — no read-then-write race if two requests arrive at once.
- The **alias** avoids the reserved-word error that a bare `count` would trigger.
- The **`int()` cast** + **`json.dumps`** make the response JSON-serializable.
- The **proxy response shape** is required by the proxy integration (step 6): API Gateway maps `statusCode` / `headers` / `body` directly onto the HTTP response.

### 5. Least-privilege IAM policy
**What:** On the Lambda's execution role, attached an inline policy (`least-privilege-policy`) allowing exactly one action — **`dynamodb:UpdateItem`** — scoped to the **exact table ARN** (`arn:aws:dynamodb:us-east-1:<ACCOUNT_ID>:table/cloud-resume-visitor-count`). **No wildcards** (`*` or `dynamodb:*`). The role also retains the AWS-managed `AWSLambdaBasicExecutionRole` for writing to CloudWatch Logs.
**Why:** Least privilege — if the function or its credentials were ever compromised, it can perform *only* one operation on *one* table and nothing else, minimizing blast radius. The basic-execution policy is the minimum needed to emit logs. This is the project's core security principle made concrete.

### 6. API Gateway (HTTP API) — the public front door
**What:** Created an **HTTP API (v2)** with a **Lambda proxy integration** (payload format **2.0**, the console default), a single route **`GET /`**, and the auto-deploying **`$default`** stage. Adding the integration auto-creates a resource policy on the Lambda permitting **only this API** to invoke it.
**Why:**
- **HTTP API** over REST API: simpler, cheaper, lower latency for a single-endpoint proxy, with built-in CORS and an auto-deploying stage.
- **Proxy integration** forwards the whole request to Lambda and returns the Lambda's response verbatim — no mapping templates (which is why the Lambda returns the proxy shape in step 4).
- **`GET /`**: the frontend calls the bare Invoke URL with nothing after the domain, so the route must be the root path `/` to match it.
- The scoped **invoke permission** means the Lambda can't be triggered by arbitrary callers — only this API.

### 7. CORS (configured on the API)
**What:** Enabled CORS on the HTTP API with **`Access-Control-Allow-Origin` = the CloudFront URL** (not `*`) and **`Access-Control-Allow-Methods` = `GET`**; all other CORS parameters left empty.
**Why:**
- Browsers enforce the **Same-Origin Policy**: JavaScript on the CloudFront origin can't read a response from the API's `execute-api` origin unless the API explicitly allows it. CORS is that allow-list.
- Scoping `Allow-Origin` to the **exact** CloudFront origin (instead of `*`) is least privilege at the browser boundary — only my own site's pages may read the counter.
- On HTTP APIs, **API-level CORS takes precedence and ignores any CORS headers the Lambda returns**, so this is the single source of truth (the Lambda's earlier `*` header is now redundant).
- Note: CORS protects the **user's browser**, not the API itself — non-browser clients (curl, scripts) ignore it. Actual request limiting comes from the next step.

### 8. Throttling (rate limiting — defense in depth)
**What:** On the `$default` stage's **default route throttling**, set a sustained **rate limit** and a **burst limit** (e.g. 10 req/s rate, 20 burst).
**Why:** The endpoint is public and CORS doesn't stop non-browser callers, so without a cap a bot could fire thousands of requests — each one a Lambda invocation **and** a DynamoDB write — running up cost and potentially swamping the service. Throttling uses a **token bucket** (**rate** = refill speed, **burst** = bucket size); excess requests get **429 Too Many Requests**. The caps sit well above real visitor traffic but low enough to make abuse cheap to absorb. *(Both limits must stay above 0 — setting either to 0 blocks all traffic.)*

### 9. Frontend wiring (brief)
**What:** `index.html` includes a small `<script>` (placed just before `</body>`) that `fetch()`es the API's public endpoint (its **Invoke URL**), parses `{"count": N}`, and writes `data.count` into a `<span id="visitor-count">`. A `try/catch` shows a fallback value and logs the error if the call fails.
**Why:** This is the only client-side logic — it connects the static page to the backend. The script sits at the end of `<body>` so the target element already exists when it runs, and the cross-origin `fetch` only succeeds from the live CloudFront origin (which the CORS rule in step 7 permits).

### 10. Deployment
**What:** Upload `index.html` to the S3 bucket, then create a **CloudFront invalidation** (`/index.html` or `/*`) and wait for it to complete. Verify on the **live HTTPS CloudFront URL** with the browser's DevTools open (Console + Network).
**Why:** CloudFront caches files at edge locations, so a fresh S3 upload won't appear until the cached copy is invalidated — the invalidation forces edges to re-fetch from S3. Testing must happen on the deployed `https://` site (not a local `file://` open), because the CORS rule only permits the CloudFront origin.

---

## Security highlights (why this is more than a static page)

- **Private origin, public CDN** — S3 is fully private (Block Public Access); only CloudFront can read it, via OAC.
- **HTTPS everywhere** — HTTP redirects to HTTPS at CloudFront.
- **Least-privilege compute** — the Lambda's IAM policy permits exactly one DynamoDB action on exactly one table; no wildcards.
- **Scoped invocation** — only the one HTTP API may invoke the Lambda.
- **Scoped CORS** — only the site's own origin may read the API from a browser.
- **Rate limiting** — throttling caps abuse of the public endpoint to bound cost and protect availability.

---

## Notes / gotchas worth remembering

- DynamoDB **`count` is a reserved word** — it must be aliased (`#c`) in the update expression.
- DynamoDB numbers come back as **`Decimal`** — cast to `int` before `json.dumps`.
- On HTTP APIs, **configured CORS overrides the integration's CORS headers** — set it in one place, not both.
- A cross-origin `fetch` from a local **`file://`** open will always fail CORS — test on the live CloudFront URL.
- After re-uploading to S3, you **must invalidate CloudFront** or you'll keep seeing the cached version.

---

## Repository

Part of [`cloudsec-journey`](https://github.com/foureyes001/cloudsec-journey) → `cloud-resume/`.

> The AWS account ID is intentionally redacted as `<ACCOUNT_ID>`. Avoid publishing real account IDs in a public repository.
