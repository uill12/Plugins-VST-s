#include "Engine.h"
#include "clap_abi_subset.h"
#ifdef _WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT extern "C" __attribute__((visibility("default")))
#endif
#ifndef NIG_MODE
#define NIG_MODE 0
#endif
using namespace nig;
static const int MODE=NIG_MODE;
static bool same(const char*a,const char*b){if(!a||!b)return false;while(*a&&*b&&*a==*b){a++;b++;}return !*a&&!*b;}
static void copy(char* out,int max,const char* src){if(!out||max<=0)return;int n=0;while(src&&src[n]&&n<max-1){out[n]=src[n];n++;}out[n]=0;}
static void zero(void* ptr,uint32_t size){uint8_t*p=(uint8_t*)ptr;for(uint32_t i=0;i<size;i++)p[i]=0;}
static void bufcpy(void* dest,const void*src,uint32_t n){uint8_t*d=(uint8_t*)dest;const uint8_t*s=(const uint8_t*)src;for(uint32_t i=0;i<n;i++)d[i]=s[i];}
static double number(const char*s){if(!s)return 0.;bool neg=*s=='-';if(neg||*s=='+')s++;double v=0.;while(*s>='0'&&*s<='9'){v=v*10.+(*s-'0');s++;}if(*s=='.'||*s==','){s++;double m=.1;while(*s>='0'&&*s<='9'){v+=(*s++-'0')*m;m*=.1;}}return neg?-v:v;}
static void numtext(char*b,uint32_t cap,double val){if(!b||cap<3)return;uint32_t n=0;val=clamp(val,-1000000.,1000000.);if(val<0.){b[n++]='-';val=-val;}uint32_t iv=(uint32_t)val;char x[20];int k=0;do{x[k++]=char('0'+iv%10);iv/=10;}while(iv&&k<18);while(k>0&&n<cap-4)b[n++]=x[--k];if(n<cap-4){b[n++]='.';int frac=(int)((val-(uint32_t)val)*1000.+.5);b[n++]=char('0'+(frac/100)%10);b[n++]=char('0'+(frac/10)%10);b[n++]=char('0'+frac%10);}b[n]=0;}

struct ClapInstance {
 clap_plugin plugin;
 const clap_host*host;
 const clap_host_params*hostParams;
 AudioEngine audio;
 MidiEngine midi;
 bool used=false,active=false;
 uint64_t uiBits[ParamCount];
 uint32_t dirty[ParamCount];
 uint32_t lastError=0;
};
static ClapInstance PLUGINS[3]{};
static ClapInstance* inst(const clap_plugin*p){return p?(ClapInstance*)p->plugin_data:nullptr;}
static double readVal(ClapInstance*i,int id){if(!i||id<0||id>=ParamCount)return 0.;uint64_t bits=__atomic_load_n(&i->uiBits[id],__ATOMIC_RELAXED);double value;bufcpy(&value,&bits,8);return value;}
static void setVal(ClapInstance*i,int id,double value){if(!i||id<0||id>=ParamCount)return;value=clamp(value,specs[MODE][id].lo,specs[MODE][id].hi);uint64_t bits;bufcpy(&bits,&value,8);__atomic_store_n(&i->uiBits[id],bits,__ATOMIC_RELAXED);}
static bool opInit(const clap_plugin*){return true;}
static void opDestroy(const clap_plugin*p){if(ClapInstance*i=inst(p)){i->used=false;i->active=false;}}
static bool opActivate(const clap_plugin*p,double sr,uint32_t,uint32_t){ClapInstance*i=inst(p);if(!i||sr<20000.||sr>192000.)return false;
 i->audio.prepare(sr);i->midi.prepare(sr);i->active=true;return true;}
