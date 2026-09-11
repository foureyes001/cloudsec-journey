# S19 — Serverless: Lambda, DynamoDB, API Gateway, Step Functions, Cognito

Lectures 209–226. Section number inferred as 19 (S18 ended at 208 + Quiz 15).
⚠ Screenshots stop at 226 — confirm whether the section continues past it.
Lab policy: **WATCH ONLY.** No lab owed.

---

## Serverless — what it means (209, 210)

- **You don't manage servers. Servers still exist — you just don't provision, patch or see them.**
- Started as Functions-as-a-Service, now covers anything fully managed.
- **AWS serverless list:** Lambda · DynamoDB · Cognito · API Gateway · S3 · SNS · SQS · Amazon Data Firehose · Aurora Serverless · Step Functions · Fargate.

---

## Lambda overview (211, 212)

| | **EC2** | **Lambda** |
|---|---|---|
| Unit | Virtual servers | Virtual **functions** |
| Limited by | RAM and CPU | **Time** — short executions |
| Running | Continuously | **On-demand** |
| Scaling | Add/remove servers | **Automatic** |

- **Pricing: pay per request + per duration.** Free tier: **1,000,000 requests/month** and **400,000 GB-seconds/month**.
- **★ Increasing RAM also increases CPU and network.** There is no separate CPU dial — this is a common question.
- **Runtimes:** Node.js, Python, Java, C#/.NET, Go, Ruby, **Custom Runtime API**, and **container images** (image must implement the Lambda Runtime API).
- **★ For an arbitrary Docker image, the answer is ECS/Fargate, not Lambda.**
- **Common integrations:** API Gateway · S3 · DynamoDB · Kinesis · SNS · SQS · CloudFront · EventBridge · CloudWatch Logs · Cognito.
- **Two canonical patterns:** S3 upload → Lambda → thumbnail to S3 + metadata to DynamoDB · **EventBridge schedule → Lambda = serverless CRON.**

---

## ★★ Lambda limits (213) — memorise these

| Limit | Value |
|---|---|
| Memory | **128 MB – 10 GB** (1 MB increments) |
| **Max execution time** | **900 seconds = 15 minutes** |
| `/tmp` ephemeral storage | **512 MB – 10 GB** |
| Environment variables | **4 KB** |
| Default concurrency | **1,000** (soft, raise via support ticket) |
| Deployment .zip (compressed) | **50 MB** |
| Deployment uncompressed | **250 MB** |

- **★ The 15-minute ceiling is the single most tested Lambda fact.** Job longer than 15 min → **Fargate, ECS, AWS Batch, or Step Functions**, never Lambda.
- **★ `/tmp` is ephemeral and per-execution-environment.** Persistent or shared storage across invocations → **EFS**.

---

## ★★ Lambda concurrency and cold starts (214, 215, 216)

- **Reserved concurrency** = a cap set at the function level.
- **★ Throttle behaviour splits by invocation type:** synchronous → **429 ThrottleError** returned · asynchronous → **automatic retry, then DLQ**.
- **★ The starvation problem:** concurrency is an account-wide pool. One busy function can throttle every other function unless reserved concurrency is set. **Trigger phrase: "one function is starving the others."**
- **Cold start** = new execution environment; code and init logic outside the handler run first, so the first request has higher latency.
- **★ Provisioned Concurrency** = environments allocated **before** invocation → no cold start, consistently low latency. Managed by **Application Auto Scaling** (schedule or target utilisation). **Trigger phrase: "predictable low latency" / "eliminate cold starts."**

### Lambda SnapStart
- Function is invoked from a **pre-initialised snapshot** taken when you publish a version. Up to **10x faster startup, no extra charge** (Java).
- ⚠ **Course is almost certainly out of date here.** Verified current: SnapStart supports **Java 11+, Python 3.12+, .NET 8+**, and since mid-2026 **container images** too. If the lecture says Java-only, that was true in 2022.
- **★ SnapStart does NOT work with Provisioned Concurrency, EFS, or `/tmp` over 512 MB.** If a question offers both SnapStart and Provisioned Concurrency together, that combination is invalid.
- Only usable on **published versions and aliases**, never `$LATEST`.

---

## ★★ Lambda@Edge vs CloudFront Functions (217)

| | **CloudFront Functions** | **Lambda@Edge** |
|---|---|---|
| Language | **JavaScript only** | Node.js or Python |
| Triggers | **Viewer request / viewer response only** | **All four** — viewer request, origin request, origin response, viewer response |
| Max execution | **< 1 ms** | **5–10 seconds** |
| Memory | 2 MB | up to 10 GB |
| Package size | 10 KB | 1–50 MB |
| Network access | **No** | Yes |
| File system access | **No** | Yes |
| Access to request body | **No** | Yes |
| Cost | ~1/6 of Lambda@Edge | — |
| Where it lives | **Native to CloudFront** | Authored in **us-east-1**, replicated to edge locations |

