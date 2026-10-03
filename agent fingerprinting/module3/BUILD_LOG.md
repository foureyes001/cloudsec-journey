# IDP build log

## 2026-10-02: Module 3 (A1 vertical slice)
- Built: CloudTrail trail (multi-region, global events, log validation), role ops-audit-role (4 S3 read actions), 8 T1 buckets (4 with policy).
- Ran: 3 blind T1 drafts (GPT, Gemini, Claude), one session each, UUID session names.
- Broke: run 1 void (script files missing, python exit 2, AssumeRole-only sessions). Fix: runner refuses missing files.
- Found: userAgent records retry mode + SDK feature codes (L0 not identical for agent vs script); scripts ~2-3 s between calls on this network (timing claim weaker).
- Learned: <your own words here>

## 2026-10-03
- Changed: study region us-east-1 -> us-west-2 (us-east-1 holds an unrelated older project). Deleted the 8 pilot buckets; pilot data kept in logs and commit 9517b43.
- Found: new account had Lambda concurrency limit 10, which blocks reserved concurrency (needed for T7). Requested 1000; us-west-2 now 1000.
- Broke: WSL had no outbound network while Windows was fine. Fixed by restarting WSL (wsl --shutdown). Project work moving to WSL Ubuntu as a non-root user.
- Decided: all collegeproj- roles on ListRoles page 2; permission-scoping test after the build; T1 re-run dropped (drafts hardcode us-east-1); build spec in YAML.
- Learned: (your words, 2 to 3 lines)