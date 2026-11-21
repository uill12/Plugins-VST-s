#!/usr/bin/env python3
"""CP58: cache invariant real-time DSP math once per host/event chunk.
Apply after CP50 and CP53. No changed algorithms, voice counts or parameters.
"""
from pathlib import Path
import argparse
parser=argparse.ArgumentParser()
parser.add_argument('--root',type=Path,default=Path('Albedo-Quintessential-Synth'))
a=parser.parse_args()
cpp=a.root/'src/vitriol_dsp.cpp'
hdr=a.root/'include/vitriol_dsp.hpp'
s=cpp.read_text(encoding='utf-8');h=hdr.read_text(encoding='utf-8')
def rep(t,old,new,tag):
 n=t.count(old)
 if n!=1:raise RuntimeError('CP58 '+tag+': expected one source anchor, found '+str(n))
 return t.replace(old,new)
h=rep(h,' double filterEnvFrame_=0;',""" // Recompute parameter-invariant math before each DAW/MIDI audio chunk.
 // Audio-rate nonlinearities, filter state and 192 presets remain unchanged.
 std::array<double,4> sourceDriveGain_{{1.,1.,1.,1.}};
 std::array<double,4> sourceDriveNormalize_{{1.,1.,1.,1.}};
 std::array<double,4> sourceResonanceK_{{2.,2.,2.,2.}};
 std::array<double,4> sourceSqrtUnison_{{1.,1.,1.,1.}};
 double sourceFilterMixRate_=1.,sourceFilterCutoffRate_=1.;
 double voiceAttackDelta_=0.,voiceSustain_=0.;
 double filterAttackDelta_=0.,filterSustain_=0.;
 double filterEnvFrame_=0;""",'header')
s=rep(s,'double Synth::voiceEnvelope(Voice& v,double fsInternal){double a=std::max(.001,get(3100)),d=std::max(.01,get(3101)),s=get(3102)*.01,r=std::max(.01,get(3103));','double Synth::voiceEnvelope(Voice& v,double fsInternal){const double s=voiceSustain_;','voice envelope')
s=rep(s,'case 0:v.env+=1./(a*fsInternal);','case 0:v.env+=voiceAttackDelta_;','voice attack')
s=rep(s,' const double rate=1.0/(1.0+.006*fsInternal);',' const double rate=sourceFilterMixRate_;','source mix')
s=rep(s,' smooth+=(cutTarget-smooth)/(1.0+.003*fsInternal);',' smooth+=(cutTarget-smooth)*sourceFilterCutoffRate_;','source cutoff')
s=rep(s,' const double resonance=clamp(get(b+24)*.01,0.,1.);\n const double k=2.0-1.91*resonance;'.replace('\\n','\n'),
 ' const double k=sourceResonanceK_[e];','resonance coefficient')
s=rep(s,' const double drive=get(b+25);\n const double gain=db(drive);\n const double x=(drive>.001)?std::tanh(input*gain)/std::max(1.,gain*.5):input;'.replace('\\n','\n'),
 ' const double drive=get(b+25);\n const double gain=sourceDriveGain_[e];\n const double x=(drive>.001)?std::tanh(input*gain)/sourceDriveNormalize_[e]:input;'.replace('\\n','\n'),'drive gain')
s=rep(s,'double Synth::filterEnvelope(Voice& v,double fsInternal){\n double a=get(3122),d=get(3123),s=get(3124)*.01,r=get(3125);'.replace('\\n','\n'),
 'double Synth::filterEnvelope(Voice& v,double fsInternal){\n const double s=filterSustain_;'.replace('\\n','\n'),'filter envelope')
s=rep(s,'v.filterEnv+=1./std::max(1.,a*fsInternal);','v.filterEnv+=filterAttackDelta_;','filter attack')
if s.count('/std::sqrt(double(uni));')!=2:raise RuntimeError('CP58 two unison normalization anchors absent')
s=s.replace('/std::sqrt(double(uni));','/sourceSqrtUnison_[e];')
# For built-in FX, all delay tap lags fit their ring sizes. Avoid per-sample
# fmod() while preserving exactly the same normal-case floating-point output.
# Out-of-range taps retain the legacy fallback.
s=rep(s,' double x=std::fmod(double(pos)-lag,double(buf.size()));\n if(x<0)x+=buf.size();'.replace('\\n','\n'),
 ''' const double size=double(buf.size());
 double x=double(pos)-lag;
 if(lag>=0. && lag<size){if(x<0.)x+=size;}
 else {x=std::fmod(x,size);if(x<0.)x+=size;}''','delay wrap')
s=rep(s,' blockRates_.bendFactor=std::exp2(bend_*2./12.);',""" blockRates_.bendFactor=std::exp2(bend_*2./12.);
 voiceAttackDelta_=1./(std::max(.001,get(3100))*srInternal);
 voiceSustain_=get(3102)*.01;
 filterAttackDelta_=1./std::max(1.,get(3122)*srInternal);
 filterSustain_=get(3124)*.01;
 sourceFilterMixRate_=1./(1.+.006*srInternal);
 sourceFilterCutoffRate_=1./(1.+.003*srInternal);
 for(int e=0;e<4;++e){
  const uint32_t b=1000u+uint32_t(e)*32u;
  const double driveGain=db(get(b+25));
  sourceDriveGain_[e]=driveGain;
  sourceDriveNormalize_[e]=std::max(1.,driveGain*.5);
  sourceResonanceK_[e]=2.-1.91*clamp(get(b+24)*.01,0.,1.);
  sourceSqrtUnison_[e]=std::sqrt(double(std::clamp(int(get(b+11)),1,8)));
 }""",'chunk invariants')
cpp.write_text(s,encoding='utf-8');hdr.write_text(h,encoding='utf-8')
print('CP58 realtime cache PASS: unchanged source DSP, fewer pow/sqrt/fmod per sample')
