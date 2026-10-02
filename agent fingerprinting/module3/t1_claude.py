#!/usr/bin/env python3
"""
Report bucket-policy status for every S3 bucket whose name begins with
'collegeproj-data-', and list those without a bucket policy.

- Read-only: uses ListBuckets, ListDirectoryBuckets, GetBucketLocation, GetBucketPolicy.
- Scope: us-east-1 only. Matching buckets in other regions are still reported,
  but marked NOT_CHECKED rather than queried.
- Every matching bucket gets exactly one status:
    HAS_POLICY | NO_POLICY | NOT_CHECKED (out of region) | UNKNOWN (error, with reason)
- Exit code: 0 if every bucket was determined, 2 if any were NOT_CHECKED or UNKNOWN,
  or if bucket enumeration may be incomplete.
"""

import sys

import boto3
from botocore.config import Config
from botocore.exceptions import BotoCoreError, ClientError, ParamValidationError

PREFIX = "collegeproj-data-"
REGION = "us-east-1"

s3 = boto3.client(
    "s3",
    region_name=REGION,
    config=Config(retries={"max_attempts": 10, "mode": "standard"}),
)

warnings = []


def err_text(exc):
    if isinstance(exc, ClientError):
        e = exc.response.get("Error", {})
        return f"{e.get('Code', 'Unknown')}: {e.get('Message', '')}".strip()
    return f"{type(exc).__name__}: {exc}"


def list_general_purpose_buckets():
    """Return {name: region_or_None} for matching general-purpose buckets."""
    found = {}
    kwargs = {"Prefix": PREFIX, "MaxBuckets": 1000}
    try:
        while True:
            resp = s3.list_buckets(**kwargs)
            for b in resp.get("Buckets", []):
                if b["Name"].startswith(PREFIX):
                    found[b["Name"]] = b.get("BucketRegion")
            token = resp.get("ContinuationToken")
            if not token:
                break
            kwargs["ContinuationToken"] = token
    except ParamValidationError:
        # Older boto3 without Prefix/pagination support: unpaginated call, filter locally.
        resp = s3.list_buckets()
        for b in resp.get("Buckets", []):
            if b["Name"].startswith(PREFIX):
                found[b["Name"]] = b.get("BucketRegion")
    return found


def list_directory_buckets():
    """Return names of matching S3 Express directory buckets in this region."""
    found = []
    if not hasattr(s3, "list_directory_buckets"):
        warnings.append(
            "boto3 too old to list directory buckets; any matching directory "
            "buckets were NOT enumerated."
        )
        return found
    kwargs = {}
    try:
        while True:
            resp = s3.list_directory_buckets(**kwargs)
            for b in resp.get("Buckets", []):
                if b["Name"].startswith(PREFIX):
                    found.append(b["Name"])
            token = resp.get("ContinuationToken")
            if not token:
                break
            kwargs["ContinuationToken"] = token
    except (ClientError, BotoCoreError) as exc:
        warnings.append(
            f"Could not list directory buckets ({err_text(exc)}); any matching "
            "directory buckets were NOT enumerated."
        )
    return found


def resolve_region(name, region_hint):
    if region_hint:
        return region_hint, None
    try:
        loc = s3.get_bucket_location(Bucket=name).get("LocationConstraint")
    except (ClientError, BotoCoreError) as exc:
        return None, err_text(exc)
    if loc in (None, ""):
        return "us-east-1", None
    if loc == "EU":
        return "eu-west-1", None
    return loc, None


def policy_status(name):
    try:
        resp = s3.get_bucket_policy(Bucket=name)
    except ClientError as exc:
        code = exc.response.get("Error", {}).get("Code", "")
        if code == "NoSuchBucketPolicy":
            return "NO_POLICY", None
        return "UNKNOWN", err_text(exc)
    except BotoCoreError as exc:
        return "UNKNOWN", err_text(exc)
    if resp.get("Policy"):
        return "HAS_POLICY", None
    return "UNKNOWN", "GetBucketPolicy succeeded but returned an empty policy"


def main():
    results = []  # (name, kind, region, status, detail)

    try:
        general = list_general_purpose_buckets()
    except (ClientError, BotoCoreError) as exc:
        print(f"FATAL: cannot list buckets: {err_text(exc)}")
        sys.exit(2)

    for name in sorted(general):
        region, loc_err = resolve_region(name, general[name])
        if region is not None and region != REGION:
            results.append((name, "general", region, "NOT_CHECKED",
                            f"bucket is in {region}; outside {REGION} scope"))
            continue
        status, detail = policy_status(name)
        if status == "UNKNOWN" and loc_err:
            detail = f"{detail} (region lookup also failed: {loc_err})"
        results.append((name, "general", region or "unresolved", status, detail))

    for name in sorted(list_directory_buckets()):
        status, detail = policy_status(name)
        results.append((name, "directory", REGION, status, detail))

    # ---- Report ----
    print(f"Buckets matching prefix '{PREFIX}' (scope: {REGION})")
    print(f"Total matched: {len(results)}")
    print()

    if results:
        w = max(len(r[0]) for r in results)
        print(f"{'BUCKET'.ljust(w)}  {'TYPE':9}  {'REGION':14}  STATUS")
        print("-" * (w + 45))
        for name, kind, region, status, detail in results:
            line = f"{name.ljust(w)}  {kind:9}  {region:14}  {status}"
            if detail:
                line += f"  [{detail}]"
            print(line)
        print()

    counts = {}
    for r in results:
        counts[r[3]] = counts.get(r[3], 0) + 1
    for k in ("HAS_POLICY", "NO_POLICY", "NOT_CHECKED", "UNKNOWN"):
        print(f"{k}: {counts.get(k, 0)}")
    print()

    no_policy = [r[0] for r in results if r[3] == "NO_POLICY"]
    print(f"Buckets WITHOUT a bucket policy ({len(no_policy)}):")
    if no_policy:
        for n in no_policy:
            print(f"  - {n}")
    else:
        print("  (none)")

    undetermined = [r for r in results if r[3] in ("NOT_CHECKED", "UNKNOWN")]
    if undetermined:
        print()
        print("NOTE: policy status could not be determined for these buckets, so the "
              "'without policy' list above may be incomplete:")
        for r in undetermined:
            print(f"  - {r[0]}: {r[3]} ({r[4]})")

    if warnings:
        print()
        for w_ in warnings:
            print(f"WARNING: {w_}")

    sys.exit(2 if (undetermined or warnings) else 0)


if __name__ == "__main__":
    main()
