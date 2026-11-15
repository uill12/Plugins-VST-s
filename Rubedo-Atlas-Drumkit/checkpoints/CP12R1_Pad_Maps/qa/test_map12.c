#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdatomic.h>
#include "rubedo_engine.h"
static void seq(RubedoEngine*e,const int16_t*a){
 for(int i=0;i<RUBEDO_PADS;i++)assert(atomic_load(&e->mappings[i])==a[i]);
}
int main(void){
 RubedoEngine e;rubedo_init_engine(&e);
 char text[320]={0};int len=rubedo_map_encode(&e,text,sizeof(text));
 printf("export default len=%d\n",len);assert(len==257);
 assert(!memcmp(text,"RUBEDO_PADMAP_V1\n",17));
 int16_t notes[24]={0};assert(rubedo_map_decode(text,len,notes));
 for(int j=0;j<24;j++)assert(notes[j]==36+j);
 assert(rubedo_assign_note(&e,21,72));assert(rubedo_assign_note(&e,0,56));
 int16_t snap[24];for(int j=0;j<24;j++)snap[j]=atomic_load(&e.mappings[j]);
 len=rubedo_map_encode(&e,text,sizeof(text));assert(len==257);
 assert(rubedo_map_decode(text,len,notes));seq(&e,notes);
 assert(rubedo_assign_note(&e,0,112));assert(atomic_load(&e.mappings[0])==112);
 rubedo_import_mappings(&e,notes);seq(&e,snap);
 // Duplicate, out of range, invalid ordering, missing line, unexpected suffix:
 char tmp[330];memcpy(tmp,text,len);tmp[len]=0;
 tmp[17+10+6]='0';tmp[17+10+7]='3';tmp[17+10+8]='6';
 for(int i=0;i<24;i++)notes[i]=-7;
 assert(!rubedo_map_decode(tmp,len,notes));for(int i=0;i<24;i++)assert(notes[i]==-7);
 memcpy(tmp,text,len);tmp[17+10+6]='2';tmp[17+10+7]='5';tmp[17+10+8]='6';assert(!rubedo_map_decode(tmp,len,notes));
 memcpy(tmp,text,len);tmp[17+10]='P';tmp[17+10+4]='9';assert(!rubedo_map_decode(tmp,len,notes));
 assert(!rubedo_map_decode(text,len-10,notes));
 memcpy(tmp,text,len);tmp[len]='X';assert(!rubedo_map_decode(tmp,len+1,notes));
 // CRLF, UTF-8 BOM supported for manually edited maps.
 char formatted[350];int n=0;formatted[n++]=(char)0xef;formatted[n++]=(char)0xbb;formatted[n++]=(char)0xbf;
 for(int i=0;i<len;i++){if(text[i]=='\n')formatted[n++]='\r';formatted[n++]=text[i];}
 assert(rubedo_map_decode(formatted,n,notes));seq(&e,notes);
 assert(!rubedo_map_encode(&e,text,20));
 // Save/Load model doesn't alter sound parameters and is independent of the selected kit.
 double gain=e.params[1];int kit=e.kit;
 rubedo_import_mappings(&e,notes);assert(e.params[1]==gain&&e.kit==kit);
 puts("PASS: 24 assignments, cross-project portability, duplicate/out-of-range rejection, UTF8 BOM/CRLF, unchanged DSP");
 return 0;
}