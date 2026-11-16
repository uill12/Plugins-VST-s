#!/usr/bin/env python3
"""Create immutable CP09 Windows release, per-format bundles and source."""
from pathlib import Path
import hashlib,zipfile,shutil,json
R=Path(__file__).resolve().parents[1]; O=Path('/mnt/data');NAME='Nigredo_Opus_Septem_CP09'
previews=[('audio','Audio'),('midi','MIDI')]
# HTML default previews. These are screenshots, not post-composited drawings.
from playwright.sync_api import sync_playwright
with sync_playwright() as pw:
 b=pw.chromium.launch(headless=True,executable_path='/usr/bin/chromium',args=['--no-sandbox','--disable-dev-shm-usage'])
 for kind,mode in previews:
  p=b.new_page(viewport={'width':1600,'height':1000},device_scale_factor=1,reduced_motion='reduce')
  p.set_content((R/'ui/Nigredo_CP09_Standalone.html').read_text(),wait_until='load')
  if kind=='midi':p.locator('#midiMode').click()
  p.wait_for_timeout(120)
  p.screenshot(path=str(O/f'{NAME}_{mode}_Preview.png'),full_page=False,animations='disabled')
  p.close()
 b.close()
shutil.copy2(R/'ui/Nigredo_CP09_Standalone.html',O/f'{NAME}.html')
files=[]
for kind in ('Audio','MIDI'):
 for ext in ('clap','vst3'):
  bin=R/'distrib'/f'Nigredo_Opus_Septem_{kind}_FX_Windows_x64.{ext}'
  assert bin.is_file()
  stem=f'Nigredo_Opus_Septem_{kind}_FX.{ext}'
  dest=(f'Windows/CLAP/{stem}' if ext=='clap' else f'Windows/VST3/{stem}/Contents/x86_64-win/{stem}')
  files.append((bin,dest,kind,ext))
for ext in ('clap','vst3'):
 with zipfile.ZipFile(O/f'{NAME}_{ext.upper()}_Windows_x64.zip','w',zipfile.ZIP_DEFLATED,compresslevel=5) as z:
  for bin,dest,kind,e in files:
   if e==ext:
    d=dest.partition('/')[2]
    z.write(bin,d)
with zipfile.ZipFile(O/f'{NAME}_Windows_x64_Complete.zip','w',zipfile.ZIP_DEFLATED,compresslevel=5) as z:
 for bin,dest,kind,ext in files:z.write(bin,dest)
 for f in (R/'release').iterdir():
  if f.name.startswith('CP09') or f.name.startswith('README_INSTALL_CP09') or f.name.startswith('Install_Nigredo_CP09'):
   z.write(f,f.name)
 z.write(R/'ui/Nigredo_CP09_Standalone.html',f'HTML/{NAME}.html')
with zipfile.ZipFile(O/f'{NAME}_Editable_Source.zip','w',zipfile.ZIP_DEFLATED,compresslevel=5) as z:
 for folder in ('src','scripts','tests','ui','assets','docs','winlibs'):
  for f in sorted((R/folder).rglob('*')):
   if f.is_file() and '__pycache__' not in f.parts and not f.name.startswith('CP09_audio_'):
    z.write(f,str(f.relative_to(R)))
 for f in sorted((R/'release').glob('*CP09*')):
  z.write(f,str(f.relative_to(R)))
# Checksums and decode integrity.
release=[]
for f in sorted(O.glob(f'{NAME}*')):
 if f.suffix=='.zip':
  with zipfile.ZipFile(f) as z: assert z.testzip() is None; assert len(z.namelist())>1
 release.append({'artifact':f.name,'bytes':f.stat().st_size,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()})
print(json.dumps(release,indent=2))
(R/'release/CP09_PACKAGE_SHA256.json').write_text(json.dumps(release,indent=2)+'\n')
