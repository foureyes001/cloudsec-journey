# S20 — Serverless Solution Architecture Discussions

**`[A - ENTIRE FILE]`**
**Lectures 227–230 · 17 min**
⚠️ Section number assumed 20 — screenshot shows lectures only. Confirm from the sidebar header.

> **★ How this section is tested.** No new services. Every lecture is a **requirement → service mapping** drill, and SAA-C03 is scenario-based, so this is closer to exam practice than to content. **Learn the mappings, not the diagrams.**

---

## 227 — Mobile Application: MyTodoList (5 min)

### Stated requirements → service

| Requirement | Answer | Why |
|---|---|---|
| REST API over HTTPS | **API Gateway** | HTTPS by default, no cert management |
| Serverless compute | **Lambda** | |
| Database that scales, serverless | **DynamoDB** | |
| Authenticate via a **managed serverless** service | **Amazon Cognito** | Not IAM users — Cognito is the answer whenever the users are *app users*, not AWS users |
| Users interact **directly with their own folder** in S3 | **Cognito Identity Pool → STS temporary credentials** + IAM policy scoped to the user's prefix | ★ The whole point: the app never proxies the upload |
| Mostly reads, **high read throughput** | **DynamoDB DAX** | Microsecond read caching, in front of DynamoDB |
| Cache the API responses too | **API Gateway caching** | Different layer from DAX — both can be used |

### ★ Two caching layers, and they are not interchangeable

| Layer | Caches | Use when |
|---|---|---|
| **DAX** | DynamoDB reads | Repeated reads of the same items |
| **API Gateway cache** | Whole API responses | Repeated identical API calls |

### ★ Cognito, the distinction the exam tests

| | Purpose |
|---|---|
| **User Pool** | **Authentication** — who are you. Sign-up/sign-in, integrates with API Gateway authorizer |
| **Identity Pool** (Federated Identities) | **Authorisation to AWS** — temporary AWS credentials via STS, for direct access to S3/DynamoDB |

**Scoping to a user's own folder** is done with an IAM policy using the Cognito identity variable in the resource path — that pattern is the answer to "each user must access only their own prefix."

---

## 228 — Serverless Website: MyBlog.com (6 min)

### Stated requirements → service

| Requirement | Answer |
|---|---|
| Entirely serverless, **scales globally** | S3 + CloudFront + API Gateway + Lambda + DynamoDB |
| Static files, served globally | **S3 origin behind CloudFront**, restricted with **OAC** |
| DNS | **Route 53** |
| Dynamic REST API | **API Gateway → Lambda → DynamoDB** |
| Blogs **rarely written, often read**, globally | ★ **DynamoDB Global Tables** — read replicas in multiple regions |
| Route users to the nearest region | **Route 53 latency-based routing** |
| Welcome email when a user subscribes | ★ **DynamoDB Streams → Lambda → SES** |
| Thumbnail generated for every uploaded photo | ★ **S3 Event Notification → Lambda → write thumbnail back to S3** |

### ★ The two event-driven patterns to memorise

| Trigger | Chain | Scenario wording |
|---|---|---|
| **DynamoDB Streams** | Streams → Lambda → SES / anything | "when a record is inserted/updated, do X" |
| **S3 Event Notification** | S3 → Lambda | "when a file is uploaded, process it" |

**DynamoDB Global Tables:** multi-region, **active-active** (read *and* write in every region), last-writer-wins conflict resolution. Requires **DynamoDB Streams enabled**. This is the answer to "global low-latency reads on DynamoDB."

---

## 229 — MicroServices Architecture (4 min)

### Communication patterns

| Type | Services |
|---|---|
| **Synchronous** | API Gateway · Application Load Balancer |
| **Asynchronous** | SQS · SNS · Kinesis · Lambda triggers · S3 events |

### ★ The four stated challenges of microservices, and why serverless fixes them

| Challenge | Serverless answer |
|---|---|
| Repeated overhead creating each new microservice | API Gateway + Lambda — nothing to provision |
| Optimising server density / utilisation | No servers; pay per invocation |
| Running multiple versions of multiple services | **Lambda versions and aliases + API Gateway stages** |
| Client-side code to integrate with many services | API Gateway as the single front door |

**Exam key:** when a question lists microservice pain points and asks for the lowest-operational-overhead fix → **API Gateway + Lambda**. When it says "containers" and "microservices" instead → **ECS/EKS on Fargate**.

---

## 230 — Software updates distribution (2 min)

**Scenario:** an application distributing a file (software update) to many users worldwide, served from EC2/ALB or S3.

**Problems:** high data-transfer-out cost · high latency for distant users · load on the origin.

### ★ Answer: put CloudFront in front. That is the whole lecture.

| Benefit | Detail |
|---|---|
| **No architecture change** | CloudFront sits in front of the existing origin — nothing else is rewritten |
| **Cost** | Data transfer from an AWS origin (S3, EC2, ALB) into CloudFront is **free**. Only CloudFront's own egress is billed, and at a lower rate |
| **Latency** | Cached at edge locations worldwide |
| **Origin load** | Cache hits never reach the origin |

**Works with any origin:** S3 · EC2 · ALB · custom HTTP origin, **including on-premises**.

> **★ Trigger phrase.** "Distribute static content / software updates globally, reduce cost and latency, **without changing the architecture**" → **CloudFront**. If the question instead stresses *many small dynamic requests* or *TCP/UDP at layer 4* → **Global Accelerator**, not CloudFront.

---

## ★ Quick-fire: scenario → answer

| Scenario | Answer |
|---|---|
| App users must sign in, not AWS users | Cognito User Pool |
| App users need direct AWS resource access | Cognito Identity Pool → STS |
| Each user accesses only their own S3 prefix | IAM policy with the Cognito identity variable |
| DynamoDB reads are hot and repeated | DAX |
| Same API response requested repeatedly | API Gateway caching |
| DynamoDB reads needed globally, low latency | Global Tables |
| Act on a DynamoDB write | DynamoDB Streams → Lambda |
| Act on an S3 upload | S3 Event Notification → Lambda |
| Send transactional email from Lambda | SES |
| Static site, global, serverless | S3 + CloudFront + Route 53 |
| Cut egress cost on large file distribution | CloudFront |
| Lowest-ops microservices | API Gateway + Lambda |

---

## ⚠️ Stale-content flags

| Course says | Current reality | How to answer the exam |
|---|---|---|
| **OAI** (Origin Access Identity) to restrict S3 to CloudFront | **OAC** (Origin Access Control) superseded it in Aug 2022; AWS recommends OAC for all new distributions. OAI still works for existing ones | ★ **Both may appear as keys.** If OAC is an option, pick it; if only OAI is offered, it is still correct on SAA-C03 banks |
| Cognito overview | Cognito introduced **Lite / Essentials / Plus** pricing tiers (Nov 2024). Does not change the architecture answers | Ignore for the exam; know it for interviews |

*Per the standing ruling: answer exam questions with course content, not current reality. The flags exist so dead facts don't get repeated in an interview.*

## Gaps to fill externally

- **CloudFront vs Global Accelerator** — the distinction is tested and these lectures only imply it.
- **Lambda versions vs aliases** — named in 229 as the versioning answer but not explained.
- **API Gateway caching specifics** — TTL, per-stage vs per-method, cache invalidation. Not covered.

---

> **★ Retention note.** This file is assistant-written, so reading it back will not build recall. **Close it and re-write the ★ blocks from memory — the two caching layers, the two event-driven chains, the four microservice challenges, and the CloudFront trigger phrase — before committing.**
