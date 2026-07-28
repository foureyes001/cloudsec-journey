# Design Doc — Project #2: AWS Security Scanner (G5b)

**Author:** (you)
**Date:** 2026-07-27
**Status:** Design frozen. Build begins Month 3.
**Repo:** `cloudsec-journey`
**Related:** `decisions/project2_why.md` (why this project was chosen over alternatives)

> Lines marked **[A]** were contributed by the assistant, not decided by me — kept
> deliberately, or to be struck before commit. Everything unmarked is my decision.

---

## 1. Purpose

A scheduled, read-only AWS account scanner. It checks four common security
misconfigurations on a fixed cadence and emails me when it finds one.

The point is not to build a better Prowler. The point is to demonstrate that I can
design a least-privilege AWS workload end to end — including the scanner's own
permissions, which is the part most portfolio projects skip.

**Non-goal:** comprehensive coverage. Four checks, deliberately.

---

## 2. Scope — the four checks (FROZEN)

| # | Check | Why it matters |
|---|---|---|
| 1 | Security groups open to `0.0.0.0/0` | Public exposure of admin ports (22/3389) is the most common real-world entry point |
| 2 | CloudTrail multi-region trail enabled | Without it, activity in unmonitored regions leaves no audit record |
| 3 | Root account has MFA | Root is unrestricted by IAM policy; MFA is the only meaningful control on it |
| 4 | Access keys older than 90 days | Long-lived static credentials are the most-leaked AWS secret |

**This list is frozen.** New check ideas go to §7 (Parked), not into v1. Scope creep is
the primary risk named in `project2_why.md`; this freeze is the mitigation.

---

## 3. Architecture

```
   EventBridge Scheduler        Lambda (Python + boto3)         SNS Topic
  +---------------------+      +----------------------+      +-------------+
  |   cron trigger      |      |  4 read-only checks  |      |    email    |
  |   twice weekly      |----->|  execution role §5   |----->| subscription|-----> me
  +---------------------+      +----------------------+      +-------------+
          invoke                          |                      publish
                                          v
                                  CloudWatch Logs
```

| Service | Role | Configuration |
|---|---|---|
| **EventBridge Scheduler** | cron trigger | **Twice weekly.** Example: `cron(0 9 ? * MON,THU *)` — 09:00 UTC Mon & Thu **[A: example only — pick the days/time; note 09:00 UTC = 14:30 IST]** |
| **Lambda** | runs the scan | Python runtime, boto3. **Memory 128 MB** (workload is API calls, not computation). **Timeout 60 s [A: recommended — the 3 s default will time out on sequential API calls plus cold start; timeout costs nothing when unused, billing is on actual duration]** |
| **SNS** | delivery | One topic, one email subscription (confirm once by clicking the link) |
| **CloudWatch Logs** | observability | Default Lambda logging; the only place a failed run is visible |

**No VPC.** The Lambda calls public AWS APIs only and needs no VPC attachment —
adding one would require NAT for egress and cost real money for zero benefit.

---

## 4. Flow

1. EventBridge Scheduler fires on the twice-weekly cron and invokes the Lambda.
2. Lambda assumes its **execution role** (§5). No static access keys exist anywhere
   in this system — credentials are obtained from the role at runtime.
3. boto3 runs the four checks, all read-only:
   - `ec2.describe_security_groups()` → inspect inbound rules for `0.0.0.0/0`
   - `cloudtrail.describe_trails()` / `get_trail_status()` → check `IsMultiRegionTrail` and logging state
   - `iam.get_account_summary()` → read `AccountMFAEnabled`
   - `iam.list_users()` → per user, `iam.list_access_keys()` → compare `CreateDate` against 90 days
4. Results collected into a structured list of `{check, resource, status, detail}`.
5. `sns.publish()` sends **one combined email per run**, listing all four check
   results together — not one email per finding.
6. Failures and exceptions land in CloudWatch Logs.

### 4.1 Notification policy — two phases (deliberate)

