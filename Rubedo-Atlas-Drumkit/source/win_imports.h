#pragma once
#ifdef _WIN32
#include <stdint.h>
#define WINCALL __attribute__((ms_abi))
#define WINIMPORT __declspec(dllimport)
typedef void* HANDLE;
typedef void* HMODULE;
WINIMPORT uint32_t WINCALL GetModuleFileNameA(HMODULE,char*,uint32_t);
WINIMPORT HANDLE WINCALL CreateFileA(const char*,uint32_t,uint32_t,void*,uint32_t,uint32_t,HANDLE);
WINIMPORT int WINCALL ReadFile(HANDLE,void*,uint32_t,uint32_t*,void*);
WINIMPORT uint32_t WINCALL SetFilePointer(HANDLE,int32_t,int32_t*,uint32_t);
WINIMPORT int WINCALL CloseHandle(HANDLE);
WINIMPORT void* WINCALL VirtualAlloc(void*,uint64_t,uint32_t,uint32_t);
WINIMPORT int WINCALL VirtualFree(void*,uint64_t,uint32_t);
#define RUBEDO_INVALID_HANDLE ((HANDLE)-1)
#define RUBEDO_MEM_COMMIT 0x1000u
#define RUBEDO_MEM_RESERVE 0x2000u
#define RUBEDO_MEM_RELEASE 0x8000u
#define RUBEDO_PAGE_READWRITE 4u
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif