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
    if root==7:complexity=2+(role_number%2)  # tonal core + audible noise excitation
    table=choose(root,selected,idx,used)
    used[table["id"]]+=1
    source_id=32+table["id"]
    # Source type determines PRIMARY synthesis architecture, not merely which
    # wavetable file is loaded. Reusing method 1 everywhere caused same-color
    # harmonic shapes even when the material file changed.
    primary_methods=(0,1,1,5,4,8,3,5)
    primary=primary_methods[root]
    if root==7:
        # Noise-only primary cannot carry a dependable melodic pitch. Use an
        # appropriate tonal resonator and keep stochastic excitation as a
        # deliberate secondary source (assigned in the terminal stage).
        primary={"Bass":0,"Lead":8,"Pad":3,"Pluck":5,
                 "Keys":2,"Percussion":5,"Drone":3,"Texture":4}[role]
    v["1000"]=primary
    if primary==1:
        v["1010"]=source_id
        v["1006"]=(15,29,49,36,69,81,75,27)[root]
        v["1007"]=(3,12,27,18,54,61,49,8)[root]
        v["1008"]=(0,5,12,9,33,47,31,5)[root]
    else:
        # The terminal stage will route this table to an audible supporting
        # wavetable engine only if the endpoint uses more than one generator.
        # Minimal endpoints use the primary synth alone by design.
        v["1032"]=1
        v["1042"]=source_id
    p["root_material"]={
      "tree":"source type > harmonic distribution > motion > role > complexity",
      "type":ROOTS[root],"complexity":COMPLEXITIES[complexity],
      "table_id":source_id,"table_name":table["file"],
      "licensed_source_sha256":table["sha256"],
      "source_family":table["family"],
      "primary_method":primary,
      "stochastic_root_uses_tonal_anchor":root==7,
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
