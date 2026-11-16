#!/usr/bin/env python3
"""RGNES G4/G5 offline-only native integrity and resource-offset verification."""
from pathlib import Path
import hashlib,struct,subprocess,json
from PIL import Image
from io import BytesIO
R=Path(__file__).resolve().parents[1]
s=(R/'src/WinGui.h').read_text()
for tok in ('offsetof(NCreateStructA,createParams)==0','offsetof(NCreateStructA,style)==48',
            'offsetof(NCreateStructA,exStyle)==72','RegisterClassExA','WM_NCCREATE_',
            'WM_NCDESTROY_','SetViewportOrgEx','clientToLogical','NGUI_POWER','NGUI_TOGGLE'):
 assert tok in s,tok
assert 'GetWindowLongPtrA(hwnd,-4' not in s
manifest=[]
for mode in ('Audio','MIDI'):
 for ext in ('clap','vst3'):
  f=R/'distrib'/f'Nigredo_Opus_Septem_{mode}_FX_Windows_x64.{ext}'
  data=f.read_bytes()
  assert data[:2]==b'MZ' and data[-120:-116]==b'NIG7'
  assert struct.unpack_from('<I',data,len(data)-116)[0]==7
  assert b'WebView2' not in data and b'WebView2Loader.dll' not in data
  assert (f'NigredoOpusSeptemCP09_{"CLAP" if ext=="clap" else "VST3"}_{mode}_PaintFixed'.encode() in data)
  for i in range(7):
   off,ln=struct.unpack_from('<QQ',data,len(data)-112+i*16)
   assert ln>1000 and off+ln<=len(data)-120
   png=data[off:off+ln]; im=Image.open(BytesIO(png));im.verify()
   im=Image.open(BytesIO(png));assert im.size==(1600,1000)
  imports=subprocess.check_output(['llvm-objdump','-p',str(f)],text=True)
  assert 'DLL Name: USER32.dll' in imports and 'DLL Name: KERNEL32.dll' in imports
  assert 'DLL Name: WebView2' not in imports and 'DLL Name: libcef' not in imports
  assert ('clap_entry' if ext=='clap' else 'GetPluginFactory') in imports
  assert (b'0.9.0-cp09' in data) if ext=='clap' else b'GetPluginFactory' in data
  manifest.append({'binary':f.name,'sha256':hashlib.sha256(data).hexdigest(),'size_bytes':len(data),'format':ext,'mode':mode,'scene_count':7,'validation':'cross-compiled; static PE/resources pass; not Windows-host tested'})
  print('PASS',f.name,'scenes=7, no WebView2, exports/imports checked, checksum',manifest[-1]['sha256'][:15])
(R/'release').mkdir(exist_ok=True)
(R/'release/CP09_SHA256.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('PASS G4/G5 offline: 4 Windows x64 binaries with dynamic control assets and correct native window classes')
