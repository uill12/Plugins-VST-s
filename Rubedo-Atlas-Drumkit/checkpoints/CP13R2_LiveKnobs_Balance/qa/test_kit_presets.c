// Rubedo CP12R3 test: CLAP host notification, state persistence, real distinct PCM render.
// Compile on Linux: clang -std=c11 -O2 -I../source test_kit_presets.c ../source/rubedo_engine.c ../source/rubedo_gui.c -o test_kit_presets
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
// Include plug-in core in this test TU to verify its static GUI callback,
// while using original CLAP ABI types and same source logic as Windows.
#include "../source/rubedo_clap.c"
static int flush_requests,process_requests,main_callbacks,dirty_marks;
static void hp_rescan(const clap_host_t*h,uint32_t flags){(void)h;(void)flags;}
static void hp_clear(const clap_host_t*h,clap_id id,uint32_t flags){(void)h;(void)id;(void)flags;}
static void hp_flush(const clap_host_t*h){(void)h;flush_requests++;}
static rb_clap_host_params_t mockhp={hp_rescan,hp_clear,hp_flush};
static void state_dirty(const clap_host_t*h){(void)h;dirty_marks++;}
static rb_clap_host_state_t mockstate={state_dirty};
static const void* getext(const clap_host_t*h,const char*id){(void)h;
 if(strcmp(id,CLAP_EXT_PARAMS)==0)return &mockhp;
 if(strcmp(id,CLAP_EXT_STATE)==0)return &mockstate;
 return NULL;
}
static void restart(const clap_host_t*h){(void)h;}
static void process(const clap_host_t*h){(void)h;process_requests++;}
static void callback(const clap_host_t*h){(void)h;main_callbacks++;}
static int output_count,output_fail;
static clap_event_param_value_t received[128];
static bool push(const clap_output_events_t*out,const clap_event_header_t*hdr){(void)out;
 if(output_fail)return false;
 assert(hdr->type==CLAP_EVENT_PARAM_VALUE);
 assert(hdr->size==sizeof(clap_event_param_value_t));
 assert(hdr->space_id==CLAP_CORE_EVENT_SPACE_ID);
 assert(hdr->flags&CLAP_EVENT_IS_LIVE);
 assert(output_count<(int)(sizeof(received)/sizeof(received[0])));
 received[output_count++]=*(const clap_event_param_value_t*)hdr;
 return true;
}
static clap_output_events_t outputs={NULL,push};
static clap_event_param_value_t hostevent;
static uint32_t insize(const clap_input_events_t*in){(void)in;return 1;}
static const clap_event_header_t* inget(const clap_input_events_t*in,uint32_t i){(void)in;assert(i==0);return &hostevent.header;}
static clap_input_events_t inputs={NULL,insize,inget};
typedef struct {unsigned char bytes[4096];size_t pos,size;}Mem;
static int64_t memwrite(const clap_ostream_t*s,const void*p,uint64_t sz){Mem*m=s->ctx;if(m->pos+sz>sizeof(m->bytes))return -1;memcpy(m->bytes+m->pos,p,sz);m->pos+=sz;if(m->size<m->pos)m->size=m->pos;return (int64_t)sz;}
static int64_t memread(const clap_istream_t*s,void*p,uint64_t sz){Mem*m=s->ctx;if(m->pos+sz>m->size)return -1;memcpy(p,m->bytes+m->pos,sz);m->pos+=sz;return (int64_t)sz;}
static unsigned u32(const unsigned char*p){return (unsigned)p[0]|(unsigned)p[1]<<8|(unsigned)p[2]<<16|(unsigned)p[3]<<24;}
int main(int argc,char**argv){
 struct clap_host host={{1,2,0},NULL,"Rubedo Test Host","Rubedo","","",getext,restart,process,callback};
 const clap_plugin_factory_t *fac=clap_entry.get_factory(CLAP_PLUGIN_FACTORY_ID);assert(fac&&fac->get_plugin_count(fac)==1);
 const clap_plugin_descriptor_t *desc=fac->get_plugin_descriptor(fac,0);assert(strcmp(desc->version,"0.9.10-cp13r2")==0);
 const clap_plugin_t*p=fac->create_plugin(fac,&host,desc->id);assert(p);
 const clap_plugin_params_t*par=p->get_extension(p,CLAP_EXT_PARAMS);assert(par);
 clap_param_info_t info;assert(par->get_info(p,0,&info));assert(info.max_value==23&&info.flags&CLAP_PARAM_IS_STEPPED);
 RubedoClap*x=context(p);assert(x->engine.kit==0);
 // UI chooses kit 17 and queues parameter edit for Bitwig on main thread.
 rubedo_set_param(&x->engine,0,17);
 rb_gui_kit_changed(x,17.0);
 assert(flush_requests==1&&main_callbacks==1);
 p->on_main_thread(p);assert(dirty_marks==1);
 output_fail=1;
 par->flush(p,NULL,&outputs);assert(output_count==0);
 output_fail=0;
 par->flush(p,NULL,&outputs);assert(output_count==1);
 assert(received[0].param_id==0&&received[0].value==17.0);
 assert(received[0].note_id==-1);
 par->flush(p,NULL,&outputs);assert(output_count==1); // exactly once
 // A knob change must be sent with unnormalized exact parameter ID.
 rb_gui_param_changed(x,2,0.83);
 par->flush(p,NULL,&outputs);assert(output_count==2);
 assert(received[1].param_id==2&&fabs(received[1].value-.83)<1e-6);
 // Bitwig automation may select any of 24 kits independently of GUI.
 memset(&hostevent,0,sizeof(hostevent));hostevent.header.size=sizeof(hostevent);
 hostevent.header.type=CLAP_EVENT_PARAM_VALUE;hostevent.param_id=0;hostevent.value=23;
 par->flush(p,&inputs,&outputs);assert(x->engine.kit==23);
 // CLAP state remains backward compatible with the existing v3 format.
 const clap_plugin_state_t*state=p->get_extension(p,CLAP_EXT_STATE);assert(state);
 Mem mem={0};clap_ostream_t wr={&mem,memwrite};assert(state->save(p,&wr));
 assert(mem.size>1000);
 rubedo_set_param(&x->engine,0,1);mem.pos=0;
 clap_istream_t rd={&mem,memread};assert(state->load(p,&rd));assert(x->engine.kit==23);
 // All actual factory kits must render DIFFERENT waveform samples, not
 // only update labels. Use PCM24 bank embedded in the original archived CLAP.
 if(argc>1){
  int fd=open(argv[1],O_RDONLY);assert(fd>=0);
  struct stat st;assert(!fstat(fd,&st));
  unsigned char*mm=mmap(NULL,(size_t)st.st_size,PROT_READ,MAP_PRIVATE,fd,0);assert(mm!=MAP_FAILED);
  const size_t bankoffset=4802136;assert(bankoffset+373714116ull<=(size_t)st.st_size);
  const unsigned char*bank=mm+bankoffset;
  assert(memcmp(bank,"RBDK24",6)==0);assert(u32(bank+16)==2304);
  rubedo_bank.data=bank;rubedo_bank.bytes=373714116u;
  for(int i=0;i<2304;i++) {rubedo_bank.regions[i].offset=u32(bank+24+i*8);rubedo_bank.regions[i].frames=u32(bank+24+i*8+4);}
  double signatures[24];
  float left[8192],right[8192];
  for(int kit=0;kit<24;kit++){
   rubedo_panic(&x->engine);rubedo_set_param(&x->engine,0,(double)kit);
   rubedo_note_on(&x->engine,36,.8);
   rubedo_render(&x->engine,left,right,NULL,NULL,8192);
   double signature=0,energy=0;
   for(int i=0;i<8192;i++){energy+=left[i]*left[i];signature+=(double)left[i]*(double)((i%127)+1);}
   assert(isfinite(signature)&&energy>1.0e-10);signatures[kit]=signature;
  }
  int diverse=0;
  for(int k=1;k<24;k++)if(fabs(signatures[k]-signatures[0])>1e-3)diverse++;
  assert(diverse==23);
  printf("PASS 24/24 real PCM bank kits audible and distinct\n");
  munmap(mm,(size_t)st.st_size);close(fd);
 }
 p->destroy(p);
 puts("PASS GUI plain-kit feedback, CLAP host flush, failed push retry, Bitwig automation, state v3 restore");
 return 0;
}
