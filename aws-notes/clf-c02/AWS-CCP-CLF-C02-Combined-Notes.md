# AWS Certified Cloud Practitioner (CLF-C02) — Combined High-Yield Notes

> **One consolidated study file**, merged from three source notes: Domain 2 (Security & Compliance), Domain 3 (Cloud Technology & Services), and the Networking "How Data Travels" deep-dive. Every section, table, and rapid-fire trap sheet from the originals is preserved in full.
>
> **Cross-references:** the originals were written not to duplicate each other, so you'll see pointers like "see your Domain 2 notes," "Networking notes," or "your other notes files." In this combined file those now mean **the corresponding Part below** (Domain 2 → Part 1, Networking → Part 3, Domain 3 services → Part 2). Nothing those pointers reference is missing — it's just in another Part.

---

## Contents

**Part 1 · Security & Compliance — Domain 2 (30%)**
- 1 · Shared Responsibility Model (2.1) — the *shift* is what's tested
- 2 · Security Groups vs NACLs (2.4)
- 3 · "Which service does X?" — disambiguation clusters (detection · audit triad · network/app protection · secrets/keys/identity)
- 4 · IAM core (2.3) + Root user
- 5 · Encryption & compliance (2.2)
- 6 · One-line recognition (2.4)
- 7 · Rapid-fire trap sheet

**Part 2 · Cloud Technology & Services — Domain 3 (34%)**
- 0 · Cross-reference index (pairs covered in Parts 1 & 3)
- 1 · NEW confusion pairs (Storage S3/EBS/EFS · SNS/SQS/EventBridge · Lambda/Fargate/ECS/EKS)
- 2 · Compute (3.3) · 3 · Databases (3.4) · 4 · Storage (3.6)
- 5 · Deploying, operating & global infra (3.1 / 3.2)
- 6 · Networking additions (3.5)
- 7 · AI/ML + Analytics (3.7) · 8 · Other categories (3.8)
- 9 · Rapid-fire trap sheet

**Part 3 · Networking: How Data Travels — Domain 3**
- 0 · The map: Region → AZ → Edge
- 1 · Inside a VPC — the packet path
- 2 · Out to global users — edge & delivery
- 3 · Into AWS — connecting from outside (VPN/DX · peering/TGW/PrivateLink · Snow/DataSync/Gateway)
- 4 · Rapid-fire trap sheet

---

## Part 1 · Security & Compliance — Domain 2 (30%)

> Focus areas flagged: (1) how the responsibility line **shifts** by service, (2) **Security Groups vs NACLs**. Both are covered in depth below, plus the "which service does X?" clusters that produce most exam traps in this domain.

---

### 1 · Shared Responsibility Model (2.1) — the *shift* is what's tested

**The fixed rule (never changes):**
- **AWS = security OF the cloud** → physical facilities, hardware, global infrastructure, networking, hypervisor, decommissioning disks.
- **Customer = security IN the cloud** → your data, IAM configuration, and how you configure what AWS gives you.

**Always the customer, regardless of service:**
- Your **data** (and classifying it)
- **IAM** users/roles/permissions and **credentials**
- Choosing **what to encrypt** and configuring it
- Client-side data and network traffic protection you control

**Always AWS, regardless of service:**
- Physical security, the global infrastructure, hardware/host patching

**What SHIFTS (the high-yield part) — responsibility gradient:**

| Service type | Example | AWS handles | You handle |
|---|---|---|---|
| **IaaS** | EC2 | Hardware, hypervisor, network | **OS patching**, firewall (SG/NACL), app, data, IAM, encryption config |
| **Managed / PaaS** | RDS | OS + **DB engine patching**, infra | Data, DB/IAM access, encryption choice, network (SG/subnet) config, backup settings |
| **Serverless / fully managed** | Lambda, S3 | OS, runtime, scaling, **all patching** | Data, IAM permissions, function/bucket configuration |

- **The single most-tested idea:** the more managed the service, the more AWS takes on (especially **OS/engine patching**) — but **data + IAM config never leave the customer**.
- Trap: "Who patches the guest OS on an **EC2** instance?" → **Customer**. "Who patches the OS for **RDS / Lambda**?" → **AWS**.

---

### 2 · Security Groups vs NACLs (2.4) — your flagged gap

