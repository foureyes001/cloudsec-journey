# S17 — Decoupling Applications: SQS, SNS, Kinesis, Amazon MQ

`[A - ENTIRE FILE]` — assistant-written scaffolding. Close this file and re-write the ★ blocks from memory before committing.

Covers lectures **183–198**. Section number inferred as 17 (S16 ended at 182).
⚠ **This is one of the heaviest-weighted sections on SAA-C03.** Decoupling appears across all four exam domains, not just "Design Resilient Architectures."

---

## 1. Why decouple (lecture 183)

- **Synchronous** = app calls app directly. Breaks under sudden traffic spikes — the downstream service gets crushed.
- **Asynchronous / event-based** = a middle layer absorbs the spike.
- **The three patterns, and this framing is the whole section:**
  - **SQS = queue model.** One message, one consumer, pulled.
  - **SNS = pub/sub model.** One message, many subscribers, pushed.
  - **Kinesis = real-time streaming model.** Ordered, replayable, big data.
- **★ The point:** the decoupling layer **scales independently** of your application.

---

## 2. Amazon SQS — Standard Queues (184, 185)

### Core attributes — memorise the numbers
| Attribute | Value |
|---|---|
| Throughput | **Unlimited** |
| Messages in queue | **Unlimited** |
| Default retention | **4 days** |
| Retention range | **60 seconds → 14 days** |
| Latency | **< 10 ms** on publish and receive |
| **Max message size** | **256 KB** |
| Messages per poll | **up to 10** |
| In-flight message quota | **120,000** (standard) |

- **★ Delivery guarantee: AT-LEAST-ONCE.** Duplicates are possible. **Consumers must be idempotent.**
- **★ Ordering: BEST-EFFORT only.** Out-of-order messages are possible on standard queues.

### Producing and consuming
- **Produce:** `SendMessage` via SDK. Message persists in SQS **until the consumer deletes it** — SQS does not delete on read.
- **Consume:** poll → receive up to 10 → process → **`DeleteMessage`**. Forgetting the delete is the classic bug: the message comes back.
- Multiple consumers poll in parallel; scale consumers **horizontally**.

### ★★ SQS + Auto Scaling Group (lecture 189) — near-certain exam question
- **CloudWatch metric: `ApproximateNumberOfMessagesVisible`** (queue length).
- CloudWatch **Alarm** on that metric → triggers ASG **scale-out**.
- **The pattern:** front-end tier writes to SQS → back-end ASG scales on queue depth. Decouples the two tiers so a front-end spike never overwhelms the back end.
- **Trigger phrase:** "requests are lost during traffic spikes" → put SQS between the tiers.

### Security
- **Encryption:** in-flight via **HTTPS API** · at-rest via **KMS** · client-side.
- **Access control:** **IAM policies** control access to the SQS **API**.
- **★ SQS Access Policies** (like S3 bucket policies) — the answer for **cross-account access**, and for **letting another AWS service (SNS, S3) write into the queue.** If a fan-out or S3-event question fails, this policy is usually the missing piece.

---

## 3. ★★ Message Visibility Timeout (186) — high-frequency question

- After a consumer polls a message, it becomes **invisible to other consumers** for the visibility timeout.
- **Default: 30 seconds.**
- If the consumer does not delete the message within that window, the message becomes **visible again and is processed twice.**
- **`ChangeMessageVisibility` API** — a consumer that needs more time extends its own timeout.

**★ The trade-off, which is what gets tested:**

| Timeout setting | Consequence |
|---|---|
| **Too high** (hours) | Consumer crashes → long delay before reprocessing |
| **Too low** (seconds) | Slow processing → **duplicate processing** |

- **Question shape:** "messages are being processed more than once" → visibility timeout is shorter than processing time. Fix: raise it, or call `ChangeMessageVisibility`.

---

## 4. Long Polling (187)

- Consumer **waits** for messages to arrive rather than returning empty.
- **★ Wait time: 1 to 20 seconds. 20 is preferred.**
- **Benefits: fewer API calls, lower cost, reduced latency.**
- **Long polling is always preferable to short polling.** If an answer option says "enable short polling to reduce cost", it's wrong.
- Enabled at the **queue level**, or per-request via **`WaitTimeSeconds`**.

---

## 5. ★★ FIFO Queues (188)

- **Ordering guaranteed. Exactly-once send** (duplicates removed).
- **★ Queue name MUST end in `.fifo`.** Easy free mark.

### Throughput — the numbers the course teaches
- **300 msg/s without batching · 3,000 msg/s with batching** (batch = 10 messages per call).
- ⚠ **Verified current reality (Sept 2026):** those defaults still hold, but **high throughput mode for FIFO now supports up to 70,000 messages/second without batching**, and more with batching. If the course calls 3,000 the ceiling, that is out of date — though the exam likely still uses the old figure.
- In-flight quota for FIFO is **20,000**, not 120,000.

