#include "vst3_min_abi.h"
#include "rubedo_engine.h"
#include "rubedo_gui.h"
#include "win_imports.h"
#include <stdint.h>
#include <stddef.h>
#ifdef _WIN32
void *rubedo_module=0;
#else
#include <stdlib.h>
#endif
static void vr_zero(void *p,uint64_t n){uint8_t*a=p;for(uint64_t i=0;i<n;i++)a[i]=0;}
static void vr_copy(void*p,const void*q,uint64_t n){uint8_t*a=p;const uint8_t*b=q;for(uint64_t i=0;i<n;i++)a[i]=b[i];}
static void vr_str(char*d,const char*s,uint32_t n){uint32_t i=0;for(;s[i]&&i+1<n;i++)d[i]=s[i];d[i]=0;}
static void vr_wstr(Char16*d,const char*s,uint32_t n){uint32_t i=0;for(;s[i]&&i+1<n;i++)d[i]=(unsigned char)s[i];d[i]=0;}
static int vr_streq(const char*a,const char*b){if(!a||!b)return 0;while(*a&&*a==*b){a++;b++;}return *a==*b;}
static int vr_eq16(const uint8_t*a,const uint8_t*b){if(!a||!b)return 0;for(int i=0;i<16;i++)if(a[i]!=b[i])return 0;return 1;}
static int vr_guid(const uint8_t *id,uint32_t a,uint32_t b,uint32_t c,uint32_t d){uint32_t x[4]={a,b,c,d};int big=1,little=1,com=1;for(int k=0;k<16;k++){uint8_t be=(x[k/4]>>(24-8*(k%4)))&255,le=(x[k/4]>>(8*(k%4)))&255;if(id[k]!=be)big=0;if(id[k]!=le)little=0;int order=k%4;if(k<4)order=3-order;else if(k==4)order=1;else if(k==5)order=0;else if(k==6)order=3;else if(k==7)order=2;uint8_t ce=(x[k/4]>>(24-8*order))&255;if(id[k]!=ce)com=0;}return big||little||com;}
static const uint8_t CLS_COMP[16]={0x52,0x42,0x44,0x4b,0x10,0x21,0x49,0x82,0xb9,0x33,0x43,0x22,0x26,0x11,0x14,0x01};
static const uint8_t CLS_CTRL[16]={0x52,0x42,0x44,0x4b,0x10,0x21,0x49,0x82,0xb9,0x33,0x43,0x22,0x26,0x11,0x14,0x02};
#define Q_UNKNOWN(id) vr_guid(id,0,0,0xc0000000,0x00000046)
#define Q_COMPONENT(id) vr_guid(id,0xe831ff31,0xf2d54301,0x928ebbee,0x25697802)
#define Q_PROCESSOR(id) vr_guid(id,0x42043f99,0xb7da453c,0xa569e79d,0x9aaec33d)
#define Q_CONTROLLER(id) vr_guid(id,0xdcd7bbe3,0x7742448d,0xa874aacc,0x979c759e)
#define Q_PLUGINBASE(id) vr_guid(id,0x22888ddb,0x156e45ae,0x8358b348,0x08190625)
#define Q_FACTORY(id) vr_guid(id,0x7a4d811c,0x52114a1f,0xaed9d2ee,0x0b43bf9f)
#define Q_FACTORY2(id) vr_guid(id,0x0007b650,0xf24b4c0b,0xa464edb9,0xf00b2abb)
#define Q_FACTORY3(id) vr_guid(id,0x4555a2ab,0xc1234e57,0x9b122910,0x36878931)

typedef struct VRubedoProcessor {const void**comp;const void**audio;uint32_t refs;uint8_t loaded;RubedoEngine e;}VRubedoProcessor;
#define RUBEDO_VST3_PARAMS 160
#define RUBEDO_VST3_MAP_BEGIN 136
#define RUBEDO_VST3_MAP_END 160
// Extended normalized host parameters bridge the separate Bitwig editor/controller
// process to the audio processor. IDs 0..33 preserve CP03 state compatibility.
typedef struct VRubedoController {const void**ctrl;uint32_t refs;double params[RUBEDO_VST3_PARAMS];void*handler;void*active_view;}VRubedoController;
typedef struct VRubedoFactory {const void**vt;uint32_t refs;}VRubedoFactory;
static const void *compVT[16],*audioVT[12],*ctrlVT[22],*factoryVT[13];
static VRubedoProcessor *g_active_processor=0;

