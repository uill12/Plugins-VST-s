#!/usr/bin/env python3
"""CP57 DAW tempo-sync source contract; native CPP render QA runs separately."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[2]
proc=(root/'Albedo-JUCE-Native/Source/AlbedoProcessor.cpp').read_text(encoding='utf-8')
applier=(root/'patches/cp50/apply.py').read_text(encoding='utf-8')
scheduler=(root/'patches/cp50/AlbedoCellArp.inc').read_text(encoding='utf-8')
programs=(root/'Albedo-JUCE-Native/Source/AlbedoPrograms.hpp').read_text(encoding='utf-8')
cmake=(root/'Albedo-JUCE-Native/CMakeLists.txt').read_text(encoding='utf-8')
editor=(root/'Albedo-JUCE-Native/Source/AlbedoWebViewEditor.cpp').read_text(encoding='utf-8')
ui=(root/'Albedo-JUCE-Native/Assets/cp57_tempo_controls.js').read_text(encoding='utf-8')
test=(root/'patches/cp50/arp_test.cpp').read_text(encoding='utf-8')
assert 'getPpqPosition()' in proc and 'setArpTransport(ppq,playing)' in proc
assert 'setArpTransport(double ppq,bool playing)' in applier
assert 'arpTransportValid_' in applier and 'arpSnap()' in applier
for token in ('void Synth::setArpTransport','void Synth::arpSnap','arpTransportBeat_+=double(n)','get(6030)>.5','get(6026)','arpRelease();arpGate_=0.'):
 assert token in scheduler,token
assert 'if(hasArp&&!hasExplicitClock)set(6030,1.0f)' in programs
for token in ('cp57_tempo_controls.js','cp57_tempo_controls_js'):
 assert token in editor or token in cmake,token
for token in ('DAW SYNC','FREE','1/4','1/8','1/16','1/32','setExtraParam?.(id,v)'):
 assert token in ui,token
assert re.search(r'VERSION 1\.0\.(?:57|58)',cmake), 'CP57 tempo sync remains compatible with CP58 metadata'
for rate in (44100,48000,88200,96000):
 assert str(rate) in test
assert 'synced->setArpTransport(.0625,true)' in test
assert 'synced->setArpTransport(8.0,true)' in test
print('CP57 PASS: host PPQ, swung grid, rhythmic-preset defaults, true clock GUI, four sample rates')
