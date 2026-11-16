/* CP12R1: VST3 wrapper vs exact canonical C DSP output comparison.
 *
 * Build with actual VST3 wrapper and CP12 engine objects. A deterministic
 * synthetic PCM24 bank stands in for the private 2304-hit resource overlay.
 * Compare rendered 32-bit PCM bit for bit, including MIDI sample offsets,
 * simultaneous pad triggers, held tails and host control changes.
 *
 * This is a Windows source-only DSP integration test. It does NOT claim the
 * released CLAP binary, real sound library, VST3 host ABI or Bitwig was tested.
 */
#include "vst3_min_abi.h"
#include "rubedo_engine.h"
#include <stdint.h>
#include <stddef.h>
__declspec(dllimport) __attribute__((noreturn)) void __attribute__((ms_abi)) ExitProcess(unsigned);
extern void*GetPluginFactory(void);
typedef struct TEvents{const void**vt;int32_t n;VEvent events[32];}TEvents;
typedef struct TPoint{int32_t offset;double norm;}TPoint;
typedef struct TQueue{const void**vt;uint32_t id;int32_t n;TPoint point[4];}TQueue;
typedef struct TChanges{const void**vt;int32_t n;TQueue q[24];}TChanges;
static const uint8_t CLS_COMP[16]={0x52,0x42,0x44,0x4b,0x10,0x21,0x49,0x82,0xb9,0x33,0x43,0x22,0x26,0x11,0x14,0x01};
static const uint8_t IID_UNKNOWN[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
static const uint8_t IID_PROCESSOR[16]={0x42,0x04,0x3f,0x99,0xb7,0xda,0x45,0x3c,0xa5,0x69,0xe7,0x9d,0x9a,0xae,0xc3,0x3d};
static int32_t ev_count(void*p){return ((TEvents*)p)->n;}
static int32_t ev_get(void*p,int32_t idx,VEvent*out){
 TEvents*t=(TEvents*)p;if(!out||idx<0||idx>=t->n)return 1;*out=t->events[idx];return 0;
}
static const void* EVT_VT[5]={0,0,0,(void*)ev_count,(void*)ev_get};
static int32_t ch_count(void*p){return ((TChanges*)p)->n;}
static void*ch_get(void*p,int32_t idx){
 TChanges*c=(TChanges*)p;return idx>=0&&idx<c->n?&c->q[idx]:0;
}
static uint32_t q_id(void*p){return ((TQueue*)p)->id;}
static int32_t q_count(void*p){return ((TQueue*)p)->n;}
static int32_t q_get(void*p,int32_t idx,int32_t*offset,double*norm){
 TQueue*q=(TQueue*)p;if(idx<0||idx>=q->n)return 1;
 *offset=q->point[idx].offset;*norm=q->point[idx].norm;return 0;
}
static const void* QUEUE_VT[7]={0,0,0,(void*)q_id,(void*)q_count,(void*)q_get,0};
static const void* CHANGES_VT[6]={0,0,0,(void*)ch_count,(void*)ch_get,0};
static uint8_t fake_pcm[4096u*3u];
static float vl[256],vr[256],rl[256],rr[256];
static float*channels[2]={vl,vr};
static VAudioBusBuffers output;
static TEvents events;
static TChanges changes;
static RubedoEngine ref_engine;
static int errors;
static uint32_t crc(float x){union {float f;uint32_t u;}k;k.f=x;return k.u;}
static void clear_block(void){
 for(int i=0;i<256;i++)vl[i]=vr[i]=rl[i]=rr[i]=0;
 events.vt=EVT_VT;events.n=0;changes.vt=CHANGES_VT;changes.n=0;
}
static void add_midi(int offset,int pitch,float velocity){
 VEvent*e=&events.events[events.n++];e->type=0;e->sampleOffset=offset;
 e->noteOn.pitch=(int16_t)pitch;e->noteOn.velocity=velocity;
}
static void add_param(uint32_t id,double norm){
 TQueue*q=&changes.q[changes.n++];q->vt=QUEUE_VT;q->id=id;q->n=1;q->point[0].offset=0;q->point[0].norm=norm;
}
static int run_block(void*processor,int32_t frames){
 output.numChannels=2;output.channelBuffers32=channels;output.silenceFlags=0;
 VProcessData d={0};d.symbolicSampleSize=0;d.numSamples=frames;d.numOutputs=1;d.outputs=&output;
 d.inputEvents=(VEventList*)&events;d.inputParameterChanges=(VParamChanges*)&changes;
 const void**vt=*(const void***)processor;
 int32_t ok=((int32_t(*)(void*,VProcessData*))vt[9])(processor,&d);
 if(ok!=0){errors++;return 0;}
 int previous=0;
 for(int i=0;i<events.n;i++){
  VEvent*e=&events.events[i];int pos=e->sampleOffset;
  if(pos<previous||pos>frames){errors++;return 0;}
  rubedo_render(&ref_engine,rl+previous,rr+previous,0,0,(uint32_t)(pos-previous));
  rubedo_note_on(&ref_engine,e->noteOn.pitch,e->noteOn.velocity);
  previous=pos;
 }
 rubedo_render(&ref_engine,rl+previous,rr+previous,0,0,(uint32_t)(frames-previous));
 int nonzero=0;
 for(int i=0;i<frames;i++){
  if(crc(vl[i])!=crc(rl[i])||crc(vr[i])!=crc(rr[i]))errors++;
  if(vl[i]!=0||vr[i]!=0)nonzero++;
 }
 return nonzero;
}
static int run(void){
 for(int i=0;i<4096;i++){
  /* Signed PCM24 triangle: deterministic full band, nonzero transients. */
  int32_t value=((i*53)&2047)-1024;value*=6000;
  fake_pcm[i*3]=(uint8_t)value;
  fake_pcm[i*3+1]=(uint8_t)(value>>8);
  fake_pcm[i*3+2]=(uint8_t)(value>>16);
 }
 rubedo_bank.data=fake_pcm;rubedo_bank.bytes=sizeof(fake_pcm);
 for(int i=0;i<RUBEDO_KITS*RUBEDO_PADS*RUBEDO_VARS;i++){
  rubedo_bank.regions[i].offset=0;rubedo_bank.regions[i].frames=4096;
 }
 const void**fvt;
 void*factory=GetPluginFactory();if(!factory)return 201;fvt=*(const void***)factory;
 const double sample_rates[]={44100.,48000.,88200.,96000.};
 for(unsigned rate=0;rate<4;rate++){
  void*component=0,*processor=0;
  if(((int32_t(*)(void*,const uint8_t*,const uint8_t*,void**))fvt[6])(factory,CLS_COMP,IID_UNKNOWN,&component)!=0||!component)return 202;
  const void**cvt=*(const void***)component;
  if(((int32_t(*)(void*,const uint8_t*,void**))cvt[0])(component,IID_PROCESSOR,&processor)!=0||!processor)return 203;
  const void**pvt=*(const void***)processor;
  VProcessSetup setup={0};setup.symbolicSampleSize=0;setup.sampleRate=sample_rates[rate];setup.maxSamplesPerBlock=256;
  if(((int32_t(*)(void*,const VProcessSetup*))pvt[7])(processor,&setup)!=0)return 204;
  rubedo_init_engine(&ref_engine);rubedo_activate(&ref_engine,sample_rates[rate]);

  clear_block();add_midi(19,36,.9f);add_midi(49,52,.51f);add_midi(103,37,.35f);
  if(run_block(processor,128)<50)return 205;
  clear_block();if(run_block(processor,256)<50)return 206;

  /* Cross-check additional per-pad gain/attack/pitch changes. The host
   * applies these normalized changes at block start in the VST3 wrapper. */
  clear_block();add_param(160,.64);add_param(184,.12);add_param(60,.55);
  ref_engine.pad_gain[0]=.64f;ref_engine.pad_attack[0]=.12f;ref_engine.pitch[0]=.55f;
  add_midi(7,36,.87f);add_midi(8,38,.41f);add_midi(64,36,.55f);
  if(run_block(processor,128)<30)return 207;
  clear_block();if(run_block(processor,128)<30)return 208;
 }
 return errors;
}
void __attribute__((ms_abi)) test_entry(void){ExitProcess((unsigned)run());}