static void*vr_alloc(uint64_t size){
#ifdef _WIN32
 return VirtualAlloc(0,size,RUBEDO_MEM_RESERVE|RUBEDO_MEM_COMMIT,RUBEDO_PAGE_READWRITE);
#else
 return calloc(1,size);
#endif
}
static void vr_free(void*p){
#ifdef _WIN32
 VirtualFree(p,0,RUBEDO_MEM_RELEASE);
#else
 free(p);
#endif
}
static VRubedoProcessor *vr_p(void*iface){VRubedoProcessor*a=iface; if(a->comp==compVT)return a;return (VRubedoProcessor*)((char*)iface-sizeof(void*));}
static int32_t p_query(void*o,const uint8_t*id,void**out){if(!out)return V_FALSE;*out=0;VRubedoProcessor*p=vr_p(o);if(Q_COMPONENT(id)||Q_PLUGINBASE(id)||Q_UNKNOWN(id))*out=&p->comp;else if(Q_PROCESSOR(id))*out=&p->audio;else return V_NOINTERFACE;p->refs++;return V_OK;}
static uint32_t p_add(void*o){return ++vr_p(o)->refs;}
static uint32_t p_rel(void*o){VRubedoProcessor*p=vr_p(o);if(p->refs<=1){if(g_active_processor==p)g_active_processor=0;if(p->loaded)rubedo_unload_bank();vr_free(p);return 0;}return --p->refs;}
static int32_t p_init(void*o,void*context){(void)context;VRubedoProcessor*p=vr_p(o);if(p->loaded)return V_OK;p->loaded=rubedo_load_bank();if(p->loaded)g_active_processor=p;return p->loaded?V_OK:V_FALSE;}
static int32_t p_term(void*o){VRubedoProcessor*p=vr_p(o);if(p->loaded){rubedo_unload_bank();p->loaded=0;}if(g_active_processor==p)g_active_processor=0;return V_OK;}
static int32_t p_controller_id(void*o,uint8_t*cid){(void)o;if(!cid)return V_FALSE;vr_copy(cid,CLS_CTRL,16);return V_OK;}
static int32_t p_io(void*o,int32_t mode){(void)o;(void)mode;return V_OK;}
static int32_t p_buscount(void*o,int32_t type,int32_t direction){(void)o;if(type==0)return direction==1?1:0;if(type==1)return direction==0?1:0;return 0;}
static int32_t p_businfo(void*o,int32_t type,int32_t direction,int32_t idx,VBusInfo*info){(void)o;if(!info||idx!=0)return V_FALSE;vr_zero(info,sizeof(*info));info->mediaType=type;info->dir=direction;info->busType=0;info->flags=1;
 if(type==0&&direction==1){info->channelCount=2;vr_wstr(info->name,"Rubedo Stereo Output",128);return V_OK;}
 if(type==1&&direction==0){info->channelCount=16;vr_wstr(info->name,"Rubedo MIDI Notes",128);return V_OK;}
 return V_FALSE;}
