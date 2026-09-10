# DECISIONS.md — Fingerprinting AI Agents in Cloud Audit Logs

Solo · Aug 2026 → Jan/Feb 2027 · design decisions and open items.

**What this file is**
- **Part A is append-only.** Never edit a past line. A reversal is a *new* line naming what it supersedes.
- **Part B is edited in place**, under one rule: a line may only leave Part B by being answered in Part A **in the same commit**.
- **This file records WHY. Git records WHAT and WHEN.** No "work done today" entries.
- **Status values:** `RATIFIED` (decided) · `APPLIED` (in use, not formally ratified) · `SUPERSEDED` (name the replacement).

> `[A]` — this file was drafted by the assistant from the design sessions. Rewrite lines in your own words when you next touch them; the encoding happens in the writing.

> **Supersedes the earlier uncommitted draft (D-001…D-009).** The feature set, task design and actor-class definitions in that draft were reworked from the ground up. Decisions from it that survive unchanged are carried into Part A below with their original reasoning intact, marked `[carried]`.

---

## PART A — DECISION LOG

### Carried forward — still live

| # | Topic | Status | Decision |
|---|---|---|---|
| 1 | Project scope | RATIFIED | Title *Fingerprinting AI Agents in Cloud Audit Logs*, locked as submitted. Approved, topic locked, solo, prior-work reuse permitted. `[carried]` |
| 2 | Session boundary | RATIFIED | **A session = one task execution.** Every feature is computed per session; a wrong window makes every feature wrong. Rejected: time-gap (breaks on agent LLM latency, and CloudTrail is second-granularity with no intra-second ordering) and per-role/per-credential (too coarse). `[carried]` |
| 3 | Phase 0 ordering | RATIFIED | Session definition precedes task design, because a task's completion criterion *is* a session boundary. Writing tasks first decides the session definition by accident. `[carried]` |
| 4 | Human actor mode | RATIFIED | CLI primary, console secondary. `[carried]` |
| 5 | Repo folder | RATIFIED | `agent-fingerprinting/`, not `idp/`. In an AWS context "IdP" reads as Identity Provider — wrong meaning to exactly the intended audience. Self-contained so it can be extracted later; **copy** `cloudtrail_recent_events.py` in rather than importing it. `[carried]` |
| 6 | Provenance | RATIFIED | College origin is **not** concealed. Screeners discount a signature (repo appears weeks before a deadline, burst commits, templated README, no design record), not the word "college" — and this repo defeats that signature. Choosing a name to prevent a true inference is a misleading omission. College-facing admin (decks, guide names, registration numbers) stays out of the public repo. `[carried]` |
| 7 | Dataset durability | RATIFIED | Treat the AWS account as **ephemeral** (free access ends Dec 2, 2026). Pull raw session logs and parsed sequences to local storage and commit continuously. The dataset is a deliverable and part of the novelty claim; it must never exist in only one place. `[carried]` |
| 8 | Working agreement | RATIFIED | Purpose before task — state what a task is *for* in one line before starting it. `[carried]` |

### Item 1 — feature set

| # | Topic | Status | Decision |
|---|---|---|---|
| 9 | Feature families | RATIFIED | **Seven families accepted**, ablation performed at family level rather than per feature. |
| 10 | Spoofability model | RATIFIED | **Three tiers** — declarative / behavioural-cost / server-determined. Supersedes the earlier two-column spoofable-vs-not table. |
| 11 | Blind experiment design | RATIFIED | **Staged removal, L0–L3.** L1 is the headline result and delivers the contribution sentence verbatim; L2 and L3 are robustness checks. |
| 12 | Feature disposition | RATIFIED | F3 and F7 demoted but still computed · F13, F14, F19–F26 accepted · F27 deferred to Phase 4. |
| 13 | Interest test | RATIFIED | Families F and G are **ablated together**. This is the "is the result interesting, or just speed?" test — if removing both destroys performance, the result is a latency artifact. |
| 14 | Timing correction | RATIFIED | Injected/fake delays change **pace, not shape**. Timing features must be shape-based to survive trivial spoofing. |
| 15 | Prompt parity | RATIFIED | Added as Risk 4: agent and human must receive equivalent task framing, or the comparison measures prompt quality. |
| 16 | Asymmetry between features and hooks | RATIFIED | **Features can be added later for free. Hooks cannot** — a hook must exist in the task before collection begins. This asymmetry drives the whole task design below. |

