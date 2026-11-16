/* Minimal independent Win64 DLL/resource lifecycle smoke test.
 * This test loads the DIAGNOSTIC SYNTHETIC-BANK bundle from disk.
 * It does not certify VST3 SDK behavior, Bitwig, real audio, or real GUI.
 */
#include <stdint.h>
typedef void* HMODULE;
__declspec(dllimport) HMODULE __stdcall LoadLibraryA(const char*);
__declspec(dllimport) void* __stdcall GetProcAddress(HMODULE,const char*);
__declspec(dllimport) int __stdcall FreeLibrary(HMODULE);
__declspec(dllimport) __attribute__((noreturn)) void __stdcall ExitProcess(unsigned int);
static const uint8_t CLS_COMP[16]={0x52,0x42,0x44,0x4b,0x10,0x21,0x49,0x82,0xb9,0x33,0x43,0x22,0x26,0x11,0x14,0x01};
static const uint8_t IID_UNKNOWN[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
typedef void* (*GetFactory)(void);
static unsigned check(void){
 const char*path="Rubedo_CP12R1_VST3_SYNTHETIC_DIAGNOSTIC.vst3\\Contents\\x86_64-win\\Rubedo Atlas Drumkit.vst3";
 HMODULE dll=LoadLibraryA(path);
 if(!dll)return 11;
 GetFactory get=(GetFactory)GetProcAddress(dll,"GetPluginFactory");
 if(!get)return 12;
 void *factory=get();if(!factory)return 13;
 const void**fvt=*(const void***)factory;
 void*component=0;
 int32_t rc=((int32_t(*)(void*,const uint8_t*,const uint8_t*,void**))fvt[6])(factory,CLS_COMP,IID_UNKNOWN,&component);
 if(rc!=0||!component)return 14;
 const void**cvt=*(const void***)component;
 /* The real Windows DllMain must have set rubedo_module to this DLL for
  * bank path lookup; this would fail otherwise (as in CP08). */
 rc=((int32_t(*)(void*,void*))cvt[3])(component,0);
 if(rc!=0)return 15;
 rc=((int32_t(*)(void*))cvt[4])(component);
 if(rc!=0)return 16;
 ((uint32_t(*)(void*))cvt[2])(component);
 if(!FreeLibrary(dll))return 17;
 return 0;
}
void __attribute__((ms_abi)) test_entry(void){ExitProcess(check());}
