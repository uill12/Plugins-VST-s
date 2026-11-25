#include "../Source/AlbedoMidiIdleGuard.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
using Clock=albedo::MidiIdleGuard::Clock;
using namespace std::chrono_literals;
int main(){
 albedo::MidiIdleGuard g;
 const auto origin=Clock::time_point{}+100s;
 g.arm(origin);
 assert(!g.onNoteOn(origin+1s,false,false,false));
 assert(!g.onNoteOn(origin+20s,false,false,false));
 // No MIDI events for 60 seconds: reset DSP before first new key.
 assert(g.onNoteOn(origin+81s,false,false,false));
 // Immediate 2nd key or the note-off after it should never rearm.
 assert(!g.onNoteOn(origin+81s+2ms,false,false,false));
 g.onOtherMidi(origin+83s);
 assert(!g.onNoteOn(origin+84s,false,false,false));
 // A deliberately held pad, sustain pedal, and an active transport must
 // survive arbitrary time spans without forcibly resetting the synth.
 g.arm(origin);
 assert(!g.onNoteOn(origin+180s,true,false,false));
 g.arm(origin);
 assert(!g.onNoteOn(origin+180s,false,true,false));
 g.arm(origin);
 assert(!g.onNoteOn(origin+180s,false,false,true));
 // Only a subsequent first free key after a NEW idle gap may rearm again.
 assert(g.onNoteOn(origin+270s,false,false,false));
 // Uninitialized timing is conservative (no bogus reset on construction).
 albedo::MidiIdleGuard fresh;
 assert(!fresh.onNoteOn(origin,false,false,false));
 std::cout<<"CP64 MIDI first-note idle reentry PASS (sustain/held/transport safe)\n";
}
