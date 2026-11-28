#!/usr/bin/env python3
"""CP66: synchronize canonical HTML Material IDs to the CP59 DSP range (0..55).

The embedded cp04Registry still advertises 0..31. Native CP59 expands four
Material parameters to 55. Only change these four known existing controls;
never mint parameter IDs or rebuild the GUI/DOM.
"""
import argparse,json,re
from pathlib import Path

MATERIAL_IDS=(1010,1042,1074,1106)
def sync(path):
    html=path.read_text(encoding="utf-8")
    start=html.find("const cp04Registry=")
    assert start>=0, "Original Albedo registry missing"
    pattern=re.compile(r'("id":(1010|1042|1074|1106),"name":"Material"[^{}]*?"min":0,"max":)(31|55)')
    found=pattern.findall(html)
    assert len(found)==4 and {int(id_) for _,id_,_ in found}==set(MATERIAL_IDS),(path,found)
    updated=pattern.sub(lambda m:m.group(1)+"55",html)
    j=updated.index("const cp04Registry=")+len("const cp04Registry=")
    raw=updated[j:]
    reg=json.JSONDecoder().raw_decode(raw)[0]
    definitions={d["id"]:d for d in reg["parameters"]}
    for id_ in MATERIAL_IDS:
        d=definitions[id_]
        assert d["name"]=="Material" and d["min"]==0 and d["max"]==55
    if updated!=html:path.write_text(updated,encoding="utf-8")
    print("CP66 GUI: four existing material parameter ranges synchronized to DSP [0..55]")
def main():
    p=argparse.ArgumentParser()
    p.add_argument("html",type=Path)
    args=p.parse_args()
    sync(args.html)
if __name__=="__main__":main()
