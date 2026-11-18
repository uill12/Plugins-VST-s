#!/usr/bin/env python3
"""CP32 native GUI 3:2 geometry regression at minimum, launch, and large sizes."""
from pathlib import Path
s=(Path(__file__).resolve().parents[1]/'Source/AlbedoEditor.cpp').read_text(encoding='utf-8')
required=[
 'setSize(1290,860);', 'constrainer.setFixedAspectRatio(1.5);',
 'constrainer.setSizeLimits(1260,840,1920,1280);',
 'const int graphHeight=juce::jlimit(24,32,visual.getHeight()/2);',
 'juce::jmax(103,int(filt.getHeight()*.285f))',
 'startTimerHz(24)', 'startTimerHz(30)'
]
assert all(a in s for a in required),[a for a in required if a not in s]
for w,h in [(1260,840),(1290,860),(1440,960),(1600,1066),(1920,1280)]:
 assert abs(w/h-1.5)<.003
 rh=h-120; eng=max(118,int(rh*.15)); lower=max(200,int(rh*.265))
 upper=rh-eng-lower-16
 visual=eng-61; graph=min(32,max(24,visual//2))
 knob=max(18,visual-graph-3)
 assert graph+3+knob<=visual,('engine overlap',w,h)
 filter_h=upper-61; band=max(103,int(filter_h*.285))
 assert band-46>=53,('filter knob too small',w,h)
 envelope=filter_h-2*band-10-49
 assert envelope>=55,('envelope controls clipped',w,h,envelope)
 assert lower-16-47-72-3>=62,('cellular controls clipped',w,h)
 assert lower-16-47-46-4>=85,('FX controls clipped',w,h)
 print('PASS 3:2 native geometry',w,h,'envelope',envelope)
print('CP32-R2 no known module/control overflow at minimum editor size')
