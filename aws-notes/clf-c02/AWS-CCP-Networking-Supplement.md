# AWS Certified Cloud Practitioner (CLF-C02)
## Networking — Supplement (Part 2)

> Companion to your "How Data Travels" networking notes. Adds **core, frequently-tested** topics that file doesn't cover — it does **not** repeat VPC parts (subnets, route tables, IGW, NAT), the packet paths, CloudFront vs Global Accelerator, VPN vs Direct Connect, or the Snow/DataSync/Storage Gateway split.
>
> Headline addition: the **Global vs Regional vs AZ scope** reference — the concept you repeatedly missed in practice.

---

## 1 · Global vs Regional vs AZ scope — high-yield, repeatedly tested

Most AWS resources are **Regional**; a handful are **Global**; a few are tied to a **single AZ**. Knowing which is which answers a whole family of questions.

| Scope | Services / resources | Memory anchor |
|---|---|---|
| **Global** (one view across all Regions) | **IAM, Route 53, CloudFront, AWS Organizations, Billing/Cost Management, AWS WAF (with CloudFront)** | Identity, DNS, and edge are global |
| **Regional** (you pick a Region; it lives there) | **VPC, S3 (bucket), EC2 service, RDS, DynamoDB, Lambda, SNS, SQS, Elastic IP, AMI** | Most things you create |
| **Availability Zone** (tied to one AZ) | **EC2 instance, EBS volume, subnet** | The physical building blocks |

- **Trigger:** "which is a **global** service?" → **IAM / Route 53 / CloudFront**. "which is tied to a **single AZ**?" → **EBS volume, subnet, EC2 instance**.
- **Watch-outs:** an **AMI** and an **EBS snapshot** are **Regional** (copy them to use in another Region); an **EBS volume** is **AZ-scoped**; a **VPC** is Regional but **spans AZs**, while a **subnet** lives in just one AZ.
- This is the common exam framing — a few services have nuances (e.g., WAF is global only when attached to CloudFront), but the table above covers what's tested.

---

## 2 · IP addressing in a VPC

| Type | Behavior | Trigger words |
|---|---|---|
| **Private IP** | Internal VPC address, **not** internet-routable | "internal communication within the VPC" |
| **Public IP** | Internet-routable; auto-assigned in public subnets, but **changes** when the instance stops/starts | "temporary public address" |
| **Elastic IP (EIP)** | A **static** public IPv4 address you allocate and keep; remappable; **persists** across stop/start | "**static public IP that doesn't change**" |

- **Trigger:** "need a fixed public IP that survives a reboot/restart" → **Elastic IP**.
- Note: you're charged for an Elastic IP that's **allocated but not associated** with a running resource.

---

## 3 · VPC Flow Logs — network visibility

- Capture information about the **IP traffic** going to and from network interfaces in your VPC.
- Used for **monitoring, troubleshooting, and security analysis**; published to **CloudWatch Logs or S3**.
- **Trigger:** "capture / monitor IP traffic in a VPC" → **VPC Flow Logs**. (Don't confuse with CloudTrail = API calls, or CloudWatch = metrics.)

---

## 4 · Amazon Route 53 routing policies — recognition

The original notes cover Route 53 = DNS. The exam may name a **routing policy**:

- **Simple** — one record, one destination.
- **Weighted** — split traffic by assigned percentages (A/B testing).
- **Latency-based** — send users to the Region with the **lowest latency**.
- **Failover** — primary/standby for **high availability**.
- **Geolocation** — route by the **user's location**.
- **Geoproximity** — route by geographic distance, with bias.
- **Multivalue answer** — return multiple healthy records.

- **Trigger:** "route users to the lowest-latency Region" → **Latency-based**. "active/passive DR" → **Failover**. "send a % of traffic to a new version" → **Weighted**.

---

## 5 · More connectivity options — recognition

- **Client VPN vs Site-to-Site VPN:** **Site-to-Site VPN** connects an **entire on-prem network** to AWS; **Client VPN** lets **individual remote users/devices** connect to AWS.
- **NAT gateway vs NAT instance:** **NAT gateway** is the AWS-managed, highly-available, recommended option; a **NAT instance** is an older, self-managed EC2-based approach.
- **Direct Connect Gateway** (recognition) — connect a Direct Connect link to **multiple VPCs across Regions**.
- **Accessing private instances:** historically a **bastion host / jump box** was used; the modern, no-open-ports approach is **Systems Manager Session Manager** (see your Services supplement).

---

## 6 · Rapid-fire additions (review last)

- **Global:** IAM, Route 53, CloudFront, Organizations, Billing, WAF (with CloudFront). **AZ-scoped:** EC2 instance, EBS volume, subnet. Everything else → **Regional**.
- **AMI / EBS snapshot = Regional** · **EBS volume / subnet = single AZ** · **VPC = Regional, spans AZs.**
- **Elastic IP** = static public IP that **persists** across stop/start (charged if unused).
- **VPC Flow Logs** = capture/monitor IP traffic in a VPC (→ CloudWatch Logs or S3).
- **Route 53 policies:** latency (lowest-latency Region) · failover (active/passive) · weighted (% split) · geolocation (by user location).
- **Site-to-Site VPN** = whole network → AWS · **Client VPN** = individual users → AWS.
- **NAT gateway** (managed) > **NAT instance** (self-managed).
