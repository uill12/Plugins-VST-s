#pragma once
// CP10: double-buffered GDI+ / GDI native GUI, RGNES scene/overlay separation. No WebView2, HTML engine, or browser DLL.
// Visual base scenes are generated from the approved CP05 HTML and packaged as PNGs
// at the end of each plugin module. All interactive overlays and hit-testing are native.
#ifdef _WIN32
#include "GuiSceneData.h"
#include <stdint.h>
#include <stddef.h>
#if defined(CLAP_API)
using i32=int32_t; using u16=uint16_t;using u32=uint32_t;using u64=uint64_t;
#endif
using NWHND=void*;
struct NRect{i32 left,top,right,bottom;};
struct NPoint{i32 x,y;};
// Correct Win64 API layouts. Unlike CP09R1, createParams MUST be offset 0.
struct NCreateStructA {
 void*createParams;void*instance;void*menu;void*parent;
 i32 cy,cx,y,x,style;const char*name;const char*cls;u32 exStyle;
};
static_assert(sizeof(NCreateStructA)==80,"Win64 CREATESTRUCTA size");
static_assert(offsetof(NCreateStructA,createParams)==0,"Win64 lpCreateParams offset");
static_assert(offsetof(NCreateStructA,style)==48,"Win64 style offset");
static_assert(offsetof(NCreateStructA,exStyle)==72,"Win64 exStyle offset");
struct NWndClassExA {
 u32 size,style;void*wndProc;i32 clsExtra,wndExtra;void*instance,*icon,*cursor,*background;const char*menu,*name;void*smallIcon;
};
static_assert(sizeof(NWndClassExA)==80,"Win64 WNDCLASSEXA size");
static_assert(offsetof(NWndClassExA,instance)==24,"Win64 WNDCLASSEXA hInstance offset");

struct NPaint{void* hdc;i32 erase;NRect paint;i32 restore,update;uint8_t reserve[32];};
extern "C" __declspec(dllimport) void* __stdcall LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* __stdcall GetProcAddress(void*,const char*);
extern "C" __declspec(dllimport) void* __stdcall GetModuleHandleA(const char*);
extern "C" __declspec(dllimport) u32 __stdcall GetModuleFileNameA(void*,char*,u32);
extern "C" __declspec(dllimport) NWHND __stdcall CreateWindowExA(u32,const char*,const char*,u32,i32,i32,i32,i32,NWHND,void*,void*,void*);
extern "C" __declspec(dllimport) i32 __stdcall DestroyWindow(NWHND);
extern "C" __declspec(dllimport) i32 __stdcall ShowWindow(NWHND,i32);
extern "C" __declspec(dllimport) int64_t __stdcall SetWindowLongPtrA(NWHND,i32,int64_t);
extern "C" __declspec(dllimport) int64_t __stdcall GetWindowLongPtrA(NWHND,i32);
extern "C" __declspec(dllimport) int64_t __stdcall CallWindowProcA(void*,NWHND,u32,u64,int64_t);
extern "C" __declspec(dllimport) int64_t __stdcall DefWindowProcA(NWHND,u32,u64,int64_t);
extern "C" __declspec(dllimport) i32 __stdcall SetWindowPos(NWHND,NWHND,i32,i32,i32,i32,u32);
extern "C" __declspec(dllimport) i32 __stdcall GetClientRect(NWHND,NRect*);
extern "C" __declspec(dllimport) u64 __stdcall SetTimer(NWHND,u64,u32,void*);
extern "C" __declspec(dllimport) i32 __stdcall KillTimer(NWHND,u64);
extern "C" __declspec(dllimport) i32 __stdcall InvalidateRect(NWHND,const NRect*,i32);
extern "C" __declspec(dllimport) void* __stdcall BeginPaint(NWHND,NPaint*);
extern "C" __declspec(dllimport) i32 __stdcall EndPaint(NWHND,const NPaint*);
extern "C" __declspec(dllimport) NWHND __stdcall SetCapture(NWHND);
extern "C" __declspec(dllimport) i32 __stdcall ReleaseCapture(void);


static void* nigModule=nullptr; // from DllMain for appended artwork retrieval
static constexpr i32 NSCENE_W=1600,NSCENE_H=1000, NDEFAULT_W=1440,NDEFAULT_H=900;
static constexpr u32 NCHILD=0x40000000u,NVISIBLE=0x10000000u;
static constexpr u32 WM_NCCREATE_=0x0081,WM_NCDESTROY_=0x0082,WM_CREATE_=0x0001,WM_DESTROY_=0x0002;
static constexpr u32 WM_PAINT_=0x000F,WM_SIZE_=0x0005,WM_ERASE_=0x0014,WM_TIMER_=0x0113,WM_LDOWN_=0x0201,WM_LUP_=0x0202,WM_MOVE_=0x0200,WM_WHEEL_=0x020A,WM_RDOWN_=0x0204;
static int nlen(const char*s){int k=0;if(s)while(s[k]&&k<200)k++;return k;}
static bool nhit(NGuiR r,int x,int y){return r.w>0&&x>=r.x&&y>=r.y&&x<r.x+r.w&&y<r.y+r.h;}
static int nround(double a){return a<0?(int)(a-.5):(int)(a+.5);}
static const int8_t COSINE_X[129]={
 -71,-73,-76,-78,-80,-82,-84,-86,-88,-90,-91,-93,-94,-95,-96,-97,
 -98,-99,-99,-100,-100,-100,-100,-100,-100,-99,-99,-98,-97,-96,-95,-94,
 -92,-91,-89,-88,-86,-84,-82,-80,-77,-75,-72,-70,-67,-64,-62,-59,
 -56,-52,-49,-46,-43,-39,-36,-33,-29,-25,-22,-18,-15,-11,-7,-4,
 0,4,7,11,15,18,22,25,29,33,36,39,43,46,49,52,
 56,59,62,64,67,70,72,75,77,80,82,84,86,88,89,91,
 92,94,95,96,97,98,99,99,100,100,100,100,100,100,99,99,
 98,97,96,95,94,93,91,90,88,86,84,82,80,78,76,73,
 71
};
static const int8_t COSINE_Y[129]={
 71,68,65,62,60,57,53,50,47,44,41,37,34,30,27,23,
 20,16,12,9,5,1,-2,-6,-10,-13,-17,-21,-24,-28,-31,-35,
 -38,-42,-45,-48,-51,-55,-58,-61,-63,-66,-69,-72,-74,-77,-79,-81,
 -83,-85,-87,-89,-90,-92,-93,-95,-96,-97,-98,-98,-99,-99,-100,-100,
 -100,-100,-100,-99,-99,-98,-98,-97,-96,-95,-93,-92,-90,-89,-87,-85,
 -83,-81,-79,-77,-74,-72,-69,-66,-63,-61,-58,-55,-51,-48,-45,-42,
 -38,-35,-31,-28,-24,-21,-17,-13,-10,-6,-2,1,5,9,12,16,
 20,23,27,30,34,37,41,44,47,50,53,57,60,62,65,68,
 71
};
// Fixed OS graphics API pointers. Resolution occurs only on the GUI thread.
struct NGfx {
 bool ok=false;void *gp=nullptr,*sh=nullptr,*gdi=nullptr,*kernel=nullptr;uintptr_t token=0;
 i32 (__stdcall*startup)(uintptr_t*,const void*,void*)=nullptr;
 void(__stdcall*shutdown)(uintptr_t)=nullptr;
 i32(__stdcall*fromStream)(void*,void**)=nullptr;
 i32(__stdcall*graphics)(void*,void**)=nullptr;
 i32(__stdcall*draw)(void*,void*,i32,i32,i32,i32)=nullptr;
 i32(__stdcall*delGraphics)(void*)=nullptr;
 i32(__stdcall*delImage)(void*)=nullptr;
 void*(__stdcall*memoryStream)(const uint8_t*,u32)=nullptr;
 void*(__stdcall*heap)()=nullptr;
 void*(__stdcall*alloc)(void*,u32,u64)=nullptr;
 i32(__stdcall*free)(void*,u32,void*)=nullptr;
 void*(__stdcall*file)(const char*,u32,u32,void*,u32,u32,void*)=nullptr;
 u32(__stdcall*seek)(void*,i32,i32*,u32)=nullptr;
 i32(__stdcall*read)(void*,void*,u32,u32*,void*)=nullptr;
 i32(__stdcall*close)(void*)=nullptr;
 void*(__stdcall*pen)(i32,i32,u32)=nullptr;
 void*(__stdcall*brush)(u32)=nullptr;
 void*(__stdcall*select)(void*,void*)=nullptr;
 i32(__stdcall*deleteObj)(void*)=nullptr;
 i32(__stdcall*ellipse)(void*,i32,i32,i32,i32)=nullptr;
 i32(__stdcall*rectangle)(void*,i32,i32,i32,i32)=nullptr;
 i32(__stdcall*move)(void*,i32,i32,void*)=nullptr;
 i32(__stdcall*line)(void*,i32,i32)=nullptr;
 i32(__stdcall*text)(void*,i32,i32,const char*,i32)=nullptr;
 u32(__stdcall*setText)(void*,u32)=nullptr;
 i32(__stdcall*bkMode)(void*,i32)=nullptr;
 void*(__stdcall*font)(i32,i32,i32,i32,i32,u32,u32,u32,u32,u32,u32,u32,u32,const char*)=nullptr;
 void*(__stdcall*compatibleDC)(void*)=nullptr;
 void*(__stdcall*compatibleBitmap)(void*,i32,i32)=nullptr;
 i32(__stdcall*deleteDC)(void*)=nullptr;
 i32(__stdcall*blit)(void*,i32,i32,i32,i32,void*,i32,i32,u32)=nullptr;
};
static NGfx NG{};
static bool ensureNativeClass();
// Include format and kind so CLAP/VST3 modules never register the same
// local Win32 class name inside one Bitwig process.
#if defined(NIG_VST_GUI)
# if NIG_MODE == 0
static const char* NCLASS="NigredoOpusSeptemCP10_VST3_Audio_PaintFixed";
# else
static const char* NCLASS="NigredoOpusSeptemCP10_VST3_MIDI_PaintFixed";
# endif
#else
# if NIG_MODE == 0
static const char* NCLASS="NigredoOpusSeptemCP10_CLAP_Audio_PaintFixed";
# else
static const char* NCLASS="NigredoOpusSeptemCP10_CLAP_MIDI_PaintFixed";
# endif
#endif