| | **Security Group (SG)** | **Network ACL (NACL)** |
|---|---|---|
| Operates at | **Instance / ENI** level | **Subnet** level |
| State | **Stateful** — return traffic auto-allowed | **Stateless** — must allow return traffic explicitly |
| Rule types | **Allow only** (no deny) | **Allow AND Deny** |
| Evaluation | **All** rules evaluated together | **Numbered order**, lowest first, **first match wins** |
| Default behavior | Denies all inbound, allows all outbound | Default NACL allows all; **custom** NACL denies all until you add rules |
| Scope | Only the resource it's attached to | **Everything in the subnet** automatically |

**Memory hooks**
- **SG = stateful** → "**S**G = **S**ticky / remembers who it let in."
- **NACL = stateless** → "**N**o memory" → must open both directions.
- Only the **NACL can explicitly Deny** (e.g., block one bad IP). An SG **cannot** block anything — it can only fail to allow.
- SG = bodyguard on the instance. NACL = border guard on the subnet.

**Common traps**
- "Stateful vs stateless?" → SG stateful, NACL stateless. (#1 trap in this domain.)
- "Block a specific malicious IP?" → **NACL** (Deny rule). An SG can't do it.
- "First line of subnet-wide defense?" → **NACL**.

---

### 3 · "Which service does X?" — the disambiguation clusters

These produce more wrong answers than anything else. Match the **trigger word** in the question to the service.

#### 3a · Detection & data-discovery

| Service | One-liner | Trigger words |
|---|---|---|
| **GuardDuty** | Threat detection by analyzing logs (VPC Flow, CloudTrail, DNS) | "detect **malicious / unusual** activity" |
| **Inspector** | Vulnerability scan of **EC2, ECR images, Lambda** | "scan for **vulnerabilities / CVEs / missing patches**" |
| **Macie** | Discovers **sensitive data / PII in S3** (uses ML) | "**PII / sensitive data** in S3" |
| **Security Hub** | Central dashboard that **aggregates** findings | "**single pane / aggregate** security findings" |
| **Detective** | **Root-cause** analysis of findings | "**investigate / root cause**" |

- Mnemonic: **Guard** the data (threats), **Inspect** the machines (vulns), **Macie** minds the **S3** (PII).

#### 3b · Audit & governance triad

| Service | Answers | Trigger words |
|---|---|---|
| **CloudTrail** | **WHO did WHAT?** (API call history) | "**who** made this change / **API** activity / audit user actions" |
| **Config** | **Is my resource configured/compliant?** (state over time) | "**resource configuration** / compliance **over time** / track config changes" |
| **CloudWatch** | **How is it performing?** (metrics, logs, alarms) | "**monitor / alarm / CPU utilization / performance**" |
| **Audit Manager** | Collects **audit evidence** for frameworks | "**evidence** for an audit" |

- Trap: **CloudTrail = actions/API**, **Config = resource state**, **CloudWatch = performance**. Don't swap CloudTrail and Config.

#### 3c · Network & application protection

| Service | Protects against | Layer |
|---|---|---|
| **Shield** (Standard = free/auto; Advanced = paid) | **DDoS** | L3/L4 (Std); + L7 & DDoS Response Team (Adv) |
| **WAF** | Web exploits — **SQL injection, XSS**, bots, geo-block | **L7 (HTTP/S)** |
| **Network Firewall** | VPC network traffic filtering | L3–L7 for the VPC |
| **Firewall Manager** | **Centrally manage** WAF/Shield/SG rules across an Org | Management |

- Trap: **Shield = DDoS**, **WAF = filtering web requests**. If it says "SQL injection / cross-site scripting" → **WAF**.

#### 3d · Secrets, keys, identity (small but frequent pairs)

**Secrets Manager vs Parameter Store**
- **Secrets Manager** → stores secrets **with automatic rotation** (e.g., DB credentials); paid per secret.
- **Parameter Store (SSM)** → config data + secrets (SecureString); **free** standard tier; **no native auto-rotation**.
- Trap: "needs automatic **rotation**" → **Secrets Manager**.

**KMS vs CloudHSM**
- **KMS** → AWS-managed, **multi-tenant**, integrated with most services, easy. (FIPS 140-2 validated.)
- **CloudHSM** → **dedicated single-tenant** hardware; **you** fully control the keys; for strict compliance. (FIPS 140-2 Level 3.)
- Trap: "**dedicated / single-tenant** HSM, full customer control" → **CloudHSM**.

**Identity services**
- **IAM** → access **within one AWS account** (users/groups/roles/policies).
- **IAM Identity Center** (formerly AWS SSO) → **SSO across multiple accounts** + external IdP; workforce access.
- **Cognito** → sign-up/sign-in for **your application's end users** (customer-facing).
- **Directory Service** → managed **Microsoft Active Directory** in AWS.
- Trap: "log in users of **my app**" → **Cognito**; "single sign-on across **AWS accounts**" → **Identity Center**.

---

### 4 · IAM core (2.3) — easy, reliable points

- **User** = a long-term identity (a person) with credentials.
- **Group** = a collection of users; attach policies to the **group**, not each user.
- **Role** = **temporary** credentials, **assumed** by a user, service, or another account — **no long-term keys**. Use for EC2→S3 access, cross-account, and federation.
- **Policy** = JSON listing permissions.
  - **Identity-based** (attached to user/group/role) vs **resource-based** (attached to the resource, e.g., an **S3 bucket policy**).
  - **AWS managed** (maintained by AWS) vs **customer managed** (you write it).
- **Best practices:** least privilege, enable **MFA**, rotate access keys, strong password policy.
- **Workloads:** prefer **roles** over hard-coded access keys (e.g., attach an **instance role** to EC2 — never store keys on the box).
- **IAM Identity Center / federation** = centralized sign-in; **cross-account roles** = grant access between accounts.

#### Root user (commonly tested)
- **Only the root user can:** change account settings (name, email, root password), **change or close the AWS Support plan**, activate **IAM access to Billing**, **close the AWS account**, enable **S3 MFA Delete**, restore lost IAM-admin access, register as a Reserved Instance Marketplace seller.
- **Protect root:** enable **MFA** on it, **delete root access keys** (never create them), and use an **IAM admin user** for daily work.

---

### 5 · Encryption & compliance (2.2)

- **At rest** = stored data (S3 SSE, EBS, RDS). Keys managed via **KMS**.
- **In transit** = data moving over the network → **TLS/SSL**; certificates issued/managed by **ACM**.
- **AWS Artifact** = self-service **compliance reports** (SOC, PCI, ISO) and **agreements** (e.g., BAA).
- **Trusted Advisor** = automated checks across **5 pillars**: cost optimization, performance, **security**, fault tolerance, service limits. Security checks include open ports, root MFA, public S3 buckets.

---

### 6 · One-line recognition (2.4) — just know what they are

- **ACM** → provision/manage **TLS/SSL certificates** (free public certs for AWS services).
- **Cognito** → app user **sign-up / sign-in** (user pools + identity pools).
- **KMS** → managed **encryption keys**.
- **CloudHSM** → **dedicated** hardware security module.
- **Directory Service** → managed **Active Directory**.
- **RAM** (Resource Access Manager) → **share resources across accounts**.
- **Network Firewall / Firewall Manager** → VPC traffic filtering / central rule management.

---

### 7 · Rapid-fire trap sheet (review last)

- SG = **stateful**; NACL = **stateless**.
- Only **NACL** can **Deny**; SG cannot block anything.
- **GuardDuty** detects (logs) · **Inspector** scans (vulns) · **Macie** = S3 PII.
- **CloudTrail** = who/API · **Config** = resource state/compliance · **CloudWatch** = performance.
- **Shield** = DDoS · **WAF** = L7 (SQLi/XSS) filtering.
- **Secrets Manager** rotates; **Parameter Store** does not (natively).
- **KMS** = shared/managed · **CloudHSM** = dedicated, you control keys.
- **Cognito** = your app's users · **Identity Center** = SSO across AWS accounts.
- Customer **always** owns **data + IAM config**; AWS **always** owns **physical infrastructure**.
- EC2 → **you** patch the OS; RDS/Lambda → **AWS** patches the OS.
- Root user → **enable MFA**, **no access keys**, use IAM admin daily.

---

## Part 2 · Cloud Technology & Services — Domain 3 (34%)

> This file covers the **new** material. It does **not** repeat what's already in your other two notes files.

---

### 0 · Already in your other notes — go there for these

| Confusion pair / topic | Where it is |
|---|---|
| SG vs NACL | Domain 2 notes |
| Config vs CloudTrail vs CloudWatch | Domain 2 notes |
| GuardDuty vs Inspector vs Macie | Domain 2 notes |
| IAM role vs user | Domain 2 notes |
| CloudFront vs Global Accelerator | Networking notes |
| VPC parts (subnets, route tables, IGW, NAT) | Networking notes |
| Region / AZ / edge location | Networking notes |
| VPN vs Direct Connect | both files |

---

### 1 · NEW confusion pairs (highest ROI)

#### 1a · Storage: S3 vs EBS vs EFS (vs instance store)

| | **S3** | **EBS** | **EFS** | **Instance store** |
|---|---|---|---|---|
| Type | **Object** | **Block** (a virtual disk) | **File** (shared NFS) | Block (ephemeral) |
| Attached to | Nothing — accessed via API/HTTP | **One** EC2 instance (same AZ) | **Many** EC2 instances (multi-AZ) | One EC2 instance (physically) |
| Persists? | Yes (independent) | Yes (independent of instance) | Yes | **No** — lost on stop/terminate |
| OS | N/A | Any | **Linux** | Any |
| Use | Files, backups, static sites, data lakes | A single server's hard drive | Shared Linux file system | High-speed temp/scratch |

- **Trap:** "shared across many instances / Linux" → **EFS**. "single instance's disk" → **EBS**. "objects, unlimited, over HTTP" → **S3**. "fast but lost on stop" → **instance store**.
- **FSx** = managed file systems for **Windows** (SMB) or specialized (Lustre for HPC, NetApp, OpenZFS). "Shared **Windows** file system" → **FSx for Windows**.

#### 1b · App integration: SNS vs SQS vs EventBridge

| Service | Model | Direction | Use |
|---|---|---|---|
| **SNS** | Pub/Sub **topic** | **Push** to many subscribers (fan-out) | Send notifications/alerts to many endpoints (Lambda, SQS, email, SMS) |
| **SQS** | Message **queue** | **Pull** (consumers poll) | **Decouple** components, buffer work, process later |
| **EventBridge** | **Event bus** | Route by **rules** | Event-driven apps; route events from AWS/SaaS/custom sources to targets; scheduling |

- **Trap:** "push / fan-out to many subscribers" → **SNS**. "decouple, buffer, poll for work" → **SQS**. "route events by rules / SaaS integration / event-driven" → **EventBridge**.

#### 1c · Compute: Lambda vs Fargate; ECS vs EKS

| Service | What it is |
|---|---|
| **Lambda** | **Serverless functions** — run code with no servers; event-driven; pay per request; short-lived (≤15 min) |
| **Fargate** | **Serverless containers** — run containers with no EC2 to manage; works under ECS *and* EKS |
| **ECS** | AWS-native **container orchestration** (simpler, AWS-specific) |
| **EKS** | Managed **Kubernetes** (for teams standardizing on Kubernetes / portability) |
| **ECR** | Container image **registry** (stores Docker images) |

- **Management spectrum (most → least you manage):** EC2 → ECS/EKS **on EC2** → **Fargate** (no host mgmt) → **Lambda** (no servers, just code).
- **Trap:** "run a function / event-driven, no servers" → **Lambda**. "run containers without managing servers" → **Fargate**. "AWS-native orchestration" → **ECS**. "managed Kubernetes" → **EKS**.

---

### 2 · Compute (3.3)

- **EC2 instance families (recognition):** General purpose (balanced) · **Compute** optimized (CPU-heavy) · **Memory** optimized (RAM-heavy, big DBs) · **Storage** optimized (high disk I/O) · Accelerated computing (GPUs).
- **Auto Scaling** = automatically add/remove capacity to match demand (**elasticity**). **ELB** = distribute incoming traffic across targets (**availability**). They work together.
  - ELB types: **ALB** (L7, HTTP/HTTPS, routing rules) · **NLB** (L4, TCP/UDP, ultra-fast, static IP).
- **Elastic Beanstalk** = **PaaS**: upload your code, AWS provisions EC2 + ELB + Auto Scaling for you (you still own the resources). "Deploy my app, don't make me configure infra" → **Beanstalk**.
- **Lightsail** = simplified **VPS** with bundled, predictable pricing. "Simple website/app, beginner-friendly" → **Lightsail**.

---

### 3 · Databases (3.4) — confusion cluster

| Service | Type | Use |
|---|---|---|
| **RDS** | Managed **relational** (MySQL, PostgreSQL, MariaDB, Oracle, SQL Server) | Standard SQL databases; AWS handles patching/backups/Multi-AZ |
| **Aurora** | AWS-built relational (MySQL/PostgreSQL-compatible) | Higher performance, cloud-native relational |
| **DynamoDB** | **NoSQL** key-value | Serverless, single-digit-ms, massive scale |
| **ElastiCache** | **In-memory cache** (Redis/Memcached) | Speed up reads / caching |
| **MemoryDB** | Durable in-memory (Redis-compatible) | In-memory **primary** database |
| **Redshift** | Data **warehouse** (OLAP) | Analytics on large datasets |
| **Neptune** | **Graph** database | Relationships: social, fraud, recommendations |
| **DMS** | Migration service | Migrate DBs to AWS with minimal downtime |
| **SCT** | Schema Conversion Tool | Convert schema when source/target engines differ |

- **Trap by keyword:** SQL/relational → **RDS/Aurora** · key-value/NoSQL → **DynamoDB** · warehouse/analytics → **Redshift** · graph/relationships → **Neptune** · caching → **ElastiCache**.
- **Managed vs EC2-hosted:** managed (RDS) = AWS does patching/backups/HA; EC2-hosted = full control but **you** do all the admin.

---

### 4 · Storage (3.6)

**S3 storage classes (frequent → archival):**

| Class | Best for |
|---|---|
| **Standard** | Frequent access; highest storage cost, no retrieval fee |
| **Intelligent-Tiering** | **Unknown/changing** access — auto-moves between tiers |
| **Standard-IA** | Infrequent access; cheaper storage, retrieval fee; multi-AZ |
| **One Zone-IA** | Infrequent access; **single AZ** (less resilient), cheapest IA |
| **Glacier (Instant / Flexible / Deep Archive)** | **Archive**; cheapest; retrieval from ms to ~12 hrs |

- **Lifecycle policies** automatically transition objects between classes (or expire them) over time.
- **Trap:** "don't know the access pattern" → **Intelligent-Tiering**. "cheapest long-term archive" → **Glacier Deep Archive**. "cheap, infrequent, OK with one AZ" → **One Zone-IA**.
- **AWS Backup** = centrally manage and automate backups across services. **Storage Gateway** = hybrid on-prem ↔ cloud storage (see networking notes).

---

### 5 · Deploying, operating & global infra (3.1 / 3.2)

**Access methods — when to use which:**

| Method | Use |
|---|---|
| **Console** | Web GUI; manual, learning, one-off tasks |
| **CLI** | Terminal commands; scripting/automation |
| **SDK** | From inside **application code** (e.g., Python boto3) |
| **CloudFormation (IaC)** | **Repeatable, version-controlled** infrastructure as code templates |

- **Trap:** "repeatable, reproducible infrastructure / version control" → **CloudFormation/IaC**. "from my app's code" → **SDK**.
- **Deployment models:** cloud · hybrid · on-premises.
- **Global infra (recap — full version in networking notes):** multi-**AZ** = high availability within a Region; multi-**Region** = disaster recovery, lower latency, data sovereignty. **Local Zones / Wavelength** = recognition only (compute closer to users / at the 5G edge).

---

### 6 · Networking additions (3.5)

- **API Gateway** = managed "front door" to create, publish, and secure **APIs** (REST/HTTP/WebSocket) for backends like Lambda or EC2.
- **Route 53** (DNS) and **VPN / Direct Connect** are in your networking notes.

---

### 7 · AI/ML + Analytics (3.7) — recognition only, don't over-invest

| AI/ML service | One-liner |
|---|---|
| **SageMaker** | Build, train, deploy ML models |
| **Rekognition** | Image/video analysis |
| **Comprehend** | NLP / text insights |
| **Lex** | Chatbots (conversational) |
| **Polly** | Text → speech |
| **Transcribe** | Speech → text |
| **Translate** | Language translation |
| **Textract** | Extract text/data from documents (OCR) |
| **Kendra** | Intelligent search |

| Analytics service | One-liner |
|---|---|
| **Athena** | SQL queries directly on S3 |
| **Kinesis** | Real-time streaming data |
| **Glue** | Serverless ETL |
| **QuickSight** | BI dashboards |
| **EMR** | Big data (Hadoop/Spark) |
| **OpenSearch** | Search/analytics on logs |

---

### 8 · Other categories (3.8) — one-liners

- **Step Functions** = serverless **workflow orchestration** across services.
- **Code\* suite** = CI/CD (CodeCommit = repo, CodeBuild = build, CodeDeploy = deploy, **CodePipeline = the pipeline**).
- **Cloud9** = cloud IDE · **CloudShell** = browser shell with the CLI ready · **X-Ray** = trace/debug app requests.
- **WorkSpaces** = managed virtual desktops (VDI) · **Connect** = cloud call center · **SES** = bulk/transactional email · **IoT Core** = connect IoT devices.

---

### 9 · Rapid-fire trap sheet — the new pairs

- **S3** = objects (HTTP) · **EBS** = one instance's disk · **EFS** = shared Linux file system · **instance store** = ephemeral.
- **FSx for Windows** = shared **Windows** file system.
- **SNS** = push/fan-out · **SQS** = pull/decouple · **EventBridge** = route events by rules.
- **Lambda** = functions (no servers) · **Fargate** = containers (no servers) · **ECS** = AWS orchestration · **EKS** = Kubernetes.
- **Auto Scaling** = elasticity · **ELB** = distribute traffic.
- **Beanstalk** = deploy an app (PaaS) · **Lightsail** = simple VPS.
- **DynamoDB** = NoSQL · **Redshift** = warehouse · **Neptune** = graph · **ElastiCache** = cache.
- **Intelligent-Tiering** = unknown access · **Glacier Deep Archive** = cheapest archive.
- **CloudFormation** = infrastructure as code · **SDK** = from app code · **CLI** = scripting.

---

## Part 3 · Networking: How Data Travels — Domain 3

> Covers the three paths you selected: **inside a VPC**, **out to global users**, and **into AWS**. Encryption *in transit* (TLS/ACM) is in your Domain 2 notes, so it's not repeated here.

---

### 0 · The map first: Region → AZ → Edge

- **Region** = a geographic area (e.g., `us-east-1`). You choose it. Data stays in-Region unless you move it.
- **Availability Zone (AZ)** = one or more isolated data centers **inside** a Region. Spread resources across AZs for high availability.
- **Edge Location** = a separate, much larger network of sites used to deliver content **close to users** (CloudFront, Route 53, Global Accelerator). **Not** the same as a Region/AZ.
- **VPC** = your private virtual network inside **one Region**; it can span multiple AZs. Everything below lives in (or connects to) a VPC.

---

### 1 · Inside a VPC — the packet path

**Core building blocks**

| Component | What it does |
|---|---|
| **VPC** | Isolated virtual network; you pick the IP range (**CIDR block**). Lives in one Region. |
| **Subnet** | An IP range inside the VPC, tied to **one AZ** (cannot span AZs). |
| **Public subnet** | Has a route to an **Internet Gateway**. |
| **Private subnet** | **No** direct route to the internet. |
| **Route table** | The "map" — rules sending traffic to a destination. Each subnet uses one route table. |
| **Internet Gateway (IGW)** | Lets **public-subnet** resources reach the internet (in + out). One per VPC. |
| **NAT Gateway** | Lets **private-subnet** instances go **outbound** to the internet (e.g., updates) while staying **unreachable from inbound**. Sits in a *public* subnet. |
| **Security Group / NACL** | Firewalls (instance-level / subnet-level — see your Domain 2 notes). |

**Inbound path (internet → public EC2):**
```
Internet → IGW → Route table (sends to subnet) → NACL (subnet) → Security Group (instance) → EC2
```
Return traffic is automatic at the SG because the **SG is stateful**.

**Outbound path (private EC2 → internet, e.g. patch download):**
```
Private EC2 → Route table (0.0.0.0/0 → NAT Gateway) → NAT Gateway (in public subnet) → IGW → Internet
```
- Nothing on the internet can **start** a connection back to that private instance — only return traffic for connections the instance opened.

**Key exam points**
- A subnet is **public only if its route table points to an IGW** — that's the defining difference, not the name.
- **NAT Gateway = outbound only** for private subnets. IGW = full internet access for public subnets.
- A subnet lives in exactly **one AZ**; design across AZs for resilience.
- **Default VPC**: AWS pre-creates one per Region (with public subnets + IGW) so you can launch immediately.

---

### 2 · Out to global users — edge & delivery

| Service | What it does | Trigger words |
|---|---|---|
| **CloudFront** | **CDN** — caches content at **edge locations** near users → lower latency. Origin can be S3, ALB, EC2, or any web server. Works with **WAF + Shield**. | "**cache / CDN / reduce latency** for global users / serve static content fast" |
| **Route 53** | Managed **DNS** + domain registration + health checks. Routing policies: simple, **weighted, latency, failover, geolocation, multivalue**. | "**DNS / domain name / route users by latency or failover**" (port **53** = DNS) |
| **Global Accelerator** | Routes users over the **AWS backbone** to the optimal endpoint using **2 static anycast IPs**. **No caching.** Good for TCP/UDP, gaming, APIs. | "**static IP / non-HTTP / fast global failover** to my app endpoints" |

**The classic trap — CloudFront vs Global Accelerator:**
- **CloudFront = caches content** (a content delivery network). Best for cacheable web content (static + dynamic).
- **Global Accelerator = optimizes the network path** to your application; it does **not** cache. Best for non-cacheable traffic, TCP/UDP, or when you need **static anycast IPs**.

---

### 3 · Into AWS — connecting from outside

#### 3a · Internet vs VPN vs Direct Connect

| Option | Path | Encrypted? | When chosen |
|---|---|---|---|
| **Internet Gateway** | Public internet | No (unless you add TLS) | Normal public access to/from a VPC |
| **Site-to-Site VPN** | **Encrypted tunnel over the public internet** | **Yes** | Quick, cheaper, secure on-prem ↔ VPC link; performance varies (it's the internet) |
| **Direct Connect (DX)** | **Dedicated private physical line** to AWS (bypasses the internet) | **No by default** (can run VPN over it) | **Consistent** low latency, high/steady bandwidth; slower to provision, costs more |

- **Trap:** "Encrypted, quick to set up, over the internet" → **VPN**. "Dedicated, consistent performance, not over the internet" → **Direct Connect**.

#### 3b · Connecting networks privately

| Service | Use |
|---|---|
| **VPC Peering** | Connect **two VPCs** privately (1-to-1, **not transitive**). |
| **Transit Gateway** | **Hub-and-spoke** to connect **many** VPCs + on-prem at scale (avoids a peering mesh). |
| **VPC Endpoint / PrivateLink** | Reach AWS services (e.g., **S3**) **privately, without the internet**. (Gateway endpoint = S3/DynamoDB; Interface endpoint = most other services.) |

#### 3c · Moving bulk data into AWS

| Service | Use | Trigger words |
|---|---|---|
| **Snow Family** (Snowcone → Snowball Edge → Snowmobile) | **Offline / physical** device shipping when the network is too slow or impractical. Snowcone = smallest/portable; Snowball Edge = tens of TB (+optional compute); Snowmobile = exabyte-scale truck.* | "**petabytes, poor/no connectivity, ship a device**" |
| **DataSync** | **Online**, automated transfer over the network to S3/EFS/FSx (and between AWS storage). | "**transfer over the network / migrate / ongoing sync**" |
| **Storage Gateway** | **Hybrid** — on-prem apps use AWS storage seamlessly (File / Volume / Tape Gateway). | "**keep on-prem access** but back to the cloud" |
| **Transfer Family** | Managed **SFTP/FTPS/FTP** into/out of S3 and EFS. | "**SFTP / FTP** into S3" |

\* *Note: AWS has stopped taking new Snowmobile orders, so it's rarely the "right" answer on newer questions — but older practice banks still list it as the exabyte option.*

- **Trap:** offline + huge + bad internet → **Snow Family**. Over the network + automated → **DataSync**. On-prem app needs cloud storage → **Storage Gateway**.

---

### 4 · Rapid-fire trap sheet (review last)

- A subnet is **public** only if its **route table points to an IGW**.
- **IGW** = full internet (public subnets); **NAT Gateway** = outbound-only (private subnets).
- A **subnet** sits in **one AZ**; a **VPC** can span AZs but stays in **one Region**.
- **CloudFront** = caches content (CDN); **Global Accelerator** = optimizes path, no cache, static IPs.
- **Route 53** = DNS + routing policies + health checks (port **53**).
- **VPN** = encrypted, over the internet; **Direct Connect** = private, dedicated, consistent, not encrypted by default.
- **VPC Peering** = two VPCs (not transitive); **Transit Gateway** = many VPCs at scale.
- **VPC Endpoint / PrivateLink** = reach AWS services without the internet.
- **Snow Family** = offline bulk; **DataSync** = online transfer; **Storage Gateway** = hybrid on-prem storage.
- **Edge locations ≠ Regions/AZs** — they're a separate, larger global network for delivery.
