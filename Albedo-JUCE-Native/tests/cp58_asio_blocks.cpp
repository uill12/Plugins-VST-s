#include "vitriol_dsp.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>

// Real-time ASIO scheduling regression: no audio allocations or offline
// rendering heuristics. Report processing cost across practical buffer sizes.
// CPU is host-dependent, so pass/fail is finite, audible and 12-voice safety.
int main(){
 constexpr std::array<int,4> rates{{44100,48000,88200,96000}};
 constexpr std::array<int,5> blocks{{64,128,256,512,1024}};
 int cases=0;
 for(int sr:rates)for(int block:blocks){
   auto synth=std::make_unique<vitriol::Synth>();
   synth->prepare(sr);
   synth->setParam(6020,0);
   synth->setParam(9002,0);
   for(int e=0;e<4;++e){
     const int b=1000+32*e;
     synth->setParam(b,e==0?0:e==1?1:e==2?2:3);
     synth->setParam(b+1,1);
     synth->setParam(b+2,65);
     synth->setParam(b+11,e==0?4:2);
     synth->setParam(b+12,13);
     synth->setParam(b+22,1);
     synth->setParam(b+23,1900+e*500);
     synth->setParam(b+24,32+e*9);
     synth->setParam(b+25,5+e*2);
   }
   for(int n=48;n<60;++n)synth->noteOn(n,.7,n%3);
   if(synth->activeVoices()<12){std::cerr<<"CP58 polyphony failure\n";return 1;}
   std::array<double,1024> left{},right{};
   double energy=0,maxPeak=0,processingSeconds=0;
   constexpr int frames=8192;
   for(int offset=0;offset<frames;offset+=block){
     const int count=std::min(block,frames-offset);
     const auto start=std::chrono::steady_clock::now();
     synth->process64(left.data(),right.data(),(uint32_t)count);
     processingSeconds+=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
     for(int k=0;k<count;++k){
       const double a=left[k],b=right[k];
       if(!std::isfinite(a)||!std::isfinite(b)){std::cerr<<"CP58 non-finite sample\n";return 2;}
       energy+=a*a+b*b;maxPeak=std::max({maxPeak,std::abs(a),std::abs(b)});
     }
   }
   if(energy<=1.e-9||maxPeak>64.){std::cerr<<"CP58 missing/unsafe audio\n";return 3;}
   ++cases;
   std::cout<<"ASIO buffer="<<block<<" rate="<<sr
            <<" CPU/host-deadline="<<processingSeconds*sr/frames
            <<" peak="<<maxPeak<<" voiceCount=12 PASS\n";
   synth->allNotesOff();
 }
 if(cases!=20)return 4;
 std::cout<<"CP58 PASS: 20 buffer/rate/12-voice native DSP cases\n";
 return 0;
}
