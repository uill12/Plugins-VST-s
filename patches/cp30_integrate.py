#!/usr/bin/env python3
"""Apply CP22 landscape/3:2 HTML to verified CP29 DirectComposition build.
Executed from GitHub repository root, after CP29/R1 source application.
All changes confined to editor HTML/resource path, host resize geometry, and QA.
"""
from pathlib import Path
import base64,gzip,hashlib,json,re
repo=Path.cwd();root=repo/'Albedo-Quintessential-Synth'
encoded=repo/'patches/cp30_cp22_3x2_html.b64'
canonical=repo/'ui/Albedo_CP30_CP22_Landscape_3x2.html'
# The editable HTML file is the live source-of-truth. A checksum-pinned
# compressed snapshot is retained for reproducible fallback/recovery.
if canonical.is_file():
 data=canonical.read_bytes().rstrip(b'\r\n')
else:
 data=gzip.decompress(base64.b64decode(encoded.read_text(encoding='ascii').strip(),validate=True))
sha=hashlib.sha256(data).hexdigest()
EXPECTED='681eaf3c20e84d8e84c45488c6e1b25a8301c8f1650911fe1f795536fa5c084d'
if not canonical.is_file():
 assert sha==EXPECTED,('CP30 fallback HTML SHA mismatch',sha)
s=data.decode('utf-8')
checks=['const BASE_W=1500,BASE_H=1000',"aspectRatio:'3:2'",'window.ALBEDO_NATIVE_CP27','window.ALBEDO_NATIVE_CP27?.onDial(k,v)','window.ALBEDO_NATIVE_CP27?.onPreset(name)','window.ALBEDO_NATIVE_CP27?.onExtraParam(id,api.state.cp04[id])','id="presetBrowser"','id="cp04Lab"','id="albedoFactoryCatalog"']
assert all(x in s for x in checks),[x for x in checks if x not in s]
assert 'window.ALBEDO_CP31_MOTION' in s, 'CP31 animated Arbor implementation absent'
assert s.count('<div class="brand"><svg')==1
assert s.count('id="presetBrowser"')==1
assert s.count('id="cp04Lab"')==1
html_path=root/'resources/Albedo_CP30_CP22_3x2_embedded.html'
html_path.write_bytes(data)
# Modify resource path in CMake and RC. Keep the exact same RCDATA resource ID (104).
cmake=root/'CMakeLists.txt';ct=cmake.read_text(encoding='utf-8')
old='set(ALBEDO_CP27_HTML_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/Albedo_CP27_5x3_embedded.html")'
new='set(ALBEDO_CP30_HTML_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/Albedo_CP30_CP22_3x2_embedded.html")'
assert ct.count(old)==1, 'Unexpected CMake HTML resource anchor'
cmake.write_text(ct.replace(old,new),encoding='utf-8',newline='\n')
rc=root/'resources/albedo_native_win.rc.in';rs=rc.read_text(encoding='utf-8')
assert rs.count('@ALBEDO_CP27_HTML_PATH@')==1
rc.write_text(rs.replace('@ALBEDO_CP27_HTML_PATH@','@ALBEDO_CP30_HTML_PATH@'),encoding='utf-8',newline='\n')
# Host starts 1440x960 (exact 3:2), resizing remains host-aware and pixel-native.
layout=root/'include/vitriol_native_layout.hpp';ls=layout.read_text(encoding='utf-8')
assert ls.count('kDefaultEditorWidth=1440, kDefaultEditorHeight=864')==1
assert ls.count('constexpr double aspect=5.0/3.0;')==1
ls=ls.replace('constexpr int kDefaultEditorWidth=1440, kDefaultEditorHeight=864',
'''#if defined(ALBEDO_CP27_WEBVIEW2)
constexpr int kDefaultEditorWidth=1440, kDefaultEditorHeight=960;
#else
// The original GDI-only standalone fixture keeps its historical dimensions.
constexpr int kDefaultEditorWidth=1440, kDefaultEditorHeight=864;
#endif''')
ls=ls.replace('constexpr double aspect=5.0/3.0;',
'''#if defined(ALBEDO_CP27_WEBVIEW2)
 constexpr double aspect=3.0/2.0;
#else
 constexpr double aspect=5.0/3.0;
#endif''')
layout.write_text(ls,encoding='utf-8',newline='\n')
# Historical contracts refer to the old CP27 resource; migrate expectations
# narrowly for the new primary renderer, never mutate audio/preset tests.
for rel in ['tests/cp27_webview_contract.py','tests/cp29_directcomposition_contract.py','tests/check_cp18_visual_contract.py','tests/check_cp15_resize_contract.py']:
 p=root/rel
 if not p.exists():continue
 t=p.read_text(encoding='utf-8'); orig=t
 t=t.replace('resources/Albedo_CP27_5x3_embedded.html','resources/Albedo_CP30_CP22_3x2_embedded.html')
 t=t.replace('const BASE_W=1650,BASE_H=990','const BASE_W=1500,BASE_H=1000')
 t=t.replace("aspectRatio:'5:3'","aspectRatio:'3:2'")
 t=t.replace('kDefaultEditorWidth=1440, kDefaultEditorHeight=864','kDefaultEditorWidth=1440, kDefaultEditorHeight=960')
 t=t.replace('constexpr double aspect=5.0/3.0;','constexpr double aspect=3.0/2.0;')
 t=t.replace('@ALBEDO_CP27_HTML_PATH@','@ALBEDO_CP30_HTML_PATH@')
 # Legacy native QA may assert startup dimensions directly.
 t=t.replace('kDefaultEditorHeight==864','kDefaultEditorHeight==960')
 t=t.replace('kDefaultEditorHeight == 864','kDefaultEditorHeight == 960')
 if t!=orig:p.write_text(t,encoding='utf-8',newline='\n')