template<class T>static T nproc(void*m,const char*name){return (T)(uintptr_t)GetProcAddress(m,name);}
static bool initNG(){if(NG.ok)return true;
 NG.gp=LoadLibraryA("gdiplus.dll");NG.sh=LoadLibraryA("shlwapi.dll");NG.gdi=LoadLibraryA("gdi32.dll");NG.kernel=GetModuleHandleA("kernel32.dll");
 if(!NG.gp||!NG.sh||!NG.gdi||!NG.kernel)return false;
 NG.startup=nproc<decltype(NG.startup)>(NG.gp,"GdiplusStartup");NG.shutdown=nproc<decltype(NG.shutdown)>(NG.gp,"GdiplusShutdown");
 NG.fromStream=nproc<decltype(NG.fromStream)>(NG.gp,"GdipCreateBitmapFromStream");
 NG.graphics=nproc<decltype(NG.graphics)>(NG.gp,"GdipCreateFromHDC");NG.draw=nproc<decltype(NG.draw)>(NG.gp,"GdipDrawImageRectI");
 NG.delGraphics=nproc<decltype(NG.delGraphics)>(NG.gp,"GdipDeleteGraphics");NG.delImage=nproc<decltype(NG.delImage)>(NG.gp,"GdipDisposeImage");
 NG.memoryStream=nproc<decltype(NG.memoryStream)>(NG.sh,"SHCreateMemStream");
 NG.heap=nproc<decltype(NG.heap)>(NG.kernel,"GetProcessHeap");NG.alloc=nproc<decltype(NG.alloc)>(NG.kernel,"HeapAlloc");NG.free=nproc<decltype(NG.free)>(NG.kernel,"HeapFree");
 NG.file=nproc<decltype(NG.file)>(NG.kernel,"CreateFileA");NG.seek=nproc<decltype(NG.seek)>(NG.kernel,"SetFilePointer");
 NG.read=nproc<decltype(NG.read)>(NG.kernel,"ReadFile");NG.close=nproc<decltype(NG.close)>(NG.kernel,"CloseHandle");
 NG.pen=nproc<decltype(NG.pen)>(NG.gdi,"CreatePen");NG.brush=nproc<decltype(NG.brush)>(NG.gdi,"CreateSolidBrush");
 NG.select=nproc<decltype(NG.select)>(NG.gdi,"SelectObject");NG.deleteObj=nproc<decltype(NG.deleteObj)>(NG.gdi,"DeleteObject");
 NG.ellipse=nproc<decltype(NG.ellipse)>(NG.gdi,"Ellipse");NG.rectangle=nproc<decltype(NG.rectangle)>(NG.gdi,"Rectangle");
 NG.move=nproc<decltype(NG.move)>(NG.gdi,"MoveToEx");NG.line=nproc<decltype(NG.line)>(NG.gdi,"LineTo");
 NG.text=nproc<decltype(NG.text)>(NG.gdi,"TextOutA");NG.setText=nproc<decltype(NG.setText)>(NG.gdi,"SetTextColor");
 NG.bkMode=nproc<decltype(NG.bkMode)>(NG.gdi,"SetBkMode");NG.font=nproc<decltype(NG.font)>(NG.gdi,"CreateFontA");
 NG.compatibleDC=nproc<decltype(NG.compatibleDC)>(NG.gdi,"CreateCompatibleDC");
 NG.compatibleBitmap=nproc<decltype(NG.compatibleBitmap)>(NG.gdi,"CreateCompatibleBitmap");
 NG.deleteDC=nproc<decltype(NG.deleteDC)>(NG.gdi,"DeleteDC");
 NG.blit=nproc<decltype(NG.blit)>(NG.gdi,"BitBlt");
 if(!NG.startup||!NG.fromStream||!NG.graphics||!NG.draw||!NG.delGraphics||!NG.delImage||!NG.memoryStream||!NG.heap||!NG.alloc||!NG.free||!NG.file||!NG.seek||!NG.read||!NG.close||!NG.pen||!NG.brush||!NG.select||!NG.deleteObj||!NG.ellipse||!NG.rectangle||!NG.move||!NG.line||!NG.text||!NG.setText||!NG.bkMode||!NG.font||!NG.compatibleDC||!NG.compatibleBitmap||!NG.deleteDC||!NG.blit)return false;
 struct GInit{u32 version;void*callback;u32 suppressBackground,suppressCodecs;}cfg{1,nullptr,0,0};
 NG.ok=NG.startup(&NG.token,&cfg,nullptr)==0;return NG.ok;
}
struct NativeView {
 bool used=false;NWHND hwnd=nullptr,parent=nullptr;void*oldProc=nullptr;
 void*owner=nullptr;int kind=0,width=NDEFAULT_W,height=NDEFAULT_H,drag=-1,dragY=0,scene=-1;
 void(*edit)(void*,int,double)=nullptr;double(*read)(void*,int)=nullptr;
 void*images[7]{},*streams[7]{},*buffers[7]{}; u32 sizes[7]{};
 double seen[ParamCount]{};
 int preset=0;bool ready=false;
 void *baseDC=nullptr,*frontDC=nullptr,*baseBmp=nullptr,*frontBmp=nullptr,*baseOld=nullptr,*frontOld=nullptr;
 int cacheW=0,cacheH=0,cacheStage=-1;
 bool compositeDirty=true;
};
static NativeView NVIEWS[12]{};
struct NViewport{int x,y,w,h;};
static NViewport viewportOf(const NativeView*v){
 int w=v&&v->width>0?v->width:NDEFAULT_W,h=v&&v->height>0?v->height:NDEFAULT_H;
 int renderW=w,renderH=nround((double)w*NSCENE_H/NSCENE_W);
 if(renderH>h){renderH=h;renderW=nround((double)h*NSCENE_W/NSCENE_H);}
 if(renderW<1)renderW=1;if(renderH<1)renderH=1;
 return {(w-renderW)/2,(h-renderH)/2,renderW,renderH};
}
// All painting is first composed offscreen. The approved scene is decoded and scaled
// only when the scene or the physical window size changes. A parameter edit marks
// its projected control region invalid but never replaces the visible full surface.
static void dropBackbuffers(NativeView*v){if(!v)return;
 if(v->baseDC){if(v->baseOld)NG.select(v->baseDC,v->baseOld);v->baseOld=nullptr;}
 if(v->frontDC){if(v->frontOld)NG.select(v->frontDC,v->frontOld);v->frontOld=nullptr;}
 if(v->baseBmp){NG.deleteObj(v->baseBmp);v->baseBmp=nullptr;}
 if(v->frontBmp){NG.deleteObj(v->frontBmp);v->frontBmp=nullptr;}
 if(v->baseDC){NG.deleteDC(v->baseDC);v->baseDC=nullptr;}
 if(v->frontDC){NG.deleteDC(v->frontDC);v->frontDC=nullptr;}
 v->cacheW=v->cacheH=0;v->cacheStage=-1;v->compositeDirty=true;
}
static bool ensureBackbuffers(NativeView*v,void*paintDC){
 if(v->baseDC&&v->frontDC&&v->cacheW==v->width&&v->cacheH==v->height)return true;
 dropBackbuffers(v);
 if(v->width<2||v->height<2||v->width>8192||v->height>8192)return false;
 v->baseDC=NG.compatibleDC(paintDC);v->frontDC=NG.compatibleDC(paintDC);
 if(!v->baseDC||!v->frontDC){dropBackbuffers(v);return false;}
 v->baseBmp=NG.compatibleBitmap(paintDC,v->width,v->height);
 v->frontBmp=NG.compatibleBitmap(paintDC,v->width,v->height);
 if(!v->baseBmp||!v->frontBmp){dropBackbuffers(v);return false;}
 v->baseOld=NG.select(v->baseDC,v->baseBmp);v->frontOld=NG.select(v->frontDC,v->frontBmp);
 if(!v->baseOld||!v->frontOld){dropBackbuffers(v);return false;}
 v->cacheW=v->width;v->cacheH=v->height;return true;
}
static NRect projectedRect(NativeView*v,NGuiR r,int margin){
 const NViewport q=viewportOf(v);
 int left=q.x+(r.x-margin)*q.w/NSCENE_W-3, top=q.y+(r.y-margin)*q.h/NSCENE_H-3;
 int right=q.x+(r.x+r.w+margin)*q.w/NSCENE_W+4;
 int bottom=q.y+(r.y+r.h+margin)*q.h/NSCENE_H+4;
 if(left<0)left=0;if(top<0)top=0;if(right>v->width)right=v->width;
 if(bottom>v->height)bottom=v->height;
 return {left,top,right,bottom};
}
static void invalidateZone(NativeView*v,NGuiR area,int margin=4){
 if(!v||!v->hwnd)return;v->compositeDirty=true;
 NRect r=projectedRect(v,area,margin);
 if(r.right>r.left&&r.bottom>r.top)InvalidateRect(v->hwnd,&r,0);
}
static void invalidateWhole(NativeView*v){if(!v||!v->hwnd)return;v->compositeDirty=true;InvalidateRect(v->hwnd,nullptr,0);}
static int stageOf(NativeView*v);
static void invalidateParam(NativeView*v,int id){if(!v||id<0||id>=ParamCount)return;
 if(id==SelectedId){invalidateWhole(v);return;}
 if(id>=0&&id<StageCount*StageParams){int stage=id/9;if(stage!=stageOf(v))return;
  NGuiParam c=NGUI_CONTROLS[v->kind][stage][id%9];invalidateZone(v,c.control,7);invalidateZone(v,c.value,13);return;}
 if(id>=EnableBase&&id<EnableBase+7){int stage=id-EnableBase;
  invalidateZone(v,NGUI_TOGGLE[v->kind][stage],4);
  if(stage==stageOf(v))invalidateZone(v,NGUI_STAGEPOWER[v->kind],5);return;}
 if(id>=OrderBase&&id<OrderBase+7){invalidateZone(v,NGUI_STAGE[v->kind][id-OrderBase],5);return;}
 if(id>=MasterBase&&id<MasterBase+3){auto c=NGUI_MASTERS[v->kind][id-MasterBase];invalidateZone(v,c.control,6);invalidateZone(v,c.value,13);return;}
 if(id==BypassId){invalidateZone(v,NGUI_POWER[v->kind],5);return;}
}
static bool clientToLogical(NativeView*v,int px,int py,int &x,int &y){
 auto a=viewportOf(v);if(px<a.x||py<a.y||px>=a.x+a.w||py>=a.y+a.h)return false;
 x=(int)((int64_t)(px-a.x)*NSCENE_W/a.w);y=(int)((int64_t)(py-a.y)*NSCENE_H/a.h);return true;
}

