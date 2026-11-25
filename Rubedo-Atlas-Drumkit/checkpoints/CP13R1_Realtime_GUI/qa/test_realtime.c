// CP13R1: end-to-end sample-offset MIDI -> audible PCM; GUI audition wakeup;
// zero reported algorithmic latency; test floats and doubles at target sample rates.
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "../source/rubedo_clap.c"
static int requests, callbacks, dirty;
static void host_restart(const clap_host_t*h){(void)h;}
static void host_process(const clap_host_t*h){(void)h;requests++;}
static void host_callback(const clap_host_t*h){(void)h;callbacks++;}
static void mark_dirty(const clap_host_t*h){(void)h;dirty++;}
static rb_clap_host_state_t st={mark_dirty};
static const void* ext(const clap_host_t*h,const char*e){(void)h;return strcmp(e,CLAP_EXT_STATE)==0?&st:0;}
static clap_host_t host={{1,2,0},0,"CP13R1", "TEST", "", "",ext,host_restart,host_process,host_callback};
static uint32_t endian32(const uint8_t*p){return p[0]|p[1]<<8|p[2]<<16|p[3]<<24;}
static clap_event_note_t note;
static clap_event_midi_t midi;
static uint32_t count_note(const clap_input_events_t*in){(void)in;return 1;}
static const clap_event_header_t* get_note(const clap_input_events_t*in,uint32_t i){(void)in;assert(i==0);return &note.header;}
static const clap_event_header_t* get_midi(const clap_input_events_t*in,uint32_t i){(void)in;assert(i==0);return &midi.header;}
static const clap_input_events_t inputs_note={0,count_note,get_note},inputs_midi={0,count_note,get_midi};
int main(int argc,char**argv){
 // With a path, test the real PCM24 archive. Without it (CI), use a tiny
 // deterministic PCM fixture that exercises the identical CLAP/audio paths.
 int fd=-1;struct stat sb={0};uint8_t*mm=NULL;
 uint8_t*fixture=NULL;
 if(argc>=2){
  fd=open(argv[1],O_RDONLY);assert(fd>=0);assert(fstat(fd,&sb)==0);
  mm=mmap(NULL,sb.st_size,PROT_READ,MAP_PRIVATE,fd,0);assert(mm!=MAP_FAILED);
  const uint8_t*bank=mm+4802136;assert(memcmp(bank,"RBDK24",6)==0);
  rubedo_bank.data=bank;rubedo_bank.bytes=373714116u;
  for(int i=0;i<2304;i++){rubedo_bank.regions[i].offset=endian32(bank+24+i*8);rubedo_bank.regions[i].frames=endian32(bank+24+i*8+4);}
 }else{
  fixture=malloc(3*2048);assert(fixture);
  for(int i=0;i<2048;i++){
   int32_t v=(i%100<50?1:-1)*1200000;
   fixture[i*3+0]=(uint8_t)v;fixture[i*3+1]=(uint8_t)(v>>8);fixture[i*3+2]=(uint8_t)(v>>16);
  }
  rubedo_bank.data=fixture;rubedo_bank.bytes=3*2048;
  for(int i=0;i<2304;i++){rubedo_bank.regions[i].offset=0;rubedo_bank.regions[i].frames=2048;}
 }
 const clap_plugin_factory_t*fac=clap_entry.get_factory(CLAP_PLUGIN_FACTORY_ID);assert(fac);
 const clap_plugin_t*p=fac->create_plugin(fac,&host,"com.rubedo.atlas-drumkit");assert(p);
 RubedoClap*x=context(p);const rb_clap_plugin_latency_t*lat=p->get_extension(p,"clap.latency");assert(lat&&lat->get(p)==0);
 float l[256],r[256];float*fp[2]={l,r};double dl[256],dr[256];double*dp[2]={dl,dr};
 clap_audio_buffer_t buf={fp,NULL,2,0,0};
 clap_process_t pr={0};pr.frames_count=256;pr.audio_outputs=&buf;pr.audio_outputs_count=1;pr.in_events=&inputs_note;
 note=(clap_event_note_t){0};note.header.size=sizeof(note);note.header.time=37;note.header.type=CLAP_EVENT_NOTE_ON;note.note_id=-1;note.port_index=0;note.key=36;note.velocity=.8;
 double rates[]={44100,48000,88200,96000};
 for(int j=0;j<4;j++)for(int k=0;k<24;k++){
  rubedo_init_engine(&x->engine);rubedo_activate(&x->engine,rates[j]);rubedo_set_param(&x->engine,0,k);
  memset(l,0x40,sizeof(l));memset(r,0x40,sizeof(r));pr.in_events=&inputs_note;
  assert(p->process(p,&pr)==CLAP_PROCESS_CONTINUE);
  int first=-1;
  for(int i=0;i<256;i++){
   assert(isfinite(l[i])&&isfinite(r[i]));
   if(i<37)assert(l[i]==0.0f&&r[i]==0.0f);
   else if(first<0&&fabs(l[i])+fabs(r[i])>0.00001)first=i;
  }
  assert(first>=37&&first<37+(int)ceil(rates[j]/1000.));
  if(k==0)printf("PASS CLAP NOTE_ON sr=%6.0f frame=37 first audible frame=%d (~%.2f ms after MIDI)\n",rates[j],first,1000.0*(first-37)/rates[j]);
 }
 rubedo_init_engine(&x->engine);rubedo_activate(&x->engine,48000);rubedo_set_param(&x->engine,0,23);
 midi=(clap_event_midi_t){0};midi.header.size=sizeof(midi);midi.header.time=84;midi.header.type=CLAP_EVENT_MIDI;midi.port_index=0;midi.data[0]=0x90;midi.data[1]=36;midi.data[2]=115;
 pr.in_events=&inputs_midi;assert(p->process(p,&pr)==CLAP_PROCESS_CONTINUE);
 int seen=0;for(int i=0;i<84;i++)assert(l[i]==0.0f);for(int i=84;i<160;i++)if(fabs(l[i])>0.0001)seen=1;assert(seen);
 // GUI-pad audition is queued from the UI thread; request_process is crucial
 // to resume the host when no external MIDI is actively calling process().
 rubedo_init_engine(&x->engine);rubedo_activate(&x->engine,48000);pr.in_events=NULL;
 rubedo_request_audition(&x->engine,0,.88);assert(requests==0);
 rb_gui_audition_requested(x);assert(requests==1);
 memset(l,0,sizeof(l));memset(r,0,sizeof(r));p->process(p,&pr);
 seen=0;for(int i=0;i<256;i++)if(fabs(l[i])>0.0001)seen=1;assert(seen);
 // GUI-only knobs/sequence changes must mark project state as dirty.
 rb_gui_state_changed(x);assert(callbacks==1);p->on_main_thread(p);assert(dirty==1);
 // A separate instance cannot inherit kit state from the previous one.
 const clap_plugin_t*p2=fac->create_plugin(fac,&host,"com.rubedo.atlas-drumkit");assert(p2);
 assert(context(p2)->engine.kit==0);assert(x->engine.kit==0);rubedo_set_param(&context(p2)->engine,0,17);
 assert(x->engine.kit==0&&context(p2)->engine.kit==17);
 // 64-bit double output supported by declared CLAP audio port (no float buffer).
 rubedo_init_engine(&x->engine);rubedo_activate(&x->engine,88200);
 clap_audio_buffer_t buf64={NULL,dp,2,0,0};pr.audio_outputs=&buf64;pr.in_events=&inputs_note;note.header.time=12;
 assert(p->process(p,&pr)==CLAP_PROCESS_CONTINUE);
 seen=0;for(int i=0;i<12;i++)assert(dl[i]==0.0);for(int i=12;i<256;i++)if(fabs(dl[i])>0.0001)seen=1;assert(seen);
 p2->destroy(p2);p->destroy(p);if(mm){munmap(mm,sb.st_size);close(fd);}if(fixture)free(fixture);
 puts("PASS: 24 kits x 4 sample rates: sample-offset MIDI; native MIDI; GUI wake; state dirty; independent instances; double output; zero reported DSP latency");
 return 0;
}
