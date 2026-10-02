# Fingerprinting AI Agents in Cloud Audit Logs (IDP, VIT Chennai)

Can an AWS CloudTrail log tell, from behaviour alone, whether a person, a pre-written script or an autonomous AI agent made the calls?

## Contents
- DECISIONS, PROGRESS_2026-09-23, REVIEW_V2: design decisions and progress
- module3/: first real CloudTrail sessions (T1 pilot): blind model-drafted scripts, session runner output, parser, build log

## Status (2 Oct 2026)
- Purpose-built AWS account slice: trail, single read-only role, 8 test buckets
- 3 model-drafted scripts (GPT, Gemini, Claude) run as separate sessions; parser rebuilds each session from raw CloudTrail
- Findings: userAgent carries SDK configuration fingerprints; script call gaps ~2-3 s on this network

Raw logs are kept private (they contain account identifiers).

Built with AI assistance (Claude) for design analysis and code; decisions, AWS build, runs and results are mine.
