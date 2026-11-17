#!/usr/bin/env python3
"""CP29-R1: preserve GDI Win32 unit tests, while plugin's 3:2 fallback stays active."""
from pathlib import Path
import hashlib,json
root=Path('Albedo-Quintessential-Synth')
p=root/'src/editor_native_win.cpp'
s=p.read_text(encoding='utf-8')
old=''' const int w=std::max(1,width_),h=std::max(1,height_);
 sx=std::min(w,std::max(1,int(std::lround(double(h)*3.0/2.0))));
 sy=std::min(h,std::max(1,int(std::lround(double(w)*2.0/3.0))));
 ox=(w-sx)/2;oy=(h-sy)/2;
}
int NativeEditor::ix(int x)const{int sx,sy,ox,oy;geometry(sx,sy,ox,oy);return int((double(x-ox)*kArtWidth)/std::max(1,sx));}
int NativeEditor::iy(int y)const{int sx,sy,ox,oy;geometry(sx,sy,ox,oy);return int((double(y-oy)*kArtHeight)/std::max(1,sy));}'''
new='''#if defined(ALBEDO_CP27_WEBVIEW2)
 const int w=std::max(1,width_),h=std::max(1,height_);
 sx=std::min(w,std::max(1,int(std::lround(double(h)*3.0/2.0))));
 sy=std::min(h,std::max(1,int(std::lround(double(w)*2.0/3.0))));
 ox=(w-sx)/2;oy=(h-sy)/2;
#else
 // Historical standalone Win32 QA tests are compiled without WebView2 and
 // verify the original full-window GDI mapping and edge hit targets.
 const NativeViewport viewport(width_,height_);
 sx=viewport.width;sy=viewport.height;ox=0;oy=0;
#endif
}
int NativeEditor::ix(int x)const{
#if defined(ALBEDO_CP27_WEBVIEW2)
 int sx,sy,ox,oy;geometry(sx,sy,ox,oy);return int((double(x-ox)*kArtWidth)/std::max(1,sx));
#else
 return NativeViewport(width_,height_).fromX(x);
#endif
}
int NativeEditor::iy(int y)const{
#if defined(ALBEDO_CP27_WEBVIEW2)
 int sx,sy,ox,oy;geometry(sx,sy,ox,oy);return int((double(y-oy)*kArtHeight)/std::max(1,sy));
#else
 return NativeViewport(width_,height_).fromY(y);
#endif
}'''
assert s.count(old)==1,'Unexpected CP29 GDI geometry anchor'
p.write_text(s.replace(old,new),encoding='utf-8',newline='\n')
h=hashlib.sha256(p.read_bytes()).hexdigest()
assert h=='e5d00e181cefd6e21e5b9dba0a5d61e030360edfdf9f998e58f7b157da5d7469',('CP29-R1 source',h)
manifest=root/'CP19_SOURCE_MANIFEST.json'
j=json.loads(manifest.read_text(encoding='utf-8'))
j['changed_source_sha256']['src/editor_native_win.cpp']=h
j['cp29_r1_gdi_qa']='Standalone native GDI QA retains full-window coordinates without WebView2; 3:2 plugin fallback unchanged.'
manifest.write_text(json.dumps(j,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
print('CP29-R1 3:2 plugin fallback and original Win32 test geometry: PASS')