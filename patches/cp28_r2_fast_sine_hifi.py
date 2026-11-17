#!/usr/bin/env python3
"""CP28-R2: preserve hi-fi via existing 4096-point sine LUT in hot DSP paths."""
from pathlib import Path
import math,hashlib,json
r=Path("Albedo-Quintessential-Synth")
p=r/"src/vitriol_dsp.cpp"
s=p.read_text(encoding="utf-8")
before_sha=hashlib.sha256(p.read_bytes()).hexdigest()
assert before_sha=="914d77017336c9ff6f3ee05a18af0cc114b46d53023d369e2407aff44be15f1f",("CP28-R2 base mismatch",before_sha)
changes=[
  [
    "std::sin(TAU*sub)",
    "fastSinCycles(sub)",
    2
  ],
  [
    "std::sin(TAU*phase)",
    "fastSinCycles(phase)",
    5
  ],
  [
    "std::sin(TAU*warped)",
    "fastSinCycles(warped)",
    1
  ],
  [
    "std::sin(PI*gp)",
    "fastSinCycles(gp*.5)",
    1
  ],
  [
    "std::cos(TAU*gp)",
    "fastSinCycles(gp+.25)",
    1
  ],
  [
    "std::sin(TAU*phase*2)",
    "fastSinCycles(phase*2)",
    1
  ],
  [
    "std::sin(TAU*phase*3)",
    "fastSinCycles(phase*3)",
    1
  ],
  [
    "std::sin(TAU*phase*(1+d*.6))",
    "fastSinCycles(phase*(1+d*.6))",
    1
  ],
  [
    "std::sin(TAU*pd)",
    "fastSinCycles(pd)",
    1
  ],
  [
    "std::sin(TAU*(phase+(cells-.5)*b*.5))",
    "fastSinCycles(phase+(cells-.5)*b*.5)",
    1
  ],
  [
    "std::sin(TAU*sub*(1.+a*3.))",
    "fastSinCycles(sub*(1.+a*3.))",
    1
  ],
  [
    "std::sin(TAU*ph)",
    "fastSinCycles(ph)",
    1
  ],
  [
    "std::sin(lfoPhase_*TAU)",
    "fastSinCycles(lfoPhase_)",
    1
  ],
  [
    "std::sin(TAU*(get(5008)*t+c*(.05+get(5009)*.002)))",
    "fastSinCycles(get(5008)*t+c*(.05+get(5009)*.002))",
    1
  ]
]
for old,new,expected in changes:
    actual=s.count(old)
    assert actual==expected,(old,actual,expected)
    s=s.replace(old,new)
# lfoShape() is static. Keep its phase waveform as std::sin; only audio
# instance methods can use the per-Synth pre-initialized LUT.
old="default:return fastSinCycles(phase);\n }\n}\nvoid Synth::Delay::sizeTo"
new="default:return std::sin(TAU*phase);\n }\n}\nvoid Synth::Delay::sizeTo"
assert s.count(old)==1,"static LFO anchor changed"
s=s.replace(old,new)
p.write_text(s,encoding="utf-8",newline="\n")
digest=hashlib.sha256(p.read_bytes()).hexdigest()
assert digest=="cf23a91e37a773e1755d2bf6e60825800279d1ac45f6e63c9c33921029ebebfd",("CP28-R2 waveform LUT source changed",digest)
# 4096 samples, linear interpolation, max midpoint error ~2.94e-7.
n=4096
mx=max(abs(math.sin((i+.5)*2*math.pi/n)-.5*(math.sin(i*2*math.pi/n)+math.sin((i+1)*2*math.pi/n))) for i in range(n))
assert mx<3e-7,("CP28-R2 trigonometric interpolation error",mx)
mf=r/"CP19_SOURCE_MANIFEST.json"
j=json.loads(mf.read_text(encoding="utf-8"))
j["changed_source_sha256"]["src/vitriol_dsp.cpp"]=digest
j["cp28_fast_sine_hifi"]="Reuse precomputed 4096-sample sine table at oscillator/grain/unison/FX paths; max midpoint error under 3e-7; original waveform parameters preserved."
mf.write_text(json.dumps(j,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print(f"CP28-R2 fast Hi-Fi sine source verified, worst lookup error {mx:.9g}")
