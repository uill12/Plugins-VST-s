from pathlib import Path
p=Path(__file__).resolve().parents[1]/'src/WinGui.h'
s=p.read_text()
marker='static void clickNative(NativeView*v,int x,int y){if(!v)return;'
code=r'''
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
 if(save){char json[8192]{};int n=0;
 n=appendTxt(json,n,sizeof(json),"{\"format\":\"nigredo-opus-septem-cp06\",\"version\":6,\"kind\":\"");
 n=appendTxt(json,n,sizeof(json),v->kind?"midi":"audio");n=appendTxt(json,n,sizeof(json),"\",\"params\":[");
 for(int i=0;i<ParamCount;i++){if(i)n=appendTxt(json,n,sizeof(json),",");n=appendNum(json,n,sizeof(json),pvRead(v,i),specs[v->kind][i]);}
 n=appendTxt(json,n,sizeof(json),"]}\r\n");
 void*f=NG.file(path,0x40000000u,0,nullptr,2,0x80,nullptr);if((intptr_t)f==-1)return;
 auto write=nproc<i32(__stdcall*)(void*,const void*,u32,u32*,void*)>(NG.kernel,"WriteFile");u32 wrote=0;
 if(write)write(f,json,(u32)n,&wrote,nullptr);NG.close(f);return;
 }
 void*f=NG.file(path,0x80000000u,1,nullptr,3,0x80,nullptr);if((intptr_t)f==-1)return;
 u32 size=NG.seek(f,0,nullptr,2);if(size<40||size>64000){NG.close(f);return;}
 char*bytes=(char*)NG.alloc(NG.heap(),0,size+1);if(!bytes){NG.close(f);return;}u32 got=0;
 NG.seek(f,0,nullptr,0);bool okay=NG.read(f,bytes,size,&got,nullptr)!=0&&got==size;NG.close(f);bytes[size]=0;
 if(okay)okay=findTxt(bytes,"nigredo-opus-septem-cp06")&&findTxt(bytes,v->kind?"\"kind\":\"midi\"":"\"kind\":\"audio\"");
 const char*start=okay?findTxt(bytes,"\"params\""):nullptr;double vals[ParamCount]{};
 if(start){start=findTxt(start,"[");if(start)start++;
  for(int i=0;start&&i<ParamCount;i++){double value=0;
   if(!nextNumber(start,value)){okay=false;break;}vals[i]=value;start=nskip(start);if(i<ParamCount-1){if(*start==',')start++;else{okay=false;break;}}}
  if(!start)okay=false;
 }else okay=false;
 if(okay)for(int i=0;i<ParamCount;i++)pvEdit(v,i,vals[i]);
 NG.free(NG.heap(),0,bytes);
}
'''
assert marker in s
s=s.replace(marker,code+'\n'+marker)
s=s.replace('if(nhit(NGUI_APPLY[v->kind],x,y)){presetApply(v);return;}','if(nhit(NGUI_APPLY[v->kind],x,y)){presetApply(v);return;}\n if(nhit(NGUI_EXPORT[v->kind],x,y)){nativePresetFile(v,true);return;}\n if(nhit(NGUI_IMPORT[v->kind],x,y)){nativePresetFile(v,false);return;}')
# Avoid false GDI+ double-paint reference in compiled code
s=s.replace('if(v->preset>0){const char*n=v->kind?"MIDI FORMULA":"AUDIO FORMULA";paintText(hdc,974,1182,n,0x003C6289);}', '''{
  static const char* A[]={"Prima Materia","Vitriolic Etch","Mercurial Bloom","Lapis / Polished","Tabula Rasa"};
  static const char* M[]={"Prima Nota","Hermetic Canon","Chance & Order","Ritual Measure","Tabula Rasa"};
  const char*name=v->kind?M[v->preset]:A[v->preset];
  if(v->preset>0){fillRect(hdc,908,1210,317,22,0x00CAD9E9);paintText(hdc,917,1211,name,0x003C5269);}
 }''')
# Stage-power indicator overlay
s=s.replace('if(pvRead(v,BypassId)>=.5)', '''if(pvRead(v,EnableBase+stage)<.5){NGuiR er=NGUI_STAGEPOWER[v->kind];fillRect(hdc,er.x+3,er.y+3,er.w-6,er.h-6,0x00B5C6E2);paintText(hdc,er.x+10,er.y+9,"OFF",0x00355A91);}
 if(pvRead(v,BypassId)>=.5)''')
p.write_text(s)
