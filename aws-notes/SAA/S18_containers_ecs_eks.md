# S18 — Containers on AWS: ECS, Fargate, ECR & EKS

Lectures 199–208 + Quiz 15. **11 items, 49 min.** Section title and count confirmed from the course sidebar.
Lab policy: **WATCH ONLY.** No lab owed.

---

## Docker (199)

- Package an app + dependencies into a **container** that runs identically anywhere.
- **Docker vs VM:** VMs each carry a **guest OS** on a hypervisor. Containers **share the host OS kernel** via the Docker daemon → more containers per server, lighter.
- Flow: `Dockerfile` → **build** → image → **push** → repository → **pull** → **run** → container.
- Image registries: **Docker Hub** (public) · **Amazon ECR** (private + ECR Public Gallery).
- Use cases: microservices · lift-and-shift from on-premises.

**The four AWS container services — know which is which:**

| Service | What it is |
|---|---|
| **ECS** | AWS's own container orchestrator |
| **EKS** | Managed **Kubernetes** |
| **Fargate** | **Serverless** compute engine — works with **both** ECS and EKS |
| **ECR** | Image registry |

---

## Amazon ECS — launch types (200)

| | **EC2 Launch Type** | **Fargate Launch Type** |
|---|---|---|
| Infrastructure | **You provision and maintain EC2 instances** | **None — serverless** |
| Agent | Each EC2 instance runs the **ECS Agent** to register with the cluster | N/A |
| Scaling | Add EC2 instances | **Just increase the number of tasks** |
| You define | Cluster + instances + task definitions | **Task definitions only** (CPU/RAM) |

- **★ Trigger phrase:** "no infrastructure to manage" / "reduce operational overhead" → **Fargate**.

----

## ★★ IAM roles for ECS — highest-frequency ECS question

| Role | Attached to | Used for |
|---|---|---|
| **EC2 Instance Profile** *(EC2 launch type only)* | The **EC2 instance** | ECS Agent API calls · send logs to **CloudWatch Logs** · **pull images from ECR** · read **Secrets Manager / SSM Parameter Store** |
| **ECS Task Role** | The **task**, defined in the **task definition** | Per-task, per-service AWS permissions |

- **★ The discriminator:** infrastructure-level actions → **instance profile**. Application-level actions → **task role**.
- **★ Different ECS services get different task roles.** If a question wants one service to reach DynamoDB and another S3, the answer is separate **task roles**, not one shared instance profile.

---

## Load balancer integration

- **ALB** — supported, works for **most** use cases. Default answer.
- **NLB** — only for **high throughput / high performance**, or to pair with **AWS PrivateLink**.
- **CLB** — supported but **not recommended**; no advanced features, **does not work with Fargate**.

---

## ★★ Data volumes — commonly tested

- **Amazon EFS** can be mounted onto ECS tasks. **Works with BOTH EC2 and Fargate.**
- Tasks in **any AZ share the same data** → persistent, multi-AZ shared storage.
- **Fargate + EFS = fully serverless persistent storage.** This is the intended answer for "serverless containers that need shared persistent state."
- **★ Amazon S3 CANNOT be mounted as a file system.** Classic distractor.

---

## ECS Auto Scaling (203)

**Two different things — do not confuse them:**

- **ECS Service Auto Scaling** = scales the **number of TASKS**. Uses **AWS Application Auto Scaling**.
- **EC2 Auto Scaling** = scales the **number of EC2 INSTANCES**.

**Three metrics for service scaling:**
- ECS Service **Average CPU Utilization**
- ECS Service **Average Memory Utilization**
- **ALB Request Count Per Target**

**Three scaling policy types:**
- **Target Tracking** — target value on a CloudWatch metric
- **Step Scaling** — driven by a CloudWatch **Alarm**
- **Scheduled Scaling** — date/time, for **predictable** changes

**Scaling the EC2 layer (EC2 launch type only), two ways:**
- **ASG Scaling** — scale the ASG on CPU utilization.
- **★ ECS Cluster Capacity Provider** — pairs with an ASG and **automatically provisions instances when tasks lack CPU/RAM capacity.** This is the modern/preferred answer for "tasks stuck in PENDING because there is no room."

- **Fargate auto scaling is far simpler** — no instance layer to scale at all.

---

## ★ ECS solution architectures (204)

- **EventBridge → Run ECS Task** — e.g. S3 upload event triggers a task that processes the object and writes to DynamoDB.
- **EventBridge Schedule → ECS Task** — hourly/daily batch jobs.
- **ECS + SQS** — service polls the queue, scales on queue depth.
- **EventBridge rule on `ECS Task State Change` → SNS** — alert on stopped/failed tasks.

---

## Amazon ECR (206)

- Stores and manages Docker images. **Private** repos + **ECR Public Gallery**.
- **★ Backed by Amazon S3.**
- **★ Access controlled by IAM. A pull failure is a permissions problem** — that phrasing in a question means an IAM policy fix.
- Features: **image vulnerability scanning**, versioning, image tags, **lifecycle policies**.

---

## Amazon EKS (207, 208)

- Managed **Kubernetes** on AWS. Same goal as ECS, **different API**.
- **★ The trigger phrase: already running Kubernetes** on-premises or in another cloud → **EKS**, because **Kubernetes is cloud-agnostic** and ECS is AWS-proprietary.

**Node types — three, know all three:**

| Type | Detail |
|---|---|
| **Managed Node Groups** | EKS creates and manages the EC2 nodes in an ASG. **On-Demand or Spot.** |
| **Self-Managed Nodes** | You create and register nodes; managed by your own ASG. Can use the **EKS-Optimized AMI**. On-Demand or Spot. |
| **Fargate** | **No nodes to manage at all.** |

**EKS data volumes:**
- Requires a **StorageClass** manifest on the cluster + a **CSI-compliant driver**.
- Supported: **EBS · EFS (the only one that works with Fargate) · FSx for Lustre · FSx for NetApp ONTAP**.

---

## ★★ Exam trap list

1. **"Already using Kubernetes"** → EKS, never ECS.
2. **"No infrastructure to manage"** → Fargate.
3. **Pull image from ECR fails** → **EC2 Instance Profile / IAM permissions**, not the task role.
4. **Per-application AWS permissions** → **ECS Task Role**.
5. **Shared persistent storage across AZs for containers** → **EFS**. Never S3 — it cannot be mounted.
6. **Fargate + persistent storage** → **EFS only** (EBS is not the ECS-Fargate answer).
7. **Tasks stuck PENDING, no capacity** → **ECS Cluster Capacity Provider**.
8. **Scale tasks vs scale instances** → Service Auto Scaling vs EC2 Auto Scaling. Read which layer the question is asking about.
9. **CLB with Fargate** → not supported. Always wrong.
10. **Very high throughput / PrivateLink in front of ECS** → **NLB**, otherwise **ALB**.

---

## ⚠ Two things to add — not covered in these lectures but testable

- **ALB dynamic port mapping** (EC2 launch type): multiple tasks of the same service on **one EC2 instance** work because the ALB maps to random host ports. Question shape: "run multiple copies of the same container on one instance."
- **Fargate Spot** — up to ~70% cheaper, interruptible. Answer for fault-tolerant/batch container workloads.

---

⚠ **Verification:** not externally checked this session. Container service fundamentals are stable, but the **EKS node-type list and the CSI storage list** are the two most likely to have drifted. One confirmation pass before the practice exams.