static int32_t p_routing(void*o,VRoutingInfo*in,VRoutingInfo*out){(void)o;(void)in;(void)out;return V_FALSE;}
static int32_t p_actbus(void*o,int32_t type,int32_t dir,int32_t idx,int32_t val){(void)o;(void)type;(void)dir;(void)idx;(void)val;return V_OK;}
static int32_t p_active(void*o,int32_t val){VRubedoProcessor*p=vr_p(o);if(!val)rubedo_panic(&p->e);return V_OK;}
// Stream::read and Stream::write are vtable indices 3 and 4.
static int32_t stream_read(void*stream,void*dst,uint32_t n){if(!stream)return V_FALSE;void**vt=*(void***)stream;int32_t got=0;typedef int32_t(*fn)(void*,void*,int32_t,int32_t*);return ((fn)vt[3])(stream,dst,(int32_t)n,&got)==V_OK&&got==n?V_OK:V_FALSE;}
static int32_t stream_write(void*stream,const void*src,uint32_t n){if(!stream)return V_FALSE;void**vt=*(void***)stream;int32_t got=0;typedef int32_t(*fn)(void*,const void*,int32_t,int32_t*);return ((fn)vt[4])(stream,src,(int32_t)n,&got)==V_OK&&got==n?V_OK:V_FALSE;}
typedef struct VSaved {uint32_t magic,version,kit;double params[34];int16_t mappings[24];}VSaved;
static int32_t p_setstate(void*o,void*stream){
 VSaved s;if(stream_read(stream,&s,sizeof(s))!=V_OK||s.magic!=0x42445252||(s.version!=1&&s.version!=2&&s.version!=3))return V_FALSE;
 RubedoEngine*e=&vr_p(o)->e;
 for(int i=0;i<34;i++)rubedo_set_param(e,i,s.params[i]);
 rubedo_import_mappings(e,s.mappings);
 if(s.kit<24)rubedo_set_param(e,0,(double)s.kit);
 if(s.version>=2){RubedoExtraState extra;if(stream_read(stream,&extra,sizeof(extra))!=V_OK)return V_FALSE;rubedo_import_extra(e,&extra);}
 if(s.version>=3){RubedoExtraStateV3 extra3;if(stream_read(stream,&extra3,sizeof(extra3))!=V_OK)return V_FALSE;rubedo_import_extra_v3(e,&extra3);}
 return V_OK;
}
static int32_t p_getstate(void*o,void*stream){
 RubedoEngine*e=&vr_p(o)->e;VSaved s;RubedoExtraState extra;RubedoExtraStateV3 extra3;
 vr_zero(&s,sizeof(s));s.magic=0x42445252;s.version=3;s.kit=e->kit;
 for(int i=0;i<34;i++)s.params[i]=e->params[i];
 for(int i=0;i<24;i++)s.mappings[i]=(int16_t)atomic_load_explicit(&e->mappings[i],memory_order_acquire);
 rubedo_export_extra(e,&extra);rubedo_export_extra_v3(e,&extra3);
 if(stream_write(stream,&s,sizeof(s))!=V_OK)return V_FALSE;
 if(stream_write(stream,&extra,sizeof(extra))!=V_OK)return V_FALSE;
 return stream_write(stream,&extra3,sizeof(extra3));
}
static int32_t a_buses(void*o,uint64_t*ins,int32_t ni,uint64_t*outs,int32_t no){(void)o;(void)ins;if(ni!=0||no!=1||!outs||outs[0]!=3)return V_FALSE;return V_OK;}
static int32_t a_arrangement(void*o,int32_t dir,int32_t idx,uint64_t*out){(void)o;if(!out||dir!=1||idx!=0)return V_FALSE;*out=3;return V_OK;}
static int32_t a_sample(void*o,int32_t symbol){(void)o;return (symbol==0||symbol==1)?V_OK:V_FALSE;}
static uint32_t a_latency(void*o){(void)o;return 0;}
static int32_t a_setup(void*o,const VProcessSetup*s){if(!s)return V_FALSE;rubedo_activate(&vr_p(o)->e,s->sampleRate);return V_OK;}
static int32_t a_processing(void*o,int32_t val){(void)o;(void)val;return V_OK;}
static void *vr_vt(void*iface){return *(void**)iface;}
static void apply_rubedo_bridge(RubedoEngine*e,uint32_t id,double norm){
 if(id<34){rubedo_set_param(e,id,id==0?norm*23.:norm);return;}
 if(norm<0)norm=0;if(norm>1)norm=1;
 if(id>=34&&id<=57){e->steps[id-34]=(uint16_t)(norm*65535.+.5);return;}
 if(id==58){e->playing=norm>=.5?1:0;e->step_index=0;e->step_clock=0;return;}
 if(id==59){e->bpm=40.+norm*210.;return;}
 if(id>=60&&id<=83){e->pitch[id-60]=(float)norm;return;}
 if(id>=84&&id<=107){e->decay_shape[id-84]=(float)norm;return;}
 if(id>=108&&id<=131){e->fm_depth[id-108]=(float)norm;return;}
 if(id==132){e->swing=(float)norm;return;}
 if(id==133){e->scatter=(float)norm;return;}
 if(id==134){e->chance=(float)norm;return;}
 if(id==135&&norm>0){int encoded=(int)(norm*49.+.5);int pad=(encoded-1)/2;
  if(pad>=0&&pad<24)rubedo_note_on(e,atomic_load_explicit(&e->mappings[pad],memory_order_acquire),.88);}
  if(id>=RUBEDO_VST3_MAP_BEGIN&&id<RUBEDO_VST3_MAP_END){int note=(int)(norm*127.+.5);atomic_store_explicit(&e->mappings[id-RUBEDO_VST3_MAP_BEGIN],note,memory_order_release);return;}
}
static void apply_param_changes(RubedoEngine*e,VParamChanges*changes){if(!changes)return;void**vt=vr_vt(changes);typedef int32_t(*cntfn)(void*);typedef void*(*getfn)(void*,int32_t);int32_t n=((cntfn)vt[3])(changes);if(n<0||n>4096)return;
 for(int32_t i=0;i<n;i++){void*q=((getfn)vt[4])(changes,i);if(!q)continue;void**qvt=vr_vt(q);uint32_t id=((uint32_t(*)(void*))qvt[3])(q);int32_t np=((int32_t(*)(void*))qvt[4])(q);if(np<=0||np>8192||id>=RUBEDO_VST3_PARAMS)continue;int32_t off=0;double norm=0;
 if(((int32_t(*)(void*,int32_t,int32_t*,double*))qvt[5])(q,np-1,&off,&norm)==V_OK)apply_rubedo_bridge(e,id,norm);}
}
static int32_t a_process(void*o,VProcessData*d){if(!d||d->numSamples<0)return V_FALSE;RubedoEngine*e=&vr_p(o)->e;
 apply_param_changes(e,d->inputParameterChanges);
 float *l=0,*r=0;double *ld=0,*rd=0;
 if(d->numOutputs>0&&d->outputs&&d->outputs[0].numChannels>=2){VAudioBusBuffers*b=&d->outputs[0];if(d->symbolicSampleSize==0&&b->channelBuffers32){l=b->channelBuffers32[0];r=b->channelBuffers32[1];}if(d->symbolicSampleSize==1&&b->channelBuffers64){ld=b->channelBuffers64[0];rd=b->channelBuffers64[1];}}
 int32_t total=0;void**evt=0;
 if(d->inputEvents){evt=vr_vt(d->inputEvents);total=((int32_t(*)(void*))evt[3])(d->inputEvents);if(total<0||total>8192)total=0;}
 uint32_t last=0,frames=(uint32_t)d->numSamples;
 for(int32_t j=0;j<total;j++){
  VEvent event;vr_zero(&event,sizeof(event));
  if(((int32_t(*)(void*,int32_t,VEvent*))evt[4])(d->inputEvents,j,&event)!=V_OK)continue;
  uint32_t pos=event.sampleOffset<0?0:(uint32_t)event.sampleOffset;if(pos>frames)pos=frames;if(pos<last)pos=last;
  if(pos>last)rubedo_render(e,l?l+last:0,r?r+last:0,ld?ld+last:0,rd?rd+last:0,pos-last);
  if(event.type==0)rubedo_note_on(e,event.noteOn.pitch,event.noteOn.velocity);
  last=pos;
 }
 if(last<frames)rubedo_render(e,l?l+last:0,r?r+last:0,ld?ld+last:0,rd?rd+last:0,frames-last);
 return V_OK;}
