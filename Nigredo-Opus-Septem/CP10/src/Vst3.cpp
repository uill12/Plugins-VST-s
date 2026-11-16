// Minimal VST3 component/controller factory, separated Audio FX and MIDI Note FX classes.
// ABI model derives from the proven Nereida source project; no binary SDK runtime required.
#include "Engine.h"
#include <stdint.h>
#include <stddef.h>
#ifndef NIG_MODE
#define NIG_MODE 0
#endif
#ifdef _WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT extern "C" __attribute__((visibility("default")))
#endif
using namespace nig;
using i32=int32_t;using u16=uint16_t;using u32=uint32_t;using u64=uint64_t;using tresult=i32;
static constexpr i32 OK=0,FALSE_RESULT=1,NO_INTERFACE=(i32)0x80004002u,INVALID_ARGUMENT=(i32)0x80070057u;
#ifdef _WIN32
#define U4(a,b,c,d) {static_cast<uint8_t>(a),static_cast<uint8_t>((a)>>8),static_cast<uint8_t>((a)>>16),static_cast<uint8_t>((a)>>24),static_cast<uint8_t>((b)>>16),static_cast<uint8_t>((b)>>24),static_cast<uint8_t>(b),static_cast<uint8_t>((b)>>8),static_cast<uint8_t>((c)>>24),static_cast<uint8_t>((c)>>16),static_cast<uint8_t>((c)>>8),static_cast<uint8_t>(c),static_cast<uint8_t>((d)>>24),static_cast<uint8_t>((d)>>16),static_cast<uint8_t>((d)>>8),static_cast<uint8_t>(d)}
#else
#define U4(a,b,c,d) {static_cast<uint8_t>((a)>>24),static_cast<uint8_t>((a)>>16),static_cast<uint8_t>((a)>>8),static_cast<uint8_t>(a),static_cast<uint8_t>((b)>>24),static_cast<uint8_t>((b)>>16),static_cast<uint8_t>((b)>>8),static_cast<uint8_t>(b),static_cast<uint8_t>((c)>>24),static_cast<uint8_t>((c)>>16),static_cast<uint8_t>((c)>>8),static_cast<uint8_t>(c),static_cast<uint8_t>((d)>>24),static_cast<uint8_t>((d)>>16),static_cast<uint8_t>((d)>>8),static_cast<uint8_t>(d)}
#endif
// GUIDs in wire-order matching VST3 TUID and the existing Nereida factory.
static const uint8_t UID_UNKNOWN[16]=U4(0x00000000,0x00000000,0xC0000000,0x00000046);
static const uint8_t UID_FACTORY[16]=U4(0x7A4D811C,0x52114A1F,0xAED9D2EE,0x0B43BF9F);
static const uint8_t UID_FACTORY2[16]=U4(0x0007B650,0xF24B4C0B,0xA464EDB9,0xF00B2ABB);
static const uint8_t UID_FACTORY3[16]=U4(0x4555A2AB,0xC1234E57,0x9B122910,0x36878931);
static const uint8_t UID_COMPONENT[16]=U4(0xE831FF32,0xF2D54301,0x928EBBEE,0x25697802);
static const uint8_t UID_AUDIO[16]=U4(0x42043F99,0xB7DA453C,0xA569E79D,0x9AAEC33D);
static const uint8_t UID_CONTROLLER[16]=U4(0xDCD7BBE4,0x7742448D,0xA874AACC,0x979C759E);
static const uint8_t CLS_PROCESSOR[2][16]={U4(0x4E494752,0x41554449,0x4F465830,0x30353031),U4(0x4E494752,0x4D494449,0x4E4F5430,0x30353031)};
static const uint8_t CLS_CONTROLLER[2][16]={U4(0x4E494752,0x41434544,0x49543030,0x30353031),U4(0x4E494752,0x4D434544,0x49543030,0x30353031)};
static bool eq16(const uint8_t*a,const uint8_t*b){if(!a||!b)return false;for(int k=0;k<16;k++)if(a[k]!=b[k])return false;return true;}
static bool eqstr(const char*a,const char*b){if(!a||!b)return false;while(*a&&*b&&*a==*b){a++;b++;}return !*a&&!*b;}
static void zerobytes(void*p,u32 size){uint8_t*d=(uint8_t*)p;for(u32 i=0;i<size;i++)d[i]=0;}
static void copybytes(void*p,const void*q,u32 n){uint8_t*d=(uint8_t*)p;const uint8_t*s=(const uint8_t*)q;for(u32 i=0;i<n;i++)d[i]=s[i];}
static void copystr(char*d,int cap,const char*s){int n=0;while(s&&s[n]&&n<cap-1){d[n]=s[n];n++;}d[n]=0;}
static void utf16(u16*d,int cap,const char*s){int n=0;while(s&&s[n]&&n<cap-1){d[n]=(uint8_t)s[n];n++;}d[n]=0;}
static int mode=NIG_MODE;
static constexpr uint32_t STAG=0x3547494E,SVERSION=5;
struct State {u32 tag,version,kind,num;double values[ParamCount];};
struct FactoryInfo {char vendor[64],url[256],email[128];i32 flags;};
struct ClassInfo {uint8_t cid[16];i32 cardinality;char category[32],name[64];};
struct ClassInfo2 {uint8_t cid[16];i32 cardinality;char category[32],name[64];u32 classFlags;char subCategories[128],vendor[64],version[64],sdkVersion[64];};
struct ClassInfoW {uint8_t cid[16];i32 cardinality;char category[32];u16 name[64];u32 classFlags;char subCategories[128];u16 vendor[64],version[64],sdkVersion[64];};
struct BusInfo {i32 media,direction,channels;u16 name[128];i32 busType;u32 flags;};
struct ParameterInfo {u32 id;u16 title[128],shortTitle[128],units[128];i32 stepCount;double defaultNormalizedValue;i32 unitId,flags;};
struct ProcessSetup {i32 mode,sampleSize,maxBlock;double sampleRate;};
struct AudioBusBuffers {i32 channels;u64 silenceFlags;union {float** channelBuffers32;double** channelBuffers64;};};
struct Event {i32 busIndex,sampleOffset;double ppqPosition;u16 flags,type;u32 padding;uint8_t payload[64];};
struct ProcessData {i32 processMode,symbolicSampleSize,numSamples,numInputs,numOutputs;AudioBusBuffers* inputs;AudioBusBuffers* outputs;void* inputParameterChanges;void* outputParameterChanges;void* inputEvents;void* outputEvents;void* processContext;};
struct FactoryVTable {tresult(*qi)(void*,const uint8_t*,void**);u32(*add)(void*);u32(*release)(void*);tresult(*getInfo)(void*,FactoryInfo*);i32(*count)(void*);tresult(*classInfo)(void*,i32,ClassInfo*);tresult(*create)(void*,const uint8_t*,const uint8_t*,void**);tresult(*classInfo2)(void*,i32,ClassInfo2*);tresult(*classInfoW)(void*,i32,ClassInfoW*);tresult(*setHostContext)(void*,void*);};
struct ComponentVTable {tresult(*qi)(void*,const uint8_t*,void**);u32(*add)(void*);u32(*release)(void*);tresult(*initialize)(void*,void*);tresult(*terminate)(void*);tresult(*controllerId)(void*,uint8_t*);tresult(*ioMode)(void*,i32);i32(*busCount)(void*,i32,i32);tresult(*busInfo)(void*,i32,i32,i32,BusInfo*);tresult(*routing)(void*,void*,void*);tresult(*activateBus)(void*,i32,i32,i32,i32);tresult(*setActive)(void*,i32);tresult(*setState)(void*,void*);tresult(*getState)(void*,void*);};
struct ProcessorVTable {tresult(*qi)(void*,const uint8_t*,void**);u32(*add)(void*);u32(*release)(void*);tresult(*setArrangements)(void*,u64*,i32,u64*,i32);tresult(*getArrangement)(void*,i32,i32,u64*);tresult(*canSize)(void*,i32);u32(*latency)(void*);tresult(*setup)(void*,ProcessSetup*);tresult(*processing)(void*,i32);tresult(*process)(void*,ProcessData*);u32(*tail)(void*);};
struct ControllerVTable {tresult(*qi)(void*,const uint8_t*,void**);u32(*add)(void*);u32(*release)(void*);tresult(*initialize)(void*,void*);tresult(*terminate)(void*);tresult(*componentState)(void*,void*);tresult(*setState)(void*,void*);tresult(*getState)(void*,void*);i32(*paramCount)(void*);tresult(*paramInfo)(void*,i32,ParameterInfo*);tresult(*stringByValue)(void*,u32,double,u16*);tresult(*valueByString)(void*,u32,const u16*,double*);double(*normToPlain)(void*,u32,double);double(*plainToNorm)(void*,u32,double);double(*getNorm)(void*,u32);tresult(*setNorm)(void*,u32,double);tresult(*handler)(void*,void*);void*(*view)(void*,const char*);};
struct StreamVTable {void* qi;void* add;void* release;tresult(*read)(void*,void*,i32,i32*);tresult(*write)(void*,void*,i32,i32*);tresult(*seek)(void*,int64_t,i32,int64_t*);tresult(*tell)(void*,int64_t*);};
struct ChangesVTable {void* qi;void* add;void* release;i32(*count)(void*);void*(*getData)(void*,i32);void*(*addData)(void*,const u32*,i32*);};
struct QueueVTable {void* qi;void* add;void* release;u32(*id)(void*);i32(*count)(void*);tresult(*get)(void*,i32,i32*,double*);tresult(*append)(void*,i32,double,i32*);};
struct EventsVTable {void* qi;void* add;void* release;i32(*count)(void*);tresult(*get)(void*,i32,Event*);tresult(*append)(void*,Event*);};
struct HostHandlerVTable {void* qi;void* add;void* release;tresult(*begin)(void*,u32);tresult(*perform)(void*,u32,double);tresult(*end)(void*,u32);tresult(*restart)(void*,i32);};
struct Factory {const FactoryVTable* vt;u32 refs;};
struct Queued {void* queue;u32 id;i32 pos,count,offset;double value;};
struct Plugin {const ComponentVTable* component;const ProcessorVTable* processor;u32 refs;bool busy,initialized;double sampleRate;AudioEngine audio;MidiEngine midi;double values[ParamCount];Queued qs[ParamCount];};
struct Controller {const ControllerVTable* vt;u32 refs;bool busy;void*handler;double normalized[ParamCount];};
static Plugin PLUGINS[3]{};static Controller CONTROLLERS[6]{};
static Plugin* processorToPlugin(void*p){return (Plugin*)((uint8_t*)p - offsetof(Plugin,processor));}
// Factory declarations
static tresult f_qi(void*,const uint8_t*,void**),f_info(void*,FactoryInfo*),f_class(void*,i32,ClassInfo*),f_create(void*,const uint8_t*,const uint8_t*,void**),f_class2(void*,i32,ClassInfo2*),f_classW(void*,i32,ClassInfoW*);static u32 f_add(void*),f_release(void*);static i32 f_count(void*){return 2;}static tresult f_context(void*,void*){return OK;}
static const FactoryVTable FACTORY_VT={f_qi,f_add,f_release,f_info,f_count,f_class,f_create,f_class2,f_classW,f_context};static Factory FACTORY={&FACTORY_VT,1};
// Component
static tresult c_qi(void*,const uint8_t*,void**),c_init(void*,void*),c_term(void*),c_id(void*,uint8_t*),c_iomode(void*,i32),c_binfo(void*,i32,i32,i32,BusInfo*),c_routing(void*,void*,void*),c_activate(void*,i32,i32,i32,i32),c_active(void*,i32),c_setstate(void*,void*),c_getstate(void*,void*);static u32 c_add(void*),c_release(void*);static i32 c_bcount(void*,i32,i32);
static const ComponentVTable COMPONENT_VT={c_qi,c_add,c_release,c_init,c_term,c_id,c_iomode,c_bcount,c_binfo,c_routing,c_activate,c_active,c_setstate,c_getstate};
// Processor
static tresult a_qi(void*,const uint8_t*,void**),a_arrange(void*,u64*,i32,u64*,i32),a_getarrange(void*,i32,i32,u64*),a_cansize(void*,i32),a_setup(void*,ProcessSetup*),a_processing(void*,i32),a_process(void*,ProcessData*);static u32 a_add(void*),a_release(void*);static u32 a_latency(void*it){Plugin*p=processorToPlugin(it);return mode?0:(u32)((p->sampleRate>0?p->sampleRate:48000.)*.010+.5);}static u32 a_tail(void*){return 480000;}
static const ProcessorVTable PROCESSOR_VT={a_qi,a_add,a_release,a_arrange,a_getarrange,a_cansize,a_latency,a_setup,a_processing,a_process,a_tail};
// Controller
static tresult e_qi(void*,const uint8_t*,void**),e_init(void*,void*),e_term(void*),e_compstate(void*,void*),e_setstate(void*,void*),e_getstate(void*,void*),e_info(void*,i32,ParameterInfo*),e_string(void*,u32,double,u16*),e_parse(void*,u32,const u16*,double*),e_setnorm(void*,u32,double),e_handler(void*,void*);static u32 e_add(void*),e_release(void*);static i32 e_count(void*){return ParamCount;}static double e_normplain(void*,u32 id,double value){return id<ParamCount?plain(mode,id,value):0.;}static double e_plainnorm(void*,u32 id,double value){return id<ParamCount?norm(mode,id,value):0.;}static double e_getnorm(void*it,u32 id){return id<ParamCount?((Controller*)it)->normalized[id]:0.;}static void* e_view(void*,const char*);
static const ControllerVTable CONTROLLER_VT={e_qi,e_add,e_release,e_init,e_term,e_compstate,e_setstate,e_getstate,e_count,e_info,e_string,e_parse,e_normplain,e_plainnorm,e_getnorm,e_setnorm,e_handler,e_view};
static tresult f_qi(void*,const uint8_t*id,void**out){if(!out)return INVALID_ARGUMENT;*out=nullptr;if(eq16(id,UID_UNKNOWN)||eq16(id,UID_FACTORY)||eq16(id,UID_FACTORY2)||eq16(id,UID_FACTORY3)){*out=&FACTORY;f_add(&FACTORY);return OK;}return NO_INTERFACE;}
static u32 f_add(void*){return ++FACTORY.refs;}static u32 f_release(void*){if(FACTORY.refs>1)--FACTORY.refs;return FACTORY.refs;}
static tresult f_info(void*,FactoryInfo*out){if(!out)return INVALID_ARGUMENT;zerobytes(out,sizeof(*out));copystr(out->vendor,64,"Nigredo Audio");out->flags=1<<4;return OK;}
static tresult f_class(void*,i32 index,ClassInfo*out){if(!out||index<0||index>=2)return INVALID_ARGUMENT;zerobytes(out,sizeof(*out));copybytes(out->cid,index?CLS_CONTROLLER[mode]:CLS_PROCESSOR[mode],16);out->cardinality=0x7fffffff;copystr(out->category,32,index?"Component Controller Class":"Audio Module Class");copystr(out->name,64,index?"Nigredo Opus Septem Editor":(mode?"Nigredo Opus Septem MIDI FX":"Nigredo Opus Septem Audio FX"));return OK;}
static tresult f_class2(void*,i32 index,ClassInfo2*out){if(!out)return INVALID_ARGUMENT;ClassInfo c{};if(f_class(nullptr,index,&c)!=OK)return INVALID_ARGUMENT;zerobytes(out,sizeof(*out));copybytes(out->cid,c.cid,16);out->cardinality=c.cardinality;copystr(out->category,32,c.category);copystr(out->name,64,c.name);copystr(out->subCategories,128,index?"":(mode?"Fx|Instrument":"Fx|Spatial"));copystr(out->vendor,64,"Nigredo Audio");copystr(out->version,64,"0.9.1-cp10");copystr(out->sdkVersion,64,"VST 3");return OK;}
static tresult f_classW(void*,i32 index,ClassInfoW*out){if(!out)return INVALID_ARGUMENT;ClassInfo2 x{};if(f_class2(nullptr,index,&x)!=OK)return INVALID_ARGUMENT;zerobytes(out,sizeof(*out));copybytes(out->cid,x.cid,16);out->cardinality=x.cardinality;copystr(out->category,32,x.category);utf16(out->name,64,x.name);copystr(out->subCategories,128,x.subCategories);utf16(out->vendor,64,x.vendor);utf16(out->version,64,x.version);utf16(out->sdkVersion,64,x.sdkVersion);return OK;}
static tresult f_create(void*,const uint8_t*cid,const uint8_t*iid,void**out){if(!out||!cid||!iid)return INVALID_ARGUMENT;*out=nullptr;
 if(eq16(cid,CLS_PROCESSOR[mode])){for(auto &p:PLUGINS)if(!p.busy){p.busy=true;p.refs=1;p.initialized=false;p.component=&COMPONENT_VT;p.processor=&PROCESSOR_VT;
 p.audio.defaults();p.midi.defaults();for(int i=0;i<ParamCount;i++)p.values[i]=specs[mode][i].def;
 if(eq16(iid,UID_COMPONENT)||eq16(iid,UID_UNKNOWN)){*out=&p;return OK;}if(eq16(iid,UID_AUDIO)){*out=&p.processor;return OK;}p.busy=false;return NO_INTERFACE;}return FALSE_RESULT;}
 if(eq16(cid,CLS_CONTROLLER[mode])){for(auto &c:CONTROLLERS)if(!c.busy){c.busy=true;c.refs=1;c.vt=&CONTROLLER_VT;c.handler=nullptr;for(int i=0;i<ParamCount;i++)c.normalized[i]=norm(mode,i,specs[mode][i].def);
 if(eq16(iid,UID_CONTROLLER)||eq16(iid,UID_UNKNOWN)){*out=&c;return OK;}c.busy=false;return NO_INTERFACE;}return FALSE_RESULT;}
 return NO_INTERFACE;}
