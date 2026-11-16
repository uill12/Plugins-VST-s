#!/usr/bin/env python3
"""CP19R5-R3: make the Win32 dirty-rectangle test use a visible host HWND."""
from pathlib import Path
import hashlib,json
root=Path("Albedo-Quintessential-Synth")
p=root/"tests/cp19_partial_redraw_win.cpp"
s=p.read_text(encoding="utf-8")
old=" if(!ui.attach(parent,900,640))return 2;\n auto w=ui.hwnd();UpdateWindow(w);"
new=""" if(!ui.attach(parent,900,640))return 2;
 // Windows suppresses child update regions when a host parent is hidden.
 // Verify dirty regions against the editor's actual visible HWND lifecycle.
 ShowWindow(parent,SW_SHOWNOACTIVATE);
 auto w=ui.hwnd();ShowWindow(w,SW_SHOW);UpdateWindow(parent);UpdateWindow(w);"""
if s.count(old)!=1:raise SystemExit("Unexpected CP19R5 Win32 test host setup")
p.write_text(s.replace(old,new),encoding="utf-8",newline="\n")
sha=hashlib.sha256(p.read_bytes()).hexdigest()
if sha!="141c4a4baca8be98bd67f0ec755c930bb52163919d70465292e117b653e30a75":
 raise SystemExit("Unexpected Win32 source test SHA256: "+sha)
mf=root/"CP19_SOURCE_MANIFEST.json"
m=json.loads(mf.read_text(encoding="utf-8"))
m["changed_source_sha256"]["tests/cp19_partial_redraw_win.cpp"]=sha
m["changes"]=str(m["changes"])+"; Win32 dirty-region test shows host HWND before GetUpdateRect."
mf.write_text(json.dumps(m,ensure_ascii=False,indent=2)+"\n",encoding="utf-8",newline="\n")
print("CP19R5-R3 real HWND dirty-rectangle fixture corrected")
