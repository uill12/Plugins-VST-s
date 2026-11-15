#include "clap_abi_min.h"
#include "rubedo_engine.h"
#include "rubedo_gui.h"
#include "clap_gui_abi.h"
#include "win_imports.h"
#include <stdint.h>
#include <stddef.h>
#ifdef _WIN32
void *rubedo_module=0;
#else
#include <stdlib.h>
#endif
static void rb_memcpy(void *dst,const void*src,uint32_t len){char*d=dst;const char*s=src;for(uint32_t i=0;i<len;i++)d[i]=s[i];}
static void rb_bzero(void*dst,uint32_t len){char*p=dst;for(uint32_t i=0;i<len;i++)p[i]=0;}
static int rb_streq(const char*a,const char*b){if(!a||!b)return 0;for(;*a&&*a==*b;a++,b++);return *a==*b;}
static void rb_strcpy(char*d,const char*s,uint32_t cap){uint32_t i=0;if(!cap)return;for(;i+1<cap&&s[i];i++)d[i]=s[i];d[i]=0;}
static void rb_u32txt(char *out,int num){char tmp[20];int i=0,j=0;if(num<0){out[j++]='-';num=-num;}do{tmp[i++]=(char)('0'+(num%10));num/=10;}while(num&&i<18);while(i)out[j++]=tmp[--i];out[j]=0;}
static void rb_value_txt(char*out,uint32_t size,double x){if(size<12){if(size)out[0]=0;return;}int neg=x<0;if(neg)x=-x;int p=(int)(x*100.+.5);int whole=p/100,frac=p%100;char b[32];rb_u32txt(b,whole);int i=0;if(neg)out[i++]='-';for(int j=0;b[j]&&i<(int)size-4;j++)out[i++]=b[j];out[i++]='.';out[i++]=(char)('0'+frac/10);out[i++]=(char)('0'+frac%10);out[i]=0;}
static double rb_parse(const char*s){if(!s)return 0;double x=0,frac=.1;int sign=1;if(*s=='-'){sign=-1;s++;}while(*s>='0'&&*s<='9'){x=x*10+(*s-'0');s++;}if(*s=='.'){s++;while(*s>='0'&&*s<='9'){x+=(*s-'0')*frac;frac*=.1;s++;}}return sign*x;}
static const char *g_features[]={"instrument","drum",NULL};
static const clap_plugin_descriptor_t g_desc={{1,2,0},"com.rubedo.atlas-drumkit","Rubedo Atlas Drumkit","Rubedo Atlas","","","","0.9.4-cp09","24 synthesized percussion kits with 24 voices, original 96k PCM24 bank",g_features};
typedef struct RubedoClap {clap_plugin_t api;const clap_host_t*host;RubedoEngine engine;uint8_t initialized,processing,gui_created;RBGui gui;} RubedoClap;
static const char *g_params[10]={"KIT / ATLAS","MASTER OUTPUT","TONE","MATERIAL MOTION","DRIVE","WIDTH","ATTACK","DECAY","CELLULAR","SPACE"};
static RubedoClap*context(const clap_plugin_t*p){return (RubedoClap*)p->plugin_data;}
static bool cb_init(const clap_plugin_t *p){RubedoClap*x=context(p);x->initialized=rubedo_load_bank();return x->initialized;}
static void cb_destroy(const clap_plugin_t*p){RubedoClap*x=context(p);if(x->gui.hwnd)rb_gui_detach(&x->gui);if(x->initialized)rubedo_unload_bank();
#ifdef _WIN32
 VirtualFree(x,0,RUBEDO_MEM_RELEASE);
#else
 free(x);
#endif
}
static bool cb_activate(const clap_plugin_t*p,double sr,uint32_t min_frames,uint32_t max_frames){(void)min_frames;(void)max_frames;rubedo_activate(&context(p)->engine,sr);return true;}
static void cb_deactivate(const clap_plugin_t*p){rubedo_panic(&context(p)->engine);}
static bool cb_start(const clap_plugin_t*p){context(p)->processing=1;return true;}
static void cb_stop(const clap_plugin_t*p){context(p)->processing=0;}
static void cb_reset(const clap_plugin_t*p){rubedo_panic(&context(p)->engine);}
static void rb_event(RubedoEngine*e,const clap_event_header_t*h){
 if(!h||h->space_id!=CLAP_CORE_EVENT_SPACE_ID)return;
 if(h->type==CLAP_EVENT_NOTE_ON&&h->size>=sizeof(clap_event_note_t)){const clap_event_note_t*n=(const clap_event_note_t*)h;rubedo_note_on(e,n->key,n->velocity);}
 else if(h->type==CLAP_EVENT_MIDI&&h->size>=sizeof(clap_event_midi_t)){const clap_event_midi_t*m=(const clap_event_midi_t*)h;uint8_t cmd=m->data[0]&0xf0;
 if(cmd==0x90&&m->data[2])rubedo_note_on(e,m->data[1],m->data[2]/127.);else if(cmd==0xc0)rubedo_set_param(e,0,m->data[1]%24);}
 else if(h->type==CLAP_EVENT_PARAM_VALUE&&h->size>=sizeof(clap_event_param_value_t)){const clap_event_param_value_t*v=(const clap_event_param_value_t*)h;rubedo_set_param(e,v->param_id,v->value);}
}
static int32_t cb_process(const clap_plugin_t*p,const clap_process_t*pr){
 RubedoEngine*e=&context(p)->engine;if(!pr)return CLAP_PROCESS_CONTINUE;
 float*lo=0,*ro=0;double*ld=0,*rd=0;
 if(pr->audio_outputs_count){const clap_audio_buffer_t*a=&pr->audio_outputs[0];
 if(a->data32&&a->channel_count>=2){lo=a->data32[0];ro=a->data32[1];}
 if(a->data64&&a->channel_count>=2){ld=a->data64[0];rd=a->data64[1];}}
 uint32_t last=0, n=pr->in_events?pr->in_events->size(pr->in_events):0;
 for(uint32_t j=0;j<n;j++){
  const clap_event_header_t*h=pr->in_events->get(pr->in_events,j);if(!h)continue;
  uint32_t pos=h->time<pr->frames_count?h->time:pr->frames_count;if(pos<last)pos=last;
  if(pos>last)rubedo_render(e,lo?lo+last:0,ro?ro+last:0,ld?ld+last:0,rd?rd+last:0,pos-last);
  rb_event(e,h);last=pos;
 }
 if(last<pr->frames_count)rubedo_render(e,lo?lo+last:0,ro?ro+last:0,ld?ld+last:0,rd?rd+last:0,pr->frames_count-last);
 return CLAP_PROCESS_CONTINUE;
}
static uint32_t audio_count(const clap_plugin_t*p,bool is_input){(void)p;return is_input?0:1;}
static bool audio_get(const clap_plugin_t*p,uint32_t idx,bool input,clap_audio_port_info_t*inf){(void)p;if(input||idx||!inf)return false;rb_bzero(inf,sizeof(*inf));inf->id=0;rb_strcpy(inf->name,"Rubedo Stereo Out",256);inf->flags=CLAP_AUDIO_PORT_IS_MAIN|CLAP_AUDIO_PORT_SUPPORTS_64BITS;inf->channel_count=2;inf->port_type="stereo";inf->in_place_pair=CLAP_INVALID_ID;return true;}
static const clap_plugin_audio_ports_t g_audio={audio_count,audio_get};
static uint32_t note_count(const clap_plugin_t*p,bool input){(void)p;return input?1:0;}
static bool note_get(const clap_plugin_t*p,uint32_t idx,bool input,clap_note_port_info_t*inf){(void)p;if(!input||idx||!inf)return false;rb_bzero(inf,sizeof(*inf));inf->id=0;rb_strcpy(inf->name,"Rubedo 24 Note Inputs",256);inf->supported_dialects=CLAP_NOTE_DIALECT_CLAP|CLAP_NOTE_DIALECT_MIDI;inf->preferred_dialect=CLAP_NOTE_DIALECT_CLAP;return true;}
static const clap_plugin_note_ports_t g_note={note_count,note_get};
static uint32_t par_count(const clap_plugin_t*p){(void)p;return RUBEDO_PARAMS;}
static bool par_info(const clap_plugin_t*p,uint32_t idx,clap_param_info_t*inf){(void)p;if(!inf||idx>=RUBEDO_PARAMS)return false;rb_bzero(inf,sizeof(*inf));inf->id=idx;inf->flags=CLAP_PARAM_IS_AUTOMATABLE;const char*module=idx>=10?"Per-Voice Level":"Rubedo / Global";rb_strcpy(inf->module,module,sizeof(inf->module));
 if(idx<10)rb_strcpy(inf->name,g_params[idx],256);else{char t[50]="VOICE ";rb_u32txt(t+6,idx-9);rb_strcpy(inf->name,t,256);}
 inf->min_value=0;inf->max_value=idx==0?23:1;inf->default_value=idx==0?0:idx>=10?1:idx==1?.65:idx==4?0:.5;if(idx==0)inf->flags|=CLAP_PARAM_IS_STEPPED;return true;}
