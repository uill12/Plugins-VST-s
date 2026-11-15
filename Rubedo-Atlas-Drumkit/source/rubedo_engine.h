#pragma once
#include <stdint.h>
#define RUBEDO_KITS 24
#define RUBEDO_PADS 24
#define RUBEDO_VARS 4
#define RUBEDO_VOICES 64
#define RUBEDO_PARAMS 34
#define RUBEDO_RATE 96000.0

typedef struct RubedoRegion {uint32_t offset,frames;} RubedoRegion;
typedef struct RubedoBank {const uint8_t *data;uint32_t bytes;RubedoRegion regions[RUBEDO_KITS*RUBEDO_PADS*RUBEDO_VARS];}RubedoBank;
typedef struct RubedoVoice {double position,step;float amplitude,last_l,last_r;uint32_t kit,pad,variation;uint8_t active,sinc_band;}RubedoVoice;
typedef struct RubedoEngine {RubedoVoice voices[RUBEDO_VOICES];double params[RUBEDO_PARAMS];int16_t mappings[RUBEDO_PADS];uint8_t rr[RUBEDO_PADS];uint32_t note_serial;uint32_t kit;double sample_rate;double gain;uint16_t steps[RUBEDO_PADS];uint32_t step_index;double step_clock;double bpm;uint8_t playing;int16_t midi_learn_target;uint32_t ui_last_note;float pitch[RUBEDO_PADS],decay_shape[RUBEDO_PADS],fm_depth[RUBEDO_PADS];float swing,scatter,chance;float tone_memory,room_l,room_r;uint32_t room_idx;float delay_l[4096],delay_r[4096];}RubedoEngine;
typedef struct RubedoExtraState {
 float pitch[24],decay_shape[24],fm_depth[24];
 float swing,scatter,chance;uint16_t steps[24];double bpm;
}RubedoExtraState;
void rubedo_export_extra(RubedoEngine*,RubedoExtraState*);
void rubedo_import_extra(RubedoEngine*,const RubedoExtraState*);
extern RubedoBank rubedo_bank;
extern const char *rubedo_kit_names[24];
extern const char *rubedo_pad_names[24];
int rubedo_load_bank(void);
void rubedo_unload_bank(void);
void rubedo_init_engine(RubedoEngine *e);
void rubedo_activate(RubedoEngine *e,double sr);
void rubedo_note_on(RubedoEngine *e,int note,double velocity);
void rubedo_panic(RubedoEngine *e);
void rubedo_set_param(RubedoEngine *e,unsigned id,double value);
void rubedo_render(RubedoEngine *e,float *l,float*r,double*ld,double*rd,uint32_t frames);