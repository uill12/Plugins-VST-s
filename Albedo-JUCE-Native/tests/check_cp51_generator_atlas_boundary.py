#!/usr/bin/env python3
"""CP51: morphology is a DEVELOPMENT-TIME sound generator, never an Atlas filter."""
from pathlib import Path
import re,json,sys
html=Path(sys.argv[1]).read_text(encoding="utf-8")
patcher=Path("patches/cp51/patch_atlas_catalog.py").read_text(encoding="utf-8")
rows=json.loads(Path(sys.argv[2]).read_text(encoding="utf-8"))
m=re.search(r'<script type="application/json" id="albedoNewSoundsCatalog">([^<]+)</script>',html)
assert m,"New Sounds catalog not embedded"
catalog=json.loads(m.group(1))
assert len(catalog)==64 and [p["id"] for p in catalog]==list(range(128,192))
assert "morphTree" not in patcher, "Offline generation graph leaked to Atlas adapter"
assert "morphTree" not in m.group(1), "No morph provenance or branch descriptors in runtime catalog"
assert "ideaPath" not in m.group(1) and "selectedPermutation" not in m.group(1)
assert all(set(x)=={"id","name","family","role","character","baseIndex","factoryKey","source","idea","trait"} for x in catalog)
assert all(x["source"]=="New" and x["factoryKey"]=="NEW:"+str(x["id"]) for x in catalog)
assert all(x["name"]==r["name"] and x["family"]==r["family"] and x["role"]==r["role"] for x,r in zip(catalog,rows))
assert 'data-collection="New"' in html and 'data-collection="Morph"' not in html
assert 'id="pbNew"' in html and 'id="pbNewCount"' in html
assert "state.collection==='New'?newSounds" in html
assert "get newSounds(){return newSounds.map" in html
assert 'data-pb-sort="name"' in html and 'data-pb-sort="family"' in html and 'data-pb-sort="role"' in html
assert 'data-pb-sort="morph"' not in html
print("CP51 PASS: morph tree exclusively offline; Atlas exposes finished voices and normal sort/filter controls")