### Deduplication
- **De-dup interval: 5 minutes.**
- Two methods: **content-based** (SHA-256 hash of the message body) or an explicit **`MessageDeduplicationId`**.

### ★ Message grouping — `MessageGroupID`
- **Same `MessageGroupID`** → messages go to the **same consumer** and are strictly ordered within that group.
- **Different `MessageGroupID`s** → groups are processed independently, so you can have **multiple consumers in parallel** while keeping order *within* each group.
- **Trigger phrase:** "order must be preserved per customer/account/device, but we need parallelism" → FIFO with `MessageGroupID` set to that entity.

---

## 6. Amazon SNS (190, 192)

- **Pub/Sub.** Producer publishes to **one topic**; every subscriber receives **every message** (unless filtered).
- **Limits: 12,500,000 subscriptions per topic · 100,000 topics.**
- **★ Subscriber types:** SQS · Lambda · **Amazon Data Firehose** · HTTP/HTTPS endpoints · Email · SMS · mobile push notifications.
- **★ Data is NOT persisted.** If delivery fails and there's no retry left, the message is gone. This is the single biggest difference from SQS and the reason fan-out exists.
- **AWS services that publish directly to SNS:** CloudWatch Alarms, ASG notifications, S3 bucket events, CloudFormation state changes, AWS Budgets, Lambda, DynamoDB.
- **Two publish modes:** Topic Publish (SDK — create topic, create subscriptions, publish) · Direct Publish (mobile SDK — create platform application, create platform endpoint, publish to endpoint).
- **Security: identical model to SQS** — HTTPS in flight, KMS at rest, IAM policies, and **SNS Access Policies** for cross-account and for letting S3 write to the topic.

---

## 7. ★★★ SNS + SQS Fan-Out (191) — the single most tested pattern in this section

- **Publish once to SNS → every subscribed SQS queue gets a copy.**
- **Why it beats SNS alone:** SQS adds **data persistence, delayed processing, and retries.** No data loss.
- **You can add new SQS subscribers later** without touching the producer.
- ⚠ **The queue's SQS Access Policy must allow SNS to write to it.** This is the step people miss, and it's a favourite distractor.
- **Cross-region delivery works** — SNS can deliver to SQS queues in other regions.

**★ Application 1 — S3 events to multiple destinations.**
- **S3 limitation: for the same combination of event type + prefix, you can only have ONE S3 event rule.**
- **Fan-out is the workaround:** S3 → SNS topic → multiple SQS queues / Lambdas.
- **This exact scenario appears regularly. Learn the limitation, not just the fix.**

**★ Application 2 — SNS → Amazon Data Firehose → S3** (or any Firehose destination).

**SNS FIFO topics**
- Same features as SQS FIFO: ordering by **message group ID**, deduplication.
- **★ Subscribers must be SQS FIFO queues only.**
- Same throughput limitations as SQS FIFO.

**Message filtering**
- A **JSON filter policy** on a subscription controls which messages that subscriber receives.
- **★ No filter policy = the subscription receives every message.**
- Trigger phrase: "route orders by status to different queues without extra code" → SNS message filtering, not Lambda.

---

## 8. Amazon Kinesis Data Streams (193, 194)

- **Real-time streaming.** Collect, process, analyse streaming data.
- **★ Retention: 1 day default, up to 365 days.**
- **★ REPLAY capability** — you can reprocess data. This is the defining difference from SQS and SNS.
- **★ Data is IMMUTABLE** — once inserted, it cannot be deleted. It only expires.
- **★ Same partition key → same shard → ordering guaranteed at the shard level.**
- **Producers:** SDK · Kinesis Producer Library (KPL) · Kinesis Agent.
- **Consumers:** custom (KCL, SDK) · managed (Lambda, Amazon Data Firehose, Managed Service for Apache Flink).

### ★★ Capacity modes — memorise the per-shard numbers

| | Provisioned | On-demand |
|---|---|---|
| Shards | You choose and scale **manually** | Managed automatically |
| Throughput in | **1 MB/s or 1,000 records/s per shard** | Default **4 MB/s or 4,000 records/s** |
| Throughput out | **2 MB/s per shard** | Scales to observed peak of last **30 days** |
| Billing | **Per shard per hour** | Per stream per hour + data in/out per GB |
| Use when | Throughput is predictable | Unpredictable or unknown |

- **Security:** IAM for access · HTTPS in flight · KMS at rest · client-side encryption · **VPC endpoints available** · CloudTrail for API monitoring.

---

## 9. Amazon Data Firehose (195, 196)

