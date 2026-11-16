#!/usr/bin/env python3
"""CP19R5 Win64 Unicode cursor compatibility. Safe, deterministic source hotfix."""
from pathlib import Path
import hashlib,json
root=Path("Albedo-Quintessential-Synth")
p=root/"src/albedo_preset_browser_win.cpp"
s=p.read_text(encoding="utf-8")
old="wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);"
new="wc.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(OCR_NORMAL));"
if s.count(old)!=1:raise SystemExit("Unexpected CP19 preset dialog source (Unicode cursor)")
p.write_text(s.replace(old,new),encoding="utf-8",newline="\n")
sha=hashlib.sha256(p.read_bytes()).hexdigest()
if sha!="e2f890129d9112e30673fa6b09b4e3d2619b2d02159f6232e00204efc091f0da":
 raise SystemExit("CP19R5 Unicode resource SHA mismatch "+sha)
mf=root/"CP19_SOURCE_MANIFEST.json";j=json.loads(mf.read_text(encoding="utf-8"))
j["changed_source_sha256"]["src/albedo_preset_browser_win.cpp"]=sha
mf.write_text(json.dumps(j,indent=2,ensure_ascii=False)+"\n",encoding="utf-8",newline="\n")
print("CP19R5: Unicode cursor identifier corrected and source manifest verified")
