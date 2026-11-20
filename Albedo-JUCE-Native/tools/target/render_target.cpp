#include "AlbedoPrograms.hpp"
#include "vitriol_dsp.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif
int main(){
#if defined(_WIN32)
 _setmode(_fileno(stdout),_O_BINARY);
#endif
 auto s=std::make_unique<vitriol::Synth>();std::string line;
 while(std::getline(std::cin,line)){
  std::istringstream in(line);int program,note,total,held,state;double velocity;
  if(!(in>>program>>note>>velocity>>held>>total>>state)||program<0||program>=128||note<0||note>127||total<1||total>384000||held<1||held>total)return 2;
  albedo_programs::apply(program,[&](uint32_t id,float v){s->setParam(id,v);});
  unsigned id;double v;while(in>>id>>v)if(!std::isfinite(v)||!s->setParam(id,v))return 3;
  s->prepare(48000);
  if(state){
   uint32_t count=(uint32_t)s->descriptors().size();std::cout.write((char*)&count,4);
   for(auto& d:s->descriptors()){double value=s->getParam(d.id);std::cout.write((const char*)&d.id,4);std::cout.write((char*)&value,8);}
  }else{
   s->noteOn(note,velocity);
   for(int pos=0;pos<total;){
    if(pos==held)s->noteOff(note);
    int n=std::min(128,total-pos);if(pos<held)n=std::min(n,held-pos);
    std::array<float,128> l{},r{};std::array<float,256> stereo{};
    s->process(l.data(),r.data(),n);
    for(int j=0;j<n;j++){stereo[2*j]=l[j];stereo[2*j+1]=r[j];}
    std::cout.write((char*)stereo.data(),n*8);pos+=n;
   }
  }
  std::cout.flush();
 }
}
