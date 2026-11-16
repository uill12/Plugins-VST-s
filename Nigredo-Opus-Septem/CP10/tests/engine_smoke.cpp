#include "../src/Engine.h"
#include <cstdio>
int main(){static nig::AudioEngine a;a.defaults();a.prepare(48000);auto x=a.frame({.3,.3});nig::MidiEngine m;m.defaults();m.prepare(48000);m.noteOn(22,0,60,.7,1);m.noteOff(22,0,60,0.,100);int out=0;m.drain(0,20000,[&](nig::Note,uint32_t){out++;});std::printf("%.4f %.4f events %d\n",x.l,x.r,out);}
