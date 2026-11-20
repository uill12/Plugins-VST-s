#include "vitriol_dsp.hpp"
#include <array>
#include <iostream>
#include <memory>
#include <vector>
int main(){
 for(int rate:{44100,48000,88200,96000}){
  for(int mode=0;mode<5;++mode){
   auto s=std::make_unique<vitriol::Synth>();s->prepare(rate);s->setArpTempo(120);
   s->setParam(6020,1);s->setParam(6021,.25);s->setParam(6023,50);s->setParam(6024,mode);s->setParam(6025,2);
   for(int i=0;i<16;++i)s->setParam(6000+i,1);
   for(int n:{67,60,64})s->noteOn(n,.8);
   std::array<double,128> l{},r{};std::vector<int> notes;
   int last=-1;
   for(int total=0;total<rate*3/4;total+=128){s->process64(l.data(),r.data(),128);if(s->arpStep()!=last){last=s->arpStep();notes.push_back(s->arpNote());}}
   const std::array<std::vector<int>,4> expected={{{60,64,67,72,76,79},{79,76,72,67,64,60},{60,64,67,72,76,79},{67,60,64,79,72,76}}};
   if(notes.size()<6)return 1;
   if(mode<4&&!std::equal(expected[mode].begin(),expected[mode].end(),notes.begin()))return 2;
   s->midiCC(64,127);for(int n:{67,60,64})s->noteOff(n);s->midiCC(64,0);s->process64(l.data(),r.data(),128);
   if(s->arpNote()!=-1||s->arpStep()!=-1)return 3;
   s->noteOn(61,.8);s->setParam(6020,0);s->process64(l.data(),r.data(),128);s->noteOff(61);s->allSoundOff();
   if(s->activeVoices())return 4;
  }
 }
 std::cout<<"Arpeggiator order, note clock, pedal release and bypass passed at four sample rates\n";
}
