#!/usr/bin/env python3
"""CP51 offline selection: choose morph-tree permutations by REAL DSP renders.

Measures low/body frequency content, temporal/spectral distinctness, mono-safe
stereo width, oscillator-to-oscillator balance, peak and per-program loudness.
The same deterministic renderer feeds inverse WAV matching (no source audio copied).
"""
from __future__ import annotations
import argparse, json, math, subprocess, itertools
from pathlib import Path
import numpy as np

N=48000
ROLES={"Bass":-19.5,"Lead":-20.5,"Pad":-22.0,"Pluck":-22.0,
       "Keys":-21.0,"Percussion":-23.0,"Drone":-22.5,"Texture":-23.0}
EDGE=np.geomspace(32,17500,30)
BANDS=np.fft.rfftfreq(4096,1/48000)
INTERVAL=np.searchsorted(BANDS,EDGE)
EPS=1e-12

class Renderer:
    def __init__(self,exe):
        self.proc=subprocess.Popen([str(Path(exe).resolve())],
            stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.DEVNULL,bufsize=0)
    def render(self,slot,settings,note=48,total=N,held=31500):
        command=f"{slot} {note} .77 {held} {total} 0 "+' '.join(
            f"{int(k)} {float(v):.9g}" for k,v in sorted(settings.items(),key=lambda x:int(x[0])))+"\n"
        self.proc.stdin.write(command.encode("ascii"));self.proc.stdin.flush()
        remaining=total*8;blocks=[]
        while remaining:
            block=self.proc.stdout.read(min(remaining,512*1024))
            if not block:raise RuntimeError(f"Renderer terminated while rendering slot {slot}")
            remaining-=len(block);blocks.append(block)
        audio=np.frombuffer(b"".join(blocks),dtype="<f4").reshape(total,2)
        if not np.isfinite(audio).all():raise RuntimeError(f"Non-finite audio at slot {slot}")
        return audio
    def close(self):
        if self.proc.poll() is None:
            self.proc.stdin.close();self.proc.wait(timeout=8)