# Sync every affected source with the existing hash manifest; preserve untouched
# audio DSP/CLAP/VST3 resources and their exact original checksums.
mp=root/'CP19_SOURCE_MANIFEST.json';m=json.loads(mp.read_text(encoding='utf-8'))
changed=[]
for rel in ['resources/Albedo_CP30_CP22_3x2_embedded.html','CMakeLists.txt','resources/albedo_native_win.rc.in','include/vitriol_native_layout.hpp','tests/cp27_webview_contract.py','tests/cp29_directcomposition_contract.py','tests/check_cp18_visual_contract.py','tests/check_cp15_resize_contract.py','tests/native_viewport_parity.cpp']:
 p=root/rel
 if not p.exists():continue
 actual=hashlib.sha256(p.read_bytes()).hexdigest()
 # only register existing known source, plus embedded HTML & CMake/RC modifications
 m['changed_source_sha256'][rel]=actual
 changed.append((rel,actual))
m['cp30_landscape_directcomposition']='CP31 readable CP22 3:2 resource, Arbor particle movement on visible editor; 64 factory, CP28 DSP unchanged.'
mp.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
# Old manifest references to the audited CP29 layout digest may exist in
# CP12/CP18 other JSON files. Update that one known hash only.
old_layout='98b82892a0a01845a564ecc379fa543c44bfc6d44a192c4cb2c31cb461ba6f08'; new_layout=hashlib.sha256(layout.read_bytes()).hexdigest()
for p in root.rglob('*.json'):
 if p==mp:continue
 try:txt=p.read_text(encoding='utf-8')
 except UnicodeError:continue
 if old_layout in txt:
  p.write_text(txt.replace(old_layout,new_layout),encoding='utf-8',newline='\n')
print('CP30 installed CP22+3:2 resource',len(data),'bytes',sha)
print('CP30 migrated source files:',len(changed))
for rel,h in changed:print(rel,h)
