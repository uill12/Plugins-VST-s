#!/usr/bin/env python3
"""CP66 Wolfram cellular synthesis: deterministic generative variations for all 192 Albedo presets.

Run after CP65 retimbre_bioluminal.py. Rules influence oscillator color, envelope
microstructure and motion. MIDI pitch, arpeggiator clock, gate, preset identity
and note ownership are not modified. Generation is a development-time process.
"""
import argparse, hashlib, json, math
from pathlib import Path

BANKS=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
RULES=(30,90,110,150)
ROLES=("Bass","Lead","Pad","Pluck","Keys","Percussion","Drone","Texture")

def evolve(rule, seed, width=32, generations=24):
    state=[(seed>>i)&1 for i in range(width)]
    if not any(state): state[width//2]=1
    history=[]
    for _ in range(generations):
        history.append(state[:])
        state=[(rule>>((state[(i-1)%width]<<2)|(state[i]<<1)|state[(i+1)%width]))&1 for i in range(width)]
    return history

def metrics(rows):
    width=len(rows[0]); total=len(rows)*width
    density=sum(sum(r) for r in rows)/total
    edges=sum(r[i]!=r[(i+1)%width] for r in rows for i in range(width))/total
    activity=sum(rows[t][i]!=rows[t-1][i] for t in range(1,len(rows)) for i in range(width))/((len(rows)-1)*width)
    variation=sum(any(rows[t][i]!=rows[t][0] for i in range(width)) for t in range(len(rows)))/len(rows)
    return density,edges,activity,variation

def bound(v,lo,hi): return round(min(hi,max(lo,float(v))),5)

def update(p):
    index=int(p["index"]); role=p["role"]
    assert role in ROLES
    v=p["overrides"]; before=dict(v)
    # Distinct deterministic initial states. Presets do not all start from one lit cell.
    seed=int.from_bytes(hashlib.sha256((str(index)+":"+role+":"+p["name"]).encode()).digest()[:4],"big")
    ri=ROLES.index(role)
    rule_index=(index//8 + ri + (seed%4))%4
    rule=RULES[rule_index]
    data=evolve(rule,seed)
    density,edges,activity,variation=metrics(data)
    # Rule 30/90/110/150, with four transformations already exposed by the GUI.
    character=1+rule_index+4*((seed>>9)&1)
    v["4003"]=rule_index
    v["6031"]=character
    # Keep arpeggiator availability, timing, note lengths and key maps unchanged.
    preserved=("6020","6021","6022","6023","6024","6025","6026","6027","6028","6029","6030","6032")
    v["4000"]=bound(float(v.get("4000",1))*(.75+.45*activity),.02,24)
    v["4001"]=bound(float(v.get("4001",40))*(.75+.36*edges+.24*variation),0,100)
    v["4002"]=bound(float(v.get("4002",20))*(.72+.45*density),0,100)
    v["4010"]=bound(float(v.get("4010",.7))*(.8+.42*activity),.01,30)
    v["4011"]=(int(v.get("4011",0))+int(4*edges))%5
    v["4012"]=(int(v.get("4012",0))+int(4*activity))%5
    for layer in range(1,4):
        base=1000+layer*32
        band=data[(index+layer*5)%len(data)]
        local=sum(band[layer*6:layer*6+8])/8
        # Harmonic/spectral partials; no note transpose, oscillator gain or root edits.
        for offset,scale in ((6,8.0),(7,7.0),(8,9.0),(9,6.0),(14,5.0)):
            key=str(base+offset)
            if key in v:
                delta=(local-.5)*scale+(activity-.5)*scale*.35
                v[key]=bound(float(v[key])+delta,0,100)
    for k in preserved:
        assert v.get(k)==before.get(k),(index,k)
    for k in ("1003","1035","1067","1099","3111","3131","9000","9001"):
        assert v.get(k)==before.get(k),(index,k)
    assert all(math.isfinite(float(a)) for a in v.values())
    p["wolfram_generation"]={
        "method":"elementary cellular automaton / TNSP permutative resynthesis",
        "rule":rule,"seed":seed,"generations":len(data),"width":len(data[0]),
        "density":round(density,5),"edge_density":round(edges,5),
        "temporal_activity":round(activity,5),"variation":round(variation,5),
        "arpeggiator_character":character,"arpeggiator_enabled_unchanged":True,
        "runtime_clock_unchanged":True,"audition_required":True}
    return {"id":index,"rule":rule,"character":character,"seed":seed,
            "changed_params":sum(v.get(k)!=before.get(k) for k in set(v)|set(before))}

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--metadata",type=Path,required=True)
    parser.add_argument("--report",type=Path,required=True)
    args=parser.parse_args()
    paths=[args.metadata/n for n in BANKS]
    banks=[json.loads(path.read_text(encoding="utf-8")) for path in paths]
    patches=sum(banks,[])
    assert len(patches)==192 and [p["index"] for p in patches]==list(range(192))
    results=[update(p) for p in patches]
    assert len({(r["rule"],r["seed"]) for r in results})==192
    assert set(r["rule"] for r in results)==set(RULES)
    for path,bank in zip(paths,banks):
        path.write_text(json.dumps(bank,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    report={"checkpoint":"CP66","generator":"Wolfram elementary cellular automata + TNSP",
            "generated":192,"rules":[30,90,110,150],
            "midi_arp_off_by_default":"unchanged from previous presets",
            "status":"parameter generation only; DSP render and listening validation pending",
            "presets":results}
    args.report.write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")
    print("CP66: 192 Wolfram-derived parameter resyntheses; clocks and MIDI untouched")

if __name__=="__main__": main()
