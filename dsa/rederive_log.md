# Re-derive Log
| Date | # | Problem | Result | Battery | Notes |
|---|---|---|---|---|---|
| 2026-07-21 | 543 | Diameter of Binary Tree | PASS/FAIL | fixed 4/4 · reuse-demo ✓ · 200k fuzz vs indep oracle · ASan/UBSan clean | cold, solo |
| 2026-07-19 | 981 | Time Based KV Store | FAIL (cold) → guided rebuild PASS | fixed 10/10 · 400×1k-op fuzz · isolation ✓ · SAN clean | BS-family confusion; re-test ≥Jul 27 |
| 2026-07-14 | 150 | Evaluate RPN | PASS | fixed 4/4 · 182k fuzz vs tree oracle · SAN clean | language-fact footnote; RETIRED |