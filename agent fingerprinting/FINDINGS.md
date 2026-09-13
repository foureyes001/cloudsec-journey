# FINDINGS — analysis log

**Project:** Fingerprinting AI Agents in Cloud Audit Logs
**Scope of this file:** analysis output that is not yet a decision. Companion to `DECISIONS.md`, which holds ratified decisions (Part A) and open items (Part B).
**Opened:** 2026-09-13. Covers the analysis session of 2026-09-12.

> **`[A]` NOTE.** This file was assembled by the assistant from the `part 2` chat transcript of 2026-09-12. Nothing here is invented; every entry traces to that session. But the *judgement* in the PROPOSED entries is assistant-originated and none of it has been accepted by me. Status labels are load-bearing — read them before acting on any line.

**Status vocabulary**

| Label | Meaning |
|---|---|
| `RESOLVED` | Settled in session. Eligible to be promoted to a `DECISIONS.md` Part A entry. |
| `PROPOSED` | Assistant recommendation. **Not accepted.** Requires an explicit yes/no from me. |
| `OPEN` | Identified, unowned, no resolution attempted. |
| `CONFLICT` | Contradicts the Day-102 handoff. Must be reconciled before either document is trusted. |

---

## 1. RESOLVED — eligible for promotion to `DECISIONS.md` Part A

| # | Finding | Consequence |
|---|---|---|
| F1 | **Agent-that-uses-a-script is excluded by construction** — the agent is given no code-execution tool. | Not a fourth actor class. Becomes one limitation line in the report. |
| F2 | **Script authoring is blind.** Participants author procedures against a written spec; no debug runs against the live account. | Removes the contamination where trial-and-error script development produces error-laden, adaptive, agent-like sessions. |
| F3 | **Three candidate features struck as non-discriminating:** read/write ratio, region spread, service spread. | They go in the null-result report as *tested non-discriminators*, which is a result, not a deletion. |
| F4 | **Sentinel-call question provisionally closed** by taking a fresh role assumption per session. | Provisional only — still requires empirical verification. See C1: if the probe is struck, that verification has no current owner. |
| F5 | **Console sessions must never be used as a human-class data point.** Whoever runs them knows the answer keys, so the session takes the optimal path. | Console is a spoofability-ladder demonstration only (trivially separable at L0, not at L1). Disclose the contamination in one line rather than engineering around it. |
| F6 | **Repeated runs of a deterministic script are not independent samples.** Effective N for the script class is the number of *distinct script authors*, not the number of runs. | Sample-size arithmetic must count authors. The "× repetitions" multiplier is real for the agent and human classes and ≈1 for scripts. **Directly gates Phase 0 item "target sample size," the next work item.** |
| F7 | **Denials must be planted per task in the IAM policy.** A role granting only `Describe*`/`List*`/`Get*` denies nothing, and Feature 1 (post-denial behaviour) requires denials to actually occur. | Ordering is tasks → policy, strictly serial. The IAM-role item stops being independent execution work and can no longer be scheduled in parallel. |

## 2. PROPOSED — assistant recommendation, NOT accepted

| # | Proposal | Argument | Cost of accepting |
|---|---|---|---|
| P1 | **Strike the probe as a separate work item; the pilot absorbs it.** | Phase 1 builds the agent and the scripts anyway, and the pilot sessions measure the same timestamps the probe was built to measure. The probe was always scoped as overlapping Phase 0/1 by ~4h. | Saves ~3h net-new. **Loses the early warning** — which was the probe's entire purpose. ⚠ F4's verification (CLI credential caching and session-lifetime behaviour) must be explicitly re-homed into the pilot or it is silently dropped. |
| P2 | **Run a reduced pilot (2 tasks) instead of the full pilot (4 tasks).** | Full pilot = 8 of 8 hooks but requires most of the ~5–7h account build before the gate. Reduced pilot = 5 of 8 hooks and needs only the IAM roles and Lambda functions. | Reduced keeps the denial and blocked-plus-open hooks, where the headline post-error-response and recovery-to-outcome features live. Loses the manufactured dead end, the large-listing hook and two services. |
| P3 | **Tier the pilot gate into PASS-timing and PASS-agency.** | As currently written ("at least one behavioural feature visibly separates"), inter-call timing satisfies the gate — but timing separation is already predicted at ~90%. A PASS therefore tells you nothing you did not already believe. | Only a non-timing (agency) feature should be allowed to change the plan. Without this the gate cannot fail informatively. |

