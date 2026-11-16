#include "../src/clap_abi_subset.h"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <stdint.h>
#include <stdlib.h>

static const void* ext(const clap_host*,const char*){return nullptr;}
static void nop(const clap_host*){}
static const clap_host HOST={{1,2,0},nullptr,"Nigredo QA host","Nigredo QA",nullptr,"0.5",ext,nop,nop,nop};
static bool push(const clap_output_events*o,const clap_event_header*ev){auto*v=(std::vector<clap_event_note>*)o->ctx;if((ev->type==0||ev->type==1)&&ev->size>=sizeof(clap_event_note))v->push_back(*(const clap_event_note*)ev);return true;}
struct In {std::vector<clap_event_header*> items;clap_input_events in={this,&size,&get};static uint32_t size(const clap_input_events*e){return (uint32_t)((In*)e->ctx)->items.size();}
static const clap_event_header*get(const clap_input_events*e,uint32_t idx){auto*self=(In*)e->ctx;return idx<self->items.size()?self->items[idx]:nullptr;}};
struct Bytes {std::vector<char> buf;size_t head=0;clap_ostream out={this,&write};clap_istream in={this,&read};static int64_t write(const clap_ostream*o,const void*b,uint64_t n){Bytes*self=(Bytes*)o->ctx;self->buf.insert(self->buf.end(),(const char*)b,(const char*)b+n);return n;}
 static int64_t read(const clap_istream*o,void*b,uint64_t n){Bytes*self=(Bytes*)o->ctx;size_t end=self->head+n;if(end>self->buf.size())return -1;memcpy(b,self->buf.data()+self->head,n);self->head=end;return n;}};
int main(int argc,char**argv){if(argc<3)return 3;bool midi=strcmp(argv[2],"midi")==0;void*lib=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);if(!lib){fprintf(stderr,"dlopen: %s\n",dlerror());return 4;}
 auto entry=(const clap_plugin_entry*)dlsym(lib,"clap_entry");if(!entry||!entry->init("."))return 5;auto factory=(const clap_plugin_factory*)entry->get_factory("clap.plugin-factory");if(!factory||factory->get_plugin_count(factory)!=1)return 6;
 auto desc=factory->get_plugin_descriptor(factory,0);if(!desc||!strstr(desc->name,midi?"MIDI":"Audio"))return 7;
 auto*plugin=factory->create_plugin(factory,&HOST,desc->id);if(!plugin||!plugin->init(plugin))return 8;
 auto params=(const clap_plugin_params*)plugin->get_extension(plugin,"clap.params");auto state=(const clap_plugin_state*)plugin->get_extension(plugin,"clap.state");if(!params||!state||params->count(plugin)!=82)return 9;
 for(uint32_t id=0;id<params->count(plugin);id++){clap_param_info pi{};double val=0;if(!params->get_info(plugin,id,&pi)||!params->get_value(plugin,id,&val)||val<pi.min_value||val>pi.max_value){fprintf(stderr,"invalid param %u\n",id);return 10;}}
 Bytes bytes;if(!state->save(plugin,&bytes.out)||bytes.buf.size()<600||!state->load(plugin,&bytes.in))return 11;
 if(!plugin->activate(plugin,48000,1,256))return 12;if(!plugin->start_processing(plugin))return 13;
 In empty;std::vector<clap_event_note> noteOutput;clap_output_events outputEvents{&noteOutput,&push};
 const int block=256;float lin[block],rin[block],lout[block],rout[block];float*src[]={lin,rin};float*dst[]={lout,rout};double rms=0;int total=0;
 clap_audio_buffer inBuf{};inBuf.data32=src;inBuf.channel_count=2;clap_audio_buffer outBuf{};outBuf.data32=dst;outBuf.channel_count=2;
 clap_event_note on{};on.header={sizeof(on),1,0,CLAP_EVENT_NOTE_ON,0};on.note_id=234;on.channel=0;on.key=60;on.velocity=.74;
 clap_event_note off{};off.header={sizeof(off),200,0,CLAP_EVENT_NOTE_OFF,0};off.note_id=234;off.channel=0;off.key=60;off.velocity=.4;
 for(int blockIdx=0;blockIdx<64;blockIdx++){
  for(int j=0;j<block;j++){lin[j]=.22f*sinf((blockIdx*block+j)*.015);rin[j]=.17f*cosf((blockIdx*block+j)*.02);lout[j]=rout[j]=0;}
  In in; if(midi&&blockIdx==0){in.items.push_back(&on.header);in.items.push_back(&off.header);}
  clap_process data{};data.frames_count=block;data.audio_inputs=midi?nullptr:&inBuf;data.audio_inputs_count=midi?0:1;data.audio_outputs=midi?nullptr:&outBuf;data.audio_outputs_count=midi?0:1;data.in_events=&in.in;data.out_events=&outputEvents;
  if(plugin->process(plugin,&data)!=1)return 14;
  if(!midi)for(int j=0;j<block;j++){if(!isfinite(lout[j])||!isfinite(rout[j]))return 15;rms+=lout[j]*lout[j]+rout[j]*rout[j];}total+=block;
 }
 if(midi){int nOn=0,nOff=0;for(auto&e:noteOutput){nOn+=e.header.type==CLAP_EVENT_NOTE_ON;nOff+=e.header.type==CLAP_EVENT_NOTE_OFF;}printf("MIDI CLAP: %d generated on, %d generated off, %d processed samples\n",nOn,nOff,total);if(nOn<1||nOff<1)return 16;}
 else{printf("Audio CLAP: 16,384 finite samples/channel, energy %.5f\n",rms);if(rms<1.)return 17;}
 plugin->stop_processing(plugin);plugin->deactivate(plugin);plugin->destroy(plugin);entry->deinit();dlclose(lib);return 0;
}
