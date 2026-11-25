#include <vitriol_dsp.hpp>
#include <array>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>
int main(){
 constexpr int block=64;
 constexpr double tempo=110.0,division=.25;
 for(int sampleRate:{44100,48000,88200,96000}){
  for(bool sync:{false,true}){
   auto s=std::make_unique<vitriol::Synth>();
   s->prepare(sampleRate);
   s->setParam(6020,1);             // ARP on
   s->setParam(6021,division);      // 1/16 = quarter beat
   s->setParam(6024,2);             // Cyclical, no character rests
   s->setParam(6031,0);             // no Wolfram rests
   s->setParam(6026,0);             // no swing for baseline timing
   s->setParam(6023,68);
   s->setParam(6027,8);
   s->setParam(6030,sync?1:0);
   s->setParam(6029,tempo);
   for(int i=0;i<16;i++)s->setParam(6000+i,1); // no gate-mask rests
   s->setArpTempo(tempo);
   s->setArpTransport(0.,sync);
   s->noteOn(60,.8,0);
   std::array<double,block> left{},right{};
   std::vector<double> times;
   int previous=-1;
   for(int total=0;total<sampleRate*2;total+=block){
    if(sync){
      // Mirror Bitwig: absolute quarter-note PPQ at the START of each
      // audio block, not a continuously reconstructed floating clock.
      const double ppq=(double(total)/sampleRate)*(tempo/60.);
      s->setArpTransport(ppq,true);
    }
    s->process64(left.data(),right.data(),block);
    const int step=s->arpStep();
    if(step>=0&&step!=previous){times.push_back(double(total));previous=step;}
   }
   const double expected=sampleRate*60.0/tempo*division; // 136.36 ms at 110 BPM
   if(times.size()<11||times.size()>17){
    std::cerr<<"CP64 ARP missing/extra steps at "<<sampleRate<<"Hz sync="<<sync
             <<" count="<<times.size()<<" expected about 15\n";return 1;
   }
   for(size_t i=2;i<times.size();++i){
    const double observed=times[i]-times[i-1];
    const double error=std::abs(observed-expected);
    if(error>block*2.5){
     std::cerr<<"CP64 ARP clock mismatch sr="<<sampleRate
              <<" sync="<<sync<<" step="<<i<<" observed="<<observed
              <<" expected="<<expected<<" frames\n";return 2;
    }
   }
   s->noteOff(60,0);
   s->allNotesOff();
  }
 }
 std::cout<<"CP64 1/16 clock @110 BPM PASS: 136.36ms, 44.1/48/88.2/96kHz, sync/internal\n";
}
