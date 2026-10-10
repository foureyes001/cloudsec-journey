"""
Offline test of build.py (no network, no real AWS, nothing is created).
Every AWS call is checked against botocore's real API models (parameter names and types);
an in-memory fake account answers. Run from the folder holding build.py and the spec:
    python test_build_offline.py
Written by Claude (AI).
"""
import copy
import importlib
import io
import json
import os
import sys
import tempfile
import contextlib

HERE = os.path.dirname(os.path.abspath(__file__))
try:
    import boto3  # noqa: F401
except ImportError:                       # only for the author's sandbox
    sys.path.append("/opt/google-cloud-sdk/lib/third_party")
os.environ.update(AWS_ACCESS_KEY_ID="testing", AWS_SECRET_ACCESS_KEY="testing", AWS_DEFAULT_REGION="us-west-2")
for k in ("AWS_SESSION_TOKEN", "AWS_PROFILE"):
    os.environ.pop(k, None)

import botocore.client
import botocore.validate
from botocore.exceptions import ClientError

ACCT = "123456789012"
CALLS = []


def E(code, msg="", op="X"):
    return ClientError({"Error": {"Code": code, "Message": msg}}, op)


class Fake:
    def __init__(self):
        self.order_mode = "ascii"            # ListRoles order: ascii | shuffled
        self.subs_die_with_function = False  # OPEN-32 negative case
        self.roles, self.users, self.user_pol = {}, {}, {}
        self.buckets = {}
        self.groups = {}
        self.fns, self.perm = {}, {}
        self.rc = {}
        self.topics, self.subs = {}, {}
        self.params = {}
        self.tables = {}
        self.stacks = []
        self.repos = {}
        self.n = 0
        for b in ["agastya-project-static-website", f"idp-trail-logs-{ACCT}", "just-testing3425"]:
            self.buckets[b] = dict(policy=None, bpa=True, lifecycle=None, tags=None, versioning=None)
        for r in ["AWSServiceRoleForAPIGateway", "AWSServiceRoleForResourceExplorer", "AWSServiceRoleForServiceQuotas",
                  "AWSServiceRoleForSupport", "AWSServiceRoleForTrustedAdvisor",
                  "cloud-resume-visitor-counter-role-7hhei42w", "ops-audit-role", "testing-role"]:
            self.roles[r] = dict(trust={"Statement": [{"Principal": {"AWS": f"arn:aws:iam::{ACCT}:user/admin-eye-v2"}}]},
                                 attached=set(), inline={})
        self.users["admin-eye-v2"] = {}

    def snapshot(self):
        d = {k: ({str(kk): vv for kk, vv in v.items()} if k == "user_pol" else v) for k, v in self.__dict__.items()}
        return json.dumps(d, default=lambda o: sorted(o) if isinstance(o, set) else str(o), sort_keys=True)

    @staticmethod
    def page(items, size, token, key_in):
        start = int(token) if token else 0
        chunk = items[start:start + size]
        nxt = str(start + size) if start + size < len(items) else None
        return chunk, nxt

    @staticmethod
    def fname(x):
        return x.split(":")[-1] if x.startswith("arn:") else x

    def handle(self, svc, op, p, region):
        CALLS.append((svc, op))
        h = getattr(self, f"{svc.replace('-', '_')}_{op}", None)
        if h is None:
            raise NotImplementedError(f"fake has no {svc}.{op}")
        return h(p, region)

    # ---- sts / cloudtrail / s3control
    def sts_GetCallerIdentity(self, p, r):
        return {"Account": ACCT, "Arn": f"arn:aws:iam::{ACCT}:user/admin-eye-v2", "UserId": "AIDA"}

    def cloudtrail_DescribeTrails(self, p, r):
        assert r == "us-east-1", "trail must be read in its home region"
        return {"trailList": [{"Name": "idp-trail", "IsMultiRegionTrail": True, "IncludeGlobalServiceEvents": True,
                               "LogFileValidationEnabled": True}]}

    def cloudtrail_GetTrailStatus(self, p, r):
        assert r == "us-east-1"
        return {"IsLogging": True}

    def s3control_GetPublicAccessBlock(self, p, r):
        raise E("NoSuchPublicAccessBlockConfiguration")

    # ---- iam
    def iam_ListRoles(self, p, r):
        names = sorted(self.roles)
        if self.order_mode == "shuffled":
            names = names[::-1]
        chunk, nxt = self.page(names, p.get("MaxItems", 100), p.get("Marker"), "Marker")
        out = {"Roles": [{"RoleName": n, "Arn": f"arn:aws:iam::{ACCT}:role/{n}"} for n in chunk], "IsTruncated": bool(nxt)}
        if nxt:
            out["Marker"] = nxt
        return out

    def iam_CreateRole(self, p, r):
        if p["RoleName"] in self.roles:
            raise E("EntityAlreadyExists")
        json.loads(p["AssumeRolePolicyDocument"])
        self.roles[p["RoleName"]] = dict(trust=json.loads(p["AssumeRolePolicyDocument"]), attached=set(), inline={})
        return {"Role": {"RoleName": p["RoleName"]}}

    def iam_GetPolicy(self, p, r):
        known = {"arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole",
                 "arn:aws:iam::aws:policy/CloudWatchLogsReadOnlyAccess", "arn:aws:iam::aws:policy/AmazonS3ReadOnlyAccess"}
        if p["PolicyArn"] not in known:
            raise E("NoSuchEntity")
        return {"Policy": {"Arn": p["PolicyArn"]}}

    def iam_AttachRolePolicy(self, p, r):
        self.roles[p["RoleName"]]["attached"].add(p["PolicyArn"])
        return {}

    def iam_PutRolePolicy(self, p, r):
        doc = json.loads(p["PolicyDocument"])
        self.roles[p["RoleName"]]["inline"][p["PolicyName"]] = doc
        return {}

    def iam_UpdateAssumeRolePolicy(self, p, r):
        princ = json.loads(p["PolicyDocument"])["Statement"][0]["Principal"]["AWS"]
        assert princ.split("/")[-1] in self.users, "trust principal user missing"
        self.roles[p["RoleName"]]["trust"] = json.loads(p["PolicyDocument"])
        return {}

    def iam_ListUsers(self, p, r):
        return {"Users": [{"UserName": u} for u in sorted(self.users)], "IsTruncated": False}

    def iam_GetUser(self, p, r):
        if p["UserName"] not in self.users:
            raise E("NoSuchEntity")
        return {"User": {"UserName": p["UserName"]}}

    def iam_CreateUser(self, p, r):
        self.users[p["UserName"]] = {}
        return {"User": {"UserName": p["UserName"]}}

    def iam_PutUserPolicy(self, p, r):
        self.user_pol[(p["UserName"], p["PolicyName"])] = json.loads(p["PolicyDocument"])
        return {}

    def iam_ListAccountAliases(self, p, r):
        return {"AccountAliases": ["eye-v2-aws"], "IsTruncated": False}

    # ---- s3
    def s3_ListBuckets(self, p, r):
        return {"Buckets": [{"Name": b} for b in sorted(self.buckets)]}

    def s3_CreateBucket(self, p, r):
        assert p["CreateBucketConfiguration"]["LocationConstraint"] == "us-west-2"
        if p["Bucket"] in self.buckets:
            raise E("BucketAlreadyOwnedByYou")
        self.buckets[p["Bucket"]] = dict(policy=None, bpa=True, lifecycle=None, tags=None, versioning=None)
        return {}

    def s3_HeadBucket(self, p, r):
        if p["Bucket"] not in self.buckets:
            raise E("404")
        return {}

    def s3_PutBucketPolicy(self, p, r):
        self.buckets[p["Bucket"]]["policy"] = json.loads(p["Policy"])
        return {}

    def s3_GetBucketPolicy(self, p, r):
        if not self.buckets[p["Bucket"]]["policy"]:
            raise E("NoSuchBucketPolicy")
        return {"Policy": json.dumps(self.buckets[p["Bucket"]]["policy"])}

    def s3_DeletePublicAccessBlock(self, p, r):
        self.buckets[p["Bucket"]]["bpa"] = False
        return {}

    def s3_GetPublicAccessBlock(self, p, r):
        if not self.buckets[p["Bucket"]]["bpa"]:
            raise E("NoSuchPublicAccessBlockConfiguration")
        return {"PublicAccessBlockConfiguration": {}}

    def s3_PutBucketLifecycleConfiguration(self, p, r):
        self.buckets[p["Bucket"]]["lifecycle"] = p["LifecycleConfiguration"]
        return {}

    def s3_PutBucketTagging(self, p, r):
        self.buckets[p["Bucket"]]["tags"] = p["Tagging"]
        return {}

    def s3_PutBucketVersioning(self, p, r):
        self.buckets[p["Bucket"]]["versioning"] = p["VersioningConfiguration"]["Status"]
        return {}

    def s3_DeleteBucket(self, p, r):
        if p["Bucket"] not in self.buckets:
            raise E("NoSuchBucket")
        del self.buckets[p["Bucket"]]
        return {}

    # ---- logs
    def logs_DescribeLogGroups(self, p, r):
        names = sorted(self.groups)
        chunk, nxt = self.page(names, p.get("limit", 50), p.get("nextToken"), "nextToken")
        out = {"logGroups": [dict(logGroupName=n, **({"retentionInDays": self.groups[n]["ret"]} if self.groups[n]["ret"] else {}))
                             for n in chunk]}
        if nxt:
            out["nextToken"] = nxt
        return out

    def logs_CreateLogGroup(self, p, r):
        if p["logGroupName"] in self.groups:
            raise E("ResourceAlreadyExistsException")
        assert p.get("logGroupClass") == "STANDARD"
        self.groups[p["logGroupName"]] = dict(ret=None, mf={}, sf={})
        return {}

    def logs_PutRetentionPolicy(self, p, r):
        assert p["retentionInDays"] in (7, 30, 90)
        self.groups[p["logGroupName"]]["ret"] = p["retentionInDays"]
        return {}

    def logs_DeleteRetentionPolicy(self, p, r):
        self.groups[p["logGroupName"]]["ret"] = None
        return {}

    def logs_PutMetricFilter(self, p, r):
        self.groups[p["logGroupName"]]["mf"][p["filterName"]] = p["filterPattern"]
        return {}

    def logs_PutSubscriptionFilter(self, p, r):
        fn = self.fname(p["destinationArn"])
        assert fn in self.fns, "destination function missing"
        assert any(s["principal"] == "logs.amazonaws.com" and p["logGroupName"] in s["src"] for s in self.perm.get(fn, {}).values())
        self.groups[p["logGroupName"]]["sf"][p["filterName"]] = fn
        return {}

    def logs_DescribeLogStreams(self, p, r):
        return {"logStreams": []}

    # ---- lambda
    def lambda_ListFunctions(self, p, r):
        names = sorted(self.fns)
        chunk, nxt = self.page(names, 50, p.get("Marker"), "Marker")
        out = {"Functions": [{"FunctionName": n} for n in chunk]}
        if nxt:
            out["NextMarker"] = nxt
        return out

    def lambda_GetAccountSettings(self, p, r):
        return {"AccountLimit": {"ConcurrentExecutions": 1000}}

    def lambda_CreateFunction(self, p, r):
        role = p["Role"].split("/")[-1]
        assert role in self.roles, "role missing"
        assert p.get("Publish") is False
        if p["FunctionName"] in self.fns:
            raise E("ResourceConflictException")
        assert isinstance(p["Code"]["ZipFile"], (bytes, bytearray)) and len(p["Code"]["ZipFile"]) > 50
        self.fns[p["FunctionName"]] = dict(runtime=p["Runtime"], timeout=p["Timeout"], tags=p.get("Tags", {}), role=role)
        return {"FunctionName": p["FunctionName"], "State": "Pending"}

    def lambda_GetFunction(self, p, r):
        n = self.fname(p["FunctionName"])
        if n not in self.fns:
            raise E("ResourceNotFoundException")
        return {"Configuration": {"FunctionName": n, "State": "Active", "LastUpdateStatus": "Successful"}}

    def lambda_GetFunctionConfiguration(self, p, r):
        n = self.fname(p["FunctionName"])
        if n not in self.fns:
            raise E("ResourceNotFoundException")
        return {"FunctionName": n, "State": "Active"}

    def lambda_PutFunctionConcurrency(self, p, r):
        self.rc[p["FunctionName"]] = p["ReservedConcurrentExecutions"]
        return {"ReservedConcurrentExecutions": p["ReservedConcurrentExecutions"]}

    def lambda_GetFunctionConcurrency(self, p, r):
        n = self.fname(p["FunctionName"])
        if n not in self.fns:
            raise E("ResourceNotFoundException")
        return {"ReservedConcurrentExecutions": self.rc[n]} if n in self.rc else {}

    def lambda_AddPermission(self, p, r):
        d = self.perm.setdefault(p["FunctionName"], {})
        if p["StatementId"] in d:
            raise E("ResourceConflictException")
        d[p["StatementId"]] = {"principal": p["Principal"], "src": p["SourceArn"]}
        return {}

    def lambda_DeleteFunction(self, p, r):
        n = self.fname(p["FunctionName"])
        del self.fns[n]
        self.perm.pop(n, None)
        self.rc.pop(n, None)
        if self.subs_die_with_function:
            for t in self.subs:
                self.subs[t] = [s for s in self.subs[t] if self.fname(s) != n]
        return {}

    def lambda_ListVersionsByFunction(self, p, r):
        return {"Versions": [{"Version": "$LATEST"}]}

    def lambda_ListAliases(self, p, r):
        return {"Aliases": []}

    # ---- sns
    def sns_CreateTopic(self, p, r):
        arn = f"arn:aws:sns:{r}:{ACCT}:{p['Name']}"
        self.topics[arn] = True
        self.subs.setdefault(arn, [])
        return {"TopicArn": arn}

    def sns_Subscribe(self, p, r):
        if p["Endpoint"] not in self.subs[p["TopicArn"]]:
            self.subs[p["TopicArn"]].append(p["Endpoint"])
        return {"SubscriptionArn": p["TopicArn"] + ":sub"}

    def sns_ListTopics(self, p, r):
        return {"Topics": [{"TopicArn": t} for t in sorted(self.topics)]}

    def sns_ListSubscriptionsByTopic(self, p, r):
        if p["TopicArn"] not in self.topics:
            raise E("NotFound")
        return {"Subscriptions": [{"Endpoint": e, "Protocol": "lambda", "TopicArn": p["TopicArn"]} for e in self.subs[p["TopicArn"]]]}

    # ---- ssm
    def ssm_PutParameter(self, p, r):
        if p["Name"] in self.params:
            raise E("ParameterAlreadyExists")
        assert p["Name"].count("/") == 3
        self.params[p["Name"]] = dict(type=p["Type"], tags=p.get("Tags"))
        return {"Version": 1}

    def ssm_DescribeParameters(self, p, r):
        names = sorted(self.params)
        chunk, nxt = self.page(names, p.get("MaxResults", 10), p.get("NextToken"), "NextToken")
        out = {"Parameters": [{"Name": n, "Type": self.params[n]["type"]} for n in chunk]}
        if nxt:
            out["NextToken"] = nxt
        return out

    # ---- dynamodb
    def dynamodb_CreateTable(self, p, r):
        if p["TableName"] in self.tables:
            raise E("ResourceInUseException")
        self.tables[p["TableName"]] = p["BillingMode"]
        return {"TableDescription": {"TableName": p["TableName"], "TableStatus": "CREATING"}}

    def dynamodb_DescribeTable(self, p, r):
        if p["TableName"] not in self.tables:
            raise E("ResourceNotFoundException")
        return {"Table": {"TableName": p["TableName"], "TableStatus": "ACTIVE"}}

    def dynamodb_ListTables(self, p, r):
        return {"TableNames": sorted(self.tables)}

    # ---- cloudformation
    def _stack(self, name):
        for s in self.stacks:
            if s["name"] == name:
                return s
        raise E("ValidationError", f"Stack with id {name} does not exist")

    def cloudformation_CreateStack(self, p, r):
        tpl = json.loads(p["TemplateBody"])
        res = {}
        for lid, rr in tpl["Resources"].items():
            self.n += 1
            if rr["Type"] == "AWS::S3::Bucket":
                phys = f"{p['StackName']}-{lid.lower()}-x{self.n:04d}"
                self.buckets[phys] = dict(policy=None, bpa=True, lifecycle=None, tags=None,
                                          versioning=rr.get("Properties", {}).get("VersioningConfiguration", {}).get("Status"))
            else:
                phys = f"{p['StackName']}-{lid}-X{self.n:04d}"
                self.tables[phys] = rr["Properties"]["BillingMode"]
            res[lid] = (rr["Type"], phys)
        self.stacks.append(dict(name=p["StackName"], res=res, prot=p.get("EnableTerminationProtection", False)))
        return {"StackId": "id-" + p["StackName"]}

    def cloudformation_DescribeStacks(self, p, r):
        s = self._stack(p["StackName"])
        return {"Stacks": [{"StackName": s["name"], "StackStatus": "CREATE_COMPLETE", "CreationTime": "2026-10-09",
                            "EnableTerminationProtection": s["prot"]}]}

    def cloudformation_ListStacks(self, p, r):   # newest first, to test that nothing depends on oldest-first
        return {"StackSummaries": [{"StackName": s["name"], "StackStatus": "CREATE_COMPLETE", "StackId": "x",
                                    "CreationTime": "2026-10-09"} for s in reversed(self.stacks)]}

    def cloudformation_UpdateTerminationProtection(self, p, r):
        self._stack(p["StackName"])["prot"] = p["EnableTerminationProtection"]
        return {}

    def cloudformation_DescribeStackResource(self, p, r):
        t, phys = self._stack(p["StackName"])["res"][p["LogicalResourceId"]]
        return {"StackResourceDetail": {"PhysicalResourceId": phys, "ResourceType": t}}

    def cloudformation_DescribeStackResources(self, p, r):
        s = self._stack(p["StackName"])
        return {"StackResources": [{"LogicalResourceId": k, "ResourceType": t, "PhysicalResourceId": ph}
                                   for k, (t, ph) in s["res"].items()]}

    # ---- ecr
    def ecr_CreateRepository(self, p, r):
        if p["repositoryName"] in self.repos:
            raise E("RepositoryAlreadyExistsException")
        self.repos[p["repositoryName"]] = dict(tags=p.get("tags"), lifecycle=None, policy=None)
        return {"repository": {"repositoryName": p["repositoryName"]}}

    def ecr_DescribeRepositories(self, p, r):
        return {"repositories": [{"repositoryName": n} for n in sorted(self.repos)]}

    def ecr_PutLifecyclePolicy(self, p, r):
        self.repos[p["repositoryName"]]["lifecycle"] = json.loads(p["lifecyclePolicyText"])
        return {}

    def ecr_SetRepositoryPolicy(self, p, r):
        self.repos[p["repositoryName"]]["policy"] = json.loads(p["policyText"])
        return {}


