#!/usr/bin/env python3
"""CP38: verify 64 stable legacy voices + 24 original Recursive Forms voices."""
from pathlib import Path
import json, re, math
root=Path(__file__).resolve().parents[1]
data=json.loads((root/"Metadata/recursive_forms_bank.json").read_text(encoding="utf-8"))
original=json.loads((root/"Metadata/factory_preset_taxonomy.json").read_text(encoding="utf-8"))
html=(root.parent/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
hpp=(root/"Source/AlbedoProcessor.h").read_text(encoding="utf-8")
cpp=(root/"Source/AlbedoProcessor.cpp").read_text(encoding="utf-8")
web=(root/"Source/AlbedoWebViewEditor.cpp").read_text(encoding="utf-8")
bridge=(root/"Assets/juce_cp33_bridge.js").read_text(encoding="utf-8")
assert len(original)==64
assert [x["index"] for x in original]==list(range(64))
assert len(data)==24
assert [x["index"] for x in data]==list(range(64,88))
assert [sum(x["family"]==f for x in data) for f in ("Resynthesis Forms","Repetition Fields")]==[12,12]
assert len(set(p["name"] for p in data))==24
assert len({(p["family"],p["name"]) for p in data})==24
assert {x["role"] for x in data}=={"Keys","Pad","Lead","Drone","Texture","Bass","Pluck","Percussion"}
assert all(p["baseIndex"] in range(64) for p in data)
for p in data:
    c=p["overrides"]
    assert len(c)>40
    assert int(c["3111"])==0, "All programs must retain keyboard polyphony"
    assert 0<=c["9001"]<=1
    assert -36<=c["9000"]<=12 and -18<=c["9003"]<=12
    assert 0<=c["4001"]<=100 and .02<=c["4000"]<=24
    assert .001<=c["3100"]<=8 and .01<=c["3103"]<=18
    assert 20<=c["3000"]<=20000 and 20<=c["3003"]<=20000
    assert all(math.isfinite(float(value)) for value in c.values())
atlas=re.search(r'<script type="application/json" id="albedoCreativeCatalog">([^<]+)</script>',html)
assert atlas,"new bank's Atlas catalog must be embedded and valid JSON"
rows=json.loads(atlas.group(1))
assert [p["id"] for p in rows]==list(range(64,88))
assert [(a["name"],a["family"],a["role"]) for a in rows]==[(p["name"],p["family"],p["role"]) for p in data]
assert 'data-collection="Creative"' in html and 'id="pbCreativeCount">24' in html
assert "state.collection==='Creative'?creative" in html
assert "source==='Creative'?creative" in html
assert "payload:'preset|'+p.id" in html
assert "get creative(){return creative.map" in html
assert "getNumPrograms() override { return 64 + (int) albedo_creative::patches.size(); }" in hpp
assert "if (creative)" in cpp and "const auto baseIndex = creative ? extra->baseIndex : index;" in cpp
assert "currentProgram.store (juce::jlimit (0, getNumPrograms()-1, in.readInt()));" in cpp
assert "if(id64>=0 && id64<processor.getNumPrograms())" in web
assert "...window.ALBEDO_PRESET_ATLAS.creative" in bridge
rotary=(root/"Assets/cp38_rotary_overlays.js").read_text(encoding="utf-8")
editor=(root/"Source/AlbedoWebViewEditor.cpp").read_text(encoding="utf-8")
cmake=(root/"CMakeLists.txt").read_text(encoding="utf-8")
assert "cp38_rotary_overlays_js" in editor
assert "cp38_rotary_overlays.js" in cmake
assert "input.dispatchEvent(new Event('input',{bubbles:true}))" in rotary
assert "role','slider" in rotary and "MutationObserver(records" in rotary
assert "window.ALBEDO_CP38_KNOBS" in rotary
assert "cp38-visual" not in rotary, "CP40 removes contextual popups"
assert "get visible(){return false}" in rotary, "Only CP37 ADSR may float"
assert "input[type=range]" in rotary
print("CP38 source and Atlas PASS: 64 immutable factory voices + 24 native polyphonic studies, 88 total")
