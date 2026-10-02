"""IDP Module 3 parser v0.1: CloudTrail logs -> one call sequence per session.
Usage:  python parse.py            (reads logs/, sessions.csv, sessions_run1_VOID.csv)
Writes: sequences.csv (one row per event, for later feature extraction)"""
import csv, gzip, json
from collections import Counter
from pathlib import Path

# 1. Load every CloudTrail record from all .json.gz files under logs/
records = []
for f in Path("logs").rglob("*.json.gz"):
    with gzip.open(f, "rt", encoding="utf-8") as fh:
        records.extend(json.load(fh).get("Records", []))
records.sort(key=lambda r: r.get("eventTime", ""))
print(f"{len(records)} events loaded")

# 2. Session mapping lives OUTSIDE the log (A8): UUID -> script, run label
sessions = {}
for fname, run in (("sessions.csv", "run2"), ("sessions_run1_VOID.csv", "run1_VOID")):
    if Path(fname).exists():
        with open(fname, newline="", encoding="utf-8") as fh:
            for row in csv.DictReader(fh):
                sessions[row["session"].strip()] = {"script": row["script"], "run": run}

# 3. Session start = AssumeRole whose roleSessionName is one of our UUIDs
key_to_sess, start_event = {}, {}
for r in records:
    if r.get("eventName") != "AssumeRole":
        continue
    name = (r.get("requestParameters") or {}).get("roleSessionName")
    if name in sessions:
        creds = (r.get("responseElements") or {}).get("credentials") or {}
        if creds.get("accessKeyId"):
            key_to_sess[creds["accessKeyId"]] = name
        start_event[name] = r

# 4. Session cut: every later event made with the issued temporary key
seq = {s: [] for s in sessions}
for r in records:
    if r.get("eventName") == "AssumeRole":
        continue
    uid = r.get("userIdentity") or {}
    s = key_to_sess.get(uid.get("accessKeyId"))
    if s is None:  # fallback: session name is the last part of the assumed-role ARN
        arn = uid.get("arn", "")
        tail = arn.rsplit("/", 1)[-1] if "assumed-role/" in arn else None
        s = tail if tail in sessions else None
    if s:
        seq[s].append(r)

# 5-7. Print sequences + summaries, and write sequences.csv
out = open("sequences.csv", "w", newline="", encoding="utf-8")
w = csv.writer(out)
w.writerow(["session", "script", "run", "idx", "eventTime", "eventSource", "eventName",
            "errorCode", "bucket", "same_second_as_prev"])
for s, meta in sessions.items():
    evs = seq[s]
    st = start_event.get(s)
    print(f"\n=== SESSION {s[:8]} ({meta['script']}, {meta['run']})  events: {len(evs)}")
    if st is None:
        print("  !! no AssumeRole found yet (logs not delivered? re-sync later)")
    else:
        ui = st.get("userIdentity") or {}
        print(f"  AssumeRole by {ui.get('type')} {ui.get('arn','')}")
        print(f"  AssumeRole userAgent: {st.get('userAgent')}")
    prev_t = None
    for i, r in enumerate(evs):
        rp = r.get("requestParameters") or {}
        bucket = rp.get("bucketName", "")
        err = r.get("errorCode", "-")
        same = r["eventTime"] == prev_t
        extra = f"bucket={bucket}" if bucket else json.dumps(rp)[:70]
        print(f"  {r['eventTime'][11:19]}{'*' if same else ' '} {r.get('eventSource',''):24} "
              f"{r.get('eventName',''):26} {err:22} {extra}")
        w.writerow([s, meta["script"], meta["run"], i, r["eventTime"], r.get("eventSource"),
                    r.get("eventName"), err, bucket, same])
        prev_t = r["eventTime"]
    errs = Counter(r.get("errorCode") for r in evs if r.get("errorCode"))
    print(f"  errors: {dict(errs) or 'none'}")
    for ua in sorted({r.get("userAgent", "") for r in evs}):
        print(f"  userAgent: {ua}")
out.close()
print("\n* = same second as previous event (intra-second ordering not guaranteed, v3 s21)")
print("Wrote sequences.csv")
