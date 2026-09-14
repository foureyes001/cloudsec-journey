# REVIEW — v2 handoff, adversarial pass

**Session:** 2026-09-14. Adversarial review of `IDP_CHAT4_HANDOFF.md` v2, the same treatment v1 received.
**Work started:** none. No §25 item begun, no ruling taken.
**Purpose of the pass:** catch defects before they are baked into the account, since hooks cannot be fixed after collection.

> **`[A]`** Assistant-written. Findings 8 and 16 are explicitly flagged in the source as unverified recollection and must be checked before use.

---

## Verified and holding

Budget sums (120–128h) · schedule arithmetic (8 days / ~5.7h / ~8×) · T1 and T4 cap windows · session counts (117 agent / 43 human) · hook table (36 placements) · the 1 : 2.7 class ratio. Two copies of v2 were present (project file + paste); identical on read, no v1 contamination.

## A. Needs a ruling or a probe check

| # | Finding |
|---|---|
| **1 ★★** | **Dead-end degeneracy is not T1-only.** By §10's own criterion — an absence error that *is* the answer is not an obstacle — T4 (no lifecycle/tags → "none" is the answer), T9 (no public-access-block → "not blocked" is the answer), T12 ("whether it still exists" → the 404 is the answer) and T7 (the task text announces the deleted function) all fail it. **The hook drops from 5 placements to 0–1, not to 4.** Two readings, pick one: **(a)** the criterion is right → redesign the hook across the account as an error the actor did *not* ask about; **(b)** the criterion is too strong — reading an error *as* an answer is itself post-error response — in which case T1 isn't in trouble either. §11's count is stale under both. |
| **2 ★** | **§9 contradicts §17 and N6 on `userIdentity`.** §9's L1 strips "any principal-identifying substring of `userIdentity`"; §17 and N6 say it is never stripped and a bad session name survives L0–L3. One is wrong. Opaque naming is correct regardless, because L0 must be clean. |
| **3 ★** | **N5's allow list manufactures agent-only denials.** Menu = union of draft calls, so scripts cannot leave it by construction and humans hold it; only the agent explores off-list. Every off-list call becomes an unplanned `AccessDenied` in Family A and in error rate **for one class only** — the contamination N5 prevents for scripts, relocated to agents. Options: **(a)** parser tags each denial *planted / shortcut / unplanned* and Family A reports on planted only; **(b)** broad read-only allow list minus shortcuts, which makes the human menu enormous. **Decide before the IAM policy is written.** |
| **4** | **Model pool composition is never named, and the vendor-line hypothesis constrains it.** If two of three are Anthropic models and the hypothesis holds, the held-out model's scripts are near-copies of a training model's and the unseen-model fold leaks. **"1 Bedrock + 2 external" should read "3 vendors"** unless the recon falsifies it. |
| **5 ★** | **"Recon drafts double as collection data" has two unstated preconditions.** (a) The drafting prompt must be frozen *before* the recon — §25 calls it frozen, §21 lists it unwritten. (b) Task text must not change *after* — but the floor check, OPEN-3 and finding 1 all expect task changes. Reuse is partial. **Corrected order: completion sentences → freeze prompt → recon.** |
| **6** | **"1 drafting per model" rests on N=1** — one model, one re-prompt, one task, and the "rewrite" may not have been an independent sample. Cheap test: re-draft 2–3 (model, task) pairs cold during the recon. **Recon output #8.** |
| **7 ★** | **The build order puts OPEN-14 after the account is built.** If ≥2 of the 3 uncertain denials fail to scope, placements get redistributed and tasks change. OPEN-14 needs one scratch resource and 15 min — **it belongs before the answer keys.** OPEN-13 is absent from the chain entirely yet feeds OPEN-4, which closes at build. |
| **8** | **The ladder's field list is silent on two real CloudTrail fields:** `resources` (resource ARNs → partial parameter re-admission at L2) and `additionalEventData` (S3 events carry `SignatureVersion`, `CipherSuite`, `AuthenticationMethod`, `bytesTransferredOut` — a client fingerprint at L1 and a response-size signal the doc says CloudTrail lacks). **From memory, unverified.** The probe should dump one full event per class and assign every field a rung. |
| **9** | **The human menu is script-derived.** Humans see `GetBucketLocation` only because two models drafted it — a convergence pressure on human-vs-script the parity argument does not cover. Cheap fix: alphabetical by service, no grouping, derivation disclosed. |
| **10** | **Contribution (c) says "trivially spoofable," which is L1 exactly** — yet §3 now says (c) is delivered by L2/L3 for the central pair, and neither is trivially spoofable. If the sentence is locked with the title, the write-up needs an explicit bridge; if not, reword now. |

## B. Consistency and arithmetic

