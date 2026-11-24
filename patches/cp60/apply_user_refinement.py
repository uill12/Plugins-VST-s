#!/usr/bin/env python3
"""Apply only the two DSP hunks of the user's CP59 refinement to CP59 WaveEdit.
No blind source replacement: each old hunk must appear exactly once. Input
CRLF is normalized in memory so this is deterministic on MSVC and Linux.
"""
from pathlib import Path
import argparse,re
ap=argparse.ArgumentParser()
ap.add_argument('--root',type=Path,required=True)
ap.add_argument('--patch',type=Path,default=Path('patches/cp60/user_refinement_dsp.patch'))
a=ap.parse_args()
patch=a.patch.read_text(encoding='utf-8')
sections=re.split(r'(?=^--- a/)',patch,flags=re.MULTILINE)
expected={'Albedo-Quintessential-Synth/src/vitriol_dsp.cpp':7,
          'Albedo-Quintessential-Synth/include/vitriol_dsp.hpp':1}
counts={}
for sec in sections:
 if not sec.startswith('--- a/'):continue
 lines=sec.splitlines(keepends=True)
 oldpath=lines[0].strip().removeprefix('--- a/')
 if oldpath not in expected:raise RuntimeError('Unrecognized DSP source: '+oldpath)
 rel=oldpath.split('/',1)[1]
 file=a.root/rel
 source=file.read_text(encoding='utf-8').replace('\r\n','\n')
 hunks=[];start=None
 for line in lines[2:]:
  if line.startswith('@@ '):
   if start is not None:hunks.append(start)
   start=[]
  elif start is not None and (line[0] in ' +-'):
   start.append(line)
  elif line.startswith('\\ No newline'):
   continue
  elif line.strip():raise RuntimeError('Unexpected patch line '+line[:55])
 if start is not None:hunks.append(start)
 if len(hunks)!=expected[oldpath]:raise RuntimeError(f'{rel}: expected {expected[oldpath]} hunks got {len(hunks)}')
 for i,h in enumerate(hunks,1):
  old=''.join(x[1:] for x in h if x.startswith((' ','-')))
  new=''.join(x[1:] for x in h if x.startswith((' ','+')))
  n=source.count(old)
  if n!=1:raise RuntimeError(f'{rel}, hunk {i}: source preimage occurrence={n}, refusing destructive merge')
  source=source.replace(old,new,1)
 file.write_text(source,encoding='utf-8',newline='\n')
 counts[rel]=len(hunks)
print('CP60 imported user FDN/ADAA/chorus/voice/string patch into CP59 WaveEdit:',counts)