static uint32_t a_tail(void*o){(void)o;return 0;}

static VRubedoController*vr_c(void*o){return (VRubedoController*)o;}
static int32_t c_query(void*o,const uint8_t*id,void**out){if(!out)return V_FALSE;*out=0;if(Q_UNKNOWN(id)||Q_PLUGINBASE(id)||Q_CONTROLLER(id)){*out=o;vr_c(o)->refs++;return V_OK;}return V_NOINTERFACE;}
static uint32_t c_add(void*o){return ++vr_c(o)->refs;}
static uint32_t c_rel(void*o){VRubedoController*c=vr_c(o);if(c->refs<=1){vr_free(c);return 0;}return --c->refs;}
static int32_t c_init(void*o,void*context){(void)o;(void)context;return V_OK;}
static int32_t c_term(void*o){(void)o;return V_OK;}
static int32_t c_compstate(void*o,void*stream){
 VSaved s;if(stream_read(stream,&s,sizeof(s))!=V_OK||s.magic!=0x42445252||(s.version!=1&&s.version!=2&&s.version!=3))return V_FALSE;
 VRubedoController*c=vr_c(o);
 for(int i=0;i<34;i++)c->params[i]=i==0?s.params[i]/23.:s.params[i];
 for(int i=0;i<24;i++)c->params[RUBEDO_VST3_MAP_BEGIN+i]=(s.mappings[i]>=0&&s.mappings[i]<128)?(double)s.mappings[i]/127.:(double)(36+i)/127.;
 if(s.version>=2){RubedoExtraState extra;
  if(stream_read(stream,&extra,sizeof(extra))!=V_OK)return V_FALSE;
  for(unsigned i=0;i<24;i++){
   c->params[34+i]=(double)extra.steps[i]/65535.;
   c->params[60+i]=extra.pitch[i];c->params[84+i]=extra.decay_shape[i];
   c->params[108+i]=extra.fm_depth[i];
  }
  c->params[59]=(extra.bpm-40.)/210.;c->params[132]=extra.swing;c->params[133]=extra.scatter;c->params[134]=extra.chance;
 }
 if(s.version>=3){RubedoExtraStateV3 extra3;if(stream_read(stream,&extra3,sizeof(extra3))!=V_OK)return V_FALSE;}
 return V_OK;
}
static int32_t c_nparams(void*o){(void)o;return RUBEDO_VST3_PARAMS;}
static const char*param_names[]={"KIT / ATLAS","MASTER OUTPUT","TONE","MATERIAL MOTION","DRIVE","WIDTH","ATTACK","DECAY","CELLULAR","SPACE"};
static int32_t c_paraminfo(void*o,int32_t idx,VParameterInfo*inf){
 (void)o;if(!inf||idx<0||idx>=RUBEDO_VST3_PARAMS)return V_FALSE;
 vr_zero(inf,sizeof(*inf));inf->id=(uint32_t)idx;
 char p[64]={0};const char *group="";int voice=0;
 if(idx<10)vr_str(p,param_names[idx],sizeof(p));
 else if(idx<34){group="LEVEL";voice=idx-9;}
 else if(idx<58){group="PATTERN";voice=idx-33;}
 else if(idx==58)vr_str(p,"SEQUENCER PLAY",64);
 else if(idx==59)vr_str(p,"TEMPO BPM",64);
 else if(idx<84){group="PITCH";voice=idx-59;}
 else if(idx<108){group="DECAY";voice=idx-83;}
 else if(idx<132){group="FM DEPTH";voice=idx-107;}
 else if(idx==132)vr_str(p,"SWING",64);
 else if(idx==133)vr_str(p,"SCATTER",64);
 else if(idx==134)vr_str(p,"CHANCE",64);
 else if(idx==135)vr_str(p,"PAD AUDITION",64);
  else if(idx>=RUBEDO_VST3_MAP_BEGIN&&idx<RUBEDO_VST3_MAP_END){group="MIDI MAP";voice=idx-RUBEDO_VST3_MAP_BEGIN+1;}
 if(group[0]){
  int a=0;for(;group[a]&&a<40;a++)p[a]=group[a];p[a++]=' ';
  p[a++]=(char)('0'+voice/10);p[a++]=(char)('0'+voice%10);p[a]=0;
 }
 vr_wstr(inf->title,p,128);
 for(int k=0;k<128;k++)inf->shortTitle[k]=inf->title[k];
 inf->stepCount=idx==0?23:idx>=34&&idx<58?65535:idx==58?1:idx==135?48:idx>=RUBEDO_VST3_MAP_BEGIN?127:0;
 inf->defaultNormalizedValue=idx>=RUBEDO_VST3_MAP_BEGIN?(36.+idx-RUBEDO_VST3_MAP_BEGIN)/127.:idx==0?0:idx>=10&&idx<34?1:idx==58?0:idx==59?(126.-40.)/210.:idx>=34&&idx<58?0:idx==132?.14:idx==133?.18:idx==134?1:idx==135?0:idx==4?0:idx==1?.65:.5;
 inf->unitId=0;inf->flags=1|(idx==0?8:0);return V_OK;
}
static int32_t c_string(void*o,uint32_t id,double norm,Char16*str){
 (void)o;if(!str||id>=RUBEDO_VST3_PARAMS)return V_FALSE;
 if(id==0){int idx=(int)(norm*23.+.5);if(idx>23)idx=23;if(idx<0)idx=0;vr_wstr(str,rubedo_kit_names[idx],128);return V_OK;}
 int v=(int)(norm*100.+.5);char tmp[8];tmp[0]=(char)('0'+(v/100)%10);tmp[1]=(char)('0'+(v/10)%10);tmp[2]=(char)('0'+v%10);tmp[3]='%';tmp[4]=0;vr_wstr(str,tmp,128);return V_OK;
}
static int32_t c_value(void*o,uint32_t id,const Char16*str,double*out){
 (void)o;if(!str||!out||id>=RUBEDO_VST3_PARAMS)return V_FALSE;
 int v=0;for(int i=0;i<7&&str[i]>='0'&&str[i]<='9';i++)v=v*10+str[i]-'0';
 *out=id==0?(v>23?23:v)/23.:(v>100?100:v)/100.;return V_OK;
}
static double c_to_plain(void*o,uint32_t id,double norm){(void)o;return id==0?norm*23.:norm;}
static double c_to_norm(void*o,uint32_t id,double value){(void)o;return id==0?value/23.:value;}
static double c_getnorm(void*o,uint32_t id){return id<RUBEDO_VST3_PARAMS?vr_c(o)->params[id]:0.;}
static void vr_controller_param_changed(VRubedoController*c,uint32_t id,double norm);
static int32_t c_setnorm(void*o,uint32_t id,double norm){
 if(id>=RUBEDO_VST3_PARAMS)return V_FALSE;
 if(norm<0)norm=0;if(norm>1)norm=1;VRubedoController*c=vr_c(o);c->params[id]=norm;vr_controller_param_changed(c,id,norm);return V_OK;
}
static int32_t c_handler(void*o,void*handler){vr_c(o)->handler=handler;return V_OK;}
// Win32 native view displaying approved HTML raster at its exact 1512x1009 geometry.
// The host/processor separation uses a shared pointer only for in-window pad audition.
// For mult-instance safety, use host MIDI / automation rather than GUI pad audition.
typedef struct VRubedoView {const void**vt;uint32_t refs;RBGui gui;VRubedoController*owner;void*frame;RubedoEngine editor_engine;}VRubedoView;
static const void*viewVT[19];
typedef struct VViewRect {int32_t left,top,right,bottom;}VViewRect;
#define Q_VIEW(id) vr_guid(id,0x5bc32507,0xd06049ea,0xa6151b52,0x2b755b29)
static int32_t w_query(void*o,const uint8_t*id,void**out){if(!out)return V_FALSE;*out=0;if(Q_VIEW(id)||Q_UNKNOWN(id)){*out=o;((VRubedoView*)o)->refs++;return V_OK;}return V_NOINTERFACE;}
static uint32_t w_add(void*o){return ++((VRubedoView*)o)->refs;}
static uint32_t w_rel(void*o){VRubedoView*w=o;if(w->refs<=1){rb_gui_detach(&w->gui);vr_free(w);return 0;}return --w->refs;}
static int32_t w_support(void*o,const char*type){(void)o;return vr_streq(type,"HWND")?V_OK:V_FALSE;}
static void w_set_kit(void*o,double norm){VRubedoController*c=o;if(!c)return;c->params[0]=norm;
 if(c->handler){void**vt=*(void***)c->handler;
  ((int32_t(*)(void*,uint32_t))vt[3])(c->handler,0);
  ((int32_t(*)(void*,uint32_t,double))vt[4])(c->handler,0,norm);
  ((int32_t(*)(void*,uint32_t))vt[5])(c->handler,0);
 }}
