#!/usr/bin/env python3
"""RGNES: evidence-constrained, local, non-networked development-feedback ledger.

This tool does not harvest ChatGPT conversations, retrain models, trigger CI,
modify source, or upload files. It accepts already-authorized sanitized events.
"""
import argparse
import json
import os
import re
import sys
import tempfile
from collections import defaultdict
from pathlib import Path

SCHEMA_VERSION = 1
REQUIRED = {"event_id", "project", "rule_id", "area", "source_kind", "status", "observed_at", "symptom", "source_ref"}
OPTIONAL = {"corrective_action", "acceptance_test", "source_revision", "target_format", "environment", "severity"}
KINDS = {"user_feedback", "assistant_history", "build_log", "source_audit", "offline_test", "host_test", "user_confirmation"}
STATUSES = {"reported", "reproduced", "fix_attempted", "offline_test_passed", "host_test_passed", "user_confirmed", "regressed", "superseded", "blocked"}
PASS = {"offline_test_passed", "host_test_passed", "user_confirmed"}
EMAIL = re.compile(r"[\w.+-]+@[\w.-]+\.[a-z]{2,}", re.I)
SECRET = re.compile(r"(?:gh[pousr]_[A-Za-z0-9]{12,}|github_pat_[A-Za-z0-9_]{12,}|(?:api[_-]?key|password|token)\s*[=:]\s*\S+)", re.I)
RULE = re.compile(r"^[a-z][a-z0-9_.-]{2,79}$")
DATE = re.compile(r"^\d{4}-\d\d-\d\d$")


def validate(event):
    if not isinstance(event, dict):
        raise ValueError("Event must be a JSON object")
    unknown = set(event) - (REQUIRED | OPTIONAL)
    missing = REQUIRED - set(event)
    if missing or unknown:
        raise ValueError(f"Missing/extra event keys: missing={sorted(missing)}, extra={sorted(unknown)}")
    for k, value in event.items():
        if not isinstance(value, str) or not value.strip() or len(value) > 650:
            raise ValueError(f"Event field {k} requires 1..650 nonblank characters")
        if EMAIL.search(value) or SECRET.search(value):
            raise ValueError("Possible personal/contact or credential data; sanitize before recording")
        if "\n" in value or "\r" in value:
            raise ValueError("Only short single-line engineering summaries may be recorded")
    if not RULE.fullmatch(event["rule_id"]):
        raise ValueError("rule_id must be a stable lowercase engineering identifier")
    if event["source_kind"] not in KINDS or event["status"] not in STATUSES:
        raise ValueError("Invalid source_kind or status")
    if not DATE.fullmatch(event["observed_at"]):
        raise ValueError("observed_at must be ISO YYYY-MM-DD")
    if event["status"] in PASS:
        expected = {"offline_test_passed": "offline_test", "host_test_passed": "host_test", "user_confirmed": "user_confirmation"}[event["status"]]
        if event["source_kind"] != expected or not event.get("acceptance_test") or not event.get("source_revision"):
            raise ValueError("Passing evidence requires a matching source_kind, acceptance_test and source_revision")
    if event["status"] == "regressed" and event["source_kind"] not in {"user_feedback", "offline_test", "host_test", "build_log"}:
        raise ValueError("Regression needs a relevant user or test signal")
    return {k: event[k].strip() for k in sorted(event)}


def load_ledger(path):
    p = Path(path)
    if not p.exists():
        return {"schema_version": SCHEMA_VERSION, "events": []}
    data = json.loads(p.read_text(encoding="utf-8"))
    if data.get("schema_version") != SCHEMA_VERSION or not isinstance(data.get("events"), list):
        raise ValueError("Invalid ledger schema")
    for event in data["events"]:
        validate(event)
    return data


def ingest(ledger, events):
    ids = {event["event_id"]: event for event in ledger["events"]}
    appended = 0
    for raw in events:
        event = validate(raw)
        old = ids.get(event["event_id"])
        if old is not None:
            if old != event:
                raise ValueError("Conflicting reused event_id: " + event["event_id"])
            continue
        ledger["events"].append(event)
        ids[event["event_id"]] = event
        appended += 1
    return appended


def report(ledger):
    groups = defaultdict(list)
    for pos, event in enumerate(ledger["events"]):
        groups[(event["project"], event["rule_id"])].append((event["observed_at"], pos, event))
    rows = []
    for (project, rule_id), history in sorted(groups.items()):
        seq = [x[2] for x in sorted(history)]
        latest = seq[-1]
        count = sum(e["status"] in {"reported", "reproduced", "regressed", "blocked"} for e in seq)
        passed = [e for e in seq if e["status"] in PASS]
        current_pass = bool(passed) and latest["status"] in PASS
        rows.append({
            "project": project,
            "rule_id": rule_id,
            "observations": len(seq),
            "failure_signals": count,
            "recurring": count >= 2,
            "latest_status": latest["status"],
            "current_fix_tested": current_pass,
            "needs_regression": not current_pass,
            "last_observed_at": latest["observed_at"],
            "last_source_ref": latest["source_ref"],
            "recommended_test": next((e["acceptance_test"] for e in reversed(seq) if e.get("acceptance_test")), None),
            "last_action": next((e["corrective_action"] for e in reversed(seq) if e.get("corrective_action")), None),
        })
    return {"schema_version": SCHEMA_VERSION, "total_events": len(ledger["events"]), "patterns": rows}


def read_events(path):
    txt = Path(path).read_text(encoding="utf-8")
    if path.endswith(".jsonl"):
        return [json.loads(line) for line in txt.splitlines() if line.strip()]
    parsed = json.loads(txt)
    return parsed if isinstance(parsed, list) else [parsed]


def atomic_save(path, ledger):
    p = Path(path)
    p.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=p.parent, prefix=".rgnes-learning-", suffix=".json")
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as out:
            json.dump(ledger, out, ensure_ascii=False, indent=2, sort_keys=True)
            out.write("\n")
        os.replace(tmp, p)
    finally:
        if os.path.exists(tmp):
            os.unlink(tmp)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    rec = commands.add_parser("record", help="ingest user/repro/test events into an explicit JSON ledger")
    rec.add_argument("--ledger", required=True)
    rec.add_argument("--event", required=True, help="sanitized event .json or .jsonl")
    rec.add_argument("--dry-run", action="store_true")
    rep = commands.add_parser("report", help="emit deterministic JSON learning report")
    rep.add_argument("--ledger", required=True)
    args = parser.parse_args(argv)
    try:
        ledger = load_ledger(args.ledger)
        if args.command == "record":
            count = ingest(ledger, read_events(args.event))
            if not args.dry_run:
                atomic_save(args.ledger, ledger)
            print(json.dumps({"added": count, "dry_run": args.dry_run, **report(ledger)}, indent=2, ensure_ascii=False))
        else:
            print(json.dumps(report(ledger), indent=2, ensure_ascii=False))
        return 0
    except (ValueError, OSError, json.JSONDecodeError) as exc:
        print("RGNES learning validation error: " + str(exc), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
