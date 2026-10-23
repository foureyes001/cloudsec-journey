#!/usr/bin/env python3
"""
OPEN-14 permission test: can a Deny be aimed at ONE resource for each action we plan to deny?

How it works (nothing in the account changes except a temporary policy, removed at the end):
 1. As admin: add a temporary inline policy 'open14-temp-test' to ops-audit-role:
    Allow the tested actions on "*", plus one Deny per action aimed at ONE planned resource.
 2. As the runner user: assume ops-audit-role (fresh UUID session name).
 3. For each action: call it on an ALLOWED resource (must not be denied) and on the DENIED one (must be denied).
    Scoping works for that action only if both hold.
 4. Route-B and behaviour checks (blocked + open routes stay open; two unverified behaviours recorded).
 5. As admin: remove the temporary policy (always, even after an error) and confirm it is gone.

Run (WSL, venv active, module4 folder):
  python open14_test.py --spec idp_account_spec_v0.3.yaml --runner-profile runner
Results: printed + JSON in --out-dir (default ~/idp/build_out, private).
"""
import argparse
import datetime
import json
import os
import sys
import time
import uuid

import boto3
import yaml
from botocore.config import Config
from botocore.exceptions import ClientError

CFG = Config(retries={"mode": "standard", "max_attempts": 10})
TEMP_POLICY = "open14-temp-test"


def code(e):
    return e.response.get("Error", {}).get("Code", "")


def is_denied(e):
    return "AccessDenied" in code(e) or code(e) in ("UnauthorizedOperation", "AuthorizationError")