static void opDeactivate(const clap_plugin*p){ClapInstance*i=inst(p);if(i)i->active=false;}
static bool opStart(const clap_plugin*){return true;}
static void opStop(const clap_plugin*){}
static void opReset(const clap_plugin*p){ClapInstance*i=inst(p);if(!i)return;if(MODE==0)i->audio.prepare(i->audio.sampleRate);else i->midi.prepare(i->midi.sr);}
static void applyHostParam(ClapInstance*i,const clap_event_param_value*e){if(!e||e->param_id>=ParamCount)return;setVal(i,(int)e->param_id,e->value);}
static void applyUiParams(ClapInstance*i){for(int k=0;k<ParamCount;k++){
 double value=readVal(i,k);if(MODE==0)i->audio.set(k,value);else i->midi.set(k,value);
}}
static uint32_t inputCount(const clap_input_events*e){return(e&&e->size)?e->size(e):0;}
static const clap_event_header* inputAt(const clap_input_events*e,uint32_t k){return(e&&e->get)?e->get(e,k):nullptr;}
static void sendNote(const clap_output_events*out,Note note,uint32_t offset){if(!out||!out->try_push)return;
 clap_event_note e{};e.header={sizeof(e),offset,0,(uint16_t)(note.type?CLAP_EVENT_NOTE_ON:CLAP_EVENT_NOTE_OFF),0};
 e.note_id=note.noteId;e.port_index=0;e.channel=(int16_t)note.channel;e.key=(int16_t)note.key;e.velocity=note.velocity;out->try_push(out,&e.header);
}
static int32_t opProcess(const clap_plugin*p,const clap_process*data){ClapInstance*i=inst(p);if(!i||!data)return CLAP_PROCESS_CONTINUE;
 uint32_t frames=data->frames_count;uint32_t count=inputCount(data->in_events),idx=0;
 applyUiParams(i); // changes from GUI are applied at block boundary; host events below may be sample accurate.
 if(MODE==0){
  const clap_audio_buffer*in=data->audio_inputs_count&&data->audio_inputs?data->audio_inputs:nullptr;
  clap_audio_buffer*out=data->audio_outputs_count&&data->audio_outputs?data->audio_outputs:nullptr;
  if(!out||out->channel_count<1)return CLAP_PROCESS_CONTINUE;
  for(uint32_t s=0;s<frames;s++){
   while(idx<count){auto* e=inputAt(data->in_events,idx);if(!e){idx++;continue;}if(e->time>s)break;
    if(e->space_id==0&&e->type==CLAP_EVENT_PARAM_VALUE&&e->size>=sizeof(clap_event_param_value)){
      auto*ev=(const clap_event_param_value*)e;applyHostParam(i,ev);i->audio.set((int)ev->param_id,ev->value);
    }idx++;
   }
   double L=0.,R=0.;if(in){if(in->data32){if(in->data32[0])L=in->data32[0][s];if(in->channel_count>1&&in->data32[1])R=in->data32[1][s];else R=L;}
   else if(in->data64){if(in->data64[0])L=in->data64[0][s];if(in->channel_count>1&&in->data64[1])R=in->data64[1][s];else R=L;}}
   Pair result=i->audio.frame({L,R});
   if(out->data32){if(out->data32[0])out->data32[0][s]=(float)result.l;if(out->channel_count>1&&out->data32[1])out->data32[1][s]=(float)result.r;}
   else if(out->data64){if(out->data64[0])out->data64[0][s]=result.l;if(out->channel_count>1&&out->data64[1])out->data64[1][s]=result.r;}
  }out->constant_mask=0;
 }else{
  uint64_t base=i->midi.absolute;
  for(uint32_t eidx=0;eidx<count;eidx++){const clap_event_header*h=inputAt(data->in_events,eidx);if(!h)continue;
   const uint64_t at=base+(h->time<frames?h->time:frames);
   if(h->space_id!=0)continue;
   if(h->type==CLAP_EVENT_PARAM_VALUE&&h->size>=sizeof(clap_event_param_value)){
      auto*ev=(const clap_event_param_value*)h;applyHostParam(i,ev);i->midi.set((int)ev->param_id,ev->value);
   }else if((h->type==CLAP_EVENT_NOTE_ON||h->type==CLAP_EVENT_NOTE_OFF||h->type==CLAP_EVENT_NOTE_CHOKE)&&h->size>=sizeof(clap_event_note)){
     const auto* ev=(const clap_event_note*)h;
     if(h->type==CLAP_EVENT_NOTE_ON){i->midi.noteOn(ev->note_id,ev->channel,ev->key,ev->velocity,at);}
     else{i->midi.noteOff(ev->note_id,ev->channel,ev->key,ev->velocity,at);}
   }else if(h->type==CLAP_EVENT_MIDI&&h->size>=sizeof(clap_event_midi)){
     const auto*ev=(const clap_event_midi*)h;uint8_t status=ev->data[0]&0xf0,ch=ev->data[0]&0x0f;
     if(status==0x90&&ev->data[2]!=0)i->midi.noteOn(-1,ch,ev->data[1],ev->data[2]/127.,at);
     if(status==0x80||(status==0x90&&ev->data[2]==0))i->midi.noteOff(-1,ch,ev->data[1],ev->data[2]/127.,at);
   }
  }
  i->midi.drain(base,base+frames,[&](Note e,uint32_t offset){sendNote(data->out_events,e,offset);});
  i->midi.absolute+=frames;
 }
 return CLAP_PROCESS_CONTINUE;
}
static uint32_t paramCount(const clap_plugin*){return ParamCount;}
static bool paramInfo(const clap_plugin*,uint32_t id,clap_param_info*out){if(!out||id>=ParamCount)return false;zero(out,sizeof(*out));const Param &p=specs[MODE][id];out->id=id;out->flags=CLAP_PARAM_IS_AUTOMATABLE|((p.kind==1||p.kind==3)?CLAP_PARAM_IS_STEPPED:0);
 copy(out->name,256,p.name);char group[128];if(id<63){int stage=id/9;const char* names[]={"01 Calcination","02 Dissolution","03 Separation","04 Conjunction","05 Fermentation","06 Distillation","07 Coagulation"};copy(group,128,names[stage]);}
 else if(id<70)copy(group,128,"Stage Enable");else if(id<77)copy(group,128,"Stage Order");else copy(group,128,"Global");copy(out->module,1024,group);
 out->min_value=p.lo;out->max_value=p.hi;out->default_value=p.def;return true;}
