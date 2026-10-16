#!/usr/bin/env python3
"""Drafting run: send the frozen v1.1 drafting prompts to a model and save every reply unedited.

Rules implemented (Chat #8 decisions S5 to S7):
  S5  every (model, task) is drafted twice in separate, independent calls. Draft 1 is the study
      script; draft 2 only measures within-model variance.
  S6  provider defaults (no system prompt, no tools, no web search), with ONE exception decided
      in Chat #8: the thinking level is set to "medium" for every vendor (Claude and OpenAI already
      default to medium; Gemini 3.5 Flash-Lite defaults to minimal and is raised to match).
  S7  extraction: the single Python code block in the reply, or the whole reply if it is pure
      Python. A draft passes when a script was extracted, it compiles, every module it imports
      exists in this environment, and it imports boto3 (agent and script both use boto3).
      If attempt 1 does not pass, ONE fresh call is made. If attempt 2 also does not pass, the
      draft is recorded as "failed" and has no study script. Scripts are never executed, so no
      AWS calls are possible.

Nothing is edited: the raw API response, the reply text and the extracted script are all saved.
The API key is never printed or written to disk.

Usage (WSL bash, venv active):
  python draft_scripts.py --dry-run                 # checks prompts and key, no network
  python draft_scripts.py --list-models             # read-only: lists models the key can use
  python draft_scripts.py --tasks T01 --drafts 1    # one call, smoke test
  python draft_scripts.py                           # full run (13 tasks x 2 drafts)
Re-running is safe: finished drafts are skipped.
Exit codes: 0 finished, 1 stopped (nothing unsafe happened; re-run continues where it stopped).
"""

import argparse
import ast
import datetime as dt
import hashlib
import importlib.util
import json
import os
import platform
import re
import stat
import sys
import time
import urllib.error
import urllib.request
import uuid
from pathlib import Path

SCRIPT_VERSION = "1.1"
HERE = Path(__file__).resolve().parent
DEFAULT_PROMPTS = HERE / "drafting_prompts_v1.1"
DEFAULT_OUT = Path.home() / "idp" / "drafts_out"
KEYS_FILE = Path.home() / ".config" / "idp" / "keys.env"
TASKS = [f"T{i:02d}" for i in range(1, 14)]
PROMPT_NAME = "drafting_prompt_v1.1_{task}.txt"

MIN_SECONDS_BETWEEN_CALLS = 5.0   # free tier allows 15 requests per minute; this keeps us at 12
RATE_LIMIT_WAIT = 65              # seconds to wait after HTTP 429
MAX_RATE_LIMIT_RETRIES = 3
SERVER_ERROR_WAIT = 15
MAX_SERVER_RETRIES = 2
HTTP_TIMEOUT = 300

AWS_REGION_RE = re.compile(r"\b(?:us|eu|ap|sa|ca|me|af|il|mx)-(?:north|south|east|west|central|northeast|southeast|northwest|southwest)-\d\b")


class Stop(Exception):
    """Raised to stop the run cleanly with a message for the user."""


def utc_now():
    return dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


# --------------------------------------------------------------------------- key handling

def load_key(env_name, keys_file=KEYS_FILE):
    """Return (key, source). Never prints the key."""
    value = os.environ.get(env_name)
    if value:
        return value.strip(), "environment"
    if keys_file.exists():
        mode = keys_file.stat().st_mode
        if mode & (stat.S_IRWXG | stat.S_IRWXO):
            print(f"WARNING: {keys_file} is readable by others. Run: chmod 600 {keys_file}")
        for line in keys_file.read_text(encoding="utf-8").splitlines():
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            name, _, val = line.partition("=")
            if name.strip().removeprefix("export ").strip() == env_name:
                val = val.strip().strip('"').strip("'")
                if val:
                    return val, str(keys_file)
    raise Stop(f"No {env_name} found in the environment or in {keys_file}.")


def scrub(text, secret):
    """Safety net: remove the key from any text before it is printed or saved."""
    if secret and isinstance(text, str):
        return text.replace(secret, "***")
    return text


