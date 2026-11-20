#!/usr/bin/env python3
"""CP51 Morph Tree: an OFFLINE SOUND-GENERATION PROCESS, never a GUI navigation tree.

source/influence -> instrument archetype -> spectral color -> performance function
-> compatible transformations -> engine parameters -> offline render/evaluate/selection.

All material is synthesized from Albedo's pre-existing 32 internal PCM materials
and DSP engines. No copyrighted artist samples, riffs, recordings or presets.
The first 128 names and indices are retained for DAW session compatibility.
"""
from __future__ import annotations
import argparse, copy, hashlib, json, math, random
from pathlib import Path

ROLE = ("Bass","Lead","Pad","Pluck","Keys","Percussion","Drone","Texture")
SOURCE = ("Max Cooper","Barker","Vril","Grimes")
# Artistic references describe GENERATIVE constraints, not exact sonic copies.
FAMILY = ("Fractal Organisms","Velocity Without Drums","Subterranean Matter","Iridescent Systems")
COLOR = ("submerged","rounded","woody","dark","velvet","airy","crystalline","metallic",
         "percussive","grainy","silken","hollow","electrical","granular","warm","wide")
FUNCTION = {"Bass":"low foundation","Lead":"melodic foreground","Pad":"harmonic bed",
            "Pluck":"articulated counterpoint","Keys":"polyphonic articulation",
            "Percussion":"pitched/noisy accent","Drone":"sustained field",
            "Texture":"evolving spectral layer"}
INSTRUMENT = {
"Bass":("analog voltage bass","reed suborgan","plucked low string","resonant bass membrane"),
"Lead":("phase-distortion reed","folded brass","formant oscillator","modal electric voice"),
"Pad":("analog string ensemble","granular cloud","hollow reed choir","slow harmonic resonator"),
"Pluck":("thumb piano","metallic tine","short resonant string","folded electric pluck"),
"Keys":("electric piano","organ stops","ceramic keys","polysynth keys"),
"Percussion":("pitched metal impact","wood and membrane","dry circuit click","short noise-resonance"),
"Drone":("low reed continuum","spectral chamber","bowed alloy","granular slow mass"),
"Texture":("cellular dust","vowel particulate","air column","spectral fragments")}
# DSP method IDs 0...9: analog, vector, additive, granular, spectral,
# modal, noise, resampled, phase-distortion, cellular.
ENGINE = {
"Bass":((0,1,5,8),(0,5,2,1)),
"Lead":((8,2,1,0),(2,4,6,1)),
"Pad":((0,3,2,7),(3,2,7,1)),
"Pluck":((5,2,8,1),(2,3,0,7)),
"Keys":((2,0,5,1),(0,2,7,3)),
"Percussion":((5,6,4,9),(2,3,7,1)),
"Drone":((2,7,4,0),(3,6,0,7)),
"Texture":((9,3,6,4),(7,4,2,1))}
TREATMENT = {
"Max Cooper":("fractal articulation","reciprocal timbre","cellular microvariation","spectral branching"),
"Barker":("rolling phase","harmonic negative space","elastic low motion","repeating resonance"),
"Vril":("saturated fundamental","warm low-mid damping","tape-like movement","deep resonant air"),
"Grimes":("crystalline halo","vocal formant color","iridescent motion","glassy transient")}
GATES = ("slow envelope","short envelope","filter motion","sample-position morph",
         "cellular timbre","modal damping","phase fold","spectral tilt")

def stable(*parts):
    data="|".join(str(v) for v in parts).encode("utf8")
    return int.from_bytes(hashlib.sha256(data).digest()[:8],"big")
