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
 assert(!g.onNoteOn(origin+1s,false,false));
 assert(!g.onNoteOn(origin+20s,false,false));
 // No MIDI events for 60 seconds: reset DSP before first new key.
 assert(g.onNoteOn(origin+81s,false,false));
 // Immediate 2nd key or the note-off after it should never rearm.
 assert(!g.onNoteOn(origin+81s+2ms,false,false));
 g.onOtherMidi(origin+83s);
 assert(!g.onNoteOn(origin+84s,false,false));
 // A deliberately held pad or sustain pedal must never be cut by an idle timer.
 g.arm(origin);
 assert(!g.onNoteOn(origin+180s,true,false));
 g.arm(origin);
 assert(!g.onNoteOn(origin+180s,false,true));
 g.arm(origin);
 // Bitwig may run its transport while no notes are present: still rearm.
 assert(g.onNoteOn(origin+180s,false,false));
 // Only a subsequent first free key after a NEW idle gap may rearm again.
 assert(g.onNoteOn(origin+270s,false,false));
 // Uninitialized timing is conservative (no bogus reset on construction).
 albedo::MidiIdleGuard fresh;
 assert(!fresh.onNoteOn(origin,false,false));
 std::cout<<"CP64 MIDI first-note idle reentry PASS (sustain/held safe; transport without MIDI rearmed)\n";
}