- **Fully managed, serverless, automatic scaling, no administration.**
- **★ NEAR real-time, not real-time.** It is **buffer-based**: minimum **60 seconds** latency, or flushes when it accumulates a minimum amount of data.
- **★★ NO storage. NO replay.** Data passes through and lands at the destination. This is the cleanest discriminator against Kinesis Data Streams.
- **Destinations:**
  - **AWS:** S3 · **Amazon Redshift** (loaded *via* S3, using COPY) · **Amazon OpenSearch**
  - **Third party:** Datadog, Splunk, New Relic, MongoDB
  - **Custom:** any HTTP endpoint
- **Can transform records in flight with Lambda.**
- **Failed or all records can be backed up to an S3 bucket.**
- **Pay for data going through Firehose** — no provisioning.
- ⚠ **Naming:** renamed from **Kinesis Data Firehose** to **Amazon Data Firehose** in 2024. The exam may still use the old name. Same service.

---

## 10. ★★★ SQS vs SNS vs Kinesis (197) — write this table from memory before the exam

| | **SQS** | **SNS** | **Kinesis** |
|---|---|---|---|
| Model | Queue | Pub/Sub | Streaming |
| Delivery | Consumer **pulls** | **Pushed** to subscribers | Pull (standard) or push (enhanced fan-out) |
| After consumption | **Message deleted** | Not persisted | **Retained 1–365 days** |
| **Replay** | **No** | **No** | **YES** |
| Consumers | As many workers as you want | Up to 12.5M subscriptions | Per-shard |
| Ordering | **FIFO queues only** | **FIFO topics only** | **At shard level** |
| Provisioning | None needed | None needed | **Provisioned or on-demand** |
| Signature use | Decouple tiers, buffer work | Fan-out, notifications | Real-time big data, analytics, ETL |

**★ Three one-line decision rules:**
- **Need to reprocess old data?** → Kinesis. Only Kinesis replays.
- **Need many independent consumers of the same message?** → SNS (fan-out to SQS if you need durability).
- **Need work distributed to a pool of workers, each message handled once?** → SQS.

---

## 11. Amazon MQ (198)

- **SQS and SNS are cloud-native with proprietary AWS APIs.** Legacy on-prem apps speak open protocols instead: **MQTT, AMQP, STOMP, OpenWire, WSS.**
- **★ Amazon MQ = managed message broker for RabbitMQ and ActiveMQ.**
- **★ The trigger phrase: "migrate/lift-and-shift an existing application to AWS without re-engineering it."** If the question emphasises *not rewriting code*, the answer is Amazon MQ, not SQS.
- **Does NOT scale like SQS/SNS** — it runs on **servers**, not serverless.
- **Has both queue features (~SQS) and topic features (~SNS)** in one broker.
- **High availability: Multi-AZ with an active/standby broker, backed by EFS** for multi-AZ storage.
- **Conversely:** if the question is greenfield, cloud-native, or emphasises scale, **choose SQS/SNS, not Amazon MQ.**

---

## ⚠⚠ CONTENT GAPS — topics NOT in lectures 183–198 that ARE on SAA-C03

**These are missing, not stale. The lecture list runs 183→198 with no gap, so the course does not cover them here.** Fill them from Tutorials Dojo or AWS docs before the practice exams.

- **★★ Dead Letter Queues (DLQ).** Heavily tested. After `MaxReceiveCount` failed processing attempts, the message moves to a separate DLQ for debugging. Set DLQ retention to **14 days**. **DLQ redrive** lets you send messages back to the source queue once the bug is fixed. **Both SQS and SNS support DLQs.** Do not sit the exam without this.
- **★ SQS Delay Queues.** `DelaySeconds` delays delivery to consumers — **default 0, maximum 15 minutes.** Can be set per-queue or per-message via `DelaySeconds`.
- **★ SQS Extended Client.** The 256 KB limit is worked around by storing the payload in **S3** and sending only a reference through SQS. Trigger phrase: "messages larger than 256 KB".
- **★ Kinesis Enhanced Fan-Out.** Each consumer gets its own **2 MB/s per shard** pushed, instead of sharing 2 MB/s across all consumers. Trigger: "multiple consumers, each needs full throughput."
- **SNS retry policies and delivery status logging** — occasionally tested.

---

## ⚠ VERIFICATION STATUS

- **Externally verified this session:** SQS FIFO default throughput (300 / 3,000), FIFO high throughput mode ceiling (up to 70,000 TPS), in-flight quotas (120,000 standard / 20,000 FIFO).
- **Everything else is from my own knowledge and unverified.** The SNS subscription limits (12.5M / 100,000) and the Kinesis per-shard figures are the most likely to have drifted — confirm those three before committing.
- **Stale-content lens:** this section decays far less than S16. The only naming change is Kinesis Data Firehose → Amazon Data Firehose, which the course has already picked up.
