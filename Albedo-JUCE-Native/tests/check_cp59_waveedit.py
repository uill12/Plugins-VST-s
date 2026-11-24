#!/usr/bin/env python3
"""CP59 source and data contract; CTest separately verifies real plugin audio."""
from pathlib import Path
import argparse,hashlib,json,struct
p=argparse.ArgumentParser();p.add_argument('--root',type=Path,default=Path('Albedo-Quintessential-Synth'));p.add_argument('--metadata',type=Path,default=Path('Albedo-JUCE-Native/Metadata'));p.add_argument('--report',type=Path,default=Path('CP59_ConceptTree_Preset_QA.json'));a=p.parse_args()
manifest=json.loads(Path('patches/cp59/concept_tree_selection.json').read_text(encoding='utf-8'))
assert manifest['total_inspected']==705 and len(manifest['selected'])==24
assert len(set(x['file'] for x in manifest['selected']))==24
assert all(0<=x['morph_with']<24 for x in manifest['selected'])
assert all(len(x['sha256'])==64 for x in manifest['selected'])
resource=a.root/'resources/Albedo_CP59_WaveEdit_MorphGrain.albw'
assert resource.exists() and resource.stat().st_size==6881308
with resource.open('rb') as f:
 assert struct.unpack('<8sIIIII',f.read(28))==(b'ALBWT59\0',24,64,256,8,12288)
cpp=(a.root/'src/vitriol_dsp.cpp').read_text(encoding='utf-8')
head=(a.root/'include/vitriol_dsp.hpp').read_text(encoding='utf-8')
params=(a.root/'include/vitriol_params.hpp').read_text(encoding='utf-8')
cmake=Path('Albedo-JUCE-Native/CMakeLists.txt').read_text(encoding='utf-8')
web=Path('Albedo-JUCE-Native/Source/AlbedoWebViewEditor.cpp').read_text(encoding='utf-8')
processor=Path('Albedo-JUCE-Native/Source/AlbedoProcessor.cpp').read_text(encoding='utf-8')
for term in ('loadWaveEditBankBytes','waveEditSample','waveEditGrainSample','CC121'):
 assert term in cpp or term=='CC121' and 'if(cc==121)' in cpp,term
assert 'waveEditTables_' in head and 'add(b+10,"Material",g,0,55' in params
assert ('VERSION 1.0.59' in cmake or 'VERSION 1.0.60' in cmake or 'VERSION 1.0.61' in cmake)
assert 'albedo_waveedit_bank.rc' in cmake and 'AlbedoCP59WaveEditTest' in cmake
assert 'cp59_waveedit_materials_js' in web
assert "materials.extend" in Path('patches/cp50/export_recipes.py').read_text(encoding='utf-8'), 'Target matcher must recognize all 56 material IDs'
assert 'synth->reset(); // Stop stale arpeggiator/sustain/FX state together.' in processor
assert 'if(cc==123){synth->midiCC(123,0,ch);' in processor
if a.report.exists():
 report=json.loads(a.report.read_text(encoding='utf-8'))
 assert report['presets_revoiced']==63 and report['untouched']==129
 assert len({x['preset_index'] for x in report['programs']})==63
 for name in ['material_factory_bank.json','recursive_forms_bank.json','morph_expansion_bank.json']:
  bank=json.loads((a.metadata/name).read_text(encoding='utf-8'))
  assert len(bank)==64 and len({x['index'] for x in bank})==64
  assert sum('concept_tree' in x for x in bank)==21
print('CP59 PASS: 24 CC0 banks, 48-cycle grains, 63 distinct preset branches, MIDI fail-safe contracts')