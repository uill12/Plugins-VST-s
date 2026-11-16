// Minimal ABI declarations corresponding to the official free-audio/CLAP C API v1.2.
// Copyright portions described by the CLAP project are MIT-licensed.
// https://github.com/free-audio/clap (refer to the official header files for documentation).
// No custom binary format is substituted for CLAP. Types and layout mirror the published ABI.
#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef _WIN32
# define CLAP_API __cdecl
# define CLAP_EXPORT __declspec(dllexport)
#else
# define CLAP_API
# define CLAP_EXPORT __attribute__((visibility("default")))
#endif
typedef uint32_t clap_id;
static constexpr clap_id CLAP_INVALID_ID = 0xffffffffu;
struct clap_version {uint32_t major,minor,revision;};
struct clap_plugin;
struct clap_host;
struct clap_plugin_factory;
struct clap_input_events;
struct clap_output_events;
struct clap_process;
struct clap_istream;
struct clap_ostream;
typedef struct clap_plugin clap_plugin_t;
typedef struct clap_host clap_host_t;
struct clap_plugin_descriptor {
 struct clap_version clap_version; const char *id,*name,*vendor,*url,*manual_url,*support_url,*version,*description;const char *const *features;
};
struct clap_plugin {
 const clap_plugin_descriptor* desc;void* plugin_data;
 bool(CLAP_API *init)(const clap_plugin*);
 void(CLAP_API *destroy)(const clap_plugin*);
 bool(CLAP_API *activate)(const clap_plugin*,double,uint32_t,uint32_t);
 void(CLAP_API *deactivate)(const clap_plugin*);
 bool(CLAP_API *start_processing)(const clap_plugin*);
 void(CLAP_API *stop_processing)(const clap_plugin*);
 void(CLAP_API *reset)(const clap_plugin*);
 int32_t(CLAP_API *process)(const clap_plugin*,const clap_process*);
 const void*(CLAP_API *get_extension)(const clap_plugin*,const char*);
 void(CLAP_API *on_main_thread)(const clap_plugin*);
};
struct clap_host {
 struct clap_version clap_version;void* host_data;const char *name,*vendor,*url,*version;
 const void*(CLAP_API *get_extension)(const clap_host*,const char*);
 void(CLAP_API *request_restart)(const clap_host*);
 void(CLAP_API *request_process)(const clap_host*);
 void(CLAP_API *request_callback)(const clap_host*);
};
struct clap_plugin_factory {
 uint32_t(CLAP_API *get_plugin_count)(const clap_plugin_factory*);
 const clap_plugin_descriptor*(CLAP_API *get_plugin_descriptor)(const clap_plugin_factory*,uint32_t);
 const clap_plugin*(CLAP_API *create_plugin)(const clap_plugin_factory*,const clap_host*,const char*);
};
struct clap_plugin_entry {
 struct clap_version clap_version;
 bool(CLAP_API *init)(const char*);
 void(CLAP_API *deinit)();
 const void*(CLAP_API *get_factory)(const char*);
};
struct clap_event_header {uint32_t size,time;uint16_t space_id,type;uint32_t flags;};
struct clap_event_note {clap_event_header header;int32_t note_id;int16_t port_index,channel,key;double velocity;};
struct clap_event_note_expression {clap_event_header header;int32_t expression_id,note_id;int16_t port_index,channel,key;double value;};
struct clap_event_param_value {clap_event_header header;clap_id param_id;void* cookie;int32_t note_id;int16_t port_index,channel,key;double value;};
struct clap_event_param_mod {clap_event_header header;clap_id param_id;void* cookie;int32_t note_id;int16_t port_index,channel,key;double amount;};
struct clap_event_param_gesture {clap_event_header header;clap_id param_id;};
struct clap_event_midi {clap_event_header header;uint16_t port_index;uint8_t data[3];};
struct clap_input_events {void* ctx;uint32_t(CLAP_API *size)(const clap_input_events*);const clap_event_header*(CLAP_API *get)(const clap_input_events*,uint32_t);};
struct clap_output_events {void* ctx;bool(CLAP_API *try_push)(const clap_output_events*,const clap_event_header*);};
struct clap_audio_buffer {float** data32;double** data64;uint32_t channel_count,latency;uint64_t constant_mask;};
struct clap_process {int64_t steady_time;uint32_t frames_count;const void* transport;const clap_audio_buffer* audio_inputs;clap_audio_buffer* audio_outputs;uint32_t audio_inputs_count,audio_outputs_count;const clap_input_events* in_events;const clap_output_events* out_events;};
struct clap_param_info {clap_id id,flags;void* cookie;char name[256];char module[1024];double min_value,max_value,default_value;};
struct clap_plugin_params {
 uint32_t(CLAP_API *count)(const clap_plugin*);
 bool(CLAP_API *get_info)(const clap_plugin*,uint32_t,clap_param_info*);
 bool(CLAP_API *get_value)(const clap_plugin*,clap_id,double*);
 bool(CLAP_API *value_to_text)(const clap_plugin*,clap_id,double,char*,uint32_t);
 bool(CLAP_API *text_to_value)(const clap_plugin*,clap_id,const char*,double*);
 void(CLAP_API *flush)(const clap_plugin*,const clap_input_events*,const clap_output_events*);
};
struct clap_host_params {void(CLAP_API *rescan)(const clap_host*,uint32_t);void(CLAP_API *clear)(const clap_host*,clap_id,uint32_t);void(CLAP_API *request_flush)(const clap_host*);};
struct clap_audio_port_info {clap_id id;char name[256];uint32_t flags,channel_count;const char* port_type;clap_id in_place_pair;};
struct clap_plugin_audio_ports {uint32_t(CLAP_API *count)(const clap_plugin*,bool);bool(CLAP_API *get)(const clap_plugin*,uint32_t,bool,clap_audio_port_info*);};
struct clap_note_port_info {clap_id id,supported_dialects,preferred_dialect;char name[256];};
struct clap_plugin_note_ports {uint32_t(CLAP_API *count)(const clap_plugin*,bool);bool(CLAP_API *get)(const clap_plugin*,uint32_t,bool,clap_note_port_info*);};
struct clap_istream {void* ctx;int64_t(CLAP_API *read)(const clap_istream*,void*,uint64_t);};
struct clap_ostream {void* ctx;int64_t(CLAP_API *write)(const clap_ostream*,const void*,uint64_t);};
struct clap_plugin_latency {uint32_t(CLAP_API *get)(const clap_plugin*);};
struct clap_plugin_state {bool(CLAP_API *save)(const clap_plugin*,const clap_ostream*);bool(CLAP_API *load)(const clap_plugin*,const clap_istream*);};
struct clap_remote_controls_page {char section_name[256];clap_id page_id;char page_name[256];clap_id param_ids[8];bool is_for_preset;};
struct clap_plugin_remote_controls {uint32_t(CLAP_API *count)(const clap_plugin*);bool(CLAP_API *get)(const clap_plugin*,uint32_t,clap_remote_controls_page*);};
struct clap_host_remote_controls {void(CLAP_API *changed)(const clap_host*);void(CLAP_API *suggest_page)(const clap_host*,clap_id);};
struct clap_voice_info {uint32_t voice_count,voice_capacity;uint64_t flags;};
struct clap_plugin_voice_info {bool(CLAP_API *get)(const clap_plugin*,clap_voice_info*);};
struct clap_window {const char* api;union {void* win32;void* cocoa;unsigned long x11;void* ptr;};};
struct clap_gui_resize_hints {bool can_resize_horizontally,can_resize_vertically,preserve_aspect_ratio;uint32_t aspect_ratio_width,aspect_ratio_height;};
struct clap_plugin_gui {
 bool(CLAP_API *is_api_supported)(const clap_plugin*,const char*,bool);
 bool(CLAP_API *get_preferred_api)(const clap_plugin*,const char**,bool*);
 bool(CLAP_API *create)(const clap_plugin*,const char*,bool);
 void(CLAP_API *destroy)(const clap_plugin*);
 bool(CLAP_API *set_scale)(const clap_plugin*,double);
 bool(CLAP_API *get_size)(const clap_plugin*,uint32_t*,uint32_t*);
 bool(CLAP_API *can_resize)(const clap_plugin*);
 bool(CLAP_API *get_resize_hints)(const clap_plugin*,clap_gui_resize_hints*);
 bool(CLAP_API *adjust_size)(const clap_plugin*,uint32_t*,uint32_t*);
 bool(CLAP_API *set_size)(const clap_plugin*,uint32_t,uint32_t);
 bool(CLAP_API *set_parent)(const clap_plugin*,const clap_window*);
 bool(CLAP_API *set_transient)(const clap_plugin*,const clap_window*);
 void(CLAP_API *suggest_title)(const clap_plugin*,const char*);
 bool(CLAP_API *show)(const clap_plugin*);
 bool(CLAP_API *hide)(const clap_plugin*);
};
struct clap_plugin_preset_load {bool(CLAP_API *from_location)(const clap_plugin*,uint32_t,const char*,const char*);};
static constexpr uint32_t CLAP_PARAM_IS_STEPPED=1u<<0,CLAP_PARAM_IS_AUTOMATABLE=1u<<5,CLAP_PARAM_IS_MODULATABLE=1u<<10,
 CLAP_PARAM_IS_MODULATABLE_PER_NOTE_ID=1u<<11,CLAP_PARAM_IS_MODULATABLE_PER_KEY=1u<<12,
 CLAP_PARAM_IS_MODULATABLE_PER_CHANNEL=1u<<13,CLAP_PARAM_REQUIRES_PROCESS=1u<<15;
static constexpr uint16_t CLAP_EVENT_NOTE_ON=0,CLAP_EVENT_NOTE_OFF=1,CLAP_EVENT_NOTE_CHOKE=2,CLAP_EVENT_NOTE_END=3,CLAP_EVENT_NOTE_EXPRESSION=4,
 CLAP_EVENT_PARAM_VALUE=5,CLAP_EVENT_PARAM_MOD=6,CLAP_EVENT_PARAM_GESTURE_BEGIN=7,CLAP_EVENT_PARAM_GESTURE_END=8,CLAP_EVENT_MIDI=10;
static constexpr int32_t CLAP_PROCESS_CONTINUE=1;
