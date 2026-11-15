#pragma once
#include <stdint.h>
#include "rubedo_engine.h"
#define RB_GUI_WIDTH 1512
#define RB_GUI_HEIGHT 1009
typedef struct RBGui {
 void *hwnd;
 RubedoEngine *engine;
 void *callback_context;
 void (*kit_changed)(void*,double);void (*param_changed)(void*,unsigned,double);uint8_t learn_active;
 uint32_t kit;
 uint32_t bank;
 uint32_t selected;
 uint32_t width,height;int dragging,drag_param,drag_y,hover_control;uint32_t changed_tick;uint8_t force,mode,routing;uint32_t last_tap_ms;uint32_t input_events;uint32_t last_input_ms;uint8_t editor_only,audition_serial;uint32_t ui_seen_seq,learn_seen_seq,unmapped_seen_seq,flash_until,learn_notice_until,unmapped_notice_until,routing_hint_until;
 int flash_pad;uint32_t pad_flash_until[24];uint8_t learn_notice;int unmapped_note;
 void *f_num,*f_name,*f_role,*f_midi,*f_title,*f_small,*f_kit;uint32_t font_height;
} RBGui;
int rb_gui_attach(RBGui*gui, void*host_parent, RubedoEngine*engine, void*context, void(*kit_changed)(void*,double));
void rb_gui_detach(RBGui*gui);
void rb_gui_show(RBGui*gui,int visible);
void rb_gui_size(RBGui*gui,uint32_t w,uint32_t h);
int rb_gui_asset_ready(void);