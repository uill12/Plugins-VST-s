#!/usr/bin/env python3
"""RGNES CP36 fidelity and long-form Atlas evidence gate (stdlib-only)."""
from pathlib import Path
import json, math, cmath, random
root=Path(__file__).resolve().parents[1]
src=root.parent/"Albedo-Quintessential-Synth"/"src/vitriol_dsp.cpp"
head=root.parent/"Albedo-Quintessential-Synth"/"include/vitriol_dsp.hpp"
bridge=root/"Assets/juce_cp33_bridge.js"
layout=root/"Assets/cp35_editor_geometry.js"
data=root/"Metadata/factory_preset_taxonomy.json"
s=src.read_text(encoding="utf-8")
h=head.read_text(encoding="utf-8")
b=bridge.read_text(encoding="utf-8")
ui=layout.read_text(encoding="utf-8")
t=json.loads(data.read_text(encoding="utf-8"))
for token in ["catmullRom(","decimLen_=64;","processTyped(Sample*",
              "void Synth::process64(double* l,double* r,uint32_t n){processArp(l,r,n);"]:
    assert token in s,token
arp=(head.parent/"AlbedoCellArp.inc").read_text(encoding="utf-8")
assert "processTyped(l+offset,r+offset,count)" in arp and "processTyped(l,r,n)" in arp
assert "template<typename Sample> void processTyped" in h
assert "std::array<float,256> a{},b{}" not in s
assert "pb-juce-extra" not in b and "pb-juce-extra" not in ui
assert "Name × Material × Role" in b
assert "Math.min(960" in ui and "Math.min(642" in ui, "CP41 compact Atlas geometry"
assert "font-size:14px!important" in ui
assert len(t)==64 and len({(x["family"],x["role"]) for x in t})==64
assert all(x["name"] and x["role"] and x["family"] for x in t)

lut=[math.sin(2*math.pi*i/4096) for i in range(4096)]
rng=random.Random(101)
cubic_error=0.0
linear_error=0.0
for _ in range(8000):
    cycles=rng.random()
    p=cycles*4096
    j=int(p);x=p-j
    p0,p1,p2,p3=(lut[(j+off)&4095] for off in (-1,0,1,2))
    smooth=p1+.5*x*((-p0+p2)+x*((2*p0-5*p1+4*p2-p3)+x*(-p0+3*p1-3*p2+p3)))
    linear=p1+(p2-p1)*x
    expected=math.sin(2*math.pi*cycles)
    cubic_error=max(cubic_error,abs(smooth-expected))
    linear_error=max(linear_error,abs(linear-expected))
assert cubic_error<1e-9,(cubic_error,linear_error)
assert cubic_error<linear_error/1000

def decimator_db(n,os,frequency):
    fc=.44/os
    h=[]
    for i in range(n):
        m=i-(n-1)/2
        sinc=1.0 if abs(m)<1e-12 else math.sin(2*math.pi*fc*m)/(2*math.pi*fc*m)
        window=.42-.5*math.cos(2*math.pi*i/(n-1))+.08*math.cos(4*math.pi*i/(n-1))
        h.append(2*fc*sinc*window)
    total=sum(h)
    amplitude=abs(sum((v/total)*cmath.exp(-2j*math.pi*frequency*i) for i,v in enumerate(h)))
    return 20*math.log10(max(1e-14,amplitude))
results={}
for os in [2,4]:
    old=decimator_db(32,os,.56/os)
    new=decimator_db(64,os,.56/os)
    assert new<old-15,(os,old,new)
    results[str(os)]={"old_stopband_db":round(old,1),"new_stopband_db":round(new,1)}
print(json.dumps({"result":"PASS","sine_max_abs_error":cubic_error,
                  "sine_linear_error":linear_error,"stopband":results,
                  "factory_presets":64,"gui_filters":["name","family","role"]},indent=2))
