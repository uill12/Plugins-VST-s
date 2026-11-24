#!/usr/bin/env python3
"""Concept Tree: generate 63 distinct CP59 source/morph variations in existing 192.
Run after verified CP51 selection and CP58R9 register refinement; preserve indices,
family, names, MIDI clock, key range and non-target programs. Never select by output
similarity alone; tree branches generate timbres by source/function/character.
"""
from pathlib import Path
from collections import Counter
import argparse,json,hashlib
p=argparse.ArgumentParser();p.add_argument('--metadata',type=Path,required=True);p.add_argument('--selection',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
selection=json.loads(a.selection.read_text(encoding='utf-8'))['selected'];assert len(selection)==24
names=['material_factory_bank.json','recursive_forms_bank.json','morph_expansion_bank.json']
roles=('Bass','Lead','Pad','Pluck','Keys','Drone','Texture')
rolePools={role:[d for d in selection if (d['role']==role or role=='Drone' and d['role']=='Pad') ]for role in roles}
rolePools['Pluck'] += [d for d in selection if d['role']=='Keys']
assert all(rolePools.values())
report=[]
for bankidx,name in enumerate(names):
 file=a.metadata/name;bank=json.loads(file.read_text(encoding='utf-8'));assert len(bank)==64
 used=set()
 for role in roles:
  targets=[x for x in bank if x.get('role')==role]
  for i,preset in enumerate(targets[:3]):
   # Distinct table selection by bank, role, timbral permutation; not a nearest-neighbor filter.
   pool=rolePools[role]
   table=pool[(bankidx*3+i*2+(preset['index']//8))%len(pool)]
   tID=32+table['id'];source=preset['overrides']
   assert isinstance(source,dict)
   # Preserve all other instruments/FX/CA/routing, and original program index.
   original={k:source.get(k) for k in ('1000','1010','1032','1042','1002','1034')}
   source['1000']=1;source['1010']=tID
   source['1002']=min(82,float(source.get('1002',72)))
   source['1006']=40+(i*22+bankidx*9)%54 # evolving frame position/brightness
   source['1007']=21+(i*11+bankidx*7)%39 # time-stable morph warp
   source['1008']=17+(i*13+bankidx*11)%38 # tonal saturation
   if role in ('Pad','Drone','Texture'):
    # Source B plays the pitch-safe 48-cycle morphed texture; high-note grains
    # remain consistent with the played MIDI pitch and no random detune.
    source['1032']=3;source['1042']=tID
    source['1034']=min(44,float(source.get('1034',38)))
    source['1039']=25+(i*10+bankidx*5)%42 # grain density, not pitch
    source['1041']=33+(i*17+bankidx*3)%44 # circular texture scan
   else:
    # Preserve existing source B, occasionally add a restrained grain layer.
    if role in ('Keys','Pluck') and i==1:
     source['1032']=3;source['1042']=tID;source['1034']=min(19,float(source.get('1034',32)))
   preset['concept_tree']={'material':'WaveEdit CC0','wave_table':table['file'],'resource_id':tID,'morph_sibling':selection[int(table['morph_with'])]['file'],'instrument_role':role,'permutation':i,'bank':bankidx,'legacy_state_compatible':True}
   report.append({'preset_index':preset['index'],'name':preset['name'],'role':role,'table':table['file'],'grain':source.get('1032')==3 and source.get('1042')==tID,'original':original})
   used.add(preset['index'])
 assert len(used)==21,(name,len(used))
 file.write_text(json.dumps(bank,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
assert len(report)==63
out={'method':'Tone Network Similar Permutation / Concept Tree (generation, not filtering)', 'presets_total':192,'presets_revoiced':63,'untouched':129,'source_wavetables':24,'morphed_grains':24,'indices_preserved':True,'programs':report}
a.report.write_text(json.dumps(out,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print('CP59 CONCEPT TREE PASS: 63 revoiced programs / 192 IDs preserved')