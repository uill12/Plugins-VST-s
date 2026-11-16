/* CP12R1: verify the handwritten VST3 host ABI against Steinberg's official
 * generated VST3 C API header, pinned in Windows CI to a known upstream SHA.
 *
 * This comparison does not license or incorporate the SDK into Rubedo.
 * Retain Steinberg's upstream LICENSE.txt when actually distributing SDK code.
 */
#include <stddef.h>
#include "vst3_min_abi.h"
#include "vst3_c_api.h"
#define S_EQ(a,b) _Static_assert((a)==(b),#a " must match " #b)
S_EQ(sizeof(VProcessSetup),sizeof(struct Steinberg_Vst_ProcessSetup));
S_EQ(offsetof(VProcessSetup,sampleRate),offsetof(struct Steinberg_Vst_ProcessSetup,sampleRate));
S_EQ(sizeof(VAudioBusBuffers),sizeof(struct Steinberg_Vst_AudioBusBuffers));
S_EQ(offsetof(VAudioBusBuffers,silenceFlags),offsetof(struct Steinberg_Vst_AudioBusBuffers,silenceFlags));
S_EQ(sizeof(VProcessData),sizeof(struct Steinberg_Vst_ProcessData));
S_EQ(offsetof(VProcessData,outputParameterChanges),offsetof(struct Steinberg_Vst_ProcessData,outputParameterChanges));
S_EQ(offsetof(VProcessData,inputEvents),offsetof(struct Steinberg_Vst_ProcessData,inputEvents));
S_EQ(offsetof(VProcessData,processContext),offsetof(struct Steinberg_Vst_ProcessData,processContext));
S_EQ(offsetof(VEvent,busIndex),offsetof(struct Steinberg_Vst_Event,busIndex));
S_EQ(offsetof(VEvent,sampleOffset),offsetof(struct Steinberg_Vst_Event,sampleOffset));
S_EQ(offsetof(VEvent,ppqPosition),offsetof(struct Steinberg_Vst_Event,ppqPosition));
S_EQ(offsetof(VEvent,flags),offsetof(struct Steinberg_Vst_Event,flags));
S_EQ(offsetof(VEvent,type),offsetof(struct Steinberg_Vst_Event,type));
S_EQ(offsetof(VEvent,noteOn),offsetof(struct Steinberg_Vst_Event,Steinberg_Vst_Event_noteOn));
_Static_assert(sizeof(VEvent)>=sizeof(struct Steinberg_Vst_Event),"VEvent must accommodate any host event");
S_EQ(sizeof(VBusInfo),sizeof(struct Steinberg_Vst_BusInfo));
S_EQ(sizeof(VParameterInfo),sizeof(struct Steinberg_Vst_ParameterInfo));
S_EQ(offsetof(VParameterInfo,defaultNormalizedValue),offsetof(struct Steinberg_Vst_ParameterInfo,defaultNormalizedValue));
S_EQ(sizeof(VFactoryInfo),sizeof(struct Steinberg_PFactoryInfo));
S_EQ(sizeof(VClassInfo),sizeof(struct Steinberg_PClassInfo));
S_EQ(sizeof(VClassInfo2),sizeof(struct Steinberg_PClassInfo2));
/* On Win64 each vtable entry occupies one pointer slot; an omitted slot
 * shifts every subsequent call and may crash host/editor. */
S_EQ(offsetof(Steinberg_Vst_IEditControllerVtbl,setComponentState),5*sizeof(void*));
S_EQ(offsetof(Steinberg_Vst_IEditControllerVtbl,setState),6*sizeof(void*));
S_EQ(offsetof(Steinberg_Vst_IEditControllerVtbl,getState),7*sizeof(void*));
S_EQ(offsetof(Steinberg_Vst_IEditControllerVtbl,getParameterCount),8*sizeof(void*));
S_EQ(offsetof(Steinberg_Vst_IEditControllerVtbl,getParamNormalized),14*sizeof(void*));
S_EQ(offsetof(Steinberg_Vst_IEditControllerVtbl,setParamNormalized),15*sizeof(void*));
S_EQ(offsetof(Steinberg_Vst_IEditControllerVtbl,setComponentHandler),16*sizeof(void*));
S_EQ(offsetof(Steinberg_Vst_IEditControllerVtbl,createView),17*sizeof(void*));
S_EQ(offsetof(Steinberg_Vst_IParameterChangesVtbl,addParameterData),5*sizeof(void*));
S_EQ(offsetof(Steinberg_Vst_IParamValueQueueVtbl,addPoint),6*sizeof(void*));
S_EQ(offsetof(Steinberg_Vst_IEventListVtbl,getEvent),4*sizeof(void*));
static void abi_typecheck(void){
 /* addParameterData accepts a const uint32_t* (ParamID pointer), not an ID
  * passed by value. Compiler checks this is a correct function signature. */
 Steinberg_Vst_IParameterChangesVtbl table={0};
 struct Steinberg_Vst_IParamValueQueue* (*fn)(void*,const Steinberg_Vst_ParamID*,Steinberg_int32*)=table.addParameterData;
 (void)fn;
}
int vst3_cp12_abi_check(void){abi_typecheck();return 0;}