static void w_set_param(void*o,unsigned id,double norm){
 VRubedoController*c=o;if(!c||id>=RUBEDO_VST3_PARAMS)return;
 if(norm<0)norm=0;if(norm>1)norm=1;
 c->params[id]=norm;
 if(c->handler){void**vt=*(void***)c->handler;
  ((int32_t(*)(void*,uint32_t))vt[3])(c->handler,id);
  ((int32_t(*)(void*,uint32_t,double))vt[4])(c->handler,id,norm);
  ((int32_t(*)(void*,uint32_t))vt[5])(c->handler,id);
 }
}
static int32_t w_attached(void*o,void*parent,const char*type){VRubedoView*w=o;if(!vr_streq(type,"HWND"))return V_FALSE;
 // The VST3 edit controller may be hosted in a separate process from DSP.
 // Never disable GUI event dispatch because there is no global DSP pointer.
 // Never bind the editor's mouse thread to a foreign/real-time audio engine.
 // In Bitwig the controller can be hosted in a different process entirely.
 RubedoEngine*engine=&w->editor_engine;
 rubedo_init_engine(engine);
 for(unsigned i=0;i<34;i++)rubedo_set_param(engine,i,i==0?w->owner->params[i]*23.:w->owner->params[i]);
 for(unsigned i=0;i<24;i++){
  engine->steps[i]=(uint16_t)(w->owner->params[34+i]*65535.+.5);
  engine->pitch[i]=(float)w->owner->params[60+i];
  engine->decay_shape[i]=(float)w->owner->params[84+i];
  engine->fm_depth[i]=(float)w->owner->params[108+i];
 }
 engine->bpm=40.+w->owner->params[59]*210.;
 engine->swing=(float)w->owner->params[132];
 engine->scatter=(float)w->owner->params[133];
 engine->chance=(float)w->owner->params[134];
 w->gui.kit=engine->kit;
 int ok=rb_gui_attach(&w->gui,parent,engine,w->owner,w_set_kit);
 w->gui.param_changed=w_set_param;
 w->gui.editor_only=1;
 return ok?V_OK:V_FALSE;}