# --------------------------------------------------------------------------- prompts

def load_prompts(prompts_dir, tasks):
    """Read prompt files and check every one against SHA256SUMS. Stops on any mismatch."""
    sums_file = prompts_dir / "SHA256SUMS"
    if not sums_file.exists():
        raise Stop(f"Missing {sums_file}")
    expected = {}
    for line in sums_file.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        parts = line.split()
        if len(parts) != 2 or not re.fullmatch(r"[0-9a-f]{64}", parts[0]):
            raise Stop(f"Unreadable line in SHA256SUMS: {line!r}")
        expected[parts[1].lstrip("*")] = parts[0]
    all_names = {PROMPT_NAME.format(task=t) for t in TASKS}
    missing_from_sums = all_names - set(expected)
    if missing_from_sums:
        raise Stop(f"SHA256SUMS does not list: {sorted(missing_from_sums)}")
    prompts = {}
    for task in tasks:
        name = PROMPT_NAME.format(task=task)
        path = prompts_dir / name
        if not path.exists():
            raise Stop(f"Missing prompt file {path}")
        data = path.read_bytes()
        digest = sha256_bytes(data)
        if digest != expected[name]:
            raise Stop(f"Hash mismatch for {name}: file {digest[:12]}..., SHA256SUMS {expected[name][:12]}...")
        prompts[task] = {"file": name, "sha256": digest, "text": data.decode("utf-8")}
    return prompts


# --------------------------------------------------------------------------- extraction and acceptance

FENCE_RE = re.compile(r"^```[ \t]*([A-Za-z0-9_+.-]*)[^\n]*\n(.*?)^```[ \t]*$", re.S | re.M)
PY_LANGS = {"python", "py", "python3"}


def compiles(source):
    try:
        compile(source, "<draft>", "exec")
        return True, None
    except (SyntaxError, ValueError) as exc:
        return False, f"{type(exc).__name__}: {exc}"


def _is_real_code(source):
    """True if the source parses and contains at least one statement that is not a bare expression.
    Stops a one-word prose reply such as 'OK' (which is technically valid Python) counting as a script."""
    try:
        tree = ast.parse(source)
    except (SyntaxError, ValueError):
        return False
    return any(not isinstance(node, ast.Expr) for node in tree.body)


def extract_script(reply):
    """Return (code or None, method, reason). Pure function; S7 rule."""
    if reply is None or not reply.strip():
        return None, None, "empty reply"
    blocks = [(m.group(1).lower(), m.group(2)) for m in FENCE_RE.finditer(reply)]
    if blocks:
        python_blocks = [code for lang, code in blocks if lang in PY_LANGS]
        if len(python_blocks) > 1:
            return None, None, f"{len(python_blocks)} python blocks"
        if len(python_blocks) == 1:
            code, method = python_blocks[0], "single python block"
        else:
            unlabeled = [code for lang, code in blocks if lang == ""]
            if len(unlabeled) != 1:
                return None, None, (f"no single python block ({len(blocks)} fenced blocks: "
                                    f"{[lang or 'unlabeled' for lang, _ in blocks]})")
            code, method = unlabeled[0], "single unlabeled block"
        if not code.strip():
            return None, None, "the code block is empty"
        return code, method, None
    if _is_real_code(reply):
        return reply, "whole reply is python", None
    return None, None, "no fenced block and reply is not a python script"


def imported_modules(source):
    """Top-level module names imported anywhere in the script (static, nothing is executed)."""
    tree = ast.parse(source)
    names, relative = set(), 0
    for node in ast.walk(tree):
        if isinstance(node, ast.Import):
            for alias in node.names:
                names.add(alias.name.split(".")[0])
        elif isinstance(node, ast.ImportFrom):
            if node.level and node.level > 0:
                relative += 1
            elif node.module:
                names.add(node.module.split(".")[0])
    return sorted(names), relative