static tresult c_qi(void*it,const uint8_t*id,void**out){if(!out)return INVALID_ARGUMENT;*out=nullptr;Plugin*p=(Plugin*)it;if(eq16(id,UID_COMPONENT)||eq16(id,UID_UNKNOWN)){*out=it;c_add(it);return OK;}if(eq16(id,UID_AUDIO)){*out=&p->processor;c_add(it);return OK;}return NO_INTERFACE;}
static u32 c_add(void*it){return ++((Plugin*)it)->refs;}static u32 c_release(void*it){Plugin*p=(Plugin*)it;if(p->refs>0)--p->refs;if(!p->refs)p->busy=false;return p->refs;}
static tresult c_init(void*it,void*){((Plugin*)it)->initialized=true;return OK;}static tresult c_term(void*it){((Plugin*)it)->initialized=false;return OK;}
static tresult c_id(void*,uint8_t*out){if(!out)return INVALID_ARGUMENT;copybytes(out,CLS_CONTROLLER[mode],16);return OK;}
static tresult c_iomode(void*,i32){return OK;}
static i32 c_bcount(void*,i32 media,i32 direction){if(!mode&&media==0)return 1;if(mode&&media==1&&(direction==0||direction==1))return 1;return 0;}
static tresult c_binfo(void*,i32 media,i32 direction,i32 index,BusInfo*out){if(!out||index!=0||c_bcount(nullptr,media,direction)==0)return INVALID_ARGUMENT;
 zerobytes(out,sizeof(*out));out->media=media;out->direction=direction;out->channels=media==0?2:16;out->flags=1;out->busType=0;
 utf16(out->name,128,media==0?(direction?"Stereo Output":"Stereo Input"):(direction?"Transmuted Notes Out":"Notes In"));return OK;}