- **v1 — verification mode: email on EVERY run**, findings or not.
  Rationale: before I can trust the scanner's silence, I need proof it actually
  runs. During this phase, every email is evidence the whole pipeline is alive.
- **v2 — after 3 consecutive clean runs, switch to findings-only.**
  Trigger condition is explicit so the switch happens by plan, not by drift.
- **Cost of switching:** one line in the handler — publish unconditionally becomes
  `if findings: publish`. No infrastructure change; the topic, subscription, cron
  and IAM policy are untouched. Redeploy the function, done.
- **Known tradeoff, accepted:** once in findings-only mode, silence is ambiguous —
  it means either "all clear" or "the scanner is broken." **[A]** The v1 verification
  phase is what buys confidence before accepting that ambiguity. If it ever bites,
  the cheap fix is a CloudWatch alarm on the Lambda `Errors` metric pointed at the
  same SNS topic. Not building it now.

---

## 5. IAM — least privilege for the execution role

**This is the core section.** The scanner's own permissions are the security lesson
of this project, not an implementation detail.

Two identities, not to be conflated:
- **Execution role** — what the Lambda *is* while running. Gets the policy below.
- **The resources scanned** (SGs, CloudTrail config, IAM users/keys) — what the Lambda
  *inspects*. It never modifies them.

### 5.1 Decision: narrow inline policy, not managed `SecurityAudit`

I chose a hand-written inline policy of ~7 actions over the AWS-managed
`SecurityAudit` policy.

`SecurityAudit` grants read access across essentially the entire account —
hundreds of `Describe*`/`List*`/`Get*` actions over 100+ services. For a scanner
that touches four, that is not least privilege; it is convenience. It would also
still require an inline statement for `sns:Publish`, so it doesn't avoid writing
JSON anyway.

The narrow policy demonstrates the principle rather than describing it. It also
fails loudly: if the code ever calls something outside its grant, I get
`AccessDenied` naming the exact permission, instead of silently succeeding.

**Accepted cost:** I maintain it. Adding a check later means updating the policy.
Given the scope freeze, that's a cost I'm choosing to pay.

### 5.2 Permissions granted

| Action | Resource | Justification |
|---|---|---|
| `ec2:DescribeSecurityGroups` | `*` | Check 1. EC2 `Describe*` actions do not support resource-level permissions — `*` is forced by the API |
| `cloudtrail:DescribeTrails` | `*` | Check 2 — enumerates all trails, so cannot be pre-scoped |
| `cloudtrail:GetTrailStatus` | `*` in v1 | Check 2 — logging state. **Narrowable to a trail ARN; left wide because the trail to check isn't known until `DescribeTrails` returns** |
| `iam:GetAccountSummary` | `*` | Check 3 — account-level API, no resource ARN exists |
| `iam:ListUsers` | `*` | Check 4 — enumerates all users; cannot be pre-scoped |
| `iam:ListAccessKeys` | `*` in v1 | Check 4 — key `CreateDate`. **Narrowable to `arn:aws:iam::<account-id>:user/*`; tighten at build time** |
| `sns:Publish` | **`arn:aws:sns:us-east-1:<account-id>:<topic-name>`** | Delivery — **scoped to the one topic**, not `*` |
| `logs:CreateLogGroup`, `logs:CreateLogStream`, `logs:PutLogEvents` | log group ARN | Standard Lambda logging |

### 5.3 Security properties this policy guarantees

- **No write, modify, or delete actions exist anywhere in the policy.** The scanner
  is structurally incapable of changing the account, regardless of code bugs.
- **The only non-read action is `sns:Publish`, scoped to a single topic ARN.**
- **No static credentials.** The role supplies short-lived credentials at runtime.

**Honest note on the wildcards.** Four of the six read actions genuinely cannot be
resource-scoped — the APIs are account-level or enumerate everything by definition.
Two (`iam:ListAccessKeys`, `cloudtrail:GetTrailStatus`) **can** be narrowed further,
and are left wide in v1 as a simplification, not because the API forces it. Recording
the difference rather than claiming all six are unavoidable — a least-privilege doc
that overstates its own tightness defeats the purpose. **Tightening those two is a
v1.1 refinement, not a parked feature.**

