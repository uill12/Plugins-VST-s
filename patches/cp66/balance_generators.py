#!/usr/bin/env python3
"""CP66 generator balance & differentiation pass, after CP66 Wolfram generation.

Each preset keeps its original primary method and root pitch. The other three
generators provide distinguishable secondary timbres at audible levels.
No audio render is performed here; energy/correlation QA still required.
"""
import argparse,json,math
from pathlib import Path
FILES=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
ROLES=("Bass","Lead","Pad","Pluck","Keys","Percussion","Drone","Texture")
ROLE_LEVELS={
 "Bass":(77,51,37,25), "Lead":(72,57,41,31),
 "Pad":(66,59,49,38),"Pluck":(76,48,33,22),
 "Keys":(72,53,40,29),"Percussion":(81,41,26,17),
 "Drone":(64,59,50,41),"Texture":(62,60,52,43)}
# Ten methods already supported; do not introduce a new FM synthesis source.
ROLE_METHODS={
 "Bass":(0,8,2,5,7,3), "Lead":(8,2,5,1,7,0),
 "Pad":(3,2,7,5,4,8), "Pluck":(5,2,8,7,0,4),
 "Keys":(2,5,0,7,3,8), "Percussion":(5,6,4,2,7,8),
 "Drone":(3,5,2,7,4,8),"Texture":(3,4,6,9,7,2)}
def clamp(x,lo=0,hi=100):return round(max(lo,min(hi,float(x))),4)
def apply(p):
 v=p["overrides"];role=p["role"];idx=p["index"]
 before={k:v.get(k) for k in ("1000","1003","6020","6030","9000","9001")}
 seed=p["wolfram_generation"]["seed"]
 candidate=ROLE_METHODS[role]
 primary=int(v["1000"])
 used={primary}
 for layer in range(1,4):
  # Repeated engine methods used to yield similar attacks and spectral profiles.
  chosen=next(m for offset in range(len(candidate))
              if (m:=candidate[(offset+idx//8+layer+(seed>>(layer*3))%len(candidate))%len(candidate)]) not in used)
  v[str(1000+32*layer)]=chosen
  used.add(chosen)
 base=ROLE_LEVELS[role]
 for layer in range(4):
  k=str(1002+32*layer)
  # Native values are 0..100 controls (not calibrated loudness).
  # Advance a small role-dependent spread without homogenizing source types.
  variation=((seed>>(layer*5))&7)-3
  v[k]=clamp(base[layer]+variation*1.6,10,91)
  for offset in (6,7,8,9):
   key=str(1000+32*layer+offset)
   if key in v and layer>0:
    # Engine-specific harmonic anatomy, not shared identical knob colors.
    v[key]=clamp(float(v[key])+(((seed>>(offset+layer*3))&15)-7)*1.4)
 # A deliberately balanced source mix still needs bounded FX to avoid masking.
 v["5004"]=clamp(v.get("5004",0),0,29)
 v["5002"]=clamp(v.get("5002",0),0,27)
 for k,val in before.items():assert v.get(k)==val,(idx,k)
 assert len({v[str(1000+32*i)] for i in range(4)})==4
 p["generator_balance"]={"method":"CP66 four-source role-aware generator weighting",
  "nominal_source_levels":[v[str(1002+32*i)] for i in range(4)],
  "synthesis_methods":[v[str(1000+32*i)] for i in range(4)],
  "warning":"levels are control percentages, not measured acoustic energy",
  "render_qa_required":True}
 return {"index":idx,"role":role,**p["generator_balance"]}
def main():
 a=argparse.ArgumentParser();a.add_argument("--metadata",required=True,type=Path)
 a.add_argument("--report",required=True,type=Path);args=a.parse_args()
 paths=[args.metadata/f for f in FILES]
 banks=[json.loads(f.read_text(encoding="utf-8")) for f in paths]
 patches=sum(banks,[])
 assert len(patches)==192
 out=[apply(p) for p in patches]
 for f,b in zip(paths,banks):f.write_text(json.dumps(b,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
 args.report.write_text(json.dumps({"checkpoint":"CP66","count":192,
  "status":"parametric mix hypothesis; not acoustic validation","presets":out},indent=2)+"\n",encoding="utf-8")
 print("CP66: 192 four-generator balances, 4 distinct synthesis methods per patch")
if __name__=="__main__":main()