static u32 nu32(const uint8_t*p){return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);}
static u64 nu64(const uint8_t*p){return nu32(p)|((u64)nu32(p+4)<<32);}
static bool readAt(void*file,u32 off,void*ptr,u32 n){if(NG.seek(file,(int32_t)off,nullptr,0)!=off)return false;u32 got=0;return NG.read(file,ptr,n,&got,nullptr)!=0&&got==n;}
static bool loadScenes(NativeView*v){if(!v||!initNG()||!nigModule)return false;
 char path[2048]{};u32 n=GetModuleFileNameA(nigModule,path,sizeof(path));if(!n||n>=sizeof(path))return false;
 void*f=NG.file(path,0x80000000u,1,nullptr,3,0x80,nullptr);if((intptr_t)f==-1)return false;
 u32 size=NG.seek(f,0,nullptr,2);static constexpr u32 tailSize=8+7*16;uint8_t footer[tailSize]{};
 if(size<tailSize||!readAt(f,size-tailSize,footer,tailSize)||footer[0]!='N'||footer[1]!='I'||footer[2]!='G'||footer[3]!='7'||nu32(footer+4)!=7){NG.close(f);return false;}
 bool okay=true;
 for(int i=0;i<7;i++){
  u64 offset=nu64(footer+8+i*16);u64 len=nu64(footer+8+i*16+8);
  if(len<100||len>6000000||offset>size-tailSize||offset+len>size-tailSize){okay=false;break;}
  void*bytes=NG.alloc(NG.heap(),0,len);if(!bytes||!readAt(f,(u32)offset,bytes,(u32)len)){if(bytes)NG.free(NG.heap(),0,bytes);okay=false;break;}
  void*stream=NG.memoryStream((uint8_t*)bytes,(u32)len);void*image=nullptr;
  if(!stream||NG.fromStream(stream,&image)!=0||!image){if(stream){auto*vtable=*(void***)stream;auto release=(u32(__stdcall*)(void*))vtable[2];release(stream);}NG.free(NG.heap(),0,bytes);okay=false;break;}
  v->buffers[i]=bytes;v->streams[i]=stream;v->images[i]=image;v->sizes[i]=(u32)len;
 }
 NG.close(f);v->ready=okay;return okay;
}
static void freeScenes(NativeView*v){if(!v)return;
 for(int i=0;i<7;i++){
  if(v->images[i]&&NG.delImage)NG.delImage(v->images[i]);v->images[i]=nullptr;
  if(v->streams[i]){auto*vt=*(void***)v->streams[i];auto rel=(u32(__stdcall*)(void*))vt[2];rel(v->streams[i]);v->streams[i]=nullptr;}
  if(v->buffers[i]&&NG.free)NG.free(NG.heap(),0,v->buffers[i]);v->buffers[i]=nullptr;
 }v->ready=false;dropBackbuffers(v);
}
static int stageOf(NativeView*v){if(!v||!v->read)return 3;int s=nround(v->read(v->owner,SelectedId));return s<0?0:s>6?6:s;}
static double pvRead(NativeView*v,int id){return v&&v->read?v->read(v->owner,id):specs[v?v->kind:0][id].def;}
static void pvEdit(NativeView*v,int id,double x){if(!v||!v->edit||id<0||id>=ParamCount)return;
 const Param&p=specs[v->kind][id];if(x<p.lo)x=p.lo;if(x>p.hi)x=p.hi;
 if(p.step>0){int count=nround((x-p.lo)/p.step);x=p.lo+count*p.step;if(x>p.hi)x=p.hi;if(x<p.lo)x=p.lo;}
 v->edit(v->owner,id,x);invalidateParam(v,id);
}
static void paintText(void*dc,int x,int y,const char*t,u32 color){if(!NG.text)return;NG.setText(dc,color);NG.text(dc,x,y,t,nlen(t));}
static void drawCircle(void*dc,int x,int y,int r,u32 color,int thick=1){void*pen=NG.pen(0,thick,color);void*old=NG.select(dc,pen);void*brush=NG.brush(0x00E0C9A4);void*oldBrush=NG.select(dc,brush);NG.ellipse(dc,x-r,y-r,x+r,y+r);NG.select(dc,oldBrush);NG.select(dc,old);NG.deleteObj(brush);NG.deleteObj(pen);}
static void drawLine(void*dc,int x1,int y1,int x2,int y2,u32 color,int thick=2){void*p=NG.pen(0,thick,color),*old=NG.select(dc,p);NG.move(dc,x1,y1,nullptr);NG.line(dc,x2,y2);NG.select(dc,old);NG.deleteObj(p);}
static void fillRect(void*dc,int x,int y,int w,int h,u32 color){void*p=NG.pen(0,1,color),*b=NG.brush(color);void*op=NG.select(dc,p),*ob=NG.select(dc,b);NG.rectangle(dc,x,y,x+w,y+h);NG.select(dc,ob);NG.select(dc,op);NG.deleteObj(b);NG.deleteObj(p);}
static void number(char*out,int cap,double value,const Param&s){if(!out||cap<4)return;
 int dec=s.step<.1?2:s.step<1?1:0,scale=dec==2?100:dec==1?10:1;
 int n=nround(value*scale),w=0;if(n<0){out[w++]='-';n=-n;}
 int whole=n/scale,fract=n%scale;char rev[20];int r=0;do{rev[r++]=char('0'+whole%10);whole/=10;}while(whole&&r<17);
 while(r&&w<cap-6)out[w++]=rev[--r];if(dec){out[w++]='.';if(dec==2)out[w++]=char('0'+fract/10);out[w++]=char('0'+fract%10);}
 for(int j=0;s.unit&&s.unit[j]&&w<cap-2;j++)out[w++]=s.unit[j];out[w]=0;
}
// Direct GDI indicators overlay browser-generated neutral knob/canvas artwork.
static void paintControl(void*dc,NativeView*v,NGuiParam c,int id){const Param&sp=specs[v->kind][id];double val=pvRead(v,id);double t=(val-sp.lo)/(sp.hi-sp.lo);if(t<0)t=0;if(t>1)t=1;
 NGuiR r=c.control;const u32 red=0x003953A8,ink=0x004A3269,gold=0x00458FC8; // COLORREF BGR
 if(c.style==0){int idx=nround(t*128.);if(idx<0)idx=0;if(idx>128)idx=128;
  int x=r.x+r.w/2,y=r.y+r.h/2;int dx=COSINE_X[idx],dy=COSINE_Y[idx];
  drawLine(dc,x+dx*10/100,y+dy*10/100,x+dx*19/100,y+dy*19/100,ink,3);
 }else if(c.style==1){
  int yy=r.y+r.h/2,px=r.x+3+nround((r.w-6)*t);drawLine(dc,r.x+2,yy,r.x+r.w-2,yy,0x007D8D9D,3);
  drawLine(dc,r.x+2,yy,px,yy,red,4);fillRect(dc,px-5,yy-10,10,20,0x00529EC7);
 }else if(c.style==2){
  char tmp[32];number(tmp,32,val,sp);int len=nlen(tmp);
  paintText(dc,r.x+(r.w-len*6)/2,r.y+r.h/2-6,tmp,ink);
 }else if(c.style==3){
  bool enabled=val>=(sp.lo+sp.hi)*.5;
  paintText(dc,r.x+r.w/2-9,r.y+r.h/2-6,enabled?"ON":"OFF",enabled?0x00335F43:0x00524A72);
 }
 char out[48];number(out,48,val,sp);
 // HTML dynamic numeric values are blank in the baked scene.
 int x=c.value.x+c.value.w/2-(nlen(out)*6)/2;
 paintText(dc,x,c.value.y,out,red);
}
static void paintNative(NativeView*v,void*hdc){if(!v||!hdc)return;int stage=stageOf(v);
 // GDI primitive coordinates are scaled to the new 1600x1000 landscape viewport.
 // Use an anisotropic viewport transform for exact hitbox/control correspondence.
 // Scaling is set outside of this function via SetMapMode/SetWindowExtEx/SetViewportExtEx.
 void*font=NG.font(-11,0,0,0,600,0,0,0,0,0,0,0,0,"Georgia");void*oldFont=font?NG.select(hdc,font):nullptr;NG.bkMode(hdc,1);
 for(int j=0;j<9;j++)paintControl(hdc,v,NGUI_CONTROLS[v->kind][stage][j],stage*9+j);
 for(int j=0;j<3;j++)paintControl(hdc,v,NGUI_MASTERS[v->kind][j],MasterBase+j);
 for(int i=0;i<7;i++){
  int enabled=pvRead(v,EnableBase+i)>=.5; const NGuiR toggle=NGUI_TOGGLE[v->kind][i];paintText(hdc,toggle.x+10,toggle.y+4,enabled?"ON":"OFF",enabled?0x00385A34:0x003B4886);
  int order=nround(pvRead(v,OrderBase+i));
  char rank[8]={'#',char('1'+order),0};NGuiR rr=NGUI_STAGE[v->kind][i];
  paintText(hdc,rr.x+rr.w-29,rr.y+rr.h-15,rank,0x004C6A90);
 }
 { // Stage power text is never baked into the underlying scene.
  NGuiR er=NGUI_STAGEPOWER[v->kind];bool on=pvRead(v,EnableBase+stage)>=.5;
  paintText(hdc,er.x+9,er.y+7,on?"ON":"OFF",on?0x00365B42:0x00355A91);
 }
 {
  // Global FX-chain toggle: exact bounds from the HTML control registry.
  // ON/OFF status is live (not baked into the manuscript PNG).
  const NGuiR power=NGUI_POWER[v->kind];bool on=pvRead(v,BypassId)<.5;
  paintText(hdc,power.x+power.w-68,power.y+12,on?"ON":"OFF",0x00E7F4D8);
  drawCircle(hdc,power.x+power.w-(on?16:29),power.y+18,5,0x0068B4DF,2);
 }
 {
  static const char* A[]={"Prima Materia","Vitriolic Etch","Mercurial Bloom","Lapis / Polished","Tabula Rasa"};
  static const char* M[]={"Prima Nota","Hermetic Canon","Chance & Order","Ritual Measure","Tabula Rasa"};
  const char*name=v->kind?M[v->preset]:A[v->preset];
  const NGuiR r=NGUI_PRESET[v->kind];paintText(hdc,r.x+14,r.y+8,name,0x003C5269);
 }
 if(font){NG.select(hdc,oldFont);NG.deleteObj(font);}
}
static void syncNative(NativeView*v){if(!v||!v->hwnd||!v->read)return;bool changed=false;
 for(int i=0;i<ParamCount;i++){double x=pvRead(v,i);if(x-v->seen[i]>.00001||v->seen[i]-x>.00001){v->seen[i]=x;invalidateParam(v,i);changed=true;}}
 (void)changed;
}
// Hit testing uses original HTML DOM rectangles, not hand-estimated geometry.
static int hitParam(NativeView*v,int x,int y){int stage=stageOf(v);
 for(int i=0;i<9;i++){NGuiR r=NGUI_CONTROLS[v->kind][stage][i].control;if(nhit({r.x-7,r.y-7,r.w+14,r.h+16},x,y))return stage*9+i;}
 for(int i=0;i<3;i++){NGuiR r=NGUI_MASTERS[v->kind][i].control;if(nhit({r.x-5,r.y-6,r.w+10,r.h+15},x,y))return MasterBase+i;}
 return -1;
}
static NGuiParam ctrlFor(NativeView*v,int id){if(id>=MasterBase&&id<MasterBase+3)return NGUI_MASTERS[v->kind][id-MasterBase];return NGUI_CONTROLS[v->kind][id/9][id%9];}
static void setFromX(NativeView*v,int id,int x){const auto &p=specs[v->kind][id];NGuiR r=ctrlFor(v,id).control;
 double t=(double)(x-r.x)/(double)(r.w?r.w:1);if(t<0)t=0;if(t>1)t=1;pvEdit(v,id,p.lo+(p.hi-p.lo)*t);
}
static void adjustBy(NativeView*v,int id,int step){const Param&p=specs[v->kind][id];double x=pvRead(v,id);
 double inc=p.step;if(p.kind==0&&p.step<1)inc=p.step*5;
 pvEdit(v,id,x+inc*step);
}
static void presetApply(NativeView*v){
 const double au[5][7]={{1,1,1,1,1,1,1},{1.55,.65,1.3,1.08,1.36,1.1,1.2},{.55,1.6,.7,1.2,1.6,1.1,.75},{.68,.72,1.25,1,.8,1.4,1.3},{0,0,0,0,0,0,0}};
 const double mi[5][7]={{1,1,1,1,1,1,1},{.76,.45,1.12,1.48,.5,1.2,1.3},{1.12,1.2,.85,1.15,1.73,1.08,1.28},{1.33,.8,1.05,1.33,1.22,.92,1.5},{0,0,0,0,0,0,0}};
 for(int i=0;i<7;i++){
  double m=v->kind?mi[v->preset][i]:au[v->preset][i];pvEdit(v,EnableBase+i,m>0?1:0);
  for(int j=0;j<9;j++){int id=i*9+j;const auto &sp=specs[v->kind][id];
   double value=m==1||m==0?sp.def:sp.lo+(sp.def-sp.lo)*m;
   if(sp.kind==3)continue;pvEdit(v,id,value);
  }
 }
}

