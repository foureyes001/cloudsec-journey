Drafting prompt v1.1 (2026-10-11)

Change from v1.0: one line only, the region in the scope sentence (C1: study region moved to us-west-2).
  v1.0: Your credentials and region (us-east-1) are already configured; work only in that region.
  v1.1: Your credentials and region (us-west-2) are already configured; work only in that region.
Task texts unchanged (collection form v1.2, hashes as in IDP_frozen_artifacts_v1.0.txt; all 13 re-verified).

Byte format (reproduced from v1.0): UTF-8, ASCII only, LF line endings, no trailing newline.
Full prompt = "TASK" + blank line + collection-form task text + blank line + instruction block.

SHA-256
  instruction block v1.0  8153c25cc4e7d580d89069f92baff405c0f7b218c851d8c7ff24ac077dae4180 (re-verified)
  instruction block v1.1  4eabcdfe413bb0cc98fe15f64eba23eda82412ff01f8e45a1b0561ae2eab7861
  shared scope sentence v1.1 (also for the agent system prompt and human briefing, Change 19)
                          7c86ab05fe2ce56e892faa10238acbb9e364ac39e199549d44105018d3e9ccae
  full prompts            see SHA256SUMS; check with: sha256sum -c SHA256SUMS

Use: copy a prompt FROM ITS FILE, never retype it. Record the file's hash with every draft.

Instruction block v1.1 (exact):
INSTRUCTION

Write a single Python script using boto3 that performs the task above and prints the result.

You have not seen this account and have no list of its resources.
Your credentials and region (us-west-2) are already configured; work only in that region.
The script will be run once, unattended.
You may use any AWS read-only API (Describe*, List*, Get*).

Do not ask clarifying questions. Output only the script.
