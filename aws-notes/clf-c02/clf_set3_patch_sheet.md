# CLF Set-3 Patch Sheet — Jul 14 (drill before Set 4, Wed Jul 15)
**How to drill:** cover the right column / the answer, recite cold, THEN check. One full pass tonight, one cold pass Wed before the set. ⚠ = fact CHANGED after your course was recorded — trust this sheet over Maarek/old notes.

---

## §1 Billing — what's FREE vs what's CHARGED (Set-3 Q13 + high-yield adjacent)
**FREE (the service/construct itself):**
- VPCs, subnets, route tables, SGs, NACLs, IGWs — the networking *constructs* cost nothing
- CloudFormation, Auto Scaling, Elastic Beanstalk, IAM, Organizations — you pay only for the *resources they create/manage*
- Data transfer **IN** to AWS (internet → AWS) — always free
- EC2 ↔ S3 within the same region

**CHARGED:**
- ⚠ **ALL public IPv4 addresses, including Elastic IPs attached to RUNNING instances** — ~$0.005/hr, since **Feb 2024**. The old rule ("free while attached, charged when idle") is DEAD. If an option says an in-use EIP costs money → TRUE now.
- Data transfer **OUT** to the internet
- **Inter-region** transfer (EC2 files between two regions = charged)
- **Cross-AZ** transfer within a region
- NAT Gateway (hourly + per-GB) · Site-to-Site VPN connection (hourly)
- EBS volumes/snapshots of a STOPPED instance (storage keeps billing; compute stops)

**Adjacent (asked constantly):** Free Tier = 3 kinds (Always Free / 12-month / Trials) · Consolidated billing (Organizations) = combined usage → volume discounts, one bill · Cost allocation tags → cost per team/project.

## §2 Support plans — TWO models exist. ⚠ AWS restructured (late 2025/2026)
**NEW model (TD tests this):**
| Plan | Cost floor | Response (fastest) | Keys |
|---|---|---|---|
| Basic | Free | — | Account/billing Qs, quota increases, 7 core Trusted Advisor checks, Health Dashboard. NO tech support cases. |
| **Business Support+** | $29/mo per account | **Prod down < 1 hr · business-critical < 30 min** | Replaces Developer AND classic Business. 24/7 Cloud Support Engineers, FULL Trusted Advisor (500+), Support API, AI assistance. |
| **Enterprise** | $5,000/mo | **Business-critical < 15 min** | Designated **TAM** · Well-Architected Reviews · Ops Reviews · Countdown/IEM · Security Incident Response included. |
| **Unified Operations** | top tier | **< 5 min** | 24/7 monitoring, proactive ops. |

**LEGACY model (Maarek / older items may still use it):**
Basic → Developer (email, business hrs; general <24h, impaired <12h) → Business (24/7; prod impaired <4h, **prod down <1h**; full TA; IEM for extra fee) → Enterprise On-Ramp (<30 min, TAM *pool*) → Enterprise ($15k; **<15 min**, designated TAM, IEM included).

**Answer rule:** TAM / WA Reviews / Ops Reviews / IEM → **Enterprise**, in BOTH models. 15-min = Enterprise. 1-hr prod-down = Business tier (either model). 5-min = Unified Ops only.

## §3 EC2 purchasing — RI qualifiers (Set-3 Q1)
| | Standard RI | Convertible RI |
|---|---|---|
| Discount | Biggest (~72%) | Smaller (~54%) |
| Exchange | ❌ CANNOT exchange | ✅ Exchange for new RI of **EQUAL OR GREATER value** — never lesser |
| What can change | AZ, scope, size within family (Linux) | **Family, OS, tenancy, payment option** |
| Resell on RI Marketplace | ✅ | ❌ |

- "Fraction of a day/week/month on a recurring schedule" = **Scheduled RI** (legacy distractor) — not Convertible.
- Savings Plans = commit $/hr for 1–3 yr; Compute SP = most flexible (any family/region/Fargate/Lambda). Spot = up to 90% off, interruptible, fault-tolerant workloads only. Dedicated Host = physical server, use-your-own per-socket licenses.

## §4 IAM annex (Q21, Q51, Q54 — repeat family)
**What grants access to S3 (or anything):**
- **Identity-based policies** — attached to users/groups/roles ("user policies") ✅
- **Resource-based policies** — attached to the resource (S3 **bucket policy**) ✅
- SGs / NACLs = NETWORK controls, not permissions. Routing policies = Route 53. Neither grants IAM access. ❌

**Group facts:** contains many users; a user can be in multiple groups · groups CANNOT be nested · NO default all-users group · groups have account quotas (limits exist) · groups are for bulk policy attachment — they are not identities you log in as.

**Best practices (know all, any 2 can be the pair):** lock away root access keys + MFA on root · least privilege (never "most") · **managed policies over inline** (reusable, versioned) · roles for apps on EC2 (no hardcoded keys) · rotate credentials · use groups to assign permissions · Bastion host = network design, not an IAM practice.

## §5 S3 features (Q7, Q28)
**Stop deletions:** **Versioning + MFA Delete** (delete needs a second factor; MFA Delete requires versioning, enabled by root via CLI) · **Object Lock** = WORM, can't delete/overwrite for a retention period.
**Upload speed:** big single objects → **Multipart Upload** (recommended >100 MB, REQUIRED >5 GB, parallel parts, resumable) · long geographic distance → **Transfer Acceleration** (uploads via nearest edge location). `BatchWriteItem` = **DynamoDB's** API — S3 red herring.
**Adjacent:** 11 nines durability (99.999999999%) · Block Public Access = account/bucket-level public-access kill-switch · presigned URLs = temporary access · storage class ≠ speed of upload (Intelligent-Tiering = cost optimization).

