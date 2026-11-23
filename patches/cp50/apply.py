import sys,shutil,re
from pathlib import Path
root=Path(sys.argv[1]);src=root/'src/vitriol_dsp.cpp';hdr=root/'include/vitriol_dsp.hpp';params=root/'include/vitriol_params.hpp'
def change(s,a,b):
    assert s.count(a)==1,a
    return s.replace(a,b)
h=hdr.read_text();h=change(h,' void prepare(double',' void setArpTempo(double bpm);\n void setArpTransport(double ppq,bool playing);\n void prepare(double')
h=change(h,' struct Voice {',''' struct ArpKey {int note,ch,id;double vel;bool down;};
 std::array<ArpKey,128> arpKeys_{};std::array<bool,16> arpPedal_{};
 int arpCount_=0,arpSound_=-1,arpChannel_=0,arpStep_=0;unsigned arpIndex_=0,arpBits_=0x8421;
 bool arpEnabled_=false,arpHostValid_=false,arpTransportValid_=false;double arpHostTempo_=120,arpClock_=0,arpGate_=0,arpTransportBeat_=0;
 void arpMode();void arpTick();void arpClear();void arpPrune();void arpRelease();void arpSnap();
 void voiceOn(int,double,int,int);void voiceOff(int,int,int);void voicePanic();
 template<typename Sample>void processArp(Sample*,Sample*,uint32_t);
 struct Voice {''')
h=h.replace('double unisonPhase','double sampleSide[4]{};double unisonPhase')
h=h.replace('materialBank_{}','materialBank_{},materialSide_{}').replace('double normalizedPhase)const','double normalizedPhase,double& side)const')
hdr.write_text(h)
p=params.read_text()
p=change(p,' add(6020,',''' add(6024,"Arp Character","Cellular Arpeggiator",0,3,2,"",true);
 add(6025,"Arp Octaves","Cellular Arpeggiator",1,3,1,"",true);
 add(6026,"Arp Swing","Cellular Arpeggiator",0,45,0,"%");
 add(6027,"Arp Steps","Cellular Arpeggiator",4,16,8,"",true);
 add(6028,"Arp Rotate","Cellular Arpeggiator",0,7,0,"",true);
 add(6029,"Arp Tempo","Cellular Arpeggiator",40,240,120,"BPM");
 add(6030,"Arp DAW Sync","Cellular Arpeggiator",0,1,1,"",true);
 add(6020,''')
# Retain prior ID ownership for 6020-6023 (existing DAW state and automation).
# Use Nereida's gate/velocity defaults and units instead of CP50's pulse nomenclature.
for ident,old_name,new_name,default in (
    (6020,"Pulse Enabled","Arpeggiator Enabled",None),
    (6021,"Pulse Division","Arp Division",None),
    (6022,"Pulse Depth","Arp Cellular Accent",30),
    (6023,"Pulse Gate","Arp Gate",68),
):
    pat=rf'add\({ident},[^;]+;'
    match=re.search(pat,p)
    assert match is not None,('missing legacy arp parameter',ident)
    item=match.group(0)
    assert old_name in item,(ident,item)
    modified=item.replace(old_name,new_name).replace('"Pulsus"','"Cellular Arpeggiator"')
    if ident==6021:
        modified=modified.replace(',"s")',',"beats")')
    if default is not None:
        # Keep original exposed min/max and update just default.
        fields=modified.split(',')
        assert len(fields)>=7,(ident,modified)
        fields[5]=str(default)
        if ident==6023:fields[3],fields[4]='10','95'
        modified=','.join(fields)
    p=p.replace(item,modified)
