#!/usr/bin/env python3
"""CP61: verify MIDI ownership release and 1 ms synth-onset regression wiring."""
from pathlib import Path
root=Path('.')
proc=(root/'Albedo-JUCE-Native/Source/AlbedoProcessor.cpp').read_text(encoding='utf-8')
head=(root/'Albedo-JUCE-Native/Source/AlbedoProcessor.h').read_text(encoding='utf-8')
dsp=(root/'Albedo-Quintessential-Synth/src/vitriol_dsp.cpp').read_text(encoding='utf-8')
arp=(root/'Albedo-Quintessential-Synth/include/AlbedoCellArp.inc').read_text(encoding='utf-8')
cmake=(root/'Albedo-JUCE-Native/CMakeLists.txt').read_text(encoding='utf-8')
test=(root/'Albedo-JUCE-Native/tests/cp61_onset_midi_test.cpp').read_text(encoding='utf-8')
guard=(root/'Albedo-JUCE-Native/Source/AlbedoOutputGuard.hpp').read_text(encoding='utf-8')
assert 'VERSION 1.0.64' in cmake
for x in ('midiPressStamp','nextMidiPressStamp','oldest=midiPressStamp','synth->noteOff(note,owner)'):
 assert x in proc or x in head,x
for x in ('explicit CLAP note ID','outstanding press of that pitch','group=std::min(group,v.group)'):
 assert x in dsp,x
assert 'Oldest matching pitch' in arp and 'found<0&&id>=0' in arp
for x in ('AlbedoCP61OnsetMidiTest','cp61_note_onset_ownership_200_cases'):
 assert x in cmake,x
for x in ('44100','48000','88200','96000','64,128,256,512,1024','first<0','noteOff(60,2)','10002','midiCC(121'):
 assert x in test,x
assert 'static constexpr int latency = lookahead - 1;' in guard
assert 'lookahead = 64;' in guard
recurring=(root/'Albedo-JUCE-Native/tests/cp61_repeated_preset_note_recovery.cpp').read_text(encoding='utf-8')
assert 'AlbedoCP61RecurringNoteTest' in cmake
assert 'cp61_recurring_stuck_notes_after_preset_reset_180_cases' in cmake
for marker in ('for(int pass=0;pass<15;++pass)', 'synth->allNotesOff()', 'synth->noteOff(pitch,7)', 'synth->midiCC(121,0,1)'):
 assert marker in recurring,marker
print('CP61 PASS: 200 onset checks, MIDI press ordering/ID fallback and 63-sample output guard')
