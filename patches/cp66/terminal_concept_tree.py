#!/usr/bin/env python3
"""CP66 terminal Concept Tree: 192 intentionally distinct leaves, conscious noise.

Runs AFTER wolfram_generate.py AND balance_generators.py. The 8 role families
branch into 6 synthesis architectures, each into 4 distinct performance
gestures. A preset is a TERMINAL LEAF, never the interchangeable mean of a
cluster. Acoustic similarity must later be assessed from rendered audio.

Method 6 = Noise / Stochastic per the Albedo GUI's ten-method registry.
Other methods are the existing 0..9 set. No new synthesis method or FM.
"""
import argparse, hashlib, json, math
from collections import Counter
from pathlib import Path
FILES=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
ROLES=("Bass","Lead","Pad","Pluck","Keys","Percussion","Drone","Texture")
ARCH=("tonal-core","transient-articulation","breath-excitation",
      "friction-resonance","granular-particles","environmental-bed")
GESTURES=("direct","elastic","recursive","suspended")
# Architectures have different *audible* source topologies, not just labels.
ARCH_METHODS=(
 (0,2,5,8,1,7,3,4,9), # tonal; no noise
 (5,8,2,0,7,4,3,1,9), # strike; noise optional if musically justified
 (3,7,2,5,4,8,9,1,0), # breath carries noise where appropriate
 (5,4,8,7,2,3,9,0,1), # friction
 (3,7,9,4,2,8,5,0,1), # particle field
 (4,3,7,9,2,5,8,0,1), # ambient field
)
# Role-sensitive noise placement, intentionally absent for tonal-only leaves.
NOISE_ALLOW={
 "Bass":{1,3},"Lead":{2,3},"Pad":{2,4,5},"Pluck":{1,3,4},
 "Keys":{1,3},"Percussion":{1,2,3,4},
 "Drone":{2,3,4,5},"Texture":{1,2,3,4,5}}
NOISE_FUNCTION=("none","attack","breath","friction","grain","air")
# Controlled percentages: lower for bass/keys, higher for field textures.
NOISE_BOUNDS={
 "Bass":(6,14),"Lead":(8,18),"Pad":(8,21),"Pluck":(9,22),
 "Keys":(5,15),"Percussion":(11,28),"Drone":(8,23),"Texture":(10,27)}
# Per-role ADSR gestures, varied by one of four endpoint types.
ROLE_ATTACK={
 "Bass":(.002,.014),"Lead":(.004,.035),"Pad":(.10,.65),
 "Pluck":(.001,.010),"Keys":(.003,.034),"Percussion":(.001,.007),
 "Drone":(.18,.80),"Texture":(.05,.55)}
ROLE_DECAY={
 "Bass":(.12,.48),"Lead":(.12,.72),"Pad":(.50,2.4),
 "Pluck":(.09,.55),"Keys":(.14,1.1),"Percussion":(.07,.37),
 "Drone":(.70,2.9),"Texture":(.20,1.5)}
ROLE_RELEASE={
 "Bass":(.12,.75),"Lead":(.16,1.2),"Pad":(1.1,4.8),
 "Pluck":(.10,.95),"Keys":(.17,1.5),"Percussion":(.07,.60),
 "Drone":(1.6,5.9),"Texture":(.55,4.2)}
ROLE_SUSTAIN={
 "Bass":(28,79),"Lead":(35,87),"Pad":(60,94),
 "Pluck":(0,34),"Keys":(24,76),"Percussion":(0,20),
 "Drone":(69,96),"Texture":(34,83)}
