// A single clock owns note selection, step rests and gate releases.
void Synth::setArpTempo(double bpm){if(std::isfinite(bpm)&&bpm>0)arpTempo_=clamp(bpm,10.,999.);}
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
 arpRelease();
 const int length=std::clamp(int(get(6027)),1,16),step=arpStep_%length;
 arpDisplay_.store(step,std::memory_order_relaxed);
 const int mode=std::clamp(int(get(6024)),0,4),octaves=std::clamp(int(get(6025)),1,4);
 const double swing=get(6026)*.005;
 const double period=std::max(2.,fs_*(60./arpTempo_)*get(6021)*((step&1)?1.-swing:1.+swing));
 arpClock_+=period;arpGate_=std::max(1.,period*clamp(get(6023)*.01,.01,.99));
 if(arpCount_&&get(6000+step)>.5){
  std::array<int,128> order{};for(int i=0;i<arpCount_;++i)order[i]=i;
  if(mode!=3)std::sort(order.begin(),order.begin()+arpCount_,[&](int a,int b){return std::make_pair(arpKeys_[a].note,a)<std::make_pair(arpKeys_[b].note,b);});
  const int span=arpCount_*octaves;
  int pos=int(arpIndex_%span);
  if(mode==1)pos=span-1-pos;
  if(mode==2&&span>1){const int t=int(arpIndex_%(2*span-2));pos=t<span?t:2*span-2-t;}
  if(mode==4)pos=(pos+int(arpBits_>>(step%12)))%span;
  const auto& key=arpKeys_[order[pos%arpCount_]];
  int note=key.note+12*(pos/arpCount_);while(note>127)note-=12;
  const double accent=((arpBits_>>step)&1)?1.:1.-.65*get(6022)*.01;
  voiceOn(note,key.vel*accent,key.ch,-1);arpSound_=note;arpChannel_=key.ch;++arpIndex_;
 }
 if(++arpStep_>=length){
  arpStep_=0;const int rules[]={30,90,110,150};int rule=rules[std::clamp(int(get(4003)),0,3)];
  unsigned next=0;for(int i=0;i<16;++i){int pattern=((arpBits_>>((i+15)%16)&1)<<2)|((arpBits_>>i&1)<<1)|(arpBits_>>((i+1)%16)&1);next|=((rule>>pattern)&1)<<i;}arpBits_=next?next:1;
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
