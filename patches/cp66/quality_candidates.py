#!/usr/bin/env python3
"""CP66 quality-first permutation generator (development-time, audio QA still required).

Construct multiple acoustically motivated parameter candidates for each root and
terminal branch; choose candidates with good role compatibility, useful spectral
shape, appropriate simplicity and distinct parameter features. This stage
*generates* actual new synthesizer settings, it does not merely filter names.
A parameter score is only a screening proxy; rendered DSP/audio QA is mandatory.
"""
from __future__ import annotations
import argparse, hashlib, json, math
from collections import Counter
from pathlib import Path

BANKS=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
ROLES=("Bass","Lead","Pad","Pluck","Keys","Percussion","Drone","Texture")
ROOT_SHAPES={
 "fundamental":((6,4,2,0),(.05,.14),("pure","rounded")),
 "low-harmonic":((17,9,5,1),(.10,.29),("warm","woody")),
 "vocal-formant":((42,78,17,35),(.22,.65),("hollow","vowel")),
 "acoustic-resonant":((26,58,9,67),(.13,.53),("wood","struck")),
 "metallic-spectral":((77,29,89,16),(.31,.88),("bell","metal")),
 "digital-inharmonic":((82,13,67,42),(.29,.90),("pixel","glass")),
 "evolving-granular":((47,64,23,79),(.25,.82),("grain","cloud")),
 "stochastic":((71,52,15,84),(.29,.98),("air","rough"))
}
GESTURE_WEIGHTS={"direct":0,"elastic":1,"recursive":2,"suspended":3}
# Lower complexity means less extra material: quality is not tied to density.
COMPLEXITY_ACTIVE={"minimal":1,"restrained":2,"layered":3,"dense":4}
def clamp(x,lo,hi):
    assert math.isfinite(float(x))
    return round(max(lo,min(hi,float(x))),5)
def hashint(*items):
    data=":".join(str(x) for x in items).encode("utf-8")
    return int.from_bytes(hashlib.sha256(data).digest()[:8],"big")
def parameters(v):
    return {k:float(x) for k,x in v.items()}
def feature(v):
    # Equalized feature families for structural novelty comparisons.
    x=[int(v[str(1000+32*i)])/9 for i in range(4)]
    x += [float(v[str(1002+32*i)])/100 for i in range(4)]
    x += [float(v[str(1006+j)])/100 for j in range(4)]
    x += [float(v["3000"])/18000,float(v["3001"])/100,float(v["3004"])/100]
    x += [float(v["3100"])/2,float(v["3101"])/3,float(v["3102"])/100,
          float(v["3103"])/6,float(v["4001"])/100]
    return tuple(x)
def distance(a,b):
    # Treat major tonal architecture, source weight, harmonic shape and envelope
    # as separate meaningful axes. Not a perceptual similarity metric.
    return math.sqrt(sum((x-y)**2 for x,y in zip(a,b))/len(a))
