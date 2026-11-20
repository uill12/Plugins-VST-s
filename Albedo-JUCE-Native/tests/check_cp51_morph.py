#!/usr/bin/env python3
"""CP51 audit: the morph tree is an offline generator, not a similarity theme.

Verifies all 192 slots, 64 additions, reproducible provenance, limits, two
forms of distinctness (roles/engines) and audio metrics when selection exists.
"""
import argparse,json,math,collections
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--metadata",type=Path,required=True)
p.add_argument("--selected",action="store_true")
a=p.parse_args()
paths=["material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json"]
rows=sum([json.loads((a.metadata/name).read_text(encoding="utf8")) for name in paths],[])
assert len(rows)==192 and [x["index"] for x in rows]==list(range(192))
assert len(set(x["name"] for x in rows))==192
assert len(set((x["family"],x["role"],x["name"]) for x in rows))==192
new=rows[128:];styles=collections.Counter(x["morphTree"]["referenceStyle"] for x in new)
assert styles=={"Max Cooper":16,"Barker":16,"Vril":16,"Grimes":16}, styles
role_counts=collections.Counter(x["role"] for x in new)
assert set(role_counts)=={"Bass","Lead","Pad","Pluck","Keys","Percussion","Drone","Texture"}
assert all(v==8 for v in role_counts.values()), role_counts
cross=sum(x["morphTree"]["crossInfluence"]=="Kauá Brava" for x in new)
assert cross==16,cross
roots=collections.Counter(x["morphTree"]["primaryEngine"] for x in new)
assert len(roots)>=6,roots
for i,x in enumerate(rows):
    v=x["overrides"];m=x["morphTree"]
    for field in ("referenceStyle","instrument","color","function","chosenTransformation",
                  "permittedTransformations","movement","primaryEngine","secondaryEngine"):
        assert field in m,(i,field)
    assert m["chosenTransformation"] in m["permittedTransformations"]
    assert len(x["ideaPath"])==5
    assert all(math.isfinite(float(z)) for z in v.values())
    assert -36<=v["9000"]<=12 and -18<=v["9003"]<=12
    assert v["3111"]==0, "factory must remain polyphonic"
    assert v["1001"]==v["1033"]==v["1065"]==v["1097"]==1
    assert all(3<=v[str(1000+32*j+2)]<=99 for j in range(4))
    assert 0<=v["2004"]<=100
    assert 0<=v["3000"]<=20000
    assert v["6025"] in (1,2,3)
    if x["role"]=="Bass":
        assert v["1003"]==0 and v["1004"]==0 and v["2004"]==0
    if a.selected:
        q=m["renderSelection"]
        assert q["target_rms_db"]<0
        assert math.isfinite(q["pre_rms_db"]) and math.isfinite(q["pre_peak_db"])
        assert len(q["oscillatorRms"])==4
assert rows[0]["index"]==0 and rows[127]["index"]==127
if a.selected:
    report=json.loads((a.metadata/"morph_selection_report.json").read_text())
    assert report["summary"]["programs"]==192
    assert report["summary"]["new_programs"]==64
    assert math.isfinite(report["summary"]["nearest_output_distance_min"])
print(f"CP51 PASS: {len(rows)} indexed timbres, 64 expansions, 8 roles, 4 style sources, 16 Kauá Brava cross-influences; render selected={a.selected}")
