#!/usr/bin/env python3
"""CP36: high-resolution DSP upgrade on the verified CP35 core.
No parameter IDs, preset values, MIDI/voice limits or DSP topology changed.
Runs after CP31/source reconstruction but before JUCE compilation.
"""
from pathlib import Path
import argparse
parser=argparse.ArgumentParser()
parser.add_argument("--root",type=Path,default=Path("Albedo-Quintessential-Synth"))
args=parser.parse_args()
cpp=args.root/"src/vitriol_dsp.cpp"
hpp=args.root/"include/vitriol_dsp.hpp"
s=cpp.read_text(encoding="utf-8")
h=hpp.read_text(encoding="utf-8")
def one(src,a,b,name):
    n=src.count(a)
    if n!=1:raise RuntimeError(f"{name}: expected one anchor, saw {n}")
    return src.replace(a,b)

s=one(s,
 "static inline double cubic(double y0,double y1,double y2,double y3,double mu){double a0=y3-y2-y0+y1, a1=y0-y1-a0, a2=y2-y0, a3=y1;return ((a0*mu+a1)*mu+a2)*mu+a3;}",
 """static inline double cubic(double y0,double y1,double y2,double y3,double mu){double a0=y3-y2-y0+y1, a1=y0-y1-a0, a2=y2-y0, a3=y1;return ((a0*mu+a1)*mu+a2)*mu+a3;}
static inline double catmullRom(double p0,double p1,double p2,double p3,double t){
 // C1-continuous four-point double Hermite interpolation.
 const double a=(-p0+3*p1-3*p2+p3),b=(2*p0-5*p1+4*p2-p3),c=(-p0+p2);
 return p1+.5*t*(c+t*(b+t*a));
}""","Catmull-Rom helper")

s=one(s,
 """double Synth::fastSinCycles(double cycles)const{
 const double pos=fract(cycles)*4096.;const int i=int(pos);const double t=pos-double(i);
 return lerp(sineLut_[i&4095],sineLut_[(i+1)&4095],t);
}""",
 """double Synth::fastSinCycles(double cycles)const{
 const double pos=fract(cycles)*4096.;const int i=int(pos);const double t=pos-double(i);
 return catmullRom(sineLut_[(i-1)&4095],sineLut_[i&4095],
                   sineLut_[(i+1)&4095],sineLut_[(i+2)&4095],t);
}""","64-bit sine LUT interpolation")

s=one(s,
 "double Synth::Delay::read(double lag)const{if(buf.empty())return 0;double x=(double)pos-lag;while(x<0)x+=buf.size();x=std::fmod(x,double(buf.size()));size_t i=size_t(x),j=(i+1)%buf.size();return lerp(buf[i],buf[j],x-i);}",
 """double Synth::Delay::read(double lag)const{
 if(buf.empty())return 0;
 double x=std::fmod(double(pos)-lag,double(buf.size()));
 if(x<0)x+=buf.size();
 const size_t n=buf.size(),i=size_t(x);
 const double result=catmullRom(buf[(i+n-1)%n],buf[i],buf[(i+1)%n],buf[(i+2)%n],x-double(i));
 return clamp(result,-8.0,8.0);
}""","higher-order chorus/reverb interpolation")
s=one(s,"decimLen_=32;const double fc=.44/os;",
       "decimLen_=64;const double fc=.44/os;","64-tap Blackman low-pass decimator")
s=one(s,
 """  auto fastSin=[&](double cycles){
   const double position=fract(cycles)*4096.;
   const int j=int(position);const double t=position-j;
   return lerp(sineLut_[j&4095],sineLut_[(j+1)&4095],t);
  };""",
 """  auto fastSin=[&](double cycles){
   const double position=fract(cycles)*4096.;
   const int j=int(position);const double t=position-j;
   return catmullRom(sineLut_[(j-1)&4095],sineLut_[j&4095],
                     sineLut_[(j+1)&4095],sineLut_[(j+2)&4095],t);
  };""","spectral partial interpolation")
s=one(s,"void Synth::process(float* l,float* r,uint32_t n){",
      "template<typename Sample>\nvoid Synth::processTyped(Sample* l,Sample* r,uint32_t n){",
      "full-resolution process entry")
s=one(s,
 """   l[i]=float(clamp(x,-1,1));r[i]=float(clamp(y,-1,1));clock_++;
 }}
void Synth::process64(double* l,double* r,uint32_t n){std::array<float,256> a{},b{};uint32_t i=0;while(i<n){uint32_t sz=std::min(uint32_t(a.size()),n-i);process(a.data(),b.data(),sz);for(uint32_t k=0;k<sz;k++){l[i+k]=a[k];r[i+k]=b[k];}i+=sz;}}
}""",
 """   l[i]=static_cast<Sample>(clamp(x,-1,1));r[i]=static_cast<Sample>(clamp(y,-1,1));clock_++;
 }}
void Synth::process(float* l,float* r,uint32_t n){processTyped(l,r,n);}
void Synth::process64(double* l,double* r,uint32_t n){processTyped(l,r,n);}
}""","true 64-bit output without float intermediary")
h=one(h,"private:\n struct Voice {",
       """private:
 // All synthesis/effects are double internally, final output cast occurs once.
 template<typename Sample> void processTyped(Sample* l,Sample* r,uint32_t n);
 struct Voice {""","processTyped declaration")
cpp.write_text(s,encoding="utf-8")
hpp.write_text(h,encoding="utf-8")
print("CP36 high-definition DSP applied (true 64-bit, Catmull-Rom, 64-tap decimator)")
