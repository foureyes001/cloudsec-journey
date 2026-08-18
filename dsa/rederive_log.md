# Re-derive Log
| Date | # | Problem | Result | Battery | Notes |
|---|---|---|---|---|---|
| 2026-07-21 | 543 | Diameter of Binary Tree | PASS/FAIL | fixed 4/4 · reuse-demo ✓ · 200k fuzz vs indep oracle · ASan/UBSan clean | cold, solo |
| 2026-07-19 | 981 | Time Based KV Store | FAIL (cold) → guided rebuild PASS | fixed 10/10 · 400×1k-op fuzz · isolation ✓ · SAN clean | BS-family confusion; re-test ≥Jul 27 |
| 2026-07-14 | 150 | Evaluate RPN | PASS | fixed 4/4 · 182k fuzz vs tree oracle · SAN clean | language-fact footnote; RETIRED |

|2026-07-23 | 567 | Permutation in String | ASSISTED (not cold) | fixed 15/15 (post-guard) · 400k fuzz vs sort-window oracle · reuse-demo ✓ · ASan/UBSan ✓ | New structure (prime-then-drop-left); core correct. REGRESSION: dropped n>m guard present in Jul-17 version → heap-buffer-overflow. Lesson: SSO masks OOB from ASan. Re-test cold ≥Jul 28.

| 2026-07-25 | 226 | Invert Binary Tree | COLD PASS/RETIRED | fixed 6/6 · 200k fuzz vs new-tree mirror · 50k involution · skew 10k · SAN | clean solo, zero hints |

2026-07-28 | 981 | Time-Based Key-Value Store | COLD PASS / RETIRED | fixed 20/20 incl. both round-1 killers · 400x1000 interleaved ops (~200k gets) vs linear-scan oracle · instance isolation ✓ · 200k reuse ✓ · SAN clean | Syntax lookup only (iterator→pair access), no algorithmic help — graded cold. Record-and-advance retained; no BS-family leak. Residual: m[key].size() should be it->second.size() (banked operator[] bug, half-applied).

2026-07-31 | #567 | Permutation in String | COLD PASS → RETIRED | fixed 16/16 (2 asst harness errors excluded) · fuzz 400k, 150,954 true-cases, 0 mism vs sort oracle · reuse 200k · OOB probe |s1|=60 |s|=20 (SSO defeated) ASan silent · UBSan clean | Jul-23 guard regression dead. Contract-reliance note: unsigned underflow if |s1|==0 (LeetCode guarantees ≥1).


2026-08-01 | #104 | Maximum Depth of Binary Tree | COLD PASS → RETIRED | fixed 8/8 · fuzz 200k, 0 mism vs BFS oracle · deep-skew 8k · reuse 200k · ASan/UBSan clean | Textbook recursion, not the #543 -1 convention. Rule-25 pattern transfer confirmed at 9 days.

2026-08-01 | #138 | Copy List with Random Pointer | ASSISTED (couldn't complete cold) | v1: two-pass map, dummy-head build, correct on LC ex1 & dup-values (verified below). v2: attempt at unified single-pass, has 3 bugs. | Tier 1. Cold re-attempt ≥ Aug 15.

2026-08-18 | #572 | Subtree of Another Tree | [(d) | ASSISTED — PICK ONE] cold re-attempt VOID (help taken during measurement) | fixed 10/10 ONLY — battery not run, no fuzz/oracle, no SAN available on MinGW | Dual recursion: check() = same-tree compare, isSubtree tests check() at every node then ORs the two children. Structure was correct on arrival; single defect was `if(!root) false;` — missing return, so the statement had no effect and the null path fell through to root->left, dereferencing null. UB/crash, not a wrong answer. Fixed by returning false. Contract: assumes root and subRoot non-null per LC constraints; isSubtree(nullptr,nullptr) returns false by construction — deliberate, not a defect, and untested. Residual: a and b both evaluated before a||b, so recursion never short-circuits — correct, wasteful. Time O(n*m), space O(n) worst. Stays Tier 1. Next cold attempt >= 2026-08-23. Rule 25: shares check() with #100 -> #100 blocked to ~Aug 23 if ratified.
