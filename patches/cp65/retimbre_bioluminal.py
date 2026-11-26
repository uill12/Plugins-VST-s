#!/usr/bin/env python3
"""CP65 TNSP: generate a coherent 192-sound bio-cyber/fluorescent Albedo atlas.

Runs AFTER the stable CP51 selection, CP59 WaveEdit branching and CP60 FX
balancing. Similarity is used to preserve each role's fundamental / dynamic
signature; permutations create new supporting octave, resonant, cellular and
transient branches. This script generates parameter changes, NOT merely labels
or a nearest-neighbour filter. It does not touch the stable CP64R1 release.
"""
from __future__ import annotations
import argparse, hashlib, json, math
from collections import Counter
from pathlib import Path

BANKS=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
ROLES=("Bass","Lead","Pad","Pluck","Keys","Percussion","Drone","Texture")
NAMES=(
    "Velvet Mycelium","Arcadia Bloom","Glass Orchard","Moss Relay",
    "Solstice Keys","Copper Pollen","Soft Cathedral","Iridescent Spores",
    "Orchid Relay","Harmonic Vine","Marble Canopy","Feral Chimes",
    "Amber Harpsichord","Cobalt Petals","Verdant Nocturne","Porcelain Current",
    "Elastic Heart","Silvergrass Pulse","Renaissance Circuit","Bouncing Parhelion",
    "Lantern Sonata","Violet Metronome","Feather Dynamo","Petal Engine",
    "Fresco Collider","Dancing Stems","Electric Minuet","Silken Spring",
    "Basilica Bells","Chrome Lullaby","Aurora Mechanism","Saffron Echo",
    "Rootwork Choir","Midnight Filament","Chapel Underwater","Emerald Marbles",
    "Blackwater Organ","Limestone Flicker","Sleeping Machines","Subsoil Firefly",
    "Stonewisp Pulse","Indigo Compass","Strata Bloom","Cinder Harp",
    "Terra Madrigal","Quartz Pebbles","Nocturnal Vault","Echoing Tunnels",
    "Prism Garden","Lucent Aureole","Neon Fresco","Beryl Carousel",
    "Everbloom Piano","Cerulean Drops","Luminous Archive","Magenta Circuit",
    "Starling Glass","Blooming Atlas","Golden Palisade","Tesserae Chimes",
    "Heliotrope Piano","Solar Pearls","Chroma Basilica","Dawn Weaving")
assert len(NAMES)==len(set(NAMES))==64

# Anchor pitch, stereo and articulation by role; each voice has a distinct
# exciter, harmonic anatomy and envelope, not an indiscriminate reverb wash.
PROFILES={
 "Bass": dict(voices=(0,8,2,5),cut=(310,1260),attack=(.003,.014),decay=(.12,.39),
              sustain=(39,78),release=(.18,.68),octaves=((-12,12,7),(-12,12,24),(-12,7,12)),
              chorus=(0,9),room=(3,11),motion=(.55,2.1),filter_env=(28,63)),
 "Lead": dict(voices=(8,2,5,1),cut=(1150,4700),attack=(.007,.040),decay=(.14,.43),
              sustain=(51,86),release=(.21,1.10),octaves=((12,7,24),(-12,12,7),(7,24,12)),
              chorus=(8,25),room=(6,19),motion=(.65,4.7),filter_env=(29,70)),
 "Pad": dict(voices=(3,2,7,5),cut=(670,4100),attack=(.085,.65),decay=(.55,2.4),
             sustain=(60,94),release=(1.3,4.7),octaves=((-12,12,24),(12,-12,7),(7,24,-12)),
             chorus=(10,29),room=(13,35),motion=(.11,1.4),filter_env=(13,48)),
 "Pluck": dict(voices=(5,2,8,7),cut=(840,6700),attack=(.002,.011),decay=(.11,.61),
               sustain=(5,33),release=(.12,.85),octaves=((12,24,7),(-12,12,24),(7,12,-12)),
               chorus=(0,14),room=(4,17),motion=(.7,5.),filter_env=(41,82)),
 "Keys": dict(voices=(2,5,0,7),cut=(1050,5400),attack=(.004,.028),decay=(.17,.82),
              sustain=(28,76),release=(.20,1.4),octaves=((-12,12,7),(12,24,7),(-12,7,24)),
              chorus=(6,19),room=(7,24),motion=(.35,2.7),filter_env=(25,65)),
 "Percussion": dict(voices=(5,6,4,2),cut=(550,9000),attack=(.001,.009),decay=(.065,.38),
                    sustain=(0,16),release=(.08,.48),octaves=((12,-12,7),(-12,7,24),(7,12,-12)),
                    chorus=(0,7),room=(1,14),motion=(.6,4.5),filter_env=(33,83)),
 "Drone": dict(voices=(3,5,2,7),cut=(230,2900),attack=(.15,.78),decay=(.65,2.7),
               sustain=(71,96),release=(1.8,5.9),octaves=((-12,12,24),(-12,7,12),(12,-12,7)),
               chorus=(13,32),room=(13,31),motion=(.05,.9),filter_env=(8,38)),
 "Texture": dict(voices=(3,4,6,9),cut=(490,7800),attack=(.06,.52),decay=(.39,1.9),
                 sustain=(37,85),release=(.85,3.9),octaves=((12,24,-12),(7,-12,24),(-12,12,7)),
                 chorus=(5,27),room=(12,31),motion=(.12,3.2),filter_env=(15,57)),
}
GESTURES=("elastic pulse","botanical growth","fluorescent filigree","recursive breath",
          "resonant renaissance","prismatic emergence")
