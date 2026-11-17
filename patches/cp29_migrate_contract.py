#!/usr/bin/env python3
"""Migrate old native viewport checks only. Preserve CP28 DSP and original HTML."""
from pathlib import Path
import hashlib,json,re
r=Path('Albedo-Quintessential-Synth')
assert r.is_dir()
changed={}
old='98b82892a0a01845a564ecc379fa543c44bfc6d44a192c4cb2c31cb461ba6f08'
new='f894a63866e6cd37304c4f0720522d600aa65c041817250d2b54938679671289'
# Prior checks were authored for CP27's horizontal window, not CP29 portrait.
# Update viewport expectations, not the actual CP26/CP27 HTML resource.
for name in ('tests/check_cp15_resize_contract.py','tests/check_cp18_visual_contract.py',
             'tests/native_viewport_parity.cpp','tests/cp27_webview_contract.py'):
 p=r/name
 if not p.exists():continue
 s=p.read_text(encoding='utf-8')
 t=s.replace('kDefaultEditorWidth=1440, kDefaultEditorHeight=864','kDefaultEditorWidth=640, kDefaultEditorHeight=960')
 t=t.replace('kDefaultEditorWidth == 1440','kDefaultEditorWidth == 640')
 t=t.replace('kDefaultEditorHeight == 864','kDefaultEditorHeight == 960')
 t=t.replace('kDefaultEditorWidth==1440','kDefaultEditorWidth==640')
 t=t.replace('kDefaultEditorHeight==864','kDefaultEditorHeight==960')
 t=t.replace('aspect=5.0/3.0','aspect=2.0/3.0')
 t=t.replace('1440x864','640x960')
 # Preserve HTML's authored 5:3 layout while migrating C++ native size tests.
 if t!=s:
  p.write_text(t,encoding='utf-8',newline='\n')
  changed[name]=hashlib.sha256(p.read_bytes()).hexdigest()
# CP29: the old live Win32 repaint test assumed a horizontal editor and
# always-visible FX. Select the portrait Effects page before touching FX.
qa=r/'tests/cp19_partial_redraw_win.cpp'
if qa.exists():
 old=qa.read_text(encoding='utf-8')
 newer=old.replace('0,0,900,640,nullptr','0,0,700,1040,nullptr')
 newer=newer.replace('ui.attach(parent,900,640)','ui.attach(parent,640,960)')
 newer=newer.replace('parameters[5000]=53.0;ui.updateParam(5000,53.0);',
  'SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(480,700));UpdateWindow(w);\n parameters[5000]=53.0;ui.updateParam(5000,53.0);')
 newer=newer.replace('NativeViewport viewport(900,640);\n const int dragX=viewport.toX(1236+93),dragY=viewport.toY(1010+8);',
  'const int dragX=149,dragY=761;')
 newer=newer.replace('const LPARAM pos=MAKELPARAM(viewport.toX(point[0]),viewport.toY(point[1]));',
  'const LPARAM pos=MAKELPARAM(point[0],point[1]);')
 newer=newer.replace('ui.resize(1000,711)','ui.resize(700,1050)')
 if newer!=old:
  qa.write_text(newer,encoding='utf-8',newline='\n')
  changed['tests/cp19_partial_redraw_win.cpp']=hashlib.sha256(qa.read_bytes()).hexdigest()
for jpath in r.rglob('*.json'):
 try:s=jpath.read_text(encoding='utf-8')
 except UnicodeDecodeError:continue
 t=s.replace(old,new)
 if t!=s:jpath.write_text(t,encoding='utf-8')
mf=r/'CP19_SOURCE_MANIFEST.json'
m=json.loads(mf.read_text(encoding='utf-8'))
m['changed_source_sha256'].update(changed)
m['cp29_native_portrait_contract']='Existing native viewport contracts migrated to 640x960 width:height 2:3; HTML assets untouched.'
mf.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
assert (r/'include/vitriol_native_layout.hpp').read_text(encoding='utf-8').count('kDefaultEditorWidth=640, kDefaultEditorHeight=960')==1
assert (r/'CMakeLists.txt').read_text(encoding='utf-8').count('ALBEDO_CP29_NATIVE_ONLY')>=2
print('CP29 contract migration verified', changed)