| # | Finding |
|---|---|
| 11 | Part A open-item count: §0 and §22 say five, the table has six (OPEN-19), §25 says six. |
| 12 | **"Human is the minority class" holds only if M ≥ 4.** At M=3: 117 / 39 / 43 makes *script* the minority; training totals 108 / 36 / 36 tie. Depends on OPEN-11. |
| 13 | §21 uses the full revised Phase 0 estimate (21–26h) as the *remainder* — implying either zero hours spent on closed items 1, 2, 2b, or that the estimate was remaining-only. Which? |
| 14 | **Pilot composition unspecified.** "12 sessions" against 4 tasks × 3 models × 2 classes = 24. |
| 15 | **Filing-rule edge:** post-error pause and post-error latency ratio exist only when an error occurred, so their missingness carries error-presence — non-timing — information. Ablating F+G therefore removes more than timing. Impute, or encode presence separately. |
| 16 | **OPEN-13, unverified recollection:** EC2 `DescribeSecurityGroups` returns all items when `MaxResults` is unset; SSM `DescribeParameters` pages at 10. **If both hold, large listing is 1 placement (T8) and T5 has ~1 live hook.** Two doc pages settle it. |
| 17 | §25 calls large listing "fully visible from drafts." Drafts show whether a script *paginates*; whether a listing *is large* is OPEN-13 plus inventory. **Half visible.** |

## C. Cross-reference drift — §0–§18 cite an older section layout

| Written | Appears in | Should be |
|---|---|---|
| §10 (ladder, L1) | §0, §3 | §9 |
| §9 (feature filing) | §0 | §8 |
| §11 (completion sentence) | §0, §6 | §10 |
| §14 (N4) | §0 | §13 |
| §16 (script class, drafting prompt, acceptance) | §4, §5 | §15 |
| §17 (botocore version, N7) | §9 | §13 |
| §18 (session naming / definition) | §0, §7, §9 | §17 |
| §19 (evaluation, sample size, timing, fallback) | §5, §7, §12, §14, §16 | §18 |
| §20 (drafting test, defensiveness, within-model variance) | §8, §15, §18 | §19 |
| §24 (intra-second ordering) | §8, §18 | §20 |

§21–§25 already cite the current layout.

## D. Second pass — §22 Part D onward

- **OPEN-25 and OPEN-12 are unconnected, and connecting them is free.** §18 says dated external milestones get hit and internal counters get missed. **Review-II is the only dated external milestone before January.** Make its deliverable *"N sessions collected + gate result"* and collection becomes the kind of thing that gets hit. Cheapest available treatment of the missing weekly quota — needs only the date.
- **OPEN-27 is uncounted, not double-counted.** The systematic survey (4–6h) appears nowhere in §21's 120–128h. Either add it, or accept that *"to our knowledge, the first"* stays hedged.
- **Spend: prepaid $100 sits below the $120 upper estimate.** The $30 collection figure depends on two unknowns — which models (finding 4) and OPEN-1. **At a 40-step cap each step re-sends the growing transcript, so cost per session rises faster than linearly with steps — possibly 2–3× on a frontier model.** Cannot be sized until the pool is named.
- ★ **Public repo + "commit raw logs continuously" is a disclosure problem.** Raw CloudTrail events carry the account ID, and every `AssumeRole` event carries the ARN of whoever assumed the role. **Private data repo, or sanitise before commit — stated nowhere.**
- **Whether unused credits survive the Paid upgrade is a docs check.** Small, but it is $160.
- ★ **The recon estimate (~3–4h) is understated roughly 2×.** 13 tasks × 5–6 models = 65–78 scripts. Prompting ≈2.5h; reading each to extract the operation union, calls-per-item, route choice and error handling at 3–5 min ≈3.5–6.5h. **Total 6–9h** — the same understatement pattern the document warns about for its unrevised phases.
- **Planted denial is partially visible from drafts, not invisible.** Drafts show whether the action to be denied (`GetRolePolicy`, `ListTagsForResource`, `DescribeSubscriptionFilters`, `ListTags`) is on the script path at all. **A denial on an action no script calls fires for no script.** Add to recon output #2.
- **The "then, in order" chain has no slot for the null-result skeleton** (Phase 0 item 4), which §5 calls primary insurance.
- OPEN-31 arithmetic holds (52–55h of ~60h; October's project load ≈5h/week, consistent with §21).

## E. Agreed order — three rulings before any writing

Layers, not alternatives. Findings 1, 7 and 16 each threaten to change task text *after* the recon, so they settle first: ~30 min of decisions, zero build hours.

| Order | Step | Cost |
|---|---|---|
| 1 | **Rule finding 1** — is an answer-bearing absence error a hook? Five of twelve tasks hang on it | ~10 min, your call |
| 2 | **Fetch the two doc pages for OPEN-13** — decides whether T5 and T8 have a live large-listing hook; feeds OPEN-3 and OPEN-4 | ~5 min |
| 3 | **Rule OPEN-3** with 1 and 2 in hand. Task text becomes final | your call |
| 4 | §25 as written, plus one insert: completion sentences → **freeze the drafting prompt** → recon | ~1h + ~1h + 3–4h |

**Proposed middle path on finding 1, offered as a lens not a ruling:** T7 and T12 already use the *surprise* kind — a referenced resource that is gone. Keep both, delete the announcing sentence from T7, and reword T12 so "still exists" is not the question. T1, T4 and T9 keep their absence errors but are reclassified as a weaker hook variant and disclosed. **Dead end then sits at 2 strong + 3 weak — honest, and ~20 min of rewording rather than a redesign.**

Findings 2, 3, 8, 9 and the cross-references can wait for a v2.1 patch; none of them change task text.

## F. Status

**Nothing ruled.** All 18 findings open. Two Part-D facts would let the rest be sized: **the Review-II date and the participant count.** Only OPEN-24 (exam cutoff) carries a deadline this week.
