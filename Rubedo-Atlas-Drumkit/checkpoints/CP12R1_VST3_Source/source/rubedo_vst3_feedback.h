#pragma once
/* CP12R1: host-mediated, per-instance VST3 MIDI feedback wire contract.
 * These are normalized values for VST3 outputParameterChanges. Not saved in
 * CP12 v1/v2/v3 component state. No allocation or blocking is performed.
 *
 * Note: actual host routing remains subject to VST3/Bitwig validation.
 */
#include <stdint.h>

enum {
 RB_V3_LEARN_TARGET=377,  /* 0=off, (pad+1)/24 for 0..23 */
 RB_V3_ACTIVITY_MASK=378, /* (24-bit mask << 1 | parity) / 33554431 */
 RB_V3_UNMAPPED_NOTE=379, /* (2*note + 1 + parity) / 257 */
 RB_V3_LEARN_RESULT=380,  /* (2*(pad*128+note)+1+parity) / 6145 */
 RB_V3_PARAM_COUNT=381
};
static inline double rb_v3_learn_arm(unsigned pad){return pad<24?(double)(pad+1u)/24.0:0.;}
static inline int rb_v3_learn_target(double norm){
 if(!(norm>0.)||norm>1.)return -1;
 int target=(int)(norm*24.0+0.5)-1;
 return target>=0&&target<24?target:-1;
}
static inline double rb_v3_mask_value(uint32_t mask,unsigned parity){
 uint32_t code=((mask&0xffffffu)<<1)|(parity&1u);
 return (double)code/33554431.0;
}
static inline uint32_t rb_v3_mask_decode(double norm){
 if(!(norm>0.)||norm>1.)return 0;
 uint32_t code=(uint32_t)(norm*33554431.0+0.5);
 return (code>>1)&0xffffffu;
}
static inline double rb_v3_unmapped_value(int note,unsigned parity){
 if(note<0||note>127)return 0.;
 return (double)((unsigned)note*2u+1u+(parity&1u))/257.0;
}
static inline int rb_v3_unmapped_decode(double norm){
 if(!(norm>0.)||norm>1.)return -1;
 int code=(int)(norm*257.0+0.5);
 return code>=1&&code<=256?(code-1)/2:-1;
}
static inline double rb_v3_learn_value(int pad,int note,unsigned parity){
 if(pad<0||pad>23||note<0||note>127)return 0.;
 unsigned packed=((unsigned)pad*128u+(unsigned)note)*2u+1u+(parity&1u);
 return (double)packed/6145.0;
}
static inline int rb_v3_learn_decode(double norm,int *pad,int *note){
 if(!pad||!note||!(norm>0.)||norm>1.)return 0;
 unsigned code=(unsigned)(norm*6145.0+0.5);
 if(code<1||code>6144)return 0;
 unsigned packed=(code-1u)/2u;
 *pad=(int)(packed/128u);*note=(int)(packed%128u);
 return *pad<24;
}
