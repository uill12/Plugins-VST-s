#include "vitriol_dsp.hpp"
#include <cstdio>
#include <memory>
int main(){
 for(int rate:{44100,48000,88200,96000})for(int block:{64,257}){
 auto s=std::make_unique<vitriol::Synth>();s->prepare(rate);
 for(int e=0;e<4;e++){int b=1000+32*e;s->setParam(b+1,1);s->setParam(b,4);s->setParam(b+9,e*30);}
 s->setParam(6020,0);s->setParam(4001,75);
 for(int n=48;n<60;n++)s->noteOn(n,.7);
 double l[257],r[257];
 for(int i=0;i<220;i++){if(i%15==0)s->setParam(1007,(i*7)%101);if(i==180)s->allNotesOff();s->process64(l,r,block);fwrite(l,8,block,stdout);fwrite(r,8,block,stdout);}
 }
}