FAKE = None


def fake_call(self, op, params):
    model = self.meta.service_model.operation_model(op)
    if model.input_shape is not None:
        botocore.validate.validate_parameters(params, model.input_shape)   # real AWS parameter rules
    svc = self.meta.service_model.service_name
    try:
        out = FAKE.handle(svc, op, params, self.meta.region_name)
    except ClientError as e:
        e.response.setdefault("ResponseMetadata", {"HTTPStatusCode": 404 if e.response["Error"]["Code"] in ("404", "NoSuchBucket") else 400})
        raise
    out.setdefault("ResponseMetadata", {"HTTPStatusCode": 200})
    return out


botocore.client.BaseClient._make_api_call = fake_call
sys.path.insert(0, HERE)
import yaml
import build

SPEC = yaml.safe_load(open(os.path.join(HERE, "idp_account_spec_v0.3.yaml"), encoding="utf-8"))


def run(apply=True, verify_only=False, stop_after=14, quiet=True):
    out = tempfile.mkdtemp()
    b = build.Build(copy.deepcopy(SPEC), apply=apply, out_dir=out)
    b.sleep = lambda s: None
    b._clients = {}
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        code = b.run(stop_after=stop_after, verify_only=verify_only)
    return code, buf.getvalue(), b


results = []


def t(name, cond, extra=""):
    results.append((name, cond))
    print(("PASS " if cond else "FAIL ") + name + (f"  [{extra}]" if extra and not cond else ""))


