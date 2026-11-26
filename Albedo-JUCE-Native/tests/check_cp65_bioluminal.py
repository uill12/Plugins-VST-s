#!/usr/bin/env python3
"""CP65: all 192 material timbres generated, 64 creative names, one bank."""
import json,math,re
from pathlib import Path
root=Path(".")
p=root/"Albedo-JUCE-Native/Metadata"
names=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
allrows=sum((json.loads((p/name).read_text(encoding="utf-8")) for name in names),[])
assert len(allrows)==192
assert [x["index"] for x in allrows]==list(range(192))
assert len({x["name"] for x in allrows})==192
report=json.loads(Path("CP65_Bioluminal_192_Generation_QA.json").read_text(encoding="utf-8"))
assert report["sound_slots"]==report["revoiced"]==192
assert report["morphogenic_renamed"]==64 and len(report["presets"])==192
assert {x["id"] for x in report["presets"]}==set(range(192))
morph=allrows[128:]
assert len({x["name"] for x in morph})==64
assert not any(re.search(r"\b(?:field|motion)\b",x["name"],flags=re.I) for x in morph)
assert all(2<=len(x["name"].split())<=3 for x in morph)
assert all(x["role"] in ("Bass","Lead","Pad","Pluck","Keys","Percussion","Drone","Texture") for x in allrows)
assert len({x["role"] for x in allrows})==8
assert all(x["name"]==r["name"] for x,r in zip(morph,report["presets"][128:]))
assert len({json.dumps(x["overrides"],sort_keys=True) for x in allrows})==192
for x,r in zip(allrows,report["presets"]):
 v=x["overrides"];i=x["index"]
 assert r["id"]==i and r["changes"]>=25,(i,r.get("changes"))
 assert x["bio_cyber_branch"]["pitch_stable"]
 assert "TNSP" in x["bio_cyber_branch"]["method"]
 assert all(math.isfinite(float(y)) for y in v.values()),i
 assert v["3111"]==0 and v["3131"]==0,i
 for layer in range(4):
  base=1000+32*layer
  assert v[str(base+5)]==1
  assert -24<=v[str(base+3)]<=24
  assert 3<=v[str(base+2)]<=99
  assert 0<=v[str(base)]<=9
  assert 0<=v[str(base+10)]<=55
 assert len(set(v[str(1000+32*j+3)] for j in range(4)))>=3,(i,"octave layers not distinct")
 assert v["1004"]==0,(i,"fundamental must remain centered")
 assert 20<=v["3000"]<=20000 and 20<=v["3003"]<=20000
 assert .001<=v["3100"]<=8 and .01<=v["3103"]<=18
 assert 0<=v["5004"]<=35 and 0<=v["5002"]<=32
 assert v["9001"]==1 and -36<=v["9000"]<=12
 assert r["midi_clock_untouched"]
for i in range(128):
 # Previous user-approved names survive even while their sounds revoice.
 assert allrows[i]["name"]==report["presets"][i]["name"]
assert len({tuple(x["octaves"]) for x in report["presets"]})>=8
# Runtime GUI is a SINGLE factory catalog of all 192 stable program slots.
ui=(root/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
browser=(root/"Albedo-JUCE-Native/Assets/cp52_nereida_browser.js").read_text(encoding="utf-8")
for token in ("const factory=Object.freeze(","const catalog=Object.freeze([...factory,...creative,...newSounds]);",
              "ALBEDO · BIOLUMINAL ATLAS · 192","BIOLUMINAL ATLAS <span id=\"pbFactoryCount\">192</span>",
              'data-collection="Creative" aria-selected="false" hidden',
              'data-collection="New" aria-selected="false" hidden'):
 assert token in ui,token
assert ui.count('optgroup label="ALBEDO · BIOLUMINAL ATLAS · 192"')==1
assert "const libraryCounts=()=>atlas.catalog.length;" in browser
assert "ALBEDO / BIOLUMINAL ATLAS" in browser
embedded=re.search(r'<script type="application/json" id="albedoNewSoundsCatalog">([^<]+)</script>',ui)
assert embedded
voices=json.loads(embedded.group(1))
assert len(voices)==64
assert [x["id"] for x in voices]==list(range(128,192))
assert [x["name"] for x in voices]==[x["name"] for x in morph]
assert "morphTree" not in embedded.group(1)
assert "location.reload" not in browser
print("CP65 PASS: 192 TNSP-resynthesized patches, 64 conventional creative names, 192 native IDs, one live searchable bank")