### Item 1 re-verification — six changes

| # | Topic | Status | Decision |
|---|---|---|---|
| 17 | Post-error response | RATIFIED | F1, F21, blind retry and halt are **mutually exclusive outcomes of one moment** — what the call after a failure is. Merged into a single categorical "post-error response" feature (retry / repair / re-route / abandon / halt). As four features they were correlated by construction. **F6 kept separate** for no-error abandonment. |
| 18 | F16 hook | RATIFIED | Switched to **error-returning dead ends**. |
| 19 | Information parity | RATIFIED | **Nobody gets the inventory** — not agent, not script author, not human. F4 demoted to supporting. |
| 20 | Role assumption | APPLIED | **Fresh role assumption per session** as collection protocol. Provisionally closes the earlier open item, pending a check in the probe. ⚠ Real-world sessions do not always re-assume; note as a limitation. |
| 21 | Minor dispositions | RATIFIED | F22 positional · F5 folded into F14 · F26 rides free · F20 gets a fallback rule. |
| 22 | Rejected features | RATIFIED | Throttling-retry (identical by construction across classes — record as a **tested non-discriminator**, worth one line in the report) · time-of-day (leaks *who*, not *what*) · formatting quirks (too fragile) · unique-call ratio (already covered by the demoted F3). |

### Item 2 — task design **(CLOSED ON DESIGN)**

| # | Topic | Status | Decision |
|---|---|---|---|
| 23 | ★ Hook coverage rule | RATIFIED | **Each hook must appear in ≥3 different tasks; core hooks in ≥5.** Reason: a hook appearing in one task only means that feature has a value in one task only — the classifier cannot separate "agents adapt after denials" from "task 6 is different." **Feature and task collapse into the same variable.** |
| 24 | ★ Task count | RATIFIED | **12 tasks + 1 control, 3 hooks each.** Arithmetic: core hooks (planted denial, dead end, unnamed target) × 5 = 15 placements; secondary hooks (non-guessable IDs, two routes, checkable end, large listing, two services, repeated units, three sub-goals) × 3 = 21; **36 placements needed.** At 3 hooks/task that requires 12 tasks. **8 tasks yields only 24 — mathematically short.** Rejected: loading 4–5 hooks per task, which produces an obvious obstacle course an examiner reads as engineered. |
| 25 | Task scoping | RATIFIED | T1, T2, T5, T8, T9 scope-capped. T2 and T9 rescoped. |
| 26 | Iteration bounds | RATIFIED | 8–12 items per iterated collection; 8–20 calls per session. |
| 27 | Service spread | RATIFIED | Demoted from a feature to a **task descriptor**. |
| 28 | Answer keys | RATIFIED | Every task needs a written answer key: minimum correct call set, sub-goal breakdown, and which calls are planted to fail. **Six features depend on it.** ~20–30 min per task ≈ **4–6h across 12** — real work that was in no phase estimate. |

### Actor classes

| # | Topic | Status | Decision |
|---|---|---|---|
| 29 | Script class definition | RATIFIED | Defined by **decision time** — when the operator's choices are made — not by tooling. |
| 30 | ★ Script authorship | RATIFIED | Recorded as an **A/B variable**, not collapsed into one method. Level A is human-authored, Level B model-drafted. **Reason: model-drafted alone guarantees convergence** — many people prompting the same model on the same task produce near-identical scripts, collapsing the class's effective sample size toward one per task per model. **Level A is the variance insurance.** The approved taxonomy term "human-authored automation" still fits both levels, since a human specifies and accepts in each. |
| 31 | Human class protocol | RATIFIED | Local and supervised · screened by **procedure-writing** · tasks cross-assigned. |
| 32 | ★ Model prohibition | RATIFIED | Human participants **must not consult a model during a live session** — that is a fourth actor class arriving uninvited. Prohibited, and **recorded as data if it happens.** |
| 33 | ★ Operation parity | RATIFIED | **One operation menu, identical for all three classes, defined as exactly the set of operations the IAM role permits.** This is not a new constraint — it is a written description of the existing one. Solves the problem that a real agent has the whole AWS API while ours would have only the menu, which would artificially suppress exploration breadth and off-task calls (two Family B features). Any attempt to go outside it produces a **logged denial, which is data.** |

