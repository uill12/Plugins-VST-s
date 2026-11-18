#!/usr/bin/env python3
"""CP37: independent pre-effect GAIN and post-effect MASTER, stable IDs.
Applied after cp36_hifi_math.py to the reconstructed Windows synth source.
"""
from pathlib import Path
import argparse
p=argparse.ArgumentParser()
p.add_argument("--root",type=Path,default=Path("Albedo-Quintessential-Synth"))
r=p.parse_args().root
params=r/"include/vitriol_params.hpp"
header=r/"include/vitriol_dsp.hpp"
cpp=r/"src/vitriol_dsp.cpp"
def replace_once(text,before,after,name):
    n=text.count(before)
    if n!=1:raise SystemExit(f"CP37 {name}: expected one insertion point, got {n}")
    return text.replace(before,after)
s=params.read_text(encoding="utf-8")
s=replace_once(s,'add(9002,"Oversampling","Master / Claritas",0,4,0,"",true);',
 'add(9002,"Oversampling","Master / Claritas",0,4,0,"",true);\n'
 ' add(9003,"Pre FX Gain","Master / Claritas",-18,12,0,"dB");',
 "new stable pre FX gain parameter")
params.write_text(s,encoding="utf-8")
s=header.read_text(encoding="utf-8")
s=replace_once(s,"outputGainSmooth_=1.0","outputGainSmooth_=1.0, preFxGainSmooth_=1.0",
               "pre FX smoothing member")
header.write_text(s,encoding="utf-8")
s=cpp.read_text(encoding="utf-8")
s=replace_once(s,"pulseGain_=1.0;outputGainSmooth_=db(get(9000));",
    "pulseGain_=1.0;outputGainSmooth_=db(get(9000));preFxGainSmooth_=db(get(9003));",
    "prepare gains")
s=replace_once(s,"const double targetOutputGain=db(get(9000));",
    "const double targetOutputGain=db(get(9000));\n"
    " const double targetPreFxGain=db(get(9003));",
    "block gain snapshot")
s=replace_once(s,
    "   previousDryL_=x;previousDryR_=y;\n   processEffects(x,y);",
    "   // GAIN stages the source signal before the effects (8ms smoothing).\n"
    "   preFxGainSmooth_+=(targetPreFxGain-preFxGainSmooth_)*gainAlpha;\n"
    "   x*=preFxGainSmooth_;y*=preFxGainSmooth_;\n"
    "   previousDryL_=x;previousDryR_=y;\n   processEffects(x,y);",
    "pre FX stage")
cpp.write_text(s,encoding="utf-8")
print("CP37: pre-FX GAIN (ID 9003) and post-FX MASTER (existing ID 9000) independent")
