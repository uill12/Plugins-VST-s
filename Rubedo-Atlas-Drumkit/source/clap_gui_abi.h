#pragma once
#include "clap_abi_min.h"
typedef struct clap_window {const char*api;union {void*win32;void*ptr;unsigned long x11;}handle;}clap_window_t;
typedef struct clap_gui_resize_hints {bool can_resize_horizontally,can_resize_vertically,preserve_aspect_ratio;uint32_t aspect_ratio_width,aspect_ratio_height;}clap_gui_resize_hints_t;
typedef struct clap_plugin_gui {
 bool(*is_api_supported)(const clap_plugin_t*,const char*,bool);
 bool(*get_preferred_api)(const clap_plugin_t*,const char**,bool*);
 bool(*create)(const clap_plugin_t*,const char*,bool);
 void(*destroy)(const clap_plugin_t*);
 bool(*set_scale)(const clap_plugin_t*,double);
 bool(*get_size)(const clap_plugin_t*,uint32_t*,uint32_t*);
 bool(*can_resize)(const clap_plugin_t*);
 bool(*get_resize_hints)(const clap_plugin_t*,clap_gui_resize_hints_t*);
 bool(*adjust_size)(const clap_plugin_t*,uint32_t*,uint32_t*);
 bool(*set_size)(const clap_plugin_t*,uint32_t,uint32_t);
 bool(*set_parent)(const clap_plugin_t*,const clap_window_t*);
 bool(*set_transient)(const clap_plugin_t*,const clap_window_t*);
 void(*suggest_title)(const clap_plugin_t*,const char*);
 bool(*show)(const clap_plugin_t*);
 bool(*hide)(const clap_plugin_t*);
}clap_plugin_gui_t;