# Design Doc — AWS Notes (assistant-assembled research half)
## Project #2: boto3 Security Scanner — G5b

> **How to use this:** These are the AWS facts + the decision points. You write the
> **decisions** (the "we will do X because Y" lines) into the actual design doc.
> I gather; you decide (rule 8). Every ★ DECISION below is a blank you fill.
> **Scope is FROZEN** — 4 checks, EventBridge → Lambda → SNS. If a new idea appears
> while writing, it goes in the "Rejected / Parked" section, not the build.

---

## 1. What the doc must contain (G5b definition, from the handoff)
A ~45-min doc covering: **services · flow · least-privilege IAM for the scanner's
execution role · cost.** That's the checklist. Below, each section gives you the
AWS facts and the decision to record.

---

## 2. Services (the frozen four checks + the pipeline)

**The 4 checks (LOCKED — do not add a 5th):**
1. Security Groups open to `0.0.0.0/0` (esp. ports 22 / 3389)
2. CloudTrail multi-region trail is enabled
3. Root account has MFA
4. Access keys older than 90 days

**Pipeline services (read-only scanner):**

| Service | Role in pipeline | Key facts you may cite |
|---|---|---|
| **EventBridge Scheduler** | cron trigger, fires the Lambda on schedule | Rate/cron expressions; runs in your account; the modern replacement for CloudWatch Events rules. A daily/weekly `rate()` is plenty. |
| **Lambda** | runs the boto3 scan logic | Python runtime; needs an **execution role** (this is your hidden 5th lesson — least-priv IAM applied to itself). Default 128 MB / 3 s is likely too low; a scan hitting several APIs wants ~256 MB and a 60–120 s timeout. |
| **boto3** (inside Lambda) | the actual AWS API calls, read-only | Each check maps to specific read APIs (table below). |
| **SNS** | delivers findings by email | Topic + email subscription (must confirm the subscription once). Lambda publishes a formatted message. |
| **IAM** | the execution role's permissions | The doc's core section — see §4. |

**boto3 API mapping (so the IAM policy in §4 is exact, not guessed):**

| Check | boto3 call(s) | IAM action needed |
|---|---|---|
| SG open to 0.0.0.0/0 | `ec2.describe_security_groups()` | `ec2:DescribeSecurityGroups` |
| CloudTrail multi-region on | `cloudtrail.describe_trails()` (and/or `get_trail_status`) | `cloudtrail:DescribeTrails`, `cloudtrail:GetTrailStatus` |
| Root MFA | `iam.get_account_summary()` (field `AccountMFAEnabled`) | `iam:GetAccountSummary` |
| Access key age > 90d | `iam.list_users()` → `iam.list_access_keys()` → read `CreateDate` | `iam:ListUsers`, `iam:ListAccessKeys` |
| Publish findings | `sns.publish()` | `sns:Publish` (on your topic ARN only) |

> **★ DECISION 2a:** State the trigger cadence (daily vs weekly) and why.
> **★ DECISION 2b:** State Lambda memory + timeout and why (tie to "several sequential API calls").
> **★ DECISION 2c:** Confirm the delivery channel is SNS-email for v1 (not Slack/EventBridge-to-anything — those are parked).

---

## 3. Flow (the request path — write this as a numbered sequence or a small diagram)

The sequence the doc should describe:
1. **EventBridge Scheduler** fires on cron → invokes **Lambda**.
2. **Lambda** assumes its **execution role** (no static keys — see §4 note).
3. boto3 runs the 4 checks (read-only calls from the table above).
4. Findings collected into a structured result (list of {check, resource, status}).
5. If any finding is non-compliant → **SNS `publish()`** → **email** to you.
6. (Optional, per done-definition) a compliant run can stay silent or send an all-clear.

> **★ DECISION 3a:** Does an all-clear run notify, or only failures? (Cost/noise tradeoff — email is effectively free at this volume, but noise trains you to ignore alerts.)
> **★ DECISION 3b:** One combined email per run, or one per finding? (Combined is standard; state it.)

**Diagram note:** a simple left-to-right box chain is enough:
`EventBridge → Lambda (boto3, read-only) → SNS → Email`.
The README wants this diagram anyway (done-definition), so drawing it now doubles up.

---

## 4. Least-privilege IAM for the execution role (THE core section)

This is the section that makes it a *security* project rather than a script. Two
distinct identities — don't conflate them:

- **Execution role** = what the Lambda *is* while running. Gets the read-only policy below.
- **Root/keys/SGs** = what the Lambda *inspects*. It never modifies them (v1 is read-only).