p=change(p,' return p;', ' add(6032,"Arp Time Signature","Cellular Arpeggiator",0,7,0,"",true);\n return p;')
params.write_text(p)
s=src.read_text()
s=s.replace('void Synth::noteOn(','void Synth::voiceOn(').replace('void Synth::noteOff(','void Synth::voiceOff(').replace('void Synth::allNotesOff(){','void Synth::voicePanic(){')
s=change(s,'void Synth::reset(){','void Synth::reset(){arpClear();arpEnabled_=false;')
s=change(s,'void Synth::allSoundOff(){','void Synth::allSoundOff(){arpClear();')
s=change(s,' if(cc==1)modwheel_', ''' if(cc==64){arpPedal_[channel]=val>=64;arpPrune();if(arpEnabled_)return;}
 if(cc==123){arpPedal_[channel]=false;for(int i=0;i<arpCount_;++i)if(arpKeys_[i].ch==channel)arpKeys_[i].down=false;arpPrune();if(arpSound_>=0&&arpChannel_==channel)arpRelease();}
 if(cc==1)modwheel_''')
a=s.index('   // CP06: amplitude pulse');b=s.index('   // 8 ms gain',a);s=s[:a]+s[b:]
s=s.replace('void Synth::process(float*','#include "AlbedoCellArp.inc"\nvoid Synth::process(float*').replace('{processTyped(l,r,n);}','{processArp(l,r,n);}')
s=s.replace('std::memcmp(bytes,"VTRB1\\0\\0\\0",8)!=0','(std::memcmp(bytes,"VTRB1\\0\\0\\0",8)!=0&&std::memcmp(bytes,"VTRB2\\0\\0\\0",8)!=0)')
s=s.replace('20ull+3ull*count*n','20ull+(bytes[4]==\'2\'?6ull:3ull)*count*n')
s=s.replace('std::array<std::vector<float>,32> staging;','std::array<std::vector<float>,32> staging,sideBank;')
s=s.replace('  staging[i].resize(n);','  for(int plane=0;plane<(bytes[4]==\'2\'?2:1);++plane){auto& dst=plane?sideBank[i]:staging[i];dst.resize(n);')
s=s.replace('   staging[i][j]=','   dst[j]=').replace(' materialBank_.swap(staging);',' }\n materialSide_.swap(sideBank);materialBank_.swap(staging);')
s=s.replace('int index,double phase)const{','int index,double phase,double& side)const{')
s=s.replace(' auto at=[&](int j){return double(v[(j+int(v.size()))%int(v.size())]);};\n return cubic(at(i-1),at(i),at(i+1),at(i+2),t);',''' auto read=[&](const std::vector<float>& data){
  auto at=[&](int j){return double(data[(j+int(data.size()))%int(data.size())]);};
  return cubic(at(i-1),at(i),at(i+1),at(i+2),t);
 };
 side=materialSide_[index].empty()?0.:read(materialSide_[index]);return read(v);''')
for phase in ['phase','position','pos']:
    s=s.replace('+10)),'+phase+')','+10)),'+phase+',v.sampleSide[e])')
s=change(s,' int m=int(get(1000+32*e));',' v.sampleSide[e]=0;int m=int(get(1000+32*e));')
s=change(s,'  double transient=','  if(m==2||m==3||m==7)unisonSide[e]+=v.sampleSide[e]*(m==2?.15*d:m==3?std::abs(std::sin(3.141592653589793*v.grainPhase[e])):1.);\n  double transient=')
src.write_text(s)
shutil.copy(Path(__file__).with_name('AlbedoCellArp.inc'),root/'include')
# Publish only the clock indicator across threads; MIDI ownership stays on the audio thread.
s=src.read_text()
s=s.replace('void Synth::noteChoke(int note,int channel,int noteId){','''void Synth::noteChoke(int note,int channel,int noteId){
 for(int i=arpCount_-1;i>=0;--i){const auto k=arpKeys_[i];if((channel<0||k.ch==channel)&&(noteId>=0?k.id==noteId:note<0||k.note==note)){for(int j=i+1;j<arpCount_;++j)arpKeys_[j-1]=arpKeys_[j];--arpCount_;}}
 if(!arpCount_)arpRelease();''')
src.write_text(s)
h=hdr.read_text().replace(' void setArpTempo',' int arpStep()const{return arpDisplay_.load(std::memory_order_relaxed);}\n int arpNote()const{return arpSound_;}\n void setArpTempo').replace(' struct ArpKey',' std::atomic<int> arpDisplay_{-1};\n struct ArpKey')
hdr.write_text(h)
