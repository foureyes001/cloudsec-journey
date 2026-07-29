# Re-derive Log
| Date | # | Problem | Result | Battery | Notes |
|---|---|---|---|---|---|
| 2026-07-21 | 543 | Diameter of Binary Tree | PASS/FAIL | fixed 4/4 · reuse-demo ✓ · 200k fuzz vs indep oracle · ASan/UBSan clean | cold, solo |
| 2026-07-19 | 981 | Time Based KV Store | FAIL (cold) → guided rebuild PASS | fixed 10/10 · 400×1k-op fuzz · isolation ✓ · SAN clean | BS-family confusion; re-test ≥Jul 27 |
| 2026-07-14 | 150 | Evaluate RPN | PASS | fixed 4/4 · 182k fuzz vs tree oracle · SAN clean | language-fact footnote; RETIRED |

|2026-07-23 | 567 | Permutation in String | ASSISTED (not cold) | fixed 15/15 (post-guard) · 400k fuzz vs sort-window oracle · reuse-demo ✓ · ASan/UBSan ✓ | New structure (prime-then-drop-left); core correct. REGRESSION: dropped n>m guard present in Jul-17 version → heap-buffer-overflow. Lesson: SSO masks OOB from ASan. Re-test cold ≥Jul 28.

| 2026-07-25 | 226 | Invert Binary Tree | COLD PASS/RETIRED | fixed 6/6 · 200k fuzz vs new-tree mirror · 50k involution · skew 10k · SAN | clean solo, zero hints |

2026-07-28 | 981 | Time-Based Key-Value Store | COLD PASS / RETIRED | fixed 20/20 incl. both round-1 killers · 400x1000 interleaved ops (~200k gets) vs linear-scan oracle · instance isolation ✓ · 200k reuse ✓ · SAN clean | Syntax lookup only (iterator→pair access), no algorithmic help — graded cold. Record-and-advance retained; no BS-family leak. Residual: m[key].size() should be it->second.size() (banked operator[] bug, half-applied).