static bool getValue(const clap_plugin*p,clap_id id,double*out){ClapInstance*i=inst(p);if(!i||!out||id>=ParamCount)return false;*out=readVal(i,(int)id);return true;}
static bool valueText(const clap_plugin*,clap_id id,double value,char*b,uint32_t size){if(id>=ParamCount||!b||size<4)return false;numtext(b,size,value);return true;}
static bool textValue(const clap_plugin*,clap_id id,const char*t,double*out){if(id>=ParamCount||!t||!out)return false;*out=clamp(number(t),specs[MODE][id].lo,specs[MODE][id].hi);return true;}
static void flush(const clap_plugin*p,const clap_input_events*inputs,const clap_output_events*out){ClapInstance*i=inst(p);if(!i)return;
 uint32_t n=inputCount(inputs);for(uint32_t k=0;k<n;k++){
  auto*h=inputAt(inputs,k);if(h&&h->space_id==0&&h->type==CLAP_EVENT_PARAM_VALUE&&h->size>=sizeof(clap_event_param_value))applyHostParam(i,(const clap_event_param_value*)h);
 }
 // GUI-origin edits must reach host automation/state. request_flush()
 // schedules this callback on a host-sanctioned thread; never send from audio UI.
 for(int id=0;id<ParamCount;id++)if(__atomic_exchange_n(&i->dirty[id],0u,__ATOMIC_ACQ_REL)){
  clap_event_param_value evt{};evt.header={sizeof(evt),0u,0u,CLAP_EVENT_PARAM_VALUE,0u};
  evt.param_id=id;evt.note_id=-1;evt.port_index=-1;evt.channel=-1;evt.key=-1;evt.value=readVal(i,id);
  if(!out||!out->try_push||!out->try_push(out,&evt.header))__atomic_store_n(&i->dirty[id],1u,__ATOMIC_RELEASE);
 }
}
static const clap_plugin_params PARAMS={paramCount,paramInfo,getValue,valueText,textValue,flush};
static uint32_t audioCount(const clap_plugin*,bool){return MODE==0?1:0;}
static bool audioInfo(const clap_plugin*,uint32_t index,bool input,clap_audio_port_info*out){if(MODE!=0||index||!out)return false;zero(out,sizeof(*out));out->id=0;copy(out->name,256,input?"Stereo Input":"Stereo Output");out->flags=1;out->channel_count=2;out->port_type="stereo";out->in_place_pair=0;return true;}
static const clap_plugin_audio_ports AUDIO_PORTS={audioCount,audioInfo};
static uint32_t noteCount(const clap_plugin*,bool){return MODE==1?1:0;}
static bool noteInfo(const clap_plugin*,uint32_t index,bool input,clap_note_port_info*out){if(MODE!=1||index||!out)return false;zero(out,sizeof(*out));out->id=0;out->supported_dialects=1|2;out->preferred_dialect=1;copy(out->name,256,input?"Notes In":"Transmuted Notes Out");return true;}
static const clap_plugin_note_ports NOTE_PORTS={noteCount,noteInfo};
static constexpr uint32_t STATE_TAG=0x3547494E,STATE_VERSION=5;
struct State {uint32_t tag,version,mode,num;double params[ParamCount];};
static bool saveState(const clap_plugin*p,const clap_ostream*out){if(!out||!out->write)return false;ClapInstance*i=inst(p);if(!i)return false;State state{};state.tag=STATE_TAG;state.version=STATE_VERSION;state.mode=MODE;state.num=ParamCount;
 for(int k=0;k<ParamCount;k++)state.params[k]=readVal(i,k);return out->write(out,&state,sizeof(state))==(int64_t)sizeof(state);}
