/* Win64 source-only VST3 processor event/host-parameter integration fixture.
 * Uses the ACTUAL compiled VST3 core and CP12 engine with mock host queues.
 * No original PCM resources required: event feedback is tested while voices
 * remain silent. This is not an in-Bitwig nor audio-equivalence test.
 */
#include "vst3_min_abi.h"
#include "rubedo_vst3_feedback.h"
#include <stdint.h>
#include <stddef.h>
__declspec(dllimport) __attribute__((noreturn)) void __attribute__((ms_abi)) ExitProcess(unsigned);
extern void*GetPluginFactory(void);
#define OK 0
typedef struct TPoint {int32_t off;double norm;}TPoint;
typedef struct TQueue {const void **vt;uint32_t id;int32_t n;TPoint points[8];}TQueue;
typedef struct TChanges {const void **vt;int32_t n;TQueue q[64];}TChanges;
typedef struct TEvents {const void **vt;int32_t n;VEvent events[8];}TEvents;
static int failures;
#define REQ(x) do{if(!(x))failures++;}while(0)
static int32_t q_id(void *v){return (int32_t)((TQueue*)v)->id;}
static int32_t q_count(void*v){return ((TQueue*)v)->n;}
static int32_t q_get(void*v,int32_t idx,int32_t *offset,double *norm){
 TQueue*q=v;if(idx<0||idx>=q->n||!offset||!norm)return 1;
 *offset=q->points[idx].off;*norm=q->points[idx].norm;return 0;
}
static int32_t q_add(void*v,int32_t offset,double norm,int32_t *idx){
 TQueue*q=v;if(q->n>=8||!idx)return 1;
 *idx=q->n;q->points[q->n].off=offset;q->points[q->n].norm=norm;q->n++;return 0;
}
static const void* Q_VT[7]={0,0,0,(void*)q_id,(void*)q_count,(void*)q_get,(void*)q_add};
static int32_t ch_count(void*v){return ((TChanges*)v)->n;}
static void* ch_data(void*v,int32_t idx){TChanges*c=v;return idx>=0&&idx<c->n?&c->q[idx]:0;}
static void* ch_add(void*v,const uint32_t* id,int32_t *idx){
 TChanges*c=v;if(!idx||!id)return 0;
 for(int i=0;i<c->n;i++)if(c->q[i].id==*id){*idx=i;return &c->q[i];}
 if(c->n>=64)return 0;
 *idx=c->n;TQueue*q=&c->q[c->n++];q->vt=Q_VT;q->id=*id;q->n=0;return q;
}
static const void* CH_VT[6]={0,0,0,(void*)ch_count,(void*)ch_data,(void*)ch_add};
static int32_t evt_count(void*v){return ((TEvents*)v)->n;}
static int32_t evt_get(void*v,int32_t idx,VEvent*e){
 TEvents*t=v;if(!e||idx<0||idx>=t->n)return 1;*e=t->events[idx];return 0;
}
static const void* E_VT[5]={0,0,0,(void*)evt_count,(void*)evt_get};
static TChanges in,out;
static TEvents events;
static void empty_ch(TChanges*c){c->vt=CH_VT;c->n=0;}
static void empty_evt(void){events.vt=E_VT;events.n=0;}
static void add_input(uint32_t id,double v){
 int32_t idx=0;TQueue*q=ch_add(&in,&id,&idx);REQ(q);if(q)REQ(q_add(q,0,v,&idx)==0);
}
static void note_event(int note){
 if(events.n>=8){failures++;return;}
 VEvent*e=&events.events[events.n++];
 e->type=0;e->sampleOffset=0;e->noteOn.pitch=(int16_t)note;e->noteOn.velocity=.9f;
}
static double find_out(uint32_t id){
 for(int i=0;i<out.n;i++)if(out.q[i].id==id&&out.q[i].n>0)return out.q[i].points[out.q[i].n-1].norm;
 failures++;return -1.;
}
static const uint8_t CLS_COMP[16]={0x52,0x42,0x44,0x4b,0x10,0x21,0x49,0x82,0xb9,0x33,0x43,0x22,0x26,0x11,0x14,0x01};
static const uint8_t CLS_CTRL[16]={0x52,0x42,0x44,0x4b,0x10,0x21,0x49,0x82,0xb9,0x33,0x43,0x22,0x26,0x11,0x14,0x02};
static const uint8_t IID_UNKNOWN[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
static const uint8_t IID_PROCESSOR[16]={0x42,0x04,0x3f,0x99,0xb7,0xda,0x45,0x3c,0xa5,0x69,0xe7,0x9d,0x9a,0xae,0xc3,0x3d};
static int32_t process(void*audio){
 VProcessData d={0};d.numSamples=64;d.outputParameterChanges=(VParamChanges*)&out;
 d.inputParameterChanges=(VParamChanges*)&in;d.inputEvents=(VEventList*)&events;
 const void**vt=*(const void***)audio;
 return ((int32_t(*)(void*,VProcessData*))vt[9])(audio,&d);
}
static int run(void){
 void*f=GetPluginFactory();if(!f)return 10;
 const void**vt=*(const void***)f;void*comp=0;
 int32_t rc=((int32_t(*)(void*,const uint8_t*,const uint8_t*,void**))vt[6])(f,CLS_COMP,IID_UNKNOWN,&comp);
 if(rc!=OK||!comp)return 11;
 void*audio=0;const void**cvt=*(const void***)comp;
 rc=((int32_t(*)(void*,const uint8_t*,void**))cvt[0])(comp,IID_PROCESSOR,&audio);
 if(rc!=OK||!audio)return 12;
 void*ctrl=0;
 rc=((int32_t(*)(void*,const uint8_t*,const uint8_t*,void**))vt[6])(f,CLS_CTRL,IID_UNKNOWN,&ctrl);
 if(rc!=OK||!ctrl)return 13;
 const void**ctrlvt=*(const void***)ctrl;
 empty_ch(&in);empty_ch(&out);empty_evt();
 add_input(RB_V3_LEARN_TARGET,rb_v3_learn_arm(0));note_event(72);
 REQ(process(audio)==OK);
 REQ(rb_v3_mask_decode(find_out(RB_V3_ACTIVITY_MASK))==1u);
 REQ(find_out(RB_V3_LEARN_TARGET)==0.);
 REQ((int)(find_out(136u)*127.+.5)==72);
 int pad=-1,note=-1;
 REQ(rb_v3_learn_decode(find_out(RB_V3_LEARN_RESULT),&pad,&note));
 REQ(pad==0&&note==72);
 /* A VST3 host returns outputParameterChanges via IEditController::
  * setParamNormalized. Exercise the real controller endpoint, not only
  * the wire codec. The editor is not attached in this source-only test. */
 for(int i=0;i<out.n;i++){
  TQueue*q=&out.q[i];if(!q->n)continue;
  double v=q->points[q->n-1].norm;
  REQ(((int32_t(*)(void*,uint32_t,double))ctrlvt[15])(ctrl,q->id,v)==OK);
 }
 double assigned=((double(*)(void*,uint32_t))ctrlvt[14])(ctrl,136u);
 REQ(assigned>.565&&assigned<.568); /* note 72 / 127 */
 double learned=((double(*)(void*,uint32_t))ctrlvt[14])(ctrl,RB_V3_LEARN_RESULT);
 int hostpad=-1,hostnote=-1;
 REQ(rb_v3_learn_decode(learned,&hostpad,&hostnote));
 REQ(hostpad==0&&hostnote==72);
 /* A detached VST3 edit controller still needs canonical swap behavior:
  * remapping pad 0 to pad 1's note must update both cached parameters. */
 REQ(((int32_t(*)(void*,uint32_t,double))ctrlvt[15])(ctrl,136u,37./127.)==OK);
 REQ((int)(((double(*)(void*,uint32_t))ctrlvt[14])(ctrl,136u)*127.+.5)==37);
 REQ((int)(((double(*)(void*,uint32_t))ctrlvt[14])(ctrl,137u)*127.+.5)==72);
 REQ(((int32_t(*)(void*,uint32_t,double))ctrlvt[15])(ctrl,136u,72./127.)==OK);
 REQ((int)(((double(*)(void*,uint32_t))ctrlvt[14])(ctrl,136u)*127.+.5)==72);
 REQ((int)(((double(*)(void*,uint32_t))ctrlvt[14])(ctrl,137u)*127.+.5)==37);
 empty_ch(&in);empty_ch(&out);empty_evt();note_event(72);
 REQ(process(audio)==OK);
 REQ(rb_v3_mask_decode(find_out(RB_V3_ACTIVITY_MASK))==1u);
 empty_ch(&in);empty_ch(&out);empty_evt();note_event(100);
 REQ(process(audio)==OK);
 REQ(rb_v3_unmapped_decode(find_out(RB_V3_UNMAPPED_NOTE))==100);
 empty_ch(&in);empty_ch(&out);empty_evt();note_event(37);note_event(38);
 REQ(process(audio)==OK);
 REQ(rb_v3_mask_decode(find_out(RB_V3_ACTIVITY_MASK))==6u);
 return failures;
}
void __attribute__((ms_abi)) test_entry(void){ExitProcess((unsigned)run());}