def acceptance(code):
    """Pass/fail for an extracted script, with the reasons it failed (empty list = passed)."""
    ok, err = compiles(code)
    result = {"compiles": ok, "compile_error": err, "imports": [], "missing_imports": [],
              "relative_imports": 0, "imports_boto3": False, "reasons": [], "accepted": False}
    if not ok:
        result["reasons"] = [f"does not compile ({err})"]
        return result
    mods, relative = imported_modules(code)
    result["imports"] = mods
    result["relative_imports"] = relative
    result["missing_imports"] = [m for m in mods if importlib.util.find_spec(m) is None]
    result["imports_boto3"] = "boto3" in mods
    if result["missing_imports"]:
        result["reasons"].append(f"missing imports {result['missing_imports']}")
    if relative:
        result["reasons"].append(f"{relative} relative import(s)")
    if not result["imports_boto3"]:
        result["reasons"].append("does not import boto3")
    result["accepted"] = not result["reasons"]
    return result


def script_info(code):
    return {
        "lines": code.count("\n") + (0 if code.endswith("\n") else 1),
        "sha256": sha256_bytes(code.encode("utf-8")),
        "aws_regions_mentioned": sorted(set(AWS_REGION_RE.findall(code))),
        "has_main_guard": "__name__" in code and "__main__" in code,
    }


# --------------------------------------------------------------------------- HTTP

def default_http(method, url, headers, body):
    """Returns (status, parsed_json_or_text). Network errors raise OSError."""
    data = json.dumps(body).encode("utf-8") if body is not None else None
    req = urllib.request.Request(url, data=data, method=method, headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=HTTP_TIMEOUT) as resp:
            raw = resp.read().decode("utf-8")
            status = resp.status
    except urllib.error.HTTPError as exc:
        raw = exc.read().decode("utf-8", errors="replace")
        status = exc.code
    try:
        return status, json.loads(raw)
    except json.JSONDecodeError:
        return status, raw


# --------------------------------------------------------------------------- providers