- **★ CloudFront Functions use cases:** cache key normalisation · header manipulation · URL rewrites and redirects · **JWT create/validate** (request auth).
- **★ Lambda@Edge use cases:** longer execution · adjustable CPU/memory · third-party libraries (e.g. AWS SDK) · network calls to external services · **needs the request body**.
- **The decision rule:** origin-side trigger, network access, or request body → **Lambda@Edge**. Everything else, sub-millisecond and cheap → **CloudFront Functions**.

---

## Lambda in a VPC (218)

- **★ By default Lambda runs OUTSIDE your VPC, in an AWS-owned VPC.** It therefore **cannot reach RDS, ElastiCache, or internal ELBs.**
- To fix: specify **VPC ID, subnets, and security groups.** Lambda creates an **ENI** in your subnets.
- **Trigger phrase: "Lambda cannot connect to the database" → it isn't in the VPC.**

### RDS Proxy with Lambda
- **The problem:** many concurrent Lambdas each open their own DB connection and exhaust the database.
- **RDS Proxy** pools and shares connections · reduces failover time by **up to 66%** while preserving connections · enforces **IAM authentication** and stores credentials in **Secrets Manager**.
- **★★ RDS Proxy is NEVER publicly accessible, so the Lambda function MUST be deployed in the VPC.** Frequently tested together.

---

## RDS → Lambda and RDS Event Notifications (219)

- **Invoking Lambda from within a DB instance:** supported on **RDS for PostgreSQL** and **Aurora MySQL**. Requires outbound path (public, NAT Gateway, or VPC endpoint) plus a Lambda resource-based policy and an IAM policy.
- **★ RDS Event Notifications tell you about the DB instance, NOT the data** — created, stopped, started, failover. **No data-change events.** Near real-time, **up to 5 minutes delay**. Delivered to **SNS** or **EventBridge**.
- **Discriminator:** "notify when rows change" → not RDS Event Notifications. "Notify when the instance fails over" → yes.

---

## Amazon DynamoDB (220, 221)

- Fully managed **NoSQL**, replicated across **multiple AZs**, **single-digit millisecond** latency, supports **transactions**.
- **★ Primary key must be decided at table creation time.** Items = rows; attributes can be added over time and can be null.
- **★ Maximum item size: 400 KB.** Tested.
- **Data types:** Scalar (String, Number, Binary, Boolean, Null) · Document (List, Map) · Set (String Set, Number Set, Binary Set).
- Table classes: **Standard** and **Standard-Infrequent Access**.

### ★★ Capacity modes

| | **Provisioned** (default) | **On-Demand** |
|---|---|---|
| You specify | **RCU / WCU** | Nothing |
| Planning | Required in advance | None |
| Scaling | Optional auto-scaling on RCU/WCU | Automatic |
| Cost | Cheaper for steady load | **More expensive** |
| Use when | **Predictable** traffic | **Unpredictable traffic, steep sudden spikes** |

---

## ★★ DynamoDB advanced features (222)

**DAX (DynamoDB Accelerator)**
- Fully managed, highly available **in-memory cache**, **microsecond** reads.
- **★ No application code changes** — API-compatible with DynamoDB.
- **Default TTL 5 minutes.**
- **★ DAX vs ElastiCache:** DAX caches **individual objects, queries and scans**; ElastiCache stores **aggregation results**.

**DynamoDB Streams**
- Ordered stream of **item-level changes** (create/update/delete).
- Consumers: **Lambda**, Kinesis Data Streams, KCL applications.
- **★ Retention: 24 hours.**
- Uses: real-time reactions (welcome email), analytics, derivative tables, cross-region replication.

**Global Tables**
- Multi-region, **active-active** — read AND write in any region.
- **★★ DynamoDB Streams MUST be enabled as a prerequisite.** Near-guaranteed exam detail.

**TTL**
- Automatically deletes items after an expiry timestamp. Uses: trim stored data, regulatory retention limits, **web session handling**.

**Backups**
- **Point-in-time recovery (PITR):** continuous, **last 35 days**, restore to any second in the window.
- **On-demand backups:** full, retained until explicitly deleted, no performance impact, manageable via **AWS Backup** (which enables **cross-region copy**).
- **★ Both recovery paths create a NEW table.** They never restore in place.

**S3 integration**
- **Export to S3 requires PITR enabled.** Any point in the last 35 days, **does not consume read capacity**, formats DynamoDB JSON or ION.
- **Import from S3: does not consume write capacity, creates a new table.** CSV, DynamoDB JSON, ION. Errors go to CloudWatch Logs.

---

## API Gateway (223, 224)

- Fronts Lambda with no infrastructure. Supports **REST and WebSocket**.
- Features: **versioning · stages (dev/test/prod) · authentication and authorization · API keys and throttling · Swagger/OpenAPI import · request and response transformation and validation · SDK generation · response caching**.