static bool par_val(const clap_plugin_t*p,clap_id id,double*value){if(!value||id>=RUBEDO_PARAMS)return false;*value=context(p)->engine.params[id];return true;}
static bool par_text(const clap_plugin_t*p,clap_id id,double value,char*buffer,uint32_t size){(void)p;if(id>=RUBEDO_PARAMS||!buffer)return false;if(id==0){int idx=(int)(value+.5);if(idx<0)idx=0;if(idx>23)idx=23;rb_strcpy(buffer,rubedo_kit_names[idx],size);return true;}rb_value_txt(buffer,size,value);return true;}
static bool par_parse(const clap_plugin_t*p,clap_id id,const char*text,double*value){(void)p;if(!text||!value||id>=RUBEDO_PARAMS)return false;if(id==0){for(int i=0;i<24;i++)if(rb_streq(text,rubedo_kit_names[i])){*value=i;return true;}}*value=rb_parse(text);return true;}
static void par_flush(const clap_plugin_t*p,const clap_input_events_t*input,const clap_output_events_t*out){(void)out;if(!input)return;RubedoEngine*e=&context(p)->engine;uint32_t n=input->size(input);for(uint32_t i=0;i<n;i++)rb_event(e,input->get(input,i));}
static const clap_plugin_params_t g_par={par_count,par_info,par_val,par_text,par_parse,par_flush};
typedef struct RubedoSaved {uint32_t magic,version,kit;double params[RUBEDO_PARAMS];int16_t mappings[RUBEDO_PADS];}RubedoSaved;
static bool st_save(const clap_plugin_t*p,const clap_ostream_t*stream){
 if(!stream||!stream->write)return false;
 RubedoEngine*e=&context(p)->engine;RubedoSaved state={0};RubedoExtraState extra;RubedoExtraStateV3 extra3;
 state.magic=0x42445252u;state.version=3;state.kit=e->kit;
 for(int i=0;i<34;i++)state.params[i]=e->params[i];
 for(int i=0;i<24;i++)state.mappings[i]=e->mappings[i];
 rubedo_export_extra(e,&extra);rubedo_export_extra_v3(e,&extra3);
 if(stream->write(stream,&state,sizeof(state))!=(int64_t)sizeof(state))return false;
 if(stream->write(stream,&extra,sizeof(extra))!=(int64_t)sizeof(extra))return false;
 return stream->write(stream,&extra3,sizeof(extra3))==(int64_t)sizeof(extra3);
}
static bool st_load(const clap_plugin_t*p,const clap_istream_t*stream){
 if(!stream||!stream->read)return false;RubedoSaved state;uint8_t*mem=(uint8_t*)&state;uint64_t off=0;
 while(off<sizeof(state)){int64_t n=stream->read(stream,mem+off,sizeof(state)-off);if(n<=0)return false;off+=n;}
 if(state.magic!=0x42445252u||(state.version!=1&&state.version!=2&&state.version!=3))return false;
 RubedoEngine*e=&context(p)->engine;
 for(int i=0;i<34;i++)rubedo_set_param(e,i,state.params[i]);
 for(int i=0;i<24;i++)if(state.mappings[i]>=0&&state.mappings[i]<=127)e->mappings[i]=state.mappings[i];
 if(state.version>=2){RubedoExtraState extra;off=0;mem=(uint8_t*)&extra;
  while(off<sizeof(extra)){int64_t n=stream->read(stream,mem+off,sizeof(extra)-off);if(n<=0)return false;off+=n;}
  rubedo_import_extra(e,&extra);
 }
 if(state.version>=3){RubedoExtraStateV3 extra3;off=0;mem=(uint8_t*)&extra3;
  while(off<sizeof(extra3)){int64_t n=stream->read(stream,mem+off,sizeof(extra3)-off);if(n<=0)return false;off+=n;}
  rubedo_import_extra_v3(e,&extra3);
 }
 return true;
}
static const clap_plugin_state_t g_state={st_save,st_load};
static bool gui_api(const clap_plugin_t*p,const char*api,bool floating){(void)p;return !floating&&rb_streq(api,"win32");}
static bool gui_pref(const clap_plugin_t*p,const char**api,bool*floating){(void)p;if(!api||!floating)return false;*api="win32";*floating=false;return true;}
static bool gui_create(const clap_plugin_t*p,const char*api,bool floating){if(!gui_api(p,api,floating))return false;context(p)->gui_created=1;return true;}
static void gui_destroy(const clap_plugin_t*p){RubedoClap*x=context(p);rb_gui_detach(&x->gui);x->gui_created=0;}
static bool gui_scale(const clap_plugin_t*p,double scale){(void)p;(void)scale;return true;}
static bool gui_size(const clap_plugin_t*p,uint32_t*w,uint32_t*h){(void)p;if(!w||!h)return false;*w=RB_GUI_WIDTH;*h=RB_GUI_HEIGHT;return true;}
static bool gui_canresize(const clap_plugin_t*p){(void)p;return false;}
static bool gui_hints(const clap_plugin_t*p,clap_gui_resize_hints_t*h){(void)p;if(!h)return false;h->can_resize_horizontally=false;h->can_resize_vertically=false;h->preserve_aspect_ratio=true;h->aspect_ratio_width=RB_GUI_WIDTH;h->aspect_ratio_height=RB_GUI_HEIGHT;return true;}
static bool gui_adjust(const clap_plugin_t*p,uint32_t*w,uint32_t*h){return gui_size(p,w,h);}
static bool gui_setsize(const clap_plugin_t*p,uint32_t w,uint32_t h){(void)p;return w==RB_GUI_WIDTH&&h==RB_GUI_HEIGHT;}
static bool gui_parent(const clap_plugin_t*p,const clap_window_t*w){RubedoClap*x=context(p);if(!w||!rb_streq(w->api,"win32")||!x->gui_created)return false;return rb_gui_attach(&x->gui,w->handle.win32,&x->engine,0,0)!=0;}
static bool gui_trans(const clap_plugin_t*p,const clap_window_t*w){(void)p;(void)w;return false;}
static void gui_title(const clap_plugin_t*p,const char*t){(void)p;(void)t;}
static bool gui_show(const clap_plugin_t*p){rb_gui_show(&context(p)->gui,1);return true;}
static bool gui_hide(const clap_plugin_t*p){rb_gui_show(&context(p)->gui,0);return true;}
static const clap_plugin_gui_t g_gui={gui_api,gui_pref,gui_create,gui_destroy,gui_scale,gui_size,gui_canresize,gui_hints,gui_adjust,gui_setsize,gui_parent,gui_trans,gui_title,gui_show,gui_hide};
static const void*cb_ext(const clap_plugin_t*p,const char*id){(void)p;if(rb_streq(id,CLAP_EXT_AUDIO_PORTS))return &g_audio;if(rb_streq(id,CLAP_EXT_NOTE_PORTS))return &g_note;if(rb_streq(id,CLAP_EXT_PARAMS))return &g_par;if(rb_streq(id,CLAP_EXT_STATE))return &g_state;if(rb_streq(id,CLAP_EXT_GUI))return &g_gui;return 0;}
static void cb_main(const clap_plugin_t*p){(void)p;}
static uint32_t factory_count(const clap_plugin_factory_t*f){(void)f;return 1;}
static const clap_plugin_descriptor_t*factory_desc(const clap_plugin_factory_t*f,uint32_t i){(void)f;return i==0?&g_desc:0;}
static const clap_plugin_t*factory_create(const clap_plugin_factory_t*f,const clap_host_t*h,const char*id){(void)f;if(!h||!rb_streq(id,g_desc.id))return 0;
#ifdef _WIN32
 RubedoClap*x=VirtualAlloc(0,sizeof(RubedoClap),RUBEDO_MEM_RESERVE|RUBEDO_MEM_COMMIT,RUBEDO_PAGE_READWRITE);
#else
 RubedoClap*x=calloc(1,sizeof(RubedoClap));
#endif
 if(!x)return 0;rb_bzero(x,sizeof(*x));x->host=h;rubedo_init_engine(&x->engine);
 x->api.desc=&g_desc;x->api.plugin_data=x;x->api.init=cb_init;x->api.destroy=cb_destroy;x->api.activate=cb_activate;x->api.deactivate=cb_deactivate;x->api.start_processing=cb_start;x->api.stop_processing=cb_stop;x->api.reset=cb_reset;x->api.process=cb_process;x->api.get_extension=cb_ext;x->api.on_main_thread=cb_main;return &x->api;
}
static const clap_plugin_factory_t g_factory={factory_count,factory_desc,factory_create};
static bool entry_init(const char*path){(void)path;return true;}
static void entry_deinit(void){}
static const void*entry_factory(const char*name){return rb_streq(name,CLAP_PLUGIN_FACTORY_ID)?&g_factory:0;}
CLAP_EXPORT const clap_plugin_entry_t clap_entry={{1,2,0},entry_init,entry_deinit,entry_factory};
#ifdef _WIN32
int __stdcall DllMain(void* module,unsigned long reason,void*reserved){(void)reserved;if(reason==1)rubedo_module=module;return 1;}
#endif