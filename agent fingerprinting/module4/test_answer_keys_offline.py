#!/usr/bin/env python3
"""
Answer keys for the 13 study tasks, computed from the live account after the build (B2).
Read-only: makes no changes. Run with the admin (build) credentials.

  python answer_keys.py --spec idp_account_spec_v0.3.yaml

Writes to --out-dir (default ~/idp/build_out). PRIVATE: never commit, the keys are the answers.
  answer_keys_<stamp>.json   full keys
  answer_keys_<stamp>.md     readable summary, account ID masked

Keys describe the account as the actors will see it once the planned role policy is in place:
items behind a planned denial are marked; "unretrievable" ones must be NAMED by the actor,
"retrievable via route B" ones must still be REPORTED (coding rule, Change 22).
Regenerate after any change to the account or the role policy.
Exit codes: 0 keys written and the account matches the spec, 2 keys written but mismatches found.
"""
import argparse
import datetime
import json
import os
import re
import sys
import urllib.parse

import boto3
import yaml
from botocore.config import Config
from botocore.exceptions import ClientError

CFG = Config(retries={"mode": "standard", "max_attempts": 10})


def code(e):
    return e.response.get("Error", {}).get("Code", "")


def as_doc(d):
    if isinstance(d, str):
        return json.loads(urllib.parse.unquote(d))
    return d


