#!/usr/bin/env python3
"""RGNES CP18: launcher, archive and embedded release metadata must match."""
from pathlib import Path
import hashlib,json
r=Path('Albedo-Quintessential-Synth')
b=r/'Build_Windows_x64.ps1';s=b.read_text(encoding='utf-8')
old="checkpoint='CP17';version='1.0.4-cp17'"
if s.count(old)!=1:raise SystemExit('Unexpected CP18 Windows release metadata')
s=s.replace(old,"checkpoint='CP18';version='1.0.5-cp18'")
old="source='CP17 bounded MIDI note lifecycle, note IDs, retrigger, sustain, voice-pressure protection; CP16 GUI and CP14 assets preserved; Steinberg 3.8.1';"
if s.count(old)!=1:raise SystemExit('CP17 original source string missing')
s=s.replace(old,"source='CP18 chord realtime recovery, CLAP NOTE_END, original Albedo typography and 900x640 editor; Steinberg 3.8.1';")
s=s.replace("previous_release='CP16 verified Windows binary distribution'","previous_release='CP17 verified Windows binary distribution'")
b.write_text(s,encoding="utf-8",newline="\n")
l=r/'Launch_Windows_Build.ps1';u=l.read_text(encoding='utf-8')
old='Albedo_Quintessential_Synth_Windows_x64_CP17.zip'
if u.count(old)!=1:raise SystemExit('Unexpected CP18 launcher archive name')
l.write_text(u.replace(old,'Albedo_Quintessential_Synth_Windows_x64_CP18.zip'),encoding='utf-8',newline='\n')
expected={'Build_Windows_x64.ps1':'067fb2cc402b3456b88fcdb5b556ea181d60df5630878206f49136a05516b71f','Launch_Windows_Build.ps1':'89d8c96372b96cffd31105fea1cb8e37c38257e4b8b1c22ae242884b59993f54'}
mfile=r/'CP18_SOURCE_MANIFEST.json';m=json.loads(mfile.read_text(encoding='utf-8'))
for filename,digest in expected.items():
 actual=hashlib.sha256((r/filename).read_bytes()).hexdigest()
 if actual!=digest:raise SystemExit('CP18 release source SHA mismatch: '+filename+' '+actual)
 m['changed_source_sha256'][filename]=digest
mfile.write_text(json.dumps(m,indent=2,ensure_ascii=False)+"\n",encoding='utf-8',newline='\n')
print('CP18 Windows launcher, metadata and source checksums updated')
