#!/usr/bin/env python3
"""CP61: deterministic MIDI note ownership and onset safety, applied after CP59/60.
Do not alter synthesis sample data, envelopes, arpeggiator patterns or effects.
"""
from pathlib import Path
import argparse
p=argparse.ArgumentParser();p.add_argument('--root',type=Path,default=Path('Albedo-Quintessential-Synth'))
a=p.parse_args()
cp=a.root/'src/vitriol_dsp.cpp';arp=a.root/'include/AlbedoCellArp.inc'
s=cp.read_text(encoding='utf-8'); t=arp.read_text(encoding='utf-8')
def change(x,old,new,marker):
 c=x.count(old)
 if c!=1:raise ValueError(f'CP61 {marker}: expected one anchor, found {c}')
 return x.replace(old,new)
s=change(s,''' if(noteId>=0){
  for(const auto&v:voices_)if(v.active&&v.held&&v.noteId==noteId&&
       (channel<0||v.ch==channel))group=std::min(group,v.group);
 }''', ''' if(noteId>=0){
  // An explicit CLAP note ID is authoritative even if a host bridge changes
  // the MIDI channel. Prefer ID over pitch so simultaneous same-pitch notes
  // on other channels remain held.
  for(const auto&v:voices_)if(v.active&&v.held&&v.noteId==noteId)
   group=std::min(group,v.group);
 }''','CLAP note ID')
s=change(s,''' // Some hosts drop the channel on a note-off. An exact pitch fallback is safe
 // only if exactly one channel owns that note, else preserve chord isolation.
 if(group==UINT64_MAX&&note>=0){
  int foundChannel=-1;bool ambiguous=false;
  for(const auto&v:voices_)if(v.active&&v.held&&v.note==note){
   if(foundChannel>=0&&foundChannel!=v.ch){ambiguous=true;break;}
   foundChannel=v.ch;group=std::min(group,v.group);
  }
  if(ambiguous)group=UINT64_MAX;
 }''',''' // If a device emits Note Off on an incorrect channel, select the oldest
 // outstanding press of that pitch. Never leave an ambiguous same-pitch chord
 // permanently held; release exactly ONE group per Note Off. Correct-channel
 // matches remain first priority. No allocations or note re-triggering.
 if(group==UINT64_MAX&&note>=0){
  for(const auto&v:voices_)if(v.active&&v.held&&v.note==note)
   group=std::min(group,v.group);
 }''','oldest pitch fallback')
t=change(t,''' if(found<0&&id<0&&n>=0){
  int owner=-1;bool ambiguous=false;
  for(int i=0;i<arpCount_;++i)if(arpKeys_[i].down&&arpKeys_[i].note==n){
   if(owner>=0&&arpKeys_[owner].ch!=arpKeys_[i].ch){ambiguous=true;break;}
   if(owner<0)owner=i;
  }
  if(!ambiguous)found=owner;
 }''',''' // A mismatched MIDI channel may occur when a keyboard bridge reconnects.
 // Oldest matching pitch is a deterministic fallback for overlapping notes.
 // Correct-channel and explicit note-ID matching always take precedence.
 if(found<0&&id>=0){
  for(int i=0;i<arpCount_;++i)
   if(arpKeys_[i].down&&arpKeys_[i].id==id){found=i;break;}
 }
 if(found<0&&n>=0){
  for(int i=0;i<arpCount_;++i)
   if(arpKeys_[i].down&&arpKeys_[i].note==n){found=i;break;}
 }''','ARP key fallback')
cp.write_text(s,encoding='utf-8');arp.write_text(t,encoding='utf-8')
print('CP61 note ownership applied: exact ID, FIFO fallback, matching ARP gate eviction')