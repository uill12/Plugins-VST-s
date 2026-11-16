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
/* Win64 CREATESTRUCTA must begin with lpCreateParams; CP08R1 crashed here. */
typedef struct {void*createParams,*instance,*menu,*parent;int32_t cy,cx,y,x,style;const char*name,*cls;uint32_t exStyle;} CREATESTRUCTA;
_Static_assert(sizeof(CREATESTRUCTA)==80,"Win64 CREATESTRUCTA layout mismatch");
_Static_assert(offsetof(CREATESTRUCTA,createParams)==0,"Win64 CREATESTRUCTA lpCreateParams offset mismatch");
_Static_assert(offsetof(CREATESTRUCTA,style)==48,"Win64 CREATESTRUCTA style offset mismatch");
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
WIMP void* W32 CreateFontA(int,int,int,int,int,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,const char*);
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
WIMP int W32 WriteFile(HANDLE,const void*,uint32_t,uint32_t*,void*);
WIMP int W32 FlushFileBuffers(HANDLE);
WIMP int W32 MoveFileExA(const char*,const char*,uint32_t);
WIMP int W32 DeleteFileA(const char*);
WIMP uint32_t W32 GetFileAttributesA(const char*);
WIMP int W32 MessageBoxA(void*,const char*,const char*,unsigned);
// OPENFILENAMEA (Win64 ABI, common file dialogs) — public SDK field layout.
typedef struct {
 uint32_t lStructSize;void *hwndOwner,*hInstance;const char *lpstrFilter;
 char *lpstrCustomFilter;uint32_t nMaxCustFilter,nFilterIndex;
 char *lpstrFile;uint32_t nMaxFile;char *lpstrFileTitle;uint32_t nMaxFileTitle;
 const char *lpstrInitialDir,*lpstrTitle;uint32_t Flags;
 uint16_t nFileOffset,nFileExtension;const char *lpstrDefExt;
 intptr_t lCustData;void *lpfnHook;const char *lpTemplateName;
 void *pvReserved;uint32_t dwReserved,FlagsEx;
} RBOpenFileName;
_Static_assert(sizeof(RBOpenFileName)==152,"Win64 OPENFILENAMEA size mismatch");
_Static_assert(offsetof(RBOpenFileName,lpstrFile)==48,"Win64 OPENFILENAMEA lpstrFile offset mismatch");
_Static_assert(offsetof(RBOpenFileName,Flags)==96,"Win64 OPENFILENAMEA Flags offset mismatch");
WIMP int W32 GetSaveFileNameA(RBOpenFileName*);
WIMP int W32 GetOpenFileNameA(RBOpenFileName*);
WIMP uint32_t W32 CommDlgExtendedError(void);


