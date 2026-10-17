#!/usr/bin/env python3
"""Offline test for draft_scripts.py. No network: a fake HTTP function stands in for the API.
Run: python test_draft_scripts_offline.py   (prints PASS/FAIL per check and a total)
"""

import contextlib
import hashlib
import io
import json
import os
import re
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import importlib.util  # noqa: E402
if importlib.util.find_spec("boto3") is None:
    # Machines without boto3 (not the WSL venv): provide an empty stand-in so the import check passes.
    _fake = Path(tempfile.mkdtemp()) / "boto3"
    _fake.mkdir()
    (_fake / "__init__.py").write_text("")
    sys.path.insert(0, str(_fake.parent))
import draft_scripts as ds  # noqa: E402

FAKE_KEY = "AIzaFAKEKEY_do_not_leak_1234567890"
MODEL = "gemini-3.5-flash-lite"
results = []


def check(name, cond, detail=""):
    results.append((name, bool(cond)))
    print(f"{'PASS' if cond else 'FAIL'}  {name}" + (f"  [{detail}]" if detail and not cond else ""))


def make_prompts(root):
    pdir = root / "prompts"
    pdir.mkdir()
    lines = []
    for t in ds.TASKS:
        name = ds.PROMPT_NAME.format(task=t)
        data = f"TASK\n\nThis is collection-form task {t}.\n\nInstruction block.".encode()
        (pdir / name).write_bytes(data)
        lines.append(f"{hashlib.sha256(data).hexdigest()}  {name}")
    (pdir / "SHA256SUMS").write_text("\n".join(lines) + "\n")
    return pdir


def reply(text, thought=None, rid="r1"):
    parts = []
    if thought:
        parts.append({"text": thought, "thought": True})
    parts.append({"text": text})
    return 200, {"candidates": [{"content": {"role": "model", "parts": parts}, "finishReason": "STOP"}],
                 "usageMetadata": {"promptTokenCount": 100, "candidatesTokenCount": 200,
                                   "thoughtsTokenCount": 50, "totalTokenCount": 350},
                 "modelVersion": MODEL, "responseId": rid}


GOOD_CODE = "import boto3\nimport json\n\ndef main():\n    print(json.dumps({}))\n\nif __name__ == '__main__':\n    main()\n"
GOOD_REPLY = f"Here you go.\n```python\n{GOOD_CODE}```\nRun it with python."
TWO_BLOCKS = "```python\nimport json\n```\nand\n```python\nprint(1)\n```\n"
PROSE = "I would call list_buckets and then get_bucket_policy for each bucket."
SYNTAX_ERR = "```python\ndef broken(:\n    pass\n```"
NO_BOTO3 = "```python\nimport json\nprint(json.dumps({}))\n```"
MISSING_IMPORT = "```python\nimport boto3\nimport not_a_real_module_xyz\n```"
REGION_REPLY = "```python\nimport boto3\nREGION = 'us-east-1'\nprint(REGION)\n```"


class FakeHTTP:
    def __init__(self, script, models=None):
        self.script = {k: list(v) for k, v in script.items()}
        self.calls = []
        self.models = models if models is not None else [
            {"name": f"models/{MODEL}", "version": "001", "displayName": "Gemini 3.5 Flash-Lite",
             "supportedGenerationMethods": ["generateContent", "countTokens"]},
            {"name": "models/gemini-3.8-flash", "supportedGenerationMethods": ["generateContent"]}]

    def __call__(self, method, url, headers, body):
        self.calls.append({"method": method, "url": url, "headers": dict(headers), "body": body})
        if url.endswith("/models?pageSize=1000") or "pageToken" in url:
            return 200, {"models": self.models}
        if url.endswith(":generateContent"):
            task = re.search(r"task (T\d\d)", body["contents"][0]["parts"][0]["text"]).group(1)
            queue = self.script.get(task) or []
            if not queue:
                return reply(GOOD_REPLY, rid=f"default-{task}")
            item = queue.pop(0)
            if isinstance(item, Exception):
                raise item
            return item
        return 404, {"error": {"message": "unknown url"}}