// Native JSON import/export through standard Windows Open/Save dialogs.
// Standalone HTML presets are a separate schema; native files preserve all 82 host parameters.
struct NFilePicker {
 u32 size;void*hwnd;void*instance;const char*filter;char*custom;u32 maxCustom,filterIndex;
 char*path;u32 maxPath;char*titlePath;u32 maxTitle;const char*startDir;const char*title;u32 flags;
 u16 fileOffset,fileExtension;const char*defaultExt;int64_t cust;void*hook;const char*templateName;
 void*reserved;u32 reservedNumber,flagsEx;
};
static_assert(sizeof(NFilePicker)==152,"Win64 OPENFILENAMEA ABI mismatch");
static_assert(offsetof(NFilePicker,path)==48,"Win64 OPENFILENAMEA file pointer offset mismatch");
static int appendTxt(char*b,int n,int max,const char*s){while(s&&*s&&n<max-1)b[n++]=*s++;b[n]=0;return n;}
static int appendNum(char*b,int n,int max,double value,const Param&sp){char tmp[48];number(tmp,48,value,{sp.name,"",sp.lo,sp.hi,sp.def,sp.step,sp.kind});return appendTxt(b,n,max,tmp);}
static const char* nskip(const char*p){while(*p==' '||*p=='\n'||*p=='\t'||*p=='\r')p++;return p;}
static bool nextNumber(const char* &p,double&value){p=nskip(p);bool neg=*p=='-';if(neg||*p=='+')p++;
 bool got=false;double v=0.;while(*p>='0'&&*p<='9'){v=v*10+(*p++-'0');got=true;}
 if(*p=='.'){p++;double x=.1;while(*p>='0'&&*p<='9'){v+=(*p++-'0')*x;x*=.1;got=true;}}
 if(!got)return false;value=neg?-v:v;return true;}