**Three integration types**
- **Lambda** — expose a REST API backed by a function.
- **HTTP** — front an existing backend (on-prem HTTP API, ALB) to add rate limiting, caching, auth, API keys.
- **AWS Service** — expose any AWS API directly (start a Step Functions workflow, post to SQS) to add auth and rate control.

**★★ Endpoint types**

| Type | Detail |
|---|---|
| **Edge-Optimized** (default) | **Global clients.** Requests routed via **CloudFront edge locations**. ⚠ The API Gateway itself still lives in **one region only** — the edge network is just the path in. |
| **Regional** | Clients in the same region. Can be combined manually with CloudFront for control over caching. |
| **Private** | Accessible **only from your VPC via an interface VPC endpoint (ENI)**. Access controlled by a **resource policy**. |

**Security**
- Authentication: **IAM roles** (internal apps) · **Cognito** (external/mobile users) · **custom authorizer** (own logic).
- **★★ Custom domain HTTPS via ACM — the certificate region is tested:** Edge-Optimized → certificate must be in **us-east-1**. Regional → certificate in the **API Gateway's own region**. Then a **CNAME or Route 53 A-alias** record.

---

## Step Functions (225)

- Workflows modelled as **state machines**, one per workflow, defined in **JSON**.
- Gives **visual workflow, execution history, and error handling/retry** across steps.
- Started by: **SDK call, API Gateway, or EventBridge**.
- **★ Trigger phrase: "orchestrate multiple Lambda functions" or "long-running multi-step workflow"** → Step Functions, not one big Lambda (which would hit the 15-minute wall).

---

## ★★ Amazon Cognito (226)

**The split that gets tested, in one line each:**

| | **User Pools (CUP)** | **Identity Pools (Federated Identities)** |
|---|---|---|
| Purpose | **Authentication** — sign-in for your app users | **Authorization** — hand out **temporary AWS credentials** |
| Result of login | **A JWT** | **AWS credentials via STS** |
| Integrates with | **API Gateway** and **Application Load Balancer** | AWS services directly, or via API Gateway |

**User Pools**
- Serverless user directory. Username/email + password, **password reset, email and phone verification, MFA**.
- **Federated identities** in: Facebook, Google, SAML.
- **★ Can block users whose credentials were compromised elsewhere.**

**Identity Pools**
- Identity sources: public providers (Amazon, Facebook, Google, Apple) · **a Cognito User Pool** · OIDC and SAML providers · developer-authenticated identities.
- **★ Supports unauthenticated (guest) access.**
- **IAM policies applied to the credentials are defined in Cognito**, and can be customised per `user_id` for fine-grained control — this is how **row-level security in DynamoDB** is done.

**★ Cognito vs IAM — the decision rule:** IAM is for your AWS account's own principals. Cognito is the answer when the question says **"hundreds/millions of users," "mobile app users," or "authenticate with SAML/social login."**

---

## ★★ Exam trap list

1. **Job over 15 minutes** → Fargate / ECS / Batch / Step Functions. Never Lambda.
2. **Lambda can't reach RDS or ElastiCache** → it's not in the VPC.
3. **RDS Proxy + Lambda** → Lambda must be in the VPC; the proxy is never public.
4. **Eliminate cold starts** → Provisioned Concurrency (or SnapStart on Java/Python/.NET — but not both together).
5. **One function throttling the rest** → reserved concurrency.
6. **Global Tables** → DynamoDB Streams must be enabled first.
7. **Restore a DynamoDB table** → always creates a **new** table; PITR covers **35 days**.
8. **Cache DynamoDB reads with no code change** → DAX. Aggregation results → ElastiCache.
9. **Edge-Optimized API custom domain** → ACM certificate in **us-east-1**.
10. **Origin-side CloudFront logic or request body access** → Lambda@Edge, not CloudFront Functions.
11. **Persistent/shared file storage for Lambda** → EFS. `/tmp` is ephemeral.
12. **Sign users in** → User Pools. **Let them call AWS directly** → Identity Pools.

---

## ⚠ Likely gaps — testable, probably not in these lectures

- **★ Step Functions Standard vs Express workflows.** Lecture 225 is 2 minutes, too short to cover it. **Standard:** up to 1 year, exactly-once, full execution history, priced per state transition. **Express:** up to 5 minutes, at-least-once, high volume, priced per execution and duration. Tested.
- **★ DynamoDB GSI vs LSI.** LSI must be created **at table creation** and shares the partition key; GSI can be added **any time** with a different partition key. Comes up on SAA.
- **Lambda Function URLs** — a direct HTTPS endpoint without API Gateway.
- **API Gateway usage plans + API keys** — the mechanism behind per-customer throttling and quotas.

---

⚠ **Verified this session:** SnapStart runtime support and its incompatibility with Provisioned Concurrency / EFS / large `/tmp`. Everything else is from my own knowledge. The Lambda limit table and the CloudFront-Functions-vs-Lambda@Edge figures are the most worth one confirmation pass.
