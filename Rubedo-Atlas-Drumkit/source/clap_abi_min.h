// Minimum CLAP 1.2 stable ABI declarations used by Rubedo.
// Compatibility shim, not a redistributed CLAP SDK; consult https://github.com/free-audio/clap
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define CLAP_EXT_AUDIO_PORTS "clap.audio-ports"
#define CLAP_EXT_NOTE_PORTS "clap.note-ports"
#define CLAP_EXT_PARAMS "clap.params"
#define CLAP_EXT_STATE "clap.state"
#define CLAP_EXT_GUI "clap.gui"
#define CLAP_PLUGIN_FACTORY_ID "clap.plugin-factory"
#define CLAP_INVALID_ID 0xffffffffu
#define CLAP_CORE_EVENT_SPACE_ID 0
#define CLAP_EVENT_NOTE_ON 0
#define CLAP_EVENT_NOTE_OFF 1
#define CLAP_EVENT_PARAM_VALUE 5
#define CLAP_EVENT_MIDI 10
#define CLAP_PORT_IS_MAIN 1u
#define CLAP_AUDIO_PORT_IS_MAIN 1u
#define CLAP_AUDIO_PORT_SUPPORTS_64BITS 2u
#define CLAP_NOTE_DIALECT_CLAP 1
#define CLAP_NOTE_DIALECT_MIDI 2
#define CLAP_PARAM_IS_STEPPED 1u
#define CLAP_PARAM_IS_AUTOMATABLE 32u
#define CLAP_PROCESS_CONTINUE 0
#define CLAP_PROCESS_CONTINUE_IF_NOT_QUIET 1
#define CLAP_PROCESS_TAIL 2
#define CLAP_PROCESS_SLEEP 3
#define CLAP_NAME_SIZE 256
#define CLAP_PATH_SIZE 1024
#define CLAP_VERSION_MAJOR 1
#define CLAP_VERSION_MINOR 2
#define CLAP_VERSION_REVISION 0
#ifdef _WIN32
#define CLAP_EXPORT __declspec(dllexport)
#else
#define CLAP_EXPORT __attribute__((visibility("default")))
#endif
typedef uint32_t clap_id;
typedef struct clap_version { uint32_t major, minor, revision; } clap_version_t;
typedef struct clap_plugin_descriptor {
  clap_version_t clap_version; const char *id,*name,*vendor,*url,*manual_url,*support_url,*version,*description; const char *const *features;
} clap_plugin_descriptor_t;
typedef struct clap_event_header { uint32_t size,time; uint16_t space_id,type; uint32_t flags; } clap_event_header_t;
typedef struct clap_event_note {clap_event_header_t header; int32_t note_id; int16_t port_index,channel,key; double velocity;} clap_event_note_t;
typedef struct clap_event_midi {clap_event_header_t header; uint16_t port_index; uint8_t data[3];} clap_event_midi_t;
typedef struct clap_event_param_value {clap_event_header_t header; clap_id param_id; void *cookie; int32_t note_id; int16_t port_index,channel,key; double value;} clap_event_param_value_t;
typedef struct clap_input_events {void *ctx;uint32_t (*size)(const struct clap_input_events*);const clap_event_header_t*(*get)(const struct clap_input_events*,uint32_t);} clap_input_events_t;
typedef struct clap_output_events {void *ctx;bool (*try_push)(const struct clap_output_events*, const clap_event_header_t*);} clap_output_events_t;
typedef struct clap_audio_buffer {float **data32; double **data64;uint32_t channel_count,latency;uint64_t constant_mask;} clap_audio_buffer_t;
typedef struct clap_process {int64_t steady_time;uint32_t frames_count;const void *transport;const clap_audio_buffer_t *audio_inputs;clap_audio_buffer_t *audio_outputs;uint32_t audio_inputs_count,audio_outputs_count;const clap_input_events_t *in_events;const clap_output_events_t *out_events;} clap_process_t;
typedef struct clap_host clap_host_t;
typedef struct clap_plugin {const clap_plugin_descriptor_t *desc;void *plugin_data;bool(*init)(const struct clap_plugin*);void(*destroy)(const struct clap_plugin*);bool(*activate)(const struct clap_plugin*,double,uint32_t,uint32_t);void(*deactivate)(const struct clap_plugin*);bool(*start_processing)(const struct clap_plugin*);void(*stop_processing)(const struct clap_plugin*);void(*reset)(const struct clap_plugin*);int32_t(*process)(const struct clap_plugin*,const clap_process_t*);const void*(*get_extension)(const struct clap_plugin*,const char*);void(*on_main_thread)(const struct clap_plugin*);} clap_plugin_t;
typedef struct clap_plugin_factory {uint32_t(*get_plugin_count)(const struct clap_plugin_factory*);const clap_plugin_descriptor_t*(*get_plugin_descriptor)(const struct clap_plugin_factory*,uint32_t);const clap_plugin_t*(*create_plugin)(const struct clap_plugin_factory*,const clap_host_t*, const char*);} clap_plugin_factory_t;
typedef struct clap_plugin_entry {clap_version_t clap_version;bool(*init)(const char* plugin_path);void(*deinit)(void);const void*(*get_factory)(const char*factory_id);} clap_plugin_entry_t;
typedef struct clap_audio_port_info {clap_id id;char name[256];uint32_t flags,channel_count;const char *port_type;clap_id in_place_pair;}clap_audio_port_info_t;
typedef struct clap_plugin_audio_ports {uint32_t(*count)(const clap_plugin_t*,bool);bool(*get)(const clap_plugin_t*,uint32_t,bool,clap_audio_port_info_t*);}clap_plugin_audio_ports_t;
typedef struct clap_note_port_info {clap_id id;char name[256];uint32_t supported_dialects,preferred_dialect;}clap_note_port_info_t;
typedef struct clap_plugin_note_ports {uint32_t(*count)(const clap_plugin_t*,bool);bool(*get)(const clap_plugin_t*,uint32_t,bool,clap_note_port_info_t*);}clap_plugin_note_ports_t;
typedef struct clap_param_info {clap_id id;uint32_t flags;void *cookie;char name[256];char module[1024];double min_value,max_value,default_value;}clap_param_info_t;
typedef struct clap_plugin_params {uint32_t(*count)(const clap_plugin_t*);bool(*get_info)(const clap_plugin_t*,uint32_t,clap_param_info_t*);bool(*get_value)(const clap_plugin_t*,clap_id,double*);bool(*value_to_text)(const clap_plugin_t*,clap_id,double,char*,uint32_t);bool(*text_to_value)(const clap_plugin_t*,clap_id,const char*,double*);void(*flush)(const clap_plugin_t*,const clap_input_events_t*,const clap_output_events_t*);}clap_plugin_params_t;
typedef struct clap_ostream {void *ctx;int64_t(*write)(const struct clap_ostream*,const void*,uint64_t);}clap_ostream_t;
typedef struct clap_istream {void *ctx;int64_t(*read)(const struct clap_istream*,void*,uint64_t);}clap_istream_t;
typedef struct clap_plugin_state {bool(*save)(const clap_plugin_t*,const clap_ostream_t*);bool(*load)(const clap_plugin_t*,const clap_istream_t*);}clap_plugin_state_t;
#ifdef __cplusplus
}
#endif