static const char* findTxt(const char*s,const char*needle){if(!s)return nullptr;for(;*s;s++){
 const char*a=s,*b=needle;while(*a&&*b&&*a==*b){a++;b++;}if(!*b)return s;
 }return nullptr;}
static bool chooseFile(NativeView*v,bool save,char*out,int cap){if(!v||!out||cap<32)return false;
 out[0]=0;void*m=LoadLibraryA("comdlg32.dll");if(!m)return false;
 auto picker=nproc<i32(__stdcall*)(NFilePicker*)>(m,save?"GetSaveFileNameA":"GetOpenFileNameA");if(!picker)return false;
 static const char filter[]="JSON preset (*.json)\0*.json\0All files (*.*)\0*.*\0\0";
 NFilePicker op{};op.size=sizeof(op);op.hwnd=v->hwnd;op.filter=filter;op.filterIndex=1;op.path=out;op.maxPath=cap;
 op.defaultExt="json";op.flags=0x00080000u|0x00000800u|(save?0x00000002u:0x00001000u);
 op.title=save?"Export Nigredo native preset":"Import Nigredo native preset";
 return picker(&op)!=0;
}
static void nativePresetFile(NativeView*v,bool save){if(!v||!v->read)return;char path[1024]{};if(!chooseFile(v,save,path,sizeof(path)))return;
 if(save){char*json=(char*)NG.alloc(NG.heap(),0,8192);if(!json)return;json[0]=0;int n=0;
 n=appendTxt(json,n,8192,"{\"format\":\"nigredo-opus-septem-cp10\",\"version\":10,\"kind\":\"");
 n=appendTxt(json,n,8192,v->kind?"midi":"audio");n=appendTxt(json,n,8192,"\",\"params\":[");
 for(int i=0;i<ParamCount;i++){if(i)n=appendTxt(json,n,8192,",");n=appendNum(json,n,8192,pvRead(v,i),specs[v->kind][i]);}
 n=appendTxt(json,n,8192,"]}\r\n");
 void*f=NG.file(path,0x40000000u,0,nullptr,2,0x80,nullptr);if((intptr_t)f==-1){NG.free(NG.heap(),0,json);return;}
 auto write=nproc<i32(__stdcall*)(void*,const void*,u32,u32*,void*)>(NG.kernel,"WriteFile");u32 wrote=0;
 if(write)write(f,json,(u32)n,&wrote,nullptr);NG.close(f);NG.free(NG.heap(),0,json);return;
 }
 void*f=NG.file(path,0x80000000u,1,nullptr,3,0x80,nullptr);if((intptr_t)f==-1)return;
 u32 size=NG.seek(f,0,nullptr,2);if(size<40||size>64000){NG.close(f);return;}
 char*bytes=(char*)NG.alloc(NG.heap(),0,size+1);if(!bytes){NG.close(f);return;}u32 got=0;
 NG.seek(f,0,nullptr,0);bool okay=NG.read(f,bytes,size,&got,nullptr)!=0&&got==size;NG.close(f);bytes[size]=0;
 if(okay)okay=(findTxt(bytes,"nigredo-opus-septem-cp06")||findTxt(bytes,"nigredo-opus-septem-cp07")||findTxt(bytes,"nigredo-opus-septem-cp08")||findTxt(bytes,"nigredo-opus-septem-cp09")||findTxt(bytes,"nigredo-opus-septem-cp10"))&&findTxt(bytes,v->kind?"\"kind\":\"midi\"":"\"kind\":\"audio\"");
 const char*start=okay?findTxt(bytes,"\"params\""):nullptr;double vals[ParamCount]{};
 if(start){start=findTxt(start,"[");if(start)start++;
  for(int i=0;start&&i<ParamCount;i++){double value=0;
   if(!nextNumber(start,value)){okay=false;break;}vals[i]=value;start=nskip(start);if(i<ParamCount-1){if(*start==',')start++;else{okay=false;break;}}}
  if(!start)okay=false;
 }else okay=false;
 if(okay)for(int i=0;i<ParamCount;i++)pvEdit(v,i,vals[i]);
 NG.free(NG.heap(),0,bytes);
}

