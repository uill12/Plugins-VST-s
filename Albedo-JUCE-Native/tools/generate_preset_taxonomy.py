#!/usr/bin/env python3
"""Generate the standalone constexpr JUCE factory Atlas metadata (no DSP edits)."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
source = ROOT / 'Metadata' / 'factory_preset_taxonomy.json'
target = ROOT / 'Source' / 'AlbedoPresetTaxonomy.hpp'
entries = json.loads(source.read_text(encoding='utf-8'))
expected = {'index', 'name', 'family', 'role', 'character', 'motion', 'context', 'description'}
assert len(entries) == 64, len(entries)
for index, entry in enumerate(entries):
    assert set(entry) == expected, (index, set(entry) ^ expected)
    assert entry['index'] == index, (index, entry['index'])
    assert all(isinstance(entry[x], str) and entry[x].strip() for x in expected - {'index'}), index
families = list(dict.fromkeys(x['family'] for x in entries))
roles = list(dict.fromkeys(x['role'] for x in entries))
assert len(families) == len(roles) == 8
assert len({(x['family'], x['role']) for x in entries}) == 64
chars = sorted(set(x['character'] for x in entries))
motions = sorted(set(x['motion'] for x in entries))
contexts = sorted(set(x['context'] for x in entries))

def quote(s):
    return json.dumps(s, ensure_ascii=True)

def array(name, values):
    return f'inline constexpr std::array<const char*, {len(values)}> {name} = {{{{\n' + ',\n'.join('    '+quote(s) for s in values) + '\n}};\n'

out = '''#pragma once
// Generated from Metadata/factory_preset_taxonomy.json.
// Curatorial metadata only. Does not alter DSP parameters, factory names or IDs.
// Rebuild with: python tools/generate_preset_taxonomy.py
#include <array>
#include <cstddef>

namespace albedo_preset_taxonomy {
struct PresetCategory {
    int index;
    const char* name;           // Original immutable patch name
    const char* family;         // Factory material family
    const char* role;           // Instrument role
    const char* character;      // Audible/timbral descriptor
    const char* motion;         // Suggested modulation/envelope behavior
    const char* context;        // Suggested musical context (not a restriction)
    const char* description;    // Curatorial description, not a measurement
};
'''
for name, vals in [('families',families),('roles',roles),('characters',chars),('motions',motions),('contexts',contexts)]:
    out += '\n'+array(name, vals)
out += '\ninline constexpr std::array<PresetCategory, 64> entries = {{\n'
for x in entries:
    out += '    {' + str(x['index']) + ', ' + ', '.join(quote(x[k]) for k in ('name','family','role','character','motion','context','description')) + '},\n'
out += '''}};
static_assert(entries.size() == 64, "Factory taxonomy must retain all 64 patch IDs");
inline constexpr const PresetCategory& byIndex(int index) noexcept
{
    return entries[static_cast<std::size_t>(index)];
}
} // namespace albedo_preset_taxonomy
'''
target.write_text(out, encoding='utf-8')
print('Generated:', target, 'with', len(entries), 'complete factory records')
print('Categories:', len(families), 'families,', len(roles), 'roles,',len(chars),'characters,',len(motions),'motions,',len(contexts),'contexts')
