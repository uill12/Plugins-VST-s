from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=r/'src/WinGui.h'
s=p.read_text()
s=s.replace('CP07: 16:10 native Win32/GDI+ GUI.', 'CP08: 16:10 corrected Rubedo CP08R2-style native Win32/GDI+ GUI.')
s=s.replace('if(size<tailSize||!readAt(f,size-tailSize,footer,tailSize)||footer[0]!=\'N\'||footer[1]!=\'I\'||footer[2]!=\'G\'||footer[3]!=\'7\'||nu32(footer+4)!=6)', 'if(size<tailSize||!readAt(f,size-tailSize,footer,tailSize)||footer[0]!=\'N\'||footer[1]!=\'I\'||footer[2]!=\'G\'||footer[3]!=\'7\'||nu32(footer+4)!=7)')
# ABI-specified Win64 CREATESTRUCT and WNDCLASSEX, as Rubedo CP08R2 corrected.
anchor='struct NPoint{i32 x,y;};'
assert anchor in s
s=s.replace(anchor,anchor+'''
// Correct Win64 API layouts. Unlike CP08R1, createParams MUST be offset 0.
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
''')
s=s.replace('static constexpr u32 WM_PAINT_=0x000F,WM_SIZE_', 'static constexpr u32 WM_NCCREATE_=0x0081,WM_NCDESTROY_=0x0082,WM_CREATE_=0x0001,WM_DESTROY_=0x0002;\nstatic constexpr u32 WM_PAINT_=0x000F,WM_SIZE_')
# New dynamic RegisterClass (avoid new static imports with nonstandard stub .lib).
needle='static NGfx NG{};'
assert needle in s
s=s.replace(needle,needle+'''
static bool ensureNativeClass();
static const char* NCLASS="NigredoOpusSeptemCP08_0_GDI_PaintFixed";
''')
# Existing declarations retained, but no subclass procedure will be used.
s=s.replace('void*oldProc=nullptr;', 'void*oldProc=nullptr;')
# Add aspect-safe viewport and client coordinate transforms after NVIEWS.
needle='static NativeView NVIEWS[12]{};'
assert needle in s
s=s.replace(needle,needle+'''
struct NViewport{int x,y,w,h;};
static NViewport viewportOf(const NativeView*v){
 int w=v&&v->width>0?v->width:NDEFAULT_W,h=v&&v->height>0?v->height:NDEFAULT_H;
 int renderW=w,renderH=nround((double)w*NSCENE_H/NSCENE_W);
 if(renderH>h){renderH=h;renderW=nround((double)h*NSCENE_W/NSCENE_H);}
 if(renderW<1)renderW=1;if(renderH<1)renderH=1;
 return {(w-renderW)/2,(h-renderH)/2,renderW,renderH};
}
static bool clientToLogical(NativeView*v,int px,int py,int &x,int &y){
 auto a=viewportOf(v);if(px<a.x||py<a.y||px>=a.x+a.w||py>=a.y+a.h)return false;
 x=(int)((int64_t)(px-a.x)*NSCENE_W/a.w);y=(int)((int64_t)(py-a.y)*NSCENE_H/a.h);return true;
}
''')
# In nativeWndProc acquire v only after nccreate registration and handle missing safely.
needle='static int64_t __stdcall nativeWndProc(NWHND hwnd,u32 msg,u64 wp,int64_t lp){NativeView*v=(NativeView*)(uintptr_t)GetWindowLongPtrA(hwnd,-21);\n if(!v)return DefWindowProcA(hwnd,msg,wp,lp);'
assert needle in s
s=s.replace(needle,'''static int64_t __stdcall nativeWndProc(NWHND hwnd,u32 msg,u64 wp,int64_t lp){
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
 if(msg==WM_DESTROY_)return 0;''')
# Adjust painting to original aspect ratio with letterbox and viewport origin.
s=s.replace('  int old=save?save(dc):0;\n  // Draw the high-resolution approved image before switching map mode;', '''  int old=save?save(dc):0;
  NViewport viewport=viewportOf(v);
  // Draw the high-resolution approved image before switching map mode;''')
s=s.replace('NG.draw(graphics,v->images[stage],0,0,v->width,v->height)', 'NG.draw(graphics,v->images[stage],viewport.x,viewport.y,viewport.w,viewport.h)')
s=s.replace('  if(map&&wext&&vext){map(dc,8);wext(dc,NSCENE_W,NSCENE_H,nullptr);vext(dc,v->width,v->height,nullptr);}', '''  if(map&&wext&&vext){
   map(dc,8);wext(dc,NSCENE_W,NSCENE_H,nullptr);vext(dc,viewport.w,viewport.h,nullptr);
   auto org=nproc<i32(__stdcall*)(void*,i32,i32,void*)>(gd,"SetViewportOrgEx");
   if(org)org(dc,viewport.x,viewport.y,nullptr);
  }''')
