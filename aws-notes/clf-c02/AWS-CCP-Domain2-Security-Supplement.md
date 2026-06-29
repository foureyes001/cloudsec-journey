# AWS Certified Cloud Practitioner (CLF-C02)
## Domain 2 — Security & Compliance · Supplement (Part 2)

> Companion to your Domain 2 Security notes. Adds the **core, frequently-tested** topics that file doesn't cover — it does **not** repeat the shared responsibility model, SG vs NACL, the detection/audit clusters, IAM basics, Artifact, KMS vs CloudHSM, or Trusted Advisor.
>
> Headline addition: **AWS Organizations + Service Control Policies** — multi-account governance is a major Domain 2 area the original file skips.

---

## 1 · AWS Organizations & multi-account governance — high-yield

| Concept | What it does | Trigger words |
|---|---|---|
| **AWS Organizations** | Centrally manage **multiple AWS accounts** — consolidated billing + central governance | "manage many accounts / one combined bill / central control" |
| **Organizational Units (OUs)** | Group accounts (e.g., by department or environment) so policies apply to the whole group | "group accounts to apply policies" |
| **Service Control Policies (SCPs)** | **Permission guardrails** at the org / OU / account level — they set the **maximum** allowed permissions | "restrict what accounts can do / set guardrails across the org" |

- **Critical SCP fact:** SCPs **do not grant** permissions — they only **restrict**. They define the *ceiling* of what's allowed. Even an account's root/admin user **cannot exceed an SCP**. Effective permissions = what IAM allows **AND** what the SCP allows.
- **Consolidated billing** (via Organizations): one bill for all accounts, and **combined usage can unlock volume discounts** shared across accounts.
- **Trigger:** "prevent any account from using a specific service/region, no matter their IAM" → **SCP**.

---

## 2 · Account setup & governance tooling — recognition

- **AWS Control Tower** — automates setup of a secure, best-practice **multi-account "landing zone"** with pre-configured guardrails (built on top of Organizations). *"Quickly stand up a well-governed multi-account environment"* → **Control Tower**.
- **IAM Access Analyzer** — identifies resources (S3 buckets, IAM roles, KMS keys, etc.) that are **shared with external entities**. *"Find resources accessible from outside my account/org"* → **Access Analyzer**.

---

## 3 · IAM deeper — how permissions are actually evaluated

- **Evaluation logic (commonly tested):**
  - By default, **all requests are denied** (implicit deny).
  - An **explicit Allow** grants access.
  - An **explicit Deny** overrides **any** Allow.
  - → **Explicit deny always wins.**
- **Policy structure:** JSON with **Effect** (Allow/Deny), **Action**, **Resource**, and optional **Condition**.
- **Permission boundaries** (recognition) — set the **maximum** permissions an IAM user/role can have, even if their attached policy grants more.
- **Least privilege** (in main notes) — grant only what's needed.

---

## 4 · MFA & credential security

- **MFA adds a second factor** beyond the password. Types: **virtual MFA** (authenticator app), **hardware MFA device**, and **FIDO security keys**.
- **Enable MFA on the root user** and all privileged identities.
- **Access keys** = programmatic credentials (CLI/SDK). Rotate them, and **never embed in code** — use IAM roles instead.
- **Password policy** — enforce length/complexity, rotation, and reuse prevention for IAM users.

---

## 5 · Encryption & DDoS — the next level of detail

**KMS key types** (the original covers KMS vs CloudHSM; this is the key breakdown):
- **AWS-managed keys** — created and managed by AWS for a service (e.g., `aws/s3`); you can't manage them directly.
- **Customer-managed keys (CMK)** — **you** create, control, rotate, and audit them (via CloudTrail). Choose these when you need control over the key lifecycle/policy.
- **AWS-owned keys** — used internally by AWS; not visible in your account.
- **Envelope encryption** (concept): KMS encrypts a **data key**, and that data key encrypts your actual data.

**AWS Shield — Standard vs Advanced** (the original lists Shield briefly):
- **Shield Standard** — **free**, automatic for all AWS customers; protects against common **L3/L4** DDoS attacks.
- **Shield Advanced** — **paid**; enhanced L3/L4/L7 protection, access to the **Shield Response Team (SRT)**, and **cost protection** against DDoS-driven usage spikes.

---

## 6 · Rapid-fire additions (review last)

- **AWS Organizations** = manage many accounts + consolidated billing. **OUs** = group accounts. **SCPs** = permission **guardrails** (max permissions; restrict-only, never grant).
- An **SCP can block even an account admin** — effective permission = IAM allow **AND** SCP allow.
- **Control Tower** = automated multi-account landing zone. **Access Analyzer** = find externally-shared resources.
- **IAM evaluation:** default deny → explicit allow grants → **explicit deny always wins.**
- **MFA types:** virtual (app), hardware device, FIDO security key. Always on root.
- **KMS keys:** AWS-managed · **customer-managed (you control/rotate)** · AWS-owned.
- **Shield Standard** = free/automatic (L3/L4). **Shield Advanced** = paid, SRT + cost protection.
