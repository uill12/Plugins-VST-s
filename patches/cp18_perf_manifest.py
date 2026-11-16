#!/usr/bin/env python3
"""RGNES CP18 performance artifact source signatures, fail-closed."""
from pathlib import Path
import json,hashlib
root=Path("Albedo-Quintessential-Synth")
mfile=root/"CP18_SOURCE_MANIFEST.json"
m=json.loads(mfile.read_text(encoding="utf-8"))
expected={
 "include/vitriol_dsp.hpp":"ec06786ce3ca35ea3ea00e1585c827075c697ac1f542d375160f6f37229f978a",
 "src/vitriol_dsp.cpp":"ff8dfd22248ab6f8663d0fd0881ca65de39a5da09dfbe342ec1c9b6cf5ac3d17",
 "tests/cp18_chords_realtime.cpp":"7a8c4c322766b02d5af43c2215b966422c12c78db1f6905d3a9c5ab181531301"}
for name,h in expected.items():
 p=root/name
 actual=hashlib.sha256(p.read_bytes()).hexdigest()
 if actual!=h:raise SystemExit(f"CP18 realtime source integrity mismatch: {name}: {actual}")
 m["changed_source_sha256"][name]=h
m["realtime_budget_note"]="CP18 8-voice 4-engine realtime QA requires <=0.85x wall-time ratio at all 48/88.2/96/192k rates"
mfile.write_text(json.dumps(m,indent=2,ensure_ascii=False)+"\n",encoding="utf-8",newline="\n")
print("CP18: strict realtime QA source manifest updated (headroom >=15%)")
