#!/usr/bin/env python3
"""CP27-R1: Microsoft WebView2 COM header ordering for Windows MSVC."""
from pathlib import Path
import hashlib,json
root=Path("Albedo-Quintessential-Synth")
p=root/"src/albedo_webview_win.cpp"
s=p.read_text(encoding="utf-8")
old="#include <WebView2.h>\n#include <wrl.h>\n"
new="#include <unknwn.h>\n#include <wrl.h>\n#include <WebView2.h>\n"
assert s.count(old)==1,"Unexpected WebView2 header anchor"
out=s.replace(old,new)
p.write_text(out,encoding="utf-8",newline="\n")
sha=hashlib.sha256(p.read_bytes()).hexdigest()
assert sha=="028588305ee77f6f4681d1f48dec3e7fd4da08e09b3d17b0a1ebfa5b129d6880",(sha,"CP27-R1 source hash")
mf=root/"CP19_SOURCE_MANIFEST.json"
j=json.loads(mf.read_text(encoding="utf-8"))
j["changed_source_sha256"]["src/albedo_webview_win.cpp"]=sha
j["cp27_msvc_header_fix"]="COM base/WRL declarations must precede WebView2-generated IDL headers."
mf.write_text(json.dumps(j,ensure_ascii=False,indent=2)+"\n",encoding="utf-8",newline="\n")
print("CP27-R1 header ordering and manifest SHA256 verified")
