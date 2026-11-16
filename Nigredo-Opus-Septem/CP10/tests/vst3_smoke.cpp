#include "vst3_abi.h"
#include <dlfcn.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
using i32=int32_t;using u32=uint32_t;using u64=uint64_t;
static void guidval(uint8_t*out,uint32_t a,uint32_t b,uint32_t c,uint32_t d){uint32_t w[4]={a,b,c,d};for(int j=0;j<4;j++)for(int k=0;k<4;k++)out[j*4+k]=(uint8_t)(w[j]>>(24-k*8));}
struct EventIn {const EventsVTable*vt;std::vector<Event>v;};
static int evCount(void*self){return (int)((EventIn*)self)->v.size();}
static int evGet(void*self,int idx,Event*out){auto*x=(EventIn*)self;if(idx<0||idx>=int(x->v.size()))return 1;*out=x->v[idx];return 0;}
static int evAppend(void*self,Event*ev){((EventIn*)self)->v.push_back(*ev);return 0;}
static const EventsVTable INEV={nullptr,nullptr,nullptr,&evCount,&evGet,&evAppend};
int main(int argc,char**argv){if(argc<3)return 3;bool midi=!strcmp(argv[2],"midi");void*lib=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);if(!lib){fprintf(stderr,"dlopen %s\n",dlerror());return 4;}
 auto factoryFn=(void*(*)())dlsym(lib,"GetPluginFactory");if(!factoryFn)return 5;void*factory=factoryFn();if(!factory)return 6;auto*fvt=*(FactoryVTable**)factory;
 if(fvt->count(factory)!=2)return 7;ClassInfo processor{},controller{};
 if(fvt->classInfo(factory,0,&processor)!=0||fvt->classInfo(factory,1,&controller)!=0)return 8;
 if(!strstr(processor.name,midi?"MIDI":"Audio"))return 9;
 uint8_t componentGuid[16],audioGuid[16],ctrlGuid[16];guidval(componentGuid,0xE831FF32,0xF2D54301,0x928EBBEE,0x25697802);guidval(audioGuid,0x42043F99,0xB7DA453C,0xA569E79D,0x9AAEC33D);guidval(ctrlGuid,0xDCD7BBE4,0x7742448D,0xA874AACC,0x979C759E);
 void*component=nullptr;void*control=nullptr;
 if(fvt->create(factory,processor.cid,componentGuid,&component)!=0||!component)return 10;
 if(fvt->create(factory,controller.cid,ctrlGuid,&control)!=0||!control)return 11;
 auto*cvt=*(ComponentVTable**)component;auto*ctrlvt=*(ControllerVTable**)control;
 if(ctrlvt->paramCount(control)!=82)return 12;
 for(int i=0;i<82;i++){ParameterInfo info{};if(ctrlvt->paramInfo(control,i,&info)!=0||info.id!=(unsigned)i)return 13;}
 if(cvt->initialize(component,nullptr)!=0)return 14;
 void*proc=nullptr;if(cvt->qi(component,audioGuid,&proc)!=0||!proc)return 15;
 auto*vt=*(ProcessorVTable**)proc;
 ProcessSetup setup{0,0,256,48000.};if(vt->setup(proc,&setup)!=0)return 16;
 if(vt->latency(proc)!=(midi?0:480)){fprintf(stderr,"latency %u\n",vt->latency(proc));return 17;}
 int num=256;float l[256],r[256],ol[256],orr[256];float*inputs[]={l,r},*outputs[]={ol,orr};AudioBusBuffers inBuf{2,0,{}},outBuf{2,0,{}};inBuf.channelBuffers32=inputs;outBuf.channelBuffers32=outputs;
 EventIn inEvents{&INEV,{}},outEvents{&INEV,{}};
 Event on{},off{};on.type=0;on.sampleOffset=4;int16_t ch=0,key=60;float vel=.85f;int32_t noteId=234;
 memcpy(on.payload+0,&ch,2);memcpy(on.payload+2,&key,2);memcpy(on.payload+8,&vel,4);memcpy(on.payload+16,&noteId,4);
 off.type=1;off.sampleOffset=198;memcpy(off.payload+0,&ch,2);memcpy(off.payload+2,&key,2);memcpy(off.payload+4,&vel,4);memcpy(off.payload+8,&noteId,4);
 double energy=0.;for(int block=0;block<80;block++){
  for(int j=0;j<num;j++){l[j]=.2f*sin((block*num+j)*.008);r[j]=.17f*cos((block*num+j)*.012);ol[j]=orr[j]=0;}
  if(midi && block==0){inEvents.v.push_back(on);inEvents.v.push_back(off);}else inEvents.v.clear();
  ProcessData data{};data.numSamples=num;data.symbolicSampleSize=0;data.numInputs=midi?0:1;data.numOutputs=midi?0:1;data.inputs=midi?nullptr:&inBuf;data.outputs=midi?nullptr:&outBuf;data.inputEvents=midi?&inEvents:nullptr;data.outputEvents=midi?&outEvents:nullptr;
  if(vt->process(proc,&data)!=0){fprintf(stderr,"process fails\n");return 18;}
  if(!midi)for(int j=0;j<num;j++){if(!std::isfinite(ol[j])||!std::isfinite(orr[j]))return 19;energy+=ol[j]*ol[j]+orr[j]*orr[j];}
 }
 if(midi){int onCt=0,offCt=0;for(auto&ev:outEvents.v){onCt+=ev.type==0;offCt+=ev.type==1;}
  printf("VST3 MIDI: %d note-on / %d note-off\n",onCt,offCt);if(!onCt||!offCt)return 20;
 }else{printf("VST3 Audio: 20,480 finite frames, energy %.5f, latency %u samples\n",energy,vt->latency(proc));if(energy<1.)return 21;}
 vt->release(proc);cvt->terminate(component);cvt->release(component);ctrlvt->release(control);dlclose(lib);return 0;
}
