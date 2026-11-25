#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "../source/rubedo_engine.h"
#include "../source/rubedo_role_balance.inc"
static uint32_t rd32(const unsigned char*p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint64_t rd64(const unsigned char*p){uint64_t z=0;for(int i=0;i<8;i++)z|=((uint64_t)p[i])<<(8*i);return z;}
int main(int argc,char**argv){
 if(argc!=2){fprintf(stderr,"Usage: test_role_balance source_full.clap\n");return 2;}
 int fd=open(argv[1],O_RDONLY);if(fd<0)return 3;struct stat st;if(fstat(fd,&st))return 3;
 unsigned char*mm=mmap(0,st.st_size,PROT_READ,MAP_PRIVATE,fd,0);if(mm==MAP_FAILED)return 3;
 unsigned char*foot=mm+st.st_size-32;
 if(memcmp(foot,"RBOVR103",8)!=0)return 4;
 uint64_t bank_offset=rd64(foot+8),bank_size=rd64(foot+16);
 if(bank_offset+bank_size+32!=(uint64_t)st.st_size||bank_size!=373714116ull)return 5;
 const unsigned char *bank=mm+bank_offset;
 if(memcmp(bank,"RBDK24",6)!=0)return 6;
 rubedo_bank.data=bank;rubedo_bank.bytes=(uint32_t)bank_size;
 for(int i=0;i<2304;i++){
  rubedo_bank.regions[i].offset=rd32(bank+24+8*i);
  rubedo_bank.regions[i].frames=rd32(bank+28+8*i);
 }
 for(int kit=0;kit<24;kit++)for(int pad=0;pad<24;pad++){
  float v=rubedo_role_gain[kit][pad];if(v<.77f||v>1.53f){fprintf(stderr,"bad trim %d %d %.6f\n",kit,pad,v);return 7;}
 }
 const int pads[]={0,2,3,4,5,9,11,13,23};
 double averages[9]={0},minv[9],maxv[9]={0};for(int i=0;i<9;i++)minv[i]=1000;
 float l[256],r[256];
 for(int kit=0;kit<24;kit++)for(int z=0;z<9;z++){
   RubedoEngine e;rubedo_init_engine(&e);rubedo_activate(&e,48000);
   rubedo_set_param(&e,0,(double)kit);rubedo_trigger_pad(&e,pads[z],.88);
   double en=0;int samples=0;float peak=0;
   for(int block=0;block<40;block++){
     rubedo_render(&e,l,r,0,0,256);
     for(int k=0;k<256;k++){en+=l[k]*l[k]+r[k]*r[k];samples+=2;
       float m=fabsf(l[k]);if(fabsf(r[k])>m)m=fabsf(r[k]);if(m>peak)peak=m;}
   }
   double rms=sqrt(en/samples);
   if(!isfinite(rms)||!isfinite(peak)||peak>0.9999f||rms<=1e-4){fprintf(stderr,"invalid render kit=%d pad=%d rms=%.6f peak=%.3f\n",kit,pads[z],rms,peak);return 8;}
   averages[z]+=rms;if(rms<minv[z])minv[z]=rms;if(rms>maxv[z])maxv[z]=rms;
 }
 for(int i=0;i<9;i++)printf("PAD%02d mean_RMS=%.6f spread=%.2fx\n",pads[i],averages[i]/24,maxv[i]/minv[i]);
 if(!(averages[0]>averages[2]*2.0&&averages[1]>averages[3]*1.1)){fprintf(stderr,"hierarchy lost\n");return 9;}
 for(int kit=0;kit<24;kit++){
   RubedoEngine e;rubedo_init_engine(&e);rubedo_activate(&e,48000);rubedo_set_param(&e,0,kit);
   int combo[]={0,2,4,7,9,11};for(int i=0;i<6;i++)rubedo_trigger_pad(&e,combo[i],.88);
   float peak=0;double energy=0;for(int block=0;block<60;block++){rubedo_render(&e,l,r,0,0,256);
    for(int i=0;i<256;i++){float a=fmaxf(fabsf(l[i]),fabsf(r[i]));if(a>peak)peak=a;energy+=l[i]*l[i]+r[i]*r[i];}}
   if(peak>1.0f||!isfinite(energy)||energy<.01){fprintf(stderr,"bad layer kit=%d peak=%.3f\n",kit,peak);return 10;}
 }
 puts("PASS: 24 kits x 9 instruments, role-balanced render, 6-layer mixes, 0 clips, timbral hierarchy maintained");
 munmap(mm,st.st_size);close(fd);return 0;
}
