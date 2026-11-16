#!/usr/bin/env python3
"""RGNES CP18 MSVC C3538 fix: preserve semantic values, separate auto deduction."""
from pathlib import Path
import json,hashlib
root=Path("Albedo-Quintessential-Synth")
p=root/"src/vitriol_dsp.cpp"
s=p.read_text(encoding="utf-8")
before="const auto group=v.group,note=v.note,ch=v.ch,id=v.noteId;"
after="const auto group=v.group; const auto note=v.note; const auto ch=v.ch; const auto id=v.noteId;"
if s.count(before)!=3:raise SystemExit("Unexpected CP18 source: C3538 repair anchors missing")
p.write_text(s.replace(before,after),encoding="utf-8",newline="\n")
h=hashlib.sha256(p.read_bytes()).hexdigest()
if h!="207dfde71bb14ae11524ff0c149eb39d4fe3df2ff247f36999e746544259302a":raise SystemExit("CP18 repaired DSP hash mismatch: "+h)
mf=root/"CP18_SOURCE_MANIFEST.json";m=json.loads(mf.read_text(encoding="utf-8"))
m["changed_source_sha256"]["src/vitriol_dsp.cpp"]=h
mf.write_text(json.dumps(m,indent=2,ensure_ascii=False)+"\n",encoding="utf-8",newline="\n")
print("CP18: C3538 MSVC note-end fix passes exact SHA")
