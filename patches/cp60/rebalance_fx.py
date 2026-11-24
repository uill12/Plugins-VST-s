#!/usr/bin/env python3
"""CP60 role-aware FX balance after CP59 Concept Tree.
Keep 192 program IDs, names, routing, macros and source materials intact;
retune only the 63 CP59 WaveEdit patches for a longer/dense stereo FDN.
"""
import argparse,hashlib,json
from pathlib import Path
parser=argparse.ArgumentParser()
parser.add_argument('--metadata',required=True,type=Path)
parser.add_argument('--report',required=True,type=Path)
args=parser.parse_args()
files=['material_factory_bank.json','recursive_forms_bank.json','morph_expansion_bank.json']
# Wet gain is role-aware: foundation dry and centered; pads/drones can hold space.
roles={
 'Bass':(.82,80),'Lead':(.90,100),'Pad':(.93,130),
 'Pluck':(.84,95),'Keys':(.91,105),'Drone':(.96,140),'Texture':(.94,135)
}
rows=[];count=0;preserved=0
for name in files:
 file=args.metadata/name
 bank=json.loads(file.read_text(encoding='utf-8'))
 assert len(bank)==64 and len({p['index'] for p in bank})==64
 for p in bank:
  ov=p['overrides'];assert isinstance(ov,dict)
  if 'concept_tree' not in p:
   preserved+=1
   continue
  role=p['role'];assert role in roles
  scale,limit=roles[role]
  oldWet=float(ov['5004']);oldSize=float(ov['5011'])
  # Round to the exact native stepped controls only when stepped; these are continuous.
  newWet=round(max(0.,min(100.,oldWet*scale)),3)
  newSize=round(min(oldSize,limit),3)
  ov['5004']=newWet
  ov['5011']=newSize
  # Do not alter master gain, macro duration, FX enable, filter, arp, pitch or MIDI.
  p['fx_role_profile']={
   'version':'CP60', 'reverb_arch':'8-line stereo FDN',
   'function':role,'wet_fraction':scale,'size_ceiling':limit,
   'output_trim_unchanged':True,'source_and_routing_unchanged':True
  }
  rows.append(dict(index=p['index'],name=p['name'],role=role,
                   reverb_wet_before=oldWet,reverb_wet_after=newWet,
                   size_before=oldSize,size_after=newSize))
  count+=1
 file.write_text(json.dumps(bank,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
assert count==63 and preserved==129,(count,preserved)
assert len({p['index'] for p in rows})==63
assert any(p['reverb_wet_after']<p['reverb_wet_before'] for p in rows)
report=dict(checkpoint='CP60',method='Concept Tree role-aware 8-line FDN send balance',
            total_programs=192,revoiced_fx=63,untouched=129,
            original_pcm24_and_waveedit=True,parameter_ids_unchanged=True,
            max_wet_cut_pp=max(p['reverb_wet_before']-p['reverb_wet_after'] for p in rows),
            audio_render_qa='required',programs=rows)
args.report.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('CP60 FX preset balance: 63 revoiced, 129 untouched; original synth level and sources retained')