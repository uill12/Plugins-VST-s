"""Optimize only the spectral oscillator's bounded nonnegative phase lookup."""
from pathlib import Path
import sys
root = Path(sys.argv[1])
path = root / 'src/vitriol_dsp.cpp'
src = path.read_text()
old = 'const double position=fract(cycles)*16384.;\n   const int j=int(position);const double t=position-j;'
assert src.count(old)==1
src = src.replace(old, '''// Spectral cycles are nonnegative and below 22. A power-of-two
   // table lets the existing index mask wrap the integer part exactly.
   const double position=cycles*16384.;
   const int j=int(position);const double t=position-j;''')
start = src.index('  double sum=0,norm=0;', src.index(' case 4:{ // CP18R3'))
end = src.index('  y=sum/std::max(norm,.01)', start)
body = src[start:end]
a = 'const double amp=lerp(harmonicTilt_[ix][k-1],harmonicTilt_[ix+1][k-1],mu);'
assert a in body
body = body.replace('  double sum=0,norm=0;', '''  auto& weights=v.spectralWeights[e];
  if(weights.tilt!=tilt){
   weights.norm=0.;
   for(int k=0;k<n;++k){
    weights.amp[k]=lerp(harmonicTilt_[ix][k],harmonicTilt_[ix+1][k],mu);
    weights.norm+=weights.amp[k];
   }
   weights.tilt=tilt;
  }
  double sum=0;
  const double norm=weights.norm;''').replace(a,'const double amp=weights.amp[k-1];').replace('   norm+=amp;\n','')
src=src[:start]+body+src[end:]
path.write_text(src)
path=root/'include/vitriol_dsp.hpp'
s=path.read_text()
anchor=' struct Voice {'
assert s.count(anchor)==1
path.write_text(s.replace(anchor,anchor+'\n  struct SpectralWeights { double tilt=-1,norm=0; std::array<double,20> amp{}; };\n  SpectralWeights spectralWeights[4];'))
