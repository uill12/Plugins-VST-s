// A single clock owns note selection, step rests and gate releases.
// Keep host tempo separate from the free-running BPM parameter. No transport
// callbacks, allocations, or UI calls are made in the scheduler.
void Synth::setArpTempo(double bpm){if(std::isfinite(bpm)&&bpm>0){arpHostTempo_=clamp(bpm,40.,240.);arpHostValid_=true;}}
void Synth::arpRelease(){
 if(arpSound_>=0){voiceOff(arpSound_,arpChannel_,-1);arpSound_=-1;}
}
void Synth::arpClear(){arpDisplay_.store(-1,std::memory_order_relaxed);arpCount_=0;arpSound_=-1;arpClock_=arpGate_=0;arpStep_=arpIndex_=0;arpBits_=0x8421;arpPedal_.fill(false);}
void Synth::arpMode(){
 const bool enabled=get(6020)>.5;
 if(enabled==arpEnabled_)return;
 arpDisplay_.store(-1,std::memory_order_relaxed);arpRelease();voicePanic();arpClock_=arpGate_=0;arpStep_=arpIndex_=0;arpBits_=0x8421;
 arpEnabled_=enabled;
 if(!enabled){
  for(int ch=0;ch<16;++ch)sustain_[ch]=arpPedal_[ch];
  for(int i=0;i<arpCount_;++i){const auto& k=arpKeys_[i];voiceOn(k.note,k.vel,k.ch,k.id);if(!k.down)voiceOff(k.note,k.ch,k.id);}
 }
}
void Synth::noteOn(int n,double vel,int ch,int id){
 if(n<0||n>127||ch<0||ch>15||!std::isfinite(vel))return;
 if(vel<=0){noteOff(n,ch,id);return;}
 arpMode();
 if(!arpCount_){arpClock_=0;arpStep_=arpIndex_=0;arpBits_=0x8421;}
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
 const double tempo=(get(6030)>.5&&arpHostValid_)?arpHostTempo_:clamp(get(6029),40.,240.);
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
  std::stable_sort(sorted.begin(),sorted.begin()+arpCount_,[&](int a,int b){
   return arpKeys_[a].note<arpKeys_[b].note;
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
 arpMode();if(!arpEnabled_||!arpCount_){processTyped(l,r,n);return;}
 unsigned offset=0;
 while(offset<n){
  if(arpClock_<=0)arpTick();
  if(arpGate_<=0)arpRelease();
  double boundary=arpSound_<0?arpClock_:std::min(arpClock_,arpGate_);
  unsigned count=std::min(n-offset,unsigned(std::max(1.,std::ceil(boundary))));
  processTyped(l+offset,r+offset,count);offset+=count;arpClock_-=count;arpGate_-=count;
 }
}
