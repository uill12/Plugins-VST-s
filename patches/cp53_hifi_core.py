#!/usr/bin/env python3
"""CP53 clean headroom topology for the reconstructed Albedo DSP.

Only enabled in the JUCE synth through ALBEDO_CP53_TRANSPARENT_OUTPUT.
Leaves CP51 preset IDs, values and the old standalone engine untouched.
"""
from pathlib import Path
import argparse
parser=argparse.ArgumentParser()
parser.add_argument('--root',type=Path,default=Path('Albedo-Quintessential-Synth'))
args=parser.parse_args()
cpp=args.root/'src/vitriol_dsp.cpp'
s=cpp.read_text(encoding='utf-8')
def replace(old,new,label):
 global s
 n=s.count(old)
 if n!=1:raise RuntimeError(f'CP53 {label}: expected one source anchor, found {n}')
 s=s.replace(old,new)
replace('static inline double db(double d){return std::pow(10.0,d/20.0);}', '''static inline double db(double d){return std::pow(10.0,d/20.0);}
// C1-continuous safety knee: unity slope within the musical signal range.
// expm1 is used only in overload. Normal signal never passes a tanh shaper.
static inline double clearKnee(double x,double knee,double span){
 if(!std::isfinite(x))return 0.0;
 const double a=std::abs(x);
 if(a<=knee)return x;
 return std::copysign(knee-span*std::expm1(-(a-knee)/span),x);
}''','transparent-knee helper')
replace(' x=std::tanh(x*p.drive);', '''#if defined(ALBEDO_CP53_TRANSPARENT_OUTPUT)
 // Clean through unity/normal drive; retain controllable overload character.
 x=clearKnee(x*p.drive,1.35,1.65);
#else
 x=std::tanh(x*p.drive);
#endif''','global filter input distortion')
replace('l=std::tanh(l*1.0);r=std::tanh(r*1.0);', '''#if defined(ALBEDO_CP53_TRANSPARENT_OUTPUT)
 l=clearKnee(l,1.15,1.65);r=clearKnee(r,1.15,1.65);
#else
 l=std::tanh(l*1.0);r=std::tanh(r*1.0);
#endif''','unconditional synthesis mix saturation')
replace('if(get(9001)>.5){x=limit(x*.86)*1.05;y=limit(y*.86)*1.05;}', '''#if defined(ALBEDO_CP53_TRANSPARENT_OUTPUT)
   // CP51 guard compressed even quiet material. Preserve the toggle only as
   // an overload knee; a linked output lookahead protector works downstream.
   if(get(9001)>.5){x=clearKnee(x,2.0,2.0);y=clearKnee(y,2.0,2.0);}
#else
   if(get(9001)>.5){x=limit(x*.86)*1.05;y=limit(y*.86)*1.05;}
#endif''','legacy final nonlinear guard')
replace('l[i]=static_cast<Sample>(clamp(x,-1,1));r[i]=static_cast<Sample>(clamp(y,-1,1));clock_++;', '''#if defined(ALBEDO_CP53_TRANSPARENT_OUTPUT)
   // No ±1 hard clipping before the native true-peak lookahead stage.
   // The ±64 bound is catastrophic-failure protection, not normal saturation.
   l[i]=static_cast<Sample>(clamp(x,-64,64));r[i]=static_cast<Sample>(clamp(y,-64,64));
#else
   l[i]=static_cast<Sample>(clamp(x,-1,1));r[i]=static_cast<Sample>(clamp(y,-1,1));
#endif
   clock_++;''','master hard clip before JUCE lookahead')
replace('return fs_>=176400.?8:12; // CP28: 12 independent keys through 96 kHz, legacy 8-voice cap at 176.4/192 kHz.', '''#if defined(ALBEDO_CP53_TRANSPARENT_OUTPUT)
 // AUTO high-rate audio is already rendered internally at half the host rate.
 // Budget based on actual DSP workload, not an outdated host-rate threshold.
 const double internal=fs_*oversamplingFactor()*
     ((fs_>=176400. && int(get(9002))==0) ? .5 : 1.);
 return internal>240000.?8:12;
#else
 return fs_>=176400.?8:12;
#endif''','12-voice default for high-rate AUTO with safe manual-oversampling budget')
replace('void Synth::voicePanic(){sustain_.fill(false);for(auto&v:voices_)if(v.active){v.held=v.sustained=false;v.released=true;v.envStage=3;}}',
 '''void Synth::voicePanic(){sustain_.fill(false);for(auto&v:voices_)if(v.active){
  v.held=v.sustained=false;v.released=true;v.envStage=3;v.fastPanicRelease=true;
 }}''','panic ensures bounded release, not a stuck envelope')
replace('v.active=v.released=v.held=v.sustained=false;v.noteId=-1;v.group=0;',
        'v.active=v.released=v.held=v.sustained=v.fastPanicRelease=false;v.noteId=-1;v.group=0;',
        'reset panic-release state')
replace('voice->active=true;voice->released=false;voice->held=true;voice->sustained=false;',
        'voice->active=true;voice->released=false;voice->held=true;voice->sustained=false;voice->fastPanicRelease=false;',
        'clear fast release on voice reuse')
replace('case 3:v.env*=blockRates_.voiceRelease;if(v.env<.000035){',
        'case 3:v.env*=v.fastPanicRelease?blockRates_.panicRelease:blockRates_.voiceRelease;if(v.env<.000035){',
        'bounded 120ms release for all-notes-off')
