// CP12R1 verified ABI shim. Must pass the CI comparison against the official Steinberg VST3 C API; no SDK code bundled.
#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef _WIN32
#define VEXPORT __declspec(dllexport)
#else
#define VEXPORT __attribute__((visibility("default")))
#endif
typedef uint8_t UID[16];
typedef uint16_t Char16;
typedef int32_t TRESULT;
#define V_OK 0
#define V_FALSE 1
#define V_NOINTERFACE (-1)
#define V_NOTIMPLEMENTED (-3)

typedef struct VFactoryInfo {char vendor[64];char url[256];char email[128];int32_t flags;}VFactoryInfo;
typedef struct VClassInfo {UID cid;int32_t cardinality;char category[32];char name[64];}VClassInfo;
typedef struct VClassInfo2 {UID cid;int32_t cardinality;char category[32];char name[64];uint32_t classFlags;char subCategories[128];char vendor[64];char version[64];char sdkVersion[64];}VClassInfo2;
typedef struct VClassInfoW {UID cid;int32_t cardinality;char category[32];Char16 name[64];uint32_t classFlags;char subCategories[128];Char16 vendor[64];Char16 version[64];Char16 sdkVersion[64];}VClassInfoW;
typedef struct VBusInfo {int32_t mediaType,dir,channelCount;Char16 name[128];int32_t busType;uint32_t flags;}VBusInfo;
typedef struct VRoutingInfo {int32_t mediaType,busIndex,channel;}VRoutingInfo;
typedef struct VProcessSetup {int32_t processMode,symbolicSampleSize,maxSamplesPerBlock;double sampleRate;}VProcessSetup;
typedef struct VAudioBusBuffers {int32_t numChannels;uint64_t silenceFlags;union{float**channelBuffers32;double**channelBuffers64;};}VAudioBusBuffers;
typedef struct VEventNoteOn {int16_t channel,pitch;float tuning,velocity;int32_t length,noteId;}VEventNoteOn;
typedef struct VEvent {int32_t busIndex,sampleOffset;double ppqPosition;uint16_t flags,type;union{VEventNoteOn noteOn;uint8_t raw[96];uint64_t _vst3_event_align8;}; /* Official VST3 Event union is 8-byte aligned. */}VEvent;
typedef struct VEventList {const void**vt;}VEventList;
typedef struct VParamChanges {const void**vt;}VParamChanges;
typedef struct VParamQueue {const void**vt;}VParamQueue;
typedef struct VProcessData {int32_t processMode,symbolicSampleSize,numSamples,numInputs,numOutputs;VAudioBusBuffers*inputs,*outputs;VParamChanges*inputParameterChanges,*outputParameterChanges;VEventList*inputEvents,*outputEvents;void*processContext;}VProcessData;
typedef struct VParameterInfo {uint32_t id;Char16 title[128],shortTitle[128],units[128];int32_t stepCount;double defaultNormalizedValue;int32_t unitId,flags;}VParameterInfo;
typedef struct VBStream {const void**vt;}VBStream;