static tresult c_routing(void*,void*,void*){return FALSE_RESULT;}static tresult c_activate(void*,i32,i32,i32,i32){return OK;}static tresult c_active(void*,i32){return OK;}
static bool streamRead(void*stream,State &s){if(!stream)return false;auto*v=*(StreamVTable**)stream;i32 n=0;if(v->read(stream,&s,sizeof(s),&n)!=OK)return false;return n==sizeof(s)&&s.tag==STAG&&s.version==SVERSION&&s.kind==(u32)mode&&s.num==ParamCount;}
static bool streamWrite(void*stream,State &s){if(!stream)return false;auto*v=*(StreamVTable**)stream;i32 n=0;return v->write(stream,&s,sizeof(s),&n)==OK&&n==sizeof(s);}
static tresult c_setstate(void*it,void*stream){State s{};if(!streamRead(stream,s))return FALSE_RESULT;Plugin*p=(Plugin*)it;for(int i=0;i<ParamCount;i++){p->values[i]=clamp(s.values[i],specs[mode][i].lo,specs[mode][i].hi);}return OK;}
static tresult c_getstate(void*it,void*stream){Plugin*p=(Plugin*)it;State s{};s.tag=STAG;s.version=SVERSION;s.kind=mode;s.num=ParamCount;for(int i=0;i<ParamCount;i++)s.values[i]=p->values[i];return streamWrite(stream,s)?OK:FALSE_RESULT;}
static tresult a_qi(void*it,const uint8_t*id,void**out){return c_qi(processorToPlugin(it),id,out);}
static u32 a_add(void*it){return c_add(processorToPlugin(it));}static u32 a_release(void*it){return c_release(processorToPlugin(it));}
static tresult a_arrange(void*,u64*in,i32 inc,u64*out,i32 outc){if(mode)return inc==0&&outc==0?OK:FALSE_RESULT;return inc==1&&outc==1&&in&&out&&in[0]==3u&&out[0]==3u?OK:FALSE_RESULT;}
static tresult a_getarrange(void*,i32 direction,i32 index,u64*out){if(mode||!out||index)return INVALID_ARGUMENT;*out=3;return OK;}
static tresult a_cansize(void*,i32 size){return (size==0||size==1)?OK:FALSE_RESULT;}
static tresult a_setup(void*it,ProcessSetup*s){if(!s||s->sampleRate<22050||s->sampleRate>192000.||s->maxBlock<1||s->maxBlock>1048576)return INVALID_ARGUMENT;Plugin*p=processorToPlugin(it);p->sampleRate=s->sampleRate;p->audio.prepare(s->sampleRate);p->midi.prepare(s->sampleRate);return OK;}
static tresult a_processing(void*,i32){return OK;}
static void fetch(Queued*q){auto*v=*(QueueVTable**)q->queue;i32 pos=0;double val=0;if(q->pos<q->count&&v->get(q->queue,q->pos,&pos,&val)==OK){q->offset=pos;q->value=val;q->pos++;}else q->offset=0x7FFFFFFF;}
static void applyQueues(Plugin*p,ProcessData*data,int&count){count=0;if(!data->inputParameterChanges)return;auto*changes=data->inputParameterChanges;auto*cv=*(ChangesVTable**)changes;
 int n=cv->count(changes);for(int k=0;k<n&&count<ParamCount;k++){
  void*q=cv->getData(changes,k);if(!q)continue;auto*vt=*(QueueVTable**)q;u32 id=vt->id(q);if(id>=ParamCount)continue;
  p->qs[count]={q,id,0,vt->count(q),0,0.};fetch(&p->qs[count]);count++;
 }}