# update mouse controls logical transform, ignore side bars in click/hit
old=''' if(msg==WM_LDOWN_){int x=(i32)(int16_t)(lp&0xFFFF),y=(i32)(int16_t)((lp>>16)&0xFFFF);
  if(v->width>0&&v->height>0)clickNative(v,x*NSCENE_W/v->width,y*NSCENE_H/v->height);return 0;}
 if(msg==WM_RDOWN_){int x=(i32)(int16_t)(lp&0xFFFF),y=(i32)(int16_t)((lp>>16)&0xFFFF);
  int id=hitParam(v,x*NSCENE_W/(v->width?v->width:1),y*NSCENE_H/(v->height?v->height:1));if(id>=0)pvEdit(v,id,specs[v->kind][id].def);return 0;}
 if(msg==WM_MOVE_&&v->drag>=0){int x=(i32)(int16_t)(lp&0xFFFF),y=(i32)(int16_t)((lp>>16)&0xFFFF);x=x*NSCENE_W/(v->width?v->width:1);y=y*NSCENE_H/(v->height?v->height:1);
  auto &s=specs[v->kind][v->drag];auto ctl=ctrlFor(v,v->drag);
  if(ctl.style==1)setFromX(v,v->drag,x);
  else{int delta=v->dragY-y;if(delta){double gain=(s.hi-s.lo)/180.;pvEdit(v,v->drag,pvRead(v,v->drag)+delta*gain);v->dragY=y;}}return 0;}
 if(msg==WM_LUP_){if(v->drag>=0){v->drag=-1;ReleaseCapture();}return 0;}
 if(msg==WM_WHEEL_){NPoint pt{(i32)(int16_t)(lp&0xFFFF),(i32)(int16_t)((lp>>16)&0xFFFF)};{auto toClient=nproc<i32(__stdcall*)(NWHND,NPoint*)>(GetModuleHandleA("user32.dll"),"ScreenToClient");if(toClient)toClient(hwnd,&pt);}
  int x=pt.x*NSCENE_W/(v->width?v->width:1),y=pt.y*NSCENE_H/(v->height?v->height:1);int id=hitParam(v,x,y);
  int wheel=(int16_t)((wp>>16)&0xFFFF);if(id>=0)adjustBy(v,id,wheel>0?1:-1);return 0;}'''
assert old in s
new=''' if(msg==WM_LDOWN_){int x,y;
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
  }return 0;}'''
s=s.replace(old,new)
# Control allocation must set v->hwnd null before calling CreateWindowEx.
old='''static bool nativeAttach(NativeView*v,NWHND parent){if(!v||!parent||!initNG())return false;v->parent=parent;
 v->hwnd=CreateWindowExA(0,"STATIC","Nigredo Opus Septem Native",NCHILD|NVISIBLE,0,0,v->width,v->height,parent,nullptr,GetModuleHandleA(nullptr),nullptr);
 if(!v->hwnd)return false;
 SetWindowLongPtrA(v->hwnd,-21,(int64_t)(uintptr_t)v);v->oldProc=(void*)(uintptr_t)SetWindowLongPtrA(v->hwnd,-4,(int64_t)(uintptr_t)&nativeWndProc);
 loadScenes(v);SetTimer(v->hwnd,11,100,nullptr);InvalidateRect(v->hwnd,nullptr,0);
 return true;
}'''
assert old in s
new='''static bool ensureNativeClass(){
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
 SetTimer(hwnd,11,100,nullptr);InvalidateRect(hwnd,nullptr,0);return true;
}'''
s=s.replace(old,new)
s=s.replace('''  SetWindowLongPtrA(v->hwnd,-4,(int64_t)(uintptr_t)v->oldProc);SetWindowLongPtrA(v->hwnd,-21,0);DestroyWindow(v->hwnd);v->hwnd=nullptr;}''','''  DestroyWindow(v->hwnd);v->hwnd=nullptr;}''')
s=s.replace(' if(v->oldProc)return CallWindowProcA(v->oldProc,hwnd,msg,wp,lp);return DefWindowProcA(hwnd,msg,wp,lp);',' return DefWindowProcA(hwnd,msg,wp,lp);')
s=s.replace('"nigredo-opus-septem-cp07","version":7', '"nigredo-opus-septem-cp08","version":8')
s=s.replace('findTxt(bytes,"nigredo-opus-septem-cp07"))', 'findTxt(bytes,"nigredo-opus-septem-cp07")||findTxt(bytes,"nigredo-opus-septem-cp08"))')
assert 'nu32(footer+4)!=7' in s
assert 'WM_NCCREATE_' in s
p.write_text(s)
for file in ['src/Clap.cpp','src/Vst3.cpp']:
 q=r/file;t=q.read_text();t=t.replace('"0.7.0"','"0.8.0-cp08"');q.write_text(t)
print('upgraded native Win32 GUI, footer, renderer viewport, input mapping; filenames',p)