def run(argv, http, keys_file):
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        code = ds.main(argv + ["--keys-file", str(keys_file)], http=http, sleep=lambda s: None)
    return code, out.getvalue()


def all_text_under(path):
    return "".join(p.read_text(errors="replace") for p in path.rglob("*") if p.is_file())


def main():
    os.environ.pop("GEMINI_API_KEY", None)

    # ---------------- extraction rules (S7)
    code, method, _ = ds.extract_script(GOOD_REPLY)
    check("extract: single python block", code == GOOD_CODE and method == "single python block")
    code, _, reason = ds.extract_script(TWO_BLOCKS)
    check("extract: two python blocks -> fail", code is None and "2 python blocks" in reason)
    code, _, _ = ds.extract_script("```bash\npip install boto3\n```\n```python\nimport json\nx = 1\n```")
    check("extract: bash + python block -> takes python", code == "import json\nx = 1\n")
    code, method, _ = ds.extract_script("```\nimport json\nx = 1\n```")
    check("extract: single unlabeled block", code == "import json\nx = 1\n" and method == "single unlabeled block")
    code, method, _ = ds.extract_script("import json\n\nprint(json.dumps({}))\n")
    check("extract: whole reply is code", code is not None and method == "whole reply is python")
    check("extract: one-word prose 'OK' is not a script", ds.extract_script("OK")[0] is None)
    check("extract: prose -> fail", ds.extract_script(PROSE)[0] is None)
    check("extract: empty python block -> fail", ds.extract_script("```python\n\n```")[0] is None)
    check("extract: None reply -> fail", ds.extract_script(None)[0] is None)
    check("extract: code is returned unedited (no strip)", ds.extract_script("```python\n  x = 1\n\n```")[0] == "  x = 1\n\n")

    # ---------------- acceptance
    acc = ds.acceptance("import boto3\nimport os.path\nfrom collections import OrderedDict\n")
    check("accept: boto3 + stdlib imports accepted", acc["accepted"] and acc["imports"] == ["boto3", "collections", "os"])
    acc = ds.acceptance("import json\nprint(1)\n")
    check("accept: no boto3 import -> not accepted", not acc["accepted"] and acc["reasons"] == ["does not import boto3"])
    acc = ds.acceptance("import boto3\nimport not_a_real_module_xyz\n")
    check("accept: missing import -> not accepted", not acc["accepted"] and acc["missing_imports"] == ["not_a_real_module_xyz"])
    acc = ds.acceptance("def broken(:\n  pass\n")
    check("accept: syntax error -> not accepted", not acc["accepted"] and not acc["compiles"])
    acc = ds.acceptance("import boto3\nfrom . import helpers\n")
    check("accept: relative import -> not accepted", not acc["accepted"] and acc["relative_imports"] == 1)
    acc = ds.acceptance("import boto3\nraise SystemExit('would run')\n")
    check("accept: script is never executed", acc["accepted"])
    info = ds.script_info(REGION_REPLY.split("```python\n")[1].split("```")[0])
    check("info: region mention detected", info["aws_regions_mentioned"] == ["us-east-1"])

    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        pdir = make_prompts(root)
        keys = root / "keys.env"
        keys.write_text(f"# keys\nexport GEMINI_API_KEY=\"{FAKE_KEY}\"\nOTHER=x\n")
        os.chmod(keys, 0o600)

        # ---------------- key loading
        k, src = ds.load_key("GEMINI_API_KEY", keys)
        check("key: parsed from file with export and quotes", k == FAKE_KEY and src == str(keys))
        try:
            ds.load_key("OPENAI_API_KEY", keys)
            check("key: missing key stops", False)
        except ds.Stop:
            check("key: missing key stops", True)

        # ---------------- prompt hashes
        prompts = ds.load_prompts(pdir, ds.TASKS)
        check("prompts: 13 loaded and hashes match", len(prompts) == 13)
        bad = root / "bad"
        bad.mkdir()
        for f in pdir.iterdir():
            (bad / f.name).write_bytes(f.read_bytes())
        (bad / ds.PROMPT_NAME.format(task="T05")).write_bytes(b"edited")
        try:
            ds.load_prompts(bad, ds.TASKS)
            check("prompts: edited file stops the run", False)
        except ds.Stop as e:
            check("prompts: edited file stops the run", "T05" in str(e))

        # ---------------- dry run: no network
        http = FakeHTTP({})
        code, out = run(["--dry-run", "--prompts-dir", str(pdir), "--out", str(root / "o0")], http, keys)
        check("dry-run: exit 0, zero HTTP calls", code == 0 and not http.calls, out)
        check("dry-run: key not printed", FAKE_KEY not in out)

        # ---------------- model missing
        http = FakeHTTP({}, models=[{"name": "models/gemini-3.8-flash", "supportedGenerationMethods": ["generateContent"]}])
        code, out = run(["--prompts-dir", str(pdir), "--out", str(root / "o1")], http, keys)
        check("model missing: stops with exit 1, no generate calls",
              code == 1 and not any(c["url"].endswith(":generateContent") for c in http.calls), out)

        # ---------------- full run with scripted behaviours
        script = {
            "T02": [(200, {"candidates": [{"content": {"parts": [{"text": TWO_BLOCKS}]}, "finishReason": "STOP"}]}),
                    reply(GOOD_REPLY, rid="t02-retry")],
            "T03": [reply(PROSE, rid="t03-a"), reply(PROSE, rid="t03-b")],
            "T04": [(429, {"error": {"code": 429, "status": "RESOURCE_EXHAUSTED"}}), reply(GOOD_REPLY, rid="t04")],
            "T05": [(503, {"error": {"code": 503}}), reply(GOOD_REPLY, rid="t05")],
            "T06": [reply(SYNTAX_ERR, rid="t06")],
            "T07": [reply(GOOD_REPLY, thought="secret reasoning text", rid="t07")],
            "T08": [reply(REGION_REPLY, rid="t08")],
            "T09": [reply(NO_BOTO3, rid="t09-a"), reply(NO_BOTO3, rid="t09-b")],
            "T10": [reply(MISSING_IMPORT, rid="t10-a"), reply(GOOD_REPLY, rid="t10-b")],
        }
        http = FakeHTTP(script)
        out_root = root / "out"
        code, out = run(["--prompts-dir", str(pdir), "--out", str(out_root)], http, keys)
        base = out_root / "gemini" / MODEL
        check("full run: exit 0", code == 0, out[-800:])
        metas = {(p.parent.parent.name, p.parent.name): json.loads(p.read_text()) for p in base.rglob("meta.json")}
        check("full run: 26 drafts recorded", len(metas) == 26, str(len(metas)))
        check("T01: accepted", metas[("T01", "draft1")]["status"] == "accepted")
        m = metas[("T02", "draft1")]
        check("T02: two blocks -> retry -> script from attempt 2",
              m["status"] == "accepted" and m["script_from_attempt"] == 2 and len(m["attempts"]) == 2
              and m["attempts"][0]["reasons"] == ["no script extracted (2 python blocks)"])
        m = metas[("T03", "draft1")]
        check("T03: prose twice -> 'failed', both replies saved",
              m["status"] == "failed" and len(m["attempts"]) == 2
              and (base / "T03/draft1/attempt1_reply.txt").exists() and (base / "T03/draft1/attempt2_reply.txt").exists()
              and not (base / "T03/draft1/script.py").exists())
        check("T04: 429 then success, same attempt", metas[("T04", "draft1")]["status"] == "accepted"
              and len(metas[("T04", "draft1")]["attempts"]) == 1)
        check("T05: 503 then success", metas[("T05", "draft1")]["status"] == "accepted")
        m = metas[("T06", "draft1")]
        check("T06: syntax error -> retried once -> accepted from attempt 2; bad attempt kept",
              m["status"] == "accepted" and m["script_from_attempt"] == 2 and len(m["attempts"]) == 2
              and not m["attempts"][0]["acceptance"]["compiles"] and (base / "T06/draft1/attempt1_script.py").exists())
        m = metas[("T09", "draft1")]
        check("T09: no boto3 twice -> 'failed', no study script",
              m["status"] == "failed" and len(m["attempts"]) == 2 and not (base / "T09/draft1/script.py").exists()
              and all(a["reasons"] == ["does not import boto3"] for a in m["attempts"]))
        m = metas[("T10", "draft1")]
        check("T10: missing import -> retried -> accepted", m["status"] == "accepted" and m["script_from_attempt"] == 2)
        check("settings recorded in meta", metas[("T01", "draft1")]["settings"] == {"thinking_level": "medium"})
        t07_script = (base / "T07/draft1/script.py").read_text()
        t07_reply = (base / "T07/draft1/attempt1_reply.txt").read_text()
        check("T07: thought part excluded from reply and script",
              "secret reasoning" not in t07_reply and t07_script == GOOD_CODE)
        check("T08: region recorded in meta", metas[("T08", "draft1")]["script"]["aws_regions_mentioned"] == ["us-east-1"])
        m = metas[("T01", "draft1")]
        check("meta: prompt hash recorded matches file",
              m["prompt_sha256"] == hashlib.sha256((pdir / m["prompt_file"]).read_bytes()).hexdigest())
        check("meta: response id, model version, usage recorded",
              m["attempts"][0]["response_id"] and m["attempts"][0]["model_version"] == MODEL
              and m["attempts"][0]["usage"]["thinking_tokens"] == 50)
        check("saved script identical to extracted block", (base / "T01/draft1/script.py").read_text() == GOOD_CODE)

        gen_calls = [c for c in http.calls if c["url"].endswith(":generateContent")]
        check("S6: request = user message + thinking level medium only (no system prompt, tools, other config)",
              all(set(c["body"]) == {"contents", "generationConfig"} and len(c["body"]["contents"]) == 1
                  and c["body"]["contents"][0]["role"] == "user"
                  and c["body"]["generationConfig"] == {"thinkingConfig": {"thinkingLevel": "medium"}} for c in gen_calls))
        check("S6: prompt sent byte-for-byte",
              gen_calls[0]["body"]["contents"][0]["parts"][0]["text"] == (pdir / ds.PROMPT_NAME.format(task="T01")).read_text())
        check("key: sent only in x-goog-api-key header, never in URL",
              all(c["headers"] == {"x-goog-api-key": FAKE_KEY, "Content-Type": "application/json"}
                  and FAKE_KEY not in c["url"] for c in http.calls))
        order = [re.search(r"task (T\d\d)", c["body"]["contents"][0]["parts"][0]["text"]).group(1) for c in gen_calls]
        collapsed = [t for j, t in enumerate(order) if j == 0 or order[j - 1] != t]
        check("order: T01..T13 draft 1, then T01..T13 draft 2; 33 generate calls (26 + 7 retries)",
              collapsed == ds.TASKS + ds.TASKS and len(gen_calls) == 33, f"{len(gen_calls)} {collapsed}")
        check("key: never printed", FAKE_KEY not in out)
        check("key: never written to any output file", FAKE_KEY not in all_text_under(out_root))

        # ---------------- re-run skips everything
        http2 = FakeHTTP({})
        code, out2 = run(["--prompts-dir", str(pdir), "--out", str(out_root)], http2, keys)
        check("re-run: finished drafts skipped (no generate calls)",
              code == 0 and not any(c["url"].endswith(":generateContent") for c in http2.calls), out2[-400:])

        # ---------------- changed settings: old draft moved aside as superseded, redrafted
        http6 = FakeHTTP({})
        code, out = run(["--prompts-dir", str(pdir), "--out", str(out_root), "--tasks", "T01", "--drafts", "1",
                         "--thinking-level", "low"], http6, keys)
        sup = [p for p in (base / "T01").iterdir() if p.name.startswith("draft1_superseded_")]
        new_meta = json.loads((base / "T01/draft1/meta.json").read_text())
        g6 = [c for c in http6.calls if c["url"].endswith(":generateContent")]
        check("settings change: old draft kept as superseded, redrafted with new level",
              code == 0 and len(sup) == 1 and (sup[0] / "meta.json").exists()
              and new_meta["settings"] == {"thinking_level": "low"} and len(g6) == 1
              and g6[0]["body"]["generationConfig"]["thinkingConfig"]["thinkingLevel"] == "low", out[-300:])
        http7 = FakeHTTP({})
        code, out = run(["--prompts-dir", str(pdir), "--out", str(root / "o7"), "--tasks", "T01", "--drafts", "1",
                         "--thinking-level", "default"], http7, keys)
        g7 = [c for c in http7.calls if c["url"].endswith(":generateContent")]
        check("thinking 'default': no generationConfig sent", code == 0 and set(g7[0]["body"]) == {"contents"})

        # ---------------- persistent 429 stops cleanly; partial draft kept aside on re-run
        script = {"T01": [reply(PROSE, rid="p1")] + [(429, {"error": {"code": 429}})] * 10}
        http3 = FakeHTTP(script)
        out3 = root / "out3"
        code, out = run(["--prompts-dir", str(pdir), "--out", str(out3), "--tasks", "T01", "--drafts", "1"], http3, keys)
        check("persistent 429: exit 1 with clear message", code == 1 and "Rate limit" in out, out)
        d1 = out3 / "gemini" / MODEL / "T01" / "draft1"
        check("persistent 429: no meta written for unfinished draft", not (d1 / "meta.json").exists())
        code, out = run(["--prompts-dir", str(pdir), "--out", str(out3), "--tasks", "T01", "--drafts", "1"], FakeHTTP({}), keys)
        aside = [p for p in d1.parent.iterdir() if p.name.startswith("draft1_interrupted_")]
        check("re-run after stop: earlier attempt kept aside, draft completed",
              code == 0 and len(aside) == 1 and (aside[0] / "attempt1_reply.txt").exists()
              and json.loads((d1 / "meta.json").read_text())["status"] == "accepted", out)

        # ---------------- HTTP error with key echoed in body is scrubbed
        http4 = FakeHTTP({"T01": [(400, {"error": {"message": f"bad key {FAKE_KEY}"}})]})
        code, out = run(["--prompts-dir", str(pdir), "--out", str(root / "o4"), "--tasks", "T01", "--drafts", "1"], http4, keys)
        check("HTTP 400: stops, key scrubbed from message", code == 1 and FAKE_KEY not in out and "***" in out, out)

        # ---------------- network error retried then stops
        http5 = FakeHTTP({"T01": [OSError("connection reset")] * 5})
        code, out = run(["--prompts-dir", str(pdir), "--out", str(root / "o5"), "--tasks", "T01", "--drafts", "1"], http5, keys)
        check("network errors: retried then clean stop", code == 1 and "Network error" in out, out)

        # ---------------- bad task name
        code, out = run(["--dry-run", "--prompts-dir", str(pdir), "--tasks", "T14"], FakeHTTP({}), keys)
        check("bad task name: stops", code == 1 and "Unknown task" in out, out)

    passed = sum(ok for _, ok in results)
    print(f"\n{passed}/{len(results)} checks passed")
    return 0 if passed == len(results) else 1


if __name__ == "__main__":
    sys.exit(main())
