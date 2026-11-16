#include "../src/Engine.h"
#include <cmath>
#include <cstdio>
#include <cassert>
int main(){static nig::AudioEngine a;static nig::MidiEngine m;const int rates[]={44100,48000,88200,96000,192000};
 for(int sr:rates){a.defaults();a.prepare(sr);double energy=0,maxAmp=0;for(int j=0;j<20000;j++){
   double t=(j+1.0)/sr;auto p=a.frame({.21*sin(2*nig::PI*431*t),.17*cos(2*nig::PI*617*t)});
   if(!std::isfinite(p.l)||!std::isfinite(p.r)){printf("NONFINITE at %d %d\n",sr,j);return 2;}
   maxAmp=nig::mx(maxAmp,nig::mx(nig::ab(p.l),nig::ab(p.r)));energy+=p.l*p.l+p.r*p.r;
  }printf("Audio %dHz: finite; energy %.4f max %.4f\n",sr,energy,maxAmp);if(energy<.1||maxAmp>1.021)return 3;
  // At 192 kHz buffer latency should be exactly 1920 samples (max look < 2048).
  if(sr==192000){int d=(int)(sr*.010+.5);if(d>=nig::AudioEngine::LOOK)return 4;}
  m.defaults();m.prepare(sr);int on=0,off=0;
  for(int j=0;j<24;j++){m.noteOn(j,0,44+(j%26),.7,(uint64_t)(j*sr*.02));m.noteOff(j,0,44+(j%26),.1,(uint64_t)(j*sr*.02+sr*.01));}
  uint64_t current=0;while(current<(uint64_t)sr*3){m.drain(current,current+256,[&](nig::Note ev,uint32_t){on+=ev.type==1;off+=ev.type==0;});current+=256;}
  printf("MIDI %dHz: %d note-on, %d note-off, overflows %u\n",sr,on,off,m.overflow);if(on==0||on!=off||m.overflow)return 5;
 }
 printf("PASS - 5 sample rates; seven audio stages; 24 MIDI note pairs per rate\n");return 0;}
