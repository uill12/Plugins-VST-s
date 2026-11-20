"""Apply CP49 to the verified CP48 engine after its source reconstruction."""
from pathlib import Path
import sys
path=Path(sys.argv[1])/"src/vitriol_dsp.cpp"
source=path.read_text(encoding="utf8")
changes=[
 [
  "xyx*(1-xyy),(1-xyx)*xyy",
  "(1-xyx)*xyy,xyx*(1-xyy)"
 ],
 [
  "w[e]=(0.24+.76*w[e])*level;gainNorm+=w[e];",
  "w[e]=std::sqrt(w[e])*level;gainNorm+=w[e]*w[e];"
 ],
 [
  "const double inverse=1./std::max(.6,gainNorm);",
  "const double inverse=1./std::sqrt(std::max(1.,gainNorm));"
 ],
 [
  "double shift=(double(u)/(uni-1)-.5)*2.*detune/1200.;",
  "const double position=(u%2 ? 1. : -1.)*((u+1)/2)/double(uni/2);\n    double shift=position*detune/1200.;"
 ],
 [
  "double sample=fastSinCycles(ph);side+=sample;sides+=sample*shift/40.*spread;",
  "double sample=fastSinCycles(ph);\n    if(m==0){const double dt=f/sr;const double width=clamp(.08+.84*c,.08,.92);const double saw=2*ph-1-polyblep(ph,dt);const double pulse=(ph<width?1.:-1.)+polyblep(ph,dt)-polyblep(fract(ph-width),dt);sample=lerp(lerp(saw,pulse,a),4*std::abs(ph-.5)-1,b*.9);}\n    side+=sample;sides+=sample*position*spread;"
 ],
 [
  "lerp(character,tri,b*.22)",
  "lerp(character,tri,b*.90)"
 ],
 [
  "case 3:return v.env*2-1;",
  "case 3:return v.filterEnv*2-1;"
 ],
 [
  ".01*exchange*.24",
  ".01*exchange*.85"
 ],
 [
  "(get(1000+e*32+6)+20*",
  "(get(1000+e*32+6)+75*"
 ],
 [
  "mod[10]*.08",
  "mod[10]*.45"
 ],
 [
  "modFrame_[0]*3+envF",
  "modFrame_[0]*3+(get(2000)*.01-.5)*3.0+envF"
 ],
 [
  "modFrame_[1]*3+envF",
  "modFrame_[1]*3+(get(2000)*.01-.5)*2.0+envF"
 ]
]
changes.append(("y=lerp(next,now,c*.7)*(0.8+a*.8);", "const double bodyTap=v.strings[e][(pos+period/3)%period];\n  y=lerp(lerp(next,now,c*.7),bodyTap,d*.8)*(0.8+a*.8);"))
for old,new in changes:
    if source.count(old)!=1:
        raise ValueError(f"Unexpected DSP revision at {old!r}")
    source=source.replace(old,new)
path.write_text(source,encoding="utf8")