def clip(x,lo,hi): return min(hi,max(lo,x))
def setv(p,id,value):p[str(id)]=round(value,6) if isinstance(value,float) else int(value)
def evolve(seed_row,index,variant=0):
    """Deterministic branching: every candidate uses genuinely different DSP settings."""
    row=copy.deepcopy(seed_row)
    role=row["role"]
    if role not in ROLE: raise ValueError(role)
    # 64 existing material, 64 existing creative, 64 new; preserve identifiers.
    is_new=index>=128
    group=(index-128)//16 if is_new else (index//8)%4
    branch=(index-128)%16 if is_new else index%16
    source=SOURCE[group]
    rng=random.Random(stable("Albedo CP51",index,variant))
    color=COLOR[(index*5+variant*7+group)%len(COLOR)]
    instrument=INSTRUMENT[role][(branch//2+variant)%4]
    technique=TREATMENT[source][(branch+variant)%4]
    movement=GATES[(index+variant*3)%len(GATES)]
    # For each new role, its two branches must explore DIFFERENT primary
    # synthesis engines; avoid the modulo-8 alias that collapsed both to one.
    if is_new:
        engine=ENGINE[role][0][(group+2*(branch//8)+variant)%4]
        secondary=ENGINE[role][1][(group+(branch//8)+variant)%4]
        third=ENGINE[role][1][(2*group+(branch//8)+variant+2)%4]
    else:
        engine=ENGINE[role][0][(branch+variant)%4]
        secondary=ENGINE[role][1][(branch//2+variant)%4]
        third=ENGINE[role][1][(branch+variant+2)%4]
    if secondary==engine:secondary=(secondary+2)%10
    if third==engine:third=(third+3)%10
    p=row["overrides"]
    if not isinstance(p,dict):raise ValueError(index)
    primary_level={"Bass":90,"Lead":79,"Pad":70,"Pluck":79,"Keys":74,"Percussion":84,"Drone":73,"Texture":66}[role]
    secondary_level={"Bass":39,"Lead":46,"Pad":54,"Pluck":30,"Keys":44,"Percussion":26,"Drone":52,"Texture":49}[role]
    base_cut={"Bass":440,"Lead":2400,"Pad":1550,"Pluck":2450,"Keys":1750,
              "Percussion":3900,"Drone":830,"Texture":2400}[role]
    for part,(method,level) in enumerate(((engine,primary_level),(secondary,secondary_level),
                                          (third,24),(7 if role in ("Pad","Drone","Texture") else 2,12))):
        a=1000+part*32
        setv(p,a,method)
        setv(p,a+1,1)
        # Allocate energy across oscillators: primary = body, second = color,
        # third/fourth = accents. Avoid four 90%-gain oscillators saturating.
        setv(p,a+2,clip(level+rng.randrange(-8,9)+variant*2,5,99))
        setv(p,a+3,0 if not part else (-12 if role=="Bass" and part==1 else (0,7,12,-5)[(part+index+variant)%4]))
        setv(p,a+4,0 if not part or role=="Bass" else (-32,26,12,-18)[(index+part)%4])
        setv(p,a+6,clip((11+index*7+part*17+variant*19)%100,5,96))
        setv(p,a+7,clip((37+index*11+part*19+variant*13)%100,4,96))
        setv(p,a+8,clip((69+index*13+part*29+variant*17)%100,3,96))
        setv(p,a+9,clip((27+index*3+part*11+variant*23)%100,4,95))
        setv(p,a+10,(index*7+part*9+variant*5)%32)
        setv(p,a+11,1 if role in ("Bass","Percussion","Pluck") else 3 if part==0 else 1)
        setv(p,a+16,(index+part*7+variant*11)%32)
        setv(p,a+18,(index*3+part*11+variant*17)%100)
    setv(p,3000,clip(base_cut*(.65+.22*((branch+variant)%5)),90,16000))
    setv(p,3003,clip(base_cut*(1.65+.32*((branch+variant*2)%5)),170,18000))
    setv(p,3001,clip(15+((index*11+variant*17)%45),4,72))
    setv(p,3010,(index+variant)%5);setv(p,3011,(index//3+variant)%5)
    setv(p,2004,0 if role=="Bass" else 34 if role=="Percussion" else clip(56+(index*7+variant*13)%40,0,96))
    setv(p,5004,12 if role=="Bass" else 7 if role=="Percussion" else 15+((index+variant*7)%30))
    setv(p,2100,(index*17+variant*29)%100)
    setv(p,2101,(index*29+variant*23)%100)
    setv(p,4003,(index+variant)%4)
    setv(p,6020,1 if role in ("Pluck","Percussion","Texture") and index%5==0 else 0)
    setv(p,6024,(index+variant)%4);setv(p,6025,1 if role=="Bass" else 1+(index%2))
    if role=="Bass":
        setv(p,1019,clip(42+variant*5+index%20,30,82))
        setv(p,1035,-12)
    if role in ("Bass","Drone","Pad","Keys"):
        setv(p,3102,clip(65+(index+variant*7)%26,55,91))
    if role in ("Percussion","Pluck"):
        setv(p,3100,.002+.002*((index+variant)%4))
        setv(p,3101,.10+.06*((index+variant)%4))
        setv(p,3103,.12+.08*((index+variant)%4))
    if role in ("Pad","Drone","Texture"):
        setv(p,3100,.07+.12*((index+variant)%4))
        setv(p,3103,.5+.25*((index+variant)%4))
    # Studio-safe initial gain is *not* the final loudness solution:
    # final offline render derives individual trims with peak protection.
    setv(p,9000,clip(float(p.get("9000",-6)),-18,3))
    setv(p,9003,clip(float(p.get("9003",-3)),-12,3))
    # Idea graph is PROVENANCE used by generation/selection, not a GUI menu.
    row["morphTree"]={
        "generation":"source > instrument > character > function > parameter branch > sound test",
        "referenceStyle":source, "inspirationDetail": "Transcendental Tree Map: fractal branching" if source=="Max Cooper" and branch%2==0 else source+" sound-design direction",
        "crossInfluence":"Kauá Brava" if index%4==0 else None,
        "instrument":instrument,"color":color,"function":FUNCTION[role],
        "permittedTransformations":list(TREATMENT[source]),"chosenTransformation":technique,
        "movement":movement,"primaryEngine":engine,"secondaryEngine":secondary,
        "rootMaterial":int(p["1010"]),"lowEndStrategy":"centered fundamental / upper harmonic spread" if role=="Bass" else "full-band transient, spectral body, controlled side",
        "variant":variant}
    if is_new:
        row["name"]=f"{('Fractal','Kinetic','Subsoil','Prismatic')[group]} {('Root','Aperture','Lattice','Memory','Hollow','Carbon','Silk','Wave')[branch%8]} {('Field','Motion')[branch//8]}"
        row["family"]=FAMILY[group]
        row["character"]=f"{color} / {technique}"
        row["tone"]=f"{instrument}; {FUNCTION[role]}"
        row["description"]=f"Original {role.lower()} sound using {technique}, {color} color and a controlled low-mid body."
        row["baseIndex"]=(index*17+group*13)%64
    else:
        # Names and role/family remain stable across host DAW sessions.
        row["character"]=f"{color} / {technique}"
    row["index"]=index
    row["ideaPath"]=[row["morphTree"][k] for k in ("referenceStyle","instrument","color","function","chosenTransformation")]
    row["voicing"]=f"{instrument} / {color} / {technique}"
    return row

def generate(seed_metadata:Path,destination:Path,variant:int):
    existing=[]
    for name in ("material_factory_bank.json","recursive_forms_bank.json"):
        existing+=json.loads((seed_metadata/name).read_text(encoding="utf8"))
    assert len(existing)==128 and [p["index"] for p in existing]==list(range(128))
    outputs=[evolve(p,p["index"],variant) for p in existing]
    for i in range(128,192):
        role=ROLE[(i-128)%8]
        sources=[r for r in existing if r["role"]==role]
        parent=sources[stable("base",i,variant)%len(sources)]
        outputs.append(evolve(parent,i,variant))
    assert len({p["name"] for p in outputs})==192, "Preset names must be unique"
    destination.mkdir(parents=True,exist_ok=True)
    for filename,rows in (("material_factory_bank.json",outputs[:64]),
                          ("recursive_forms_bank.json",outputs[64:128]),
                          ("morph_expansion_bank.json",outputs[128:])):
        (destination/filename).write_text(json.dumps(rows,indent=1,ensure_ascii=False)+"\n",encoding="utf8")
    counts={k:sum(p["role"]==k for p in outputs) for k in ROLE}
    manifest={"version":"CP51","candidate":variant,"count":192,"added":64,
              "sourcePools":SOURCE,"roleCounts":counts,
              "license":"DSP synthesis and internal bank only; no third-party music samples",
              "selectionStatus":"unselected candidate; render/measure before release"}
    (destination/"morph_candidate_manifest.json").write_text(json.dumps(manifest,indent=2)+"\n",encoding="utf8")
    return outputs

if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--seed",type=Path,default=Path("Albedo-JUCE-Native/Metadata"))
    parser.add_argument("--out",type=Path,required=True)
    parser.add_argument("--variant",type=int,choices=range(4),default=0)
    args=parser.parse_args()
    result=generate(args.seed,args.out,args.variant)
    print(f"CP51 generated {len(result)} distinct-ID candidates ({len(result)-128} new) variant {args.variant}")
