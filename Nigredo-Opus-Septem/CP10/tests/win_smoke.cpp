extern "C" __declspec(dllexport) int test(){return 42;}
extern "C" int __stdcall DllMain(void*,unsigned long,void*){return 1;}