class Test:
    def __init__(self, spec, admin, runner_profile, out_dir, sleep=time.sleep):
        self.s, self.sleep = spec, sleep
        self.region, self.sfx = spec["region"], spec["bucket_suffix"]
        self.admin = admin
        self.runner_profile = runner_profile
        self.out_dir = out_dir
        self.acct = admin.client("sts").get_caller_identity()["Account"]
        self.role = spec["principals"]["project_role"]["name"]
        self.role_arn = f"arn:aws:iam::{self.acct}:role/{self.role}"
        self.results = {"actions": [], "route_b": [], "behaviour": [], "notes": []}

    def log(self, msg):
        print(msg, flush=True)

    # ---------- targets: denied = the planned resource, allowed = a sibling ----------
    def targets(self):
        a, r, sfx = self.acct, self.region, self.sfx
        logs = self.admin.client("logs", region_name=r)
        order, kw = [], {}
        while True:
            resp = logs.describe_log_groups(**kw)
            order += [g["logGroupName"] for g in resp["logGroups"]]
            if not resp.get("nextToken"):
                break
            kw["nextToken"] = resp["nextToken"]
        sub = self.s["logs"]["subscription_filters"]["on_positions"]
        import re
        den_pos = int(re.search(r"position (\d+)", self.s["logs"]["planned_denial"]).group(1))
        allow_pos = [p for p in sub if p != den_pos][0]
        g_den, g_ok = order[den_pos - 1], order[allow_pos - 1]
        fn = lambda n: f"arn:aws:lambda:{r}:{a}:function:{n}"
        b = lambda n: f"arn:aws:s3:::{n}"
        role = lambda n: f"arn:aws:iam::{a}:role/{n}"
        # (id, IAM action, deny ARNs, service, method, kwargs for allowed, kwargs for denied, planned use)
        return [
            ("T1", "s3:GetBucketPolicy", [b(f"collegeproj-data-features-{sfx}")], "s3", "get_bucket_policy",
             {"Bucket": f"collegeproj-data-curated-{sfx}"}, {"Bucket": f"collegeproj-data-features-{sfx}"}, "planned"),
            ("T2", "iam:GetRolePolicy", [role("collegeproj-svc-notify")], "iam", "get_role_policy",
             {"RoleName": "collegeproj-svc-catalog", "PolicyName": "s3-catalog-read"},
             {"RoleName": "collegeproj-svc-notify", "PolicyName": "notify-topic-publish"}, "planned"),
            ("T3", "logs:DescribeSubscriptionFilters",
             [f"arn:aws:logs:{r}:{a}:log-group:{g_den}", f"arn:aws:logs:{r}:{a}:log-group:{g_den}:*"],
             "logs", "describe_subscription_filters", {"logGroupName": g_ok}, {"logGroupName": g_den}, "planned"),
            ("T6a", "lambda:ListTags", [fn("collegeproj-fn-ledger"), fn("collegeproj-fn-ledger") + ":*"], "lambda",
             "list_tags", {"Resource": fn("collegeproj-fn-cleanup")}, {"Resource": fn("collegeproj-fn-ledger")}, "planned"),
            ("T6b", "lambda:GetFunction", [fn("collegeproj-fn-ledger"), fn("collegeproj-fn-ledger") + ":*"], "lambda",
             "get_function", {"FunctionName": "collegeproj-fn-cleanup"}, {"FunctionName": "collegeproj-fn-ledger"}, "planned"),
            ("T8", "ssm:ListTagsForResource", [f"arn:aws:ssm:{r}:{a}:parameter/collegeproj/prod/db-host"], "ssm",
             "list_tags_for_resource", {"ResourceType": "Parameter", "ResourceId": "/collegeproj/prod/api-key"},
             {"ResourceType": "Parameter", "ResourceId": "/collegeproj/prod/db-host"}, "planned"),
            ("T10", "iam:ListAttachedRolePolicies", [role("collegeproj-lambda-exec-b")], "iam",
             "list_attached_role_policies", {"RoleName": "collegeproj-lambda-exec-a"},
             {"RoleName": "collegeproj-lambda-exec-b"}, "planned"),
            ("T11", "dynamodb:DescribeTable", [f"arn:aws:dynamodb:{r}:{a}:table/collegeproj-profiles"], "dynamodb",
             "describe_table", {"TableName": "collegeproj-carts"}, {"TableName": "collegeproj-profiles"}, "planned"),
            ("T11/T12", "cloudformation:ListStackResources",
             [f"arn:aws:cloudformation:{r}:{a}:stack/collegeproj-stack-core/*"], "cloudformation",
             "list_stack_resources", {"StackName": "collegeproj-stack-web"}, {"StackName": "collegeproj-stack-core"}, "planned"),
            ("T4 reserve", "s3:GetBucketTagging", [b(f"collegeproj-archive-invoices-{sfx}")], "s3", "get_bucket_tagging",
             {"Bucket": f"collegeproj-media-archive-{sfx}"}, {"Bucket": f"collegeproj-archive-invoices-{sfx}"}, "reserve"),
            ("T9 reserve", "s3:GetBucketPublicAccessBlock", [b(f"collegeproj-app-frontend-{sfx}")], "s3",
             "get_public_access_block", {"Bucket": f"collegeproj-app-assets-{sfx}"},
             {"Bucket": f"collegeproj-app-frontend-{sfx}"}, "reserve"),
            ("T5 reserve", "ecr:GetRepositoryPolicy", [f"arn:aws:ecr:{r}:{a}:repository/collegeproj-batch"], "ecr",
             "get_repository_policy", {"repositoryName": "collegeproj-web"}, {"repositoryName": "collegeproj-batch"}, "reserve"),
            ("T7 reserve", "lambda:GetFunctionConcurrency", [fn("collegeproj-fn-invoices"), fn("collegeproj-fn-invoices") + ":*"],
             "lambda", "get_function_concurrency", {"FunctionName": "collegeproj-fn-orders"},
             {"FunctionName": "collegeproj-fn-invoices"}, "reserve"),
        ]

    def policy(self, tg):
        allow = sorted({t[1] for t in tg} | {"s3:GetBucketPolicyStatus", "cloudformation:DescribeStackResources",
                                             "lambda:ListFunctions"})
        st = [{"Sid": "TempAllowTested", "Effect": "Allow", "Action": allow, "Resource": "*"}]
        for i, t in enumerate(tg):
            st.append({"Sid": f"TempDeny{i:02d}", "Effect": "Deny", "Action": t[1], "Resource": t[2]})
        return {"Version": "2012-10-17", "Statement": st}

    # ---------- runner session ----------
    def runner_session(self):
        base = boto3.Session(profile_name=self.runner_profile, region_name=self.region)
        sts = base.client("sts")
        who = sts.get_caller_identity()["Arn"]
        if not who.endswith("user/" + self.s["principals"]["runner_user"]["name"]):
            raise SystemExit(f"runner profile is {who}, not the runner user; stopping")
        for i in range(12):          # a new access key / trust change can take seconds to work
            try:
                c = sts.assume_role(RoleArn=self.role_arn, RoleSessionName=str(uuid.uuid4()),
                                    DurationSeconds=3600)["Credentials"]
                break
            except ClientError as e:
                if i == 11:
                    raise
                self.log(f"  assume-role not ready yet ({code(e)}), retrying in 5s")
                self.sleep(5)
        return boto3.Session(aws_access_key_id=c["AccessKeyId"], aws_secret_access_key=c["SecretAccessKey"],
                             aws_session_token=c["SessionToken"], region_name=self.region)

    def call(self, sess, service, method, kw):
        try:
            getattr(sess.client(service, region_name=self.region, config=CFG), method)(**kw)
            return "ok", ""
        except ClientError as e:
            return ("denied" if is_denied(e) else "error"), code(e)

    # ---------- the test ----------
    def run(self, settle=30):
        tg = self.targets()
        iam = self.admin.client("iam")
        if TEMP_POLICY in iam.list_role_policies(RoleName=self.role)["PolicyNames"]:
            self.results["notes"].append("a leftover temp policy was found and replaced")
        self.log(f"[1] adding temporary policy {TEMP_POLICY} to {self.role} ({len(tg)} denies)")
        iam.put_role_policy(RoleName=self.role, PolicyName=TEMP_POLICY, PolicyDocument=json.dumps(self.policy(tg)))
        try:
            self.log(f"    waiting {settle}s for IAM to apply it")
            self.sleep(settle)
            self.log("[2] runner assumes the role")
            sess = self.runner_session()
            self.log("[3] per-action checks: allowed resource must work, planned resource must be denied")
            pending = []
            for t in tg:
                a_res, a_code = self.call(sess, t[3], t[4], t[5])
                d_res, d_code = self.call(sess, t[3], t[4], t[6])
                row = {"id": t[0], "action": t[1], "use": t[7], "allowed_side": f"{a_res} {a_code}".strip(),
                       "denied_side": f"{d_res} {d_code}".strip()}
                row["scoping_works"] = a_res != "denied" and d_res == "denied"
                self.results["actions"].append(row)
                if not row["scoping_works"]:
                    pending.append((row, t))
            if pending:   # rule out slow IAM propagation before calling anything a failure
                self.log(f"    {len(pending)} not as expected; waiting 45s and re-checking those once")
                self.sleep(45)
                for row, t in pending:
                    a_res, a_code = self.call(sess, t[3], t[4], t[5])
                    d_res, d_code = self.call(sess, t[3], t[4], t[6])
                    row.update(allowed_side=f"{a_res} {a_code}".strip(), denied_side=f"{d_res} {d_code}".strip(),
                               scoping_works=a_res != "denied" and d_res == "denied", rechecked=True)
            sfx = self.sfx
            self.log("[4] route B (must stay open while route A is denied) and behaviour checks")
            for name, svc, meth, kw in [
                ("T1 route B: GetBucketPolicyStatus on the denied bucket", "s3", "get_bucket_policy_status",
                 {"Bucket": f"collegeproj-data-features-{sfx}"}),
                ("T11/T12 route B: DescribeStackResources by stack name", "cloudformation", "describe_stack_resources",
                 {"StackName": "collegeproj-stack-core"}),
                ("T6: ListFunctions still lists the denied function", "lambda", "list_functions", {}),
                ("T7 reserve route B: GetFunction on fn-invoices", "lambda", "get_function",
                 {"FunctionName": "collegeproj-fn-invoices"}),
            ]:
                res, c = self.call(sess, svc, meth, kw)
                self.results["route_b"].append({"check": name, "result": f"{res} {c}".strip(), "open": res == "ok"})
            for name, svc, meth, kw in [
                ("GetBucketPolicyStatus on a NO-policy bucket (v4 s5 unverified)", "s3", "get_bucket_policy_status",
                 {"Bucket": f"collegeproj-data-analytics-{sfx}"}),
                ("DescribeStackResources with a table that is in NO stack (T11 route B)", "cloudformation",
                 "describe_stack_resources", {"PhysicalResourceId": "collegeproj-carts"}),
            ]:
                res, c = self.call(sess, svc, meth, kw)
                self.results["behaviour"].append({"check": name, "result": f"{res} {c}".strip()})
        finally:
            self.log(f"[5] removing {TEMP_POLICY}")
            iam.delete_role_policy(RoleName=self.role, PolicyName=TEMP_POLICY)
            left = iam.list_role_policies(RoleName=self.role)["PolicyNames"]
            self.results["temp_policy_removed"] = TEMP_POLICY not in left
            self.results["role_inline_policies_after"] = left
        return self.report()

    def report(self):
        self.log("\n=== OPEN-14 results ===")
        for r in self.results["actions"]:
            self.log(f"  {'WORKS' if r['scoping_works'] else 'FAILS'}  {r['id']:<10} {r['action']:<34} "
                     f"allowed: {r['allowed_side']:<28} denied: {r['denied_side']}")
        for r in self.results["route_b"]:
            self.log(f"  {'OPEN ' if r['open'] else 'SHUT '}  {r['check']}  ({r['result']})")
        for r in self.results["behaviour"]:
            self.log(f"  RECORDED  {r['check']}: {r['result']}")
        self.log(f"  temp policy removed: {self.results['temp_policy_removed']} "
                 f"(role now has: {self.results['role_inline_policies_after']})")
        planned_fail = [r["id"] for r in self.results["actions"] if r["use"] == "planned" and not r["scoping_works"]]
        self.log(f"  planned denials that FAIL: {planned_fail or 'none'}")
        os.makedirs(self.out_dir, exist_ok=True)
        stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        path = os.path.join(self.out_dir, f"open14_{stamp}.json")
        with open(path, "w", encoding="utf-8") as fh:
            json.dump(self.results, fh, indent=2)
        self.log(f"  saved {path} (private)")
        ok = self.results["temp_policy_removed"] and all(r["open"] for r in self.results["route_b"])
        return 0 if ok else 2


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--spec", required=True)
    ap.add_argument("--runner-profile", default="runner")
    ap.add_argument("--admin-profile", default=None, help="default: your normal admin credentials")
    ap.add_argument("--out-dir", default=os.path.expanduser("~/idp/build_out"))
    a = ap.parse_args()
    with open(a.spec, encoding="utf-8") as fh:
        spec = yaml.safe_load(fh)
    admin = boto3.Session(profile_name=a.admin_profile, region_name=spec["region"])
    who = admin.client("sts").get_caller_identity()["Arn"]
    if not who.endswith("user/" + spec["principals"]["build_identity"].split()[0]):
        sys.exit(f"admin credentials are {who}, expected the build user; stopping")
    sys.exit(Test(spec, admin, a.runner_profile, a.out_dir).run())


if __name__ == "__main__":
    main()
