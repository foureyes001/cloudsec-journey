# AWS Certified Cloud Practitioner (CLF-C02)
## Domain 3 — Cloud Technology & Services · Supplement (Part 2)

> Companion to your Domain 3 Services notes. Adds **core, frequently-tested** topics that file doesn't cover — it does **not** repeat S3/EBS/EFS, SNS/SQS/EventBridge, Lambda/Fargate/ECS/EKS, EC2 families, Auto Scaling/ELB, the database list, S3 storage classes, or the access methods.
>
> Headline addition: **EC2 purchasing options** — one of the most-tested topics on the exam, and absent from the original.

---

## 1 · EC2 purchasing options — high-yield (also relevant to Domain 4)

| Option | Commitment / nature | Best for |
|---|---|---|
| **On-Demand** | Pay per second/hour, no commitment; highest hourly rate | Short-term, **spiky/unpredictable** workloads; dev/test |
| **Reserved Instances (RI)** | 1- or 3-year commitment; up to ~72% discount | **Steady, predictable** usage |
| **Savings Plans** | Commit to a $/hour amount for 1–3 years; more flexible than RIs | Steady usage, with flexibility (Compute Savings Plans also cover **Fargate & Lambda**) |
| **Spot Instances** | Use spare capacity; up to ~90% discount; **can be interrupted** (2-min notice) | **Fault-tolerant / flexible / batch** workloads |
| **Dedicated Hosts / Instances** | Physical server/hardware dedicated to you | **Compliance** or **bring-your-own-license (BYOL)** |

- **Triggers:** "steady & predictable, lowest long-term cost" → **Reserved / Savings Plans**. "interruptible, cheapest, batch" → **Spot**. "short-term, unpredictable" → **On-Demand**. "dedicated hardware / licensing / compliance" → **Dedicated Hosts**.

---

## 2 · Amazon RDS — Multi-AZ vs Read Replicas (a classic trap)

| | **Multi-AZ** | **Read Replicas** |
|---|---|---|
| Purpose | **High availability / failover** | **Scale read traffic** |
| Replication | **Synchronous** standby in another AZ | **Asynchronous** copies |
| Readable? | No — standby is for failover only | Yes — serve read queries |
| Can span Regions? | (Within Region by default) | Yes — cross-Region supported |

- **Trigger:** "automatic failover / survive an AZ outage / high availability" → **Multi-AZ**. "offload or scale read queries" → **Read Replicas**. (They solve **different** problems — don't swap them.)

---

## 3 · Key Amazon S3 features (beyond storage classes)

The original covers storage classes and lifecycle policies. The exam also tests these **features**:

- **Versioning** — keep multiple versions of an object; protects against accidental **delete/overwrite**.
- **Replication (CRR / SRR)** — automatically copy objects to another bucket (**Cross-Region** or **Same-Region**) for DR/compliance/latency. *(Requires versioning.)*
- **S3 Transfer Acceleration** — faster long-distance uploads via CloudFront edge locations.
- **Static website hosting** — serve a static site directly from a bucket.
- **S3 Object Lock** — **WORM** (write-once-read-many); block deletion for retention/compliance.
- **Encryption** — server-side encryption (SSE-S3, SSE-KMS, SSE-C) protects data at rest.
- **Durability** — designed for **99.999999999% (11 nines)** durability.
- **Trigger:** "protect against accidental deletion" → **Versioning**. "copy objects to another Region automatically" → **Cross-Region Replication**. "speed up uploads from far away" → **Transfer Acceleration**.

---

## 4 · AWS Storage Gateway — the three types

The original lists Storage Gateway as "hybrid storage." The exam tests the **three gateway types**:

- **File Gateway** — files via **NFS/SMB**, stored as objects in **S3**.
- **Volume Gateway** — **block** storage via **iSCSI**, backed by EBS snapshots.
- **Tape Gateway** — virtual tape library for **backup/archive** to S3/Glacier.
- **Trap:** "Object Gateway" and "Block Gateway" are **not** real names — the three are **File, Volume, Tape**.

---

## 5 · AWS Lambda — common triggers / event sources

The original covers what Lambda *is*; the exam often tests **what triggers it**:

- **Amazon S3** — run code when an object is **uploaded** (e.g., resize an image, process a log).
- **Amazon EventBridge** — run on a **schedule** (cron) or in response to an event.
- **API Gateway** — run in response to an **HTTP request**.
- **SNS / SQS / DynamoDB Streams** — run in response to messages/changes.
- **Patterns to remember:** S3 upload → **Lambda** · scheduled job → **EventBridge + Lambda** · "**execute code**" needs a **compute** service (Lambda), **not** a queue (SQS only *stores* messages).

---

## 6 · Operations & management services

The original is light on operations. These are testable (and several map to questions you've already seen):

- **AWS Systems Manager** — a suite to manage resources at scale: **Patch Manager** (OS patching), **Run Command**, **Parameter Store** (config/secrets), **Automation**, **Inventory**, and:
  - **Session Manager** — **shell access to EC2 with no open inbound ports, no public IP, and no SSH keys** (access goes through the SSM agent). *Contrast: EC2 Instance Connect uses SSH over port 22.*
- **AWS Health Dashboard** — two views:
  - **Service health** — **public** status of **all** AWS services across **all** Regions.
  - **Your account health** — **personalized** events affecting **your** account/resources.
- **Recognition:** **AWS Batch** (run batch computing jobs at scale) · **AWS App Runner** (easily deploy containerized web apps) · **AWS Outposts** (AWS hardware running **on-premises** / hybrid).
- **Cost & optimization** tools (Compute Optimizer, Cost Explorer, Budgets) live in **Domain 4 (Billing)**.

---

## 7 · Rapid-fire additions (review last)

- **EC2 pricing:** On-Demand (spiky) · **Reserved/Savings Plans** (steady, discounted) · **Spot** (interruptible, cheapest) · **Dedicated Hosts** (compliance/BYOL).
- **RDS Multi-AZ = HA/failover** (sync standby, not readable) · **Read Replicas = read scaling** (async, readable).
- **S3 features:** Versioning (anti-delete) · Cross-Region Replication · Transfer Acceleration (fast uploads) · static website hosting · Object Lock (WORM) · 11-nines durability.
- **Storage Gateway:** **File, Volume, Tape** (not "Object/Block").
- **Lambda triggers:** S3 upload · EventBridge schedule · API Gateway request. "Execute code" → **Lambda**, not SQS.
- **Session Manager** = shell to EC2, **no ports / no public IP / no SSH keys**.
- **AWS Health Dashboard:** Service health (public, all services) vs Your account health (personalized).
