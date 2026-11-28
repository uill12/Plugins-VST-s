#!/usr/bin/env python3
"""CP66 verify/sanitize preset values to the actual registered parameter schema.

The final 192 candidate presets may inherit method-specific fields whose
original values fall outside their *host-registered* range (e.g. grain count
1016=0 despite min=1). A host would clamp these implicitly; enforce and log
that transformation explicitly BEFORE header generation and audio tests.

Does not alter unknown IDs, pitches, user clock, host MIDI, ARP or program IDs.
"""
from __future__ import annotations
import argparse,json,math
from pathlib import Path
from collections import Counter
BANKS=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
def read_schema(html_file):
    html=html_file.read_text(encoding="utf-8")
    marker="const cp04Registry="
    i=html.find(marker)
    if i<0:raise ValueError("Canonical HTML parameter registry absent")
    definitions=json.JSONDecoder().raw_decode(html[i+len(marker):])[0]["parameters"]
    schema={str(v["id"]):v for v in definitions}
    # 9003 CP37 native master gain is defined in a separate embedded block.
    schema["9003"]={"id":9003,"name":"CP37 Gain","min":-18,"max":12,"stepped":False}
    assert len(schema)>=190
    for b in (1010,1042,1074,1106):
        assert schema[str(b)]["max"]==55,"Run CP66 sync_ui_schema.py first"
    return schema
def normalize(metadata,html_file,report):
    schema=read_schema(html_file)
    paths=[metadata/name for name in BANKS]
    banks=[json.loads(path.read_text(encoding="utf-8")) for path in paths]
    rows=sum(banks,[])
    assert len(rows)==192 and [r["index"] for r in rows]==list(range(192))
    changes=[];id_counts=Counter()
    protected={1000,1001,1002,1003,1032,1033,1034,1035,1064,1065,1066,1067,
               1096,1097,1098,1099,6020,6021,6023,6024,6029,6030,6031,6032,
               3111,3131,9000,9001}
    for p in rows:
        idx=p["index"];v=p["overrides"];local=[]
        for key,value in v.items():
            if key not in schema:raise ValueError(f"Unknown parameter {key} in preset {idx}")
            if not isinstance(value,(int,float)) or not math.isfinite(value):
                raise ValueError(f"Nonfinite param {key} in preset {idx}")
            d=schema[key];lo=float(d["min"]);hi=float(d["max"])
            if lo>hi:raise ValueError(f"Invalid schema for {key}")
            n=float(value)
            if d.get("stepped"):n=round(n)
            n=max(lo,min(hi,n))
            if n!=value:
                if int(key) in protected:
                    raise ValueError(f"Protected musical or engine param {key} out of range: {value} -> {n}, preset {idx}")
                v[key]=int(n) if d.get("stepped") else round(n,6)
                local.append({"parameter":int(key),"label":d["name"],"old":value,"new":v[key]})
                id_counts[key]+=1
        p["registered_range_normalization"]={"corrections":local,"passed":True}
        if local:changes.append({"index":idx,"corrected":local})
    for path,bank in zip(paths,banks):
        path.write_text(json.dumps(bank,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    result={"checkpoint":"CP66","presets":192,"modified_presets":len(changes),
            "corrected_values":sum(id_counts.values()),
            "corrected_ids":dict(sorted(id_counts.items(),key=lambda x:int(x[0]))),
            "all_host_ranges_valid":True,
            "audio_render_validation":"pending","details":changes}
    report.write_text(json.dumps(result,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    print(f"CP66 registered range normalization: corrected {sum(id_counts.values())} values in {len(changes)} presets")
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--metadata",required=True,type=Path)
    p.add_argument("--gui",required=True,type=Path)
    p.add_argument("--report",required=True,type=Path)
    a=p.parse_args();normalize(a.metadata,a.gui,a.report)
if __name__=="__main__":main()
