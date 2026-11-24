// CP60 DSP real-time safety and modulation regression (source-internal test only).
// No Windows or plugin host required; native CI also runs this in Release.
#include <array>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include "vitriol_dsp.hpp"
namespace vitriol { struct CP60TestProbe { static int run(){
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
 // The offline target matcher reuses a Synth across 192 candidate renders.
 // After prepare(), its reverb must match the same preset from a fresh process.
 // Regression for missed FDN modulation phase reset after long wet tails.
 for(int sr:{48000,96000}){
  auto renderer=std::make_unique<Synth>();
  renderer->setParam(1000,1);renderer->setParam(1001,1);renderer->setParam(1002,65);
  renderer->setParam(5005,1);renderer->setParam(5004,60);renderer->setParam(5011,125);
  renderer->setParam(5003,1);renderer->setParam(5002,15);renderer->setParam(3103,1.0);
  auto render=[&](){
   renderer->prepare(sr);renderer->noteOn(57,.8,0);
   const int count=sr/2,off=count/2;
   std::vector<float> output(count*2);
   for(int p=0;p<count;){
    if(p==off)renderer->noteOff(57,0);
    int n=std::min(128,count-p);if(p<off)n=std::min(n,off-p);
    float l[128]{},r[128]{};renderer->process(l,r,n);
    for(int k=0;k<n;k++){output[2*(p+k)]=l[k];output[2*(p+k)+1]=r[k];}
    p+=n;
   }
   return output;
  };
  const auto first=render(),second=render();
  double delta=0.,peak=0.;for(size_t i=0;i<first.size();i++){
   delta=std::max(delta,std::abs(double(first[i])-second[i]));
   peak=std::max(peak,std::abs(double(first[i])));
  }
  if(peak<1.e-5||delta>1.e-7){std::fprintf(stderr,"non-deterministic FDN reset rate=%d delta=%.9g\n",sr,delta);return 9;}
  ++cases;
 }
 std::printf("CP60 FX QA PASS: %d cases across 44.1/48/88.2/96/192 kHz\n",cases);
return 0;
}}; }
int main(){return vitriol::CP60TestProbe::run();}