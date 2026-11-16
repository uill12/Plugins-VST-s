#!/usr/bin/env python3
"""CP08 release with strict Win32 native scene footer + PE audit."""
from pathlib import Path
import zipfile,hashlib,shutil,struct,subprocess,json
r=Path(__file__).resolve().parents[1]
out=Path('/mnt/data')
name='Nigredo_Opus_Septem_CP08'
manifest=[]
for kind in ('Audio','MIDI'):
 for ext in ('clap','vst3'):
  f=r/'distrib'/f'Nigredo_Opus_Septem_{kind}_FX_Windows_x64.{ext}'
  d=f.read_bytes()
  assert d[:2]==b'MZ' and d[-120:-116]==b'NIG7'
  assert struct.unpack_from('<I',d,len(d)-116)[0]==7
  for i in range(7):
   off,ln=struct.unpack_from('<QQ',d,len(d)-112+i*16)
   assert d[off:off+8]==b'\x89PNG\r\n\x1a\n'
   assert d[off+16:off+24]==struct.pack('>II',1600,1000)
   assert 0<ln<=6000000 and off+ln<=len(d)-120
  assert b'WebView2' not in d
  manifest.append({'filename':f.name,'sha256':hashlib.sha256(d).hexdigest(),'bytes':len(d),'scenes':7})
# Single-format packages
for ext in ('clap','vst3'):
 with zipfile.ZipFile(out/f'{name}_{ext.upper()}_Windows_x64.zip','w',zipfile.ZIP_DEFLATED,compresslevel=5) as z:
  for kind in ('Audio','MIDI'):
   f=r/'distrib'/f'Nigredo_Opus_Septem_{kind}_FX_Windows_x64.{ext}'
   dest=f'Nigredo_Opus_Septem_{kind}_FX.{ext}'
   z.write(f, dest if ext=='clap' else f'{dest}/Contents/x86_64-win/{dest}')
# Complete package
with zipfile.ZipFile(out/f'{name}_Windows_x64_Complete.zip','w',zipfile.ZIP_DEFLATED,compresslevel=5) as z:
 for kind in ('Audio','MIDI'):
  for ext in ('clap','vst3'):
   f=r/'distrib'/f'Nigredo_Opus_Septem_{kind}_FX_Windows_x64.{ext}'
   dest=f'Nigredo_Opus_Septem_{kind}_FX.{ext}'
   arc=f'Windows/CLAP/{dest}' if ext=='clap' else f'Windows/VST3/{dest}/Contents/x86_64-win/{dest}'
   z.write(f,arc)
 z.write(r/'release/Install_Nigredo_CP08.ps1','Install_Nigredo_CP08.ps1')
 z.write(r/'release/README_INSTALL_CP08.txt','README_INSTALL_CP08.txt')
 z.write(r/'release/CP08_NATIVE_GUI_QA.md','CP08_NATIVE_GUI_QA.md')
 z.write(r/'release/CP08_SHA256.json','CP08_SHA256.json')
 z.write(r/'ui/Nigredo_CP07_Standalone.html','HTML/Nigredo_Opus_Septem_CP08.html')
# Editable source, including exact asset and build scripts. No redundant Windows binaries.
with zipfile.ZipFile(out/f'{name}_Editable_Source.zip','w',zipfile.ZIP_DEFLATED,compresslevel=5) as z:
 for folder in ('src','scripts','tests','ui','assets/scenes','winlibs'):
  for f in sorted((r/folder).rglob('*')):
   if not f.is_file() or '__pycache__' in str(f) or f.name=='WebHtml.h':continue
   z.write(f,str(f.relative_to(r)))
 for f in ('README_INSTALL_CP08.txt','CP08_NATIVE_GUI_QA.md','CP08_SHA256.json','Install_Nigredo_CP08.ps1'):
  z.write(r/'release'/f, 'release/'+f)
# The browser GUI still reflects CP07's visual arrangement; CP08 changes the native method.
shutil.copy2(r/'ui/Nigredo_CP07_Standalone.html',out/f'{name}.html')
shutil.copy2(r/'assets/scenes/audio_3.png',out/f'{name}_Audio_Preview.png')
shutil.copy2(r/'assets/scenes/midi_3.png',out/f'{name}_MIDI_Preview.png')
for f in sorted(out.glob(f'{name}*')):
 print(f.name,round(f.stat().st_size/1048576,2),'MiB')
