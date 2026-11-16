#!/usr/bin/env python3
"""CP08 native GUI static integrity, compatibility, paint-safe asset audit.
Runtime host behaviour still requires an actual Windows Bitwig test.
"""
from pathlib import Path
import hashlib,struct,subprocess,sys
from PIL import Image
from io import BytesIO
r=Path(__file__).resolve().parents[1]
s=(r/'src/WinGui.h').read_text()
for token in ('struct NCreateStructA','offsetof(NCreateStructA,createParams)==0',
              'offsetof(NCreateStructA,style)==48','offsetof(NCreateStructA,exStyle)==72',
              'RegisterClassExA','WM_NCCREATE_','WM_NCDESTROY_','GetWindowLongPtrA',
              'SetViewportOrgEx','clientToLogical','nu32(footer+4)!=7'):
 assert token in s, f'missing native correction: {token}'
assert 'GetWindowLongPtrA(hwnd,-4' not in s, 'old fragile STATIC subclass still active'
manifest=[]
for mode in ('Audio','MIDI'):
 for ext in ('clap','vst3'):
  f=r/'distrib'/f'Nigredo_Opus_Septem_{mode}_FX_Windows_x64.{ext}'
  data=f.read_bytes()
  assert data[:2]==b'MZ' and data[-120:-116]==b'NIG7'
  assert struct.unpack_from('<I',data,len(data)-116)[0]==7, 'scene-footer version mismatch'
  assert b'WebView2' not in data and b'WebView2Loader.dll' not in data
  assert (f'NigredoOpusSeptemCP08_{"CLAP" if ext=="clap" else "VST3"}_{mode}_PaintFixed'.encode() in data)
  assert b'RegisterClassExA' in data
  for i in range(7):
   off,ln=struct.unpack_from('<QQ',data,len(data)-112+i*16)
   assert 0<off<len(data)-120 and 100<ln<6000000
   png=data[off:off+ln];assert png.startswith(b'\x89PNG\r\n\x1a\n')
   img=Image.open(BytesIO(png));img.verify()
   img=Image.open(BytesIO(png));assert img.size==(1600,1000)
  entry= 'clap_entry' if ext=='clap' else 'GetPluginFactory'
  imports=subprocess.check_output(['llvm-objdump','-p',str(f)],text=True)
  assert 'DLL Name: USER32.dll' in imports and 'DLL Name: KERNEL32.dll' in imports
  assert 'DLL Name: WebView2' not in imports and 'DLL Name: libcef' not in imports
  sha=hashlib.sha256(data).hexdigest()
  manifest.append({'file':f.name,'bytes':len(data),'sha256':sha,'embedded_scenes':7,'scene_version':7})
  print('PASS',f.name,'scenes=7','native paint class','SHA256',sha[:16])
print('PASS: corrected Win64 class lifecycle, 14 images/mode, 16:10 aspect, no WebView2, 4 PE binaries')
import json
(r/'release/CP08_SHA256.json').write_text(json.dumps(manifest,indent=2)+'\n')
