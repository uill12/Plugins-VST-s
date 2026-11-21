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
h=a.root/'include/vitriol_dsp.hpp'
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
h.write_text(header,encoding='utf-8')
cpp.write_text(s,encoding='utf-8')
print('CP53: clean Corpus, mix, and master with transparent JUCE output headroom')
