#include "vitriol_dsp.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <vector>
// Exact Nereida CP-03B character/event patterns ported to Albedo CP50.
// No pitch drift: octave shifts only when Octaves >= 2. -1 represents a rest.
int main(){
 const std::array<std::vector<int>,4> expected={{
  {60,67,79,64,76,60,72,-1},   // Oblique: diagonal crossing + eighth-step gap
  {60,64,67,72,76,79,76,72},   // Symmetrical: mirror
  {60,64,67,72,76,79,60,64},   // Cyclical: circular
  {60,-1,76,-1,72,67,-1,64}   // Asymmetrical: five of eight gates
 }};
 for(int sampleRate:{44100,48000,88200,96000}){
  for(int character=0;character<4;++character){
   auto s=std::make_unique<vitriol::Synth>();
   s->prepare(sampleRate);
   s->setParam(6020,1);s->setParam(6021,.25);s->setParam(6022,0);
   s->setParam(6023,68);s->setParam(6024,character);
   s->setParam(6025,2);s->setParam(6026,0);s->setParam(6027,8);
   s->setParam(6028,0);s->setParam(6029,120);s->setParam(6030,0);
   for(int i=0;i<16;++i)s->setParam(6000+i,1);
   for(int n:{67,60,64})s->noteOn(n,.8);
   std::array<double,128> l{},r{};
   std::vector<int> notes;
   int lastStep=-1;
   for(int total=0;total<sampleRate*11/10 && notes.size()<8;total+=128){
    s->process64(l.data(),r.data(),128);
    const int step=s->arpStep();
    if(step>=0 && step!=lastStep){lastStep=step;notes.push_back(s->arpNote());}
   }
   if(notes!=expected[size_t(character)]){
    std::cerr<<"FAIL rate="<<sampleRate<<" char="<<character<<" notes:";
    for(auto n:notes)std::cerr<<" "<<n;
    std::cerr<<"\n";return 1;
   }
   // Sustain pedal should retain the chord until release, then clear the clock.
   s->midiCC(64,127);
   for(int n:{67,60,64})s->noteOff(n);
   s->midiCC(64,0);
   s->process64(l.data(),r.data(),128);
   if(s->arpStep()!=-1||s->arpNote()!=-1)return 2;
   // Disabling the arpeggiator restores ordinary stable-pitch polyphony.
   s->noteOn(61,.8);s->setParam(6020,0);
   s->process64(l.data(),r.data(),128);
   s->noteOff(61);s->allSoundOff();
   if(s->activeVoices())return 3;
  }
 }
 // Rotation and 1-octave mode must never transpose notes unexpectedly.
 auto s=std::make_unique<vitriol::Synth>();s->prepare(48000);
 s->setParam(6020,1);s->setParam(6021,.25);s->setParam(6024,2);
 s->setParam(6025,1);s->setParam(6027,8);s->setParam(6028,1);
 s->setParam(6030,0);s->setParam(6029,120);
 for(int i=0;i<16;++i)s->setParam(6000+i,1);
 s->noteOn(60,.7);s->noteOn(64,.7);s->noteOn(67,.7);
 std::array<double,128> l{},r{};std::vector<int> got;int prev=-1;
 for(int i=0;i<48000&&got.size()<3;i+=128){
  s->process64(l.data(),r.data(),128);
  if(s->arpStep()>=0&&s->arpStep()!=prev){prev=s->arpStep();got.push_back(s->arpNote());}
 }
 if(got!=std::vector<int>{64,67,60})return 4;
 s->allNotesOff();
 std::cout<<"Nereida character, silence mask, rotation, 1-octave pitch lock, sustain, bypass PASS at 44.1/48/88.2/96 kHz\n";
}
