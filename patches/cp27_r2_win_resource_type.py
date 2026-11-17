#!/usr/bin/env python3
"""CP27-R2: force Unicode MAKEINTRESOURCE for Win32 RT_RCDATA."""
from pathlib import Path
import hashlib,json
r=Path("Albedo-Quintessential-Synth")
p=r/"src/albedo_webview_win.cpp"
s=p.read_text(encoding="utf-8")
old="FindResourceW(module,MAKEINTRESOURCEW(104),RT_RCDATA)"
new="FindResourceW(module,MAKEINTRESOURCEW(104),MAKEINTRESOURCEW(10))"
assert s.count(old)==1,"WebView2 resource lookup anchor changed"
p.write_text(s.replace(old,new),encoding="utf-8",newline="\n")
h=hashlib.sha256(p.read_bytes()).hexdigest()
assert h=="248d894a0c97f5bafdb9661dbd79428cb64ec4583b44af07e3ae4f1a0a077f86",h
t=r/"tests/cp27_webview_contract.py"
src=t.read_text(encoding="utf-8")
assert src.count("'"+old+"'")==1,"Resource test anchor changed"
t.write_text(src.replace("'"+old+"'","'"+new+"'"),encoding="utf-8",newline="\n")
ht=hashlib.sha256(t.read_bytes()).hexdigest()
assert ht=="3ec565e608f35ed6b35555e44db8f67f85035ddd33e49fcf776f52bb348e1aab",ht
m=r/"CP19_SOURCE_MANIFEST.json"
j=json.loads(m.read_text(encoding="utf-8"))
j["changed_source_sha256"]["src/albedo_webview_win.cpp"]=h
j["changed_source_sha256"]["tests/cp27_webview_contract.py"]=ht
j["cp27_msvc_resource_fix"]="FindResourceW must use Unicode resource type 10, rather than ANSI RT_RCDATA."
m.write_text(json.dumps(j,ensure_ascii=False,indent=2)+"\n",encoding="utf-8",newline="\n")
print("CP27-R2 explicit Unicode RT_RCDATA resource and SHA256 QA passed")
