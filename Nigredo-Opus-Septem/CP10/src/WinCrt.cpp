#ifdef _WIN32
#include <stddef.h>
extern "C" { int _fltused=0;
void* memset(void*d,int c,size_t n){volatile unsigned char*p=(volatile unsigned char*)d;for(size_t i=0;i<n;i++)p[i]=(unsigned char)c;return d;}
void* memcpy(void*d,const void*s,size_t n){volatile unsigned char*p=(volatile unsigned char*)d;const unsigned char*q=(const unsigned char*)s;for(size_t i=0;i<n;i++)p[i]=q[i];return d;}
void* memmove(void*d,const void*s,size_t n){unsigned char*p=(unsigned char*)d;const unsigned char*q=(const unsigned char*)s;if(p<q){for(size_t i=0;i<n;i++)p[i]=q[i];}else{for(size_t i=n;i>0;i--)p[i-1]=q[i-1];}return d;}
}
#endif