static int32_t w_removed(void*o){rb_gui_detach(&((VRubedoView*)o)->gui);return V_OK;}
static int32_t w_wheel(void*o,float wheel){(void)o;(void)wheel;return V_FALSE;}
static int32_t w_key(void*o,Char16 a,int16_t b,int16_t c){(void)o;(void)a;(void)b;(void)c;return V_FALSE;}
static int32_t w_size(void*o,VViewRect*rect){(void)o;if(!rect)return V_FALSE;rect->left=rect->top=0;rect->right=RB_GUI_WIDTH;rect->bottom=RB_GUI_HEIGHT;return V_OK;}
static int32_t w_on_size(void*o,VViewRect*rect){if(!rect)return V_FALSE;rb_gui_size(&((VRubedoView*)o)->gui,rect->right-rect->left,rect->bottom-rect->top);return V_OK;}
static int32_t w_focus(void*o,int state){(void)o;(void)state;return V_OK;}
static int32_t w_frame(void*o,void*frame){((VRubedoView*)o)->frame=frame;return V_OK;}
static int32_t w_canresize(void*o){(void)o;return V_FALSE;}
static int32_t w_constraint(void*o,VViewRect*r){return w_size(o,r);}
static const void *viewVT[19]={w_query,w_add,w_rel,w_support,w_attached,w_removed,w_wheel,w_key,w_key,w_size,w_on_size,w_focus,w_frame,w_canresize,w_constraint};
static void*c_view(void*o,const char*view){if(!vr_streq(view,"editor"))return 0;VRubedoView*w=vr_alloc(sizeof(*w));if(!w)return 0;vr_zero(w,sizeof(*w));w->vt=viewVT;w->refs=1;w->owner=vr_c(o);return w;}


