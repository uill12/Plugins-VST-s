#!/usr/bin/env python3
"""Append seven lossless PNG scenes with footer exactly as WinGui.h expects.
The Windows loader ignores appended PE overlay data; the native GUI loads by
GetModuleFileNameA + CreateFileA, like Rubedo's embedded GUI bitmap technique.
"""
from pathlib import Path
import sys,struct,hashlib
root=Path(__file__).resolve().parents[1]
for kind in ('Audio','MIDI'):
 files=[root/'assets/scenes'/f'{kind.lower()}_{i}.png' for i in range(7)]
 for ext in ('clap','vst3'):
  path=root/'distrib'/f'Nigredo_Opus_Septem_{kind}_FX_Windows_x64.{ext}'
  data=path.read_bytes()
  # Idempotent: only raw, unappended binaries are allowed.
  if data[-120:-116]==b'NIG7':raise SystemExit(f'Already appended: {path}')
  offsets=[]
  with path.open('ab') as out:
   for f in files:
    d=f.read_bytes();offsets.append((len(data),len(d)));out.write(d);data+=d
   footer=b'NIG7'+struct.pack('<I',7)+b''.join(struct.pack('<QQ',o,n) for o,n in offsets)
   assert len(footer)==120
   out.write(footer)
  print(path.name, path.stat().st_size,hashlib.sha256(path.read_bytes()).hexdigest()[:20])
