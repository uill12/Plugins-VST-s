"""Export the audible construction paths used by the offline target matcher."""
import json,sys
from pathlib import Path
folder=Path(sys.argv[1])
rows=sum([json.loads((folder/n).read_text(encoding='utf8')) for n in ['material_factory_bank.json','recursive_forms_bank.json','morph_expansion_bank.json']],[])
methods=['Analog','Wavetable','Harmonic additive','Granular','Spectral','Physical modal','Noise','Time-warp sampler','Phase distortion','Cellular']
materials='Bronze Quiver|Hollow Glass|Copper Bloom|Bell Dust|Reed Current|Plucked Wire|Wooden Vein|Ceramic Grain|Felt Pulse|Membrane Ember|Air Column|Spring Alloy|Ice Lattice|Stone Ripple|Water Pearl|Electric Filament|Soft Machine|Sand Drift|Vowel Foam|Carbon Friction|Polyphonic Metal|Marimba Wood|Granular Resin|Light Ash|Vowel Foam|Shimmer Film|Mechanical Ember|Earth Drone|Azure Swell|Porcelain Bloom|Mineral Rain|Living Circuit'.split('|')
materials[24]='Ink Falls'
# CP59: expand source names when the CC0 Concept Tree is present. Preserve the
# original 32 material indices, and append verified 24 tables as IDs 32..55.
selection=Path('patches/cp59/concept_tree_selection.json')
if selection.is_file():
    picked=sorted(json.loads(selection.read_text(encoding='utf8'))['selected'],key=lambda x:x['id'])
    assert len(picked)==24 and [x['id'] for x in picked]==list(range(24))
    materials.extend(f"WaveEdit {x['family']} ({x['role']})" for x in picked)

out=[]
for p in rows:
    v=p['overrides'];r=p['recipe'].copy();r['function']=p['role']+' / '+r['function'];layers=[]
    for i in range(4):
        a=1000+i*32;m=int(v[str(a)])
        layers.append(dict(layer='ABCD'[i],engine=methods[m],material=materials[int(v[str(a+10)])] if m in (1,2,3,7) else None,
                           level=v[str(a+2)],semitones=v[str(a+3)],pan=v[str(a+4)],character=[v[str(a+j)] for j in (6,7,8,9)]))
    r.update(layers=layers,ampEnvelope=[v[str(j)] for j in (3100,3101,3102,3103)],filterACutoff=v['3000'],cellArpeggiator=bool(v['6020']),
             morphTree=p.get('morphTree'),generationPath=p.get('ideaPath'),
             auditionMetrics=p.get('morphTree',{}).get('renderSelection'))
    out.append(dict(slot=p['index'],name=p['name'],recipe=r))
Path(sys.argv[2]).write_text(json.dumps(out,indent=1,ensure_ascii=False),encoding='utf8')
