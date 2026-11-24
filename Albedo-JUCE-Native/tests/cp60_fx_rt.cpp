// CP60 DSP real-time safety and modulation regression (source-internal test only).
// No Windows or plugin host required; native CI also runs this in Release.
#include <array>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cmath>
#define private public
#include "vitriol_dsp.hpp"
#undef private
int main(){
 using vitriol::Synth;
 auto synth=std::make_unique<Synth>();
 int cases=0;
 for(int sr:{44100,48000,88200,96000,192000}){
  synth->prepare(sr);
  for(int i=0;i<4;i++)synth->setParam(5001+i*2,0.0);
  synth->setParam(5005,1.0);synth->setParam(5004,100.0);
  synth->refreshFxCoefficients();
  // Poison the dry/bypassed FDN ring to detect stale audio on re-enable.
  for(auto& line:synth->rev_.line)std::fill(line.begin(),line.end(),0.75);
  synth->rev_.clearCursor=synth->rev_.clearRemaining=0;
  synth->fxRevMix_=0.;synth->fxIdle_[2]=true;
  double l=0.,r=0.;synth->processEffects(l,r);
  const size_t length=synth->rev_.mask+1;
  if(synth->rev_.clearRemaining!=length-32){std::fprintf(stderr,"unbounded FDN clear at %d\n",sr);return 1;}
  for(auto& line:synth->rev_.line){
   for(int i=0;i<32;i++)if(line[i]!=0.){std::fprintf(stderr,"first slice not cleared\n");return 2;}
   if(line[32]!=0.75){std::fprintf(stderr,"realtime cleared too many samples\n");return 3;}
  }
  if(l!=0.||r!=0.){std::fprintf(stderr,"stale reverb emitted before cleaning\n");return 4;}
  ++cases;
  // Progress one host sample at a time until ring is completely scrubbed.
  for(size_t i=0;i<length/32;i++){l=r=0;synth->processEffects(l,r);}
  if(synth->rev_.clearRemaining!=0){std::fprintf(stderr,"FDN never cleared\n");return 5;}
  for(auto& line:synth->rev_.line)if(line[100]!=0.){std::fprintf(stderr,"old FDN tail revived\n");return 6;}
  ++cases;
  // Restore normal destination-8 modulation. Chorus and diffusion must follow
  // the established mod routing without activating disabled effects.
  synth->reset();synth->setParam(5003,1.);synth->setParam(5002,30.);
  synth->setParam(5007,1.);synth->setParam(5006,17.);
  synth->setParam(5005,0.);
  synth->refreshFxCoefficients();
  synth->modFrame_[8]=0.;l=r=0;synth->processEffects(l,r);
  const double a=synth->fxChorusMix_,b=synth->fxDiffMix_;
  synth->reset();synth->refreshFxCoefficients();synth->modFrame_[8]=1.;
  l=r=0;synth->processEffects(l,r);
  if(!(synth->fxChorusMix_>a&&synth->fxDiffMix_>b)){
   std::fprintf(stderr,"shared spatial modulation missing %d\n",sr);return 7;
  }
  synth->setParam(5003,0.);synth->setParam(5007,0.);synth->reset();synth->refreshFxCoefficients();
  synth->modFrame_[8]=1.;l=r=0;synth->processEffects(l,r);
  if(synth->fxChorusMix_!=0. || synth->fxDiffMix_!=0.){
   std::fprintf(stderr,"disabled FX woke via modulation\n");return 8;
  }
  ++cases;
 }
 std::printf("CP60 FX QA PASS: %d cases across 44.1/48/88.2/96/192 kHz\n",cases);
}