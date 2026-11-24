#!/usr/bin/env python3
"""CP60 additive patch atop imported user CP59 FDN/effects refinement.
Avoids unbounded full reverb memset in a 64-frame ASIO callback and extends
Space/Mix modulation coherently to chorus and diffusion, without new IDs.
"""
from pathlib import Path
import argparse
p=argparse.ArgumentParser();p.add_argument('--root',required=True,type=Path);a=p.parse_args()
hp=a.root/'include/vitriol_dsp.hpp';cp=a.root/'src/vitriol_dsp.cpp'
h=hp.read_text(encoding='utf-8');s=cp.read_text(encoding='utf-8')
def swap(src,old,new,lab):
 n=src.count(old)
 if n!=1:raise RuntimeError(f'CP60 missing/unexpected {lab}: {n} matches')
 return src.replace(old,new)
h=swap(h,'double dampA=0,modDepth=0,outScale=1,inScale=1,lenSlew=1;bool lenValid=false;',
   'double dampA=0,modDepth=0,outScale=1,inScale=1,lenSlew=1;bool lenValid=false;\n  size_t clearCursor=0,clearRemaining=0; // bounded real-time reactivation',
   'reverb clear cursor')
s=swap(s,'rev_.pos.fill(0);rev_.lp.fill(0.);rev_.lenValid=false;',
       'rev_.pos.fill(0);rev_.lp.fill(0.);rev_.lenValid=false;rev_.clearCursor=rev_.clearRemaining=0;',
       'reset cursor')
s=swap(s,'if(k==2){for(auto& ll:rev_.line)std::fill(ll.begin(),ll.end(),0.0);rev_.lp.fill(0.);rev_.pos.fill(0);for(auto& f:fx_)for(auto& ap:f.inputDiffuse)ap.clear();}',
       '''if(k==2){
     // Do not memset the entire 8-line FDN in a 64-sample ASIO block.
     // Delay memory is cleared in small deterministic slices before audible wet output.
     rev_.lp.fill(0.);rev_.pos.fill(0);
     rev_.clearCursor=0;rev_.clearRemaining=rev_.mask+1;
     for(auto& f:fx_)for(auto& ap:f.inputDiffuse)ap.clear();
    }''','avoid FDN RT memset')
s=swap(s,'if(settle(2,fxRevMix_,tRev)){\n  double wl=0,wr=0;reverbTick(l,r,wl,wr);',
'''if(settle(2,fxRevMix_,tRev)){
  if(rev_.clearRemaining){
   // At most 32 samples per delay line per host sample; constant upper bound.
   // Existing feedback must never leak from the previous bypassed state.
   const size_t count=std::min<size_t>(32,rev_.clearRemaining);
   for(auto& line:rev_.line)std::fill_n(line.begin()+rev_.clearCursor,count,0.0);
   rev_.clearCursor+=count;
   rev_.clearRemaining-=count;
  }
  double wl=0,wr=0;
  if(rev_.clearRemaining==0)reverbTick(l,r,wl,wr);''',
'bounded FDN cleanup')
s=swap(s,''' const double tCh=get(5003)>.5?get(5002)*.01:0.;
 const double tRev=get(5005)>.5?clamp((get(5004)+modFrame_[8]*20)*.01,0,1):0.;
 const double tDif=get(5007)>.5?get(5006)*.01:0.;''',
''' // Mod Matrix destination 8 is "Space / Mix". Send it to all three
 // spatial effects in proportion to their existing wet controls.
 // A zero route is an exact backwards-compatible bypass.
 const double space=clamp(modFrame_[8],-1.,1.);
 const double tCh=get(5003)>.5?clamp((get(5002)+space*10.)*.01,0.,1.):0.;
 const double tRev=get(5005)>.5?clamp((get(5004)+space*20.)*.01,0.,1.):0.;
 const double tDif=get(5007)>.5?clamp((get(5006)+space*10.)*.01,0.,1.):0.;''',
 'shared spatial modulation routing')
hp.write_text(h,encoding='utf-8');cp.write_text(s,encoding='utf-8')
print('CP60: bounded real-time FDN clears, spatial modulation integrated, no param ID changes')