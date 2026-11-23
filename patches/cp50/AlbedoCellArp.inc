// A single clock owns note selection, step rests and gate releases.
// Keep host tempo separate from the free-running BPM parameter. No transport
// callbacks, allocations, or UI calls are made in the scheduler.
void Synth::setArpTempo(double bpm){arpHostValid_=std::isfinite(bpm)&&bpm>0;if(arpHostValid_)arpHostTempo_=clamp(bpm,20.,400.);}
// Host PPQ time is sample-advanced between MIDI events and never driven by UI.
void Synth::setArpTransport(double ppq,bool playing){
 const bool valid=playing&&std::isfinite(ppq)&&ppq>=0.&&arpHostValid_;
 if(!valid){arpTransportValid_=false;return;}
 const bool jumped=arpTransportValid_&&std::abs(ppq-arpTransportBeat_)>0.02;
 arpTransportBeat_=ppq;arpTransportValid_=true;
 if(arpEnabled_&&arpCount_){
  if(jumped){arpRelease();arpGate_=0.;}
  arpSnap();
 }
}
// Lock step phase to paired swung musical divisions, without changing pitch.
void Synth::arpSnap(){
 if(!arpTransportValid_||!arpHostValid_)return;
 const double division=clamp(get(6021),.05,1.);
 const double phase=arpTransportBeat_/division;
 const double pair=std::floor(phase*.5),pairStart=2.*pair;
 const double swing=clamp(get(6026)*.01,0.,.45);
 double nextBeat=pairStart;
 uint64_t index=uint64_t(pairStart);
 if(phase>nextBeat+1.e-9){nextBeat=pairStart+1.+swing;++index;}
 if(phase>nextBeat+1.e-9){nextBeat=pairStart+2.;++index;}
 const unsigned length=unsigned(std::clamp(int(get(6027)),4,16));
 arpStep_=int(index%length);arpIndex_=unsigned(index);
 arpClock_=std::max(0.,(nextBeat-phase)*division*fs_*60./arpHostTempo_);
}