**Least-privilege principles to state explicitly in the doc:**
- **No wildcards on actions** where avoidable — list the exact actions from §2's table, not `ec2:*` or `iam:*`.
- **Resource-scope what you can.** `sns:Publish` should be scoped to *your topic ARN*, not `*`. The `iam:` and `ec2:` describe/list calls are account-level reads and generally need `Resource: *` (they don't take a resource ARN) — **state that this is why the wildcard exists there**, so the reader knows it's reasoned, not lazy.
- **No write/delete actions at all in v1.** The role literally cannot change anything. That's the security property worth naming.
- **AWS managed `SecurityAudit` policy exists** and covers most read calls — but for the doc, a hand-written inline policy of ~6 actions demonstrates least-privilege far better than attaching a broad managed policy. Cite that you *chose* the narrow inline policy over `SecurityAudit` deliberately.

**Sketch of the policy (you finalize the exact JSON in the build; the doc just needs the shape + reasoning):**
- Statement 1 — read-only describes/lists: the 6 actions from §2, `Resource: *` (justified: these APIs are account-scoped reads).
- Statement 2 — `sns:Publish`, `Resource: <your-topic-ARN>` (justified: scoped to the one topic).
- Plus the standard Lambda basic-execution logging perms (`logs:CreateLogGroup/Stream/PutLogEvents`) — mention these exist so CloudWatch Logs work.

> **★ DECISION 4a:** Inline narrow policy vs managed `SecurityAudit` — record your choice + the one-line why.
> **★ DECISION 4b:** State the read-only-in-v1 property as an explicit security guarantee.
> **★ DECISION 4c:** Note where a wildcard resource is unavoidable and why (the account-scoped IAM/EC2 reads).

---

## 5. Cost (short section — the point is "this is free-tier / near-zero")

Order-of-magnitude facts for a daily-or-weekly scan in one account:

- **Lambda:** free tier = 1M requests + 400k GB-seconds/month. A scan running once
  a day = ~30 invocations/month, seconds each. **Effectively $0.**
- **EventBridge Scheduler:** first 14M scheduler invocations/month are free-tier-ish;
  a daily trigger is ~30/month. **~$0.**
- **SNS:** first 1,000 email notifications/month are free; you'll send a handful.
  **~$0.**
- **CloudWatch Logs:** tiny log volume from a short Lambda; pennies at most, likely
  inside free tier.
- **The resources being scanned** (SGs, CloudTrail, IAM) cost nothing to *read*.

> **★ DECISION 5a:** State the expected monthly cost (≈ $0 / within free tier) and name
> your existing **cost budget + alarm** as the backstop (already live — mention it).
> **★ DECISION 5b:** Note the one real cost risk if scope ever creeps (e.g. per-account
> scanning across an Org, or high-frequency triggers) — and that it's explicitly out of v1 scope.

---

## 6. Rejected / Parked (write these so scope-creep is on the record)

Per the handoff, scope creep is the **named primary risk**. Pre-empt it in the doc:
- **Auto-remediation** (e.g. auto-closing port 22 to 0.0.0.0/0) — **parked**, listed as
  the single *gated* stretch goal, at most ONE stretch, only after v1 done-definition.
- **Prowler-comparison paragraph** — the other gated stretch; at most one of the two.
- **5th+ check of any kind** — frozen out. New check ideas go here, not into v1.
- **Non-email delivery (Slack, etc.)** — parked; SNS-email is v1.
- **Multi-account / Org-wide scan** — parked; single-account v1.

---

## 7. Done-definition (restate at the doc's end, from the handoff — so "done" is unambiguous)
- Deployed; cron firing on schedule.
- A **real finding** produced from one deliberately-misconfigured test resource
  (e.g. an SG you open to 0.0.0.0/0 on purpose, then delete after).
- README with a sample report + the architecture diagram from §3.
- Clean commits (specific-file adds, honest messages, never `git add .`).
- Hidden 5th lesson demonstrated: the scanner's own least-priv execution role (§4).

---

## Fill-in checklist (the decisions that make this YOUR doc, not my notes)
- [ ] 2a cadence · 2b Lambda mem/timeout · 2c SNS-email confirmed
- [ ] 3a all-clear notify? · 3b combined vs per-finding email
- [ ] 4a inline vs SecurityAudit · 4b read-only guarantee stated · 4c wildcard justification
- [ ] 5a expected cost + budget backstop · 5b scope-creep cost risk noted
- [ ] 6 parked list written · 7 done-definition restated

Each checkbox = one or two sentences from you. That's the ~30–40 min.
```
