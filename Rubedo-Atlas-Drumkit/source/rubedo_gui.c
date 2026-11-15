#include "rubedo_gui.h"
#ifdef _WIN32
#include "win_imports.h"
#include <stdint.h>
#include <stddef.h>
extern void *rubedo_module;
#define W32 __attribute__((ms_abi))
#define WIMP __declspec(dllimport)
typedef intptr_t LRESULT;typedef uintptr_t WPARAM;typedef intptr_t LPARAM;
typedef struct {int32_t left,top,right,bottom;}RECT;
typedef struct {uint32_t cbSize;uint32_t style;void*proc;int32_t clsExtra,wndExtra;void*instance,*icon,*cursor,*background;const char*menu,*className;void*smallIcon;} WNDCLASSEXA;
// The real Win32 CREATESTRUCTA layout begins with lpCreateParams. CP08R1
// incorrectly placed createParams at the end, leaving GWLP_USERDATA invalid.
// This is a 64-bit Win32 ABI structure, not an arbitrary C data record.
typedef struct {
 void *createParams;  // lpCreateParams, offset 0
 void *instance;      // hInstance, offset 8
 void *menu;          // hMenu, offset 16
 void *parent;        // hwndParent, offset 24
 int32_t cy,cx,y,x;   // offsets 32..47
 int32_t style;       // offset 48
 const char *name;    // lpszName, offset 56
 const char *cls;     // lpszClass, offset 64
 uint32_t exStyle;    // dwExStyle, offset 72
} CREATESTRUCTA;
_Static_assert(sizeof(CREATESTRUCTA)==80,"Win64 CREATESTRUCTA size mismatch");
_Static_assert(offsetof(CREATESTRUCTA,createParams)==0,"Win64 CREATESTRUCTA lpCreateParams offset mismatch");
_Static_assert(offsetof(CREATESTRUCTA,style)==48,"Win64 CREATESTRUCTA style offset mismatch");
_Static_assert(offsetof(CREATESTRUCTA,exStyle)==72,"Win64 CREATESTRUCTA dwExStyle offset mismatch");
typedef struct {void*hdc;int erased;RECT rect;int restore,update;uint8_t reserved[32];} PAINTSTRUCT;
typedef struct {uint32_t size;int32_t width,height;uint16_t planes,bits;uint32_t compression,imageSize;int32_t xppu,yppu;uint32_t colorsUsed,important;}BITMAPINFOHEADER;
typedef struct {BITMAPINFOHEADER h;uint32_t colors[1];}BITMAPINFO;
WIMP uint16_t W32 RegisterClassExA(const WNDCLASSEXA*);
WIMP void* W32 CreateWindowExA(uint32_t,const char*,const char*,uint32_t,int,int,int,int,void*,void*,void*,void*);
WIMP int W32 DestroyWindow(void*);
WIMP LRESULT W32 DefWindowProcA(void*,uint32_t,WPARAM,LPARAM);
WIMP intptr_t W32 SetWindowLongPtrA(void*,int,intptr_t);
WIMP intptr_t W32 GetWindowLongPtrA(void*,int);
WIMP int W32 InvalidateRect(void*,const RECT*,int);
WIMP void* W32 BeginPaint(void*,PAINTSTRUCT*);
WIMP int W32 EndPaint(void*,const PAINTSTRUCT*);
WIMP int W32 GetClientRect(void*,RECT*);
WIMP int W32 ShowWindow(void*,int);
WIMP int W32 SetWindowPos(void*,void*,int,int,int,int,uint32_t);
WIMP int W32 StretchDIBits(void*,int,int,int,int,int,int,int,int,const void*,const BITMAPINFO*,uint32_t,uint32_t);
WIMP int W32 SetBkMode(void*,int);
WIMP uint32_t W32 SetTextColor(void*,uint32_t);
WIMP int W32 TextOutA(void*,int,int,const char*,int);
WIMP uint32_t W32 SetBkColor(void*,uint32_t);
WIMP int W32 ExtTextOutA(void*,int,int,uint32_t,const RECT*,const char*,uint32_t,const int*);
WIMP void* W32 CreatePen(int,int,uint32_t);
WIMP void* W32 CreateSolidBrush(uint32_t);
WIMP void* W32 SelectObject(void*,void*);
WIMP int W32 DeleteObject(void*);
WIMP void* W32 GetStockObject(int);
WIMP int W32 Rectangle(void*,int,int,int,int);
WIMP int W32 Ellipse(void*,int,int,int,int);
WIMP int W32 MoveToEx(void*,int,int,void*);
WIMP int W32 LineTo(void*,int,int);
WIMP int W32 SetROP2(void*,int);
WIMP int W32 SetTextAlign(void*,uint32_t);
WIMP uintptr_t W32 SetTimer(void*,uintptr_t,uint32_t,void*);
WIMP int W32 KillTimer(void*,uintptr_t);
WIMP uint32_t W32 GetTickCount(void);
WIMP void* W32 SetCapture(void*);
WIMP int W32 ReleaseCapture(void);
WIMP void* W32 SetFocus(void*);
WIMP int W32 ScreenToClient(void*,void*);

