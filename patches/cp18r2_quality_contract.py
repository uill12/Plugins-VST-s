#!/usr/bin/env python3
"""CP18R2 Windows real-time quality matrix regression."""
import hashlib,json
from pathlib import Path
r=Path("Albedo-Quintessential-Synth")
p=r/"tests/cp14_multirate_filter_qa.cpp"
raw=p.read_bytes()
old_hash="050eeafddc0a01a886815ded5fa711509d66a8d7df6dfb12ebece60fa9f42fa0"
if hashlib.sha256(raw).hexdigest()!=old_hash:raise SystemExit("Unknown original quality QA revision")
txt=raw.decode("utf-8")
old="{{22050,8},{32000,8},{44100,4},{48000,4},{88200,2},{96000,2},{176400,1},{192000,1},{352800,1},{384000,1}}"
new="{{22050,4},{32000,4},{44100,2},{48000,2},{88200,1},{96000,1},{176400,1},{192000,1},{352800,1},{384000,1}}"
if txt.count(old)!=1:raise SystemExit("Could not find rate matrix")
txt=txt.replace(old,new).replace("// Default quality holds internal sample rate near 176–192 kHz for standard DAW rates.","// CP18R2 AUTO targets 96 kHz internal at 48 kHz and relies on native rates above 88.2 kHz. Higher manual quality remains supported.")
p.write_text(txt,encoding="utf-8",newline="\n")
sha=hashlib.sha256(p.read_bytes()).hexdigest()
if sha!="72736737e2489f699411933897ff90e4d8f0f7bc9ca0d829cffa81a9c41fa90b":raise SystemExit("Rate QA hash mismatch")
manifest=r/"CP18_SOURCE_MANIFEST.json"
m=json.loads(manifest.read_text(encoding="utf-8"));m["changed_source_sha256"][str(p.relative_to(r)).replace("\\","/")]=sha
manifest.write_text(json.dumps(m,indent=2,ensure_ascii=False)+"\n",encoding="utf-8",newline="\n")
print("CP18R2 AUTO quality QA and SHA-256 updated")
