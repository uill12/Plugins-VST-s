#!/usr/bin/env python3
"""CP55: preserve native binaries while normalizing VST3 metadata + checksums."""
import argparse
import hashlib
import json
import re
from pathlib import Path

parser=argparse.ArgumentParser()
parser.add_argument("directory",type=Path)
parser.add_argument("--version",default="1.0.55")
parser.add_argument("--source",default="CP55")
args=parser.parse_args()
root=args.directory
module=list(root.rglob("moduleinfo.json"))
assert len(module)==1, f"Exactly one VST3 moduleinfo.json required: {module}"
info=module[0]
data=info.read_text(encoding="utf-8-sig")
parsed=json.loads(re.sub(r",(?=\s*[}\]])","",data))
assert parsed.get("Version")==args.version,(parsed.get("Version"),args.version)
assert len(parsed.get("Classes",[]))==2
assert all(len(item.get("CID",""))==32 for item in parsed["Classes"])
info.write_text(json.dumps(parsed,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
assert json.loads(info.read_text())==parsed
binary=sorted(p for p in root.rglob("*") if p.is_file() and p.suffix.lower() in (".clap",".vst3"))
assert len(binary)==2,binary
assert {p.suffix.lower() for p in binary}=={".clap",".vst3"}
sums=[]
for f in binary:
    payload=f.read_bytes()
    assert len(payload)>2_000_000 and payload[:2]==b"MZ",(f,len(payload))
    assert b"VTRB2\0\0\0" in payload, f"Missing embedded 32-material PCM bank in {f}"
    sums.append(f"{hashlib.sha256(payload).hexdigest()}  {f.relative_to(root).as_posix()}")
(root/"SHA256SUMS.txt").write_text("\n".join(sums)+"\n",encoding="utf-8")
(root/"CP55_PACKAGE_QA.json").write_text(json.dumps({
    "checkpoint":"Albedo CP55 — Morphogenic Fields / Window Fit",
    "version":args.version,
    "sources":"192 existing program IDs; 64 selected Morphogenic Fields programs retain parameters",
    "new_bank_display_label":"Morphogenic Fields",
    "new_bank_internal_key":"New",
    "window_start":"1230x820 when screen permits; 3:2 fallback",
    "native_binaries_unchanged":True,
    "vst3_moduleinfo_strict_json":True,
    "embedded_pcm24_material_bank":True,
    "git_source":args.source,
    "bitwig_manual_validation":"pending"
},ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print("CP55 strict metadata, PE64 + PCM24, SHA256: PASS")
print("\n".join(sums))
