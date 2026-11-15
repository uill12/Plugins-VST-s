#!/usr/bin/env python3
"""CP17 source QA hotfix: deterministic voice RNG after audio-thread-safe reset."""
import hashlib,json
from pathlib import Path
r=Path("Albedo-Quintessential-Synth")
p=r/"src/vitriol_dsp.cpp"
s=p.read_text(encoding="utf-8")
old="v.cellValue=0;v.caBits=1u<<16;v.age=0;"
new="v.cellValue=0;v.caBits=1u<<16;v.age=0;v.random=0x18953u;"
if s.count(old)!=1:raise SystemExit("Cannot locate deterministic RNG reset anchor")
p.write_text(s.replace(old,new),encoding="utf-8",newline="\n")
mfile=r/"CP17_SOURCE_MANIFEST.json"
m=json.loads(mfile.read_text(encoding="utf-8"))
h=hashlib.sha256(p.read_bytes()).hexdigest()
if h!="d9311814c485a8817ec0706aa98063a44533f2adae696fe273b89949da99707d":raise SystemExit("Unexpected CP17 MIDI RNG hotfix SHA")
m["changed_source_sha256"]["src/vitriol_dsp.cpp"]=h
mfile.write_text(json.dumps(m,ensure_ascii=False,indent=2)+"\n",encoding="utf-8",newline="\n")
print("RGNES CP17: deterministic MIDI note reset preserved")