### Environment build

| # | Topic | Status | Decision |
|---|---|---|---|
| 34 | Account freeze | RATIFIED | **Account-wide freeze** during collection; global service events enabled on the trail. |
| 35 | Inventory | RATIFIED | ~60 buckets · ~80 SSM parameters · ~60 security groups · ~12 roles · ~8 Lambda functions · ~4 CloudFormation stacks · ~6 DynamoDB tables · ~10 log groups · ~3 SNS topics · plus **three manufactured dead ends.** **All free** — empty buckets, standard parameters, uninvoked functions, empty on-demand tables. No cost exposure. |
| 36 | Build cost | RATIFIED | **~5–7h scripted**, revised up from 3–4h. Phase 0 estimate needs updating. |
| 37 | Resource scoping | RATIFIED | Console-tested, with a **denial → dead-end fallback**. |

### Protocol

| # | Topic | Status | Decision |
|---|---|---|---|
| 38 | Options framing | RATIFIED | When options are presented, state plainly whether they are **alternatives** (pick one) or **layers** (one builds on the other). No shorthand labels. |

---

## PART B — OPEN ITEMS

*A line leaves this list only by becoming a Part A entry in the same commit.*

| Item | Note |
|---|---|
| **Item 3 — target sample size** | **NEXT.** Constraints already fixed: script class counts **distinct procedures, not runs** (a re-run frozen script gives a near-identical log) · authorship A/B splits script sessions, each level needs its own floor · agent class multiplies by model (2–3), and train-on-two/test-on-third sets a per-model minimum · human class is the expensive one and can legitimately be thinner · **group-aware evaluation splits mean usable N is the number of independent groups (procedures, participants, models), not the raw session count.** |
| **Item 4 — IAM role + CloudTrail trail → S3** | Not started. ⚠ **No longer "pure execution."** Feature 1 needs denials to actually occur, and denials must be **planted per task in the policy** — so the order is tasks → policy, not parallel. |
| **Item 5 — null-result report skeleton** | Not started. Written now, a negative result reads as a finding; written in January, it reads as an excuse. |
| **The probe** | ~7h, ~3h net-new. Not started across three handoff periods. Tests the load-bearing timing claim before Phase 3 is committed. Also checks decision 20. |
| **Pilot gate criterion** | ⚠ As written the gate **cannot fail informatively** — "at least one behavioural feature visibly separates" is satisfied by inter-call timing, which is already predicted and which the probe tests. Needs tiering: **PASS-timing** (expected, weak) vs **PASS-agency** (non-timing feature). Only the second should change plans. |
| **Pilot gate date** | Moved from ~Sept 22 to **~Oct 10**, after the Sept 25 – Oct 1 exams. Sept 22 was unreachable: it required Phase 1 (~25h) complete. |
| **Phase 0 hour estimate** | Stale. Add ~5–7h build + ~4–6h answer keys. |
| **Script debug environment** | Where do participant script authors debug, if they cannot touch the account? Debug runs are error-laden and adaptive — i.e. agent-like — and would contaminate the script class. |
| **Taxonomy reframe status** | Did the three-level taxonomy reach the submitted proposal, or did the guide see the original three-way split? Asked repeatedly, never answered. |
| **LLM models + API budget** | The 2–3 models are unnamed and their cost (~$20–40 est.) is unbudgeted. The $5 AWS alarm cannot see this spend. |
| **Multiclass vs pairwise ensemble** | Both cited precedents went multi-class. Decide before Phase 4. |
| **Agent-using-a-script** | Genuine taxonomy hole. May touch the contribution sentence, not just task design. |
| **Review-II date** | None yet. Outranks everything the day it appears. |
