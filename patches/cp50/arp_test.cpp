#include "vitriol_dsp.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <vector>
// Exact Nereida CP-03B character/event patterns ported to Albedo CP50.
// No pitch drift: octave shifts only when Octaves >= 2. -1 represents a rest.
static int arpCellularIndex(int mode,uint32_t cells,uint32_t step,int count,int rotate,bool* hit,bool* emphasis){
 if(count<2){*hit=true;*emphasis=(cells&1u)!=0;return 0;}
 const unsigned shift=(unsigned)((step*7u+uint32_t(rotate)*3u)&31u);
 const uint32_t window=(cells>>shift)|(cells<<((32u-shift)&31u));
 const int b0=(int)(window&1u),b1=(int)((window>>1)&1u),b2=(int)((window>>2)&1u);
 const int walk=(int)((window>>3)&7u);
 int k=(int)(step%uint32_t(count));
 *hit=true;*emphasis=b0!=0;
 switch(mode){
  case 4: // Rule30 / CHAOS: irregular bounded chord traversal
   k=(k+walk+b1*2)%count;*hit=(b0||b1||((step&3u)==0u));break;
  case 5: { // Rule90 / FRACTAL: additive-rule mirrored rise/fall
   int span=2*count-2,t=(int)((step+uint32_t(b0+b2))%uint32_t(span));
   k=t<count?t:span-t;*hit=b0||((step&1u)==0u);break;
  }
  case 6: // Rule110 / GLIDER: paired steps interrupted by local domains
   k=(int)(((step/2u)+uint32_t(walk%3))%uint32_t(count));*hit=(b0||b1||((step&3u)!=3u));break;
  case 7: // Rule150 / WEAVE: alternating opposing voices through the held chord
   k=(int)(((step/2u)+(b0?uint32_t(count)-1u:0u))%uint32_t(count));
   if((step&1u)!=0u)k=count-1-k;break;
  case 8: // Rule30 / SPARSE: gaps and irregular accents
   k=(int)((step*3u+uint32_t(walk))%uint32_t(count));*hit=b0&&(b1||((step&3u)==0u));break;
  case 9: { // Rule90 / MIRROR: fractal palindromic scan
   int span=2*count-2,t=(int)((step+uint32_t(b0))%uint32_t(span));
   k=count-1-(t<count?t:span-t);if(b2)k=(k+count-1)%count;
   *hit=b0||b2||((step&3u)!=3u);break;
  }
  case 10: // Rule110 / DOMAIN: stable local clusters with occasional shifts
   k=(int)(((step/3u)+uint32_t((window>>5)&3u))%uint32_t(count));
   *hit=b0||b1||((step%3u)==0u);break;
  case 11: // Rule150 / BRAID: alternate interleaved trajectories
   k=(int)((((step/2u)*2u)+((step&1u)?uint32_t(count)-1u:0u)+uint32_t(b1))%uint32_t(count));
   *hit=b0||((step&3u)!=3u);break;
 }
 return (k+rotate)%count;
}

static inline uint32_t nereidaCellularNext(uint32_t cells,int selected){
 static const uint32_t rules[4]={30u,90u,110u,150u};
 uint32_t rule=rules[selected<0?0:selected>3?3:selected];
 uint32_t left=(cells<<1)|(cells>>31),right=(cells>>1)|(cells<<31);
 uint32_t next=0;
 for(int pattern=0;pattern<8;pattern++)if(rule&(1u<<pattern)){
  uint32_t l=(pattern&4)?left:~left;
  uint32_t c=(pattern&2)?cells:~cells;
  uint32_t r=(pattern&1)?right:~right;
  next|=l&c&r;
 }
 return next;
}

int main(){
 const std::array<std::vector<int>,4> expected={{
  {60,67,79,64,76,60,72,-1},   // Oblique: diagonal crossing + eighth-step gap
  {60,64,67,72,76,79,76,72},   // Symmetrical: mirror
  {60,64,67,72,76,79,60,64},   // Cyclical: circular
  {60,-1,76,-1,72,67,-1,64}   // Asymmetrical: five of eight gates
 }};
 for(int sampleRate:{44100,48000,88200,96000}){
  for(int character=0;character<12;++character){
   auto s=std::make_unique<vitriol::Synth>();
   s->prepare(sampleRate);
   s->setParam(6020,1);s->setParam(6021,.25);s->setParam(6022,0);
   s->setParam(6023,68);s->setParam(6024,character%4);s->setParam(6031,character>=4?character-3:0);
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
   std::vector<int> reference=expected[size_t(character%4)];
   if(character>=4){
    uint32_t cells=0x00010001u;
    const int keys[]={60,64,67,72,76,79};
    for(unsigned step=0;step<8;++step){
     cells=nereidaCellularNext(cells,(character-4)&3);
     if(!cells)cells=1u<<((step*7u+11u)&31u);
     bool hit,emphasis;
     int pos=arpCellularIndex(character,cells,step,6,0,&hit,&emphasis);
     reference[step]=hit?keys[pos]:-1;
    }
   }
   if(notes!=reference){
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
 // CP57: an off-beat keypress waits for the next host-aligned sixteenth note.
 for(int sr:{44100,48000,88200,96000}){
  auto synced=std::make_unique<vitriol::Synth>();synced->prepare(sr);
  synced->setParam(6020,1);synced->setParam(6021,.25);
  synced->setParam(6030,1);synced->setParam(6026,0);
  synced->setArpTempo(120);synced->setArpTransport(.0625,true);
  synced->noteOn(60,.8);
  std::array<double,128> sl{},rr{};
  synced->process64(sl.data(),rr.data(),128);
  if(synced->arpStep()!=-1){std::cerr<<"Arp started outside grid at "<<sr<<"\\n";return 5;}
  for(int i=128;i<sr/8&&synced->arpStep()!=1;i+=128)
   synced->process64(sl.data(),rr.data(),128);
  if(synced->arpStep()!=1){std::cerr<<"Host grid tick missing at "<<sr<<"\\n";return 6;}
  synced->setArpTransport(8.0,true);
  synced->process64(sl.data(),rr.data(),128);
  if(synced->arpStep()!=0){std::cerr<<"Transport seek not re-phased at "<<sr<<"\\n";return 7;}
  synced->setArpTransport(8.0,false);
  synced->allNotesOff();
 }
 std::cout<<"Nereida character, silence mask, rotation, 1-octave pitch lock, sustain, bypass PASS at 44.1/48/88.2/96 kHz\n";
}
