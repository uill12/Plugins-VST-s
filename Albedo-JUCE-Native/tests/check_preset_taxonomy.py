#!/usr/bin/env python3
"""Check complete immutable 64-index metadata, taxonomy selectors and JUCE Atlas wiring."""
from pathlib import Path
import json, re, subprocess, sys

r=Path(__file__).resolve().parents[1]
factory=r.parent/'base/include/albedo_factory_patches.hpp'
metadata=r/'Metadata/factory_preset_taxonomy.json'
header=r/'Source/AlbedoPresetTaxonomy.hpp'
editor=r/'Source/AlbedoEditor.cpp'
original=re.findall(r'\{L"([^\"]+)"\s*,\s*\{',factory.read_text(encoding='utf-8'))
rows=json.loads(metadata.read_text(encoding='utf-8'))
assert len(original)==len(rows)==64
expected_columns={'index','name','family','role','character','motion','context','description'}
assert all(set(r)==expected_columns for r in rows)
assert all(row['name']==original[i] and row['index']==i for i,row in enumerate(rows))
assert len(set(original))==64
expected_families=['Vitreous','Organic','Metallic','Cellular','Nocturnal','Aerial','Kinetic','Chimeric']
expected_roles=['Bass','Lead','Pad','Pluck','Keys','Percussion','Drone','Texture']
assert [rows[i*8]['family'] for i in range(8)]==expected_families
assert [rows[i]['role'] for i in range(8)]==expected_roles
for i,row in enumerate(rows):
    assert row['family']==expected_families[i//8]
    assert row['role']==expected_roles[i%8]
    for key in expected_columns-{'index'}:
        assert row[key] and row[key].strip() not in ('Unknown','Unassigned','TBD','N/A','None'),(i,key)
assert len({(r['family'],r['role']) for r in rows})==64
# Search and filters must reach every preset through a nonempty metadata field.
for field in ['family','role','character','motion','context']:
    categories={r[field] for r in rows}
    assert len(categories)>1
    assert all(any(x[field]==category for x in rows) for category in categories)
for row in rows:
    for field in ['name','character','motion','context']:
        needle=row[field].casefold()
        assert any(needle in ' '.join(str(v) for k,v in x.items() if k!='index').casefold() for x in rows)
# Re-running the generator must be deterministic.
old=header.read_bytes()
subprocess.run([sys.executable,str(r/'tools/generate_preset_taxonomy.py')],check=True,capture_output=True)
assert header.read_bytes()==old,'C++ taxonomy is stale relative to checked JSON metadata'
s=editor.read_text(encoding='utf-8')
for token in ['#include "AlbedoPresetTaxonomy.hpp"','ALL CHARACTERS','ALL MOTIONS',
              'ALL CONTEXTS','8 x 8 MATRIX','getCurrentProgram()',
              'loadFactoryPreset(selectedIndex)','m.description','character.onChange',
              'motion.onChange','context.onChange','albedo_preset_taxonomy::roles']:
    assert token in s,token
assert 'factoryName(i).fromLastOccurrenceOf' not in s,'Atlas still guessing taxonomy from display strings'
print('PASS: 64 indexed factory patches, 8x8 matrix, all seven fields filled, filters & sorting wired')
print('Coverage: 8 families, 8 roles, %d characters, %d motions, %d suggested contexts' % (
    len(set(x['character'] for x in rows)),len(set(x['motion'] for x in rows)),len(set(x['context'] for x in rows))))
