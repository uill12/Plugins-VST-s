#!/usr/bin/env python3
"""CP10 preserve CP09 rollback, create Windows dev package and source archive."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json,shutil
from playwright.sync_api import sync_playwright
R=Path(__file__).resolve().parents[1];O=Path('/mnt/data');N='Nigredo_Opus_Septem_CP10'
with sync_playwright() as pw:
 b=pw.chromium.launch(headless=True,executable_path='/usr/bin/chromium',args=['--no-sandbox','--disable-dev-shm-usage'])
 for mode in ('Audio','MIDI'):
  p=b.new_page(viewport={'width':1600,'height':1000},reduced_motion='reduce')
  p.set_content((R/'ui/Nigredo_CP10_Standalone.html').read_text(),wait_until='load')
  if mode=='MIDI':p.locator('#midiMode').click()
  p.screenshot(path=str(O/f'{N}_{mode}_Preview.png'),animations='disabled')
  p.close()
 b.close()
shutil.copy2(R/'ui/Nigredo_CP10_Standalone.html',O/f'{N}.html')
files=[]
for kind in ('Audio','MIDI'):
 for ext in ('clap','vst3'):
  f=R/'distrib'/f'Nigredo_Opus_Septem_{kind}_FX_Windows_x64.{ext}'
  base=f'Nigredo_Opus_Septem_{kind}_FX.{ext}'
  dest=f'Windows/CLAP/{base}' if ext=='clap' else f'Windows/VST3/{base}/Contents/x86_64-win/{base}'
  files.append((f,dest,ext))
for ext in ('clap','vst3'):
 with ZipFile(O/f'{N}_{ext.upper()}_Windows_x64.zip','w',ZIP_DEFLATED,compresslevel=5) as z:
  for f,d,k in files:
   if k==ext:z.write(f,d.replace('Windows/CLAP/','').replace('Windows/VST3/',''))
with ZipFile(O/f'{N}_Windows_x64_Complete.zip','w',ZIP_DEFLATED,compresslevel=5) as z:
 for f,d,_ in files:z.write(f,d)
 for f in sorted((R/'release').glob('*CP10*')):z.write(f,f.name)
 z.write(R/'ui/Nigredo_CP10_Standalone.html',f'HTML/{N}.html')
with ZipFile(O/f'{N}_Editable_Source.zip','w',ZIP_DEFLATED,compresslevel=5) as z:
 for folder in ('src','scripts','tests','ui','assets','docs','winlibs'):
  for f in sorted((R/folder).rglob('*')):
   if f.is_file() and '__pycache__' not in f.parts:z.write(f,str(f.relative_to(R)))
 for f in sorted((R/'release').glob('*CP10*')):z.write(f,str(f.relative_to(R)))
report=[]
for f in sorted(O.glob(N+'*')):
 if f.suffix=='.zip':
  with ZipFile(f) as z:assert z.testzip() is None and len(z.namelist())>1
 report.append({'filename':f.name,'bytes':f.stat().st_size,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()})
(R/'release/CP10_PACKAGE_SHA256.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
