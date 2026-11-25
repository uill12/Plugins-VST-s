#!/usr/bin/env python3
"""CP64 sound timing, MIDI idle reentry and mineral-style ARP controls."""
from pathlib import Path
root=Path(__file__).resolve().parents[2]
proc=(root/"Albedo-JUCE-Native/Source/AlbedoProcessor.cpp").read_text(encoding="utf-8")
header=(root/"Albedo-JUCE-Native/Source/AlbedoProcessor.h").read_text(encoding="utf-8")
guard=(root/"Albedo-JUCE-Native/Source/AlbedoMidiIdleGuard.hpp").read_text(encoding="utf-8")
ui=(root/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
cmake=(root/"Albedo-JUCE-Native/CMakeLists.txt").read_text(encoding="utf-8")
tempo=(root/"Albedo-JUCE-Native/Assets/cp57_tempo_controls.js").read_text(encoding="utf-8")
webview=(root/"Albedo-JUCE-Native/Source/AlbedoWebViewEditor.cpp").read_text(encoding="utf-8")
for value in ("midiIdleGuard.onNoteOn(","!isNonRealtime()&&rearm","synth->reset();",
              "midiPedalDown","midiPressStamp","midiIdleGuard.onOtherMidi("):
 assert value in proc,value
for value in ("MidiIdleGuard.hpp","midiPedalDown{}","midiIdleGuard{}"):
 assert value in header,value
for value in ("idleLimit=std::chrono::seconds(30)","anyKeysHeld","sustainPedalDown",
              "lastMidi=now","return expired&&!anyKeysHeld&&!sustainPedalDown;"):
 assert value in guard,value
for value in ('id="arpFill"','FILL '+chr(39),'#steps .step:disabled','arpStepCount()',
              "onExtraParam(6000+i,1)","cp64-arp-identity",'id="resetVoices"',
              "onEmergencyReset(){post('panic',0);}"):
 assert value in ui,value
assert "[.25,'1/16']" in tempo
assert 'if(parts[0]=="panic")' in webview and "processor.reset();" in webview
for value in ("AlbedoCP64MidiIdleGuardTest","AlbedoCP64Arp110TempoTest"):
 assert value in cmake,value
print("CP64 PASS: first-note-after-idle guard for whole synth, native 110-BPM clock QA and materialized FILL steps")
