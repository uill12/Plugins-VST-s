#include "rubedo_engine.h"
#include "win_imports.h"
#include <stdint.h>
#include <stddef.h>
#include "rubedo_names.inc"
#include "rubedo_sinc_coeffs.inc"

RubedoBank rubedo_bank={0};
static unsigned rubedo_users=0;
#ifdef _WIN32
extern void *rubedo_module;
#endif

static void rb_zero(void *a,uint64_t n){uint8_t*x=(uint8_t*)a;for(uint64_t i=0;i<n;++i)x[i]=0;}
static void rb_copy(void *a,const void *b,uint64_t n){uint8_t*x=(uint8_t*)a;const uint8_t*y=(const uint8_t*)b;for(uint64_t i=0;i<n;++i)x[i]=y[i];}
static int rb_eq(const char*a,const char*b){while(*a&&*a==*b){a++;b++;}return *a==*b;}
static float rb_clamp(float a,float low,float high){if(a<low)return low;if(a>high)return high;return a;}
static uint32_t rb_u32(const uint8_t*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint64_t rb_u64(const uint8_t*p){return (uint64_t)rb_u32(p)|((uint64_t)rb_u32(p+4)<<32);}

int rubedo_load_bank(void){
 if (rubedo_users){rubedo_users++;return rubedo_bank.data!=0;}
#ifdef _WIN32
 char path[2048];uint32_t n=GetModuleFileNameA((HMODULE)rubedo_module,path,sizeof(path));if(!n||n>=sizeof(path))return 0;
 HANDLE f=CreateFileA(path,0x80000000u,1,0,3,0x80,0);if(f==RUBEDO_INVALID_HANDLE)return 0;
 uint32_t size=SetFilePointer(f,0,0,2);if(size<64){CloseHandle(f);return 0;}
 uint8_t footer[32];uint32_t got=0;
 if(SetFilePointer(f,(int32_t)(size-32),0,0)==0xffffffffu||!ReadFile(f,footer,32,&got,0)||got!=32){CloseHandle(f);return 0;}
 if(footer[0]!='R'||footer[1]!='B'||footer[2]!='O'||footer[3]!='V'||footer[4]!='R'||footer[5]!='1'||footer[6]!='0'||footer[7]!='3'){CloseHandle(f);return 0;}
 uint64_t offset=rb_u64(footer+8),bytes=rb_u64(footer+16);
 if(offset>=size||bytes>size||offset+bytes+32!=size||bytes>0xffffffffull){CloseHandle(f);return 0;}
 uint8_t *memory=VirtualAlloc(0,bytes,RUBEDO_MEM_RESERVE|RUBEDO_MEM_COMMIT,RUBEDO_PAGE_READWRITE);
 if(!memory){CloseHandle(f);return 0;}
 SetFilePointer(f,(int32_t)offset,0,0);
 for(uint32_t at=0;at<(uint32_t)bytes;){uint32_t chunk=(uint32_t)bytes-at;if(chunk>(1u<<20))chunk=1u<<20;got=0;if(!ReadFile(f,memory+at,chunk,&got,0)||got!=chunk){VirtualFree(memory,0,RUBEDO_MEM_RELEASE);CloseHandle(f);return 0;}at+=chunk;}
 CloseHandle(f);
#else
 const char *path="Rubedo_96k_PCM24.bank";
 FILE*f=fopen(path,"rb");if(!f)return 0;
 if(fseek(f,0,SEEK_END)){fclose(f);return 0;}
 long z=ftell(f); if(z<=0){fclose(f);return 0;} rewind(f);
 uint64_t bytes=(uint64_t)z;uint8_t*memory=(uint8_t*)malloc(bytes);
 if(!memory){fclose(f);return 0;}
 if(fread(memory,1,bytes,f)!=bytes){free(memory);fclose(f);return 0;}fclose(f);
#endif
 if(!rb_eq((const char*)memory,"RBDK24")||rb_u32(memory+8)!=1||rb_u32(memory+12)!=96000||rb_u32(memory+16)!=2304){
#ifdef _WIN32
   VirtualFree(memory,0,RUBEDO_MEM_RELEASE);
#else
   free(memory);
#endif
   return 0;
 }
 const uint32_t bytes32=(uint32_t)bytes;
 for(uint32_t i=0;i<2304;i++){
  uint32_t off=rb_u32(memory+24+i*8),frames=rb_u32(memory+24+i*8+4);
  if(off>bytes32||frames>(bytes32-off)/3){
#ifdef _WIN32
   VirtualFree(memory,0,RUBEDO_MEM_RELEASE);
#else
   free(memory);
#endif
   return 0;
  }
  rubedo_bank.regions[i].offset=off;rubedo_bank.regions[i].frames=frames;
 }
 rubedo_bank.data=memory;rubedo_bank.bytes=bytes32;rubedo_users=1;return 1;
}
void rubedo_unload_bank(void){if(!rubedo_users)return;if(--rubedo_users)return;
#ifdef _WIN32
 VirtualFree((void*)rubedo_bank.data,0,RUBEDO_MEM_RELEASE);
#else
 free((void*)rubedo_bank.data);
#endif
 rubedo_bank.data=0;rubedo_bank.bytes=0;
}
void rubedo_init_engine(RubedoEngine *e){
 rb_zero(e,sizeof(*e));e->sample_rate=48000.;e->gain=0.7;e->kit=0;
 for(int i=0;i<24;++i)atomic_init(&e->mappings[i],36+i);
 for(int i=0;i<34;i++)e->params[i]=0.5;
 e->bpm=126.;atomic_init(&e->midi_learn_target,-1);atomic_init(&e->ui_note_seq,0);atomic_init(&e->ui_last_pad,-1);atomic_init(&e->ui_pending_mask,0);atomic_init(&e->audition_head,0);atomic_init(&e->audition_tail,0);atomic_init(&e->ui_learn_seq,0);atomic_init(&e->ui_learn_note,-1);atomic_init(&e->host_mapping_dirty,0);atomic_init(&e->ui_unmapped_note,-1);atomic_init(&e->ui_unmapped_seq,0);e->chance=1.0f;e->swing=.14f;e->scatter=.18f;for(int z=0;z<24;z++){e->pitch[z]=.5f;e->decay_shape[z]=.5f;e->fm_depth[z]=.5f;e->pad_gain[z]=.5f;e->pad_attack[z]=0.f;e->pad_release[z]=.5f;}e->steps[0]=0x1111;e->steps[2]=0x1010;e->steps[4]=0x5555;e->steps[5]=0xaaaa;
 for(int v=0;v<4;v++)for(int pad=0;pad<RUBEDO_PADS;pad++){
  uint16_t s=e->steps[pad];unsigned rotation=(unsigned)(4*v);e->patterns[v][pad]=(uint16_t)((s<<rotation)|(s>>(16-rotation)));
 }e->variation=0;e->params[0]=0.;e->params[1]=0.65;e->params[2]=0.5;e->params[3]=0.5;e->params[4]=0.;e->params[5]=0.5;
 for(int i=10;i<34;i++)e->params[i]=1.0;
}
void rubedo_activate(RubedoEngine *e,double sr){e->sample_rate=(sr>=8000.&&sr<=384000.)?sr:48000.;rubedo_panic(e);}
void rubedo_panic(RubedoEngine *e){for(int i=0;i<RUBEDO_VOICES;i++)e->voices[i].active=0;}
void rubedo_set_param(RubedoEngine *e,unsigned id,double value){
 if(id>=RUBEDO_PARAMS)return;
 if(id==0){int k=(int)(value+0.5);if(k<0)k=0;if(k>23)k=23;e->kit=(uint32_t)k;e->params[id]=k;return;}
 if(value<0)value=0;if(value>1)value=1;e->params[id]=value;
}
static void rb_start_voice(RubedoEngine *e,int pad,double velocity){
 if(!e||!rubedo_bank.data||pad<0||pad>=RUBEDO_PADS||velocity<=0)return;
 if(pad==4){for(int i=0;i<RUBEDO_VOICES;i++)if(e->voices[i].active&&e->voices[i].pad==5)e->voices[i].active=0;}
 uint32_t var=(velocity>=0.52?2:0)+(e->rr[pad]++ & 1);
 uint32_t index=(e->kit*24u+(uint32_t)pad)*4u+var;
 if(!rubedo_bank.regions[index].frames)return;
 int victim=-1;double far=0;
 for(int i=0;i<RUBEDO_VOICES;i++){if(!e->voices[i].active){victim=i;break;}if(e->voices[i].position>far){far=e->voices[i].position;victim=i;}}
 if(victim<0)return;
 RubedoVoice *v=&e->voices[victim];
 v->active=1;v->position=0;v->step=(96000./e->sample_rate)*(0.70+0.60*(double)e->pitch[pad]);v->kit=e->kit;v->pad=(uint32_t)pad;v->variation=var;
 v->amplitude=rb_clamp((float)(0.16+velocity*0.84),0.f,1.f)*rb_clamp((float)e->params[10+pad],0.f,1.f);
 if(e->pad_gain[pad]!=.5f)v->amplitude*=(float)(.25+1.5*(double)e->pad_gain[pad]);
 v->last_l=v->last_r=0;
 // Use bandlimited reconstruction when source-position advances faster than 1.15 samples/frame.
 // Preserves natural 96k->88.2k sample rate conversion at neutral pitch; prevents 44.1/48k aliasing.
 v->sinc_band=255;
 if(v->step>1.15){
   float c=(float)(.94/v->step);
   for(unsigned b=0;b<(unsigned)(sizeof(rubedo_cutoffs)/sizeof(rubedo_cutoffs[0]));++b){
     if(rubedo_cutoffs[b]<=c){v->sinc_band=(uint8_t)b;break;}
   }
   if(v->sinc_band==255)v->sinc_band=(uint8_t)(sizeof(rubedo_cutoffs)/sizeof(rubedo_cutoffs[0])-1);
 }
}
static void rb_mark_pad(RubedoEngine *e,int pad){
 atomic_store_explicit(&e->ui_last_pad,pad,memory_order_relaxed);
 atomic_fetch_or_explicit(&e->ui_pending_mask,1u<<pad,memory_order_release);
 atomic_fetch_add_explicit(&e->ui_note_seq,1,memory_order_release);
}
void rubedo_trigger_pad(RubedoEngine *e,int pad,double velocity){
 rb_start_voice(e,pad,velocity);if(e&&pad>=0&&pad<24&&velocity>0)rb_mark_pad(e,pad);
}
void rubedo_request_audition(RubedoEngine*e,int pad,double velocity){
 if(!e||pad<0||pad>=RUBEDO_PADS||velocity<=0)return;
 if(velocity>1.)velocity=1.;
 unsigned head=atomic_load_explicit(&e->audition_head,memory_order_relaxed);
 unsigned tail=atomic_load_explicit(&e->audition_tail,memory_order_acquire);
 if(head-tail>=64)return; // bounded, lock-free SPSC GUI -> audio queue
 e->audition_queue[head&63u]=((uint32_t)pad<<8)|(uint32_t)(velocity*255.+.5);
 atomic_store_explicit(&e->audition_head,head+1,memory_order_release);
}
// One authoritative MIDI-note -> voice mapping, shared by audio and GUI.
int rubedo_note_to_pad(const RubedoEngine *e,int note){
 if(!e||note<0||note>127)return -1;
 for(int pad=0;pad<RUBEDO_PADS;pad++)
  if(atomic_load_explicit(&e->mappings[pad],memory_order_acquire)==note)return pad;
 return -1;
}
int rubedo_assign_note(RubedoEngine *e,int pad,int note){
 if(!e||pad<0||pad>=RUBEDO_PADS||note<0||note>127)return 0;
 int old=atomic_load_explicit(&e->mappings[pad],memory_order_acquire);
 int conflict=rubedo_note_to_pad(e,note);
 if(conflict==pad)return 1;
 // If another pad owns the target note, swap; no silent duplicate/unreachable pads.
 if(conflict>=0)atomic_store_explicit(&e->mappings[conflict],old,memory_order_release);
 atomic_store_explicit(&e->mappings[pad],note,memory_order_release);
 return 1;
}
void rubedo_import_mappings(RubedoEngine*e,const int16_t notes[RUBEDO_PADS]){
 if(!e||!notes)return;
 // Preserve every unique mapping from old CP08/CP09 projects, repair duplicates and
 // out-of-range assignments deterministically using free original MIDI numbers.
 uint8_t used[128]={0};int repaired[RUBEDO_PADS];
 for(int pad=0;pad<RUBEDO_PADS;pad++){
  int n=notes[pad];
  if(n>=0&&n<=127&&!used[n]){used[n]=1;repaired[pad]=n;}
  else repaired[pad]=-1;
 }
 for(int pad=0;pad<RUBEDO_PADS;pad++)if(repaired[pad]<0){
  int n=36+pad;
  if(used[n]){for(n=0;n<128&&used[n];n++);}
  if(n>=128)n=36+pad;
  used[n]=1;repaired[pad]=n;
 }
 for(int pad=0;pad<RUBEDO_PADS;pad++)
  atomic_store_explicit(&e->mappings[pad],repaired[pad],memory_order_release);
}
void rubedo_note_on(RubedoEngine *e,int note,double velocity){
 if(!e||velocity<=0||note<0||note>127)return;
 e->ui_last_note=(uint32_t)note;
 int target=atomic_exchange_explicit(&e->midi_learn_target,-1,memory_order_acq_rel);
 if(target>=0&&target<RUBEDO_PADS){
  rubedo_assign_note(e,target,note);
  atomic_store_explicit(&e->ui_learn_note,note,memory_order_relaxed);
  atomic_fetch_add_explicit(&e->ui_learn_seq,1,memory_order_release);
  atomic_store_explicit(&e->host_mapping_dirty,1,memory_order_release);
  rb_start_voice(e,target,velocity);rb_mark_pad(e,target);return;
 }
 int pad=rubedo_note_to_pad(e,note);
 if(pad>=0){rb_start_voice(e,pad,velocity);rb_mark_pad(e,pad);}
 else {atomic_store_explicit(&e->ui_unmapped_note,note,memory_order_relaxed);
       atomic_fetch_add_explicit(&e->ui_unmapped_seq,1,memory_order_release);}
}
void rubedo_pattern_changed(RubedoEngine*e){
 if(e)for(int i=0;i<RUBEDO_PADS;i++)e->patterns[e->variation&3u][i]=e->steps[i];
}
void rubedo_select_variation(RubedoEngine*e,unsigned variation){
 if(!e)return;
 rubedo_pattern_changed(e);e->variation=(uint8_t)(variation&3u);
 for(int i=0;i<RUBEDO_PADS;i++)e->steps[i]=e->patterns[e->variation][i];
}
static float rb_sample(const uint8_t*ptr,uint32_t j){
 const uint8_t*p=ptr+3ull*j;
 int32_t s=(int32_t)((uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16));
 if(s&0x800000)s|=(int32_t)0xff000000u;
 return (float)s*(1.f/8388608.f);
}
static float rb_cubic(const uint8_t*p,uint32_t count,double position){
 uint32_t k=(uint32_t)position;
 if(k>=count)return 0;
 float f=(float)(position-(double)k);
 float a=rb_sample(p,k>0?k-1:k),b=rb_sample(p,k),c=rb_sample(p,k+1<count?k+1:count-1),d=rb_sample(p,k+2<count?k+2:count-1);
 float x=-0.5f*a+1.5f*b-1.5f*c+0.5f*d;
 float y=a-2.5f*b+2.f*c-0.5f*d;
 float z=-0.5f*a+0.5f*c;
 return ((x*f+y)*f+z)*f+b;
}
/* Anti-aliasing 24-tap polyphase interpolation. Coefficients are immutable to avoid
   CPU-heavy sin/cos calls on the audio thread. The table has 65 phases so phase 64
   is a continuous endpoint. No extra latency or allocations. */
static float rb_sinc_sample(const uint8_t*p,uint32_t count,double position,uint8_t band){
 uint32_t k=(uint32_t)position;
 float phase=(float)((position-(double)k)*64.0);
 int phase0=(int)phase;
 if(phase0<0)phase0=0;if(phase0>63)phase0=63;
 float mix=phase-(float)phase0;
 const float *c0=rubedo_polyphase[band][phase0];
 const float *c1=rubedo_polyphase[band][phase0+1];
 float sum=0;
 for(int j=0;j<24;j++){
  int64_t ix=(int64_t)k+(j-11);
  if(ix<0)ix=0;
  else if((uint64_t)ix>=count)ix=count-1;
  float w=c0[j]+(c1[j]-c0[j])*mix;
  sum+=w*rb_sample(p,(uint32_t)ix);
 }
 return sum;
}
/* Linked, continuously curving final output guard. Unlike the CP05 hard ceiling,
   even overload peaks retain waveform dynamics; both channels receive the same gain. */
static void rb_soft_ceiling(float *l,float*r){
 float a=*l<0?-*l:*l,b=*r<0?-*r:*r;
 float p=a>b?a:b;
 const float knee=.70f,ceiling=.88f;
 if(p>knee){
  float d=p-knee;
  float mapped=knee+d/(1.0f+d/(ceiling-knee));
  float g=mapped/p;
  *l*=g;*r*=g;
 }
}
void rubedo_render(RubedoEngine*e,float*l,float*r,double*ld,double*rd,uint32_t frames){
 if(!e||!rubedo_bank.data){for(uint32_t i=0;i<frames;i++){if(l)l[i]=0;if(r)r[i]=0;if(ld)ld[i]=0;if(rd)rd[i]=0;}return;}
 // UI pad audition is applied on the audio thread, never directly to live voices.
 unsigned tail=atomic_load_explicit(&e->audition_tail,memory_order_relaxed);
 unsigned head=atomic_load_explicit(&e->audition_head,memory_order_acquire);
 while(tail!=head){uint32_t packet=e->audition_queue[tail&63u];
  int pad=(int)((packet>>8)&255u);double vel=(double)(packet&255u)/255.;
  rb_start_voice(e,pad,vel);tail++;
 }
 atomic_store_explicit(&e->audition_tail,tail,memory_order_release);
 float gain=(float)(e->params[1]*1.25);
 float drive=(float)e->params[4];
 for(uint32_t i=0;i<frames;++i){
  if(e->playing){
   double period=e->sample_rate*60.0/(e->bpm*4.0);
   if(e->step_clock<=0.0){
    uint32_t st=e->step_index&15u;
    for(int pad=0;pad<RUBEDO_PADS;pad++)if(e->steps[pad]&(1u<<st)){
      // Repeatable non-pitched cellular probability mask, governed by Scatter/Chance.
      uint32_t cell=((st*1103515245u+pad*747796405u+e->note_serial*2891336453u)>>17)&1023u;
      double probability=e->chance*(1.0-e->scatter*.25)*(1.0-e->params[8]*.20);
      if((double)cell/1023.0 < probability)rb_start_voice(e,pad,0.85);
    }
    e->step_index=(st+1)&15u;e->note_serial++;
    double sh=e->swing*.45+(e->params[3]-.5)*.10;
    e->step_clock=period*((st&1u)?(1.0+sh):(1.0-sh));
   }
   e->step_clock-=1.0;
  }
  float sumL=0,sumR=0;
  for(uint32_t j=0;j<RUBEDO_VOICES;++j){RubedoVoice *v=&e->voices[j];if(!v->active)continue;
   uint32_t idx=(v->kit*24u+v->pad)*4u+v->variation;
   const RubedoRegion*reg=&rubedo_bank.regions[idx];
   if(v->position+1>=reg->frames){v->active=0;continue;}
   float s=(v->sinc_band==255?rb_cubic(rubedo_bank.data+reg->offset,reg->frames,v->position):rb_sinc_sample(rubedo_bank.data+reg->offset,reg->frames,v->position,v->sinc_band))*v->amplitude;
   float d=e->decay_shape[v->pad]+(float)((e->params[7]-.5)*.35);
   if(d>.5f||(e->decay_active[v->pad]&&d!=.5f)){double t=v->position/96000.0;float decay=1.0f-(float)(t*(d-.5f)*1.6);
     if(decay<0)decay=0;if(decay>1.18f)decay=1.18f;s*=decay;}
   float fm=e->fm_depth[v->pad];if(fm>.5f||(e->fm_active[v->pad]&&fm!=.5f)){float depth=(fm-.5f)*.32f;s+=depth*(s*s*s-s*.33f);}
   // Independent voice envelope controls (defaults are sonically neutral).
   if(e->pad_attack[v->pad]>.0001f){
     double age=v->position/96000.0,attack=(double)e->pad_attack[v->pad]*.095;
     if(age<attack)s*=(float)(age/attack);
   }
   float rel=e->pad_release[v->pad];
   if(rel<.499f){float t=(float)(v->position/(double)reg->frames);float amt=(.5f-rel)*2.f;
     float fade=1.0f-amt*t*t;if(fade<0)fade=0;s*=fade;
   }else if(rel>.501f){float t=(float)(v->position/(double)reg->frames);s*=1.f+(rel-.5f)*.38f*t;}
   // Attack contour only softens transients when moved above neutral.
   if(e->params[6]>.5){double age=v->position/96000.0,ms=(e->params[6]-.5)*.07;
     if(age<ms)s*=(float)(age/ms);}
   // Tone parameter darkens using a continuous single-pole filter.
   if(e->params[2]<.5){float alpha=(float)(.05+.95*(e->params[2]*2));
      v->last_l+=alpha*(s-v->last_l);s=v->last_l;}

   v->position+=v->step;
   // Fixed gentle stereo placement by role, still fully mono-compatible.
   float p=((int)(v->pad%5)-2)*.035f;
   sumL+=s*(1.f-p);sumR+=s*(1.f+p);
  }
  sumL*=gain;sumR*=gain;
  if(e->params[2]>.5){float edge=(float)((e->params[2]-.5)*.25);
    sumL+=edge*(sumL*sumL*sumL);sumR+=edge*(sumR*sumR*sumR);}
  if(e->params[9]>.5){ // very short bounded stereo ambience
    uint32_t at=e->room_idx&4095u;
    float wet=(float)((e->params[9]-.5)*.32);
    float dl=e->delay_l[at],dr=e->delay_r[at];
    e->delay_l[at]=sumL+dr*.10f;e->delay_r[at]=sumR+dl*.10f;
    sumL+=dr*wet;sumR+=dl*wet;e->room_idx++;
  }
  // Width and tone macros now alter actual output (neutral at 0.5).
  float mid=(sumL+sumR)*.5f,side=(sumL-sumR)*.5f;
  float width=(float)(e->params[5]*2.0);
  sumL=mid+side*width;sumR=mid-side*width;
  // Neutral drive is fully bypassed, non-zero drive adds a bounded soft-knee.
  if(drive>0.001f){float amount=drive*3.f;sumL=sumL*(1.f+amount)/(1.f+amount*(sumL<0?-sumL:sumL));sumR=sumR*(1.f+amount)/(1.f+amount*(sumR<0?-sumR:sumR));}
  // Hi-fi linked soft ceiling: no flat-topped hard clipping on layered hits.
  rb_soft_ceiling(&sumL,&sumR);
  if(l)l[i]=sumL;if(r)r[i]=sumR;if(ld)ld[i]=sumL;if(rd)rd[i]=sumR;
 }
}

void rubedo_export_extra(RubedoEngine*e,RubedoExtraState*s){
 if(!e||!s)return;
 for(int i=0;i<24;i++){s->pitch[i]=e->pitch[i];s->decay_shape[i]=e->decay_shape[i];s->fm_depth[i]=e->fm_depth[i];s->steps[i]=e->steps[i];}
 s->swing=e->swing;s->scatter=e->scatter;s->chance=e->chance;s->bpm=e->bpm;
}
void rubedo_import_extra(RubedoEngine*e,const RubedoExtraState*s){
 if(!e||!s)return;
 for(int i=0;i<24;i++){
  if(s->pitch[i]>=0&&s->pitch[i]<=1)e->pitch[i]=s->pitch[i];
  if(s->decay_shape[i]>=0&&s->decay_shape[i]<=1)e->decay_shape[i]=s->decay_shape[i];
  if(s->fm_depth[i]>=0&&s->fm_depth[i]<=1)e->fm_depth[i]=s->fm_depth[i];e->steps[i]=s->steps[i];
 }
 if(s->swing>=0&&s->swing<=1)e->swing=s->swing;
 if(s->scatter>=0&&s->scatter<=1)e->scatter=s->scatter;
 if(s->chance>=0&&s->chance<=1)e->chance=s->chance;
 if(s->bpm>=40&&s->bpm<=250)e->bpm=s->bpm;
}

void rubedo_export_extra_v3(RubedoEngine*e,RubedoExtraStateV3*s){
 if(!e||!s)return;rubedo_pattern_changed(e);
 for(int i=0;i<24;i++){
  s->gain[i]=e->pad_gain[i];s->attack[i]=e->pad_attack[i];s->release[i]=e->pad_release[i];s->decay_active[i]=e->decay_active[i];s->fm_active[i]=e->fm_active[i];
  for(int v=0;v<4;v++)s->patterns[v][i]=e->patterns[v][i];
 }
 s->variation=e->variation;s->reserved[0]=s->reserved[1]=s->reserved[2]=0;
}
void rubedo_import_extra_v3(RubedoEngine*e,const RubedoExtraStateV3*s){
 if(!e||!s)return;
 for(int i=0;i<24;i++){
  if(s->gain[i]>=0.f&&s->gain[i]<=1.f)e->pad_gain[i]=s->gain[i];
  if(s->attack[i]>=0.f&&s->attack[i]<=1.f)e->pad_attack[i]=s->attack[i];
  if(s->release[i]>=0.f&&s->release[i]<=1.f)e->pad_release[i]=s->release[i];
  e->decay_active[i]=(uint8_t)(s->decay_active[i]!=0);e->fm_active[i]=(uint8_t)(s->fm_active[i]!=0);
  for(int v=0;v<4;v++)e->patterns[v][i]=s->patterns[v][i];
 }
 e->variation=(uint8_t)(s->variation&3u);
 for(int i=0;i<24;i++)e->steps[i]=e->patterns[e->variation][i];
}