void Synth::arpRelease(){
 if(arpSound_>=0){voiceOff(arpSound_,arpChannel_,-1);arpSound_=-1;}
}
void Synth::arpClear(){arpDisplay_.store(-1,std::memory_order_relaxed);arpCount_=0;arpSound_=-1;arpClock_=arpGate_=0;arpStep_=arpIndex_=0;arpBits_=0x8421;arpPedal_.fill(false);}
void Synth::arpMode(){
 const bool enabled=get(6020)>.5;
 if(enabled==arpEnabled_)return;
 arpDisplay_.store(-1,std::memory_order_relaxed);arpRelease();voicePanic();arpClock_=arpGate_=0;arpStep_=arpIndex_=0;arpBits_=0x8421;
 arpEnabled_=enabled;
 if(enabled)arpSnap();
 if(!enabled){
  for(int ch=0;ch<16;++ch)sustain_[ch]=arpPedal_[ch];
  for(int i=0;i<arpCount_;++i){const auto& k=arpKeys_[i];voiceOn(k.note,k.vel,k.ch,k.id);if(!k.down)voiceOff(k.note,k.ch,k.id);}
 }
}
void Synth::noteOn(int n,double vel,int ch,int id){
 if(n<0||n>127||ch<0||ch>15||!std::isfinite(vel))return;
 if(vel<=0){noteOff(n,ch,id);return;}
 arpMode();
 if(!arpCount_){arpClock_=0;arpStep_=arpIndex_=0;arpBits_=0x8421;arpSnap();}
 if(arpCount_==int(arpKeys_.size())){for(int i=1;i<arpCount_;++i)arpKeys_[i-1]=arpKeys_[i];--arpCount_;}
 arpKeys_[arpCount_++]={n,ch,id,vel,true};
 if(!arpEnabled_)voiceOn(n,vel,ch,id);
}
void Synth::noteOff(int n,int ch,int id){
 arpMode();
 int found=-1;
 for(int i=0;i<arpCount_;++i){const auto& k=arpKeys_[i];if(k.down&&(ch<0||k.ch==ch)&&(id>=0?k.id==id:k.note==n)){found=i;break;}}
 if(found<0&&id>=0){for(int i=0;i<arpCount_;++i)if(arpKeys_[i].down&&arpKeys_[i].note==n&&(ch<0||arpKeys_[i].ch==ch)){found=i;break;}}
 if(found>=0){arpKeys_[found].down=false;arpPrune();}
 if(!arpEnabled_)voiceOff(n,ch,id);
}
void Synth::arpPrune(){
 int write=0;for(int i=0;i<arpCount_;++i){auto k=arpKeys_[i];if(k.down||arpPedal_[k.ch])arpKeys_[write++]=k;}
 arpCount_=write;if(!write){arpRelease();arpDisplay_.store(-1,std::memory_order_relaxed);}
}
void Synth::allNotesOff(){arpClear();voicePanic();}
void Synth::arpTick(){
 // Nereida CP-03B characters: Oblique / Symmetrical / Cyclical / Asymmetrical.
 // Only note-event selection and deliberate octave additions affect pitch.
 arpRelease();
 const int length=std::clamp(int(get(6027)),4,16);
 const unsigned step=unsigned(arpStep_%length);
 arpDisplay_.store(int(step),std::memory_order_relaxed);
 const int character=std::clamp(int(get(6024)),0,3);
 const int octaves=std::clamp(int(get(6025)),1,3);
 const int rotate=std::clamp(int(get(6028)),0,7);
 const double swing=clamp(get(6026)*.01,0.,.45);
 const double tempo=arpHostValid_?arpHostTempo_:120.;
 const double division=get(6021); // 1, 1/2, 1/4, 1/8 beats for quarter ... thirty-second
 const double period=std::max(2.,fs_*60./tempo*division*((step&1)?1.-swing:1.+swing));
 arpClock_+=period;
 arpGate_=std::max(1.,period*clamp(get(6023)*.01,.10,.95));
 const bool obliqueGap=character==0 && (step%8==7);
 // Fixed Bjorklund-like 5-of-8 pulse mask, deterministic for any step count.
 const bool asymmetricGap=character==3 && ((step*5)%8)>=5;
 const bool active=get(6000+step)>.5&&!obliqueGap&&!asymmetricGap;
 if(arpCount_>0 && active){
  std::array<int,128> sorted{};for(int i=0;i<arpCount_;++i)sorted[i]=i;
  std::sort(sorted.begin(),sorted.begin()+arpCount_,[&](int a,int b){
   return std::make_pair(arpKeys_[a].note,a)<std::make_pair(arpKeys_[b].note,b);
  });
  const int span=arpCount_*octaves;
  const unsigned cursor=arpIndex_;
  int pos=0;
  if(character==0)pos=int((2ull*cursor+cursor/2+unsigned(rotate))%unsigned(span));
  else if(character==1){
   const int cycle=std::max(1,2*span-2);
   const int t=int((cursor+unsigned(rotate))%unsigned(cycle));
   pos=t<span?t:cycle-t;
  }else if(character==2)pos=int((cursor+unsigned(rotate))%unsigned(span));
  else pos=int((5ull*cursor+cursor/3+unsigned(rotate))%unsigned(span));
  const auto& key=arpKeys_[sorted[pos%arpCount_]];
  const int note=key.note+12*(pos/arpCount_);
  if(note<=127){
   // Separate CA lattice shapes accents only; no pitch drift or random transpose.
   const bool energetic=((arpBits_>>step)&1)!=0;
   const double accent=clamp(get(6022)*.01,0.,1.);
   const double velocityScale=energetic?1.:(1.-.70*accent);
   voiceOn(note,clamp(key.vel*velocityScale,.001,1.),key.ch,-1);
   arpSound_=note;arpChannel_=key.ch;
  }
 }
 ++arpIndex_; // Advance through rests as Nereida's note-event clock does.
 if(++arpStep_>=length){
  arpStep_=0;const int rules[]={30,90,110,150};
  const int rule=rules[std::clamp(int(get(4003)),0,3)];
  unsigned next=0;
  for(int i=0;i<16;++i){
   const int pattern=(((arpBits_>>((i+15)%16))&1)<<2)
                    |(((arpBits_>>i)&1)<<1)
                    |((arpBits_>>((i+1)%16))&1);
   next|=unsigned((rule>>pattern)&1)<<i;
  }
  arpBits_=next?next:1u;
 }
}
template<typename Sample>void Synth::processArp(Sample* l,Sample* r,uint32_t n){
 arpMode();if(!arpEnabled_||!arpCount_){processTyped(l,r,n);if(arpTransportValid_)arpTransportBeat_+=double(n)*arpHostTempo_/(60.*fs_);return;}
 unsigned offset=0;
 while(offset<n){
  if(arpClock_<=0)arpTick();
  if(arpGate_<=0)arpRelease();
  double boundary=arpSound_<0?arpClock_:std::min(arpClock_,arpGate_);
  unsigned count=std::min(n-offset,unsigned(std::max(1.,std::ceil(boundary))));
  processTyped(l+offset,r+offset,count);offset+=count;arpClock_-=count;arpGate_-=count;
 }
 if(arpTransportValid_)arpTransportBeat_+=double(n)*arpHostTempo_/(60.*fs_);
}