LAYERS=("copper","glass","moss","reed","tesserae","iridescent","porcelain","electric")

def lerp(a,b,t):return a+(b-a)*t
def fclip(v,lo,hi):return round(max(lo,min(hi,float(v))),5)
def hash_values(x):
 return hashlib.sha256(json.dumps(x,sort_keys=True,separators=(",",":")).encode()).hexdigest()

def mutate(p):
 idx=p["index"];role=p["role"]
 if role not in PROFILES:raise ValueError(("missing role",idx,role))
 v=p["overrides"];before={k:float(val) for k,val in v.items()}
 profile=PROFILES[role];ri=ROLES.index(role)
 # Distinct branches at the same played note, with a common organic
 # harmonic palette. No randomness, no required arp, no FM, no pitch drift.
 b=(idx*7+ri*3+idx//16)%len(GESTURES)
 tone=(idx*11+ri*17+idx//8)%101/100.
 morph=(idx*13+ri*19+idx//24)%97/96.
 phase=(idx*5+ri*7+idx//16)%3
 gest=GESTURES[b]
 outline=(idx//8+ri)%4
 octave=profile["octaves"][phase]
 original_primary=int(v.get("1000",1))
 # Keep the original spectral/fundamental exciter: its role and source ID
 # are the partial-invariant edge shared with the old validated patch.
 # Replace only supporting exciter topologies; 4 engines still coexist.
 roles=profile["voices"]
 v["1032"]=roles[(b+1)%4]
 v["1064"]=roles[(b+2)%4]
 v["1096"]=roles[(b+3)%4]
 if original_primary==v["1032"]:v["1032"]=roles[(b+2)%4]
 if v["1064"]==v["1032"]:v["1064"]=roles[(b+3)%4]
 # Harmonic register: original MIDI pitch remains the fundamental.
 # All pitches remain within -24..+24 semitones and follow keytracking.
 for e,tune in enumerate((int(v.get("1003",0)),*octave)):
  base=1000+e*32
  v[str(base+3)]=int(max(-24,min(24,tune if e else int(v.get("1003",0)))))
  v[str(base+5)]=1
  if e:v[str(base+4)]=(-1 if e%2 else 1)*int(9+22*morph+e*5)
  else:v[str(base+4)]=0
  # Preserve the recognizable primary body; supporting octaves occupy
  # successively smaller spectral energy roles, preventing large gain jumps.
  if e==0:
   v[str(base+2)]=fclip(lerp(83,96,.65*tone+.35*morph),65,98)
  else:
   low,high=((36,58),(18,39),(8,25))[e-1]
   v[str(base+2)]=fclip(lerp(low,high,(tone+e*.21)%1),5,65)
  v[str(base+11)]=1 if e in (0,3) else (2 if (idx+e)%5==0 else 1)
  v[str(base+12)]=fclip(6+9*morph+(idx+e)%5,0,28)
  v[str(base+13)]=fclip(35+25*tone+e*6,0,85)
  v[str(base+20)]=0 # avoid unintentional per-note pitch instability
  v[str(base+21)]=0
  # Palette: alternating glass-like upper partials, porous organic
  # texture and spring-loaded transients; never uniform 4-layer shimmer.
  if e>0:
   v[str(base+6)]=fclip(24+(17*b+19*e+idx*3)%64,0,100)
   v[str(base+7)]=fclip(16+(11*b+13*e+idx*5)%65,0,100)
   v[str(base+8)]=fclip(18+(7*b+17*e+idx*11)%70,0,100)
   v[str(base+9)]=fclip(14+(13*b+7*e+idx*2)%67,0,100)
   if v[str(base)] in (1,3,7):v[str(base+10)]=32+(idx*5+ri*7+e*11)%24
  v[str(base+14)]=fclip(24+((idx*5+11*e+ri*7)%63),0,100)
 # Encourage structural motion; contrast recursive modes, never compulsory
 # on-beat arpeggiation, always keep the user's note attack at the MIDI input.
 v["4000"]=fclip(lerp(*profile["motion"],.28+.61*morph),.02,24)
 v["4001"]=fclip(lerp(18,64,(tone+.23*b)%1),0,100)
 v["4002"]=fclip(lerp(16,65,morph),0,100)
 v["4003"]=(idx+ri+b)%4
 v["4010"]=fclip(.09+.19*((idx+ri)%8)+.41*tone,.01,30)
 v["4011"]=(idx+ri)%5
 v["4012"]=(idx//3+b)%5
 # Stable octave orchestration: timbral motion, not analog pitch flutter.
 v["3131"]=0
 v["3111"]=0 # polyphony; no legato/mono voice stealing
 v["3010"]=int(v.get("3010",0))%5
 v["3011"]=(idx//5+b)%5
 v["3012"]=(idx//13+b)%3
 low,high=profile["cut"]
 v["3000"]=fclip(lerp(low,high,.22+.68*tone),20,20000)
 v["3003"]=fclip(min(19000,v["3000"]*(1.58+.62*morph)),20,20000)
 v["3001"]=fclip(lerp(16,44,morph),0,100)
 v["3004"]=fclip(lerp(10,37,tone),0,100)
 v["3002"]=fclip(lerp(1.5,6.7,morph),0,24)
 v["3005"]=fclip(lerp(.4,4.5,tone),0,24)
 for k,(lo,hi) in {"3100":profile["attack"],"3101":profile["decay"],
                    "3102":profile["sustain"],"3103":profile["release"]}.items():
  v[k]=fclip(lerp(lo,hi,(morph+.37*tone+(int(k)%7)*.08)%1),lo,hi)
 v["3120"]=fclip(lerp(*profile["filter_env"],morph),-100,100)
 v["3121"]=fclip(lerp(-25,39,tone),-100,100)
 v["3122"]=fclip(max(.002,v["3100"]*.73),.001,5)
 v["3123"]=fclip(max(.03,v["3101"]*.79),.01,10)
 v["3124"]=fclip(max(0,v["3102"]*.44),0,100)
 v["3125"]=fclip(max(.06,v["3103"]*.75),.01,12)
 # Shared color bounds, role-adjusted. Never use identical global wet mix to
 # force false cohesion or bury percussive transients.
 v["5000"]=fclip(lerp(7,22,tone),0,100);v["5001"]=1
 v["5002"]=fclip(lerp(*profile["chorus"],morph),0,100)
 v["5003"]=int(v["5002"]>2)
 v["5004"]=fclip(lerp(*profile["room"],tone),0,100)
 v["5005"]=int(v["5004"]>2)
 v["5006"]=fclip(lerp(2,16,morph),0,100);v["5007"]=int((idx+ri)%4==0)
 v["5008"]=fclip(.08+.07*((idx+ri)%7),.02,6)
 v["5009"]=fclip(35+28*morph,0,100)
 v["5010"]=fclip(59+19*tone,0,100)
 v["5011"]=fclip(72+33*morph,40,140)
 v["5012"]=fclip(-12+24*tone,-60,60)
 # Preserve stable MIDI routing, mode, tempo, gate masks and global trim.
 for k in ("6020","6021","6022","6023","6024","6025","6026","6027",
           "6030","6031","6032","9000","9001","9002","9003"):
  if k in before:assert float(v[k])==before[k],(idx,k)
 # Preserve real playable notes, never bake a sequence, no new audio assets.
 if idx>=128:
  p["name"]=NAMES[idx-128]
  if "Field" in p["name"] or "Motion" in p["name"]:
   raise RuntimeError("concept placeholder leaked into creative name")
 p["bio_cyber_branch"]={
  "method":"TNSP / element-based partial similarity and recursive permutation",
  "palette":"bio-cyber fluorescent / neo-renaissance alternative electronica",
  "shared_invariant":"original primary generator and MIDI fundamental",
  "component_to_preserve":"original fundamental and instrument role",
  "component_to_permute":"secondary exciter / octave harmonics / articulation / color / cellular motion",
  "gesture":gest,"material_reference":LAYERS[(idx+b+ri)%len(LAYERS)],
  "octaves":list(octave),"neighbor":(idx-128)^1 if idx>=128 else idx^1,
  "generation_seed":65001+idx,"pitch_stable":True,"user_audition_pending":True}
 p["character"]=(gest+" / "+LAYERS[(idx+b+ri)%len(LAYERS)]) if idx>=128 else p.get("character",gest)
 if idx>=128:
  p["tone"]=role.lower()+" / "+gest+" / harmonic register expansion"
  p["description"]=f"{role}: {gest}, {LAYERS[(idx+b+ri)%len(LAYERS)]} overtones, {octave} supporting-semitone layers; original tonal root retained"
 after={k:float(val) for k,val in v.items()}
 changes={k:[before.get(k),value] for k,value in after.items() if before.get(k)!=value}
 if len(changes)<25:raise AssertionError(("too few synthesis changes",idx,len(changes)))
 for k,value in after.items():
  if not math.isfinite(value):raise ValueError(("nonfinite",idx,k))
 return {"id":idx,"role":role,"name":p["name"],"gesture":gest,
         "layer":p["bio_cyber_branch"]["material_reference"],"octaves":list(octave),
         "changes":len(changes),"original_primary":original_primary,
         "before_sha256":hash_values(before),"after_sha256":hash_values(after),
         "changed_parameter_ids":sorted(map(int,changes)), "midi_clock_untouched":True}

def main():
 parser=argparse.ArgumentParser()
 parser.add_argument("--metadata",required=True,type=Path)
 parser.add_argument("--report",required=True,type=Path)
 args=parser.parse_args()
 banks=[args.metadata/name for name in BANKS]
 rows=[json.loads(f.read_text(encoding="utf-8")) for f in banks]
 assert [len(b) for b in rows]==[64,64,64]
 allp=sum(rows,[])
 assert [p["index"] for p in allp]==list(range(192))
 original_names=[p["name"] for p in allp[:128]]
 report=[mutate(p) for p in allp]
 assert [p["name"] for p in allp[:128]]==original_names
 assert len(set(p["name"] for p in allp[128:]))==64
 assert set(p["role"] for p in allp)==set(ROLES)
 assert all(sum(p["role"]==r for p in allp)>=20 for r in ROLES)
 assert len({hash_values(p["overrides"]) for p in allp})==192
 for file,data in zip(banks,rows):
  file.write_text(json.dumps(data,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
 args.report.write_text(json.dumps({
  "checkpoint":"CP65R1","generation_method":"TNSP partial-component similarity plus branch permutations",
  "aesthetic":"Bio-Cyber Fluorescent / Elastic Neo-Renaissance Alternative Electronica",
  "sound_slots":192,"revoiced":192,"morphogenic_renamed":64,
  "original_factory_names_unchanged":128,"host_program_indices_preserved":True,
  "bank_display":"ALBEDO · BIOLUMINAL ATLAS","role_counts":dict(Counter(p["role"] for p in allp)),
  "selection_status":"parameter-generated; full DSP render + level match/audition required",
  "presets":report},ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
 print("CP65 TNSP PASS: 192/192 multi-parameter revoicings; 64 creative names; preserved 192 IDs, no forced ARP")

if __name__=="__main__":main()