class GeminiProvider:
    name = "gemini"
    key_env = "GEMINI_API_KEY"
    base = "https://generativelanguage.googleapis.com/v1beta"

    def __init__(self, key, http=default_http, sleep=time.sleep):
        self.key = key
        self.http = http
        self.sleep = sleep
        self._last_call = 0.0

    def _headers(self):
        return {"x-goog-api-key": self.key, "Content-Type": "application/json"}

    def _call(self, method, url, body=None):
        """One logical call with polite pacing and limited retries for 429/5xx/network errors."""
        rate_retries = server_retries = 0
        while True:
            wait = MIN_SECONDS_BETWEEN_CALLS - (time.monotonic() - self._last_call)
            if wait > 0:
                self.sleep(wait)
            self._last_call = time.monotonic()
            try:
                status, payload = self.http(method, url, self._headers(), body)
            except OSError as exc:
                if server_retries >= MAX_SERVER_RETRIES:
                    raise Stop(f"Network error after retries: {scrub(str(exc), self.key)}")
                server_retries += 1
                print(f"  network error, waiting {SERVER_ERROR_WAIT}s and retrying")
                self.sleep(SERVER_ERROR_WAIT)
                continue
            if status == 429:
                if rate_retries >= MAX_RATE_LIMIT_RETRIES:
                    raise Stop("Rate limit (HTTP 429) persists. Daily quota may be used up; re-run later to continue.")
                rate_retries += 1
                print(f"  rate limited (429), waiting {RATE_LIMIT_WAIT}s ({rate_retries}/{MAX_RATE_LIMIT_RETRIES})")
                self.sleep(RATE_LIMIT_WAIT)
                continue
            if status >= 500:
                if server_retries >= MAX_SERVER_RETRIES:
                    raise Stop(f"Server error {status} after retries: {scrub(str(payload)[:300], self.key)}")
                server_retries += 1
                print(f"  server error {status}, waiting {SERVER_ERROR_WAIT}s and retrying")
                self.sleep(SERVER_ERROR_WAIT)
                continue
            if status != 200:
                raise Stop(f"HTTP {status}: {scrub(str(payload)[:500], self.key)}")
            return payload

    def list_models(self):
        models, token = [], None
        while True:
            url = f"{self.base}/models?pageSize=1000" + (f"&pageToken={token}" if token else "")
            payload = self._call("GET", url)
            models.extend(payload.get("models", []))
            token = payload.get("nextPageToken")
            if not token:
                return models

    def check_model(self, model_id):
        models = self.list_models()
        by_name = {m.get("name"): m for m in models}
        info = by_name.get(f"models/{model_id}")
        if info is None:
            close = sorted(n for n in by_name if "lite" in (n or "") or model_id.split("-")[0] in (n or ""))
            raise Stop(f"Model {model_id} is not offered to this key. Similar names: {close[:15]}")
        if "generateContent" not in info.get("supportedGenerationMethods", []):
            raise Stop(f"Model {model_id} does not support generateContent.")
        return {k: info.get(k) for k in ("name", "baseModelId", "version", "displayName",
                                         "inputTokenLimit", "outputTokenLimit", "thinking")}

    THINKING_LEVELS = ("minimal", "low", "medium", "high")

    @staticmethod
    def request_body(prompt_text, thinking_level=None):
        # S6: user message only. No system instruction or tools. The only setting is the thinking
        # level (generationConfig.thinkingConfig.thinkingLevel); None leaves the provider default.
        body = {"contents": [{"role": "user", "parts": [{"text": prompt_text}]}]}
        if thinking_level is not None:
            body["generationConfig"] = {"thinkingConfig": {"thinkingLevel": thinking_level}}
        return body

    def generate(self, model_id, prompt_text, thinking_level=None):
        body = self.request_body(prompt_text, thinking_level)
        payload = self._call("POST", f"{self.base}/models/{model_id}:generateContent", body)
        candidates = payload.get("candidates") or []
        text, finish = None, None
        if candidates:
            cand = candidates[0]
            finish = cand.get("finishReason")
            parts = (cand.get("content") or {}).get("parts") or []
            # Thought summaries are not returned by default; skip them defensively if present.
            texts = [p.get("text", "") for p in parts if "text" in p and not p.get("thought")]
            text = "".join(texts) if texts else None
        usage = payload.get("usageMetadata") or {}
        return {
            "raw": payload,
            "request_body_sha256": sha256_bytes(json.dumps(body, sort_keys=True).encode("utf-8")),
            "text": text,
            "finish_reason": finish,
            "response_id": payload.get("responseId"),
            "model_version": payload.get("modelVersion"),
            "prompt_feedback": payload.get("promptFeedback"),
            "usage": {
                "input_tokens": usage.get("promptTokenCount"),
                "output_tokens": usage.get("candidatesTokenCount"),
                "thinking_tokens": usage.get("thoughtsTokenCount"),
                "total_tokens": usage.get("totalTokenCount"),
            },
        }


PROVIDERS = {"gemini": GeminiProvider}
DEFAULT_MODELS = {"gemini": "gemini-3.5-flash-lite"}


# --------------------------------------------------------------------------- run

def write_json(path, obj, secret):
    path.write_text(scrub(json.dumps(obj, indent=2, ensure_ascii=False), secret) + "\n", encoding="utf-8")


def finished_draft(ddir, settings):
    """Return the saved meta if this draft is complete AND was made with the same settings."""
    meta_path = ddir / "meta.json"
    if not meta_path.exists():
        return None
    meta = json.loads(meta_path.read_text(encoding="utf-8"))
    if meta.get("complete") and meta.get("settings") == settings:
        return meta
    return None


