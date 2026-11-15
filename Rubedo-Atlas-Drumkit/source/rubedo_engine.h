#pragma once
#include <stdint.h>
#include <stdatomic.h>
#define RUBEDO_KITS 24
#define RUBEDO_PADS 24
#define RUBEDO_VARS 4
#define RUBEDO_VOICES 64
#define RUBEDO_PARAMS 34
#define RUBEDO_RATE 96000.0

typedef struct RubedoRegion {uint32_t offset,frames;} RubedoRegion;
typedef struct RubedoBank {const uint8_t *data;uint32_t bytes;RubedoRegion regions[RUBEDO_KITS*RUBEDO_PADS*RUBEDO_VARS];}RubedoBank;
typedef struct RubedoVoice {double position,step;float amplitude,last_l,last_r;uint32_t kit,pad,variation;uint8_t active,sinc_band;}RubedoVoice;
typedef struct RubedoEngine {RubedoVoice voices[RUBEDO_VOICES];double params[RUBEDO_PARAMS];atomic_int mappings[RUBEDO_PADS];uint8_t rr[RUBEDO_PADS];uint32_t note_serial;uint32_t kit;double sample_rate;double gain;uint16_t steps[RUBEDO_PADS];uint32_t step_index;double step_clock;double bpm;uint8_t playing;atomic_int midi_learn_target;uint32_t ui_last_note;atomic_uint ui_note_seq;atomic_int ui_last_pad;atomic_uint ui_pending_mask;atomic_uint ui_learn_seq;atomic_int ui_learn_note;atomic_uint host_mapping_dirty;atomic_int ui_unmapped_note;atomic_uint ui_unmapped_seq;float pitch[RUBEDO_PADS],decay_shape[RUBEDO_PADS],fm_depth[RUBEDO_PADS];float pad_gain[RUBEDO_PADS],pad_attack[RUBEDO_PADS],pad_release[RUBEDO_PADS];uint8_t decay_active[RUBEDO_PADS],fm_active[RUBEDO_PADS];uint16_t patterns[4][RUBEDO_PADS];uint8_t variation;atomic_uint audition_head,audition_tail;uint32_t audition_queue[64];float swing,scatter,chance;float tone_memory,room_l,room_r;uint32_t room_idx;float delay_l[4096],delay_r[4096];}RubedoEngine;
typedef struct RubedoExtraState {
 float pitch[24],decay_shape[24],fm_depth[24];
 float swing,scatter,chance;uint16_t steps[24];double bpm;
}RubedoExtraState;
typedef struct RubedoExtraStateV3 {float gain[24],attack[24],release[24];uint16_t patterns[4][24];uint8_t variation;uint8_t decay_active[24],fm_active[24];uint8_t reserved[3];}RubedoExtraStateV3;
void rubedo_export_extra(RubedoEngine*,RubedoExtraState*);
void rubedo_import_extra(RubedoEngine*,const RubedoExtraState*);
void rubedo_export_extra_v3(RubedoEngine*,RubedoExtraStateV3*);
void rubedo_import_extra_v3(RubedoEngine*,const RubedoExtraStateV3*);
void rubedo_select_variation(RubedoEngine*,unsigned variation);
void rubedo_pattern_changed(RubedoEngine*);
void rubedo_trigger_pad(RubedoEngine*,int pad,double velocity);
void rubedo_request_audition(RubedoEngine*,int pad,double velocity);
extern RubedoBank rubedo_bank;
extern const char *rubedo_kit_names[24];
extern const char *rubedo_pad_names[24];
int rubedo_load_bank(void);
void rubedo_unload_bank(void);
void rubedo_init_engine(RubedoEngine *e);
void rubedo_activate(RubedoEngine *e,double sr);
int rubedo_note_to_pad(const RubedoEngine *e,int note);
int rubedo_assign_note(RubedoEngine *e,int pad,int note);
void rubedo_import_mappings(RubedoEngine*e,const int16_t notes[RUBEDO_PADS]);
void rubedo_note_on(RubedoEngine *e,int note,double velocity);
void rubedo_panic(RubedoEngine *e);
void rubedo_set_param(RubedoEngine *e,unsigned id,double value);
void rubedo_render(RubedoEngine *e,float *l,float*r,double*ld,double*rd,uint32_t frames);