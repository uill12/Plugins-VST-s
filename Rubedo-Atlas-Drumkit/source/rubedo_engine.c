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
 for(int i=0;i<24;++i)e->mappings[i]=(int16_t)(36+i);
 for(int i=0;i<34;i++)e->params[i]=0.5;
 e->bpm=126.;e->midi_learn_target=-1;e->chance=1.0f;e->swing=.14f;e->scatter=.18f;for(int z=0;z<24;z++){e->pitch[z]=.5f;e->decay_shape[z]=.5f;e->fm_depth[z]=.5f;}e->steps[0]=0x1111;e->steps[2]=0x1010;e->steps[4]=0x5555;e->steps[5]=0xaaaa;e->params[0]=0.;e->params[1]=0.65;e->params[2]=0.5;e->params[3]=0.5;e->params[4]=0.;e->params[5]=0.5;
 for(int i=10;i<34;i++)e->params[i]=1.0;
}
void rubedo_activate(RubedoEngine *e,double sr){e->sample_rate=(sr>=8000.&&sr<=384000.)?sr:48000.;rubedo_panic(e);}
void rubedo_panic(RubedoEngine *e){for(int i=0;i<RUBEDO_VOICES;i++)e->voices[i].active=0;}
void rubedo_set_param(RubedoEngine *e,unsigned id,double value){
 if(id>=RUBEDO_PARAMS)return;
 if(id==0){int k=(int)(value+0.5);if(k<0)k=0;if(k>23)k=23;e->kit=(uint32_t)k;e->params[id]=k;return;}
 if(value<0)value=0;if(value>1)value=1;e->params[id]=value;
}
void rubedo_note_on(RubedoEngine *e,int note,double velocity){
 if(!rubedo_bank.data||velocity<=0)return;
 e->ui_last_note=(uint32_t)note;
 if(e->midi_learn_target>=0&&e->midi_learn_target<RUBEDO_PADS){
   e->mappings[e->midi_learn_target]=(int16_t)note;e->midi_learn_target=-1;
 }
 int pad=-1;
 for(int i=0;i<RUBEDO_PADS;i++)if(e->mappings[i]==note){pad=i;break;}
 if(pad<0)return;
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
      if((double)cell/1023.0 < probability)rubedo_note_on(e,e->mappings[pad],0.85);
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
   if(d>.5f){double t=v->position/96000.0;float decay=1.0f-(float)(t*(d-.5f)*1.6);
     if(decay<0)decay=0;s*=decay;}
   float fm=e->fm_depth[v->pad];if(fm>.5f){float depth=(fm-.5f)*.32f;s+=depth*(s*s*s-s*.33f);}
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