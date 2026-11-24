# Albedo CP61 — MIDI note-on onset and note-off ownership safety

Baseline: verified CP60R2 v1.0.60, commit `ecfb1d391781c98f128028fed247e1dcee719955`.
CP61: v1.0.61, branch `albedo/cp61-midi-onset-note-ownership`.
Retain original 192 preset IDs, 32 PCM24 materials, 24 WaveEdit CC0 wavetables and granular textures, CP60 FDN/ADAA FX, CP57 tempo sync, WebView2 HTML, and plugin IDs.

## User-reported defects
- Key strike is heard late **even with the arpeggiator OFF and amp attack set to 1 ms**.
- Notes remain active after MIDI Note Off.
- Playback uses Bitwig and ASIO4ALL; the earlier crackling decreased with larger blocks.

## Direct DSP onset findings
Local native C++20 tests at 44.1/48/88.2/96 kHz, buffers 64/128/256/512/1024, ten source modes: in the base synth a 1 ms envelope with arp disabled already yields PCM within milliseconds of a Note On. Typical first nonzero sample at 48 kHz was ~0.33–0.6 ms, depending on source and FX. The downstream transparent output guard reports 63 samples fixed lookahead (=1.3125 ms at 48 kHz), and the host compensates for this latency. It is not technically sound to attribute a large strike delay to attack time or the disabled arpeggiator without a host/driver latency measurement. Do not fake Note On timestamps or reduce Hi-Fi output protection merely to mask ASIO delay.

## Confirmed stuck-note bug and correction
With two held voices on the same pitch, different MIDI channels, and a Note Off on an unexpected channel, CP60R2 abandoned the release because fallback ownership was marked ambiguous. Reproduced: baseline returned **two held voices** after first unmatched Note Off (expected one).

CP61:
1. Give exact CLAP note ID priority even if host bridge changed channel.
2. Otherwise release the oldest outstanding same-pitch voice group, exactly one per Note Off. Exact channel still wins, so ordinary chords are not interrupted.
3. Match the arpeggiator held-key fallback to the same oldest-press rule, including ID when possible, so switching the arp OFF cannot resurrect a stale key.
4. Add preallocated per-key MIDI press stamps to JUCE (16 channels x 128 notes) to resolve stray-channel Note Off at the processor boundary and keep the UI/processor key map coherent.
5. Preserve all FX and sound generators, processing order, programs and MIDI CC64/121/123 behavior. No dynamic allocation or lock in the audio callback.

## Regression evidence
- On the original CP60R2 DSP the 200-case onset and release regression fails on the duplicate-pitch/wrong-channel Note Off with error 5.
- Against CP61 DSP it passes: **200 onset-mode/buffer/rate cases**; duplicate pitch across channels; explicit note ID priority; arpeggiator OFF transition; sustain CC121 release.
- More demanding sample-based programs may have their own source waveform evolution, but the base DSP does not impose a global several-tens-of-milliseconds Note On wait.

## Windows QA required before promoting
Run all existing CP60R2 CTests, CP61OnsetMidiTest, 192-program Hi-Fi QA, 18,336-pair uniqueness test, resource integrity/sha256, CLAP/VST3 Windows packaging, GUI regression, target matcher.
Only then update Drive project checkpoint. Do not overwrite CP60R2.
**Hardware ASIO4ALL/Bitwig manual validation remains pending** even after CI success. For troubleshooting: solo Albedo in an empty project, no other latency-inducing plugins, 48 kHz at 128/256 then 512 samples; compare native interface ASIO driver when available. If audio is late only in Bitwig/ASIO4ALL while PCM begins immediately, measure driver input/output and host plugin delay compensation separately.

## Recurrence after changing presets (user reproduction)
User confirms a stuck note stops after changing/toggling a preset, then reappears later. CP60 preset switching invokes `pendingPanic` and `synth->allNotesOff()`, which stops held voices even when reverb is unchanged. That is consistent with an orphaned held/sustained gate and should not be mislabeled as reverb tail only. The cause of missing host MIDI Note Off events cannot be proven without Bitwig MIDI observation. Do **not** impose a timeout on legitimate held notes.

Added CP61 recurring stress regression: 180 cycles across 44.1/48/88.2/96 kHz and 64/256/1024 frames. For each cycle start three overlapping notes of the same pitch on separate channels; deliver their Note Off events on a remapped channel, verify none remain; then sustain and switch/panic preset, verify no latched voice after release; follow with CC121 reset. Local C++ test PASS (180/180). CTest registered as `cp61_recurring_stuck_notes_after_preset_reset_180_cases`.