def proposed(p,number):
    root=p["root_material"];typ=root["type"];v=dict(p["overrides"])
    shape,band,variants=ROOT_SHAPES[typ]
    idx=p["index"];role=p["role"];complexity=root["complexity"]
    gesture=p["concept_tree_terminal"]["gesture"]
    seed=hashint(idx,typ,number,p["wolfram_generation"]["seed"])
    # Different proposals may deliberately be bare, asymmetrical, peaked,
    # odd-harmonic or evolving. Avoid a shared all-bright shape.
    shapes=(
        (1.00,.18,.08,.04),(.34,1.00,.18,.08),(.24,.16,1.00,.19),
        (.36,.09,.28,1.00),(1.00,.75,.12,.05),(.10,.63,.20,.85),
        (.85,.10,.70,.08),(.27,.80,.10,.68))
    weights=shapes[(number+idx//7)%len(shapes)]
    base=1000
    for j in range(4):
        # Primary spectral structure is intentional; unlike the earlier
        # CP65/66 passes, it is no longer left nearly unchanged for all roots.
        k=str(base+6+j)
        val=shape[j]*weights[j]+4*(number%3)+((seed>>(5*j))&7)-3
        v[k]=clamp(val,0,96)
    v["1014"]=clamp((9,14,23,31,40,56,68,76)[number]
                      *(.65+.55*band[1]),0,95)
    # Source methods/table IDs must remain valid. No silent WAV replacement.
    primary=int(root["primary_method"])
    assert int(v["1000"])==primary
    if primary==1:
        v["1010"]=int(root["table_id"])
    # Real instrument shapes, not arbitrary complexity maximization.
    active=COMPLEXITY_ACTIVE[complexity]
    if active==1:
        for e in range(1,4):v[str(1002+32*e)]=0
        v["4001"]=0;v["5002"]=0;v["5004"]=0
    else:
        for e in range(active,4):v[str(1002+32*e)]=0
        # Different interaction topology; preserve B as root wavetable.
        for e in range(1,active):
            k=str(1002+32*e)
            v[k]=clamp(float(v[k])*(.70+.09*((seed>>(4*e))&7)),4,84)
    # Adaptive parameter topology: disable inactive engine contributions,
    # transform active modulation/filter/FX structure, and add usable existing
    # synthesis parameters where the source truly benefits. These are native
    # registered IDs, NOT metadata-only phantom controls.
    edits={"enabled":[],"disabled":[],"transformed":[]}
    if active==1:
        # One-source tone: disable unnecessary secondary source shapes and
        # global motion instead of carrying a hidden 4-engine preset.
        for e in (1,2,3):
            b=1000+32*e
            for j in (6,7,8,9):
                key=str(b+j)
                if key in v and float(v[key])!=0:
                    v[key]=0; edits["disabled"].append(key)
        for key in ("4013","4014"):
            if key in v:
                v[key]=0;edits["disabled"].append(key)
    else:
        # Selective modulation activation on a supporting source, not on
        # every oscillator. Retain pitch stability and musical key tracking.
        e=1 if active==2 else 1+(number%min(2,active-1))
        b=1000+32*e
        for j,amount in ((6,14+number*8),(7,82-number*6)):
            key=str(b+j)
            if key in v:
                v[key]=clamp(amount,0,100);edits["transformed"].append(key)
        if "4013" in v:
            v["4013"]=clamp((0,12,28,43,0,61,19,36)[number],0,100)
            edits["transformed"].append("4013")
    # Filter bypass/routing can vary independently of oscillator timbre.
    # Keep the valid stepped native enum instead of creating a new ID.
    if "3012" in v:
        v["3012"]=(0,1,2)[(number+idx//9)%3]
        edits["transformed"].append("3012")
    if "3010" in v:
        v["3010"]=(0,1,2,3,4)[(number+idx//13)%5]
        edits["transformed"].append("3010")
    # Earlier minimal patches still inherited always-on saturation/diffusion
    # from their CP65 parents, which reintroduced the same harmonic color.
    # A clean tone must be allowed to remain clean; richer material may choose
    # coloration, but it is not mandatory.
    if active==1:
        for key in ("5000","5006"):
            v[key]=0
            edits["disabled"].append(key)
        for key in ("5001","5007"):
            v[key]=0
            edits["disabled"].append(key)
    else:
        v["5000"]=clamp((0,4,9,15,2,22,7,13)[number],0,28)
        v["5001"]=int(float(v["5000"])>1)
        edits["enabled" if v["5001"] else "disabled"].append("5001")
    # FX enable bits: turning wet percentage to zero is insufficient if
    # the generator leaves an effect state and CPU-expensive pathway enabled.
    for mix_id,on_id in (("5002","5003"),("5004","5005"),("5006","5007")):
        if on_id in v:
            enabled=float(v.get(mix_id,0))>1 and active>1
            v[on_id]=int(enabled)
            edits["enabled" if enabled else "disabled"].append(on_id)
    # Only reuse existing supported parameters. If a new runtime parameter
    # is needed, it requires a coordinated DSP + registration + HTML + host
    # state migration; never invent one in metadata.
    assert all(k.isdecimal() for group in edits.values() for k in group)
    # Spectral filters and envelope are shaped for musical function, not
    # pasted identical across eight roles.
    role_cut={"Bass":.65,"Lead":1.50,"Pad":1.05,"Pluck":1.65,
              "Keys":1.15,"Percussion":1.85,"Drone":.76,"Texture":1.35}[role]
    cutoff=(180,430,860,1900,3600,5800,8500,12600)[(number+idx//11)%8]
    v["3000"]=clamp(cutoff*role_cut,30,17500)
    v["3003"]=clamp(float(v["3000"])*(1.25+.16*(number%5)),35,19000)
    v["3001"]=clamp((8,25,61,14,74,38,47,19)[number],0,95)
    v["3004"]=clamp((12,44,9,63,18,71,33,56)[(number+idx)%8],0,95)
    # Preserve the role's reasonable envelope from CP66 while creating
    # two distinct articulation possibilities per role.
    atk=float(v["3100"]);dec=float(v["3101"]);rel=float(v["3103"])
    factor=(.65,.80,1.,1.18,1.34,.72,1.10,.90)[number]
    v["3100"]=clamp(atk*factor,.001,8)
    v["3101"]=clamp(dec*(1.4-.3*factor),.01,12)
    v["3103"]=clamp(rel*(.7+.3*factor),.01,18)
    # Wolfram supports expressive motion, but a stable/simple tone can
    # remain completely still. Do not generate detuned or drifting pitch.
    if active==1 or number in (0,4):
        v["4001"]=0
    else:
        v["4001"]=clamp(float(v["4001"])*(.55+.12*(number%4)),0,100)
    if complexity=="minimal":
        v["5004"]=0;v["5002"]=0
    elif number in (0,3):
        v["5004"]=clamp(v["5004"]*.55,0,25)
        v["5002"]=clamp(v["5002"]*.55,0,25)
    # Noise is a deliberate instrument ingredient. Do not accidentally
    # boost an active stochastic layer on clean roots.
    for e in range(active):
        if int(v[str(1000+32*e)])==6 and typ not in ("stochastic","evolving-granular"):
            v[str(1002+32*e)]=clamp(v[str(1002+32*e)],0,25)
    # The previous CP66 stage set source levels to zero but left source power
    # switches ON. That can waste CPU and display misleading active engines.
    # Make the actual native 1001/1033/1065/1097 enabled flags consistent.
    for e in range(4):
        enabled=int(float(v[str(1002+32*e)])>0)
        key=str(1001+32*e)
        v[key]=enabled
        edits["enabled" if enabled else "disabled"].append(key)
    # Existing older metadata sometimes stored Arp Character=4 even though
    # 6024 has a registered [0..3] range. Wolfram expansion belongs to 6031,
    # which is already generated separately and is NOT altered here.
    if "6024" in v:
        corrected=max(0,min(3,int(round(float(v["6024"])))))
        if v["6024"]!=corrected:
            v["6024"]=corrected
            edits["transformed"].append("6024")
    return v,edits

def proxy_quality(p,v):
    root=p["root_material"];role=p["role"]
    n=COMPLEXITY_ACTIVE[root["complexity"]]
    levels=[float(v[str(1002+32*i)]) for i in range(n)]
    spectral=[float(v[str(1006+j)])/100 for j in range(4)]
    # A weighted parametric screening target: neither flat/homogeneous
    # harmonic color, inaudible body, nor universally extreme resonance.
    body=levels[0]/100
    contrast=max(spectral)-min(spectral)
    mid=sum(spectral)/4
    base=1.0
    base-=.70*abs(body-.68)
    base-=.14*max(0,.15-contrast)
    base-=.18*max(0,mid-.87)
    base-=.14*max(0,.08-mid)
    base-=.1*max(0,float(v["3001"])/100-.86)
    if n==1:base+=.07
    if root["type"]=="fundamental" and root["complexity"]=="minimal" and sum(spectral[1:])>.65:base-=.1
    if root["type"]=="metallic-spectral" and max(spectral)<.4:base-=.25
    # Structure/provenance: never evaluate for similarity to a named artist.
    return round(clamp(base,0,1),5)

def generate(p,accepted,offer_count=8):
    before=parameters(p["overrides"])
    original=p["overrides"]
    candidates=[]
    for n in range(offer_count):
        v,edits=proposed(p,n)
        score=proxy_quality(p,v)
        nearest=min((distance(feature(v),f) for f in accepted),default=1.)
        # A novelty bonus never overrides missing tone, instrument body or
        # quality. This is NOT a claim about audible distinctness.
        final=score+.12*min(1,nearest/.20)
        candidates.append((final,score,nearest,n,v,edits))
    candidates.sort(key=lambda x:(-x[0],-x[1],-x[2],x[3]))
    overall,score,nearest,choice,winner,edits=candidates[0]
    safe=("1000","1003","1035","1067","1099","6020","6021","6023",
          "6029","6030","6031","6032","3111","3131","9000","9001")
    for k in safe:
        assert winner.get(k)==original.get(k),(p["index"],k)
    assert len([e for e in range(4) if float(winner[str(1002+32*e)])>0])==COMPLEXITY_ACTIVE[p["root_material"]["complexity"]]
    p["overrides"]=winner
    # Runtime-visible metadata must describe the final selected parameters,
    # not pre-search topology or levels from a previous stage.
    terminal=p["concept_tree_terminal"]
    terminal["source_levels"]=[float(winner[str(1002+32*e)]) for e in range(4)]
    terminal["active_engines"]=sum(x>0 for x in terminal["source_levels"])
    terminal["source_enabled"]=[int(winner[str(1001+32*e)]) for e in range(4)]
    terminal["spectral_contour"]=[winner[k] for k in ("3000","3003","3001","3004")]
    terminal["noise"]["audible"]=any(int(winner[str(1000+32*e)])==6 and float(winner[str(1002+32*e)])>0 for e in range(4))
    if "generator_balance" in p:
        p["generator_balance"]["nominal_source_levels"]=terminal["source_levels"]
        p["generator_balance"]["synthesis_methods"]=[int(winner[str(1000+32*e)]) for e in range(4)]
    accepted.append(feature(winner))
    p["instrument_quality_search"]={
       "method":"source-aware divergent generation with role/quality proxy and diversity",
       "candidates_generated":offer_count,"chosen_candidate":choice,
       "proxy_score":score,"nearest_parameter_distance":round(nearest,5),
       "changed_parameters":sum(before.get(k)!=float(winner[k]) for k in winner),
       "parameter_topology_edits":edits,
       "new_runtime_parameter_ids":[],"removed_host_parameter_ids":[],
       "is_audio_validated":False,"is_listening_validated":False,
       "requires_rendered_qa":True}
    return {"index":p["index"],**p["instrument_quality_search"]}

def main():
    arg=argparse.ArgumentParser()
    arg.add_argument("--metadata",required=True,type=Path)
    arg.add_argument("--report",required=True,type=Path)
    arg.add_argument("--candidates",type=int,default=8)
    args=arg.parse_args()
    assert args.candidates==8, "CP66 currently provides exactly eight validated divergent candidate architectures"
    paths=[args.metadata/name for name in BANKS]
    banks=[json.loads(path.read_text(encoding="utf-8")) for path in paths]
    patches=sum(banks,[])
    assert len(patches)==192
    assert [p["index"] for p in patches]==list(range(192))
    accepted=[];report=[generate(p,accepted,args.candidates) for p in patches]
    assert all(r["changed_parameters"]>=6 for r in report)
    assert len({json.dumps(p["overrides"],sort_keys=True) for p in patches})==192
    assert len({p["instrument_quality_search"]["chosen_candidate"] for p in patches})>=4
    for path,bank in zip(paths,banks):
        path.write_text(json.dumps(bank,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    args.report.write_text(json.dumps({
        "checkpoint":"CP66","preset_count":192,
        "candidate_count":192*args.candidates,"generated_not_just_filtered":True,
        "proxy_only":True,"audio_render_validation_pending":True,
        "selection_counts":dict(Counter(r["chosen_candidate"] for r in report)),
        "presets":report},indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    print(f"CP66 candidates: {192*args.candidates} generated, 192 final parameter leaves; audio QA pending")
if __name__=="__main__":main()