def measurements(audio):
    x=audio.astype(float)
    rms=np.sqrt(np.mean(x*x)+EPS)
    peak=float(np.max(abs(x)))
    mid=(x[:,0]+x[:,1])/2
    side=(x[:,0]-x[:,1])/2
    corr=float(np.corrcoef(x.T)[0,1]) if np.std(x[:,0])>1e-9 and np.std(x[:,1])>1e-9 else 1.
    width=10*math.log10((np.mean(side*side)+EPS)/(np.mean(mid*mid)+EPS))
    windows=[mid[i:i+4096] for i in (2500,9000,19000,30000,38000)]
    frames=np.stack(windows)*np.hanning(4096)
    spec=(abs(np.fft.rfft(frames,axis=1))**2).mean(axis=0)
    hz=BANDS
    low=float(spec[(hz>=35)&(hz<180)].sum())
    body=float(spec[(hz>=180)&(hz<1500)].sum())
    air=float(spec[(hz>=1500)&(hz<16000)].sum())
    total=low+body+air+EPS
    perband=np.array([spec[a:max(a+1,b)].sum() for a,b in zip(INTERVAL[:-1],INTERVAL[1:])])
    perband=np.maximum(-58,10*np.log10((perband+EPS)/(perband.max()+EPS)))
    envelope=np.sqrt(np.mean(x[:N//480*480].reshape(-1,480,2)**2,axis=(1,2))+EPS)
    envdb=np.clip(20*np.log10((envelope+EPS)/(envelope.max()+EPS)),-58,0)
    temporal=np.array([envdb[i:j].mean() for i,j in ((0,6),(6,18),(18,35),(35,61),(61,90))])
    features=np.r_[perband/34,temporal/38,np.array([width/35,corr/3,low/total,body/total,air/total])]
    return dict(rms_db=20*math.log10(rms),peak_db=20*math.log10(max(peak,EPS)),
                low_fraction=low/total,body_fraction=body/total,air_fraction=air/total,
                width_db=width,corr=corr,feature=features.tolist())

def render_metrics(engine,slot,patch):
    # Audition the underlying tone, not the arpeggiated sequence.
    p={**patch["overrides"],"6020":0}
    low=measurements(engine.render(slot,p,note=48))
    high=measurements(engine.render(slot,p,note=67))
    feat=np.r_[low["feature"],high["feature"]]
    return dict(low=low,high=high,feature=feat,peak_db=max(low["peak_db"],high["peak_db"]),
                rms_db=(low["rms_db"]+high["rms_db"])/2)

def intrinsic_score(info,role):
    low=info["low"];high=info["high"]
    if not np.isfinite(info["feature"]).all() or info["peak_db"]<-95:return -1e9
    if high["rms_db"]<-62 or low["rms_db"]<-62:return -1e8
    # Bass/Body must be audible, but bass expectations depend on role.
    floor=.18 if role=="Bass" else .07 if role in ("Pad","Drone","Keys") else .035
    body_floor=.17 if role in ("Bass","Keys","Pad","Lead") else .085
    lows=min(0,(low["low_fraction"]-floor))*25
    body=min(0,(high["body_fraction"]-body_floor))*17
    # Avoid hollow/wispy presets, full bandwidth but protect mono compatibility.
    width_penalty=max(0,-.65-low["corr"])*8 + max(0,high["width_db"]-5)*.8
    clipping=max(0,info["peak_db"]+1)*3
    return 8+lows+body-width_penalty-clipping

def diverse_select(candidates,measurements_by_variant):
    """Greedy maximum-separation search in actual output space, never name/ID space."""
    n=len(candidates[0])
    order=sorted(range(n),key=lambda i:(i%8,(i*67)%n))
    chosen={};feature_map={}
    for i in order:
        best=None
        for v in range(len(candidates)):
            metrics=measurements_by_variant[v][i]
            base=intrinsic_score(metrics,candidates[v][i]["role"])
            feat=metrics["feature"]
            distances=[np.linalg.norm(feat-old) for old in feature_map.values()]
            near=sorted(distances)[:5]
            # Natural selection: high-quality body + maximized nearest output distance.
            score=base+(min(near) if near else 0)*2.6+sum(near)/max(len(near),1)*.35
            key=(score,base,-v)
            if best is None or key>best[0]:best=(key,v)
        v=best[1]
        chosen[i]=v
        feature_map[i]=measurements_by_variant[v][i]["feature"]
    # Two rounds of local repair: reduce near-duplicates across all 192 programs.
    for _ in range(2):
        for i in reversed(order):
            others=[f for j,f in feature_map.items() if j!=i]
            best=None
            for v in range(len(candidates)):
                m=measurements_by_variant[v][i]
                base=intrinsic_score(m,candidates[v][i]["role"])
                dist=sorted(np.linalg.norm(m["feature"]-x) for x in others)[:5]
                score=base+2.6*dist[0]+.35*sum(dist)/len(dist)
                if best is None or score>best[0]:best=(score,v)
            chosen[i]=best[1]
            feature_map[i]=measurements_by_variant[best[1]][i]["feature"]
    return chosen

def oscillator_balance(engine,slot,patch):
    """Measure *isolated* sources, compensate gain mismatch rather than by eye."""
    p=patch["overrides"]
    enabled=[1001,1033,1065,1097]
    gains=[1002,1034,1066,1098]
    measured=[]
    for j in range(4):
        solo={**p,"6020":0}
        for k,id in enumerate(enabled):solo[str(id)]=int(k==j)
        raw=engine.render(slot,solo,note=60,total=24000,held=21000)
        value=np.sqrt(np.mean(raw.astype(float)**2)+EPS)
        measured.append(value)
    if max(measured)<1e-6:return measured
    primary=max(measured[0],1e-6)
    desired=(1,.52,.28,.16)
    for j in range(1,4):
        if measured[j]<1e-6:continue # a disabled/inaudible engine should not blow up
        ratio=float(np.clip(primary*desired[j]/measured[j],.55,1.55))
        p[str(gains[j])]=int(np.clip(round(p[str(gains[j])]*math.sqrt(ratio)),4,95))
    return measured

def balance_master(engine,slot,patch):
    info=render_metrics(engine,slot,patch)
    role=patch["role"]
    wanted=ROLES[role]
    correction=min(wanted-info["rms_db"],-5.5-info["peak_db"])
    correction=float(np.clip(correction,-22,16))
    base=float(patch["overrides"].get("9000",-6))
    # Native host's -36..+12 dB gain range. Trim is 0 at initial render.
    final=float(np.clip(base+correction,-36,12))
    patch["overrides"]["9000"]=round(final,3)
    return dict(target_rms_db=wanted,pre_rms_db=info["rms_db"],
                pre_peak_db=info["peak_db"],gain_correction_db=round(final-base,3),
                final_master_db=round(final,3),stereo_width_db=round(info["low"]["width_db"],2),
                low_fraction=round(info["low"]["low_fraction"],4),
                body_fraction=round(info["high"]["body_fraction"],4))

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--candidates",type=Path,required=True)
    parser.add_argument("--renderer",type=Path,required=True)
    parser.add_argument("--out",type=Path,required=True)
    args=parser.parse_args()
    names=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
    folders=sorted((x for x in args.candidates.iterdir() if x.is_dir() and x.name.isdigit()),key=lambda x:int(x.name))
    if len(folders)<2:raise RuntimeError("Selection requires at least two permutative branches.")
    variants=[]
    for folder in folders:
        rows=sum([json.loads((folder/name).read_text(encoding="utf8")) for name in names],[])
        assert len(rows)==192 and [p["index"] for p in rows]==list(range(192))
        variants.append(rows)
    engine=Renderer(args.renderer)
    try:
        results=[]
        for v,rows in enumerate(variants):
            per=[]
            for i,row in enumerate(rows):
                per.append(render_metrics(engine,i,row))
            results.append(per)
            print(f"Candidate {v}: rendered and analyzed {len(per)} programs",flush=True)
        selected=diverse_select(variants,results)
        output=[json.loads(json.dumps(variants[selected[i]][i])) for i in range(192)]
        report=[]
        for i,row in enumerate(output):
            rms=oscillator_balance(engine,i,row)
            gain=balance_master(engine,i,row)
            row["morphTree"]["selectedPermutation"]=selected[i]
            row["morphTree"]["renderSelection"]={
                "method":"two-note stereo PCM spectrum/envelope/width + maximin distance",
                "oscillatorRms":[round(float(x),6) for x in rms],**gain}
            report.append(dict(index=i,variant=selected[i],name=row["name"],role=row["role"],**gain))
        args.out.mkdir(parents=True,exist_ok=True)
        for name,rows in zip(names,(output[:64],output[64:128],output[128:])):
            (args.out/name).write_text(json.dumps(rows,ensure_ascii=False,indent=1)+"\n",encoding="utf8")
        feat=np.stack([results[selected[i]][i]["feature"] for i in range(192)])
        delta=np.sqrt(((feat[:,None]-feat[None,:])**2).sum(axis=2))
        np.fill_diagonal(delta,np.inf)
        nearest=delta.min(axis=1)
        summary=dict(programs=192,new_programs=64,
            nearest_output_distance_min=float(nearest.min()),
            nearest_output_distance_median=float(np.median(nearest)),
            nearest_output_distance_p05=float(np.percentile(nearest,5)),
            source="same Albedo DSP internal samples and original synthesis",
            audition="C3 and G4; 48 kHz stereo; underlying tone with arpeggiator disabled",
            loudness_calibration="role dependent -19.5..-23 dBFS RMS, -5.5 dBFS peak cap, per-oscillator balance",
            limits="measured audio similarity is a screening proxy; listening and DAW QA still required",
            selection_counts={str(v):list(selected.values()).count(v) for v in range(len(folders))})
        (args.out/"morph_selection_report.json").write_text(json.dumps(dict(summary=summary,programs=report),indent=2)+"\n",encoding="utf8")
        print(json.dumps(summary,indent=2),flush=True)
    finally:
        engine.close()

if __name__=="__main__":main()
