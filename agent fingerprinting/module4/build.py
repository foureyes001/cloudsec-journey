#!/usr/bin/env python3
"""
IDP account build for the study region (us-west-2), driven by the spec file.
Code written by Claude (AI); spec decisions, review and runs by the user.

Usage (WSL, venv active, run from the folder that holds the spec):
  python build.py --spec idp_account_spec_v0.3.yaml --dry-run          # read-only: preflight + plan
  python build.py --spec idp_account_spec_v0.3.yaml --apply --stop-after 2
  python build.py --spec idp_account_spec_v0.3.yaml --apply            # full build (safe to re-run)
  python build.py --spec idp_account_spec_v0.3.yaml --verify-only      # step 13 + 14 again

Build log and inventory (contain the account ID) go to --out-dir, default ~/idp/build_out.
Never put that folder in git.
Exit codes: 0 ok, 1 stopped (preflight or a STOP rule or an unexpected AWS error), 2 verify found problems.
"""
import argparse
import datetime
import io
import json
import os
import re
import sys
import time
import zipfile

import boto3
import yaml
from botocore.config import Config
from botocore.exceptions import ClientError, WaiterError

CFG = Config(retries={"mode": "standard", "max_attempts": 10})   # backoff on throttling (IAM especially)
LAST_STEP = 14


class Stop(Exception):
    """A rule in the spec says: stop here and decide by hand."""


def err_code(e):
    return e.response.get("Error", {}).get("Code", "")


def err_msg(e):
    return e.response.get("Error", {}).get("Message", "")