# 1 dry run changes nothing
FAKE = Fake(); before = FAKE.snapshot()
code, out, _ = run(apply=False)
t("dry run exits 0", code == 0, out[-800:])
t("dry run changes nothing", FAKE.snapshot() == before)
t("dry run preflight all PASS", "FAIL" not in out, [l for l in out.splitlines() if "FAIL" in l])

# 2 full apply
CALLS.clear()
code, out, b = run(apply=True)
t("apply exits 0 (BUILD VERIFIED)", code == 0 and "BUILD VERIFIED" in out,
  "\n".join(l for l in out.splitlines() if "FAIL" in l or "STOPPED" in l))
t("buckets 38", len(FAKE.buckets) == 38, len(FAKE.buckets))
t("roles 117", len(FAKE.roles) == 117)
t("users 2", sorted(FAKE.users) == ["admin-eye-v2", "ops-audit-runner"])
t("runner has no keys, one policy", list(FAKE.user_pol) == [("ops-audit-runner", "assume-ops-audit-role")])
t("ops-audit-role trusts runner only",
  FAKE.roles["ops-audit-role"]["trust"]["Statement"][0]["Principal"]["AWS"].endswith("user/ops-audit-runner"))
t("log groups 58, 8 without retention", len(FAKE.groups) == 58 and sum(g["ret"] is None for g in FAKE.groups.values()) == 8)
t("metric filters on 14 groups", sum(bool(g["mf"]) for g in FAKE.groups.values()) == 14)
t("subscription filters on 4 groups", sum(bool(g["sf"]) for g in FAKE.groups.values()) == 4)
t("8 live functions, fn-export gone", len(FAKE.fns) == 8 and "collegeproj-fn-export" not in FAKE.fns)
t("reserved concurrency only fn-orders=5", FAKE.rc == {"collegeproj-fn-orders": 5})
t("T7 dead end: exports topic still subscribed to deleted fn",
  any(e.endswith("collegeproj-fn-export") for e in FAKE.subs[f"arn:aws:sns:us-west-2:{ACCT}:collegeproj-exports"]))
