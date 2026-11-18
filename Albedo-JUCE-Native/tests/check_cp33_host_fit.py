#!/usr/bin/env python3
"""CP33: native JUCE 3:2 window fit at 1050..1800 and avoidance of clipped controls."""
from pathlib import Path
s=(Path(__file__).resolve().parents[1]/'Source/AlbedoEditor.cpp').read_text(encoding='utf-8')
assert 'constrainer.setFixedAspectRatio(1.5)' in s
assert 'setSize(1110,740)' in s
assert 'constrainer.setSizeLimits(1050,700,1800,1200)' in s
assert 'auto r=juce::Rectangle<int>(10,78,juce::jmax(0,W-20),juce::jmax(0,H-102))' in s
assert 'juce::jlimit(76,126,int(filt.getHeight()*.27f))' in s
for w,h in [(1050,700),(1110,740),(1200,800),(1290,860),(1440,960),(1800,1200)]:
 assert w*2==h*3
 rw,rh=w-20,h-102
 engine=max(93,min(135,int(rh*.16)))
 lower=max(155,min(275,int(rh*.27)))
 upper=rh-engine-lower-16
 assert engine>77 and lower>146 and upper>250,(w,h,engine,lower,upper)
 filt=upper-16-45
 section=max(76,min(126,int(filt*.27)))
 graph=max(30,min(46,section//2))
 knobs=section-graph
 remain=filt-2*(section+3)
 envgraph=max(31,min(46,remain//3))
 envknobs=remain-envgraph
 assert knobs>=38 and envknobs>=48,(w,h,section,graph,knobs,envknobs)
 rack=engine-16-37
 enginegraph=max(24,min(32,rack//2))
 assert rack-enginegraph-3>=18,(w,h,rack,enginegraph)
 bottomcontent=lower-16-47
 assert bottomcontent>=90,(w,h,bottomcontent)
 for graphsize in [min(72,max(40,bottomcontent//2)),min(46,max(30,bottomcontent//3)),min(58,max(35,bottomcontent//2))]:
  assert bottomcontent-graphsize>=38,(w,h,bottomcontent,graphsize)
 print('PASS GUI fit',w,h,'envelope knobs',envknobs,'rack',rack)
print('CP33: all 8 module bounds, filter ADSR and rack rows satisfy 3:2 layout geometry')