class Build:
    def __init__(self, spec, apply, out_dir):
        self.s = spec
        self.apply = apply
        self.region = spec["region"]
        self.sfx = spec["bucket_suffix"]
        self.build_user = spec["principals"]["build_identity"].split()[0]
        self.acct = None
        self._clients = {}
        self.sleep = time.sleep                       # replaced by the offline test
        self.rec = {"recorded": {}, "orders": {}, "page_sizes": {}, "verify": [], "positions": {}}
        os.makedirs(out_dir, exist_ok=True)
        self.out_dir = out_dir
        self.stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        self.logf = open(os.path.join(out_dir, f"build_{self.stamp}.log"), "a", encoding="utf-8")

    # ---------------- plumbing ----------------
    def log(self, msg):
        line = f"{datetime.datetime.now(datetime.timezone.utc).strftime('%H:%M:%S')} {msg}"
        print(line, flush=True)
        self.logf.write(line + "\n")
        self.logf.flush()

    def c(self, service, region=None):
        key = (service, region or self.region)
        if key not in self._clients:
            self._clients[key] = boto3.client(service, region_name=key[1], config=CFG)
        return self._clients[key]

    def fmt(self, text):
        return text.replace("{suffix}", self.sfx).replace("{account_id}", self.acct or "ACCOUNT")

    def retry(self, fn, ok_to_retry, what, max_wait=150):
        """Retry while AWS is still propagating something we just created (IAM roles, permissions)."""
        delay, waited = 3, 0
        while True:
            try:
                return fn()
            except ClientError as e:
                if not ok_to_retry(e) or waited >= max_wait:
                    raise
                self.log(f"    waiting for propagation ({what}): {err_code(e)}; retry in {delay}s")
                self.sleep(delay)
                waited += delay
                delay = min(delay * 2, 20)

    def would(self, msg):
        self.log(f"    {'DO' if self.apply else 'WOULD'}: {msg}")
        return self.apply

    @staticmethod
    def pages(call, items_key, name_fn, token_in, token_out, **kw):
        """Walk every page with default page size; keep the page boundaries (B16 needs them)."""
        out = []
        while True:
            r = call(**kw)
            out.append([name_fn(x) for x in r.get(items_key, [])])
            token = r.get(token_out)
            if not token:
                return out
            kw[token_in] = token

    # ---------------- names from the spec ----------------
    def t1(self):
        return [f"collegeproj-data-{n}-{self.sfx}" for n in self.s["s3"]["t1_data"]["names"]]

    def t9(self):
        return [f"collegeproj-app-{n}-{self.sfx}" for n in self.s["s3"]["t9_app"]["names"]]

    def t4(self):
        return [(self.fmt(b["name"]), b) for b in self.s["s3"]["t4_archive"]["buckets"]]

    def filler_buckets(self):
        return [f"collegeproj-{n}-{self.sfx}" for n in self.s["s3"]["fillers"]["names"]]

    def planned_buckets(self):
        return self.t1() + self.t9() + [n for n, _ in self.t4()] + self.filler_buckets()

    def filler_roles(self):
        fr = self.s["iam"]["filler_roles"]
        names = [f"{fam}-{i:02d}-role" for fam, n in fr["families"].items() for i in range(1, n + 1)]
        assert len(names) == fr["total"], "filler count does not match spec total"
        return names

    def cp_roles(self):
        return self.s["iam"]["collegeproj_roles"]

    def runner(self):
        return self.s["principals"]["runner_user"]["name"]

    def functions(self):
        return self.s["lambda"]["functions"]

    def fn_arn(self, name):
        return f"arn:aws:lambda:{self.region}:{self.acct}:function:{name}"

    def export_fn(self):
        return [f["name"] for f in self.functions() if "lifecycle" in f][0]

    def lambda_groups(self):
        return ["/aws/lambda/" + f["name"] for f in self.functions()]

    def other_groups(self):
        og = self.s["logs"]["other_groups"]
        return [f"/collegeproj/{fam}/{fam}-{i:02d}" for fam, n in og["families"].items() for i in range(1, n + 1)]

    def ssm_params(self):
        """(name, type, value, tags or None) in creation order (alphabetical by family)."""
        sp = self.s["ssm"]
        values = {"api-key": "placeholder-not-a-secret", "api-url": "https://api.example.internal",
                  "cache-ttl": "300", "db-host": "db.example.internal", "db-password": "placeholder-not-a-secret",
                  "db-port": "5432", "feature-flags": "checkout,search,reviews", "log-level": "INFO",
                  "queue-name": "orders-queue", "region-list": "us-west-2"}
        out, n = [], 0
        for fam in sp["create_order"]:
            if fam == "/collegeproj/prod":
                for p in sp["prod"]:
                    out.append((f"{fam}/{p['name']}", p["type"], values[p["name"]],
                                self.s["tags"]["ssm"] if p["tags"] else None))
                continue
            for i in range(1, sp["others"]["families"][fam] + 1):
                n += 1
                ptype = "SecureString" if n % 10 == 0 else ("StringList" if n % 5 == 0 else "String")
                value = "a,b,c" if ptype == "StringList" else f"placeholder-{n:02d}"
                out.append((f"{fam}/param-{i:02d}", ptype, value, self.s["tags"]["ssm"] if n % 4 == 0 else None))
        return out

    def stacks(self):
        return {st["name"]: st for st in self.s["cloudformation"]["stacks"]}

    def repos(self):
        return self.s["ecr"]["repositories"]

    @staticmethod
    def tagset(d):
        return [{"Key": k, "Value": v} for k, v in d.items()]

    # ---------------- live listings ----------------
    def live_buckets(self):
        return self.pages(self.c("s3").list_buckets, "Buckets", lambda b: b["Name"],
                          "ContinuationToken", "ContinuationToken")

    def live_roles(self):
        return self.pages(self.c("iam").list_roles, "Roles", lambda r: r["RoleName"], "Marker", "Marker")

    def live_users(self):
        return sum(self.pages(self.c("iam").list_users, "Users", lambda u: u["UserName"], "Marker", "Marker"), [])

    def live_functions(self):
        return self.pages(self.c("lambda").list_functions, "Functions", lambda f: f["FunctionName"],
                          "Marker", "NextMarker")

    def live_tables(self):
        return self.pages(self.c("dynamodb").list_tables, "TableNames", lambda t: t,
                          "ExclusiveStartTableName", "LastEvaluatedTableName")

    def live_groups(self):
        return self.pages(self.c("logs").describe_log_groups, "logGroups", lambda g: g["logGroupName"],
                          "nextToken", "nextToken")

    def live_group_details(self):
        return sum(self.pages(self.c("logs").describe_log_groups, "logGroups", lambda g: g,
                              "nextToken", "nextToken"), [])

    def live_topics(self):
        return self.pages(self.c("sns").list_topics, "Topics", lambda t: t["TopicArn"], "NextToken", "NextToken")

    def live_params(self):
        return self.pages(self.c("ssm").describe_parameters, "Parameters", lambda p: p["Name"],
                          "NextToken", "NextToken")

    def live_stacks(self):
        pages = self.pages(self.c("cloudformation").list_stacks, "StackSummaries",
                           lambda s: (s["StackName"], s["StackStatus"]), "NextToken", "NextToken")
        return [[n for n, st in p if st != "DELETE_COMPLETE"] for p in pages]

    def live_repos(self):
        return self.pages(self.c("ecr").describe_repositories, "repositories", lambda r: r["repositoryName"],
                          "nextToken", "nextToken")

    @staticmethod
    def flat(pages):
        return [x for p in pages for x in p]

    def topic_arn(self, name):
        return f"arn:aws:sns:{self.region}:{self.acct}:{name}"

    def fn_exists(self, name):
        try:
            self.c("lambda").get_function_configuration(FunctionName=name)
            return True
        except ClientError as e:
            if err_code(e) == "ResourceNotFoundException":
                return False
            raise

    def export_dead_end_done(self):
        """The T7 dead end exists: export function gone, its subscription still on the topic."""
        fn = self.export_fn()
        if self.fn_exists(fn):
            return False
        topic = [t["name"] for t in self.s["sns"]["topics"] if t["lambda_subscriber"] == fn][0]
        try:
            subs = self.c("sns").list_subscriptions_by_topic(TopicArn=self.topic_arn(topic))["Subscriptions"]
        except ClientError as e:
            if err_code(e) == "NotFound":
                return False
            raise
        return any(s["Endpoint"] == self.fn_arn(fn) for s in subs)

    # ================= STEP 0: PREFLIGHT =================
    def step0_preflight(self):
        ex = self.s["existing"]
        results = []

        def check(name, ok, detail=""):
            results.append((name, ok, detail))

        arn = self.c("sts").get_caller_identity()["Arn"]
        self.acct = arn.split(":")[4]
        check("identity is the build user", arn.endswith(f":user/{self.build_user}"), arn.split(":")[-1])
        check("spec region", self.region == "us-west-2", self.region)

        tr = ex["trail"]
        ct = self.c("cloudtrail", tr["home_region"])
        trails = ct.describe_trails(trailNameList=[tr["name"]])["trailList"]
        t = trails[0] if trails else {}
        logging_on = bool(trails) and ct.get_trail_status(Name=tr["name"]).get("IsLogging") is True
        check("trail exists, multi-region, global events, validation, logging",
              bool(t) and t.get("IsMultiRegionTrail") and t.get("IncludeGlobalServiceEvents")
              and t.get("LogFileValidationEnabled") and logging_on, tr["name"])

        def compare(name, live, existing, planned, planned_prefixes=()):
            live, existing = set(live), set(existing)
            missing = existing - live
            unknown = {x for x in live - existing - set(planned) if not x.startswith(tuple(planned_prefixes))}
            check(name, not missing and not unknown,
                  f"missing={sorted(missing)} unknown={sorted(unknown)}" if (missing or unknown) else "")

        compare("buckets", self.flat(self.live_buckets()), [self.fmt(b) for b in ex["buckets"]],
                self.planned_buckets(), ("collegeproj-stack-",))
        compare("roles", self.flat(self.live_roles()), ex["roles"],
                self.filler_roles() + [r["name"] for r in self.cp_roles()])
        compare("users", self.live_users(), ex["users"], [self.runner()])
        aliases = self.c("iam").list_account_aliases()["AccountAliases"]
        check("account alias", aliases == [ex["account_alias"]], str(aliases))

        # us-west-2 holds nothing except what this spec builds (re-runs allowed)
        compare("lambda functions", self.flat(self.live_functions()), [], [f["name"] for f in self.functions()])
        compare("dynamodb tables", self.flat(self.live_tables()), [],
                [t["name"] for t in self.s["dynamodb"]["standalone"]], ("collegeproj-stack-",))
        compare("log groups", self.flat(self.live_groups()), [], self.lambda_groups() + self.other_groups())
        compare("sns topics", [a.split(":")[-1] for a in self.flat(self.live_topics())], [],
                [t["name"] for t in self.s["sns"]["topics"]])
        compare("ssm parameters", self.flat(self.live_params()), [], [p[0] for p in self.ssm_params()])
        compare("cloudformation stacks", self.flat(self.live_stacks()), [], list(self.stacks()))
        compare("ecr repositories", self.flat(self.live_repos()), [], [r["name"] for r in self.repos()])

        lim = self.c("lambda").get_account_settings()["AccountLimit"]["ConcurrentExecutions"]
        check("lambda concurrency limit", lim == ex["lambda_concurrency_limit_us_west_2"], str(lim))

        try:
            self.c("s3control").get_public_access_block(AccountId=self.acct)
            check("account-level S3 BPA unconfigured (C15)", False, "a configuration EXISTS")
        except ClientError as e:
            check("account-level S3 BPA unconfigured (C15)",
                  err_code(e) == "NoSuchPublicAccessBlockConfiguration", err_code(e))

        for name, ok, detail in results:
            self.log(f"  {'PASS' if ok else 'FAIL'}  {name}  {detail}")
        if not all(ok for _, ok, _ in results):
            raise Stop("preflight failed; nothing was changed")

    # ================= STEP 1: RUNNER USER (R1) =================
    def step1_runner(self):
        iam, name = self.c("iam"), self.runner()
        role_arn = f"arn:aws:iam::{self.acct}:role/{self.s['principals']['project_role']['name']}"
        doc = {"Version": "2012-10-17",
               "Statement": [{"Effect": "Allow", "Action": "sts:AssumeRole", "Resource": role_arn}]}
        try:
            iam.get_user(UserName=name)
            self.log(f"    SKIP: user {name} exists")
        except ClientError as e:
            if err_code(e) != "NoSuchEntity":
                raise
            if self.would(f"create user {name} (no console, no keys)"):
                iam.create_user(UserName=name, Path=self.s["principals"]["runner_user"]["path"])
        if self.would(f"put inline policy assume-ops-audit-role on {name} -> {role_arn}"):
            iam.put_user_policy(UserName=name, PolicyName="assume-ops-audit-role", PolicyDocument=json.dumps(doc))

    # ================= STEP 2: FILLER ROLES + ListRoles order check =================
    def trust_root(self):
        return {"Version": "2012-10-17", "Statement": [{"Effect": "Allow", "Action": "sts:AssumeRole",
                "Principal": {"AWS": f"arn:aws:iam::{self.acct}:root"}}]}

    def step2_fillers(self):
        iam, names = self.c("iam"), self.filler_roles()
        live = set(self.flat(self.live_roles()))
        todo = [n for n in names if n not in live]
        self.log(f"    {len(names) - len(todo)} filler roles exist, {len(todo)} to create")
        if not self.would(f"create {len(todo)} roles {todo[:2]}...{todo[-1:]} trusting the account root only"):
            return
        for n in todo:
            try:
                iam.create_role(RoleName=n, Path="/", AssumeRolePolicyDocument=json.dumps(self.trust_root()))
            except ClientError as e:
                if err_code(e) != "EntityAlreadyExists":
                    raise
        for _ in range(12):   # IAM listing can lag behind creation
            if set(names) <= set(self.flat(self.live_roles())):
                break
            self.sleep(5)
        self.check_listroles(final=False)

    def check_listroles(self, final):
        pages = self.live_roles()
        flat = self.flat(pages)
        if flat == sorted(flat):
            rule, key = "ascii", (lambda x: x)
        elif flat == sorted(flat, key=str.lower):
            rule, key = "case-insensitive", str.lower
        else:
            raise Stop("ListRoles is NOT alphabetical: rule 5 / C11 / C35 must be redesigned (Addendum item 6)")
        self.rec["orders"]["ListRoles_rule"] = rule
        self.rec["page_sizes"]["ListRoles"] = [len(p) for p in pages]
        before = sum(1 for r in flat if key(r) < key("collegeproj-"))
        ok = before >= len(pages[0]) if len(pages) > 1 else False
        if final:
            ok = ok and not any(r.startswith("collegeproj-") for r in pages[0])
        self.log(f"    ListRoles: {rule} order, page sizes {self.rec['page_sizes']['ListRoles']}, "
                 f"{before} roles sort before the collegeproj- block")
        if not ok:
            raise Stop("the collegeproj- block would not sit entirely on page 2 (C35)")
        return True

    # ================= STEP 3: collegeproj- ROLES + project-role trust =================
    def step3_cp_roles(self):
        iam = self.c("iam")
        live = set(self.flat(self.live_roles()))
        for r in self.cp_roles():
            name = r["name"]
            principal = ({"Service": r["trust"]} if r["trust"].endswith(".amazonaws.com")
                         else {"AWS": f"arn:aws:iam::{self.acct}:root"})
            trust = {"Version": "2012-10-17",
                     "Statement": [{"Effect": "Allow", "Principal": principal, "Action": "sts:AssumeRole"}]}
            if name in live:
                self.log(f"    SKIP: role {name} exists")
            elif self.would(f"create role {name} trusting {r['trust']}"):
                iam.create_role(RoleName=name, Path="/", AssumeRolePolicyDocument=json.dumps(trust))
            for parn in r["attached"]:
                iam.get_policy(PolicyArn=parn)          # fails loudly if the managed policy name is wrong
                if self.would(f"attach {parn.split('/')[-1]} to {name}"):
                    iam.attach_role_policy(RoleName=name, PolicyArn=parn)
            for p in r["inline"]:
                doc = {"Version": "2012-10-17", "Statement": [{"Effect": "Allow", "Action": p["actions"],
                       "Resource": [self.fmt(x) for x in p["resources"]]}]}
                if self.would(f"inline policy {p['name']} on {name}"):
                    iam.put_role_policy(RoleName=name, PolicyName=p["name"], PolicyDocument=json.dumps(doc))
        role = self.s["principals"]["project_role"]["name"]
        runner_arn = f"arn:aws:iam::{self.acct}:user/{self.runner()}"
        trust = {"Version": "2012-10-17", "Statement": [{"Effect": "Allow", "Action": "sts:AssumeRole",
                 "Principal": {"AWS": runner_arn}}]}
        if self.would(f"set {role} trust to {self.runner()} only (admin-eye-v2 can no longer assume it)"):
            self.retry(lambda: iam.update_assume_role_policy(RoleName=role, PolicyDocument=json.dumps(trust)),
                       lambda e: err_code(e) == "MalformedPolicyDocument", "new user as principal")

    # ================= STEP 4: S3 =================
    def make_bucket(self, name):
        s3 = self.c("s3")
        if not self.would(f"create bucket {name}"):
            return False
        try:
            s3.create_bucket(Bucket=name, CreateBucketConfiguration={"LocationConstraint": self.region})
            s3.get_waiter("bucket_exists").wait(Bucket=name)
        except ClientError as e:
            if err_code(e) == "BucketAlreadyOwnedByYou":
                self.log(f"    SKIP: bucket {name} exists")
            elif err_code(e) == "BucketAlreadyExists":
                raise Stop(f"bucket name {name} is taken by another account: change bucket_suffix in the spec")
            else:
                raise
        return True

    def step4_s3(self):
        s3, b = self.c("s3"), self.s["s3"]
        for name in self.planned_buckets():
            self.make_bucket(name)
        if not self.apply:
            self.log("    WOULD: T1 policies, T9 BPA deletions, T4 lifecycle/tags/versioning (see spec)")
            return
        for n in b["t1_data"]["with_policy"]:
            name = f"collegeproj-data-{n}-{self.sfx}"
            pol = {"Version": "2012-10-17", "Statement": [{"Sid": "DenyInsecureTransport", "Effect": "Deny",
                   "Principal": "*", "Action": "s3:*",
                   "Resource": [f"arn:aws:s3:::{name}", f"arn:aws:s3:::{name}/*"],
                   "Condition": {"Bool": {"aws:SecureTransport": "false"}}}]}
            self.log(f"    DO: non-public policy on {name}")
            s3.put_bucket_policy(Bucket=name, Policy=json.dumps(pol))
        for n in b["t9_app"]["bpa_deleted"]:
            name = f"collegeproj-app-{n}-{self.sfx}"
            self.log(f"    DO: delete bucket-level public access block on {name}")
            s3.delete_public_access_block(Bucket=name)
        for name, cfg in self.t4():
            if cfg["lifecycle"]:
                self.log(f"    DO: lifecycle on {name}")
                s3.put_bucket_lifecycle_configuration(Bucket=name, LifecycleConfiguration={"Rules": [{
                    "ID": "expire-old-objects", "Filter": {"Prefix": ""}, "Status": "Enabled",
                    "Expiration": {"Days": 365}, "AbortIncompleteMultipartUpload": {"DaysAfterInitiation": 7}}]})
            if cfg["tags"]:
                self.log(f"    DO: tags on {name}")
                s3.put_bucket_tagging(Bucket=name, Tagging={"TagSet": self.tagset(self.s["tags"]["bucket"])})
            if cfg["versioning"] == "Enabled":
                self.log(f"    DO: versioning on {name}")
                s3.put_bucket_versioning(Bucket=name, VersioningConfiguration={"Status": "Enabled"})
        # record the two answer-bearing error codes (one was unverified)
        nopol = f"collegeproj-data-{b['t1_data']['without_policy'][0]}-{self.sfx}"
        nobpa = f"collegeproj-app-{b['t9_app']['bpa_deleted'][0]}-{self.sfx}"
        for label, fn in (("GetBucketPolicy on a no-policy bucket", lambda: s3.get_bucket_policy(Bucket=nopol)),
                          ("GetPublicAccessBlock on a deleted-BPA bucket",
                           lambda: s3.get_public_access_block(Bucket=nobpa))):
            try:
                fn()
                self.rec["recorded"][label] = "NO ERROR (unexpected: dead end missing)"
            except ClientError as e:
                self.rec["recorded"][label] = err_code(e)
            self.log(f"    RECORDED: {label} -> {self.rec['recorded'][label]}")

    # ================= STEP 5: LOG GROUPS =================
    def make_group(self, name, retention):
        logs = self.c("logs")
        if not self.would(f"log group {name} retention {retention}"):
            return
        try:
            logs.create_log_group(logGroupName=name, logGroupClass=self.s["logs"]["log_group_class"])
        except ClientError as e:
            if err_code(e) == "ResourceAlreadyExistsException":
                return   # never touch retention on re-run (step 8 may have removed it on purpose)
            raise
        logs.put_retention_policy(logGroupName=name, retentionInDays=retention)

    def step5_logs(self):
        live = set(self.flat(self.live_groups()))
        cycle = self.s["logs"]["other_groups"]["initial_retention_cycle"]
        groups = [(g, 30) for g in self.lambda_groups()] + \
                 [(g, cycle[i % len(cycle)]) for i, g in enumerate(self.other_groups())]
        todo = [(g, r) for g, r in groups if g not in live]
        self.log(f"    {len(groups) - len(todo)} log groups exist, {len(todo)} to create (lambda groups first, B30)")
        for g, r in todo:
            self.make_group(g, r)

    # ================= STEP 6: LAMBDA =================
    @staticmethod
    def zip_code(runtime):
        buf = io.BytesIO()
        with zipfile.ZipFile(buf, "w", zipfile.ZIP_DEFLATED) as z:
            if runtime.startswith("python"):
                z.writestr("lambda_function.py", "def handler(event, context):\n    return {}\n")
            else:
                z.writestr("index.mjs", "export const handler = async () => ({});\n")
        return buf.getvalue(), ("lambda_function.handler" if runtime.startswith("python") else "index.handler")

    def step6_lambda(self):
        lam = self.c("lambda")
        dead_end_done = self.export_dead_end_done() if self.apply else False
        for f in self.functions():
            name = f["name"]
            if name == self.export_fn() and dead_end_done:
                self.log(f"    SKIP: {name} already deleted on purpose (T7 dead end exists)")
                continue
            if self.fn_exists(name):
                self.log(f"    SKIP: function {name} exists")
            elif self.would(f"create {name} {f['runtime']} timeout {f['timeout']} role {f['role']}"):
                code, handler = self.zip_code(f["runtime"])
                kw = dict(FunctionName=name, Runtime=f["runtime"], Handler=handler, Code={"ZipFile": code},
                          Role=f"arn:aws:iam::{self.acct}:role/collegeproj-lambda-{f['role']}",
                          Timeout=f["timeout"], MemorySize=128, Publish=False)
                if f["tags"]:
                    kw["Tags"] = f["tags"]
                self.retry(lambda: lam.create_function(**kw),
                           lambda e: err_code(e) == "InvalidParameterValueException" and "assume" in err_msg(e).lower(),
                           "new execution role")
                lam.get_waiter("function_active_v2").wait(FunctionName=name)
            if f.get("reserved_concurrency") and self.would(f"reserved concurrency {f['reserved_concurrency']} on {name}"):
                lam.put_function_concurrency(FunctionName=name, ReservedConcurrentExecutions=f["reserved_concurrency"])

    # ================= STEP 7: SNS + T7 dead end =================
    def add_permission(self, fn, sid, principal, source_arn):
        try:
            self.c("lambda").add_permission(FunctionName=fn, StatementId=sid, Action="lambda:InvokeFunction",
                                            Principal=principal, SourceArn=source_arn, SourceAccount=self.acct)
        except ClientError as e:
            if err_code(e) != "ResourceConflictException":
                raise

    def step7_sns(self):
        sns, lam = self.c("sns"), self.c("lambda")
        dead_end_done = self.export_dead_end_done() if self.apply else False
        export = self.export_fn()
        for t in self.s["sns"]["topics"]:
            if not self.would(f"topic {t['name']}" + (f" -> lambda {t['lambda_subscriber']}" if t["lambda_subscriber"] else "")):
                continue
            arn = sns.create_topic(Name=t["name"])["TopicArn"]
            fn = t["lambda_subscriber"]
            if not fn or (fn == export and dead_end_done):
                continue
            self.add_permission(fn, f"sns-{t['name']}", "sns.amazonaws.com", arn)
            sns.subscribe(TopicArn=arn, Protocol="lambda", Endpoint=self.fn_arn(fn), ReturnSubscriptionArn=True)
        if not self.apply:
            self.log(f"    WOULD: delete {export}, then check OPEN-32 and OPEN-33")
            return
        if self.fn_exists(export):
            self.log(f"    DO: delete {export} (T7 dead end)")
            lam.delete_function(FunctionName=export)
        topic = [t["name"] for t in self.s["sns"]["topics"] if t["lambda_subscriber"] == export][0]
        subs = sns.list_subscriptions_by_topic(TopicArn=self.topic_arn(topic))["Subscriptions"]
        survived = any(s["Endpoint"] == self.fn_arn(export) for s in subs)
        self.rec["recorded"]["OPEN-32 subscription survives function deletion"] = survived
        self.log(f"    RECORDED: OPEN-32 subscription survives -> {survived}")
        if not survived:
            raise Stop("OPEN-32: the subscription vanished with the function; rule a T7 fallback")
        try:
            r = lam.get_function_concurrency(FunctionName=self.fn_arn(export))
            self.rec["recorded"]["OPEN-33 GetFunctionConcurrency on deleted ARN"] = \
                f"no error: {r.get('ReservedConcurrentExecutions', 'field absent')}"
        except ClientError as e:
            self.rec["recorded"]["OPEN-33 GetFunctionConcurrency on deleted ARN"] = err_code(e)
        self.log(f"    RECORDED: OPEN-33 -> {self.rec['recorded']['OPEN-33 GetFunctionConcurrency on deleted ARN']}")

    # ================= STEP 8: T3 selection and filters =================
    def step8_t3(self):
        lg = self.s["logs"]
        if not self.apply:
            self.log("    WOULD: observe DescribeLogGroups order, remove retention at positions "
                     f"{lg['no_retention_selection']['positions_1_based']}, metric + subscription filters")
            return
        logs = self.c("logs")
        pages = self.live_groups()
        order = self.flat(pages)
        psize = len(pages[0])
        self.rec["orders"]["DescribeLogGroups"] = order
        self.rec["page_sizes"]["DescribeLogGroups"] = [len(p) for p in pages]
        if sorted(order) != sorted(self.lambda_groups() + self.other_groups()):
            raise Stop("log groups in us-west-2 differ from the spec; not selecting")
        at = lambda pos: order[pos - 1]
        nr = lg["no_retention_selection"]["positions_1_based"]
        mf = lg["metric_filters"]["retained_positions"]
        sub = lg["subscription_filters"]["on_positions"]
        problems = []
        if any(at(p).startswith("/aws/lambda/") for p in nr + mf + sub):
            problems.append("a chosen position is a /aws/lambda group")
        if 1 in nr or len(order) in nr:
            problems.append("first or last group chosen")
        if sum(p <= psize for p in nr) != 4 or sum(p > psize for p in nr) != 4:
            problems.append(f"no-retention split is not 4/4 at observed page size {psize}")
        if not set(sub) <= set(nr) or set(mf) & set(nr):
            problems.append("filter positions inconsistent")
        if problems:
            raise Stop("T3 selection rule fails under the observed order: " + "; ".join(problems))
        self.rec["positions"] = {"no_retention": {p: at(p) for p in nr}, "metric_filter_retained": {p: at(p) for p in mf},
                                 "subscription": {p: at(p) for p in sub}}
        has_ret = {g["logGroupName"] for g in self.live_group_details() if "retentionInDays" in g}
        for p in nr:
            if at(p) in has_ret:
                self.log(f"    DO: remove retention on {at(p)} (position {p})")
                logs.delete_retention_policy(logGroupName=at(p))
            else:
                self.log(f"    SKIP: {at(p)} already has no retention")
        mdef = {"filterName": "error-count", "filterPattern": "ERROR", "metricTransformations": [
            {"metricName": "ErrorCount", "metricNamespace": "CollegeProj/Logs", "metricValue": "1"}]}
        for p in nr + mf:
            logs.put_metric_filter(logGroupName=at(p), **mdef)
        self.log(f"    DO: metric filters on {len(nr + mf)} groups")
        dest = self.fn_arn("collegeproj-fn-reports")
        for p in sub:
            g = at(p)
            self.add_permission("collegeproj-fn-reports", f"logs-sub-{p}", "logs.amazonaws.com",
                                f"arn:aws:logs:{self.region}:{self.acct}:log-group:{g}:*")
            self.log(f"    DO: subscription filter on {g} -> collegeproj-fn-reports")
            self.retry(lambda: logs.put_subscription_filter(logGroupName=g, filterName="forward-to-reports",
                                                            filterPattern="", destinationArn=dest),
                       lambda e: err_code(e) == "InvalidParameterException", "Logs permission on the function")
        den = int(re.search(r"position (\d+)", lg["planned_denial"]).group(1))
        self.rec["recorded"]["T3 denial target (for the role policy later)"] = at(den)

    # ================= STEP 9: SSM =================
    def step9_ssm(self):
        ssm = self.c("ssm")
        live = set(self.flat(self.live_params()))
        params = self.ssm_params()
        todo = [p for p in params if p[0] not in live]
        self.log(f"    {len(params) - len(todo)} parameters exist, {len(todo)} to create, in name order")
        if not self.would(f"create {len(todo)} parameters (Standard tier)"):
            return
        for name, ptype, value, tags in todo:
            kw = dict(Name=name, Value=value, Type=ptype, Tier="Standard")
            if tags:
                kw["Tags"] = self.tagset(tags)
            try:
                ssm.put_parameter(**kw)
            except ClientError as e:
                if err_code(e) != "ParameterAlreadyExists":
                    raise

    # ================= STEP 10: DYNAMODB =================
    def table_kw(self, billing, rcu=None, wcu=None):
        kw = dict(AttributeDefinitions=[{"AttributeName": "pk", "AttributeType": "S"}],
                  KeySchema=[{"AttributeName": "pk", "KeyType": "HASH"}], BillingMode=billing)
        if billing == "PROVISIONED":
            kw["ProvisionedThroughput"] = {"ReadCapacityUnits": rcu, "WriteCapacityUnits": wcu}
        return kw

    def step10_dynamodb(self):
        ddb = self.c("dynamodb")
        for t in self.s["dynamodb"]["standalone"]:
            if not self.would(f"table {t['name']} {t['billing']}"):
                continue
            try:
                ddb.create_table(TableName=t["name"], **self.table_kw(t["billing"], t.get("rcu"), t.get("wcu")))
            except ClientError as e:
                if err_code(e) != "ResourceInUseException":
                    raise
                self.log(f"    SKIP: table {t['name']} exists")
            ddb.get_waiter("table_exists").wait(TableName=t["name"])

    # ================= STEP 11: CLOUDFORMATION =================
    def template(self, st):
        res = {}
        for r in st["resources"]:
            if r["type"] == "table":
                kw = self.table_kw(r["billing"], r.get("rcu"), r.get("wcu"))
                res[r["logical_id"]] = {"Type": "AWS::DynamoDB::Table", "Properties": kw}
            else:
                props = {"VersioningConfiguration": {"Status": "Enabled"}} if r["versioning"] == "Enabled" else {}
                res[r["logical_id"]] = {"Type": "AWS::S3::Bucket", "Properties": props}
        return json.dumps({"AWSTemplateFormatVersion": "2010-09-09", "Resources": res})

    def step11_cfn(self):
        cfn, s3 = self.c("cloudformation"), self.c("s3")
        live = set(self.flat(self.live_stacks()))
        stacks = self.stacks()
        for name in self.s["cloudformation"]["create_order"]:
            if name in live:
                self.log(f"    SKIP: stack {name} exists")
                if self.apply:
                    cfn.update_termination_protection(EnableTerminationProtection=True, StackName=name)
                continue
            if self.would(f"stack {name} ({', '.join(r['logical_id'] for r in stacks[name]['resources'])}), termination protection on"):
                cfn.create_stack(StackName=name, TemplateBody=self.template(stacks[name]),
                                 EnableTerminationProtection=True)
                cfn.get_waiter("stack_create_complete").wait(StackName=name)   # one at a time: creation order = known order
        for name, st in stacks.items():
            for r in st["resources"]:
                if "after_create" not in r:
                    continue
                if not self.would(f"delete bucket {r['logical_id']} of {name} out-of-band (stack stays)"):
                    continue
                phys = cfn.describe_stack_resource(StackName=name, LogicalResourceId=r["logical_id"])[
                    "StackResourceDetail"]["PhysicalResourceId"]
                self.rec["recorded"]["T12 deleted stack bucket"] = phys
                try:
                    s3.delete_bucket(Bucket=phys)
                    self.log(f"    DONE: deleted {phys}")
                except ClientError as e:
                    if err_code(e) != "NoSuchBucket":
                        raise
                    self.log(f"    SKIP: {phys} already deleted")

    # ================= STEP 12: ECR =================
    def step12_ecr(self):
        ecr = self.c("ecr")
        e = self.s["ecr"]
        lifecycle = {"rules": [{"rulePriority": 1, "description": "expire untagged images after 14 days",
                                "selection": {"tagStatus": "untagged", "countType": "sinceImagePushed",
                                              "countUnit": "days", "countNumber": 14},
                                "action": {"type": "expire"}}]}
        policy = {"Version": "2012-10-17", "Statement": [{"Sid": "AllowPullFromThisAccount", "Effect": "Allow",
                  "Principal": {"AWS": f"arn:aws:iam::{self.acct}:root"},
                  "Action": ["ecr:BatchGetImage", "ecr:GetDownloadUrlForLayer"]}]}
        for r in self.repos():
            if not self.would(f"repo {r['name']} lifecycle={r['lifecycle']} policy={r['repo_policy']} tags={r['tags']}"):
                continue
            kw = dict(repositoryName=r["name"], imageTagMutability="MUTABLE",
                      imageScanningConfiguration={"scanOnPush": False},
                      encryptionConfiguration={"encryptionType": "AES256"})
            if r["tags"]:
                kw["tags"] = self.tagset(self.s["tags"]["ecr"])
            try:
                ecr.create_repository(**kw)
            except ClientError as ex:
                if err_code(ex) != "RepositoryAlreadyExistsException":
                    raise
                self.log(f"    SKIP: repo {r['name']} exists")
            if r["lifecycle"]:
                ecr.put_lifecycle_policy(repositoryName=r["name"], lifecyclePolicyText=json.dumps(lifecycle))
            if r["repo_policy"]:
                ecr.set_repository_policy(repositoryName=r["name"], policyText=json.dumps(policy))

    # ================= STEP 13: VERIFY =================
    def step13_verify(self):
        if self.acct is None:
            self.acct = self.c("sts").get_caller_identity()["Account"]
        v = []

        def check(name, ok, detail=""):
            v.append({"check": name, "ok": bool(ok), "detail": detail})

        def mid(name, items, target):
            ok = target in items and items.index(target) not in (0, len(items) - 1)
            check(f"mid-list: {name}", ok, f"{items.index(target) + 1} of {len(items)}" if target in items else "missing")

        exp = self.s["expected_counts_after_build"]
        o = self.rec["orders"]
        bpages = self.live_buckets()
        o["ListBuckets"] = self.flat(bpages)
        check("buckets_total", len(o["ListBuckets"]) == exp["buckets_total"], len(o["ListBuckets"]))
        try:
            self.check_listroles(final=True)
            check("ListRoles alphabetical + collegeproj- block on page 2", True, self.rec["orders"]["ListRoles_rule"])
        except Stop as s:
            check("ListRoles alphabetical + collegeproj- block on page 2", False, str(s))
        roles = self.flat(self.live_roles())
        o["ListRoles"] = roles
        check("roles_total", len(roles) == exp["roles_total"], len(roles))
        users = self.live_users()
        check("users_total", len(users) == exp["users_total"], users)
        gpages = self.live_groups()
        o["DescribeLogGroups"] = self.flat(gpages)
        self.rec["page_sizes"]["DescribeLogGroups"] = [len(p) for p in gpages]
        check("log_groups_us_west_2", len(o["DescribeLogGroups"]) == exp["log_groups_us_west_2"], len(o["DescribeLogGroups"]))
        details = self.live_group_details()
        nores = [g["logGroupName"] for g in details if "retentionInDays" not in g]
        p1 = set(gpages[0])
        check("T3: 8 groups without retention, on both pages", len(nores) == 8 and 0 < sum(g in p1 for g in nores) < 8,
              f"{len(nores)} total, {sum(g in p1 for g in nores)} on page 1")
        check("B20: retention values only 7/30/90",
              all(g.get("retentionInDays") in (None, 7, 30, 90) for g in details))
        fns = self.flat(self.live_functions())
        o["ListFunctions"] = fns
        check("lambda_functions_live", len(fns) == exp["lambda_functions_live"], len(fns))
        topics = self.flat(self.live_topics())
        o["ListTopics"] = topics
        check("sns_topics", len(topics) == exp["sns_topics"], len(topics))
        nsubs = sum(1 for t in topics for s in self.c("sns").list_subscriptions_by_topic(TopicArn=t)["Subscriptions"]
                    if s["Protocol"] == "lambda")
        check("sns_lambda_subscriptions", nsubs == exp["sns_lambda_subscriptions"], nsubs)
        ppages = self.live_params()
        o["DescribeParameters"] = self.flat(ppages)
        self.rec["page_sizes"]["DescribeParameters"] = [len(p) for p in ppages]
        check("ssm_parameters", len(o["DescribeParameters"]) == exp["ssm_parameters"], len(o["DescribeParameters"]))
        prod = [x for x in o["DescribeParameters"] if x.startswith("/collegeproj/prod/")]
        check("T8 (B16): no /collegeproj/prod/ parameter on page 1", prod and not any(x in ppages[0] for x in prod),
              f"page sizes {self.rec['page_sizes']['DescribeParameters']}")
        tables = self.flat(self.live_tables())
        o["ListTables"] = tables
        check("dynamodb_tables", len(tables) == exp["dynamodb_tables"], len(tables))
        stacks = self.flat(self.live_stacks())
        o["ListStacks"] = stacks
        check("cloudformation_stacks", len(stacks) == exp["cloudformation_stacks"], len(stacks))
        repos = self.flat(self.live_repos())
        o["DescribeRepositories"] = repos
        check("ecr_repositories", len(repos) == exp["ecr_repositories"], len(repos))

        # mid-list placements of planned and reserve denials, under the OBSERVED orders
        sfx = self.sfx
        mid("T1 features", [x for x in o["ListBuckets"] if x.startswith("collegeproj-data-")], f"collegeproj-data-features-{sfx}")
        mid("T4 archive-invoices (reserve)", [x for x in o["ListBuckets"] if "archive" in x], f"collegeproj-archive-invoices-{sfx}")
        mid("T9 frontend (reserve)", [x for x in o["ListBuckets"] if x.startswith("collegeproj-app-")], f"collegeproj-app-frontend-{sfx}")
        mid("T2 svc-notify", [x for x in roles if x.startswith("collegeproj-svc-")], "collegeproj-svc-notify")
        svc_trust = [r["name"] for r in self.cp_roles() if r["trust"].endswith(".amazonaws.com")]
        mid("T10 lambda-exec-b", [x for x in roles if x in svc_trust], "collegeproj-lambda-exec-b")
        mid("T6 fn-ledger", [x for x in fns if x.startswith("collegeproj-")], "collegeproj-fn-ledger")
        mid("T5 collegeproj-batch (reserve)", [x for x in repos if x.startswith("collegeproj-")], "collegeproj-batch")
        mid("T8 db-host", prod, "/collegeproj/prod/db-host")
        mid("T11 profiles", tables, "collegeproj-profiles")
        mid("B17 shared stack", stacks, "collegeproj-stack-core")

        lam, cfn = self.c("lambda"), self.c("cloudformation")
        for f in fns:
            vers = lam.list_versions_by_function(FunctionName=f)["Versions"]
            al = lam.list_aliases(FunctionName=f)["Aliases"]
            rc = lam.get_function_concurrency(FunctionName=f).get("ReservedConcurrentExecutions")
            want = next((x.get("reserved_concurrency") for x in self.functions() if x["name"] == f), None)
            check(f"B29/B21 {f}: no versions/aliases, reserved concurrency as spec",
                  [x["Version"] for x in vers] == ["$LATEST"] and not al and rc == want, f"rc={rc}")
        logs = self.c("logs")
        streams = {g: len(logs.describe_log_streams(logGroupName=g, limit=1)["logStreams"])
                   for g in self.lambda_groups()}
        bad = [g for g, n in streams.items() if n and not g.endswith("collegeproj-fn-reports")]
        check("B14: no Lambda log streams (fn-reports tolerated)", not bad, bad)
        if streams.get("/aws/lambda/collegeproj-fn-reports"):
            self.rec["recorded"]["fn-reports log stream after subscription setup"] = True
        prot = {s: cfn.describe_stacks(StackName=s)["Stacks"][0].get("EnableTerminationProtection") for s in stacks}
        check("B1: termination protection on every stack", all(prot.values()), prot)
        s3 = self.c("s3")
        stack_buckets = {}
        for s in stacks:
            for r in cfn.describe_stack_resources(StackName=s)["StackResources"]:
                if r["ResourceType"] == "AWS::S3::Bucket":
                    try:
                        s3.head_bucket(Bucket=r["PhysicalResourceId"])
                        stack_buckets[r["PhysicalResourceId"]] = "exists"
                    except ClientError:
                        stack_buckets[r["PhysicalResourceId"]] = "deleted"
        self.rec["recorded"]["stack buckets"] = stack_buckets
        check("T12: 5 stack buckets, exactly 1 deleted",
              len(stack_buckets) == 5 and list(stack_buckets.values()).count("deleted") == 1, stack_buckets)
        ct_home = self.s["existing"]["trail"]["home_region"]
        check("trail still logging",
              self.c("cloudtrail", ct_home).get_trail_status(Name=self.s["existing"]["trail"]["name"]).get("IsLogging"))
        try:
            self.c("s3control").get_public_access_block(AccountId=self.acct)
            check("C15: account-level BPA still unconfigured", False)
        except ClientError as e:
            check("C15: account-level BPA still unconfigured", err_code(e) == "NoSuchPublicAccessBlockConfiguration")
        self.rec["verify"] = v
        for x in v:
            self.log(f"  {'PASS' if x['ok'] else 'FAIL'}  {x['check']}  {x['detail']}")
        return all(x["ok"] for x in v)

    # ================= STEP 14: INVENTORY =================
    def step14_inventory(self):
        path = os.path.join(self.out_dir, f"inventory_{self.stamp}.json")
        with open(path, "w", encoding="utf-8") as fh:
            json.dump({"account": self.acct, "spec_version": self.s["spec_version"], "stamp": self.stamp,
                       **self.rec}, fh, indent=2, default=str)
        self.log(f"    inventory written to {path} (private: never commit)")

    STEPS = [(0, "preflight", "step0_preflight"), (1, "runner user", "step1_runner"),
             (2, "filler roles + ListRoles check", "step2_fillers"), (3, "collegeproj- roles", "step3_cp_roles"),
             (4, "S3", "step4_s3"), (5, "log groups", "step5_logs"), (6, "Lambda", "step6_lambda"),
             (7, "SNS + T7 dead end", "step7_sns"), (8, "T3 retention + filters", "step8_t3"),
             (9, "SSM", "step9_ssm"), (10, "DynamoDB", "step10_dynamodb"), (11, "CloudFormation", "step11_cfn"),
             (12, "ECR", "step12_ecr")]

    def run(self, stop_after=LAST_STEP, verify_only=False):
        mode = "VERIFY-ONLY" if verify_only else ("APPLY" if self.apply else "DRY RUN (nothing is changed)")
        self.log(f"=== IDP build, spec {self.s['spec_version']}, {mode} ===")
        try:
            if not verify_only:
                for n, label, fn in self.STEPS:
                    if n > stop_after:
                        self.log(f"=== stopped after step {stop_after} as asked ===")
                        return 0
                    self.log(f"--- step {n}: {label} ---")
                    getattr(self, fn)()
                if not self.apply:
                    self.log("=== dry run complete; verify skipped (nothing was built) ===")
                    return 0
                if stop_after < 13:
                    return 0
            self.log("--- step 13: verify ---")
            ok = self.step13_verify()
            self.log("--- step 14: inventory ---")
            self.step14_inventory()
            self.log("=== BUILD VERIFIED ===" if ok else "=== VERIFY FOUND PROBLEMS: see FAIL lines ===")
            return 0 if ok else 2
        except Stop as s:
            self.log(f"=== STOPPED: {s} ===")
            return 1
        except ClientError as e:
            self.log(f"=== STOPPED on unexpected AWS error: {err_code(e)}: {err_msg(e)} ===")
            return 1
        except WaiterError as e:
            self.log(f"=== STOPPED: a resource did not become ready in time: {e} (re-run is safe) ===")
            return 1


def main():
    ap = argparse.ArgumentParser(description="IDP account build (spec-driven)")
    ap.add_argument("--spec", required=True)
    ap.add_argument("--out-dir", default=os.path.expanduser("~/idp/build_out"))
    ap.add_argument("--stop-after", type=int, default=LAST_STEP, help="last step to run (0-14)")
    m = ap.add_mutually_exclusive_group(required=True)
    m.add_argument("--dry-run", action="store_true")
    m.add_argument("--apply", action="store_true")
    m.add_argument("--verify-only", action="store_true")
    a = ap.parse_args()
    with open(a.spec, encoding="utf-8") as fh:
        spec = yaml.safe_load(fh)
    if spec.get("region") != "us-west-2":
        sys.exit("spec region is not us-west-2; refusing to run")
    b = Build(spec, apply=a.apply, out_dir=a.out_dir)
    sys.exit(b.run(stop_after=a.stop_after, verify_only=a.verify_only))


if __name__ == "__main__":
    main()
