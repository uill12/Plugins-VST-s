#include "vitriol_dsp.hpp"
#include <array>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
using namespace vitriol;
int main(){
 auto synth=std::make_unique<Synth>();
 constexpr std::array<int,4> rates={44100,48000,88200,96000};
 constexpr std::array<int,5> blocks={64,128,256,512,1024};
 std::array<double,1024>l{},r{};
 int cases=0;
 for(int sr:rates){synth->prepare(sr);for(int frames:blocks){
  for(int mode=0;mode<=9;++mode){
   synth->setParam(6020,0);synth->setParam(9002,0);
   synth->setParam(3100,.001);synth->setParam(3102,100);synth->setParam(3103,.015);
   for(int e=0;e<4;++e){synth->setParam(1000+e*32,mode);synth->setParam(1001+e*32,e==0?1:0);synth->setParam(1002+e*32,85);}
   for(int j=0;j<4;++j){synth->setParam(5001+j*2,0);}
   synth->reset();synth->noteOn(60,.9,0);
   double peak=0;int first=-1;
   for(int offset=0;offset<4096;offset+=frames){
    synth->process64(l.data(),r.data(),frames);
    for(int i=0;i<frames;++i){const double value=std::max(std::abs(l[i]),std::abs(r[i]));if(!std::isfinite(value))return 2;peak=std::max(peak,value);if(first<0&&value>1e-5)first=offset+i;}
   }
   if(first<0 || double(first)*1000./sr>=3.5){std::cerr<<"CP61 late intrinsic onset mode="<<mode<<" sr="<<sr<<" frame="<<frames<<" ms="<<double(first)*1000/sr<<" peak="<<peak<<"\n";return 3;}
   if(mode==0){
    synth->noteOff(60,0);
    const int cycles=(int(std::ceil(.18*sr/frames)));
    for(int i=0;i<cycles;++i)synth->process64(l.data(),r.data(),frames);
    if(synth->activeVoices()!=0){std::cerr<<"CP61 stuck normal note off\n";return 4;}
   }
   ++cases;
  }
 }}
 synth->prepare(48000);synth->setParam(6020,0);synth->setParam(3100,.001);synth->setParam(3103,.01);
 for(int e=0;e<4;++e)synth->setParam(1001+32*e,e==0?1:0);
 synth->noteOn(60,.9,0);synth->noteOn(60,.9,1);
 synth->noteOff(60,2); // Host changed channel on release. Release oldest only.
 for(int i=0;i<40;++i)synth->process64(l.data(),r.data(),1024);
 if(synth->activeVoices()!=1){std::cerr<<"CP61 missing-channel first off should leave exactly one voice, got "<<synth->activeVoices()<<"\n";return 5;}
 synth->noteOff(60,2);
 for(int i=0;i<40;++i)synth->process64(l.data(),r.data(),1024);
 if(synth->activeVoices()!=0){std::cerr<<"CP61 mismatched-channel repeated off leaked voice\n";return 6;}
 synth->noteOn(62,.8,2,10001);synth->noteOn(62,.8,3,10002);
 synth->noteOff(62,7,10002); // Explicit note ID beats wrong channel.
 for(int i=0;i<40;++i)synth->process64(l.data(),r.data(),1024);
 if(synth->activeVoices()!=1){std::cerr<<"CP61 CLAP ID wrong channel off stole/leaked notes\n";return 7;}
 synth->noteOff(62,7,10001);
 for(int i=0;i<40;++i)synth->process64(l.data(),r.data(),1024);
 if(synth->activeVoices()!=0){std::cerr<<"CP61 CLAP ID stuck note\n";return 8;}
 synth->setParam(6020,1);synth->noteOn(65,.85,2);synth->noteOn(65,.85,3);
 synth->noteOff(65,7);synth->noteOff(65,7); // Arp held-key owner must match voice owner.
 synth->setParam(6020,0);
 for(int i=0;i<60;++i)synth->process64(l.data(),r.data(),1024);
 if(synth->activeVoices()!=0){std::cerr<<"CP61 arp-off transition resurrected stale held note\n";return 9;}
 synth->midiCC(64,127,0);synth->noteOn(67,.8,0);synth->noteOff(67,0);synth->midiCC(121,0,0);
 for(int i=0;i<50;++i)synth->process64(l.data(),r.data(),1024);
 if(synth->activeVoices()!=0){std::cerr<<"CP61 sustain reset leaked voice\n";return 10;}
 std::cout<<"CP61 PASS: "<<cases<<" 1ms arp-off onset+release cases, 20 buffer/rate, 10 modes; same-pitch channel mismatch, CLAP exact ID, arp transition, CC121\n";
}