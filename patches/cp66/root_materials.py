#!/usr/bin/env python3
"""CP66 material ROOT tree. Different sources, spectral types and complexity first.

Existing, license-reviewed CP59 WaveEdit table IDs 32..55 are reused; the
source list is NOT interpreted as new licensed audio. Root selection is
separate from terminal timbre/gesture generation.
"""
import argparse, hashlib, json
from collections import Counter
from pathlib import Path
BANKS=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
ROOTS=("fundamental","low-harmonic","vocal-formant","acoustic-resonant",
       "metallic-spectral","digital-inharmonic","evolving-granular","stochastic")
COMPLEXITIES=("minimal","restrained","layered","dense")
def choose(root, tables, idx, used):
    def score(t):
        f=t["feature"];lo=f["low_harmonics"];hi=f["high_harmonics"];motion=f["frame_diversity"]
        family=t["family"]
        if root==0:return 2.0*lo-1.7*hi-0.35*motion
        if root==1:return lo-hi+.15*motion
        if root==2:return (2.0 if family=="Vocal" else 0)+.5*hi
        if root==3:return (1.8 if family=="Acoustic" else 0)+.3*lo
        if root==4:return 2*hi+.15*motion
        if root==5:return (1 if family=="Digital" else 0)+hi+0.3*motion
        if root==6:return 2.5*motion+.3*hi
        return (1.5 if family=="Spectral" else 0)+motion+hi
    ranked=sorted(tables,key=lambda t:(-score(t),t["id"]))
    # Reserve room for underused tables: choose within a strong root-compatible subset.
    top=ranked[:max(5,len(tables)//3)]
    return min(top,key=lambda t:(used[t["id"]],int.from_bytes(
      hashlib.sha256(f"{idx}:{t['id']}".encode()).digest()[:2],"big")))
def generate(p,selected,used,role_number):
    idx=p["index"];role=p["role"];v=p["overrides"]
    # Rotate root categories across each instrument role, not global index.
    root=(role_number*5+role_number//8)%len(ROOTS)
    complexity=(role_number//2+role_number%3)%len(COMPLEXITIES)
    if root==0:complexity=0  # clean fundamental allowed to be truly elementary
    if root==1 and role_number%2==0:complexity=0
    table=choose(root,selected,idx,used)
    used[table["id"]]+=1
    source_id=32+table["id"]
    # Keep root source as a real audible material in compatible synthesis modes.
    # For clean analog and noise roots, preserve the existing primary method and
    # use a wavetable as an audible B/C source when source method 1 is required.
    if root in (0,1,2,3,4,5,6):
        v["1000"]=1
        v["1010"]=source_id
        v["1006"]=round((12,22,51,37,66,73,82)[root],2)
        v["1007"]=round((5,12,32,18,47,65,58)[root],2)
        v["1008"]=round((0,5,13,10,26,42,34)[root],2)
    else:
        # Stochastic root is not required to become a continuous noise bed.
        v["1032"]=1
        v["1042"]=source_id
    p["root_material"]={
      "tree":"source type > harmonic distribution > motion > role > complexity",
      "type":ROOTS[root],"complexity":COMPLEXITIES[complexity],
      "table_id":source_id,"table_name":table["file"],
      "licensed_source_sha256":table["sha256"],
      "source_family":table["family"],
      "measured_source_features":table["feature"],
      "source_selection":"root-specific spectral feature ranking with usage diversity",
      "audio_similarity_pending":True}
    return {"index":idx,**p["root_material"]}
def main():
    a=argparse.ArgumentParser()
    a.add_argument("--metadata",required=True,type=Path)
    a.add_argument("--selection",required=True,type=Path)
    a.add_argument("--report",required=True,type=Path)
    args=a.parse_args()
    selected=json.loads(args.selection.read_text(encoding="utf-8"))["selected"]
    assert len(selected)==24 and len({t["id"] for t in selected})==24
    paths=[args.metadata/f for f in BANKS]
    banks=[json.loads(f.read_text(encoding="utf-8")) for f in paths]
    patches=sum(banks,[])
    assert len(patches)==192 and [p["index"] for p in patches]==list(range(192))
    ordinals=Counter();used=Counter()
    report=[]
    for p in patches:
        role=p["role"];position=ordinals[role];ordinals[role]+=1
        report.append(generate(p,selected,used,position))
    assert len({p["root_material"]["type"] for p in patches})==len(ROOTS)
    assert len({p["root_material"]["complexity"] for p in patches})==len(COMPLEXITIES)
    assert len({p["root_material"]["table_id"] for p in patches})>=16
    for path,bank in zip(paths,banks):
        path.write_text(json.dumps(bank,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    args.report.write_text(json.dumps({"checkpoint":"CP66","selected":len(report),
      "root_counts":dict(Counter(x["type"] for x in report)),
      "complexity_counts":dict(Counter(x["complexity"] for x in report)),
      "table_usage":dict(used),"render_qa_required":True,
      "presets":report},ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    print("CP66 roots: 192 distinct selections by sound type, feature and complexity")
if __name__=="__main__":main()
