#!/usr/bin/env python3
"""CP10 GDI native renderer: cached background + atomic front-buffer + narrow dirty rects."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]; p=r/'src/WinGui.h'
s=p.read_text()
if "static void dropBackbuffers(NativeView*v)" in s:
 print("CP10 native renderer already patched; no action");raise SystemExit(0)
s=s.replace('CP09: 16:10 corrected Rubedo CP09R2-style native Win32/GDI+ GUI.', 'CP10: double-buffered GDI+ / GDI native GUI, RGNES scene/overlay separation.')
s=s.replace('NigredoOpusSeptemCP09_', 'NigredoOpusSeptemCP10_')
s=s.replace(' void*(__stdcall*font)(i32,i32,i32,i32,i32,u32,u32,u32,u32,u32,u32,u32,u32,const char*)=nullptr;', ''' void*(__stdcall*font)(i32,i32,i32,i32,i32,u32,u32,u32,u32,u32,u32,u32,u32,const char*)=nullptr;
 void*(__stdcall*compatibleDC)(void*)=nullptr;
 void*(__stdcall*compatibleBitmap)(void*,i32,i32)=nullptr;
 i32(__stdcall*deleteDC)(void*)=nullptr;
 i32(__stdcall*blit)(void*,i32,i32,i32,i32,void*,i32,i32,u32)=nullptr;''')
s=s.replace('NG.bkMode=nproc<decltype(NG.bkMode)>(NG.gdi,"SetBkMode");NG.font=nproc<decltype(NG.font)>(NG.gdi,"CreateFontA");', '''NG.bkMode=nproc<decltype(NG.bkMode)>(NG.gdi,"SetBkMode");NG.font=nproc<decltype(NG.font)>(NG.gdi,"CreateFontA");
 NG.compatibleDC=nproc<decltype(NG.compatibleDC)>(NG.gdi,"CreateCompatibleDC");
 NG.compatibleBitmap=nproc<decltype(NG.compatibleBitmap)>(NG.gdi,"CreateCompatibleBitmap");
 NG.deleteDC=nproc<decltype(NG.deleteDC)>(NG.gdi,"DeleteDC");
 NG.blit=nproc<decltype(NG.blit)>(NG.gdi,"BitBlt");''')
s=s.replace('||!NG.setText||!NG.bkMode||!NG.font)return false;', '||!NG.setText||!NG.bkMode||!NG.font||!NG.compatibleDC||!NG.compatibleBitmap||!NG.deleteDC||!NG.blit)return false;')
s=s.replace(' int preset=0;bool ready=false;', ''' int preset=0;bool ready=false;
 void *baseDC=nullptr,*frontDC=nullptr,*baseBmp=nullptr,*frontBmp=nullptr,*baseOld=nullptr,*frontOld=nullptr;
 int cacheW=0,cacheH=0,cacheStage=-1;
 bool compositeDirty=true;''')
needle='static bool clientToLogical(NativeView*v,int px,int py,int &x,int &y){'
pos=s.index(needle)
new='''// All painting is first composed offscreen. The approved scene is decoded and scaled
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
''' 
s=s[:pos]+new+s[pos:]
s=s.replace('v->edit(v->owner,id,x);if(v->hwnd)InvalidateRect(v->hwnd,nullptr,0);','v->edit(v->owner,id,x);invalidateParam(v,id);')
s=s.replace('void*font=NG.font(-13,0,0,0,600,0,0,0,0,0,0,0,0,"Georgia");','void*font=NG.font(-11,0,0,0,600,0,0,0,0,0,0,0,0,"Georgia");')
s=s.replace(''' if(pvRead(v,EnableBase+stage)<.5){NGuiR er=NGUI_STAGEPOWER[v->kind];fillRect(hdc,er.x+3,er.y+3,er.w-6,er.h-6,0x00B5C6E2);paintText(hdc,er.x+10,er.y+9,"OFF",0x00355A91);}''',''' { // Stage power text is never baked into the underlying scene.
  NGuiR er=NGUI_STAGEPOWER[v->kind];bool on=pvRead(v,EnableBase+stage)>=.5;
  paintText(hdc,er.x+9,er.y+7,on?"ON":"OFF",on?0x00365B42:0x00355A91);
 }''')
s=s.replace('''  fillRect(hdc,power.x+power.w-70,power.y+7,31,power.h-13,on?0x00A8CFAB:0x00C7B2AD);
  paintText(hdc,power.x+power.w-66,power.y+10,on?"ON":"OFF",0x00463833);
  fillRect(hdc,power.x+power.w-30+(on?12:0),power.y+12,10,10,0x00B6DCF6);''','''  paintText(hdc,power.x+power.w-68,power.y+12,on?"ON":"OFF",0x00E7F4D8);
  drawCircle(hdc,power.x+power.w-(on?16:29),power.y+18,5,0x0068B4DF,2);''')
s=s.replace('''  if(v->preset>0){const NGuiR r=NGUI_PRESET[v->kind];fillRect(hdc,r.x+9,r.y+5,r.w-46,r.h-10,0x00CAD9E9);paintText(hdc,r.x+14,r.y+7,name,0x003C5269);}''','''  const NGuiR r=NGUI_PRESET[v->kind];paintText(hdc,r.x+14,r.y+8,name,0x003C5269);''')
s=s.replace(''' for(int i=0;i<ParamCount;i++){double x=pvRead(v,i);if(x-v->seen[i]>.00001||v->seen[i]-x>.00001){v->seen[i]=x;changed=true;}}
 if(changed)InvalidateRect(v->hwnd,nullptr,0);''',''' for(int i=0;i<ParamCount;i++){double x=pvRead(v,i);if(x-v->seen[i]>.00001||v->seen[i]-x>.00001){v->seen[i]=x;invalidateParam(v,i);changed=true;}}
 (void)changed;''')
s=s.replace('v->preset=(v->preset+1)%5;InvalidateRect(v->hwnd,nullptr,0);return;', 'v->preset=(v->preset+1)%5;invalidateZone(v,NGUI_PRESET[v->kind],5);return;')
start=s.index(' if(msg==WM_PAINT_){')
end=s.index(' if(msg==WM_TIMER_){',start)
replace=''' if(msg==WM_PAINT_){NPaint ps{};void*dc=BeginPaint(hwnd,&ps);if(dc){
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
'''
s=s[:start]+replace+s[end:]
s=s.replace('}InvalidateRect(hwnd,nullptr,0);return 0;}','}dropBackbuffers(v);invalidateWhole(v);return 0;}')
s=s.replace(''' }v->ready=false;
}
static int stageOf''',''' }v->ready=false;dropBackbuffers(v);
}
static int stageOf''')
s=s.replace('''  v.width=NDEFAULT_W;v.height=NDEFAULT_H;v.scene=-1;v.drag=-1;v.preset=0;v.ready=false;''','''  v.width=NDEFAULT_W;v.height=NDEFAULT_H;v.scene=-1;v.drag=-1;v.preset=0;v.ready=false;
  v.baseDC=v.frontDC=v.baseBmp=v.frontBmp=v.baseOld=v.frontOld=nullptr;
  v.cacheW=v.cacheH=0;v.cacheStage=-1;v.compositeDirty=true;''')
s=s.replace(''' SetTimer(hwnd,11,100,nullptr);InvalidateRect(hwnd,nullptr,0);return true;''',''' for(int i=0;i<ParamCount;i++)v->seen[i]=pvRead(v,i);
 SetTimer(hwnd,11,100,nullptr);invalidateWhole(v);return true;''')
# Update native preset JSON formats
s=s.replace('"nigredo-opus-septem-cp09","version":9','"nigredo-opus-septem-cp10","version":10')
s=s.replace('findTxt(bytes,"nigredo-opus-septem-cp09"))','findTxt(bytes,"nigredo-opus-septem-cp09")||findTxt(bytes,"nigredo-opus-septem-cp10"))')
p.write_text(s)
for other in ['src/Clap.cpp','src/Vst3.cpp']:
 q=r/other;t=q.read_text().replace('0.9.0-cp09','0.9.1-cp10');q.write_text(t)
print('Updated',p,'lines',len(s.splitlines()))
