#pragma once
// Nigredo Opus Septem CP05: real-time-safe reference DSP/MIDI kernels.
// Self-contained, no heap allocation or external audio SDK at processing time.
#include "SharedParams.h"
namespace nig {
static constexpr double PI=3.14159265358979323846264338328;
inline double ab(double x){return x<0?-x:x;}
inline double mn(double a,double b){return a<b?a:b;}
inline double mx(double a,double b){return a>b?a:b;}
inline int imod(int x,int mod){int y=x%mod;return y<0?y+mod:y;}
inline double dbgain(double db){ // accurate to about 0.04%, using repeated squaring exp approximation
 double x=db*.11512925464970229; // ln(10)/20
 int n=16;double t=1.+x/65536.; for(int i=0;i<n;i++)t*=t;return t;
}
inline double fastSin(double x){ // range reduce then polynomial, suitable for slow modulation
 int64_t turns=(int64_t)(x/(2*PI)+(x>=0?.5:-.5));x-=turns*2*PI;
 double x2=x*x;return x*(1.-x2*(1./6.-x2*(1./120.-x2*(1./5040.-x2/362880.))));
}
inline double softclip(double x){ x=clamp(x,-8.,8.);double a=x*x;double z=x*(27.+a)/(27.+9.*a);return clamp(z,-1.,1.); }
inline uint32_t randomStep(uint32_t &s){s^=s<<13;s^=s>>17;s^=s<<5;return s;}
inline double random01(uint32_t &s){return (randomStep(s)>>8)*(1./16777216.);}
inline double roundNearest(double v){return v>=0?(int64_t)(v+.5):(int64_t)(v-.5);}
struct Pair {double l,r;};

struct AudioEngine {
 static constexpr int DELAY=131072, CHORUS=8192, LOOK=2048;
 double p[ParamCount],smooth[ParamCount],sampleRate=48000.,globalEnv=0.,lfo=0.,lastIn[2]{};
 double pre[2]{},sepLo[2]{},conjPrev[2]{},eqLo[2]{},eqMid[2]{},chorusDamp[2]{},tailDamp[2]{},gain=1.;
 float delay[2][DELAY]{},chorus[2][CHORUS]{},look[2][LOOK]{},dry[2][LOOK]{},wet[2][LOOK]{};
 uint32_t dpos=0,cpos=0,lpos=0,finalPos=0,noise=0xa51f0821;double meterIn=0.,meterOut=0.;
 void defaults(){for(int i=0;i<ParamCount;i++)p[i]=smooth[i]=specs[0][i].def;}
 void prepare(double sr){sampleRate=clamp(sr,22050.,192000.);globalEnv=0.;gain=1.;lfo=0.;dpos=cpos=lpos=finalPos=0;
  for(int c=0;c<2;c++){pre[c]=sepLo[c]=conjPrev[c]=eqLo[c]=eqMid[c]=chorusDamp[c]=tailDamp[c]=lastIn[c]=0.;
   for(int j=0;j<DELAY;j++)delay[c][j]=0.;for(int j=0;j<CHORUS;j++)chorus[c][j]=0.;for(int j=0;j<LOOK;j++){look[c][j]=0.;dry[c][j]=0.;wet[c][j]=0.;}}
 }
 void set(int id,double value){if(id>=0&&id<ParamCount)p[id]=clamp(value,specs[0][id].lo,specs[0][id].hi);}
 double get(int id)const{return id>=0&&id<ParamCount?p[id]:0;}
 double v(int stage,int n)const{return smooth[stage*9+n];}
 double pct(int stage,int n)const{return .01*v(stage,n);}
 void smoothParams(){double coef=1./(1.+sampleRate*.002);for(int i=0;i<ParamCount;i++){
   if(i>=EnableBase&&i<=SelectedId)smooth[i]=p[i];else smooth[i]+=coef*(p[i]-smooth[i]);}
 }
 int stageAt(int slot)const{int winner=slot; // order values represent per-stage slots. Resolve duplicate positions deterministically.
  int order[7]={0,1,2,3,4,5,6};for(int i=0;i<7;i++)for(int j=i+1;j<7;j++)
   if(p[OrderBase+order[j]]<p[OrderBase+order[i]]){int t=order[i];order[i]=order[j];order[j]=t;}
  return order[winner];}
 Pair effect(int id,Pair x){
  switch(id){
   case 0:{ // CALCINATION: pre-emphasized anti-alias-aware drive with wet and trim.
    double heat=pct(0,0),d=1.+v(0,1)*.13,wet=pct(0,2),harm=pct(0,3),bias=(v(0,4)-50.)*.007;
    double body=pct(0,5),preCut=pct(0,6),trans=pct(0,7),trim=dbgain(v(0,8));
    double ch[2]={x.l,x.r},out[2];
    int q=(int)smooth[MasterBase+2];if(q>=6)q=8;else if(q>=3)q=4;else if(q==2)q=2;else q=1;
    for(int c=0;c<2;c++){
     double src=ch[c],high=src-pre[c],a=.02+.32*preCut;pre[c]+=a*high;
     double driven=(src*(.6+.9*body)+high*(.1+.9*trans)+bias)*(1.+heat*1.5)*d;
     double last=lastIn[c],sum=0.;for(int i=0;i<q;i++){double t=(i+1.)/q;sum+=softclip(last+(driven-last)*t);}
     lastIn[c]=driven;double sat=sum/q,bright=sat+harm*.18*(sat*sat*sat-sat);
     out[c]=(src*(1.-wet)+bright*wet*trim*dbgain(-v(0,1)*.085));
    }return {out[0],out[1]};}
   case 1:{ // DISSOLUTION: stereo crossfed feedback diffusion with controllable damping.
    double diffuse=pct(1,0),feedback=(.12+.78*pct(1,1))*.93,wet=pct(1,2),predelay=v(1,3)*.001;
    double spread=pct(1,4),damp=pct(1,5),early=pct(1,6),rate=v(1,7),tail=pct(1,8);
    double ch[2]={x.l,x.r},out[2];
    for(int c=0;c<2;c++){
     double wav=fastSin(lfo*rate*(c?1.07:1.)+c*2.5);
     int d=(int)(sampleRate*(predelay+.03+.22*diffuse+(c?spread*.012:0.)+wav*diffuse*.003));
     if(d<1)d=1;if(d>=DELAY)d=DELAY-1;double delayed=delay[c][(dpos+DELAY-(uint32_t)d)%DELAY];
     double low=(.01+.22*(1.-damp))*(delayed-tailDamp[c]);tailDamp[c]+=low;
     double cross=delay[1-c][(dpos+DELAY-(uint32_t)(d+17))%DELAY];
     double regen=(tailDamp[c]*(1.-.45*spread)+cross*.45*spread)*feedback;
     delay[c][dpos]=(float)clamp(ch[c]*(.25+.75*early)+regen,-3.,3.);
     double blurred=delayed*(1.-tail*.4)+tailDamp[c]*tail*.4;
     out[c]=ch[c]*(1.-wet)+wet*blurred;
    }dpos=(dpos+1u)%DELAY;return {out[0],out[1]};}
   case 2:{ // SEPARATION: complementary low/high, mid/side and transient structure.
    double freq=v(2,0),focus=pct(2,1),balance=(v(2,2)-50.)*.01,ms=(v(2,3)-50.)*.01;
    double trans=(pct(2,4)-.45)*1.2,body=(pct(2,5)-.52)*1.6;
    double slope=v(2,6),phase=pct(2,7),link=pct(2,8);
    double src[2]={x.l,x.r},out[2];
    double a=clamp(2.*PI*freq/sampleRate,.0005,.65);
    for(int c=0;c<2;c++){
      sepLo[c]+=a*(src[c]-sepLo[c]);double lo=sepLo[c],hi=src[c]-lo;
      double altered=lo*(1.+body)+hi*(1.+trans)*(1.+(slope-6.)*.002);
      altered=src[c]*(1.-focus*.9)+altered*focus*.9;
      out[c]=altered*(1.+(c?balance:-balance)*.15);
    }
    double mid=.5*(out[0]+out[1]),side=.5*(out[0]-out[1]);side*=clamp(1.+ms*(1.-link*.7)*(phase*1.3),.2,2.5);
    return {mid+side,mid-side};}
   case 3:{ // CONJUNCTION: harmonic re-combination and stereo coherence.
    double u=pct(3,0),harm=pct(3,1),blend=pct(3,2),width=pct(3,3),coh=pct(3,4),fund=pct(3,5);
    double detune=v(3,6)*.001,xf=pct(3,7)*.3,ceil=dbgain(v(3,8));
    double mid=(x.l+x.r)*.5,side=(x.l-x.r)*.5*(.65+width*.9);
    double h=softclip(mid*(1.+harm*2.))*(.7+.3*fund)-mid*(.08*harm);
    double add=(h-mid)*blend*u;
    double l=(mid+side+add)*(1.-xf)+x.r*xf*.4;
    double r=(mid-side+add)*(1.-xf)+x.l*xf*.4;
    l=l*(.8+.2*coh)+conjPrev[0]*detune; r=r*(.8+.2*coh)+conjPrev[1]*detune;
    conjPrev[0]=l;conjPrev[1]=r;return {clamp(l,-ceil,ceil),clamp(r,-ceil,ceil)};}
   case 4:{ // FERMENTATION: modulated delay, living texture, bounded feedback.
    double move=pct(4,0),growth=pct(4,1),texture=pct(4,2),rate=v(4,3),feedback=pct(4,4)*.65;
    double stereo=pct(4,5),damp=pct(4,7),chaos=pct(4,8),seedPhase=(v(4,6)/999.)*2.*PI;
    double src[2]={x.l,x.r},out[2];
    for(int c=0;c<2;c++){
      double wobble=fastSin(lfo*rate*(1.+chaos*.15)+c*(.8+stereo*2.3)+seedPhase);
      double delayMs=2.5+move*(3.+growth*10.)+(2.+growth*5.)*wobble;
      int d=(int)(mx(1.,delayMs*.001*sampleRate));if(d>=CHORUS)d=CHORUS-1;
      float echo=chorus[c][(cpos+CHORUS-(uint32_t)d)%CHORUS];
      chorusDamp[c]+=(.007+.2*(1.-damp))*(echo-chorusDamp[c]);
      double fb=chorusDamp[c]*feedback;chorus[c][cpos]=(float)clamp(src[c]+fb,-3.,3.);
      out[c]=src[c]*(1.-move*.28)+chorusDamp[c]*(.06+texture*.36)*move;
    }cpos=(cpos+1)%CHORUS;return {out[0],out[1]};}
   case 5:{ // DISTILLATION: smooth low/high spectral rebalancing, dynamic refinement.
    double purity=pct(5,0),air=pct(5,1),weight=pct(5,2),center=v(5,3),res=pct(5,4),dynamic=pct(5,5),q=v(5,6),tilt=v(5,7),soft=pct(5,8);
    double a=clamp(2.*PI*center/sampleRate,.0005,.6),b=clamp(a/(1.+q),.0002,.5);
    double src[2]={x.l,x.r},out[2];
    for(int c=0;c<2;c++){
      eqLo[c]+=a*(src[c]-eqLo[c]);eqMid[c]+=b*(src[c]-eqMid[c]);
      double hi=src[c]-eqLo[c];double mid=eqMid[c]-eqLo[c];
      double result=src[c]+(weight-.45)*.6*eqLo[c]+(air-.48)*.9*hi;
      result+=(res-.25)*mid*(.6+.6*purity);
      result*=dbgain(tilt*(hi>=0?1:-1)*.025);
      if(dynamic>0.)result=src[c]*(dynamic*.08)+result*(1.-dynamic*.08);
      out[c]=src[c]*(1.-purity*.7)+softclip(result*(1.-soft*.035))*purity*.7;
    }return {out[0],out[1]};}
   case 6:{ // COAGULATION: linked dynamics, fixed latency with lookahead.
    double glue=pct(6,0),impact=pct(6,1),output=dbgain(v(6,2));
    double attack=v(6,3),release=v(6,4),ratio=v(6,5),knee=pct(6,6),lookMs=v(6,7),ceil=dbgain(v(6,8));
    // Constant 10 ms stereo latency: detector anticipates the delayed program
    // by the selected LOOKAHEAD amount. This is reported through CLAP/VST3.
    int constantDelay=(int)(sampleRate*.010+.5);if(constantDelay>=LOOK)constantDelay=LOOK-1;
    int future=(int)(sampleRate*lookMs*.001+.5);if(future>constantDelay)future=constantDelay;
    int detectorDelay=constantDelay-future;
    double detectorL=detectorDelay?look[0][(lpos+LOOK-detectorDelay)%LOOK]:x.l;
    double detectorR=detectorDelay?look[1][(lpos+LOOK-detectorDelay)%LOOK]:x.r;
    double current=mx(ab(detectorL),ab(detectorR));
    double ac=1./(1.+sampleRate*(attack*.001+.0005)),rc=1./(1.+sampleRate*release*.001);
    globalEnv+=(current-globalEnv)*(current>globalEnv?ac:rc);
    double threshold=dbgain(-8.-glue*16.+impact*6.);
    double thresholdDb=clamp(globalEnv/(threshold+1.e-12),0.,1000.);
    double reduction=thresholdDb<=1.?1.:1./(1.+(thresholdDb-1.)*(ratio-1.)/(ratio+(1.-knee)*2.));
    gain+=.005*(reduction-gain);
    int d=constantDelay;
    float L=look[0][(lpos+LOOK-d)%LOOK],R=look[1][(lpos+LOOK-d)%LOOK];
    look[0][lpos]=(float)x.l;look[1][lpos]=(float)x.r;lpos=(lpos+1)%LOOK;
    return {clamp(L*gain*output,-ceil,ceil),clamp(R*gain*output,-ceil,ceil)};}
  }return x;
 }
 Pair frame(Pair input){smoothParams();lfo+=2*PI/sampleRate;if(lfo>2*PI*1024)lfo=0.;
  meterIn+=(mx(ab(input.l),ab(input.r))-meterIn)*.003;
  int latency=(int)(sampleRate*.010+.5);if(latency>=LOOK)latency=LOOK-1;
  Pair delayedInput={dry[0][(finalPos+LOOK-latency)%LOOK],dry[1][(finalPos+LOOK-latency)%LOOK]};
  dry[0][finalPos]=(float)input.l;dry[1][finalPos]=(float)input.r;
  Pair x=input;bool coagActive=p[EnableBase+6]>.5;
  if(p[BypassId]<.5){
   int ordered[7]={0,1,2,3,4,5,6};
   for(int i=0;i<7;i++)for(int j=i+1;j<7;j++)if(p[OrderBase+ordered[j]]<p[OrderBase+ordered[i]]){
    int tmp=ordered[i];ordered[i]=ordered[j];ordered[j]=tmp;
   }
   for(int slot=0;slot<7;slot++){int stage=ordered[slot];if(p[EnableBase+stage]>.5)x=effect(stage,x);}
  }
  // Always keep an aligned wet delay line warm for stage bypass and reorder transitions.
  Pair delayedWet={wet[0][(finalPos+LOOK-latency)%LOOK],wet[1][(finalPos+LOOK-latency)%LOOK]};
  wet[0][finalPos]=(float)x.l;wet[1][finalPos]=(float)x.r;
  finalPos=(finalPos+1)%LOOK;
  if(p[BypassId]>=.5){meterOut+=(mx(ab(delayedInput.l),ab(delayedInput.r))-meterOut)*.003;return delayedInput;}
  if(!coagActive)x=delayedWet;
  double wetMix=clamp(smooth[77]*.01,0.,1.),master=dbgain(smooth[78]);
  x.l=(delayedInput.l*(1.-wetMix)+x.l*wetMix)*master;
  x.r=(delayedInput.r*(1.-wetMix)+x.r*wetMix)*master;
  x.l=clamp(x.l,-1.02,1.02);x.r=clamp(x.r,-1.02,1.02);
  meterOut+=(mx(ab(x.l),ab(x.r))-meterOut)*.003;
  return x;
 }
};

struct Note {uint64_t at;int noteId,channel,key,type;double velocity;};
struct MidiEngine {
 static constexpr int SCHEDULE=2048, TRACK=192, GENERATED=32;
 double p[ParamCount];double sr=48000.,bpm=120.;uint64_t absolute=0;
 uint32_t rng=0x67c12a0bu;uint32_t sequence=0;
 struct Group {int id,key,ch;uint64_t started;bool used;};
 struct MVoice {int group,key,ch,id;uint64_t start;double gate;bool used;};
 Group groups[TRACK]{};MVoice voices[TRACK]{};
 Note pending[SCHEDULE]{};uint32_t pendingSize=0,overflow=0;
 void defaults(){for(int i=0;i<ParamCount;i++)p[i]=specs[1][i].def;}
 void prepare(double sampleRate){sr=clamp(sampleRate,22050.,192000.);absolute=0.;pendingSize=0;overflow=0;sequence=0;for(int i=0;i<TRACK;i++){groups[i].used=false;voices[i].used=false;}}
 void set(int id,double val){if(id>=0&&id<ParamCount)p[id]=clamp(val,specs[1][id].lo,specs[1][id].hi);}
 double v(int stage,int i)const{return p[stage*9+i];}
 double pc(int stage,int i)const{return .01*v(stage,i);}
 int stageAt(int slot)const{int order[7]={0,1,2,3,4,5,6};for(int i=0;i<7;i++)for(int j=i+1;j<7;j++)if(p[OrderBase+order[j]]<p[OrderBase+order[i]]){int t=order[i];order[i]=order[j];order[j]=t;}return order[slot];}
 bool schedule(Note n){if(pendingSize>=SCHEDULE){overflow++;return false;}pending[pendingSize++]=n;return true;}
 void transpose(int &key,int amount){key=(int)clamp(key+amount,0.,127.);}
 int findGroup(int id,int ch,int key){for(int i=0;i<TRACK;i++)if(groups[i].used&&groups[i].ch==ch&&groups[i].key==key&&(id<0||groups[i].id==id))return i;return -1;}
 int createGroup(int id,int ch,int key){for(int i=0;i<TRACK;i++)if(!groups[i].used){groups[i]={id,key,ch,absolute,true};return i;}return -1;}
 void noteOn(int id,int ch,int key,double velocity,uint64_t when){
  if(key<0||key>127||ch<0||ch>15)return;
  if(p[BypassId]>.5||p[77]<.5){schedule({when,id,ch,key,1,velocity});int g=createGroup(id,ch,key);if(g>=0){for(int i=0;i<TRACK;i++)if(!voices[i].used){voices[i]={g,key,ch,id,when,1.,true};break;}}return;}
  int group=createGroup(id,ch,key);if(group<0){overflow++;return;}
  struct Candidate {int key,ch;double velocity,ms,gate;}; Candidate notes[GENERATED]={{key,ch,clamp(velocity,0.,1.),0.,1.}};int n=1;
  int voicecap=(int)p[79];if(voicecap<1)voicecap=1;if(voicecap>GENERATED)voicecap=GENERATED;
  uint32_t seed=((uint32_t)v(4,6)<<16)^((uint32_t)v(1,6)*0x9E3779B9u)^rng^(++sequence*0x85ebca6bu);
  for(int order=0;order<7;order++){
   int stage=stageAt(order);if(p[EnableBase+stage]<.5)continue;
   switch(stage){
    case 0:for(int k=0;k<n;k++){double vel=notes[k].velocity;
     double curve=.4+pc(0,3)*1.2;vel=vel*(.55+pc(0,1)*.85)*(1.+pc(0,0)*.3);
     vel=clamp(vel+(.5-vel)*(pc(0,5)-.65)*.17,0.,1.);
     if(curve>1.)vel*=1.-(curve-1.)*.25*(1.-vel);
     vel=clamp(vel, v(0,4)/127.,v(0,6)/127.);
     notes[k].velocity=vel;notes[k].gate*=.55+pc(0,2)*.85+(pc(0,7)-.5)*.15;
     if(pc(0,8)>=.5){notes[k].velocity=clamp(notes[k].velocity,1./127.,1.);notes[k].gate=mx(.1,notes[k].gate);}
    }break;
    case 1:for(int k=0;k<n;k++){
      double shift=(random01(seed)*2.-1.)*pc(1,0)*v(1,1);
      notes[k].ms+=mx(0.,shift+(pc(1,7)-.5)*v(1,1)*.45+pc(1,5)*1.5);
      notes[k].gate*=.7+pc(1,4)*.6+pc(1,2)*.2;
      notes[k].ms+=((sequence&1)?1:-1)*v(1,3)*.04;
      if(pc(1,8)>=.5){notes[k].ms=clamp(notes[k].ms,0.,250.);notes[k].gate=clamp(notes[k].gate,.1,3.);}
    }break;
    case 2:for(int k=0;k<n;k++){
      const double split=v(2,0),edge=v(2,6)*.5;
      // Routing only inside the selected octave span; preserve per-note
      // expression channels when MPE SAFE is enabled for identified notes.
      const bool within=ab(notes[k].key-split)<=12.*v(2,4);
      const bool mpeSafe=pc(2,7)>=.5 && id>=0;
      if(within&&!mpeSafe){
       if(notes[k].key<split-edge)notes[k].ch=imod(notes[k].ch+(int)(pc(2,1)*7),16);
       else if(notes[k].key>split+edge)notes[k].ch=imod(notes[k].ch+(int)(pc(2,2)*7),16);
      }
      if(notes[k].velocity*127.<v(2,3))notes[k].velocity*=.7+pc(2,5)*.3;
    }break;
    case 3:{int original=n;int voicesCount=(int)v(3,1);int interval=(int)v(3,0),inv=(int)v(3,3);for(int k=0;k<original;k++){
      if(random01(seed)>pc(3,5))continue;
      for(int c=1;c<voicesCount&&n<voicecap;c++){
       Candidate extra=notes[k];int pitchShift=c*interval-12*inv;
       if(pc(3,6)>.5){ // voice leading: prefer a nearby octave for the added harmony
        while(pitchShift>9)pitchShift-=12;while(pitchShift< -9)pitchShift+=12;
       }
       transpose(extra.key,pitchShift);extra.velocity*=clamp(pc(3,2)*(.92-.09*c),.05,1.);
       extra.ms+=pc(3,4)*c*12.;extra.ch=imod((int)v(3,8)-1,16);
       bool dup=false;for(int e=0;e<n;e++)if(notes[e].key==extra.key&&notes[e].ch==extra.ch)dup=true;
       if(!dup||pc(3,7)<.5)notes[n++]=extra;
      }
     }break;}
    case 4:{int original=n;for(int k=0;k<original;k++)if(random01(seed)<pc(4,1)){
      if(random01(seed)<pc(4,8)*pc(4,0))notes[k].velocity=0.;
      else {int range=(int)(v(4,3)*pc(4,7));int change=(int)((random01(seed)*2.-1.)*range*pc(4,0));
       notes[k].key=(int)clamp(notes[k].key+change,0.,127.);
       notes[k].ms+=random01(seed)*v(4,4)*.4;notes[k].gate*=.7+pc(4,2)*.6;}
      // Generative repeat: a second delayed articulation belonging to same note-off group.
      if(random01(seed)<pc(4,5)*.45 && n<voicecap){Candidate repeat=notes[k];
       repeat.ms+=(35.+random01(seed)*150.)*(.3+pc(4,4));
       repeat.velocity*=.5+.4*pc(4,2);repeat.gate*=.75;
       notes[n++]=repeat;}
     }break;}
    case 5:{for(int k=0;k<n;k++){
      int root=(int)v(5,0),notePc=imod(notes[k].key-root,12);
      bool major[12]={true,false,true,false,true,true,false,true,false,true,false,true};
      bool pentatonic[12]={true,false,true,false,true,false,false,true,false,true,false,false};
      bool inScale=pc(5,4)>=.5?pentatonic[notePc]:major[notePc];
      if(!inScale&&random01(seed)<pcct(5,1)){
       // Choose the closest scale tone. For ties favor the lower note.
       int delta=0;for(int d=1;d<=6;d++){
        bool lower=pc(5,4)>=.5?pentatonic[imod(notePc-d,12)]:major[imod(notePc-d,12)];
        bool higher=pc(5,4)>=.5?pentatonic[imod(notePc+d,12)]:major[imod(notePc+d,12)];
        if(lower||higher){delta=lower?-d:d;break;}
       }notes[k].key=(int)clamp(notes[k].key+delta,0.,127.);
      }
      double dist=ab(notes[k].key-v(5,3));if(dist>v(5,6)*.7+24.)notes[k].velocity*=.5;
      if(random01(seed)>pcct(5,2))notes[k].velocity=0.;
      if(pcct(5,7)>.5&&notes[k].key%12!=root)notes[k].velocity*=.9;
      if(imod(notes[k].key-root,12)==0)notes[k].velocity=clamp(notes[k].velocity*(1.+pc(5,8)*.18),0.,1.);
      if(pc(5,7)>=.5 && pc(5,8)>.75 && random01(seed)<.1){
       notes[k].key=(int)clamp(notes[k].key+imod(root-notes[k].key,12),0.,127.);
      }
    }
    if(pc(5,5)>=.5){for(int k=0;k<n;k++)for(int j=k+1;j<n;){
      if(notes[k].key==notes[j].key&&notes[k].ch==notes[j].ch){for(int r=j;r<n-1;r++)notes[r]=notes[r+1];n--;}
      else j++;
    }}break;}
    case 6:{int original=n;for(int k=0;k<original;k++){
      double stepMs=60000./mx(30.,bpm)*4./mx(1.,v(6,3));
      double target=roundNearest((when/sr*1000.+notes[k].ms)/stepMs)*stepMs;
      double next=mx(when/sr*1000.,target);
      notes[k].ms+=mx(0.,next-(when/sr*1000.+notes[k].ms))*pc(6,0)*pc(6,2);
      notes[k].ms+=v(6,6); // signed musical timing offset; final scheduling clamps to zero
      notes[k].gate*=.4+pc(6,1)*.8+pc(6,4)*.18+pc(6,7)*.5;
      if(random01(seed)<pc(6,5)*.35&&n<voicecap){Candidate retrig=notes[k];
       retrig.ms+=stepMs*.5;retrig.velocity*=.7;retrig.gate*=.7;
       notes[n++]=retrig;
      }
    }break;}
   }
  }
  int created=0;for(int k=0;k<n&&created<voicecap;k++){
    if(notes[k].velocity<=.001)continue;
    int pitch=(int)clamp(notes[k].key+p[78],0.,127.);int chan=imod(notes[k].ch,16);
    uint64_t time=when+(uint64_t)(mx(0.,notes[k].ms)*sr*.001);
    // Unique generated CLAP note ID for chord voices where host supports IDs.
    int outputId=(id>=0&&pc(2,8)>=.5)?(id*37+created+1)&0x7FFFFFFF:-1;
    int freeIndex=-1;for(int j=0;j<TRACK;j++)if(!voices[j].used){freeIndex=j;break;}
    if(freeIndex<0){overflow++;break;}
    if(!schedule({time,outputId,chan,pitch,1,notes[k].velocity}))continue;
    voices[freeIndex]={group,pitch,chan,outputId,time,clamp(notes[k].gate,.05,4.),true};created++;
  }
  // Preserve a suppressed note group until its real note-off; never emit an orphan off.
 }
 double pcct(int stage,int i)const{return p[stage*9+i]*.01;}
 void noteOff(int id,int ch,int key,double velocity,uint64_t when){
  int group=findGroup(id,ch,key);if(group<0){ // passthrough unmatched offs avoids an orphan if FX enabled mid-note
    schedule({when,id,ch,key,0,velocity});return;
  }
  for(int i=0;i<TRACK;i++)if(voices[i].used&&voices[i].group==group){
    MVoice &v=voices[i];uint64_t t=when;if(t<v.start+1)t=v.start+1;
    double gate=clamp(v.gate,.05,4.);
    t=v.start+(uint64_t)((t-v.start)*gate);if(t<=v.start)t=v.start+1;
    schedule({t,v.id,v.ch,v.key,0,velocity});v.used=false;
  }groups[group].used=false;
 }
 template<class Sink>void drain(uint64_t from,uint64_t to,Sink emit){
  // Stuck-note failsafe: release any voice older than twelve seconds even
  // if the host never sends its note-off. Avoids permanently hanging notes.
  if(pc(6,8)>=.5)for(int j=0;j<TRACK;j++)if(voices[j].used){
   const uint64_t limit=voices[j].start+(uint64_t)(sr*12.);
   if(limit<to){const uint64_t at=limit<from?from:limit;
    schedule({at,voices[j].id,voices[j].ch,voices[j].key,0,0.});
    voices[j].used=false;
   }
  }
  // Emits monotonically sorted events, retaining events scheduled in future blocks.
  for(int pass=0;pass<SCHEDULE;pass++){
   int found=-1;uint64_t earliest=to;
   for(uint32_t i=0;i<pendingSize;i++)if(pending[i].at>=from&&pending[i].at<earliest){earliest=pending[i].at;found=(int)i;}
   if(found<0)break;
   Note event=pending[found];pending[found]=pending[--pendingSize];emit(event,(uint32_t)(event.at-from));
  }
  // Drop any stale pending events (never push them into the past).
  for(uint32_t i=0;i<pendingSize;)if(pending[i].at<from)pending[i]=pending[--pendingSize];else i++;
 }
};
} // namespace nig
