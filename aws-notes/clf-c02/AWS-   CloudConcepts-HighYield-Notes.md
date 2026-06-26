# AWS Certified Cloud Practitioner (CLF-C02)
## Domain 1 — Cloud Concepts (24%) · High-Yield Notes

> The domain that cratered on practice exam 3 (22%) — and the one with no notes until now. Built to close the exact gaps that showed up: cloud **value / agility** (Q10), **IaaS/PaaS/SaaS** (Q15), **AWS Global Infrastructure** components (Q27), and the **Well-Architected pillar for IaC** (Q59).
>
> Cross-references: data-migration tools and Region/AZ/edge depth live in your **Networking notes**; DMS/SCT and deployment-model detail are in your **Domain 3 (Services) notes**; the shared-responsibility shift is in your **Domain 2 (Security) notes**. Not repeated here.

---

## 1 · The 6 advantages of the AWS Cloud (1.1) — memorize this list

AWS's official **Six Advantages of Cloud Computing** (directly tested):

1. **Trade capital expense (CapEx) for variable expense (OpEx)** — pay only for what you consume, when you consume it.
2. **Benefit from massive economies of scale** — AWS's aggregated usage drives lower pay-as-you-go prices.
3. **Stop guessing capacity** — scale up/down on demand; no over- or under-provisioning.
4. **Increase speed and agility** — spin up resources in minutes, not weeks → experiment and innovate faster.
5. **Stop spending money running and maintaining data centers** — focus on your customers, not racking servers.
6. **Go global in minutes** — deploy across Regions worldwide with a few clicks → low latency for users.