t("params 80", len(FAKE.params) == 80)
t("tables 6", len(FAKE.tables) == 6)
t("stacks 4, all protected", len(FAKE.stacks) == 4 and all(s["prot"] for s in FAKE.stacks))
t("repos 6", len(FAKE.repos) == 6)
t("T4 reserve bucket has lifecycle+tags+versioning",
  all(FAKE.buckets["collegeproj-archive-invoices-u7w2"][k] for k in ("lifecycle", "tags", "versioning")))
t("T9 deleted BPA on 3", sum(not v["bpa"] for k, v in FAKE.buckets.items() if k.startswith("collegeproj-app-")) == 3)
t("T1 policies on 4", sum(bool(v["policy"]) for k, v in FAKE.buckets.items() if k.startswith("collegeproj-data-")) == 4)
t("no Lambda invoke / SNS publish calls", not any(op in ("Invoke", "Publish", "PublishBatch") for _, op in CALLS))
t("no deletes except fn-export, MediaThumbs bucket, retention",
  {op for _, op in CALLS if op.startswith("Delete")} <= {"DeleteFunction", "DeleteBucket", "DeleteRetentionPolicy", "DeletePublicAccessBlock"})
t("no stack deletion ever", not any(op == "DeleteStack" for _, op in CALLS))
t("recorded error codes", b.rec["recorded"].get("GetBucketPolicy on a no-policy bucket") == "NoSuchBucketPolicy"
  and b.rec["recorded"].get("GetPublicAccessBlock on a deleted-BPA bucket") == "NoSuchPublicAccessBlockConfiguration")