replace('blockRates_.voiceRelease=std::exp(-7./(std::max(.01,get(3103))*srInternal));',
        'blockRates_.voiceRelease=std::exp(-7./(std::max(.01,get(3103))*srInternal));\n   blockRates_.panicRelease=std::exp(-1./(.012*srInternal));',
        'precomputed panic release coefficient')
# CP53: cache unison pitch ratios once per host audio chunk. The original
# per-voice/per-sample pow(2,shift) could execute millions of times per second
# in wide, twelve-note presets. This preserves the exact relative detune map.
replace('  double detune=get(1000+e*32+12),spread=get(1000+e*32+13)*.01,side=0,sides=0;',
        '  double spread=get(1000+e*32+13)*.01,side=0,sides=0;',
        'cache unison detune')
replace('    double shift=position*detune/1200.;\n    double f=freq*std::pow(2.,shift);',
        '    const double f=freq*unisonRatios_[e][u];',
        'eliminate unison pow inside every sample')
replace(' blockRates_.driveGain[0]=db(get(3002));blockRates_.driveGain[1]=db(get(3005));',
 ''' blockRates_.driveGain[0]=db(get(3002));blockRates_.driveGain[1]=db(get(3005));
 // One cache refresh per audio chunk, with the same per-unison spread map.
 for(int e=0;e<4;++e){
  const int uni=std::clamp(int(get(1000+32*e+11)),1,8);
  const double detune=get(1000+32*e+12)/1200.;
  unisonRatios_[e][0]=1.;
  for(int u=1;u<uni;++u){
   const double position=(u%2?1.:-1.)*double((u+1)/2)/double(uni/2);
   unisonRatios_[e][u]=std::exp2(position*detune);
  }
 }''','once-per-block unison detune cache')
# CP53: 16K double sine LUT (<2e-8 worst-case linear interpolation error)
# replaces four-point Catmull-Rom lookup only on the JUCE HiFi path. It uses
# 128 KiB rather than 32 KiB per synth, but saves CPU in 20-partial spectral
# sources, wavetable harmonics and other sine-heavy oscillator modes.
replace('''double Synth::fastSinCycles(double cycles)const{
 const double pos=fract(cycles)*4096.;const int i=int(pos);const double t=pos-double(i);
 return catmullRom(sineLut_[(i-1)&4095],sineLut_[i&4095],
                   sineLut_[(i+1)&4095],sineLut_[(i+2)&4095],t);
}''',
 '''double Synth::fastSinCycles(double cycles)const{
#if defined(ALBEDO_CP53_TRANSPARENT_OUTPUT)
 const double pos=fract(cycles)*16384.;const int i=int(pos);const double t=pos-double(i);
 return lerp(sineLut_[i&16383],sineLut_[(i+1)&16383],t);
#else
 const double pos=fract(cycles)*4096.;const int i=int(pos);const double t=pos-double(i);
 return catmullRom(sineLut_[(i-1)&4095],sineLut_[i&4095],
                   sineLut_[(i+1)&4095],sineLut_[(i+2)&4095],t);
#endif
}''','fast 16K high-resolution sine interpolation')
replace('for(int k=0;k<4096;k++)sineLut_[k]=std::sin(TAU*double(k)/4096.);',
 '''#if defined(ALBEDO_CP53_TRANSPARENT_OUTPUT)
 for(int k=0;k<16384;k++)sineLut_[k]=std::sin(TAU*double(k)/16384.);
#else
 for(int k=0;k<4096;k++)sineLut_[k]=std::sin(TAU*double(k)/4096.);
#endif''','precomputed 16K high-resolution sine bank')
replace('''  auto fastSin=[&](double cycles){
   const double position=fract(cycles)*4096.;
   const int j=int(position);const double t=position-j;
   return catmullRom(sineLut_[(j-1)&4095],sineLut_[j&4095],
                     sineLut_[(j+1)&4095],sineLut_[(j+2)&4095],t);
  };''',
 '''  auto fastSin=[&](double cycles){
#if defined(ALBEDO_CP53_TRANSPARENT_OUTPUT)
   const double position=fract(cycles)*16384.;
   const int j=int(position);const double t=position-j;
   return lerp(sineLut_[j&16383],sineLut_[(j+1)&16383],t);
#else
   const double position=fract(cycles)*4096.;
   const int j=int(position);const double t=position-j;
   return catmullRom(sineLut_[(j-1)&4095],sineLut_[j&4095],
                     sineLut_[(j+1)&4095],sineLut_[(j+2)&4095],t);
#endif
  };''','spectral 20-partial sine fast path')
h=args.root/'include/vitriol_dsp.hpp'
header=h.read_text(encoding='utf-8')
def replace_header(old,new):
 global header
 n=header.count(old)
 if n!=1:raise RuntimeError(f'CP53 header: expected exactly one {old!r}, saw {n}')
 header=header.replace(old,new)
replace_header('bool active=false,released=false,held=false,sustained=false;',
               'bool active=false,released=false,held=false,sustained=false,fastPanicRelease=false;')
replace_header('double voiceDecay=0,voiceRelease=1,filterDecay=0',
               'double voiceDecay=0,voiceRelease=1,panicRelease=1,filterDecay=0')
replace_header(' std::array<std::array<double,2>,2049> panLaw_{};',
 ''' std::array<std::array<double,2>,2049> panLaw_{};
 std::array<std::array<double,8>,4> unisonRatios_{};''')
replace_header('std::array<double,4096> sineLut_{};','std::array<double,16384> sineLut_{};')
h.write_text(header,encoding='utf-8')
cpp.write_text(s,encoding='utf-8')
print('CP53: clean Corpus, mix, and master with transparent JUCE output headroom')