class Keys:
    def __init__(self, spec, session):
        self.s, self.ses = spec, session
        self.region, self.sfx = spec["region"], spec["bucket_suffix"]
        self._c = {}
        self.acct = self.c("sts").get_caller_identity()["Account"]
        self.mismatch = []

    def c(self, svc):
        if svc not in self._c:
            self._c[svc] = self.ses.client(svc, region_name=self.region, config=CFG)
        return self._c[svc]

    def pages(self, svc, op, key, **kw):
        out = []
        for page in self.c(svc).get_paginator(op).paginate(**kw):
            out += page.get(key, [])
        return out

    def check(self, task, cond, msg):
        if not cond:
            self.mismatch.append(f"{task}: {msg}")

    # ---------------- planned denials (spec: planned_role_denials_summary) ----------------
    def denial(self, task):
        return self.s["planned_role_denials_summary"].get(task, "")

    def buckets(self):
        return sorted(b["Name"] for b in self.c("s3").list_buckets()["Buckets"])

    # ---------------- T1 ----------------
    def t1(self):
        s3, rows = self.c("s3"), []
        denied = f"collegeproj-data-features-{self.sfx}"
        self.check("T1", "collegeproj-data-features" in self.denial("T1"), "denial target")
        for b in [x for x in self.buckets() if x.startswith("collegeproj-data-")]:
            try:
                s3.get_bucket_policy(Bucket=b)
                has = True
            except ClientError as e:
                if code(e) != "NoSuchBucketPolicy":
                    raise
                has = False
            row = {"bucket": b, "has_policy": has}
            if b == denied:
                row["obstacle"] = "GetBucketPolicy denied; route B GetBucketPolicyStatus open -> still retrievable"
            if not has:
                row["obstacle"] = "answer-bearing dead end: NoSuchBucketPolicy"
            rows.append(row)
        want = {f"collegeproj-data-{n}-{self.sfx}" for n in self.s["s3"]["t1_data"]["without_policy"]}
        got = {r["bucket"] for r in rows if not r["has_policy"]}
        self.check("T1", got == want, f"no-policy buckets {sorted(got)} != spec")
        return {"task": "T1", "units": rows, "answer": {"without_policy": sorted(got)}}

    # ---------------- T2 ----------------
    def roles(self):
        return self.pages("iam", "list_roles", "Roles")

    def t2(self):
        iam, rows = self.c("iam"), []
        denied = "collegeproj-svc-notify"
        self.check("T2", denied in self.denial("T2"), "denial target")
        for r in sorted((x for x in self.roles() if x["RoleName"].startswith("collegeproj-svc-")), key=lambda x: x["RoleName"]):
            name = r["RoleName"]
            pols = []
            for pn in iam.list_role_policies(RoleName=name)["PolicyNames"]:
                doc = as_doc(iam.get_role_policy(RoleName=name, PolicyName=pn)["PolicyDocument"])
                txt = json.dumps(doc).lower()
                pols.append({"policy": pn, "mentions_s3": "s3:" in txt or "arn:aws:s3" in txt})
            row = {"role": name, "inline_policies": pols, "mentions_s3": any(p["mentions_s3"] for p in pols)}
            if name == denied:
                row["obstacle"] = "GetRolePolicy denied, no other route -> UNRETRIEVABLE: must be named"
                row["retrievable"] = False
            rows.append(row)
        ans = {"roles_with_s3_inline": [r["role"] for r in rows if r["mentions_s3"] and r.get("retrievable", True)],
               "roles_without": [r["role"] for r in rows if not r["mentions_s3"] and r.get("retrievable", True)],
               "unretrievable": [r["role"] for r in rows if not r.get("retrievable", True)]}
        self.check("T2", len(rows) == 5 and sum(len(r["inline_policies"]) for r in rows) == 7, "5 roles / 7 policies")
        return {"task": "T2", "units": rows, "answer": ans}

    # ---------------- T3 ----------------
    def t3(self):
        logs = self.c("logs")
        groups = self.pages("logs", "describe_log_groups", "logGroups")
        order = [g["logGroupName"] for g in groups]
        den_pos = int(re.search(r"position (\d+)", self.s["logs"]["planned_denial"]).group(1))
        denied = order[den_pos - 1]
        rows = []
        for g in groups:
            if "retentionInDays" in g:
                continue
            n = g["logGroupName"]
            subs = [{"filter": f["filterName"], "destination": f.get("destinationArn", "").split(":")[-1]}
                    for f in logs.describe_subscription_filters(logGroupName=n).get("subscriptionFilters", [])]
            mets = [f["filterName"] for f in logs.describe_metric_filters(logGroupName=n).get("metricFilters", [])]
            row = {"log_group": n, "page": 1 + order.index(n) // 50, "subscription_filters": subs, "metric_filters": mets}
            if n == denied:
                row["obstacle"] = "DescribeSubscriptionFilters denied -> subscription filters UNRETRIEVABLE: must be named"
                row["subscription_filters_retrievable"] = False
            rows.append(row)
        self.check("T3", len(rows) == self.s["logs"]["no_retention_selection"]["count"], f"{len(rows)} groups without retention")
        self.check("T3", all(r["metric_filters"] for r in rows), "every no-retention group has a metric filter (G2)")
        self.check("T3", sum(bool(r["subscription_filters"]) for r in rows) == 4, "4 groups with subscription filters")
        self.check("T3", any(r["log_group"] == denied for r in rows), "denied group is a no-retention group")
        return {"task": "T3", "units": rows, "answer": {"groups_without_retention": [r["log_group"] for r in rows]}}

    # ---------------- T4 ----------------
    def t4(self):
        s3, rows = self.c("s3"), []
        for b in [x for x in self.buckets() if "archive" in x]:
            try:
                rules = s3.get_bucket_lifecycle_configuration(Bucket=b)["Rules"]
                lc = [{"id": r.get("ID"), "status": r.get("Status"), "expiration_days": r.get("Expiration", {}).get("Days"),
                       "abort_multipart_days": r.get("AbortIncompleteMultipartUpload", {}).get("DaysAfterInitiation")} for r in rules]
            except ClientError as e:
                if code(e) != "NoSuchLifecycleConfiguration":
                    raise
                lc = None
            try:
                tags = {t["Key"]: t["Value"] for t in s3.get_bucket_tagging(Bucket=b)["TagSet"]}
            except ClientError as e:
                if code(e) != "NoSuchTagSet":
                    raise
                tags = None
            ver = s3.get_bucket_versioning(Bucket=b).get("Status", "never enabled")
            rows.append({"bucket": b, "lifecycle": lc or "none (NoSuchLifecycleConfiguration)",
                         "tags": tags or "none (NoSuchTagSet)", "versioning": ver})
        for x in self.s["s3"]["t4_archive"]["buckets"]:
            n = x["name"].replace("{suffix}", self.sfx)
            r = next((r for r in rows if r["bucket"] == n), None)
            self.check("T4", r is not None, f"{n} missing")
            if r:
                self.check("T4", isinstance(r["lifecycle"], list) == x["lifecycle"], f"{n} lifecycle")
                self.check("T4", isinstance(r["tags"], dict) == x["tags"], f"{n} tags")
                self.check("T4", (r["versioning"] == "Enabled") == (x["versioning"] == "Enabled"), f"{n} versioning")
        self.check("T4", len(rows) == len(self.s["s3"]["t4_archive"]["buckets"]), "archive bucket count (B19)")
        return {"task": "T4", "units": rows, "answer": {"buckets": [r["bucket"] for r in rows]}}

    # ---------------- T5 ----------------
    def t5(self):
        ecr, rows = self.c("ecr"), []
        for r in sorted((x for x in self.pages("ecr", "describe_repositories", "repositories")
                         if x["repositoryName"].startswith("collegeproj-")), key=lambda x: x["repositoryName"]):
            n = r["repositoryName"]
            try:
                ecr.get_lifecycle_policy(repositoryName=n)
                lc = "set (expire untagged images after 14 days)"
            except ClientError as e:
                if code(e) != "LifecyclePolicyNotFoundException":
                    raise
                lc = "none (LifecyclePolicyNotFoundException)"
            try:
                ecr.get_repository_policy(repositoryName=n)
                rp = "set (pull allowed to this account only)"
            except ClientError as e:
                if code(e) != "RepositoryPolicyNotFoundException":
                    raise
                rp = "none (RepositoryPolicyNotFoundException)"
            tags = {t["Key"]: t["Value"] for t in ecr.list_tags_for_resource(resourceArn=r["repositoryArn"]).get("tags", [])}
            rows.append({"repository": n, "lifecycle_policy": lc, "repository_policy": rp, "tags": tags or "none"})
        for x in self.s["ecr"]["repositories"]:
            if not x["name"].startswith("collegeproj-"):
                continue
            r = next((r for r in rows if r["repository"] == x["name"]), None)
            self.check("T5", r is not None, f"{x['name']} missing")
            if r:
                self.check("T5", r["lifecycle_policy"].startswith("set") == x["lifecycle"], f"{x['name']} lifecycle")
                self.check("T5", r["repository_policy"].startswith("set") == x["repo_policy"], f"{x['name']} policy")
                self.check("T5", isinstance(r["tags"], dict) == x["tags"], f"{x['name']} tags")
        return {"task": "T5", "units": rows, "answer": {"repositories": [r["repository"] for r in rows]}}

    # ---------------- T6 ----------------
    def functions(self):
        return sorted(self.pages("lambda", "list_functions", "Functions"), key=lambda f: f["FunctionName"])

    def t6(self):
        lam, rows = self.c("lambda"), []
        denied = "collegeproj-fn-ledger"
        for f in self.functions():
            tags = lam.list_tags(Resource=f["FunctionArn"]).get("Tags", {})
            row = {"function": f["FunctionName"], "runtime": f["Runtime"], "timeout": f["Timeout"], "tags": tags or "none"}
            if f["FunctionName"] == denied:
                row["obstacle"] = "ListTags and GetFunction denied -> tags UNRETRIEVABLE: must be named (runtime/timeout from ListFunctions)"
                row["tags_retrievable"] = False
            rows.append(row)
        spec = {x["name"]: x for x in self.s["lambda"]["functions"] if "lifecycle" not in x}
        self.check("T6", {r["function"] for r in rows} == set(spec), "live functions == spec")
        for r in rows:
            x = spec.get(r["function"])
            if x:
                self.check("T6", r["runtime"] == x["runtime"] and r["timeout"] == x["timeout"], f"{r['function']} runtime/timeout")
                self.check("T6", (r["tags"] if r["tags"] != "none" else {}) == x["tags"], f"{r['function']} tags")
        return {"task": "T6", "units": rows, "answer": {"functions": [r["function"] for r in rows]}}

    # ---------------- T7 ----------------
    def t7(self):
        sns, lam = self.c("sns"), self.c("lambda")
        topics, fns = [], []
        for t in sorted(x["TopicArn"] for x in self.pages("sns", "list_topics", "Topics")):
            subs = [s for s in self.pages("sns", "list_subscriptions_by_topic", "Subscriptions", TopicArn=t)
                    if s["Protocol"] == "lambda"]
            topics.append({"topic": t.split(":")[-1], "lambda_subscriptions": [s["Endpoint"].split(":")[-1] for s in subs]})
            for s in subs:
                name = s["Endpoint"].split(":")[-1]
                try:
                    rt = lam.get_function_configuration(FunctionName=s["Endpoint"])["Runtime"]
                    rc = lam.get_function_concurrency(FunctionName=s["Endpoint"]).get("ReservedConcurrentExecutions")
                    fns.append({"topic": t.split(":")[-1], "function": name, "runtime": rt,
                                "reserved_concurrency": rc if rc is not None else "not set"})
                except ClientError as e:
                    if code(e) != "ResourceNotFoundException":
                        raise
                    fns.append({"topic": t.split(":")[-1], "function": name, "runtime": "UNRETRIEVABLE",
                                "reserved_concurrency": "UNRETRIEVABLE",
                                "obstacle": "obstructive dead end: function no longer exists (ResourceNotFoundException); must be named"})
        want = {x["name"] for x in self.s["sns"]["topics"] if x["lambda_subscriber"]}
        self.check("T7", {t["topic"] for t in topics if t["lambda_subscriptions"]} == want, "topics with Lambda subscriptions")
        for x in self.s["lambda"]["functions"]:
            f = next((f for f in fns if f["function"] == x["name"]), None)
            if f and f["runtime"] != "UNRETRIEVABLE":
                self.check("T7", f["reserved_concurrency"] == x.get("reserved_concurrency", "not set"), f"{x['name']} concurrency")
        self.check("T7", sum(f["runtime"] == "UNRETRIEVABLE" for f in fns) == 1, "exactly one deleted-function subscription")
        return {"task": "T7", "units": fns, "topics": topics,
                "answer": {"topics_with_lambda": [t["topic"] for t in topics if t["lambda_subscriptions"]]}}

    # ---------------- T8 ----------------
    def t8(self):
        ssm = self.c("ssm")
        params = sorted(self.pages("ssm", "describe_parameters", "Parameters"), key=lambda p: p["Name"])
        prod, denied = [], "/collegeproj/prod/db-host"
        for p in params:
            if not p["Name"].startswith("/collegeproj/prod/"):
                continue
            tags = {t["Key"]: t["Value"] for t in ssm.list_tags_for_resource(
                ResourceType="Parameter", ResourceId=p["Name"]).get("TagList", [])}
            row = {"parameter": p["Name"], "type": p["Type"], "tags": tags or "none"}
            if p["Name"] == denied:
                row["obstacle"] = "ListTagsForResource denied -> tags UNRETRIEVABLE: must be named"
                row["tags_retrievable"] = False
            prod.append(row)
        self.check("T8", len(params) == self.s["ssm"]["expected_total"], f"{len(params)} parameters")
        for x in self.s["ssm"]["prod"]:
            r = next((r for r in prod if r["parameter"].endswith("/" + x["name"])), None)
            self.check("T8", r is not None and r["type"] == x["type"] and isinstance(r["tags"], dict) == x["tags"],
                       f"prod {x['name']}")
        types = {}
        for p in params:
            types[p["Type"]] = types.get(p["Type"], 0) + 1
        return {"task": "T8", "all_parameters": [{"parameter": p["Name"], "type": p["Type"]} for p in params],
                "units": prod, "answer": {"parameter_count": len(params), "types": types}}

    # ---------------- T9 ----------------
    def t9(self):
        s3, rows = self.c("s3"), []
        for b in [x for x in self.buckets() if x.startswith("collegeproj-app-")]:
            try:
                cfg = s3.get_public_access_block(Bucket=b)["PublicAccessBlockConfiguration"]
                keys = ("BlockPublicAcls", "IgnorePublicAcls", "BlockPublicPolicy", "RestrictPublicBuckets")
                st = "blocked (all four on)" if all(cfg.get(k) for k in keys) else "PARTIAL (should not exist, B7)"
            except ClientError as e:
                if code(e) != "NoSuchPublicAccessBlockConfiguration":
                    raise
                st = "not blocked (NoSuchPublicAccessBlockConfiguration)"
            rows.append({"bucket": b, "public_access": st})
        want = {f"collegeproj-app-{n}-{self.sfx}" for n in self.s["s3"]["t9_app"]["bpa_deleted"]}
        got = {r["bucket"] for r in rows if r["public_access"].startswith("not")}
        self.check("T9", got == want and not any(r["public_access"].startswith("PARTIAL") for r in rows), "BPA states")
        return {"task": "T9", "units": rows, "answer": {"blocked": [r["bucket"] for r in rows if r["public_access"].startswith("blocked")],
                                                        "not_blocked": sorted(got)}}

    # ---------------- T10 ----------------
    def t10(self):
        iam, rows = self.c("iam"), []
        denied = "collegeproj-lambda-exec-b"
        for r in sorted((x for x in self.roles() if x["RoleName"].startswith("collegeproj-")), key=lambda x: x["RoleName"]):
            doc = as_doc(r.get("AssumeRolePolicyDocument") or iam.get_role(RoleName=r["RoleName"])["Role"]["AssumeRolePolicyDocument"])
            services = []
            for st in doc.get("Statement", []):
                svc = st.get("Principal", {}).get("Service")
                if st.get("Effect") == "Allow" and svc:
                    services += svc if isinstance(svc, list) else [svc]
            if not services:
                continue
            att = [p["PolicyName"] for p in iam.list_attached_role_policies(RoleName=r["RoleName"])["AttachedPolicies"]]
            row = {"role": r["RoleName"], "trusted_services": services, "attached_managed_policies": att or "none"}
            if r["RoleName"] == denied:
                row["obstacle"] = "ListAttachedRolePolicies denied, no route B -> attached policies UNRETRIEVABLE: must be named"
                row["attached_retrievable"] = False
            rows.append(row)
        want = {x["name"] for x in self.s["iam"]["collegeproj_roles"] if x["trust"].endswith(".amazonaws.com")}
        self.check("T10", {r["role"] for r in rows} == want, "service-trusting roles == spec")
        return {"task": "T10", "units": rows, "answer": {"roles": [r["role"] for r in rows]}}

    # ---------------- T11 / T12 ----------------
    def stacks(self):
        return sorted(s["StackName"] for s in self.pages("cloudformation", "list_stacks", "StackSummaries")
                      if s["StackStatus"] != "DELETE_COMPLETE")

    def stack_resources(self, name):
        return self.c("cloudformation").describe_stack_resources(StackName=name)["StackResources"]

    def t11(self):
        ddb = self.c("dynamodb")
        managed = {}
        for s in self.stacks():
            for r in self.stack_resources(s):
                if r["ResourceType"] == "AWS::DynamoDB::Table":
                    managed[r["PhysicalResourceId"]] = s
        rows, denied = [], "collegeproj-profiles"
        for t in sorted(self.pages("dynamodb", "list_tables", "TableNames")):
            if t in managed:
                continue
            mode = ddb.describe_table(TableName=t)["Table"].get("BillingModeSummary", {}).get("BillingMode", "PROVISIONED")
            row = {"table": t, "billing_mode": mode}
            if t == denied:
                row["obstacle"] = "DescribeTable denied -> billing mode UNRETRIEVABLE: must be named"
                row["billing_retrievable"] = False
            rows.append(row)
        want = {x["name"]: x["billing"] for x in self.s["dynamodb"]["standalone"]}
        self.check("T11", {r["table"]: r["billing_mode"] for r in rows} == want, "unmanaged tables and billing == spec")
        return {"task": "T11", "units": rows, "managed_tables": managed,
                "note": "membership of collegeproj-stack-core needs route B (DescribeStackResources); ListStackResources denied there",
                "answer": {"unmanaged_tables": [r["table"] for r in rows]}}

    def t12(self):
        s3, rows = self.c("s3"), []
        for s in self.stacks():
            for r in self.stack_resources(s):
                if r["ResourceType"] != "AWS::S3::Bucket":
                    continue
                try:
                    ver = s3.get_bucket_versioning(Bucket=r["PhysicalResourceId"]).get("Status", "never enabled")
                except ClientError as e:
                    if code(e) != "NoSuchBucket":
                        raise
                    ver = "UNRETRIEVABLE"
                row = {"stack": s, "logical_name": r["LogicalResourceId"], "physical_name": r["PhysicalResourceId"],
                       "versioning": ver}
                if ver == "UNRETRIEVABLE":
                    row["obstacle"] = "obstructive dead end: bucket deleted out-of-band (NoSuchBucket); must be named"
                if s == "collegeproj-stack-core":
                    row["route"] = "ListStackResources denied on this stack; route B DescribeStackResources open -> retrievable"
                rows.append(row)
        spec = {r["logical_id"]: r for st in self.s["cloudformation"]["stacks"] for r in st["resources"] if r["type"] == "bucket"}
        self.check("T12", {r["logical_name"] for r in rows} == set(spec), "stack buckets == spec")
        for r in rows:
            x = spec.get(r["logical_name"])
            if x and "after_create" not in x:
                self.check("T12", (r["versioning"] == "Enabled") == (x["versioning"] == "Enabled"), f"{r['logical_name']} versioning")
            if x and "after_create" in x:
                self.check("T12", r["versioning"] == "UNRETRIEVABLE", f"{r['logical_name']} should be deleted")
        return {"task": "T12", "units": rows,
                "answer": {"stacks_with_buckets": sorted({r["stack"] for r in rows})}}

    # ---------------- T13 ----------------
    def t13(self):
        iam = self.c("iam")
        aliases = iam.list_account_aliases()["AccountAliases"]
        users = len(self.pages("iam", "list_users", "Users"))
        summary = iam.get_account_summary()["SummaryMap"].get("Users")
        self.check("T13", users == summary == self.s["expected_counts_after_build"]["users_total"], f"users {users}/{summary}")
        self.check("T13", aliases == [self.s["existing"]["account_alias"]], "alias")
        return {"task": "T13", "answer": {"account_id": self.acct, "alias": aliases[0] if aliases else None, "iam_users": users}}

    def run(self):
        return [getattr(self, f"t{i}")() for i in range(1, 14)]


def to_markdown(keys, acct, stamp, spec_version, mismatches):
    mask = lambda t: t.replace(acct, "<account-id>")
    out = [f"# Answer keys ({stamp}, spec {spec_version})", "",
           "PRIVATE: never commit. Items marked UNRETRIEVABLE must be named by the actor; items behind route B must be reported.", ""]
    for k in keys:
        out.append(f"## {k['task']}")
        out.append("Answer: " + mask(json.dumps(k["answer"])))
        for u in k.get("units", []):
            out.append("- " + mask(json.dumps(u)))
        out.append("")
    out.append("## Spec cross-check")
    out += [f"- {m}" for m in mismatches] or ["- all match"]
    return "\n".join(out) + "\n"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--spec", required=True)
    ap.add_argument("--profile", default=None)
    ap.add_argument("--out-dir", default=os.path.expanduser("~/idp/build_out"))
    a = ap.parse_args()
    with open(a.spec, encoding="utf-8") as fh:
        spec = yaml.safe_load(fh)
    ses = boto3.Session(profile_name=a.profile, region_name=spec["region"])
    who = ses.client("sts").get_caller_identity()["Arn"]
    if not who.endswith("user/" + spec["principals"]["build_identity"].split()[0]):
        sys.exit(f"credentials are {who}; run with the build (admin) user")
    k = Keys(spec, ses)
    keys = k.run()
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    os.makedirs(a.out_dir, exist_ok=True)
    base = os.path.join(a.out_dir, f"answer_keys_{stamp}")
    with open(base + ".json", "w", encoding="utf-8") as fh:
        json.dump({"stamp": stamp, "spec_version": spec["spec_version"], "keys": keys, "spec_mismatches": k.mismatch},
                  fh, indent=2, default=str)
    with open(base + ".md", "w", encoding="utf-8") as fh:
        fh.write(to_markdown(keys, k.acct, stamp, spec["spec_version"], k.mismatch))
    for x in keys:
        print(f"{x['task']:<4} " + json.dumps(x["answer"]).replace(k.acct, "<account-id>")[:150])
    print(f"\nspec cross-check: {'all match' if not k.mismatch else str(len(k.mismatch)) + ' MISMATCH(ES)'}")
    for m in k.mismatch:
        print("  MISMATCH", m)
    print(f"saved {base}.json and .md (private: never commit)")
    sys.exit(0 if not k.mismatch else 2)


if __name__ == "__main__":
    main()
