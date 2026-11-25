#pragma once
#include <chrono>

// CP64: rearm after MIDI inactivity, not after a long musical note.
// The clock is monotonic and is only checked on MIDI events (no GUI polling).
namespace albedo {
struct MidiIdleGuard {
 using Clock=std::chrono::steady_clock;
 static constexpr auto idleLimit=std::chrono::seconds(30);
 Clock::time_point lastMidi{};
 void arm(Clock::time_point now) noexcept { lastMidi=now; }
 bool onNoteOn(Clock::time_point now,bool anyKeysHeld,
               bool sustainPedalDown) noexcept {
  const bool expired=lastMidi!=Clock::time_point{} &&
       now>lastMidi && now-lastMidi>idleLimit;
  lastMidi=now;
  // Only physical key/sustain ownership protects long notes. A running DAW
  // transport without MIDI is still idle; its first new key must rearm too.
  return expired&&!anyKeysHeld&&!sustainPedalDown;
 }
 void onOtherMidi(Clock::time_point now) noexcept { lastMidi=now; }
};
}