def draft_one(provider, model_id, task, draft, prompt, out_dir, run_id, settings):
    """Make one draft: attempt 1, plus ONE fresh attempt if attempt 1 does not pass (S7)."""
    ddir = out_dir / task / f"draft{draft}"
    meta = finished_draft(ddir, settings)
    if meta is not None:
        return meta, True
    if ddir.exists() and any(ddir.iterdir()):
        # Earlier files exist: either a run stopped part-way, or the draft was made with different
        # settings. Never overwrite a saved response: move the folder aside, then start again.
        label = "superseded" if (ddir / "meta.json").exists() else "interrupted"
        aside = ddir.with_name(f"{ddir.name}_{label}_{utc_now().replace(':', '')}")
        ddir.rename(aside)
        print(f"    earlier files moved to {aside.name}")
    ddir.mkdir(parents=True, exist_ok=True)
    attempts, chosen = [], None
    for attempt in (1, 2):
        started = utc_now()
        result = provider.generate(model_id, prompt["text"], settings.get("thinking_level"))
        write_json(ddir / f"attempt{attempt}_response.json", result["raw"], provider.key)
        if result["text"] is not None:
            (ddir / f"attempt{attempt}_reply.txt").write_text(scrub(result["text"], provider.key), encoding="utf-8")
        code, method, reason = extract_script(result["text"])
        rec = {
            "attempt": attempt, "requested_at": started, "finished_at": utc_now(),
            "response_id": result["response_id"], "model_version": result["model_version"],
            "finish_reason": result["finish_reason"], "prompt_feedback": result["prompt_feedback"],
            "usage": result["usage"], "request_body_sha256": result["request_body_sha256"],
            "extracted": code is not None, "extraction_method": method,
        }
        if code is None:
            rec["passed"] = False
            rec["reasons"] = [f"no script extracted ({reason})"]
        else:
            (ddir / f"attempt{attempt}_script.py").write_text(code, encoding="utf-8")
            rec["script"] = script_info(code)
            rec["acceptance"] = acceptance(code)
            rec["passed"] = rec["acceptance"]["accepted"]
            rec["reasons"] = rec["acceptance"]["reasons"]
        attempts.append(rec)
        if rec["passed"]:
            chosen = (attempt, code)
            break
    meta = {
        "run_id": run_id, "script_version": SCRIPT_VERSION, "provider": provider.name,
        "model_requested": model_id, "task": task, "draft": draft, "settings": settings,
        "prompt_file": prompt["file"], "prompt_sha256": prompt["sha256"],
        "attempts": attempts, "complete": True,
    }
    if chosen is None:
        meta["status"] = "failed"
    else:
        attempt, code = chosen
        (ddir / "script.py").write_text(code, encoding="utf-8")
        meta["status"] = "accepted"
        meta["script_from_attempt"] = attempt
        meta["script_file"] = "script.py"
        meta["script"] = attempts[-1]["script"]
    write_json(ddir / "meta.json", meta, provider.key)
    return meta, False


def parse_list(value, allowed, label):
    items = [v.strip().upper() if label == "task" else v.strip() for v in value.split(",") if v.strip()]
    bad = [v for v in items if v not in allowed]
    if bad:
        raise Stop(f"Unknown {label}(s): {bad}. Allowed: {sorted(allowed)}")
    return items


