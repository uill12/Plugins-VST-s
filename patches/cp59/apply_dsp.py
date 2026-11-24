#!/usr/bin/env python3
"""Patch exact CP58R9 reconstructed DSP (post CP58R4 and CP58R3) for CP59 WaveEdit.
Run before CMake; preserve legacy PCM24 and all previous note/preset parameter IDs.
"""
from pathlib import Path
import argparse
p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);a=p.parse_args()
hp=a.root/'include/vitriol_dsp.hpp';pp=a.root/'include/vitriol_params.hpp';cp=a.root/'src/vitriol_dsp.cpp'
h=hp.read_text(encoding='utf-8');s=cp.read_text(encoding='utf-8');pr=pp.read_text(encoding='utf-8')
def rep(target,old,new,label):
 count=target.count(old)
 if count!=1:raise RuntimeError(f'CP59 {label}: expected one anchor, found {count}')
 return target.replace(old,new)
h=rep(h,' bool loadResourceBankBytes(const unsigned char* bytes,size_t length);', ''' bool loadResourceBankBytes(const unsigned char* bytes,size_t length);
 bool loadWaveEditBankBytes(const unsigned char* bytes,size_t length);
 bool loadWaveEditBank(const std::string& filename);
 bool waveEditReady() const noexcept{return waveEditReady_;}''','header public')
h=rep(h,' std::vector<double> proceduralSample_;', ''' std::vector<double> proceduralSample_;
 // CP59: immutable, per-instance, predecoded before the audio callback.
 // 24 original CC0 banks × 64 frames × 256 samples × 8 FFT mip levels,
 // plus 24 pre-morphed 48-cycle granular textures. No callback I/O.
 std::vector<int16_t> waveEditTables_,waveEditGrains_;
 bool waveEditReady_=false;
 double waveEditSample(int index,double morph,double phase,double pitch,double rate)const;
 double waveEditGrainSample(int index,double phase)const;''','header resource state')
pr=rep(pr,'add(b+10,"Material",g,0,31,i*7,"",true);','add(b+10,"Material",g,0,55,i*7,"",true);','material range')
s=rep(s,'''   if(r){HGLOBAL h=LoadResource(mod,r);if(h)loadResourceBankBytes(static_cast<const unsigned char*>(LockResource(h)),SizeofResource(mod,r));}
''','''   if(r){HGLOBAL h=LoadResource(mod,r);if(h)loadResourceBankBytes(static_cast<const unsigned char*>(LockResource(h)),SizeofResource(mod,r));}
   HRSRC wr=FindResourceW(mod,MAKEINTRESOURCEW(103),MAKEINTRESOURCEW(10));
   if(wr){HGLOBAL h=LoadResource(mod,wr);if(h)loadWaveEditBankBytes(static_cast<const unsigned char*>(LockResource(h)),SizeofResource(mod,wr));}
''','win resource 103')
anchor='double Synth::fastSinCycles(double cycles)const{'
new=r'''// The material file is produced *offline* from the verified WaveEdit CC0
// archive: 24 banks, each with 64 samplesets of 256 PCM16 cycles. No audio
// thread memory allocation, file loading or FFT.
bool Synth::loadWaveEditBank(const std::string& filename){
 std::ifstream f(filename,std::ios::binary);if(!f)return false;
 std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)),{});
 return loadWaveEditBankBytes(bytes.data(),bytes.size());
}
bool Synth::loadWaveEditBankBytes(const unsigned char* bytes,size_t size){
 if(!bytes||size<28||std::memcmp(bytes,"ALBWT59\0",8)!=0)return false;
 auto u32=[&](size_t p){return uint32_t(bytes[p])|(uint32_t(bytes[p+1])<<8)|(uint32_t(bytes[p+2])<<16)|(uint32_t(bytes[p+3])<<24);};
 if(u32(8)!=24||u32(12)!=64||u32(16)!=256||u32(20)!=8||u32(24)!=12288)return false;
 constexpr size_t tables=24u*8u*64u*256u,grains=24u*12288u;
 if(size!=28u+2u*(tables+grains))return false;
 std::vector<int16_t> t(tables),g(grains);
 for(size_t i=0;i<tables+grains;++i){
  const size_t at=28+i*2;
  const uint16_t sample=uint16_t(bytes[at])|(uint16_t(bytes[at+1])<<8);
  if(i<tables)t[i]=static_cast<int16_t>(sample);
  else g[i-tables]=static_cast<int16_t>(sample);
 }
 waveEditTables_.swap(t);waveEditGrains_.swap(g);waveEditReady_=true;return true;
}
double Synth::waveEditSample(int index,double morph,double phase,double pitch,double rate)const{
 if(!waveEditReady_)return 0;
 index=std::clamp(index,0,23);
 const int allowed=std::max(1,int(.43*rate/std::max(1.,pitch)));
 int mip=0;while(mip<7 && (128>>mip)>allowed)++mip;
 const double frame=clamp(morph,0.,1.)*63., cycle=fract(phase)*256.;
 const int f0=std::clamp(int(frame),0,63),f1=std::min(f0+1,63),j=int(cycle)&255,k=(j+1)&255;
 const double ft=frame-f0,pt=cycle-int(cycle);
 auto sample=[&](int f,int x){const size_t at=(((size_t(index)*8+size_t(mip))*64+size_t(f))*256)+size_t(x);return double(waveEditTables_[at])/32768.;};
 return lerp(lerp(sample(f0,j),sample(f0,k),pt),lerp(sample(f1,j),sample(f1,k),pt),ft);
}
double Synth::waveEditGrainSample(int index,double phase)const{
 if(!waveEditReady_)return 0;
 const double p=fract(phase)*12288.;const int j=int(p),k=(j+1)%12288;
 const size_t base=size_t(std::clamp(index,0,23))*12288u;
 return lerp(double(waveEditGrains_[base+j]),double(waveEditGrains_[base+k]),p-double(j))/32768.;
}
'''
s=rep(s,anchor,new+anchor,'WaveEdit load/read functions')
s=rep(s,' case 1:{ // bandlimited mipmapped wavetable with color warp and fold', ''' case 1:{ // bandlimited mipmapped wavetable with color warp and fold
  const int resource=int(get(1000+e*32+10));
  if(waveEditReady_ && resource>=32 && resource<56){
   const double warped=fract(phase+b*.12*fastSinCycles(phase));
   const double pcm=waveEditSample(resource-32,clamp(a+.15*c*(v.cellValue-.5),0.,1.),warped,freq,fsInt);
   y=std::tanh(pcm*(1.+c*1.2))/(1.+c*.16)+.14*d*fastSinCycles(sub);
   break;
  }''','wavetable source')