## 3. OPEN — identified, unowned

| # | Item | Note |
|---|---|---|
| O1 | **LLM API spend has no budget line.** | ~$90–120 all-in, real out-of-pocket. The existing $5 alarm is AWS-only and cannot see it. Not a decision — a line item with no owner. |
| O2 | **Weekly session quota.** | Data collection is the phase most exposed to the documented failure pattern (continuous internal counters get missed; dated artifact-producing deadlines get hit). No quota exists anywhere in the plan. |
| O3 | **Systematic literature survey before Review-II.** | Estimated ~4–6h. Appears in no capacity table. |
| O4 | **Completion criteria + operation menu.** | Estimated ~1.5h. Appears in no capacity table. |
| O5 | **Script-session definition: which execution counts?** | "A session is one task execution" does not say *which* execution of a script is the sample. Undefined here is how the script class gets contaminated. Partially addressed by F2; not fully closed. |

## 4. CONFLICT — must be reconciled before either document is trusted

| # | Conflict | Detail |
|---|---|---|
| C1 | **The probe.** | The Day-102 handoff (Sept 13) calls the probe "the highest-leverage single action in the entire project," records it as unstarted for a third period, and builds its central diagnosis on that fact. The Sept 12 analysis recommends **striking it**. The handoff appears not to have ingested P1. **Until this is settled, the handoff's diagnosis is resting on an item that may not exist.** |
| C2 | **Phase 0 item numbering, third variant.** | Day-102 handoff: item 4 = IAM role, item 5 = null-result skeleton. The Sept 12 session: item 4 = null-result skeleton. `IDP part 1`: item 3 = IAM role. Three numberings are live. **Renumber once, canonically, in `DECISIONS.md`, and reference items by name thereafter.** |
| C3 | **Hook count: 8 or 10.** | The 12-task arithmetic uses 3 core + 7 secondary = 10 hooks (36 placements). The Sept 12 pilot analysis works against **8** hooks throughout ("all 8 hooks", "5 of 8"). One of these is stale. The 12-task result depends on which. |
| C4 | **Phase 0 total effort.** | Sept 12 revision: ~10h → **~21–26h**. The Day-102 capacity table carries ~19h for the same scope and omits O3 and O4 entirely. The real shortfall is larger than the handoff states. |
| C5 | **Internal to the Sept 12 session:** the forced-ordering chain still contains the probe as a step (`…→ trail → S3 → probe → Phase 1 → pilot gate`) while §6 of the same document recommends deleting it. | The chain has not been rewritten for P1. Whichever way C1 resolves, the chain needs one edit. |

## 5. BLOCKER

- **A first-pass review document (items D1–D7, E1–E10, F1–F4) was referenced but never supplied to the analysis.** Roughly 18 of those ~21 items are unaddressed because they were never seen. "They still hold" is my assertion, not a check.
- **Consequence: target sample size — the next work item — should not be computed until that list is on the table**, because any one of the unseen items could contain a conflict of the kind already found in this session.

---

## Forced ordering (current best understanding)

```
task definitions [done]
  → completion criteria + operation menu
    → IAM denial policy + answer keys        (needs resource naming patterns)
      → account build
        → resource-scoping console test
          → CloudTrail trail → S3            (global service events ON)
            → [probe — status contested, see C1]
              → Phase 1
                → pilot gate
```

---

## Next action

**Supply the first-pass list (§5), then compute target sample size using F6's author-counted N.** Everything downstream of sample size is blocked on it.
