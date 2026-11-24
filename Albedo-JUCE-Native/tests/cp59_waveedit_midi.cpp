#include "vitriol_dsp.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <memory>

// CI loads RCDATA 102/103 from the Windows test executable. It never assumes
// that a source-contract check proves actual audio or MIDI lifecycle.
int main(){
 constexpr std::array<int,4> sampleRates{{44100,48000,88200,96000}};
 constexpr std::array<int,5> bufferSizes{{64,128,256,512,1024}};
 auto synth=std::make_unique<vitriol::Synth>();
 if(!synth->waveEditReady()||!synth->resourcesReady()){
  std::cerr<<"CP59 ERROR: embedded WaveEdit or original PCM bank missing\n";return 1;
 }
 int cases=0;
 for(int sr:sampleRates)for(int block:bufferSizes)for(int m:{1,3}){
  synth->prepare(sr);synth->setParam(6020,0);
  synth->setParam(1000,m);synth->setParam(1010,32+(cases%24));
  synth->setParam(1032,m==1?3:1);synth->setParam(1042,32+((cases+7)%24));
  synth->setParam(3103,.015);
  for(int note=48;note<60;++note)synth->noteOn(note,.62,note%3);
  if(synth->activeVoices()<12)return 2;
  std::array<double,1024> left{},right{};
  double energy=0;
  for(int rendered=0;rendered<4096;rendered+=block){
   synth->process64(left.data(),right.data(),(uint32_t)block);
   for(int i=0;i<block;++i){
    const double a=left[i],b=right[i];
    if(!std::isfinite(a)||!std::isfinite(b))return 3;
    energy+=a*a+b*b;
   }
  }
  if(energy<1.e-12){std::cerr<<"CP59 silence rate="<<sr<<" block="<<block<<" material="<<m<<"\n";return 4;}
  for(int note=48;note<60;++note)synth->noteOff(note,note%3);
  for(int i=0;i<40;++i)synth->process64(left.data(),right.data(),(uint32_t)block);
  if(synth->activeVoices()>0){std::cerr<<"CP59 stuck MIDI after note offs\n";return 5;}
  synth->allSoundOff();
  if(synth->activeVoices()!=0)return 6;
  ++cases;
 }
 // A disconnected sustain-pedal source may send Reset All Controllers (121),
 // not a second CC64. No orphaned held voices may survive that event.
 synth->prepare(48000);synth->setParam(1000,3);synth->setParam(1010,37);
 synth->midiCC(64,127,2);
 synth->noteOn(57,.8,2);synth->noteOff(57,2);
 if(!synth->activeVoices())return 7;
 synth->midiCC(121,0,2);
 std::array<double,1024> left{},right{};
 for(int i=0;i<30;i++)synth->process64(left.data(),right.data(),1024);
 if(synth->activeVoices()){
  std::cerr<<"CP59 stuck sustain after CC121\n";return 8;
 }
 synth->setParam(6020,1);
 synth->noteOn(62,.8,3);
 synth->process64(left.data(),right.data(),1024);
 synth->noteOff(62,4); // MIDI bridge lost the channel: unique pitch fallback
 for(int i=0;i<60;i++)synth->process64(left.data(),right.data(),1024);
 if(synth->activeVoices()){
  std::cerr<<"CP59 stuck arp after mismatch channel note-off\n";return 9;
 }
 synth->reset();
 if(synth->activeVoices())return 10;
 std::cout<<"CP59 PASS: "<<cases<<" 12-voice, 5-block, 4-sample-rate, CC121 and arp note-off tests\n";
 return 0;
}