t("T3 denial target recorded", b.rec["recorded"].get("T3 denial target (for the role policy later)", "").startswith("/collegeproj/"))
inv = [f for f in os.listdir(b.out_dir) if f.startswith("inventory_")]
t("inventory written", len(inv) == 1)

# 3 re-run is idempotent
snap = FAKE.snapshot()
code, out, _ = run(apply=True)
t("re-run exits 0", code == 0, "\n".join(l for l in out.splitlines() if "FAIL" in l or "STOPPED" in l))
t("re-run changes nothing", FAKE.snapshot() == snap)
t("re-run did not recreate fn-export", "collegeproj-fn-export" not in FAKE.fns)

# 4 verify-only
code, out, _ = run(verify_only=True)
t("verify-only exits 0", code == 0)

# 5 preflight stops on an unknown resource, changing nothing
FAKE = Fake(); FAKE.buckets["someone-elses-bucket"] = dict(policy=None, bpa=True, lifecycle=None, tags=None, versioning=None)
before = FAKE.snapshot()
code, out, _ = run(apply=True)
t("unknown bucket -> preflight stop, exit 1, nothing changed", code == 1 and FAKE.snapshot() == before)

# 6 ListRoles not alphabetical -> STOP at step 2
FAKE = Fake(); FAKE.order_mode = "shuffled"
code, out, _ = run(apply=True)
t("non-alphabetical ListRoles -> STOP", code == 1 and "NOT alphabetical" in out)
t("...and nothing after step 2 was built", not FAKE.buckets.get("collegeproj-data-raw-u7w2") and not FAKE.fns)

# 7 OPEN-32 negative
FAKE = Fake(); FAKE.subs_die_with_function = True
code, out, _ = run(apply=True)
t("subscription vanishes -> OPEN-32 STOP", code == 1 and "OPEN-32" in out)

# 8 stop-after 2
FAKE = Fake()
code, out, _ = run(apply=True, stop_after=2)
t("stop-after 2 builds only user + fillers", code == 0 and len(FAKE.roles) == 108 and len(FAKE.buckets) == 3)

print(f"\n{sum(c for _, c in results)}/{len(results)} passed; API operations exercised: {len(set(CALLS))}")