static void clickNative(NativeView*v,int x,int y){if(!v)return;
 for(int i=0;i<7;i++)if(nhit(NGUI_TOGGLE[v->kind][i],x,y)){pvEdit(v,EnableBase+i,pvRead(v,EnableBase+i)>.5?0:1);return;}
 for(int i=0;i<7;i++)if(nhit(NGUI_STAGE[v->kind][i],x,y)||nhit(NGUI_PATH[v->kind][i],x,y)){pvEdit(v,SelectedId,i);return;}
 if(nhit(NGUI_STAGEPOWER[v->kind],x,y)){int s=stageOf(v);pvEdit(v,EnableBase+s,pvRead(v,EnableBase+s)>.5?0:1);return;}
 if(nhit(NGUI_POWER[v->kind],x,y)){pvEdit(v,BypassId,pvRead(v,BypassId)>.5?0:1);return;}
 if(nhit(NGUI_RESET[v->kind],x,y)){int s=stageOf(v);for(int j=0;j<9;j++)pvEdit(v,s*9+j,specs[v->kind][s*9+j].def);return;}
 int delta=nhit(NGUI_EARLIER[v->kind],x,y)?-1:nhit(NGUI_LATER[v->kind],x,y)?1:0;
 if(delta){int stage=stageOf(v),pos=nround(pvRead(v,OrderBase+stage)),target=pos+delta;
  if(target>=0&&target<=6)for(int i=0;i<7;i++)if(i!=stage&&nround(pvRead(v,OrderBase+i))==target){pvEdit(v,OrderBase+stage,target);pvEdit(v,OrderBase+i,pos);break;}return;
 }
 if(nhit(NGUI_PRESET[v->kind],x,y)){v->preset=(v->preset+1)%5;invalidateZone(v,NGUI_PRESET[v->kind],5);return;}
 if(nhit(NGUI_APPLY[v->kind],x,y)){presetApply(v);return;}
 if(nhit(NGUI_EXPORT[v->kind],x,y)){nativePresetFile(v,true);return;}
 if(nhit(NGUI_IMPORT[v->kind],x,y)){nativePresetFile(v,false);return;}
 int id=hitParam(v,x,y);if(id<0)return;auto c=ctrlFor(v,id);const auto&sp=specs[v->kind][id];
 if(c.style==3){pvEdit(v,id,pvRead(v,id)>=(sp.lo+sp.hi)*.5?sp.lo:sp.hi);return;}
 if(c.style==2){double val=pvRead(v,id)+sp.step;if(val>sp.hi)val=sp.lo;pvEdit(v,id,val);return;}
 if(c.style==1){setFromX(v,id,x);v->drag=id;v->dragY=y;SetCapture(v->hwnd);return;}
 v->drag=id;v->dragY=y;SetCapture(v->hwnd);
}
static int64_t __stdcall nativeWndProc(NWHND hwnd,u32 msg,u64 wp,int64_t lp){
 if(msg==WM_NCCREATE_){
  const auto*cs=(const NCreateStructA*)(uintptr_t)lp;
  NativeView*v=cs?(NativeView*)cs->createParams:nullptr;
  if(!v||!v->used||v->hwnd)return 0; // Fail creation, do not read garbage user data.
  v->hwnd=hwnd;SetWindowLongPtrA(hwnd,-21,(int64_t)(uintptr_t)v);return 1;
 }
 NativeView*v=(NativeView*)(uintptr_t)GetWindowLongPtrA(hwnd,-21);
 if(!v){return DefWindowProcA(hwnd,msg,wp,lp);}
 if(msg==WM_CREATE_)return 0;
 if(msg==WM_NCDESTROY_){
  if(v->drag>=0){ReleaseCapture();v->drag=-1;}
  SetWindowLongPtrA(hwnd,-21,0);v->hwnd=nullptr;return DefWindowProcA(hwnd,msg,wp,lp);
 }
 if(msg==WM_DESTROY_)return 0;
 if(msg==WM_ERASE_)return 1;
 if(msg==WM_PAINT_){NPaint ps{};void*dc=BeginPaint(hwnd,&ps);if(dc){
  NRect cr{};if(GetClientRect(hwnd,&cr)){
   int w=cr.right-cr.left,h=cr.bottom-cr.top;
   if(w!=v->width||h!=v->height){v->width=w;v->height=h;dropBackbuffers(v);}
  }
  if(ensureBackbuffers(v,dc)){
   int stage=stageOf(v);
   if(v->cacheStage!=stage){
    // The expensive 1600x1000 reference scene is scaled once per scene/resize.
    fillRect(v->baseDC,0,0,v->width,v->height,0x00CDD8E5);
    NViewport q=viewportOf(v);
    if(v->ready&&v->images[stage]){
     void*graphics=nullptr;
     if(NG.graphics(v->baseDC,&graphics)==0&&graphics){
      NG.draw(graphics,v->images[stage],q.x,q.y,q.w,q.h);NG.delGraphics(graphics);
     }
    }
    v->cacheStage=stage;v->compositeDirty=true;
   }
   if(v->compositeDirty){
    NG.blit(v->frontDC,0,0,v->width,v->height,v->baseDC,0,0,0x00CC0020u);
    void*gd=NG.gdi;
    auto map=nproc<i32(__stdcall*)(void*,i32)>(gd,"SetMapMode");
    auto wext=nproc<i32(__stdcall*)(void*,i32,i32,void*)>(gd,"SetWindowExtEx");
    auto vext=nproc<i32(__stdcall*)(void*,i32,i32,void*)>(gd,"SetViewportExtEx");
    auto org=nproc<i32(__stdcall*)(void*,i32,i32,void*)>(gd,"SetViewportOrgEx");
    auto save=nproc<i32(__stdcall*)(void*)>(gd,"SaveDC");
    auto restore=nproc<i32(__stdcall*)(void*,i32)>(gd,"RestoreDC");
    int old=save?save(v->frontDC):0;
    if(map&&wext&&vext&&org){NViewport vp=viewportOf(v);
     map(v->frontDC,8);wext(v->frontDC,NSCENE_W,NSCENE_H,nullptr);
     vext(v->frontDC,vp.w,vp.h,nullptr);org(v->frontDC,vp.x,vp.y,nullptr);
    }
    paintNative(v,v->frontDC);
    if(restore&&old)restore(v->frontDC,old);
    v->compositeDirty=false;
   }
   // Atomic display update: copy only Win32's invalid region from the front buffer.
   NRect rr=ps.paint;
   if(rr.left<0)rr.left=0;if(rr.top<0)rr.top=0;
   if(rr.right>v->width)rr.right=v->width;if(rr.bottom>v->height)rr.bottom=v->height;
   if(rr.right>rr.left&&rr.bottom>rr.top)
    NG.blit(dc,rr.left,rr.top,rr.right-rr.left,rr.bottom-rr.top,
            v->frontDC,rr.left,rr.top,0x00CC0020u);
  }
 }EndPaint(hwnd,&ps);return 0;}
 if(msg==WM_TIMER_){syncNative(v);return 0;}
 if(msg==WM_SIZE_){NRect r{};if(GetClientRect(hwnd,&r)){v->width=r.right;v->height=r.bottom;}dropBackbuffers(v);invalidateWhole(v);return 0;}
 if(msg==WM_LDOWN_){int x,y;
  if(clientToLogical(v,(i32)(int16_t)(lp&0xFFFF),(i32)(int16_t)((lp>>16)&0xFFFF),x,y))clickNative(v,x,y);
  return 0;}
 if(msg==WM_RDOWN_){int x,y;
  if(clientToLogical(v,(i32)(int16_t)(lp&0xFFFF),(i32)(int16_t)((lp>>16)&0xFFFF),x,y)){
   int id=hitParam(v,x,y);if(id>=0)pvEdit(v,id,specs[v->kind][id].def);
  }return 0;}
 if(msg==WM_MOVE_&&v->drag>=0){
  // Wheel and drag use the same aspect-correct inverse mapping as rendering.
  auto box=viewportOf(v);int x=(i32)(int16_t)(lp&0xFFFF),y=(i32)(int16_t)((lp>>16)&0xFFFF);
  x=(int)((int64_t)(x-box.x)*NSCENE_W/box.w);y=(int)((int64_t)(y-box.y)*NSCENE_H/box.h);
  auto &spec=specs[v->kind][v->drag];auto ctl=ctrlFor(v,v->drag);
  if(ctl.style==1)setFromX(v,v->drag,x);
  else{int delta=v->dragY-y;if(delta){double gain=(spec.hi-spec.lo)/180.;pvEdit(v,v->drag,pvRead(v,v->drag)+delta*gain);v->dragY=y;}}return 0;}
 if(msg==WM_LUP_){if(v->drag>=0){v->drag=-1;ReleaseCapture();}return 0;}
 if(msg==WM_WHEEL_){NPoint pt{(i32)(int16_t)(lp&0xFFFF),(i32)(int16_t)((lp>>16)&0xFFFF)};
  auto toClient=nproc<i32(__stdcall*)(NWHND,NPoint*)>(GetModuleHandleA("user32.dll"),"ScreenToClient");
  if(toClient)toClient(hwnd,&pt);
  int x,y;if(clientToLogical(v,pt.x,pt.y,x,y)){
   int id=hitParam(v,x,y);int wheel=(int16_t)((wp>>16)&0xFFFF);
   if(id>=0&&wheel)adjustBy(v,id,wheel>=120?wheel/120:wheel<=-120?wheel/120:wheel>0?1:-1);
  }return 0;}
 return DefWindowProcA(hwnd,msg,wp,lp);
}
static NativeView* nativeAllocate(void*owner,int kind,void(*edit)(void*,int,double),double(*read)(void*,int)){
 for(auto &v:NVIEWS)if(!v.used){v.used=true;v.owner=owner;v.kind=kind;v.edit=edit;v.read=read;v.hwnd=nullptr;v.parent=nullptr;v.oldProc=nullptr;
  v.width=NDEFAULT_W;v.height=NDEFAULT_H;v.scene=-1;v.drag=-1;v.preset=0;v.ready=false;
  v.baseDC=v.frontDC=v.baseBmp=v.frontBmp=v.baseOld=v.frontOld=nullptr;
  v.cacheW=v.cacheH=0;v.cacheStage=-1;v.compositeDirty=true;
  for(int j=0;j<ParamCount;j++)v.seen[j]=-987654.0;return &v;}return nullptr;
}
static bool ensureNativeClass(){
 static bool registered=false;if(registered)return true;
 void*u=LoadLibraryA("user32.dll");if(!u)return false;
 auto reg=nproc<u16(__stdcall*)(const NWndClassExA*)>(u,"RegisterClassExA");if(!reg||!nigModule)return false;
 NWndClassExA cls{};cls.size=sizeof(cls);cls.style=0x0003;
 cls.wndProc=(void*)(uintptr_t)&nativeWndProc;cls.instance=nigModule;cls.name=NCLASS;
 registered=reg(&cls)!=0;return registered;
}
static bool nativeAttach(NativeView*v,NWHND parent){
 if(!v||!parent||v->hwnd||!initNG()||!ensureNativeClass())return false;
 v->parent=parent;v->ready=false;
 NWHND hwnd=CreateWindowExA(0,NCLASS,"Nigredo Opus Septem Native",NCHILD|NVISIBLE,
                            0,0,v->width,v->height,parent,nullptr,nigModule,v);
 if(!hwnd)return false;
 v->hwnd=hwnd;
 if(!loadScenes(v)){DestroyWindow(hwnd);freeScenes(v);return false;}
 for(int i=0;i<ParamCount;i++)v->seen[i]=pvRead(v,i);
 SetTimer(hwnd,11,100,nullptr);invalidateWhole(v);return true;
}
static void nativeStop(NativeView*v){if(!v)return;
 if(v->hwnd){KillTimer(v->hwnd,11);if(v->drag>=0)ReleaseCapture();v->drag=-1;
  DestroyWindow(v->hwnd);v->hwnd=nullptr;}
 freeScenes(v);v->used=false;v->owner=nullptr;
}
#endif // _WIN32 shared
#if defined(NIG_MODE) && defined(CLAP_API)
static NativeView* clapview[3]{};
static NativeView* vOf(const clap_plugin*p){ClapInstance*i=inst(p);if(!i)return nullptr;int n=(int)(i-PLUGINS);return n>=0&&n<3?clapview[n]:nullptr;}
static void clapGuiEdit(void*owner,int id,double val){ClapInstance*i=(ClapInstance*)owner;setVal(i,id,val);
 __atomic_store_n(&i->dirty[id],1u,__ATOMIC_RELEASE);if(i->hostParams&&i->hostParams->request_flush)i->hostParams->request_flush(i->host);
}
static double clapGuiRead(void*owner,int id){return readVal((ClapInstance*)owner,id);}
static bool __cdecl g_api(const clap_plugin*,const char*api,bool floating){return !floating&&same(api,"win32");}
static bool __cdecl g_pref(const clap_plugin*,const char**api,bool*floating){if(!api||!floating)return false;*api="win32";*floating=false;return true;}
static bool __cdecl g_create(const clap_plugin*p,const char*api,bool floating){if(!g_api(p,api,floating))return false;ClapInstance*i=inst(p);if(!i)return false;
 int k=(int)(i-PLUGINS);if(k<0||k>=3)return false;if(!clapview[k])clapview[k]=nativeAllocate(i,NIG_MODE,clapGuiEdit,clapGuiRead);return clapview[k]!=nullptr;}
