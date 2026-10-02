# IDP build log

## 2026-10-02: Module 3 (A1 vertical slice)
- Built: CloudTrail trail (multi-region, global events, log validation), role ops-audit-role (4 S3 read actions), 8 T1 buckets (4 with policy).
- Ran: 3 blind T1 drafts (GPT, Gemini, Claude), one session each, UUID session names.
- Broke: run 1 void (script files missing, python exit 2, AssumeRole-only sessions). Fix: runner refuses missing files.
- Found: userAgent records retry mode + SDK feature codes (L0 not identical for agent vs script); scripts ~2-3 s between calls on this network (timing claim weaker).
- Learned: <your own words here>