def clamp(x,lo,hi):return round(max(lo,min(hi,float(x))),5)
def mix(a,b,t):return a+(b-a)*t
def topology(v,idx,role,a,g,seed):
    original=int(v["1000"])
    candidates=ARCH_METHODS[a]
    secondary=[]
    # Deliberately different methods across all four engines, while retaining
    # stable tonal primary. Method 6 is reserved for assigned noise work.
    for layer in range(1,4):
        for off in range(len(candidates)):
            method=candidates[(off+g+idx//8+layer+(seed>>(layer*4))%7)%len(candidates)]
            if method!=original and method not in secondary:
                secondary.append(method);break
    assert len(set([original]+secondary))==4
    return [original]+secondary
def process(p,ordinal):
    idx=int(p["index"]);role=p["role"];v=p["overrides"]
    assert role in ROLES and "wolfram_generation" in p
    # The ordinal within each role, not the global program ID, spreads the six
    # conceptual architectures evenly across each instrument function.
    architecture=ordinal%6
    gesture=(ordinal//6)%4
    seed=int(p["wolfram_generation"]["seed"])
    before={k:v.get(k) for k in ("1000","1003","1035","1067","1099",
              "6020","6021","6023","6030","9000","9001","3111","3131")}
    methods=topology(v,idx,role,architecture,gesture,seed)
    for e,method in enumerate(methods):v[str(1000+32*e)]=method

    has_noise=architecture in NOISE_ALLOW[role] and int(v["1000"])!=6
    noise_engine=3 if has_noise and methods[3]!=6 else 2
    if has_noise:
        # Normally place stochastic excitation as a supporting engine.
        if methods[noise_engine]==6:
            pass
        else:
            # Replace one supporting engine only, never the primary.
            methods[noise_engine]=6
            if methods[3-noise_engine+1] == 6:
                raise AssertionError("duplicate noise topology")
            v[str(1000+32*noise_engine)]=6
    assert len({v[str(1000+32*e)] for e in range(4)})==4,(idx,methods)

    # If a source preset already uses stochastic primary, retain its identity\n    # but record that as an intentional inherited excitation in this branch.\n    inherited_noise=int(v["1000"])==6\n    # Level distribution is deliberately NOT the same across endpoints.
    # Differentiating structure and excitation comes before level balancing.
    dominance=((seed>>13)&3)
    source_levels=[]
    base_per_arch=(
      (74,49,32,14), (64,60,37,19), (65,52,40,22),
      (61,56,48,20), (60,47,54,28), (56,42,50,39))
    for e in range(4):
        value=base_per_arch[architecture][e]
        if e==dominance and e!=0 and not has_noise:value+=9
        value+=(gesture-1.5)*(-3 if e%2 else 3)
        value+=((seed>>(e*5))&7)-3
        if e==noise_engine and has_noise:
            lo,hi=NOISE_BOUNDS[role]
            value=mix(lo,hi,((seed>>17)&7)/7)
        if e==0 and int(v["1000"])==6:
            # An externally inherited noise primary is not silently replaced;
            # keep its level reasonable and expose the structural caveat.
            value=min(value,45)
        v[str(1002+32*e)]=clamp(value,4,89)
        source_levels.append(v[str(1002+32*e)])
    # Spectral ranges diverge by architectures: different tonal materials,
    # modulation/partial profiles and resonant envelopes per endpoint.
    spectral_base=((11,67,25,72),(81,27,58,14),(39,65,18,82),
                   (71,42,86,24),(27,82,69,38),(55,19,79,61))[architecture]
    for e in range(4):
        b=1000+32*e
        if e==0:continue # preserve original primary spectral fingerprint
        for j,offset in enumerate((6,7,8,9)):
            key=str(b+offset)
            value=spectral_base[(j+e)%4]+(gesture-1.5)*(7 if j%2 else -7)
            value+=((seed>>(j*3+e))&15)-7
            v[key]=clamp(value,3,95)
        v[str(b+14)]=clamp(18+((seed>>(e*7))%62)+gesture*3,4,92)

    def envelope(key,ranges,factor):
        lo,hi=ranges
        v[key]=clamp(mix(lo,hi,factor),lo,hi)
    envelope("3100",ROLE_ATTACK[role],(.10,.44,.79,.97)[gesture])
    envelope("3101",ROLE_DECAY[role],(.18,.76,.43,.91)[gesture])
    envelope("3103",ROLE_RELEASE[role],(.14,.33,.64,.94)[gesture])
    envelope("3102",ROLE_SUSTAIN[role],(.28,.48,.72,.90)[gesture])
    # Resonant contour and spectra vary strongly, without touching pitch.
    cutoff_base=(330,1100,2200,740,3800,1350)[architecture]
    role_scale={"Bass":.7,"Lead":1.7,"Pad":1.3,"Pluck":2.,
                "Keys":1.5,"Percussion":1.9,"Drone":.8,"Texture":1.6}[role]
    v["3000"]=clamp(cutoff_base*role_scale*(.77+.19*gesture),35,16000)
    v["3003"]=clamp(v["3000"]*(1.5+.3*gesture),45,19000)
    v["3001"]=clamp((18,63,29,78,43,22)[architecture]+gesture*3,0,95)
    v["3004"]=clamp((15,21,66,39,75,55)[architecture]-gesture*2,0,95)
    v["3120"]=clamp((-9,72,18,56,39,-24)[architecture]+gesture*6,-100,100)
    v["3121"]=clamp((0,-18,25,38,-28,12)[architecture]+gesture*4,-100,100)
    # Wolfram dynamics can be large or small, but never uncontrolled pitch jitter.
    activity=p["wolfram_generation"]["temporal_activity"]
    v["4000"]=clamp((.25,3.1,.7,1.6,5.2,.11)[architecture]*(1+.38*activity),.02,24)
    v["4001"]=clamp((16,66,45,59,83,28)[architecture]+gesture*3,0,100)
    v["4002"]=clamp((68,17,43,39,11,86)[architecture],0,100)
    # Sensible acoustic space only when it complements instrument function.
    v["5004"]=clamp((7,9,13,12,16,24)[architecture],0,29)
    v["5002"]=clamp((8,5,17,12,24,18)[architecture],0,27)
    # Never add sustained broadband noise to every patch via a wet effect.
    if has_noise:
        # Noise generator profile: shaped by source-specific harmonic and
        # filter controls. Native noise mode does not expose a distinct colored
        # noise spectrum parameter, so do not claim direct pink/brown selection.
        nb=1000+32*noise_engine
        v[str(nb+6)]=clamp((56,67,39,78,46,25)[architecture],0,100)
        v[str(nb+7)]=clamp((17,68,81,44,32,73)[architecture],0,100)
        v[str(nb+8)]=clamp(21+gesture*18,0,100)
        v[str(nb+9)]=clamp(73-gesture*13,0,100)
    for k,val in before.items():
        assert v.get(k)==val,(idx,k)
    assert all(math.isfinite(float(z)) for z in v.values())
    effective_noise=has_noise or inherited_noise\n    p["concept_tree_terminal"]={
      "tree":"function > excitation architecture > performance gesture > unique terminal",
      "leaf_id":f"{role.lower()}-{architecture:02}-{gesture:02}-{idx:03}",
      "role":role,"architecture":ARCH[architecture],
      "gesture":GESTURES[gesture],"wolfram_rule":p["wolfram_generation"]["rule"],
      "noise":{"enabled":effective_noise,"method_id":6 if effective_noise else None,
               "engine":0 if inherited_noise else (noise_engine if has_noise else None),
               "function":NOISE_FUNCTION[architecture] if has_noise else ("inherited-primary-excitation" if inherited_noise else "none"),
               "level_control":v[str(1002+32*(0 if inherited_noise else noise_engine))] if effective_noise else 0},
      "synthesis_methods":[int(v[str(1000+32*e)]) for e in range(4)],
      "source_levels":source_levels,
      "spectral_contour":[v["3000"],v["3003"],v["3001"],v["3004"]],
      "audition_and_similarity_qa_required":True}
    return p["concept_tree_terminal"]

def main():
    a=argparse.ArgumentParser()
    a.add_argument("--metadata",required=True,type=Path)
    a.add_argument("--report",required=True,type=Path)
    args=a.parse_args()
    paths=[args.metadata/f for f in FILES]
    banks=[json.loads(f.read_text(encoding="utf-8")) for f in paths]
    patches=sum(banks,[])
    assert len(patches)==192 and [p["index"] for p in patches]==list(range(192))
    ordinals=Counter()
    report=[]
    for p in patches:
        role=p["role"];position=ordinals[role];ordinals[role]+=1
        report.append({"index":p["index"],**process(p,position)})
    keys=[p["leaf_id"] for p in report]\n    assert len(set(keys))==192
    assert len({p["index"] for p in report})==192
    assert all(p["noise"]["enabled"] == (6 in p["synthesis_methods"]) for p in report)
    for path,bank in zip(paths,banks):
        path.write_text(json.dumps(bank,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    args.report.write_text(json.dumps({
      "checkpoint":"CP66","generator":"TNSP terminal-branch differentiation and conscious noise",
      "preset_count":192,"role_counts":dict(ordinals),
      "terminal_keys_unique":len(set(keys))==192,
      "noise_used_count":sum(x["noise"]["enabled"] for x in report),
      "noise_off_count":sum(not x["noise"]["enabled"] for x in report),
      "status":"parameter-level synthesis only; real-audio feature and listening QA required",
      "presets":report},ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    print(f"CP66: 192 terminal branches / {sum(x['noise']['enabled'] for x in report)} intentional noise excitations")

if __name__=="__main__":main()