static int32_t f_query(void*o,const uint8_t*id,void**out){if(!out)return V_FALSE;*out=0;if(Q_FACTORY(id)||Q_FACTORY2(id)||Q_FACTORY3(id)||Q_UNKNOWN(id)){*out=o;((VRubedoFactory*)o)->refs++;return V_OK;}return V_NOINTERFACE;}
static uint32_t f_add(void*o){return ++((VRubedoFactory*)o)->refs;}
static uint32_t f_rel(void*o){VRubedoFactory*f=o;if(f->refs>1)--f->refs;return f->refs;}
static int32_t f_info(void*o,VFactoryInfo*inf){(void)o;if(!inf)return V_FALSE;vr_zero(inf,sizeof(*inf));vr_str(inf->vendor,"Rubedo Atlas",sizeof(inf->vendor));inf->flags=16;return V_OK;}
static int32_t f_count(void*o){(void)o;return 2;}
static void get_cid(int idx,uint8_t*o){vr_copy(o,idx==0?CLS_COMP:CLS_CTRL,16);}
static int32_t f_class(void*o,int32_t idx,VClassInfo*inf){(void)o;if(!inf||idx<0||idx>1)return V_FALSE;vr_zero(inf,sizeof(*inf));get_cid(idx,inf->cid);inf->cardinality=0x7fffffff;vr_str(inf->category,idx==0?"Audio Module Class":"Component Controller Class",32);vr_str(inf->name,idx==0?"Rubedo Atlas Drumkit":"Rubedo Atlas Controller",64);return V_OK;}
static int32_t f_class2(void*o,int32_t idx,VClassInfo2*inf){(void)o;if(!inf||idx<0||idx>1)return V_FALSE;vr_zero(inf,sizeof(*inf));get_cid(idx,inf->cid);inf->cardinality=0x7fffffff;vr_str(inf->category,idx==0?"Audio Module Class":"Component Controller Class",32);vr_str(inf->name,idx==0?"Rubedo Atlas Drumkit":"Rubedo Atlas Controller",64);vr_str(inf->subCategories,idx==0?"Instrument|Drum":"",128);vr_str(inf->vendor,"Rubedo Atlas",64);vr_str(inf->version,"0.9.7-cp12r1-vst3",64);vr_str(inf->sdkVersion,"VST 3.7",64);return V_OK;}
static int32_t f_classW(void*o,int32_t idx,VClassInfoW*inf){(void)o;if(!inf||idx<0||idx>1)return V_FALSE;vr_zero(inf,sizeof(*inf));get_cid(idx,inf->cid);inf->cardinality=0x7fffffff;vr_str(inf->category,idx==0?"Audio Module Class":"Component Controller Class",32);vr_wstr(inf->name,idx==0?"Rubedo Atlas Drumkit":"Rubedo Atlas Controller",64);vr_str(inf->subCategories,idx==0?"Instrument|Drum":"",128);vr_wstr(inf->vendor,"Rubedo Atlas",64);vr_wstr(inf->version,"0.9.3-cp08",64);vr_wstr(inf->sdkVersion,"VST 3.7",64);return V_OK;}
static int32_t f_context(void*o,void*context){(void)o;(void)context;return V_OK;}
static int32_t f_instance(void*o,const uint8_t*cid,const uint8_t*id,void**result){(void)o;if(!result)return V_FALSE;*result=0;
 if(vr_eq16(cid,CLS_COMP)){
  VRubedoProcessor*p=vr_alloc(sizeof(*p));if(!p)return V_FALSE;vr_zero(p,sizeof(*p));p->comp=compVT;p->audio=audioVT;p->refs=1;rubedo_init_engine(&p->e);
  if(Q_COMPONENT(id)||Q_UNKNOWN(id)||Q_PLUGINBASE(id))*result=&p->comp;else if(Q_PROCESSOR(id))*result=&p->audio;else {vr_free(p);return V_NOINTERFACE;}return V_OK;
 }
 if(vr_eq16(cid,CLS_CTRL)){
  VRubedoController*c=vr_alloc(sizeof(*c));if(!c)return V_FALSE;vr_zero(c,sizeof(*c));c->ctrl=ctrlVT;c->refs=1;
  for(int i=0;i<34;i++)c->params[i]=(i>=10)?1:(i==1?.65:i==4?0:.5);c->params[0]=0.;
  for(int i=0;i<24;i++){c->params[60+i]=.5;c->params[84+i]=.5;c->params[108+i]=.5;}
  c->params[34]=0x1111/65535.;c->params[36]=0x1010/65535.;
  c->params[38]=0x5555/65535.;c->params[39]=0xaaaa/65535.;
  c->params[59]=(126.-40.)/210.;c->params[132]=.14;c->params[133]=.18;c->params[134]=1.;
  if(Q_CONTROLLER(id)||Q_UNKNOWN(id)||Q_PLUGINBASE(id))*result=c;else {vr_free(c);return V_NOINTERFACE;}return V_OK;
 }return V_NOINTERFACE;
}
static VRubedoFactory g_factory={factoryVT,1};
// C vtable slots implement Steinberg's documented COM-style interfaces.
static const void *factoryVT[13]={f_query,f_add,f_rel,f_info,f_count,f_class,f_instance,f_class2,f_classW,f_context};
static const void *compVT[16]={p_query,p_add,p_rel,p_init,p_term,p_controller_id,p_io,p_buscount,p_businfo,p_routing,p_actbus,p_active,p_setstate,p_getstate};
static const void *audioVT[12]={p_query,p_add,p_rel,a_buses,a_arrangement,a_sample,a_latency,a_setup,a_processing,a_process,a_tail};
static const void *ctrlVT[22]={c_query,c_add,c_rel,c_init,c_term,c_compstate,c_nparams,c_paraminfo,c_string,c_value,c_to_plain,c_to_norm,c_getnorm,c_setnorm,c_handler,c_view};
VEXPORT void *GetPluginFactory(void){g_factory.refs++;return &g_factory;}
VEXPORT int InitDll(void){return 1;}
VEXPORT int ExitDll(void){return 1;}
#ifdef _WIN32
int __stdcall DllMain(void*module,unsigned long reason,void*unused){(void)unused;if(reason==1)rubedo_module=module;return 1;}
#endif