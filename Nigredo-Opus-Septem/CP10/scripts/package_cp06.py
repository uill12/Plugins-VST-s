from pathlib import Path
import zipfile,shutil,json,struct,hashlib
r=Path(__file__).resolve().parents[1]
out=Path('/mnt/data')
base='Nigredo_Opus_Septem_CP06'
# Verify every Windows plugin before packaging.
manifest={}
for kind in ('Audio','MIDI'):
 for ext in ('clap','vst3'):
  f=r/'distrib'/f'Nigredo_Opus_Septem_{kind}_FX_Windows_x64.{ext}'
  buf=f.read_bytes(); assert buf[-120:-116]==b'NIG6'
  assert struct.unpack('<I',buf[-116:-112])[0]==6
  for i in range(7):
   start=len(buf)-112+i*16; off,ln=struct.unpack('<QQ',buf[start:start+16])
   assert buf[off:off+8]==b'\x89PNG\r\n\x1a\n'
   assert buf[off+16:off+24]==struct.pack('>II',1320,1300)
   assert off+ln<=len(buf)-120
  assert b'WebView2Loader.dll' not in buf and b'WebView2' not in buf
  manifest[f.name]={'bytes':len(buf),'sha256':hashlib.sha256(buf).hexdigest()}
# Format-specific ZIPs, native binaries only
for ext,label in [('clap','CLAP'),('vst3','VST3')]:
 path=out/f'{base}_{label}_Windows_x64.zip'
 with zipfile.ZipFile(path,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=4) as z:
  for kind in ('Audio','MIDI'):
   f=r/'distrib'/f'Nigredo_Opus_Septem_{kind}_FX_Windows_x64.{ext}'
   if ext=='clap':arc=f.name.replace('_Windows_x64','')
   else:
    name=f'Nigredo_Opus_Septem_{kind}_FX.vst3'
    arc=f'{name}/Contents/x86_64-win/{name}'
   z.write(f,arc)
# Complete package install tree
package=out/f'{base}_Windows_x64_Complete.zip'
with zipfile.ZipFile(package,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=4) as z:
 for kind in ('Audio','MIDI'):
  for ext in ('clap','vst3'):
   f=r/'distrib'/f'Nigredo_Opus_Septem_{kind}_FX_Windows_x64.{ext}'
   new=f'Nigredo_Opus_Septem_{kind}_FX.{ext}'
   if ext=='clap':arc='Windows/CLAP/'+new
   else:arc='Windows/VST3/'+new+'/Contents/x86_64-win/'+new
   z.write(f,arc)
 for f in (r/'release').glob('*CP06*'):
  z.write(f,f.name)
 z.write(r/'ui/Nigredo_CP06_Standalone.html','HTML/Nigredo_CP06_Standalone.html')
# Fully editable source (including assets and scripts)
source=out/f'{base}_Editable_Source.zip'
with zipfile.ZipFile(source,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=4) as z:
 for folder in ('src','scripts','tests','ui','assets/scenes','winlibs','release'):
  for f in (r/folder).rglob('*'):
   if not f.is_file():continue
   if f.name=='WebHtml.h' or '__pycache__' in str(f):continue
   z.write(f,str(f.relative_to(r)))
 (r/'release/CP06_SHA256.json').write_text(json.dumps(manifest,indent=2)+'\n')
 z.write(r/'release/CP06_SHA256.json','release/CP06_SHA256.json')
# user-facing previews + standalone HTML
shutil.copy2(r/'ui/Nigredo_CP06_Standalone.html',out/f'{base}.html')
shutil.copy2(r/'assets/scenes/audio_3.png',out/f'{base}_Audio_Preview.png')
shutil.copy2(r/'assets/scenes/midi_3.png',out/f'{base}_MIDI_Preview.png')
for p in [package,source,out/f'{base}_CLAP_Windows_x64.zip',out/f'{base}_VST3_Windows_x64.zip']:
 print(p.name,round(p.stat().st_size/1048576,2),'MiB')