s=rep(s,'    double sample=fastSinCycles(ph);', '''    double sample=fastSinCycles(ph);
    // Keep stacked unison layers on the same selected WaveEdit material,
    // rather than adding an unrelated sine tone on every extra voice.
    const int chosen=int(get(1000+e*32+10));
    if(m==1&&waveEditReady_&&chosen>=32&&chosen<56)
     sample=waveEditSample(chosen-32,a,ph,f,sr);''','WaveEdit unison')
s=rep(s,' if(cc==123){arpPedal_[channel]=false;', ''' if(cc==121){ // MIDI Reset All Controllers: clear orphaned sustain pedals.
  arpPedal_[channel]=false;arpPrune();
  sustain_[channel]=false;modwheel_=0;
  for(auto& v:voices_)if(v.active&&v.ch==channel&&v.sustained&&!v.held){
   v.sustained=false;v.released=true;v.envStage=3;v.fastPanicRelease=true;
  }
 }
 if(cc==123){arpPedal_[channel]=false;''','CC121 sustain reset')
s=rep(s,' case 3:{ // procedural Hann-windowed particle cloud; deterministic embedded source, no licensed samples', ''' case 3:{ // granular cloud: original PCM24 or concept-morphed CC0 WaveEdit texture
  const int resource=int(get(1000+e*32+10));
  if(waveEditReady_ && resource>=32 && resource<56){
   const double position=fract(v.samplePos[e]+d*.18);
   const double sample=waveEditGrainSample(resource-32,position);
   const double gp=v.grainPhase[e];const int winType=int(get(1000+e*32+15));
   const double win=winType==1?fastSinCycles(gp*.5):winType==2?std::min(1.,std::min(gp,1.-gp)*7.):winType==3?std::exp(-18.*(gp-.5)*(gp-.5)):.5-.5*fastSinCycles(gp+.25);
   y=sample*win*1.65+(c*.16)*fastSinCycles(phase);
   // Texture contains precisely 48 single cycles, so playback at noteHz
   // never drifts with density, a scratch parameter or the host sample rate.
   v.samplePos[e]=fract(v.samplePos[e]+freq/(48.*fsInt));
   v.grainPhase[e]=fract(v.grainPhase[e]+(2.+28*b)/fsInt);
   break;
  }''','granular source')
s=rep(s,' case 7:{ // timewarp / procedural one-shot sample; editable start and time-stretch surrogate', ''' case 7:{ // timewarp / procedural one-shot sample; editable start and time-stretch surrogate
  const int resource=int(get(1000+e*32+10));
  if(waveEditReady_ && resource>=32 && resource<56){
   y=waveEditGrainSample(resource-32,fract(v.samplePos[e]+a*.5));
   v.samplePos[e]=fract(v.samplePos[e]+freq/(48.*fsInt));
   break;
  }''','time warp path')
# Arpeggiator sometimes receives a valid note-off on a different channel
# (some MIDI controllers/host bridges). Release only when pitch uniquely maps
# to one held channel; do not steal other notes from a chord.
arp=a.root/'include/AlbedoCellArp.inc'
ss=arp.read_text(encoding='utf-8')
needle=' if(found>=0){arpKeys_[found].down=false;arpPrune();}'
ss=rep(ss,needle,''' if(found<0&&id<0&&n>=0){
  int owner=-1;bool ambiguous=false;
  for(int i=0;i<arpCount_;++i)if(arpKeys_[i].down&&arpKeys_[i].note==n){
   if(owner>=0&&arpKeys_[owner].ch!=arpKeys_[i].ch){ambiguous=true;break;}
   if(owner<0)owner=i;
  }
  if(!ambiguous)found=owner;
 }
 if(found>=0){arpKeys_[found].down=false;arpPrune();}''','arp unique pitch fallback')
# No changes to non-selected historical sources or the always-on DAW clock.
hp.write_text(h,encoding='utf-8');pp.write_text(pr,encoding='utf-8');cp.write_text(s,encoding='utf-8');arp.write_text(ss,encoding='utf-8')
print('CP59 DSP PASS: Wavetable mipmaps, 48-cycle granular textures, note-off fallback')