#!/usr/bin/env python3
"""CP51 contract: ALL 128 existing sounds must be regenerated, not just renamed.
Preserves stable slots, names, roles and families for Bitwig session recall.
New 128..191 programs remain additions, not replacements.
"""
import argparse,json,math
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("--original",type=Path,required=True)
p.add_argument("--updated",type=Path,required=True)
p.add_argument("--selected",action="store_true")
a=p.parse_args()

old=sum([json.loads((a.original/x).read_text(encoding="utf8"))
         for x in ("material_factory_bank.json","recursive_forms_bank.json")],[])
new=sum([json.loads((a.updated/x).read_text(encoding="utf8"))
         for x in ("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")],[])
assert len(old)==128 and len(new)==192
assert [x["index"] for x in old]==list(range(128))
assert [x["index"] for x in new]==list(range(192))
# The audio source defines the branch, NOT the current patch name. Names remain
# compatible and their sonic character comes from generation / listening tests.
modified=[]
for before,after in zip(old,new[:128]):
    idx=before["index"]
    for identity in ("index","name","role","family"):
        assert before[identity]==after[identity],(idx,identity)
    assert before["overrides"] != after["overrides"],("Old patch left untouched",idx)
    changes={k for k in set(before["overrides"])|set(after["overrides"])
             if before["overrides"].get(k)!=after["overrides"].get(k)}
    # A real sound redesign, not merely a change to master trim or patch metadata.
    audible=changes.intersection({
      *(str(1000+j*32+k) for j in range(4) for k in (0,2,3,4,6,7,8,9,10,16,18)),
      "3000","3001","3003","3010","3011","3100","3101","3102","3103",
      "4003","5004","2100","2101","2004"})
    assert len(audible)>=12,("Not enough real DSP redesign changes",idx,len(audible))
    modified.append(dict(index=idx,name=after["name"],
                         dsp_parameter_changes=len(audible),
                         prior_master=before["overrides"].get("9000"),
                         updated_master=after["overrides"].get("9000"),
                         branch=after["morphTree"]["chosenTransformation"]))
for x in new[128:]:
    assert x["index"]>=128 and x["morphTree"]["referenceStyle"] in ("Max Cooper","Barker","Vril","Grimes")
if a.selected:
    assert all("renderSelection" in x["morphTree"] for x in new)
    m=json.loads((a.updated/"morph_selection_report.json").read_text(encoding="utf8"))
    assert m["summary"]["programs"]==192
    assert len(m["programs"])==192
    for idx,x in enumerate(new):
        rec=x["morphTree"]["renderSelection"]
        assert len(rec["oscillatorRms"])==4
        assert math.isfinite(rec["gain_correction_db"])
        assert math.isfinite(rec["pre_peak_db"])
        assert abs(x["overrides"]["9000"]-rec["final_master_db"])<.01
out=a.updated/("cp51_existing_preset_regeneration.json")
out.write_text(json.dumps({
  "existingProgramsUpdated":128,"newProgramsAdded":64,"selectedAudioValidated":a.selected,
  "compatibility":"original 0..127 IDs, names, roles and families retained; sound redesigned",
  "existing":modified},indent=2,ensure_ascii=False)+"\n",encoding="utf8")
print("CP51 PASS: all 128 legacy presets materially regenerated (>=12 DSP changes each); 64 new slots retained",
      "with render selection" if a.selected else "as candidates")