- **Q10 trigger:** "innovate faster / rapidly develop, test, and launch applications" → **agility / speed** (advantage #4).

---

## 2 · Core benefit concepts (1.1) — the distinctions that get tested

| Concept | What it means | Trigger words |
|---|---|---|
| **Agility** | Provision/experiment fast; low cost of trying ideas | "innovate / develop-test-launch faster" |
| **Elasticity** | **Automatically** grow *and* shrink with demand (Auto Scaling) | "auto add/remove to match demand" |
| **Scalability** | Ability to grow capacity (vertical = bigger instance; horizontal = more instances) | "handle increased load / grow" |
| **High Availability (HA)** | Stays *up* despite a component/AZ failure (Multi-AZ) | "minimize downtime / survive an AZ outage" |
| **Fault tolerance** | Keeps running *through* failures with **zero** interruption (full redundancy) | "no downtime even if components fail" |
| **Disaster recovery (DR)** | Restore service after a major outage (often Multi-Region) | "recover after a disaster / region failure" |
| **Reliability** | Recover from failures and meet demand | "dependable / recovers automatically" |

- **Elasticity vs scalability:** elasticity is *automatic and demand-driven, both directions*; scalability is the *ability to grow*.
- **HA vs fault tolerance vs DR:** HA = minimize downtime · fault tolerance = **no** downtime (redundant) · DR = recover *after* a disaster.
- Architecture services that deliver these (ELB, Auto Scaling, Multi-AZ) are in your **Domain 3 (Services) notes**.

---

## 3 · Cloud computing service models — IaaS / PaaS / SaaS (Q15)

| Model | You manage | Provider manages | Examples |
|---|---|---|---|
| **IaaS** (Infrastructure) | OS, runtime, apps, data | Hardware, network, virtualization | **EC2**, VPC |
| **PaaS** (Platform) | Just your app + data | OS, runtime, scaling, infra | **Elastic Beanstalk**, RDS |
| **SaaS** (Software) | Nothing — just use it | Everything | **Gmail**, Dropbox, Salesforce; AWS apps like Rekognition/QuickSight |

- **Control decreases, convenience increases:** IaaS → PaaS → SaaS (mirrors the shared-responsibility shift in your Domain 2 notes).
- **Q15 trap:** Gmail = a ready-to-use application you don't manage → **SaaS**.
- **Don't confuse** service models (IaaS/PaaS/SaaS) with **deployment models** (cloud / hybrid / on-premises) — deployment models are in your **Domain 3 notes (3.1)**.

---

## 4 · AWS Well-Architected Framework (1.2) — the 6 pillars

> **Acronym trap:** "Well-Architected Framework (WAF)" = **6 design pillars**. This is **NOT** the same as **WAF = Web Application Firewall** (the L7 security service in your Domain 2 notes). Same letters, unrelated.

| Pillar | Key idea | Signature design principle |
|---|---|---|
| **Operational Excellence** | Run, monitor, and continuously improve | **Perform operations as code (IaC)**; frequent small reversible changes |
| **Security** | Protect data and systems | Least privilege, encryption, traceability, security at every layer |
| **Reliability** | Recover from failure, scale to demand | Auto-recovery, test recovery, scale horizontally |
| **Performance Efficiency** | Use resources efficiently | Right-size, go serverless, experiment |
| **Cost Optimization** | Avoid unnecessary spend | Pay for what you use, right-size, use managed services |
| **Sustainability** | Minimize environmental impact (added 2021) | Maximize utilization, use managed services, right-size |

- **Q59 trap:** "which pillar recommends maintaining **infrastructure as code**?" → **Operational Excellence**.
- **Memory hook:** **O-S-R-P-C-S** (Operational, Security, Reliability, Performance, Cost, Sustainability).
- General design principles (recognition): stop guessing capacity · test at production scale · automate to ease experimentation · allow evolutionary architectures · drive decisions with data · improve through game days.

---

## 5 · Migration: Cloud Adoption Framework + strategies (1.3)

**AWS Cloud Adoption Framework (CAF)** — organizes adoption into **6 perspectives**:

- **Business capabilities:** Business · People · Governance
- **Technical capabilities:** Platform · Security · Operations

> **Don't swap:** **CAF = 6 perspectives** (above) vs **Well-Architected = 6 pillars** (§4). Both are "6 things," easy to confuse.

**Migration strategies — the "R"s** (you move each app via one):

| Strategy | Meaning |
|---|---|
| **Rehost** | "Lift and shift" — move as-is |
| **Replatform** | "Lift, tinker, and shift" — small optimizations (e.g., move to a managed DB) |
| **Repurchase** | "Drop and shop" — switch to a different product, often **SaaS** |
| **Refactor / Re-architect** | Redesign for cloud-native |
| **Retire** | Decommission what's no longer needed |
| **Retain** | Keep on-prem for now; revisit later |

- (7th, **Relocate** — move at the hypervisor level, e.g., VMware Cloud on AWS.)
- **Migration tools:** Snow Family / DataSync / Storage Gateway / Transfer Family are in your **Networking notes**; **DMS / SCT** (database migration) are in your **Domain 3 notes**.

---

## 6 · Cloud economics (1.4)

- **CapEx → OpEx:** trade large upfront capital expense for pay-as-you-go operating expense.
- **Economies of scale:** AWS's scale lowers per-unit cost, passed on as lower prices.
- **Pay-as-you-go / consumption-based:** pay only for what you use; no forced long-term commitment (though Savings Plans / Reserved Instances discount commitment).
- **Total Cost of Ownership (TCO):** cloud lowers TCO vs on-prem — no data-center CapEx, power, cooling, hardware refresh, or facilities staff.
- **Right sizing:** match resource size to the actual workload to cut waste.
- **Tool: AWS Pricing Calculator** (`calculator.aws`) — estimate the cost of a planned architecture. *(The older "AWS TCO Calculator" and "Simple Monthly Calculator" are retired — some old practice banks still name them.)*
- Cost levers (detail in Billing/Domain 4): Savings Plans, Reserved Instances, Spot, right-sizing, S3 storage classes.

---

## 7 · AWS Global Infrastructure (1.1 / 1.2) — what counts (Q27)

> Full Region/AZ/edge mechanics are in your **Networking notes** — this is the concept layer Domain 1 tests.

**Components that ARE "AWS Global Infrastructure":**

- **Region** — a geographic area containing multiple AZs.
- **Availability Zone (AZ)** — one or more isolated data centers inside a Region.
- **Edge locations / Points of Presence** — content-delivery sites (CloudFront, Route 53, Global Accelerator).
- **Local Zones** — compute/storage placed closer to large population centers.
- **Wavelength Zones** — at the 5G network edge.
- **AWS Outposts** — AWS infrastructure running **on-premises** (hybrid).

- **Q27 trap:** "which is part of the AWS Global Infrastructure?" → Regions / AZs / edge locations (and Local Zones / Wavelength / Outposts). Things you **provision** — a **VPC, subnet, EC2 instance, security group** — are **not** global infrastructure; they're resources you create *within* it.

---

## 8 · Rapid-fire trap sheet (review last)

- **6 advantages:** CapEx→variable · economies of scale · stop guessing capacity · **speed/agility** · no data centers · go global in minutes.
- "Innovate faster / develop-test-launch quickly" → **agility**.
- **Elasticity** = auto grow/shrink with demand · **Scalability** = ability to grow · **HA** = stay up (Multi-AZ) · **Fault tolerance** = no downtime (redundant) · **DR** = recover (Multi-Region).
- **IaaS** = you control OS/infra (EC2) · **PaaS** = just deploy code (Beanstalk) · **SaaS** = ready-to-use app (**Gmail**).
- **Well-Architected = 6 pillars** (design) vs **WAF service = Web Application Firewall** (security) — same acronym, different thing.
- Pillars: **Operational Excellence, Security, Reliability, Performance Efficiency, Cost Optimization, Sustainability.**
- **IaC / "operations as code" → Operational Excellence pillar.**
- **CAF = 6 perspectives** (Business, People, Governance, Platform, Security, Operations) — don't swap with the 6 pillars.
- Migration **R's:** Rehost (lift-shift) · Replatform (lift-tinker-shift) · Repurchase (drop-shop / SaaS) · Refactor (re-architect) · Retire · Retain.
- Economics: **CapEx→OpEx**, economies of scale, pay-as-you-go, lower **TCO**, estimate with **AWS Pricing Calculator**.
- **Global Infrastructure** = Regions / AZs / edge / Local Zones / Wavelength / Outposts. A **VPC or subnet is NOT** global infrastructure — it's something you create.
