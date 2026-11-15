#!/usr/bin/env python3
"""RGNES CP16 Windows-only macro dial click geometry after 64%-startup resize."""
from pathlib import Path
import json,hashlib
root=Path('Albedo-Quintessential-Synth')
p=root/'tests/win32_gui_smoke.cpp'
old='send(WM_LBUTTONDOWN,382,420,MK_LBUTTON);\n send(WM_MOUSEMOVE,382,370,MK_LBUTTON);\n send(WM_LBUTTONUP,382,370);'
new='send(WM_LBUTTONDOWN,306,336,MK_LBUTTON);\n send(WM_MOUSEMOVE,306,296,MK_LBUTTON);\n send(WM_LBUTTONUP,306,296);'
s=p.read_text(encoding='utf-8')
if s.count(old)!=1: raise SystemExit('Unexpected native resize smoke test source')
p.write_text(s.replace(old,new),encoding='utf-8',newline='\n')
manifest=root/'CP16_SOURCE_MANIFEST.json'
m=json.loads(manifest.read_text(encoding='utf-8'))
h=hashlib.sha256(p.read_bytes()).hexdigest()
if h!='d864c2ee391df31d938b4b3fab49ec3067f24a691bfc59418426ac9bfd7828f9':raise SystemExit('Unexpected CP16 resized dial QA hash: '+h)
m['changed_source_sha256']['tests/win32_gui_smoke.cpp']=h
manifest.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
print('RGNES CP16 resized macro dial hit point verified at 306,336')
