#include "vitriol_dsp.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <memory>

int main(){
  using vitriol::Synth;
  auto synth=std::make_unique<Synth>();
  constexpr std::array<int,4> sampleRates{44100,48000,88200,96000};
  constexpr std::array<int,3> blocks{64,256,1024};
  std::array<double,1024> left{},right{};
  unsigned cases=0;
  for(int sr:sampleRates){
    synth->prepare(sr);
    synth->setParam(6020,0); // arp OFF, including after all-notes-off
    synth->setParam(3100,.001); synth->setParam(3103,.012);
    for(int e=0;e<4;++e) {
      synth->setParam(1000+32*e,e==0?1:0);
      synth->setParam(1001+32*e,e==0?1:0);
    }
    for(int block:blocks){
      for(int pass=0;pass<15;++pass){
        // The host can receive several Note Ons with the same pitch on
        // separate input channels and then route each Note Off on channel 7.
        const int pitch=48+pass%24;
        for(int ch=0;ch<3;++ch)synth->noteOn(pitch,.7,ch);
        for(int ch=0;ch<3;++ch)synth->noteOff(pitch,7);
        for(int i=0;i<30;++i)synth->process64(left.data(),right.data(),block);
        if(synth->activeVoices()!=0){std::cerr<<"stuck after repeated channel remap, pass="<<pass<<"\n";return 1;}

        // Switching presets in JUCE issues AllNotesOff and fast panic release.
        // Repeat because the reported problem returns after switching.
        synth->midiCC(64,127,1);
        synth->noteOn(pitch+1,.8,1);
        synth->noteOff(pitch+1,1);
        synth->allNotesOff();
        for(int i=0;i<30;++i)synth->process64(left.data(),right.data(),block);
        if(synth->activeVoices()!=0){std::cerr<<"stuck after preset panic, pass="<<pass<<"\n";return 2;}
        synth->midiCC(121,0,1); // some keyboards send controller reset later
        ++cases;
      }
    }
  }
  std::cout<<"PASS: "<<cases<<" recurring mismatched MIDI channel and preset panic recovery cases\n";
}