static bool loadState(const clap_plugin*p,const clap_istream*in){if(!in||!in->read)return false;ClapInstance*i=inst(p);if(!i)return false;State state{};
 if(in->read(in,&state,sizeof(state))!=(int64_t)sizeof(state)||state.tag!=STATE_TAG||state.mode!=(uint32_t)MODE||state.num!=ParamCount)return false;
 for(int k=0;k<ParamCount;k++)setVal(i,k,state.params[k]);return true;}
static const clap_plugin_state STATE_EXT={saveState,loadState};
static uint32_t getLatency(const clap_plugin*p){ClapInstance*i=inst(p);double sr=i?i->audio.sampleRate:48000.;return (uint32_t)(sr*.010+.5);}
static const clap_plugin_latency LATENCY_EXT={getLatency};
// GUI bridge is added through the Windows embedding layer in CP05. Linux builds are headless.
#ifdef _WIN32
#include "WinGui.h"
#endif
static const void* getExtension(const clap_plugin*,const char*id){if(same(id,"clap.params"))return &PARAMS;if(same(id,"clap.state"))return &STATE_EXT;if(same(id,"clap.latency")&&MODE==0)return &LATENCY_EXT;
 if(same(id,"clap.audio-ports"))return &AUDIO_PORTS;if(same(id,"clap.note-ports"))return &NOTE_PORTS;
#ifdef _WIN32
 if(same(id,"clap.gui"))return &NIG_GUI;
#endif
 return nullptr;}
static void onMain(const clap_plugin*){}
static const char* AUDIO_FEATURES[]={"audio-effect","stereo",nullptr};
static const char* MIDI_FEATURES[]={"note-effect",nullptr};
static const clap_plugin_descriptor DESCS[2]={
 {{1,2,0},"com.nigredo.opus-septem.audio","Nigredo · Opus Septem — Audio FX","Nigredo Audio",nullptr,nullptr,nullptr,"0.9.1-cp10","Seven-stage alchemical stereo audio processor",AUDIO_FEATURES},
 {{1,2,0},"com.nigredo.opus-septem.midi","Nigredo · Opus Septem — MIDI Note FX","Nigredo Audio",nullptr,nullptr,nullptr,"0.9.1-cp10","Seven-stage alchemical note transformation",MIDI_FEATURES}}
;
static uint32_t factoryCount(const clap_plugin_factory*){return 1;}
static const clap_plugin_descriptor* factoryDescriptor(const clap_plugin_factory*,uint32_t idx){return idx==0?&DESCS[MODE]:nullptr;}
static const clap_plugin* factoryCreate(const clap_plugin_factory*,const clap_host*host,const char*id){if(!same(id,DESCS[MODE].id)||!host)return nullptr;
 for(auto &slot:PLUGINS)if(!slot.used){ClapInstance*i=&slot;i->used=true;i->active=false;i->host=host;
 i->hostParams=(const clap_host_params*)(host->get_extension?host->get_extension(host,"clap.host.params"):nullptr);
 i->audio.defaults();i->midi.defaults();for(int k=0;k<ParamCount;k++){setVal(i,k,specs[MODE][k].def);i->dirty[k]=0;}
 i->plugin={&DESCS[MODE],i,opInit,opDestroy,opActivate,opDeactivate,opStart,opStop,opReset,opProcess,getExtension,onMain};return &i->plugin;}
 return nullptr;}
static const clap_plugin_factory FACTORY={factoryCount,factoryDescriptor,factoryCreate};
static bool entryInit(const char*){return true;}static void entryDeinit(){}
static const void*entryFactory(const char*id){return same(id,"clap.plugin-factory")?&FACTORY:nullptr;}
DLL_EXPORT const clap_plugin_entry clap_entry={{1,2,0},entryInit,entryDeinit,entryFactory};
#ifdef _WIN32
DLL_EXPORT int __stdcall DllMain(void*module,unsigned long reason,void*){if(reason==1)nigModule=module;return 1;}
#endif