def main(argv=None, http=default_http, sleep=time.sleep):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--provider", default="gemini", choices=sorted(PROVIDERS))
    ap.add_argument("--model", default=None, help="model ID (default per provider)")
    ap.add_argument("--prompts-dir", type=Path, default=DEFAULT_PROMPTS)
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT)
    ap.add_argument("--keys-file", type=Path, default=KEYS_FILE)
    ap.add_argument("--tasks", default=",".join(TASKS))
    ap.add_argument("--drafts", default="1,2")
    ap.add_argument("--thinking-level", default="medium",
                    choices=["default", "minimal", "low", "medium", "high"],
                    help="thinking level for every call (Chat #8: medium); 'default' sends no setting")
    mode = ap.add_mutually_exclusive_group()
    mode.add_argument("--dry-run", action="store_true", help="check prompts and key only; no network")
    mode.add_argument("--list-models", action="store_true", help="read-only: list models for this key")
    args = ap.parse_args(argv)

    try:
        tasks = parse_list(args.tasks, set(TASKS), "task")
        drafts = [int(d) for d in parse_list(args.drafts, {"1", "2"}, "draft")]
        provider_cls = PROVIDERS[args.provider]
        model_id = args.model or DEFAULT_MODELS[args.provider]
        prompts = load_prompts(args.prompts_dir, tasks)
        print(f"Prompts: {len(prompts)} file(s) match SHA256SUMS.")
        key, source = load_key(provider_cls.key_env, args.keys_file)
        print(f"API key: found ({provider_cls.key_env} from {source}; not shown).")
        settings = {"thinking_level": None if args.thinking_level == "default" else args.thinking_level}
        print(f"Settings: thinking level {args.thinking_level}; no system prompt, tools or other settings.")
        plan = [(t, d) for d in drafts for t in tasks]   # all draft-1 calls first
        out_dir = args.out / args.provider / model_id
        if args.dry_run:
            done = sum(1 for t, d in plan if finished_draft(out_dir / t / f"draft{d}", settings))
            print(f"Dry run: would draft {len(plan)} item(s) with {model_id} into {out_dir} "
                  f"({done} already done with these settings). No network calls made.")
            return 0
        provider = provider_cls(key, http=http, sleep=sleep)
        if args.list_models:
            for m in provider.list_models():
                if "generateContent" in m.get("supportedGenerationMethods", []):
                    print(f"  {m.get('name')}  ({m.get('displayName')})")
            return 0

        model_info = provider.check_model(model_id)
        print(f"Model: {model_info['name']} version {model_info.get('version')} is available.")
        out_dir.mkdir(parents=True, exist_ok=True)
        run_id = str(uuid.uuid4())
        run_info = {
            "run_id": run_id, "started_at": utc_now(), "script_version": SCRIPT_VERSION,
            "provider": args.provider, "model": model_info, "tasks": tasks, "drafts": drafts,
            "python": sys.version.split()[0], "platform": platform.platform(),
            "settings": settings,
            "notes": "no system prompt; no tools; no settings other than the thinking level",
        }
        write_json(out_dir / f"run_{run_info['started_at'].replace(':', '')}.json", run_info, key)

        rows = []
        for i, (task, draft) in enumerate(plan, 1):
            print(f"[{i}/{len(plan)}] {task} draft {draft} ...", flush=True)
            meta, skipped = draft_one(provider, model_id, task, draft, prompts[task], out_dir, run_id, settings)
            rows.append(meta)
            note = "already done, skipped" if skipped else meta["status"]
            if meta.get("script"):
                note += f", {meta['script']['lines']} lines"
            print(f"    {note}")
    except Stop as exc:
        print(f"STOPPED: {exc}")
        return 1

    print_summary(rows, out_dir)
    return 0


def print_summary(rows, out_dir):
    print("\nSUMMARY")
    print(f"{'task':<5} {'draft':<6} {'status':<9} {'lines':>5} {'tries':>5} {'in':>7} {'out':>7} {'think':>7}  regions")
    totals = {"input_tokens": 0, "output_tokens": 0, "thinking_tokens": 0}
    counts = {}
    for m in rows:
        lines = m.get("script", {}).get("lines", "-")
        regions = ",".join(m.get("script", {}).get("aws_regions_mentioned", [])) or "-"
        u = {k: sum((a["usage"].get(k) or 0) for a in m["attempts"]) for k in totals}
        for k in totals:
            totals[k] += u[k]
        counts[m["status"]] = counts.get(m["status"], 0) + 1
        print(f"{m['task']:<5} {m['draft']:<6} {m['status']:<9} {lines:>5} {len(m['attempts']):>5} "
              f"{u['input_tokens']:>7} {u['output_tokens']:>7} {u['thinking_tokens']:>7}  {regions}")
        for a in m["attempts"]:
            if not a.get("passed"):
                print(f"      attempt {a['attempt']} did not pass: {'; '.join(a.get('reasons', []))}")
        finishes = [a["finish_reason"] for a in m["attempts"]]
        if any(f != "STOP" for f in finishes):
            print(f"      note: finish reason(s) {finishes}")
    print(f"\nStatus counts: {counts}")
    print(f"Tokens: input {totals['input_tokens']}, output {totals['output_tokens']}, thinking {totals['thinking_tokens']}"
          " (thinking shows 0 when the API does not report it)")
    print(f"Saved under: {out_dir}")


if __name__ == "__main__":
    sys.exit(main())
