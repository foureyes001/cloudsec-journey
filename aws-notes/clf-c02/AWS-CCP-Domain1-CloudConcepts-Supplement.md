# AWS Certified Cloud Practitioner (CLF-C02)
## Domain 1 — Cloud Concepts (24%) · Supplement (Part 2)

> Companion to your existing Cloud Concepts notes. This sheet adds the **core, frequently-tested** Domain 1 topics that file doesn't cover — it does **not** repeat the 6 advantages, IaaS/PaaS/SaaS, Well-Architected pillars, CAF, or the migration R's.
>
> Biggest addition: **cloud deployment models**, which your other file points to Domain 3 — but they're tested as a Domain 1 *concept* and deserve a clean treatment here.

---

## 1 · Cloud deployment models — high-yield, often a 1-line scenario

> Don't confuse these **deployment models** (where things run) with the **service models** IaaS/PaaS/SaaS (how much you manage) in your main Domain 1 file.

| Model | What it is | Trigger words |
|---|---|---|
| **Cloud / Cloud-native** (public cloud) | The entire application runs **in the cloud** — all parts built on or migrated to AWS | "fully in AWS / all resources in the cloud / cloud-native" |
| **Hybrid** | **Mix** of cloud resources **and** on-premises infrastructure, connected together | "keep some workloads on-prem **and** extend to AWS / connect data center to cloud" |
| **On-premises** (private cloud) | Resources deployed **on-prem** using virtualization/resource-management tools (incl. **AWS Outposts** to run AWS services in your own data center) | "everything stays in our own data center / private cloud" |

- **The hybrid signal:** any question mentioning **both** on-prem **and** cloud, or needing to keep certain data/workloads on-prem (latency, compliance, legacy) while using AWS for the rest → **hybrid**.
- The *connectivity* that makes hybrid work (VPN, Direct Connect) is in your **Networking notes**; **AWS Outposts** also appears in the Global Infrastructure list in your main Domain 1 file.

---

## 2 · Benefits of migrating to AWS (1.3) — the half your migration section doesn't cover

Your other file has the migration **strategies** (the R's) and **CAF**. The exam also tests the **benefits** of moving to the cloud. Common ones:

- **Reduced cost** — CapEx → OpEx, pay-as-you-go, lower TCO.
- **Business agility / faster time to market** — launch and iterate in minutes.
- **Operational resilience** — higher availability and easier disaster recovery.
- **Scalability & elasticity** — handle demand spikes without over-provisioning.
- **Staff productivity / focus** — stop maintaining data centers; focus on the product.
- **Security & compliance** — inherit AWS's certifications and security tooling.

- **Trigger:** "why migrate to the cloud?" → cost savings, agility, resilience, scalability. (Map each benefit back to the **6 advantages** in your main file.)

---

## 3 · Cloud application design principles (1.2) — beyond the pillars

The Well-Architected **pillars** are in your other file; the exam also tests these **architecture best practices** (they reinforce the Reliability & Performance pillars):

- **Design for failure** — assume any component can fail; build in redundancy so one failure doesn't take the system down.
- **Loose coupling / decoupling** — keep components independent so one can fail or scale without breaking others. (Enabled by **SQS / SNS / EventBridge** — see Domain 3 notes.)
- **Scale horizontally, not just vertically** — add **more** instances (horizontal) rather than only a **bigger** instance (vertical); horizontal scaling is more resilient.
- **Automate everything** — use Infrastructure as Code and Auto Scaling instead of manual steps.
- **Prefer managed/serverless services** — "services, not servers" — let AWS run the undifferentiated heavy lifting.

- **Memory hook:** *Fail → Decouple → Scale out → Automate → Go managed.*
- **Trigger:** "build a resilient/highly available architecture" → design for failure + decouple + scale horizontally across **multiple AZs**.

---

## 4 · AWS Well-Architected Tool (1.2) — recognition

- A **free tool in the AWS console** that lets you **review a workload against the 6 pillars** and get improvement recommendations.
- **Three-way disambiguation (all sound alike):**
  - **Well-Architected Framework** = the 6 **pillars** / design guidance (a *concept*).
  - **Well-Architected Tool** = the *service* that **reviews** your workload against those pillars.
  - **WAF** = **Web Application Firewall** (L7 security — Domain 2). Unrelated, despite the name.

---

## 5 · How AWS pricing works (1.4) — concept layer

> Detailed pricing models (On-Demand, Reserved, Spot, Savings Plans) and the Free Tier live in **Domain 4 (Billing)** — this is just the Domain 1 economics concept.

**The pricing principles:**
- **Pay-as-you-go** — pay only for what you use, no upfront commitment.
- **Save when you commit** — Reserved Instances / Savings Plans discount steady usage.
- **Pay less by using more** — volume-based (tiered) discounts; per-unit cost drops at scale.
- **Prices drop over time** — AWS passes on its economies of scale.

**The 3 main cost drivers** (what you're billed for):
- **Compute** (e.g., EC2 running time)
- **Storage** (e.g., S3, EBS)
- **Outbound data transfer** — **data transfer IN is generally free; data transfer OUT costs.** (Common trap.)

- **Trigger:** "which incurs cost — data into AWS or out of AWS?" → **out**. Inbound is typically free.

---

## 6 · Quick definitions worth knowing (1.1)

- **Cloud computing** (AWS's definition): the **on-demand delivery of IT resources over the internet with pay-as-you-go pricing** — no buying/owning/maintaining physical data centers.
- **Multi-cloud** — using more than one cloud provider (e.g., AWS + another). Recognition only.

---

## 7 · Rapid-fire additions (review last)

- **Deployment models:** **Cloud** (all in AWS) · **Hybrid** (cloud **+** on-prem, connected) · **On-premises/private** (your data center, e.g., **Outposts**). On-prem **and** cloud together → **hybrid**.
- Deployment model = *where it runs*; service model (IaaS/PaaS/SaaS) = *how much you manage*. Don't mix.
- **Benefits of migrating:** cost savings · agility/time-to-market · resilience · scalability · staff focus · security.
- **Design principles:** design for failure · loose coupling (SQS/SNS/EventBridge) · scale **horizontally** · automate · prefer managed/serverless.
- **Well-Architected Framework** (pillars/concept) ≠ **Well-Architected Tool** (the review service) ≠ **WAF** (firewall).
- **Pricing principles:** pay-as-you-go · save when you commit · pay less at volume · prices fall over time.
- **Cost drivers:** compute · storage · **outbound** data transfer. **Data IN = free, data OUT = paid.**
- **Cloud computing** = on-demand IT over the internet, pay-as-you-go.