static void __cdecl g_destroy(const clap_plugin*p){ClapInstance*i=inst(p);if(!i)return;int k=(int)(i-PLUGINS);if(k>=0&&k<3&&clapview[k]){nativeStop(clapview[k]);clapview[k]=nullptr;}}
static bool __cdecl g_scale(const clap_plugin*,double){return true;}
static bool __cdecl g_size(const clap_plugin*p,u32*w,u32*h){NativeView*v=vOf(p);if(!v||!w||!h)return false;*w=v->width;*h=v->height;return true;}
static bool __cdecl g_canresize(const clap_plugin*){return true;}
static bool __cdecl g_hints(const clap_plugin*,clap_gui_resize_hints*h){if(!h)return false;h->can_resize_horizontally=true;h->can_resize_vertically=true;h->preserve_aspect_ratio=true;h->aspect_ratio_width=NSCENE_W;h->aspect_ratio_height=NSCENE_H;return true;}
static bool __cdecl g_adjust(const clap_plugin*,u32*w,u32*h){if(!w||!h)return false;if(*w<1120)*w=1120;*h=(u32)((u64)(*w)*NSCENE_H/NSCENE_W);return true;}
static bool __cdecl g_setsize(const clap_plugin*p,u32 w,u32 h){NativeView*v=vOf(p);if(!v)return false;v->width=w;v->height=h;if(v->hwnd)SetWindowPos(v->hwnd,nullptr,0,0,w,h,0x4u);return true;}
static bool __cdecl g_parent(const clap_plugin*p,const clap_window*w){NativeView*v=vOf(p);return v&&w&&same(w->api,"win32")&&nativeAttach(v,w->win32);}
static bool __cdecl g_transient(const clap_plugin*,const clap_window*){return false;}
static void __cdecl g_title(const clap_plugin*,const char*){}
static bool __cdecl g_show(const clap_plugin*p){NativeView*v=vOf(p);if(!v||!v->hwnd)return false;ShowWindow(v->hwnd,5);return true;}
static bool __cdecl g_hide(const clap_plugin*p){NativeView*v=vOf(p);if(!v||!v->hwnd)return false;ShowWindow(v->hwnd,0);return true;}
static const clap_plugin_gui NIG_GUI={g_api,g_pref,g_create,g_destroy,g_scale,g_size,g_canresize,g_hints,g_adjust,g_setsize,g_parent,g_transient,g_title,g_show,g_hide};
#endif
#ifdef NIG_VST_GUI
struct PlugView;
struct PlugViewVT{tresult(*qi)(void*,const uint8_t*,void**);u32(*add)(void*);u32(*release)(void*);tresult(*platform)(void*,const char*);tresult(*attach)(void*,void*,const char*);tresult(*remove)(void*);tresult(*wheel)(void*,float);tresult(*keyDown)(void*,u16,int16_t,int16_t);tresult(*keyUp)(void*,u16,int16_t,int16_t);tresult(*getSize)(void*,NRect*);tresult(*onSize)(void*,NRect*);tresult(*onFocus)(void*,i32);tresult(*setFrame)(void*,void*);tresult(*canResize)(void*);tresult(*checkSize)(void*,NRect*);};
struct PlugView{const PlugViewVT*vt;u32 refs;bool used;Controller*ctrl;NativeView*view;};
static PlugView PV[6]{};
static void vstGuiEdit(void*owner,int id,double value){Controller*c=(Controller*)owner;c->normalized[id]=norm(mode,id,value);
 if(c->handler){auto*vt=*(HostHandlerVTable**)c->handler;vt->begin(c->handler,id);vt->perform(c->handler,id,c->normalized[id]);vt->end(c->handler,id);}
}
static double vstGuiRead(void*owner,int id){Controller*c=(Controller*)owner;return plain(mode,id,c->normalized[id]);}
static const uint8_t NIG_UID_VIEW[16]=U4(0x5BC32507,0xD06049EA,0xA6151B52,0x2B755B29);
static tresult pv_qi(void*it,const uint8_t*id,void**out){if(!out)return INVALID_ARGUMENT;*out=nullptr;if(eq16(id,UID_UNKNOWN)||eq16(id,NIG_UID_VIEW)){*out=it;((PlugView*)it)->refs++;return OK;}return NO_INTERFACE;}
static u32 pv_add(void*it){return ++((PlugView*)it)->refs;}
static u32 pv_release(void*it){auto*v=(PlugView*)it;if(v->refs)v->refs--;if(!v->refs){if(v->view)nativeStop(v->view);v->view=nullptr;v->used=false;}return v->refs;}
static tresult pv_platform(void*,const char*type){return eqstr(type,"HWND")?OK:FALSE_RESULT;}
static tresult pv_attach(void*it,void*parent,const char*type){auto*v=(PlugView*)it;if(!eqstr(type,"HWND")||!parent)return FALSE_RESULT;
 if(!v->view)v->view=nativeAllocate(v->ctrl,mode,vstGuiEdit,vstGuiRead);return v->view&&nativeAttach(v->view,parent)?OK:FALSE_RESULT;}