#include "rubedo_blank_card.inc"
// CP12 standalone pad-map files, deliberately independent of sound kit/presets.
// File dialogs, disk I/O and host callback requests run on the GUI thread only.
enum {RB_MAP_SAVED=1,RB_MAP_LOADED=2,RB_MAP_INVALID=3,RB_MAP_IOERROR=4,RB_MAP_LEARN_LIMIT=5};
static void rb_map_status(RBGui*g,uint8_t code){
 g->map_notice=code;g->map_notice_until=GetTickCount()+2900;
 if(g->hwnd)InvalidateRect(g->hwnd,0,0);
}
static void rb_map_cancel_learn(RBGui*g){
 g->learn_active=0;
 if(!g->editor_only)atomic_store_explicit(&g->engine->midi_learn_target,-1,memory_order_release);
}
static int rb_map_dialog(RBGui*g,int save,char path[1024]){
 static const char filter[]="Rubedo MIDI Map (*.rbmap)\0*.rbmap\0All files (*.*)\0*.*\0";
 RBOpenFileName ofn={0};ofn.lStructSize=sizeof(ofn);ofn.hwndOwner=g->hwnd;
 ofn.lpstrFilter=filter;ofn.nFilterIndex=1;ofn.lpstrFile=path;ofn.nMaxFile=1024;
 ofn.lpstrTitle=save?"Save Rubedo Pad Map":"Load Rubedo Pad Map";
 // Avoid Win32 GetSaveFileNameA truncating default extensions to 3 chars.
 // Set the extension ourselves after the dialog, retaining a portable .rbmap suffix.
 ofn.lpstrDefExt=0;
 ofn.Flags=0x00080000u|0x00000800u|0x00000008u|(save?0x00000002u:0x00001000u);
 // CP11 captures the mouse during WM_LBUTTONDOWN. Release it before opening
 // a modal file dialog so neither Bitwig nor the child editor retains capture.
 ReleaseCapture();
 int ok=save?GetSaveFileNameA(&ofn):GetOpenFileNameA(&ofn);
 if(!ok&&CommDlgExtendedError())rb_map_status(g,RB_MAP_IOERROR);
 if(!ok||!save)return ok;
 // If the user types a basename without an extension, append .rbmap. The
 // overwrite prompt must check that final path rather than the bare basename.
 unsigned length=0,base=0;while(path[length]&&length<1024){
  if(path[length]=='\\'||path[length]=='/')base=length+1;
  length++;
 }
 if(length>=1024){rb_map_status(g,RB_MAP_IOERROR);return 0;}
 int has_extension=0;for(unsigned j=base;j<length;j++)if(path[j]=='.'&&j+1<length)has_extension=1;
 if(!has_extension){
  static const char extension[]=".rbmap";
  if(length+sizeof(extension)>1024){rb_map_status(g,RB_MAP_IOERROR);return 0;}
  for(unsigned j=0;j<sizeof(extension);j++)path[length+j]=extension[j];
  if(GetFileAttributesA(path)!=0xffffffffu){
   if(MessageBoxA(g->hwnd,"A map with this name exists. Replace it?","Rubedo Pad Maps",0x00000004u|0x00000020u)!=6)
    return 0;
  }
 }
 return 1;
}
static int rb_map_save_file(const char*path,RubedoEngine*e){
 char text[320];int bytes=rubedo_map_encode(e,text,sizeof(text));if(bytes<=0)return 0;
 char temp[1100];unsigned k=0;
 while(path[k]&&k<1024){temp[k]=path[k];k++;}if(k>1023||k+5>=sizeof(temp))return 0;
 static const char suffix[]={'.','t','m','p',0};
 for(unsigned j=0;j<sizeof(suffix);j++)temp[k++]=suffix[j];
 HANDLE f=CreateFileA(temp,0x40000000u,0,0,2,0x80,0);
 if(f==RUBEDO_INVALID_HANDLE)return 0;
 uint32_t written=0;
 int ok=WriteFile(f,text,(uint32_t)bytes,&written,0)&&written==(uint32_t)bytes;
 if(ok)ok=FlushFileBuffers(f);
 if(!CloseHandle(f))ok=0;
 if(ok)ok=MoveFileExA(temp,path,0x1u|0x8u); // replace atomically on the same drive
 if(!ok)DeleteFileA(temp);
 return ok;
}
static int rb_map_load_file(const char*path,RubedoEngine*e){
 HANDLE f=CreateFileA(path,0x80000000u,1,0,3,0x80,0);
 if(f==RUBEDO_INVALID_HANDLE)return -1;
 uint32_t size=SetFilePointer(f,0,0,2);
 if(size==0xffffffffu||size==0||size>512){CloseHandle(f);return 0;}
 if(SetFilePointer(f,0,0,0)==0xffffffffu){CloseHandle(f);return -1;}
 char text[513];uint32_t got=0;
 int ok=ReadFile(f,text,size,&got,0)&&got==size;
 CloseHandle(f);if(!ok)return -1;
 int16_t notes[RUBEDO_PADS];if(!rubedo_map_decode(text,size,notes))return 0;
 // Decoder completed all validation first: invalid maps never partly apply.
 rubedo_import_mappings(e,notes);
 atomic_store_explicit(&e->host_mapping_dirty,1,memory_order_release);
 return 1;
}
static void rb_map_action(RBGui*g,int save){
 if(!g||!g->engine)return;
 rb_map_cancel_learn(g);
 char path[1024]={0};
 if(save){static const char def[]="Rubedo_Pad_Map.rbmap";for(unsigned i=0;i<sizeof(def);i++)path[i]=def[i];}
 if(!rb_map_dialog(g,save,path))return;
 int result=save?rb_map_save_file(path,g->engine):rb_map_load_file(path,g->engine);
 if(result>0){
  rb_map_status(g,save?RB_MAP_SAVED:RB_MAP_LOADED);
  if(!save&&g->map_changed)g->map_changed(g->callback_context);
 }else rb_map_status(g,result==0?RB_MAP_INVALID:RB_MAP_IOERROR);
}

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
// The CP10 default TextOutA font and opaque text rectangles broke the approved
// visual identity. CP11 uses the original embossed card textures and themed fonts.
// No live text is painted over a mismatched solid-color debug surface.
static int rb_font_px(RBGui*g,int v){int x=-(int)((int64_t)v*g->height/RB_GUI_HEIGHT);return x>-6?-6:x;}
static void rb_drop_fonts(RBGui*g){
 if(!g)return;
 void **f[]={&g->f_num,&g->f_name,&g->f_role,&g->f_midi,&g->f_title,&g->f_small,&g->f_kit};
 for(int k=0;k<7;k++)if(*f[k]){DeleteObject(*f[k]);*f[k]=0;}
 g->font_height=0;
}
static void*rb_font(RBGui*g,int size,int weight,const char*face){
 return CreateFontA(rb_font_px(g,size),0,0,0,weight,0,0,0,0,0,0,5,0,face);
}
static void rb_ensure_fonts(RBGui*g){
 if(g->f_num&&g->font_height==g->height)return;
 rb_drop_fonts(g);
 g->f_num=rb_font(g,23,400,"Georgia");
 g->f_name=rb_font(g,11,700,"Arial");
 g->f_role=rb_font(g,9,500,"Arial");
 g->f_midi=rb_font(g,10,600,"Georgia");
 g->f_title=rb_font(g,23,700,"Georgia");
 g->f_small=rb_font(g,10,600,"Arial");
 g->f_kit=rb_font(g,21,700,"Georgia");
 g->font_height=g->height;
}
static void rb_ink(void*dc,RBGui*g,void*font,int x,int y,const char*t,uint32_t color){
 if(!t||!t[0])return;
 int n=0;while(t[n]&&n<95)n++;
 void*old=0;if(font)old=SelectObject(dc,font);
 SetBkMode(dc,1);SetTextColor(dc,color);
 TextOutA(dc,rb_scale_x(g,x),rb_scale_y(g,y),t,n);
 if(old)SelectObject(dc,old);
}
static void rb_uint(char*dst,unsigned n){char t[16];int p=0,j=0;do{t[p++]=(char)('0'+n%10);n/=10;}while(n&&p<15);while(p)dst[j++]=t[--p];dst[j]=0;}
static void rb_two_digits(char b[4],unsigned n){b[0]=(char)('0'+n/10%10);b[1]=(char)('0'+n%10);b[2]=0;}
static void rb_pad_role_title(char*dst,unsigned pad,int caps){
 const char*nm=rubedo_pad_names[pad];int k=0,initial=1;
 while(*nm&&k<38){char c=*nm++;
  if(c=='-'||c=='_'){dst[k++]=' ';initial=1;continue;}
  if(caps&&c>='a'&&c<='z')c=(char)(c-'a'+'A');
  else if(!caps&&initial&&c>='a'&&c<='z')c=(char)(c-'a'+'A');
  dst[k++]=c;initial=0;
 }dst[k]=0;
}
static void rb_pad_note(void*dc,RBGui*g,int x,int y,int pad){
 if(!g->engine)return;
 int midi=atomic_load_explicit(&g->engine->mappings[pad],memory_order_acquire);
 if(midi<0||midi>127)return;
 char digits[16],label[18]="";rb_uint(digits,(unsigned)midi);
 label[0]='M';int j=1;for(int k=0;digits[k]&&j<12;k++)label[j++]=digits[k];label[j]=0;
 rb_ink(dc,g,g->f_midi,x+75,y+68,label,0x008cbadb); // warm faded gold
}
static void rb_pad_skin(void*dc,RBGui*g,int x,int y,int pad,int selected,int active){
 BITMAPINFO bi={{40,101,-89,1,24,0,0,0,0,0,0},{0}};
 const uint8_t *src=selected?rb_selected_card_rgb:rb_clean_card_rgb;
 StretchDIBits(dc,rb_scale_x(g,x),rb_scale_y(g,y),rb_scale_x(g,101),rb_scale_y(g,89),0,0,101,89,src,&bi,0,0x00CC0020);
 if(!active)return;
 char num[4],name[40],role[40];rb_two_digits(num,(unsigned)pad+1);
 rb_pad_role_title(name,(unsigned)pad,1);rb_pad_role_title(role,(unsigned)pad,1);
 if(pad==0&&g->kit==0){const char*glass="GLASS SUB I...";int k=0;while(glass[k]){name[k]=glass[k];k++;}name[k]=0;}
 rb_ink(dc,g,g->f_num,x+8,y+12,num,0x0076b4d4);
 static const char*keys[16]={"1","2","3","4","Q","W","E","R","A","S","D","F","Z","X","C","V"};
 if(!g->bank)rb_ink(dc,g,g->f_role,x+89,y+7,keys[pad%16],0x00c7d9e9);
 rb_ink(dc,g,g->f_name,x+8,y+47,name,0x00dfebf4);
 rb_ink(dc,g,g->f_role,x+8,y+68,role,0x00b6cbbf);
}
static void rb_pad_card(void*dc,RBGui*g,int x,int y,int pad,int pressed,int active){
 int selected=active&&pad==(int)g->selected;
 // On Core, preserve original card labels, shadows and color; paint only the
 // one card with originally baked selection, plus the currently selected card.
 // On Extended, cover every baked Core label using authentic blank skin.
 if(g->bank||pad==0||selected)rb_pad_skin(dc,g,x,y,pad,selected,active);
 if(!active)return;
 rb_pad_note(dc,g,x,y,pad);
 if(pressed)rb_outline(dc,g,x+2,y+2,97,85,0x0069b3da,2);
}
static void rb_update_learn_target(RBGui*g){
 if(g&&g->engine&&g->learn_active&&!g->editor_only)
  atomic_store_explicit(&g->engine->midi_learn_target,(int)g->selected,memory_order_release);
}
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
 if(!g||id<0||id>=16)return -1;
 // Explicit 1:1 correspondence to the printed controls. No duplicate fake connections.
 static const int ids[16]={-1,-1,2,-1,6,4,9,-1,-1,1,-1,-1,-1,-1,8,-1};
 if(id==7)return 10+(int)g->selected; // VOICE LEVEL
 return ids[id];
}
static double rb_knob_value(RBGui*g,int id){
 RubedoEngine*e=g->engine;unsigned pad=g->selected;
 int par=rb_knob_param(g,id);
 if(par>=0)return e->params[par];
 switch(id){
  case 0:return e->pitch[pad];case 1:return e->decay_shape[pad];
  case 3:return e->fm_depth[pad];case 8:return e->pad_gain[pad];
  case 10:return e->pad_attack[pad];case 11:return e->pad_release[pad];
  case 12:return e->swing;case 13:return e->scatter;case 15:return e->chance;
  default:return .5;
 }
}
static void rb_broadcast(RBGui*g,unsigned id,double normalized);
static void rb_set_knob(RBGui*g,int id,double value){
 if(!g||!g->engine||id<0||id>=16)return;
 RubedoEngine*e=g->engine;unsigned pad=g->selected;int par=rb_knob_param(g,id);
 if(value<0)value=0;if(value>1)value=1;
 if(par>=0){rubedo_set_param(e,(unsigned)par,value);
  if(g->param_changed)g->param_changed(g->callback_context,(unsigned)par,value);
 }else switch(id){
  case 0:e->pitch[pad]=(float)value;rb_broadcast(g,60u+pad,value);break;
  case 1:e->decay_shape[pad]=(float)value;e->decay_active[pad]=1;rb_broadcast(g,84u+pad,value);rb_broadcast(g,329u+pad,1.);break;
  case 3:e->fm_depth[pad]=(float)value;e->fm_active[pad]=1;rb_broadcast(g,108u+pad,value);rb_broadcast(g,353u+pad,1.);break;
  case 8:e->pad_gain[pad]=(float)value;rb_broadcast(g,160u+pad,value);break;
  case 10:e->pad_attack[pad]=(float)value;rb_broadcast(g,184u+pad,value);break;
  case 11:e->pad_release[pad]=(float)value;rb_broadcast(g,208u+pad,value);break;
  case 12:e->swing=(float)value;rb_broadcast(g,132,value);break;
  case 13:e->scatter=(float)value;rb_broadcast(g,133,value);break;
  case 15:e->chance=(float)value;rb_broadcast(g,134,value);break;
 }
 g->hover_control=id;g->changed_tick++;InvalidateRect(g->hwnd,0,0);
}
static void rb_flash_pad(RBGui*g,int pad){
 if(!g||pad<0||pad>=24)return;
 g->selected=(uint32_t)pad;g->bank=(uint32_t)(pad>=16);
 g->flash_pad=pad;g->flash_until=GetTickCount()+220;g->pad_flash_until[pad]=g->flash_until;
 if(g->hwnd)InvalidateRect(g->hwnd,0,0);
}
// CP11: restrained reactive linework and typeset fields only. Static engraved
// artwork is the foundation; generic opaque labels and full-card fills removed.
static void rb_show_overlay(void*hdc,RBGui*g){
 if(!g||!g->engine)return;
 rb_ensure_fonts(g);
 RubedoEngine*e=g->engine;
 const uint32_t gold=0x006eaaC9,ink=0x002b3329,moss=0x004e6558;
 char number[16],label[64];
 // The base image has been inpainted ONLY at dynamic text positions.
 rb_two_digits((char*)label,(unsigned)g->kit+1);
 rb_ink(hdc,g,g->f_num,26,120,label,gold);
 rb_ink(hdc,g,g->f_kit,75,120,rubedo_kit_names[g->kit],ink);
 rb_uint(number,(unsigned)(e->bpm+.5));rb_ink(hdc,g,g->f_title,811,35,number,ink);
 // CP12: actual Save/Load file actions in embossed, artwork-matched buttons.
 rb_ink(hdc,g,g->f_small,266,228,"SAVE MAP",ink);
 rb_ink(hdc,g,g->f_small,366,228,"LOAD MAP",ink);
 // Alchemical forces: subtle engraved rings, not opaque diagnostic boxes.
 static const int fx[4]={756,622,893,757},fy[4]={333,442,443,557};
 rb_disc(hdc,g,fx[g->force],fy[g->force],35,gold,0);
 static const int mode_x[4]={614,685,751,836},mode_w[4]={62,58,77,64};
 rb_outline(hdc,g,mode_x[g->mode],696,mode_w[g->mode],26,gold,1);
 if(g->routing)rb_outline(hdc,g,1403,739,87,29,moss,2);
 rb_outline(hdc,g,g->bank?93:24,218,g->bank?83:65,29,gold,1);
 const int families[4]={5,11,17,23};int family=0;
 while(family<3&&g->kit>(unsigned)families[family])family++;
 static const int family_x[4]={606,692,770,858},family_w[4]={77,69,79,109};
 rb_outline(hdc,g,family_x[family],121,family_w[family],27,gold,1);
 // The MIDI map is also the source for each dynamic pad number. These cards
 // never introduce a separate display-only note map.
 int base=g->bank?16:0;
 for(int i=0;i<16;i++){
  int pad=base+i,x=24+(i%4)*110,y=256+(i/4)*96;
  int active=pad<RUBEDO_PADS;
  int pressed=active&&((int32_t)(g->pad_flash_until[pad]-GetTickCount())>0);
  rb_pad_card(hdc,g,x,y,active?pad:0,pressed,active);
 }
 // Selected voice metadata occupies the original parchment panel, with
 // typesetting that matches the approved hierarchy rather than cyan strips.
 char heading[64]="SELECTED INSTRUMENT / VOICE ";
 if(g->map_notice&&(int32_t)(g->map_notice_until-GetTickCount())>0){
  const char*t=g->map_notice==RB_MAP_SAVED?"PAD MAPPING SAVED":
     g->map_notice==RB_MAP_LOADED?"PAD MAPPING LOADED":
     g->map_notice==RB_MAP_INVALID?"INVALID PAD MAP FILE":
      g->map_notice==RB_MAP_LEARN_LIMIT?"VST3 LEARN: HOST BRIDGE PENDING":"PAD MAP FILE ERROR";
  rb_ink(hdc,g,g->f_small,37,693,t,moss);
 } else if(g->learn_active){const char*t="MIDI LEARN  -  PLAY A NOTE";rb_ink(hdc,g,g->f_small,37,693,t,moss);}
 else if(g->learn_notice&&(int32_t)(g->learn_notice_until-GetTickCount())>0){
  rb_ink(hdc,g,g->f_small,37,693,"MIDI NOTE ASSIGNED",moss);
 } else if(g->unmapped_note>=0&&(int32_t)(g->unmapped_notice_until-GetTickCount())>0){
  rb_uint(number,(unsigned)g->unmapped_note);int j=0;const char*prefix="UNMAPPED MIDI ";
  while(prefix[j]){heading[j]=prefix[j];j++;}for(int k=0;number[k]&&j<58;k++)heading[j++]=number[k];heading[j]=0;
  rb_ink(hdc,g,g->f_small,37,693,heading,moss);
 } else {
  rb_two_digits(number,(unsigned)g->selected+1);int j=28;
  for(int k=0;number[k]&&j<60;k++)heading[j++]=number[k];heading[j]=0;
  rb_ink(hdc,g,g->f_small,37,693,heading,0x006d6d68);
 }
 char title[40];rb_pad_role_title(title,g->selected,0);
 if(g->selected==0&&g->kit==0){const char*t="Glass Sub impact";int k=0;while(t[k]){title[k]=t[k];k++;}title[k]=0;}
 rb_ink(hdc,g,g->f_title,38,707,title,ink);
 rb_ink(hdc,g,g->f_title,1064,218,title,ink);
 rb_pad_role_title(label,g->selected,0);
 rb_ink(hdc,g,g->f_small,48,745,label,ink);
 unsigned midi=(unsigned)atomic_load_explicit(&e->mappings[g->selected],memory_order_acquire);
 if(midi<128){const char*note_names[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
  char b[32]="MIDI ";rb_uint(number,midi);int j=5;
  for(int k=0;number[k]&&j<30;k++)b[j++]=number[k];b[j++]=' ';b[j++]='-';b[j++]=' ';
  const char*n=note_names[midi%12];for(int k=0;n[k]&&j<29;k++)b[j++]=n[k];
  int oct=(int)(midi/12)-1;if(oct<0){b[j++]='-';oct=-oct;}
  rb_uint(number,(unsigned)oct);for(int k=0;number[k]&&j<30;k++)b[j++]=number[k];b[j]=0;
  rb_ink(hdc,g,g->f_small,365,744,b,0x00757b73);
 }
 if(g->learn_active)rb_outline(hdc,g,349,639,100,32,gold,2);
 // Existing sequencer and transport. No temporary words cross the artwork.
 for(int step=0;step<16;step++){
  int x=24+step*72;
  if(e->steps[g->selected]&(1u<<step))rb_outline(hdc,g,x,848,67,66,moss,2);
  if(e->playing&&(e->step_index+15)%16==(unsigned)step)rb_outline(hdc,g,x+2,849,62,63,gold,3);
 }
 if(e->playing)rb_outline(hdc,g,688,29,48,48,moss,2);
 rb_uint(number,(unsigned)e->variation+1u);rb_ink(hdc,g,g->f_small,1130,813,number,ink);
 for(int k=0;k<16;k++){
  int x=(k/4==3?(k%4==0?1247:k%4==1?1317:k%4==2?1386:1455):1117+(k%4)*106),
      y=(k/4==0?303:k/4==1?412:k/4==2?678:867);
  double v=rb_knob_value(g,k);
  int px=x+(int)((v-.5)*34.),py=y-29;
  rb_disc(hdc,g,px,py,3,gold,1);
  if(k==g->hover_control)rb_disc(hdc,g,x,y,31,gold,0);
 }
}
static void rb_broadcast(RBGui*g,unsigned id,double normalized){
 if(g&&g->param_changed)g->param_changed(g->callback_context,id,normalized);
}
static void rb_audition(RBGui*g,int pad,double velocity){
 if(!g||!g->engine||pad<0||pad>=24)return;
 // CLAP owns its audio engine, VST3 conveys trigger via host automation.
 if(!g->editor_only)rubedo_request_audition(g->engine,pad,velocity);
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
 if(y0>=217&&y0<=252&&x0>=20&&x0<=187){g->bank=x0>=94?1:0;g->selected=g->bank?16:0;rb_update_learn_target(g);rb_notify_change(g);return;}
 if(y0>=256&&y0<637&&x0>=24&&x0<452){int c=(x0-24)/110,r=(y0-256)/96;if(c<0||c>3||r<0||r>3)return;
  int pad=r*4+c+g->bank*16;if(pad>23)return;rb_flash_pad(g,pad);
  if(g->learn_active)rb_update_learn_target(g);else rb_audition(g,pad,.88);
  rb_notify_change(g);return;}
 if(x0>=38&&x0<170&&y0>=739&&y0<=775){
  rb_flash_pad(g,(int)((g->selected+1)%24));rb_update_learn_target(g);return;}
 if(y0>=218&&y0<=252&&x0>=243&&x0<=452){
  rb_map_action(g,x0<344);rb_notify_change(g);return;}
 if(x0>=345&&x0<452&&y0>=639&&y0<=674){if(g->editor_only){rb_map_status(g,RB_MAP_LEARN_LIMIT);return;}g->learn_active=(uint8_t)!g->learn_active;
   if(!g->editor_only)atomic_store(&e->midi_learn_target,g->learn_active?(int)g->selected:-1);
   rb_notify_change(g);return;}
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
 if(x0>=706&&x0<815&&y0>387&&y0<513){rb_flash_pad(g,(int)g->selected);rb_audition(g,(int)g->selected,.95);return;}
 if(y0>20&&y0<80&&x0>=686&&x0<740){e->playing^=1;e->step_clock=0;e->step_index=0;rb_broadcast(g,58,e->playing?1.:0.);rb_notify_change(g);return;}
 if(y0>=29&&y0<=74&&x0>=919&&x0<=955){
  uint32_t now=GetTickCount();
  if(g->last_tap_ms&&now>g->last_tap_ms){uint32_t delta=now-g->last_tap_ms;
   if(delta>=240&&delta<=1500)e->bpm=60000.0/(double)delta;}
  g->last_tap_ms=now;rb_broadcast(g,59,(e->bpm-40.)/210.);rb_notify_change(g);return;}
 if(y0>20&&y0<80&&x0>=956&&x0<1030){e->playing=0;e->step_index=0;e->step_clock=0;rb_broadcast(g,58,0.);rb_notify_change(g);return;}
 if(x0>=1065&&x0<=1487&&y0>=543&&y0<=638){
  // Envelope graph: drag attack, decay, gain or release region directly.
  int id=x0<1155?10:x0<1282?1:x0<1385?8:11;
  g->dragging=1;g->drag_param=id;g->drag_y=y0;g->hover_control=id;
  rb_set_knob(g,id,rb_knob_value(g,id));return;
 }
 int knob=rb_get_knob_id(x0,y0);if(knob>=0){g->dragging=1;g->drag_param=knob;g->drag_y=y0;g->hover_control=knob;
  rb_set_knob(g,knob,rb_knob_value(g,knob));return;}
 if(y0>=812&&y0<842&&x0>=954&&x0<1181){
  if(x0<1032){uint32_t z=(uint32_t)(g->kit*1009u+g->selected*199u+ ++g->changed_tick*7919u);
    uint16_t pattern=0;for(int i=0;i<16;i++){z^=z<<13;z^=z>>17;z^=z<<5;
     if((z&255u)<(i%4==0?195u:103u))pattern|=(uint16_t)(1u<<i);}
    e->steps[g->selected]=pattern?pattern:0x1111;
  }else if(x0<1099)e->steps[g->selected]=0;
  else {rubedo_select_variation(e,(e->variation+1)&3u);rb_broadcast(g,232u,(double)e->variation/3.);rb_notify_change(g);return;}
  rubedo_pattern_changed(e);rb_broadcast(g,34u+g->selected,(double)e->steps[g->selected]/65535.);rb_broadcast(g,233u+(unsigned)e->variation*24u+g->selected,(double)e->steps[g->selected]/65535.);rb_notify_change(g);return;}
 if(y0>=847&&y0<915&&x0>=24&&x0<1173){int step=(x0-24)/72;if(step>=0&&step<16)e->steps[g->selected]^=(uint16_t)(1u<<step);rubedo_pattern_changed(e);rb_broadcast(g,34u+g->selected,(double)e->steps[g->selected]/65535.);rb_broadcast(g,233u+(unsigned)e->variation*24u+g->selected,(double)e->steps[g->selected]/65535.);rb_notify_change(g);return;}
 if(y0>=28&&y0<80&&x0>=745&&x0<914){if(x0<790)e->bpm-=1;else e->bpm+=1;if(e->bpm<40)e->bpm=40;if(e->bpm>250)e->bpm=250;rb_broadcast(g,59,(e->bpm-40.)/210.);rb_notify_change(g);return;}
}
static LRESULT W32 rb_window_proc(void*hwnd,uint32_t message,WPARAM wp,LPARAM lp){
 // WM_NCCREATE: store the instance pointer before any input, size, or paint dispatch.
 if(message==129){CREATESTRUCTA *cs=(CREATESTRUCTA*)lp;
  SetWindowLongPtrA(hwnd,-21,(intptr_t)cs->createParams);
  return DefWindowProcA(hwnd,message,wp,lp);
 }
 if(message==1){CREATESTRUCTA *cs=(CREATESTRUCTA*)lp;
  SetWindowLongPtrA(hwnd,-21,(intptr_t)cs->createParams);return 0;}
 RBGui *gui=(RBGui*)GetWindowLongPtrA(hwnd,-21);
 if(message==20)return 1; // WM_ERASEBKGND: no flash over approved artwork.
 if(message==5&&gui){rb_actual_size(gui);InvalidateRect(hwnd,0,0);return 0;}
 if(message==15){PAINTSTRUCT ps;void*dc=BeginPaint(hwnd,&ps);
  if(dc&&rb_picture&&gui){
   rb_actual_size(gui);
   if(gui->engine)gui->kit=gui->engine->kit;
   BITMAPINFO bitmap={{40,RB_GUI_WIDTH,-RB_GUI_HEIGHT,1,24,0,0,0,0,0,0},{0}};
   StretchDIBits(dc,0,0,gui->width,gui->height,0,0,RB_GUI_WIDTH,RB_GUI_HEIGHT,rb_picture,&bitmap,0,0x00CC0020);
   rb_show_overlay(dc,gui);
  }
  EndPaint(hwnd,&ps);return 0;
 }
 if(message==275){if(gui&&gui->engine){RubedoEngine*e=gui->engine;
   uint32_t seq=atomic_load_explicit(&e->ui_note_seq,memory_order_acquire);
   if(seq!=gui->ui_seen_seq){gui->ui_seen_seq=seq;
    uint32_t mask=atomic_exchange_explicit(&e->ui_pending_mask,0,memory_order_acquire);
    uint32_t until=GetTickCount()+220;
    for(int i=0;i<RUBEDO_PADS;i++)if(mask&(1u<<i))gui->pad_flash_until[i]=until;
    int pad=atomic_load_explicit(&e->ui_last_pad,memory_order_relaxed);
    if(pad>=0&&pad<RUBEDO_PADS)rb_flash_pad(gui,pad);
   }
   uint32_t unmapped=atomic_load_explicit(&e->ui_unmapped_seq,memory_order_acquire);
   if(unmapped!=gui->unmapped_seen_seq){gui->unmapped_seen_seq=unmapped;
    gui->unmapped_note=atomic_load_explicit(&e->ui_unmapped_note,memory_order_relaxed);
    gui->unmapped_notice_until=GetTickCount()+2000;InvalidateRect(hwnd,0,0);
   }
   uint32_t learned=atomic_load_explicit(&e->ui_learn_seq,memory_order_acquire);
   if(learned!=gui->learn_seen_seq){gui->learn_seen_seq=learned;gui->learn_active=0;
    gui->learn_notice=1;gui->learn_notice_until=GetTickCount()+1800;InvalidateRect(hwnd,0,0);
   }
   uint32_t now=GetTickCount();
   if(gui->flash_pad>=0&&(int32_t)(now-gui->flash_until)>=0){gui->flash_pad=-1;InvalidateRect(hwnd,0,0);}
   for(int pad=0;pad<RUBEDO_PADS;pad++)if(gui->pad_flash_until[pad]&&(int32_t)(now-gui->pad_flash_until[pad])>=0){gui->pad_flash_until[pad]=0;InvalidateRect(hwnd,0,0);}
   if(gui->learn_notice&&(int32_t)(now-gui->learn_notice_until)>=0){gui->learn_notice=0;InvalidateRect(hwnd,0,0);}
   if(gui->unmapped_note>=0&&(int32_t)(now-gui->unmapped_notice_until)>=0){gui->unmapped_note=-1;InvalidateRect(hwnd,0,0);}
   if(gui->routing_hint_until&&(int32_t)(now-gui->routing_hint_until)>=0){gui->routing_hint_until=0;InvalidateRect(hwnd,0,0);}
   if(gui->map_notice&&(int32_t)(now-gui->map_notice_until)>=0){gui->map_notice=0;InvalidateRect(hwnd,0,0);}
   if(e->playing)InvalidateRect(hwnd,0,0);
  }return 0;}
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
  if(wp==27){gui->learn_active=0;atomic_store(&gui->engine->midi_learn_target,-1);InvalidateRect(hwnd,0,0);return 0;}
  if(lp&(1ll<<30))return 0; // suppress held-key retriggers
  const char *keys="1234QWERASDFZXCV";
  for(int j=0;j<16;j++)if((int)wp==(int)keys[j]){
   int pad=j+gui->bank*16;
   if(pad<24){rb_flash_pad(gui,pad);if(gui->learn_active)rb_update_learn_target(gui);else rb_audition(gui,pad,.88);
    gui->input_events++;InvalidateRect(hwnd,0,0);}return 0;}
 }
 if(message==512&&gui&&gui->engine){ // WM_MOUSEMOVE
  rb_actual_size(gui);
  int cx=(int)((int64_t)(int16_t)(lp&0xffff)*RB_GUI_WIDTH/gui->width);
  int cy=(int)((int64_t)(int16_t)((lp>>16)&0xffff)*RB_GUI_HEIGHT/gui->height);
  if(gui->dragging){
   if(!(wp&0x0001u)){gui->dragging=0;ReleaseCapture();return 0;}
   int id=gui->drag_param;
   rb_set_knob(gui,id,rb_knob_value(gui,id)+(gui->drag_y-cy)*.004);gui->drag_y=cy;
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
  if(knob>=0){
   int delta=(int16_t)((wp>>16)&0xffff);
   rb_set_knob(gui,knob,rb_knob_value(gui,knob)+(delta/120.)*.012);gui->input_events++;
  }else if(xx>=744&&xx<=915&&yy>=26&&yy<=80){
    gui->engine->bpm+=(int16_t)((wp>>16)&0xffff)/120.0;
    if(gui->engine->bpm<40)gui->engine->bpm=40;
    if(gui->engine->bpm>250)gui->engine->bpm=250;
    InvalidateRect(hwnd,0,0);
  }
  return 0;
 }
 return DefWindowProcA(hwnd,message,wp,lp);
}
int rb_gui_attach(RBGui *g,void*parent,RubedoEngine*e,void*ctx,void(*fn)(void*,double)){
 if(!g||!parent)return 0;g->engine=e;g->callback_context=ctx;g->kit_changed=fn;
 g->kit=e?e->kit:g->kit;g->selected=0;g->width=RB_GUI_WIDTH;g->height=RB_GUI_HEIGHT;g->hover_control=-1;g->font_height=0;g->f_num=0;g->f_name=0;g->f_role=0;g->f_midi=0;g->f_title=0;g->f_small=0;g->f_kit=0;g->learn_active=0;g->force=0;g->mode=0;g->routing=0;g->dragging=0;g->input_events=0;g->editor_only=0;g->flash_pad=-1;g->flash_until=0;g->learn_notice=0;g->unmapped_note=-1;g->map_notice=0;g->map_notice_until=0;for(int i=0;i<24;i++)g->pad_flash_until[i]=0;
 g->ui_seen_seq=e?atomic_load_explicit(&e->ui_note_seq,memory_order_acquire):0;
 g->learn_seen_seq=e?atomic_load_explicit(&e->ui_learn_seq,memory_order_acquire):0;
 g->unmapped_seen_seq=e?atomic_load_explicit(&e->ui_unmapped_seq,memory_order_acquire):0;
 if(!rb_read_picture())return 0;
 WNDCLASSEXA cls={0};cls.cbSize=sizeof(cls);cls.style=3;cls.proc=(void*)rb_window_proc;cls.instance=rubedo_module;cls.className="RubedoAtlasDrumkitInputCP12VST3";cls.background=0;cls.clsExtra=0;cls.wndExtra=0;cls.style=3;
 cls.smallIcon=0;cls.icon=0;cls.cursor=0;cls.menu=0;cls.style=3;
 RegisterClassExA(&cls);
 void*hwnd=CreateWindowExA(0,cls.className,"Rubedo Atlas Drumkit",0x50000000,0,0,RB_GUI_WIDTH,RB_GUI_HEIGHT,parent,0,rubedo_module,g);
 if(!hwnd){rb_drop_picture();return 0;}g->hwnd=hwnd;
 rb_actual_size(g);SetTimer(hwnd,1,35,0);InvalidateRect(hwnd,0,0);return 1;
}
void rb_gui_detach(RBGui *g){if(g&&g->hwnd){KillTimer(g->hwnd,1);DestroyWindow(g->hwnd);g->hwnd=0;rb_drop_fonts(g);rb_drop_picture();}}
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