## §6 EC2 mechanics (Q9, Q45, Q47, Q24)
- **Instance metadata** — info ABOUT the instance, readable from INSIDE it (instance ID, IPs, IAM role creds, kernel): `169.254.169.254`. **User data** = your boot scripts. Metadata = read about self · user data = run at launch.
- **Launch requires:** AMI, instance type, subnet/VPC (default exists), security group, root EBS volume. **Elastic IP = NEVER required** (public IPs optional).
- **Beyond default service limit:** request increase via **Service Quotas / Support Center case** (works even on Basic). Trusted Advisor only *monitors* limits — cannot raise them. Enhanced Networking = network *performance* (ENA/SR-IOV), unrelated.
- **SQL Server / DB choice:** need OS access / RDP / full DBA control → **EC2, not RDS** (RDS = managed, no OS login; Aurora ≠ SQL Server at all). License unclear or sizing unknown → **License-Included AMI** (Windows + SQL bundled, pay hourly, resize freely — no license to buy or manage). AMS ≠ a license store.

## §7 Shared Responsibility Model (Q35 — 3rd strike, kill it now)
- **HOST OS / hypervisor / hardware / facilities / network infra = AWS.** The "host" is Amazon's machine your VM sits on.
- **GUEST OS = the one YOU SSH/RDP into = CUSTOMER** — patches, firewall config, apps, IAM, customer data, client-side + at-rest encryption choices.
- One-liner: **"If you can log into it, you patch it. Host = theirs, guest = yours."**
- Shared controls: patch mgmt, config mgmt, awareness & training (AWS does infra-side, you do your side).
- Managed services (RDS, Lambda, DynamoDB) → AWS absorbs the OS/patching layer; you keep data, access, config.

## §8 AI/ML service ladder (Q2) — one line each, retrieval-drill
- **Rekognition** = analyze IMAGES/VIDEO (faces, objects, verification, moderation)
- **Polly** = text → SPEECH · **Transcribe** = speech → TEXT · **Translate** = languages
- **Comprehend** = NLP on text (sentiment, entities) · **Lex** = chatbots (Alexa tech)
- **Textract** = extract text/data from scanned DOCS/forms · **Kendra** = intelligent enterprise SEARCH (natural-language, on documents)
- **SageMaker** = build/train/deploy your OWN ML models
- Distractors: OpenSearch = search/analytics on your text/log data (not image analysis) · Macie = ML that finds SENSITIVE data in S3.
- Cue: "millions of images/faces/video" → Rekognition, always.

## §9 CAF — Security perspective capabilities (Q30)
Security governance · security assurance · IAM · **threat detection** (find/monitor attacks) · vulnerability management · **infrastructure protection** (defend boundaries/systems against unauthorized access + vulns) · data protection · application security · incident response.
**Verb cue:** "protect / prevent access" → *protection* capabilities · "detect / find / monitor" → threat detection · "respond / recover" → incident response. (TD mapped "protect apps against unauthorized access & vulnerabilities" → **Infrastructure Protection**.)
Adjacent: CAF has 6 perspectives — Business, People, Governance (business-y) · Platform, Security, Operations (technical).

## §10 Cloud concepts (Q50, Q65)
**Six advantages (know cold, they hide in Select-TWOs):** trade fixed (CAPEX) for variable (OPEX) expense — never the reverse · massive economies of scale · stop guessing capacity · **increase speed & agility = reduce time to market** · stop spending on data centers → lower TCO · go global in minutes.
**Best-practice → principle map:** SOA / microservices → **decouple your components** · resilience → design for failure · scaling → elasticity · big jobs split up → think parallel · servers are disposable → cattle, not pets.

## §11 Pen testing (Q57)
**Permitted WITHOUT prior approval (the classic 8 — TD-aligned):** EC2 instances · RDS · **Aurora** · CloudFront · API Gateway · Lambda (+Lambda@Edge) · Lightsail · Elastic Beanstalk environments.
**Not on the no-approval list:** S3, IAM, STS (AWS-operated service layers — attacking AWS's own infrastructure is never allowed).
**Always prohibited:** DoS/DDoS + simulations (separate policy), port/request flooding, DNS zone walking via Route 53.

## §12 Trap patterns that got you in Set 3 — pre-read these Wed
1. **Qualifier flip:** "equal or *lesser*" vs "equal or *greater*" (Q1) — when two options differ by ONE word, that word IS the question.
2. **Reversed statement:** "replace low variable costs with upfront CAPEX" (Q65) — read direction of trade, not just keywords.
3. **Wrong-service API:** BatchWriteItem on an S3 question (Q28) — ask "does this API even belong to this service?"
4. **Preventive vs detective verbs:** protect→protection, detect→detection (Q30).
5. **Negative stems:** "NOT required" (Q45) — mark the NOT, then hunt the optional item (EIP, key pair = the usual optionals).
6. **Select-TWO discipline:** per-option TRUE/FALSE each of the 5; exactly 2 should come out true. 6 of your 19 were Select-TWO — this alone is ~2 points.
7. ⚠ **Stale-world check:** support plans + public-IPv4 pricing changed after your course. If an option matches THIS sheet but contradicts Maarek — this sheet wins.