static tresult pv_remove(void*it){PlugView*v=(PlugView*)it;if(v->view){nativeStop(v->view);v->view=nullptr;}return OK;}
static tresult pv_wheel(void*,float){return FALSE_RESULT;}static tresult pv_key(void*,u16,int16_t,int16_t){return FALSE_RESULT;}
static tresult pv_size(void*it,NRect*out){if(!out)return INVALID_ARGUMENT;PlugView*v=(PlugView*)it;int w=v->view?v->view->width:NDEFAULT_W,h=v->view?v->view->height:NDEFAULT_H;*out={0,0,w,h};return OK;}
static tresult pv_onSize(void*it,NRect*r){if(!r)return INVALID_ARGUMENT;PlugView*v=(PlugView*)it;if(v->view){v->view->width=r->right-r->left;v->view->height=r->bottom-r->top;if(v->view->hwnd)SetWindowPos(v->view->hwnd,nullptr,0,0,v->view->width,v->view->height,0x4);}return OK;}
static tresult pv_focus(void*,i32){return OK;}static tresult pv_frame(void*,void*){return OK;}static tresult pv_resize(void*){return OK;}
static tresult pv_check(void*,NRect*r){if(!r)return INVALID_ARGUMENT;int w=r->right-r->left;if(w<1120)w=1120;r->right=r->left+w;r->bottom=r->top+w*NSCENE_H/NSCENE_W;return OK;}
static const PlugViewVT PV_VT={pv_qi,pv_add,pv_release,pv_platform,pv_attach,pv_remove,pv_wheel,pv_key,pv_key,pv_size,pv_onSize,pv_focus,pv_frame,pv_resize,pv_check};
static void*e_view(void*it,const char*type){if(!eqstr(type,"editor"))return nullptr;for(auto&v:PV)if(!v.used){v.used=true;v.refs=1;v.vt=&PV_VT;v.ctrl=(Controller*)it;v.view=nullptr;return &v;}return nullptr;}
#endif
