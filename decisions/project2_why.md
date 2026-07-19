# Project #2 Decision — boto3 Security Scanner

Date: 2026-07-19 (Day 46)

## Decision

Project #2 = **(a) boto3 security scanner**.

## What it is

This project's aim is to create a security tool that runs scheduled checks on my AWS
account and detects four common **misconfigurations** by scanning it. The four checks:

1. Security groups open to the internet (0.0.0.0/0)
2. CloudTrail multi-region trail enabled or not
3. Root MFA enabled or not
4. Access keys older than 90 days or not

## Why this over the alternatives

The synergy of this project aligns the most with my future goals — the cloud-security
path. The project itself is a security tool, so the repo does the signaling on its own.

- **(b) EC2 + ALB + ASG:** already within the scope of what I will build hands-on in
  Maarek's SAA course labs, and the ALB has no free tier — it costs real money while
  adding little new signal for my path.
- **(c) CI/CD static site:** too small to stand as project #2, and it largely duplicates
  project #1. Instead, it gets folded into project #1 as a GitHub Actions enhancement.

## What it teaches me

It teaches me how to create serverless workflows using EventBridge, which triggers a
Lambda that runs the boto3 script, with notification of any relevant findings through
SNS. It also forces me to write a least-privilege IAM execution role for the scanner
itself (only the specific Describe/Get/List actions it needs) — applying the IAM
material from the SAA course to a real artifact.

## Scope lock

- **Core = the four checks above. Checklist FROZEN.**
- **Done (Month 3) means:** deployed · daily cron firing · email report arriving with a
  real finding from one deliberately misconfigured test resource · README with a sample
  report and architecture diagram · clean commit history.
- **Stretch (gated behind core-done, at most one):**
  1. One auto-remediation action (auto-revoke 0.0.0.0/0 on port 22 + "found & fixed" notice)
  2. Prowler comparison paragraph in the README

## Risks

- **Scope creep** — the main risk. Mitigation: frozen checklist; stretch modules gated.
- **Cost** — low. All services are free-tier at this scale; a cost budget alarm is live
  on the account.