### 5.4 Known limitation **[A]**

`iam:ListUsers` does not return the root user, so check 4 covers IAM user keys but
not root access keys. `iam:GetAccountSummary` already returns
`AccountAccessKeysPresent`, which would flag their existence — noted here as a real
gap in coverage, **not** added to v1 scope.

---

## 6. Cost

**Target: $0/month. Expected: $0/month.**

At twice weekly, the scanner runs roughly **8–9 times per month**.

| Service | Usage | Cost |
|---|---|---|
| Lambda | ~9 invocations, seconds each, 128 MB | $0 — orders of magnitude inside the always-free tier |
| EventBridge Scheduler | ~9 scheduled invocations | ~$0 |
| SNS | ~9 email notifications | $0 — well inside the free email allowance |
| CloudWatch Logs | a few KB of logs | ~$0 |
| Resources scanned | reads only | $0 — describing resources is free |

**Backstop: a $5 AWS cost budget with an alarm is already live** in this account. If
anything in this project ever produces a real charge, I hear about it immediately.

**[A] Verify current free-tier figures at build time** — pricing pages change, and the
stale-world lens applies. The order of magnitude (single-digit invocations/month) is
robust regardless.

**Cost risk if scope ever creeps:** per-account scanning across an Organization, or a
high-frequency trigger, are the two changes that would move this off $0. Both are
explicitly out of scope (§7).

---

## 7. Parked — deliberately NOT building

Recorded so that scope creep hits a decision I already made, rather than a decision
I make at 11pm mid-build.

| Item | Status |
|---|---|
| **Auto-remediation** (auto-closing port 22 to `0.0.0.0/0`) | Gated stretch. **At most ONE** stretch total, only after §8 is fully met |
| **Prowler-comparison README paragraph** | The other gated stretch — one or the other, not both |
| **Any 5th+ check** | Frozen out. New ideas are logged here, not built |
| **Slack or non-email delivery** | SNS-email is v1 |
| **Multi-account / Organization-wide scanning** | Single account in v1 |
| **CloudWatch `Errors` alarm** | Deferred — see §4.1; only if findings-only mode proves to hide failures |
| **Config-driven notification toggle** (env var) | Rejected — the one-line code edit is simpler than a config system |

---

## 8. Done-definition

v1 is complete when **all** of these are true:

- [ ] Deployed, with the twice-weekly cron confirmed firing
- [ ] A **real finding** produced from one deliberately-misconfigured test resource
      (create a security group open to `0.0.0.0/0`, confirm it's caught, delete it)
- [ ] README containing a sample report and the §3 architecture diagram
- [ ] Clean commits throughout — specific-file adds, honest messages, never `git add .`
- [ ] The execution role is the narrow inline policy from §5, and the README explains why

Stretch goals are gated behind all five boxes and capped at one.

---

## 9. Build seeds (existing code to reuse)

- `utils.py` — pending refactor; becomes the shared helper module for the scanner
- `cloudtrail_recent_events.py` — existing boto3 CloudTrail code; check 2 builds on it

---

## 10. Decision log

| Decision | Choice | Rationale |
|---|---|---|
| Cadence | Twice weekly | Enough to catch drift; low noise |
| IAM policy | Narrow inline (~7 actions) | Demonstrates least privilege rather than describing it (§5.1) |
| Notification | Every run in v1 → findings-only after 3 clean runs | Verify the pipeline works before trusting its silence |
| Email format | One combined email per run | Standard; suits verification mode |
| Lambda memory | 128 MB | API-bound, not compute-bound |
| Lambda timeout | 60 s **[A: recommended]** | 3 s default will time out on sequential calls + cold start |
| Delivery | SNS → email | Simplest thing that works; no extra services |
| VPC | None | Public APIs only; VPC would require NAT and real cost |
| Write permissions | None | Read-only is a structural guarantee, not a convention |
| Cost target | $0, $5 budget alarm live | Free tier by design |
