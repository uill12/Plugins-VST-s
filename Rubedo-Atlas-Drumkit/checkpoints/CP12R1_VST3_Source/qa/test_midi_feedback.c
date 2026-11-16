/* Portable C11 regression tests for the VST3 host-feedback wire format.
 * No Bitwig host/callback simulator is implied by this unit test.
 */
#include "rubedo_vst3_feedback.h"
#include <stdint.h>
#include <stdio.h>
static int bad;
#define CHECK(x) do{if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);bad++;}}while(0)
int main(void){
 CHECK(RB_V3_LEARN_TARGET==377);
 CHECK(RB_V3_PARAM_COUNT==381);
 CHECK(rb_v3_learn_target(0.)==-1);
 CHECK(rb_v3_learn_target(1.01)==-1);
 for(unsigned pad=0;pad<24;pad++)
  CHECK(rb_v3_learn_target(rb_v3_learn_arm(pad))==(int)pad);
 CHECK(rb_v3_learn_arm(24)==0.);
 const uint32_t masks[]={1u,2u,1u<<15,1u<<16,1u<<23,0xffffffu,
                           0xaaaaaau,0x555555u,0x3u,0x800001u};
 for(unsigned i=0;i<sizeof(masks)/sizeof(masks[0]);i++){
  for(unsigned parity=0;parity<2;parity++){
   double n=rb_v3_mask_value(masks[i],parity);
   CHECK(rb_v3_mask_decode(n)==(masks[i]&0xffffffu));
   if(parity)CHECK(n!=rb_v3_mask_value(masks[i],0));
  }
 }
 uint32_t seed=0x9e3779b9u;
 for(int i=0;i<10000;i++){
  seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
  uint32_t mask=seed&0xffffffu;
  CHECK(rb_v3_mask_decode(rb_v3_mask_value(mask,(unsigned)i&1u))==mask);
 }
 for(int note=0;note<128;note++){
  for(unsigned parity=0;parity<2;parity++)
   CHECK(rb_v3_unmapped_decode(rb_v3_unmapped_value(note,parity))==note);
  for(int pad=0;pad<24;pad++){
   for(unsigned parity=0;parity<2;parity++){
    int outpad=-1,outnote=-1;
    CHECK(rb_v3_learn_decode(rb_v3_learn_value(pad,note,parity),&outpad,&outnote));
    CHECK(outpad==pad&&outnote==note);
   }
  }
 }
 CHECK(rb_v3_unmapped_decode(0.)==-1);
 int p=0,n=0;
 CHECK(!rb_v3_learn_decode(0.,&p,&n));
 CHECK(rb_v3_learn_value(-1,36,0)==0.);
 CHECK(rb_v3_learn_value(24,36,0)==0.);
 if(bad){printf("VST3 MIDI feedback: %d errors\n",bad);return 1;}
 puts("VST3 MIDI feedback wire: PASS (pad masks, 24 learn targets, 24x128 learn results, unmapped notes, parity)");
 return 0;
}