static uint8_t *rb_picture;
static unsigned rb_picture_refs;
static uint32_t rb_u32(const uint8_t*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint64_t rb_u64(const uint8_t*p){return (uint64_t)rb_u32(p)|((uint64_t)rb_u32(p+4)<<32);}
static int rb_read_picture(void){
 if(rb_picture){rb_picture_refs++;return 1;}
 char path[2048];uint32_t n=GetModuleFileNameA((HMODULE)rubedo_module,path,sizeof(path));if(!n||n>=sizeof(path))return 0;
 HANDLE f=CreateFileA(path,0x80000000u,1,0,3,0x80,0);if(f==RUBEDO_INVALID_HANDLE)return 0;
 uint32_t filesize=SetFilePointer(f,0,0,2);if(filesize<32){CloseHandle(f);return 0;}
 uint8_t footer[32];uint32_t got=0;SetFilePointer(f,(int32_t)(filesize-32),0,0);
 if(!ReadFile(f,footer,32,&got,0)||got!=32||footer[0]!='R'||footer[1]!='B'||footer[2]!='O'||footer[3]!='V'){CloseHandle(f);return 0;}
 uint64_t bankstart=rb_u64(footer+8),picsize=rb_u64(footer+24);
 if(picsize!=RB_GUI_WIDTH*RB_GUI_HEIGHT*3u||bankstart<picsize||bankstart>filesize){CloseHandle(f);return 0;}
 uint8_t *im=VirtualAlloc(0,picsize,RUBEDO_MEM_RESERVE|RUBEDO_MEM_COMMIT,RUBEDO_PAGE_READWRITE);if(!im){CloseHandle(f);return 0;}
 SetFilePointer(f,(int32_t)(bankstart-picsize),0,0);
 uint64_t offset=0;while(offset<picsize){uint32_t sz=(uint32_t)(picsize-offset);if(sz>1u<<20)sz=1u<<20;got=0;
 if(!ReadFile(f,im+offset,sz,&got,0)||got!=sz){VirtualFree(im,0,RUBEDO_MEM_RELEASE);CloseHandle(f);return 0;}offset+=got;}
 CloseHandle(f);rb_picture=im;rb_picture_refs=1;return 1;
}
int rb_gui_asset_ready(void){return rb_picture!=0;}
static void rb_drop_picture(void){if(rb_picture&&rb_picture_refs&&!--rb_picture_refs){VirtualFree(rb_picture,0,RUBEDO_MEM_RELEASE);rb_picture=0;}}
// CP04: interactive vector controls rendered OVER the original approved 1512x1009 artwork.
// All coordinates are in approved CSS pixel space and scale with the editor.
static void rb_actual_size(RBGui*g){
 if(!g||!g->hwnd)return;
 RECT rect={0};if(GetClientRect(g->hwnd,&rect)){
  int w=rect.right-rect.left,h=rect.bottom-rect.top;
  if(w>1&&h>1){g->width=(uint32_t)w;g->height=(uint32_t)h;}
 }
}
static int rb_scale_x(RBGui*g,int x){return (int)((int64_t)x*g->width/RB_GUI_WIDTH);}
static int rb_scale_y(RBGui*g,int y){return (int)((int64_t)y*g->height/RB_GUI_HEIGHT);}
static void rb_outline(void*dc,RBGui*g,int x,int y,int w,int h,uint32_t color,int stroke){
 void*pen=CreatePen(0,stroke,color),*old=SelectObject(dc,pen),*brush=SelectObject(dc,GetStockObject(5));
 Rectangle(dc,rb_scale_x(g,x),rb_scale_y(g,y),rb_scale_x(g,x+w),rb_scale_y(g,y+h));
 SelectObject(dc,brush);SelectObject(dc,old);DeleteObject(pen);
}
static void rb_disc(void*dc,RBGui*g,int x,int y,int radius,uint32_t color,int filled){
 void*pen=CreatePen(0,2,color),*old=SelectObject(dc,pen),*brush0=GetStockObject(5),*brush;
 if(filled){brush=CreateSolidBrush(color);}else brush=brush0;
 void*oldbrush=SelectObject(dc,brush);
 Ellipse(dc,rb_scale_x(g,x-radius),rb_scale_y(g,y-radius),rb_scale_x(g,x+radius),rb_scale_y(g,y+radius));
 SelectObject(dc,oldbrush);SelectObject(dc,old);DeleteObject(pen);if(filled)DeleteObject(brush);
}
static void rb_str(void*dc,RBGui*g,int x,int y,const char*label,uint32_t color,int bgwidth){
 int n=0;while(label[n]&&n<99)n++;
 SetBkMode(dc,1);SetTextColor(dc,color);
 if(bgwidth){RECT box={rb_scale_x(g,x-3),rb_scale_y(g,y-2),rb_scale_x(g,x+bgwidth),rb_scale_y(g,y+17)};
 SetBkColor(dc,0x00e6d9be);ExtTextOutA(dc,rb_scale_x(g,x),rb_scale_y(g,y),2,&box,label,n,0);
 }else TextOutA(dc,rb_scale_x(g,x),rb_scale_y(g,y),label,n);
}
static void rb_uint(char*dst,unsigned n){char t[16];int p=0,j=0;do{t[p++]=(char)('0'+n%10);n/=10;}while(n&&p<15);while(p)dst[j++]=t[--p];dst[j]=0;}
static int rb_get_knob_id(int x,int y){
 const int xx[4][4]={{1117,1223,1329,1435},{1117,1223,1329,1435},
                       {1117,1223,1329,1435},{1247,1317,1386,1455}};
 const int yy[4]={303,412,678,867};
 for(int r=0;r<4;r++)for(int c=0;c<4;c++){
  int dx=x-xx[r][c],dy=y-yy[r];if(dx*dx+dy*dy<=33*33)return r*4+c;
 }
 return -1;
}
static int rb_knob_param(RBGui*g,int id){
 // Native engine has 10 global parameters and 24 per-voice levels. Do not pretend
 // pitch/filter/FM controls are available unless native DSP actually implements them.
 static const int idmap[16]={-1,-1,2,-1,6,4,9,-1,1,1,6,7,-1,-1,8,-1};
 if(id<0||id>=16)return -1;
 if(id==7)return 10+(int)g->selected;
 return idmap[id];
}
static void rb_set_knob(RBGui*g,int id,double value){
 if(!g||!g->engine)return;int param=rb_knob_param(g,id);
 if(value<0)value=0;if(value>1)value=1;
 if(id==0)g->engine->pitch[g->selected]=(float)value;
 else if(id==1)g->engine->decay_shape[g->selected]=(float)value;
 else if(id==3)g->engine->fm_depth[g->selected]=(float)value;
 else if(id==12)g->engine->swing=(float)value;
 else if(id==13)g->engine->scatter=(float)value;
 else if(id==15)g->engine->chance=(float)value;
 else if(param>=0)rubedo_set_param(g->engine,(unsigned)param,value);
 if(g->param_changed){
  if(param>=0)g->param_changed(g->callback_context,(unsigned)param,value);
  else if(id==0)g->param_changed(g->callback_context,60+g->selected,value);
  else if(id==1)g->param_changed(g->callback_context,84+g->selected,value);
  else if(id==3)g->param_changed(g->callback_context,108+g->selected,value);
  else if(id==12)g->param_changed(g->callback_context,132,value);
  else if(id==13)g->param_changed(g->callback_context,133,value);
  else if(id==15)g->param_changed(g->callback_context,134,value);
 }
 g->hover_control=id;g->changed_tick++;
 InvalidateRect(g->hwnd,0,0);
}
static void rb_show_overlay(void*hdc,RBGui*g){
 if(!g)return;
 RubedoEngine*e=g->engine;
 const uint32_t gold=0x00619bb7,deep=0x00262f29,green=0x003a6552;
 // Current kit name (replaces the static first-kit title with repaint-safe background).
 if(g->kit){const char*name=rubedo_kit_names[g->kit];rb_str(hdc,g,75,122,name,deep,324);}
 // In-host diagnostic: visibly changes with native mouse and wheel events.
 // This is drawn in WM_PAINT, not part of the baked screenshot.
 if(g->input_events){
  char line[48]="INPUT OK #";char number[18];rb_uint(number,g->input_events);
  int pos=10;for(int z=0;number[z]&&pos<42;z++)line[pos++]=number[z];line[pos]=0;
  rb_outline(hdc,g,1276,8,222,69,gold,2);
  rb_str(hdc,g,1286,12,line,green,180);
  if(g->editor_only)rb_str(hdc,g,1286,33,"VST3: HOST PARAM MODE",deep,194);
  else rb_str(hdc,g,1286,33,"LIVE ENGINE LINKED",deep,194);
 }
 // Four active transformation forces and four modal choices.
 static const int fx[4]={756,622,893,757};static const int fy[4]={333,442,443,557};
 rb_disc(hdc,g,fx[g->force],fy[g->force],36,gold,0);
 static const int mode_x[4]={614,685,751,836};static const int mode_w[4]={62,58,77,64};
 rb_outline(hdc,g,mode_x[g->mode],696,mode_w[g->mode],26,gold,2);
 if(g->routing)rb_outline(hdc,g,1403,739,87,29,green,3);
 // Selected bank tabs and chosen kit family.
 rb_outline(hdc,g,g->bank?93:24,218,g->bank?83:65,29,gold,2);
 const int families[4]={5,11,17,23};int family=0;while(family<3&&g->kit>(unsigned)families[family])family++;
 static const int family_x[4]={606,692,770,858};static const int family_w[4]={77,69,79,109};
 rb_outline(hdc,g,family_x[family],121,family_w[family],27,gold,2);
 // Clearly visible selection ring; highlight follows pad clicks and MIDI note input.
 int bankstart=g->bank?16:0;
 for(int j=0;j<(g->bank?8:16);j++){
  int row=j/4,col=j%4,pad=bankstart+j;
  int x=24+col*110,y=256+row*96;
  if((unsigned)pad==g->selected){rb_outline(hdc,g,x,y,101,90,gold,4);rb_disc(hdc,g,x+90,y+11,4,0x0077f2df,1);}
 }
 // Selected instrument name, MIDI note number and voice number (not a fake static label).
 if(e){
  const char *nm=rubedo_pad_names[g->selected];
  rb_str(hdc,g,39,724,nm,deep,300);
  char padtxt[18]="VOICE ",numb[16];rb_uint(numb,g->selected+1);int j=6;for(int k=0;numb[k]&&j<17;k++)padtxt[j++]=numb[k];padtxt[j]=0;
  rb_str(hdc,g,375,724,padtxt,deep,68);
  char maptext[24]="MIDI ";rb_uint(numb,(unsigned)e->mappings[g->selected]);j=5;for(int k=0;numb[k]&&j<22;k++)maptext[j++]=numb[k];maptext[j]=0;
  rb_str(hdc,g,366,745,maptext,deep,78);
 }
 if(g->editor_only)rb_str(hdc,g,38,650,"VST3: MIDI NOTE FOR AUDIO",0x00262f29,250);
 if(g->learn_active)rb_str(hdc,g,275,655,g->editor_only?"VST3: USE HOST MIDI MAP":"LEARN: HIT MIDI NOTE",0x00003ba8,190);
 // Sequencer progress, enabled beats and active transport highlighting.
 if(e){
  for(int step=0;step<16;step++){
   int x=24+step*72;
   if(e->steps[g->selected]&(1u<<step))rb_outline(hdc,g,x,848,67,66,green,3);
   if(e->playing&&(e->step_index+15)%16==(unsigned)step)rb_outline(hdc,g,x+2,849,62,63,gold,4);
  }
  if(e->playing){rb_outline(hdc,g,688,29,48,48,green,3);rb_str(hdc,g,968,41,"PLAYING",green,65);}
  char bpm[16];rb_uint(bpm,(unsigned)(e->bpm+.5));rb_str(hdc,g,815,40,bpm,deep,44);
  for(int k=0;k<16;k++){
   int param=rb_knob_param(g,k);
   int x=(k/4==3?(k%4==0?1247:k%4==1?1317:k%4==2?1386:1455):1117+(k%4)*106),
       y=(k/4==0?303:k/4==1?412:k/4==2?678:867);
   double v=param>=0?e->params[param]:(k==0?e->pitch[g->selected]:k==1?e->decay_shape[g->selected]:k==3?e->fm_depth[g->selected]:k==12?e->swing:k==13?e->scatter:e->chance);
   // Dynamic gold pointer dot, independent of the baked default knob drawing.
   int px=x+(int)((v-.5)*34.),py=y-29;
   rb_disc(hdc,g,px,py,3,gold,1);
   if(k==g->hover_control){rb_disc(hdc,g,x,y,31,gold,0);
    char line[40]="VALUE ";char digits[16];rb_uint(digits,(unsigned)(v*100+.5));int i=6;for(int z=0;digits[z]&&i<35;z++)line[i++]=digits[z];line[i++]='%';line[i]=0;
    rb_str(hdc,g,x-24,y+39,line,deep,80);
   }
  }
 }
}
static void rb_broadcast(RBGui*g,unsigned id,double normalized){
 if(g&&g->param_changed)g->param_changed(g->callback_context,id,normalized);
}
static void rb_audition(RBGui*g,int pad,double velocity){
 if(!g||!g->engine||pad<0||pad>=24)return;
 // CLAP owns its audio engine, VST3 conveys trigger via host automation.
 if(!g->editor_only)rubedo_note_on(g->engine,g->engine->mappings[pad],velocity);
 // Toggle least-significant trigger bit to distinguish repeated pad hits.
 unsigned code=2u*(unsigned)pad+1u+(g->audition_serial++&1u);
 rb_broadcast(g,135,(double)code/49.0);
}
static void rb_notify_change(RBGui*g){if(g&&g->hwnd)InvalidateRect(g->hwnd,0,0);}
static void rb_click(RBGui*g,int x,int y){if(!g||!g->engine)return;
 rb_actual_size(g);g->input_events++;g->last_input_ms=GetTickCount();
 rb_notify_change(g);
 int x0=(int)((int64_t)x*RB_GUI_WIDTH/g->width),y0=(int)((int64_t)y*RB_GUI_HEIGHT/g->height);
 RubedoEngine*e=g->engine;
 if(y0>=112&&y0<=158&&x0>=409&&x0<=473){g->kit=(uint32_t)((g->kit+(x0>442?1:23))%24);rubedo_set_param(e,0,(double)g->kit);
  if(g->kit_changed)g->kit_changed(g->callback_context,(double)g->kit/23.);rb_notify_change(g);return;}
 if(y0>=119&&y0<=152&&x0>=544&&x0<=603){g->kit=0;rubedo_set_param(e,0,0);
  if(g->kit_changed)g->kit_changed(g->callback_context,0.);rb_notify_change(g);return;}
 // Family filter chooses first preset of the given family.
 if(y0>119&&y0<153&&x0>=605&&x0<966){int k=x0<690?0:x0<767?6:x0<857?12:18;
  g->kit=k;rubedo_set_param(e,0,k);if(g->kit_changed)g->kit_changed(g->callback_context,(double)k/23.);rb_notify_change(g);return;}
 if(y0>=217&&y0<=252&&x0>=20&&x0<=187){g->bank=x0>=94?1:0;g->selected=g->bank?16:0;rb_notify_change(g);return;}
 if(y0>=256&&y0<637&&x0>=24&&x0<452){int c=(x0-24)/110,r=(y0-256)/96;if(c<0||c>3||r<0||r>3)return;
  int pad=r*4+c+g->bank*16;if(pad>23)return;g->selected=pad;
  rb_audition(g,pad,.88);rb_notify_change(g);return;}
 if(x0>=38&&x0<170&&y0>=739&&y0<=775){
  g->selected=(g->selected+1)%24;g->bank=g->selected>=16?1:0;rb_notify_change(g);return;}
 if(x0>=345&&x0<452&&y0>=218&&y0<=252){
  g->learn_active=1;if(!g->editor_only)e->midi_learn_target=(int16_t)g->selected;
  rb_notify_change(g);return;}
 if(x0>=345&&x0<452&&y0>=639&&y0<=674){g->learn_active=1;if(!g->editor_only)e->midi_learn_target=(int16_t)g->selected;rb_notify_change(g);return;}
 if(y0>695&&y0<726&&x0>=611&&x0<899){
  g->mode=(uint8_t)(x0<678?0:x0<748?1:x0<834?2:3);
  static const double tones[4]={.5,.6,.8,.35};
  rubedo_set_param(e,2,tones[g->mode]);rb_broadcast(g,2,tones[g->mode]);rb_notify_change(g);return;}
 if(y0>293&&y0<594&&x0>=580&&x0<=927){
  int force=-1;
  if(x0>710&&x0<806&&y0<381)force=0;
  else if(x0>580&&x0<666&&y0>387&&y0<494)force=1;
  else if(x0>849&&x0<940&&y0>384&&y0<493)force=2;
  else if(x0>713&&x0<800&&y0>515&&y0<602)force=3;
  if(force>=0){g->force=(uint8_t)force;
   rubedo_set_param(e,2,force==2?.83:force==0?.50:force==3?.66:.42);
   rubedo_set_param(e,6,force==1?.74:.5);
   rb_broadcast(g,2,e->params[2]);rb_broadcast(g,6,e->params[6]);rb_notify_change(g);return;}
 }
 if(y0>=739&&y0<772&&x0>=1402&&x0<=1494){g->routing^=1;
  rubedo_set_param(e,5,g->routing?.85:.5);rb_broadcast(g,5,e->params[5]);rb_notify_change(g);return;}
 if(x0>=706&&x0<815&&y0>387&&y0<513){rb_audition(g,(int)g->selected,.95);rb_notify_change(g);return;}
 if(y0>20&&y0<80&&x0>=686&&x0<740){e->playing^=1;e->step_clock=0;e->step_index=0;rb_broadcast(g,58,e->playing?1.:0.);rb_notify_change(g);return;}
 if(y0>=29&&y0<=74&&x0>=919&&x0<=955){
  uint32_t now=GetTickCount();
  if(g->last_tap_ms&&now>g->last_tap_ms){uint32_t delta=now-g->last_tap_ms;
   if(delta>=240&&delta<=1500)e->bpm=60000.0/(double)delta;}
  g->last_tap_ms=now;rb_broadcast(g,59,(e->bpm-40.)/210.);rb_notify_change(g);return;}
 if(y0>20&&y0<80&&x0>=956&&x0<1030){e->playing=0;e->step_index=0;e->step_clock=0;rb_broadcast(g,58,0.);rb_notify_change(g);return;}
 int knob=rb_get_knob_id(x0,y0);if(knob>=0){g->dragging=1;g->drag_param=knob;g->drag_y=y0;g->hover_control=knob;
  int par=rb_knob_param(g,knob);rb_set_knob(g,knob,par>=0?e->params[par]:knob==0?e->pitch[g->selected]:knob==1?e->decay_shape[g->selected]:knob==3?e->fm_depth[g->selected]:knob==12?e->swing:knob==13?e->scatter:e->chance);return;}
 if(y0>=812&&y0<842&&x0>=954&&x0<1181){if(x0<1032){for(int i=0;i<16;i+=4)e->steps[g->selected]|=(uint16_t)(1u<<i);}else if(x0<1099)e->steps[g->selected]=0;else e->steps[g->selected]^=0xaaaa;
  rb_broadcast(g,34+g->selected,(double)e->steps[g->selected]/65535.0);rb_notify_change(g);return;}
 if(y0>=847&&y0<915&&x0>=24&&x0<1173){int step=(x0-24)/72;if(step>=0&&step<16)e->steps[g->selected]^=(uint16_t)(1u<<step);rb_broadcast(g,34+g->selected,(double)e->steps[g->selected]/65535.0);rb_notify_change(g);return;}
 if(y0>=28&&y0<80&&x0>=745&&x0<914){if(x0<790)e->bpm-=1;else e->bpm+=1;if(e->bpm<40)e->bpm=40;if(e->bpm>250)e->bpm=250;rb_broadcast(g,59,(e->bpm-40.)/210.);rb_notify_change(g);return;}
}
static LRESULT W32 rb_window_proc(void*hwnd,uint32_t message,WPARAM wp,LPARAM lp){
 // WM_NCCREATE: store the instance pointer before any input, size, or paint dispatch.
 if(message==129){ // WM_NCCREATE: Windows supplies CREATESTRUCTA.lpCreateParams at offset 0.
  const CREATESTRUCTA *cs=(const CREATESTRUCTA*)lp;
  if(!cs||!cs->createParams)return 0;
  SetWindowLongPtrA(hwnd,-21,(intptr_t)cs->createParams);
  return DefWindowProcA(hwnd,message,wp,lp);
 }
 if(message==1){ // WM_CREATE: retrieve the same stable editor object.
  const CREATESTRUCTA *cs=(const CREATESTRUCTA*)lp;
  if(!cs||!cs->createParams)return -1;
  SetWindowLongPtrA(hwnd,-21,(intptr_t)cs->createParams);
  return 0;
 }
 RBGui *gui=(RBGui*)GetWindowLongPtrA(hwnd,-21);
 if(message==20)return 1; // WM_ERASEBKGND: no flash over approved artwork.
 if(message==5&&gui){rb_actual_size(gui);InvalidateRect(hwnd,0,0);return 0;}
 if(message==15){PAINTSTRUCT ps;void*dc=BeginPaint(hwnd,&ps);
  if(dc&&rb_picture&&gui){
   rb_actual_size(gui);
   if(gui->engine){gui->kit=gui->engine->kit;
    if(gui->learn_active&&gui->engine->midi_learn_target<0)gui->learn_active=0;}
   BITMAPINFO bitmap={{40,RB_GUI_WIDTH,-RB_GUI_HEIGHT,1,24,0,0,0,0,0,0},{0}};
   StretchDIBits(dc,0,0,gui->width,gui->height,0,0,RB_GUI_WIDTH,RB_GUI_HEIGHT,rb_picture,&bitmap,0,0x00CC0020);
   rb_show_overlay(dc,gui);
  }
  EndPaint(hwnd,&ps);return 0;
 }
 if(message==275){if(gui&&gui->engine&&gui->engine->playing)InvalidateRect(hwnd,0,0);return 0;}
 if(message==513){ // WM_LBUTTONDOWN
  if(gui){rb_actual_size(gui);SetFocus(hwnd);SetCapture(hwnd);
   rb_click(gui,(int)(int16_t)(lp&0xffff),(int)(int16_t)((lp>>16)&0xffff));}
  return 0;
 }
 if(message==514){ // WM_LBUTTONUP
  if(gui){gui->dragging=0;gui->input_events++;InvalidateRect(hwnd,0,0);}ReleaseCapture();return 0;
 }
 if(message==533){if(gui)gui->dragging=0;return 0;} // WM_CAPTURECHANGED
 if(message==256&&gui&&gui->engine){
  const char *keys="1234QWERASDFZXCV";
  for(int j=0;j<16;j++)if((int)wp==(int)keys[j]){
   int pad=j+gui->bank*16;
   if(pad<24){gui->selected=pad;rb_audition(gui,pad,.88);
    gui->input_events++;InvalidateRect(hwnd,0,0);}return 0;}
 }
 if(message==512&&gui&&gui->engine){ // WM_MOUSEMOVE
  rb_actual_size(gui);
  int cx=(int)((int64_t)(int16_t)(lp&0xffff)*RB_GUI_WIDTH/gui->width);
  int cy=(int)((int64_t)(int16_t)((lp>>16)&0xffff)*RB_GUI_HEIGHT/gui->height);
  if(gui->dragging){
   if(!(wp&0x0001u)){gui->dragging=0;ReleaseCapture();return 0;}
   int id=gui->drag_param,par=rb_knob_param(gui,id);
   double old=par>=0?gui->engine->params[par]:id==0?gui->engine->pitch[gui->selected]:id==1?gui->engine->decay_shape[gui->selected]:id==3?gui->engine->fm_depth[gui->selected]:id==12?gui->engine->swing:id==13?gui->engine->scatter:gui->engine->chance;
   rb_set_knob(gui,id,old+(gui->drag_y-cy)*.004);gui->drag_y=cy;
   return 0;
  }
  int id=rb_get_knob_id(cx,cy);
  if(id!=gui->hover_control){gui->hover_control=id;InvalidateRect(hwnd,0,0);}
  return 0;
 }
 if(message==522&&gui&&gui->engine){ // WM_MOUSEWHEEL: coordinates are SCREEN coords, not client.
  rb_actual_size(gui);
  struct {int x,y;} pt={(int)(int16_t)(lp&0xffff),(int)(int16_t)((lp>>16)&0xffff)};
  ScreenToClient(hwnd,&pt);
  int xx=(int)((int64_t)pt.x*RB_GUI_WIDTH/gui->width);
  int yy=(int)((int64_t)pt.y*RB_GUI_HEIGHT/gui->height);
  int knob=rb_get_knob_id(xx,yy);
  if(knob>=0){int par=rb_knob_param(gui,knob);
   double old=par>=0?gui->engine->params[par]:knob==0?gui->engine->pitch[gui->selected]:knob==1?gui->engine->decay_shape[gui->selected]:knob==3?gui->engine->fm_depth[gui->selected]:knob==12?gui->engine->swing:knob==13?gui->engine->scatter:gui->engine->chance;
   int delta=(int16_t)((wp>>16)&0xffff);
   rb_set_knob(gui,knob,old+(delta/120.)*.015);gui->input_events++;
  }
  return 0;
 }
 return DefWindowProcA(hwnd,message,wp,lp);
}
int rb_gui_attach(RBGui *g,void*parent,RubedoEngine*e,void*ctx,void(*fn)(void*,double)){
 if(!g||!parent)return 0;g->engine=e;g->callback_context=ctx;g->kit_changed=fn;
 g->kit=e?e->kit:g->kit;g->selected=0;g->width=RB_GUI_WIDTH;g->height=RB_GUI_HEIGHT;g->hover_control=-1;g->learn_active=0;g->force=0;g->mode=0;g->routing=0;g->dragging=0;g->input_events=0;g->editor_only=0;
 if(!rb_read_picture())return 0;
 WNDCLASSEXA cls={0};cls.cbSize=sizeof(cls);cls.style=3;cls.proc=(void*)rb_window_proc;cls.instance=rubedo_module;cls.className="RubedoAtlasDrumkitInputCP08R2";cls.background=0;cls.clsExtra=0;cls.wndExtra=0;cls.style=3;
 cls.smallIcon=0;cls.icon=0;cls.cursor=0;cls.menu=0;cls.style=3;
 RegisterClassExA(&cls);
 void*hwnd=CreateWindowExA(0,cls.className,"Rubedo Atlas Drumkit",0x50000000,0,0,RB_GUI_WIDTH,RB_GUI_HEIGHT,parent,0,rubedo_module,g);
 if(!hwnd){rb_drop_picture();return 0;}g->hwnd=hwnd;
 rb_actual_size(g);SetTimer(hwnd,1,95,0);InvalidateRect(hwnd,0,0);return 1;
}
void rb_gui_detach(RBGui *g){if(g&&g->hwnd){KillTimer(g->hwnd,1);DestroyWindow(g->hwnd);g->hwnd=0;rb_drop_picture();}}
void rb_gui_show(RBGui*g,int visible){if(g&&g->hwnd)ShowWindow(g->hwnd,visible?5:0);}
void rb_gui_size(RBGui*g,uint32_t w,uint32_t h){if(g&&g->hwnd&&w>1&&h>1){g->width=w;g->height=h;SetWindowPos(g->hwnd,0,0,0,(int)w,(int)h,0x14u);InvalidateRect(g->hwnd,0,0);}}
#else
int rb_gui_attach(RBGui*a,void*b,RubedoEngine*c,void*d,void(*e)(void*,double)){(void)a;(void)b;(void)c;(void)d;(void)e;return 0;}
void rb_gui_detach(RBGui*a){(void)a;}
void rb_gui_show(RBGui*a,int b){(void)a;(void)b;}
void rb_gui_size(RBGui*a,uint32_t b,uint32_t c){(void)a;(void)b;(void)c;}
int rb_gui_asset_ready(void){return 0;}
#endif
#if defined(_WIN32) && defined(RUBEDO_GUI_TEST)
intptr_t rb_test_message(void*window,uint32_t message,uintptr_t wp,intptr_t lp){
 return rb_window_proc(window,message,wp,lp);
}
#endif