static void applyQueuePoint(Plugin*p,Queued&q){p->values[q.id]=plain(mode,q.id,q.value);if(mode==0)p->audio.set(q.id,p->values[q.id]);else p->midi.set(q.id,p->values[q.id]);fetch(&q);}
static tresult a_process(void*it,ProcessData*data){if(!data||data->numSamples<0)return INVALID_ARGUMENT;Plugin*p=processorToPlugin(it);int nq=0;applyQueues(p,data,nq);
 for(int id=0;id<ParamCount;id++)if(mode==0)p->audio.set(id,p->values[id]);else p->midi.set(id,p->values[id]);
 if(mode==0){AudioBusBuffers*in=data->numInputs&&data->inputs?&data->inputs[0]:nullptr;AudioBusBuffers*out=data->numOutputs&&data->outputs?&data->outputs[0]:nullptr;
  if(!out||out->channels<2)return OK;
  for(int s=0;s<data->numSamples;s++){
   for(int i=0;i<nq;i++)while(p->qs[i].offset<=s)applyQueuePoint(p,p->qs[i]);
   Pair source{0.,0.};if(in&&in->channels>=2){if(data->symbolicSampleSize==0&&in->channelBuffers32){source={in->channelBuffers32[0]?in->channelBuffers32[0][s]:0.,in->channelBuffers32[1]?in->channelBuffers32[1][s]:0.};}
    else if(data->symbolicSampleSize==1&&in->channelBuffers64){source={in->channelBuffers64[0]?in->channelBuffers64[0][s]:0.,in->channelBuffers64[1]?in->channelBuffers64[1][s]:0.};}}
   Pair result=p->audio.frame(source);if(data->symbolicSampleSize==0&&out->channelBuffers32){if(out->channelBuffers32[0])out->channelBuffers32[0][s]=(float)result.l;if(out->channelBuffers32[1])out->channelBuffers32[1][s]=(float)result.r;}
   else if(data->symbolicSampleSize==1&&out->channelBuffers64){if(out->channelBuffers64[0])out->channelBuffers64[0][s]=result.l;if(out->channelBuffers64[1])out->channelBuffers64[1][s]=result.r;}
  }out->silenceFlags=0;
 }else{
  uint64_t base=p->midi.absolute;
  auto*in=data->inputEvents;auto*ivt=in?*(EventsVTable**)in:nullptr;int count=ivt?ivt->count(in):0;
  for(int k=0;k<count;k++){Event ev{};if(ivt->get(in,k,&ev)!=OK)continue;int offset=ev.sampleOffset;
    if(offset<0)offset=0;if(offset>data->numSamples)offset=data->numSamples;
    for(int q=0;q<nq;q++)while(p->qs[q].offset<=offset)applyQueuePoint(p,p->qs[q]);
    if(ev.type==0){int16_t ch=*(int16_t*)(ev.payload+0),pitch=*(int16_t*)(ev.payload+2);
     float velocity=*(float*)(ev.payload+8);i32 id=*(i32*)(ev.payload+16);
     p->midi.noteOn(id,ch,pitch,velocity,base+offset);
    }else if(ev.type==1){int16_t ch=*(int16_t*)(ev.payload+0),pitch=*(int16_t*)(ev.payload+2);float velocity=*(float*)(ev.payload+4);i32 id=*(i32*)(ev.payload+8);
     p->midi.noteOff(id,ch,pitch,velocity,base+offset);
    }
  }
  auto*out=data->outputEvents;auto*ovt=out?*(EventsVTable**)out:nullptr;
  p->midi.drain(base,base+data->numSamples,[&](Note ev,u32 offset){if(!out||!ovt)return;Event e{};e.busIndex=0;e.sampleOffset=(int)offset;e.type=ev.type?0:1;
   int16_t ch=ev.channel,key=ev.key;float velocity=(float)ev.velocity;i32 id=ev.noteId;
   copybytes(e.payload+0,&ch,2);copybytes(e.payload+2,&key,2);
   if(ev.type){copybytes(e.payload+8,&velocity,4);copybytes(e.payload+16,&id,4);}
   else {copybytes(e.payload+4,&velocity,4);copybytes(e.payload+8,&id,4);}
   ovt->append(out,&e);
  });p->midi.absolute+=data->numSamples;
 }
 return OK;
}
static tresult e_qi(void*it,const uint8_t*id,void**out){if(!out)return INVALID_ARGUMENT;*out=nullptr;if(eq16(id,UID_UNKNOWN)||eq16(id,UID_CONTROLLER)){*out=it;e_add(it);return OK;}return NO_INTERFACE;}
static u32 e_add(void*it){return ++((Controller*)it)->refs;}static u32 e_release(void*it){auto*c=(Controller*)it;if(c->refs>0)--c->refs;if(c->refs==0)c->busy=false;return c->refs;}
static tresult e_init(void*,void*){return OK;}static tresult e_term(void*){return OK;}
static tresult e_compstate(void*it,void*stream){State s{};if(!streamRead(stream,s))return FALSE_RESULT;Controller*c=(Controller*)it;for(int i=0;i<ParamCount;i++)c->normalized[i]=norm(mode,i,s.values[i]);return OK;}
static tresult e_setstate(void*it,void*stream){return e_compstate(it,stream);}
static tresult e_getstate(void*it,void*stream){Controller*c=(Controller*)it;State s{};s.tag=STAG;s.version=SVERSION;s.kind=mode;s.num=ParamCount;for(int i=0;i<ParamCount;i++)s.values[i]=plain(mode,i,c->normalized[i]);return streamWrite(stream,s)?OK:FALSE_RESULT;}
static tresult e_info(void*,i32 index,ParameterInfo*out){if(!out||index<0||index>=ParamCount)return INVALID_ARGUMENT;zerobytes(out,sizeof(*out));auto&s=specs[mode][index];out->id=index;utf16(out->title,128,s.name);utf16(out->shortTitle,128,s.name);utf16(out->units,128,s.unit);out->stepCount=(s.kind==1||s.kind==3)?(i32)((s.hi-s.lo)/s.step):0;out->defaultNormalizedValue=norm(mode,index,s.def);out->unitId=-1;out->flags=1;return OK;}
static tresult e_string(void*,u32 id,double normalized,u16*out){if(id>=ParamCount||!out)return INVALID_ARGUMENT;double v=plain(mode,id,normalized);bool neg=v<0;if(neg)v=-v;int n=(int)(v*100.+.5),whole=n/100,frac=n%100;char tmp[32];int idx=0;if(neg)tmp[idx++]='-';char digits[16];int count=0;do{digits[count++]=char('0'+whole%10);whole/=10;}while(whole&&count<15);while(count)tmp[idx++]=digits[--count];tmp[idx++]='.';tmp[idx++]=char('0'+frac/10);tmp[idx++]=char('0'+frac%10);tmp[idx]=0;utf16(out,128,tmp);return OK;}
static tresult e_parse(void*,u32 id,const u16*s,double*out){if(id>=ParamCount||!s||!out)return INVALID_ARGUMENT;double n=0,frac=.1;bool minus=*s=='-';if(minus||*s=='+')s++;while(*s>='0'&&*s<='9'){n=n*10+*s-'0';s++;}if(*s=='.'){s++;while(*s>='0'&&*s<='9'){n+=(*s-'0')*frac;frac*=.1;s++;}}*out=norm(mode,id,minus?-n:n);return OK;}
static tresult e_setnorm(void*it,u32 id,double n){if(id>=ParamCount)return INVALID_ARGUMENT;((Controller*)it)->normalized[id]=clamp(n,0.,1.);return OK;}
static tresult e_handler(void*it,void*h){((Controller*)it)->handler=h;return OK;}
#ifdef _WIN32
#define NIG_VST_GUI 1
#include "WinGui.h"
#else
static void*e_view(void*,const char*){return nullptr;}
#endif
DLL_EXPORT void* GetPluginFactory(){return &FACTORY;}
DLL_EXPORT bool InitDll(){return true;}
DLL_EXPORT bool ExitDll(){return true;}
#ifdef _WIN32
DLL_EXPORT int __stdcall DllMain(void*module,unsigned long reason,void*){if(reason==1)nigModule=module;return 1;}
#endif
