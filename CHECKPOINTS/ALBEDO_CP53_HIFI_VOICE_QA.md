# Albedo Quintessential Synth — CP53 Hi-Fi voice and signal-path checkpoint

**Source branch:** `albedo/cp53-hifi-voice-safety`  
**Base checkpoint:** CP52 compact Nereida-style browser  
**Format:** Windows x64 JUCE, CLAP + VST3. Stable host plugin identifiers and 192 indexed CP51 preset names/parameters.

## Why this checkpoint exists

Reported symptoms: unintentional distortion, clipping, lingering release/sustain notes, plus CPU demands across synth sources and FX. The previous DSP always used a nonlinear `tanh` at the global filter input and mix, then applied a strongly nonlinear output guard and a final ±1 hard clamp before audio left the synth. CC64 messages from the JUCE host also defaulted to MIDI channel zero regardless of incoming channel.

## Implemented in source

- **Source/filter/mix clarity:** a C1 continuous wide safety knee replaces unconditional filter-input and summing saturation in the JUCE signal path, preserving full linear response at ordinary levels. Intentional distortion/drive effects remain parameter-controlled.
- **Hi-Fi output:** the DSP skips the legacy hard ±1 limiter for the new JUCE path. A dedicated, fixed-latency 63-sample lookahead, stereo-linked true-peak-aware output guard handles transient protection with DC rejection at 1.5 Hz and a -1.11 dBFS digital ceiling. It uses a monotonic peak deque with O(1) amortized work, no audio-thread allocations or locks. The host is told its actual processing latency.
- **MIDI lifecycle:** CC64 receives its original channel; repeated notes cannot silently accumulate independently held keyboard states; program swaps and restored states trigger an audio-thread panic/release; transport stop always releases, not only when the arpeggiator is enabled; CC120/123 and MIDI Stop clear notes.
- **Preset switching:** generation-counted snapshot staging prevents a partially written factory preset from being applied by the audio thread, and avoids reallocation inside processBlock.
- **Panic release:** all-notes-off uses an accelerated, bounded smooth emergency release. Normal Note Off retains the programmed envelope (including long pads/drones).
- **Polyphony and rates:** twelve independent voices in AUTO 176.4/192 kHz are restored using the already existing half-rate internal reconstruction, while manually excessive oversampling retains an 8-voice resource budget. 44.1, 48, 88.2, 96, 176.4 and 192 kHz paths are subject to tests.
- **Compatibility:** JUCE-specific DSP macro protects other historical native synth builds; CP51/CP52 output and rollback artifacts remain untouched.

## Test evidence

- Locally compiled JUCE DSP-source reconstruction and tested the output guard under six sample rates, float/double protection, NaN/Inf sanitation, stereo correlation and arbitrary block splitting: **PASS**.
- Locally rendered 192 indexed CP51 parameter banks through 12-voice and output-protection tests with the reconstructed core and six host rates: **PASS** (without Windows PCM bank; this is not equivalent to final host audio QA).
- Verified channel-specific sustain and normal release. Added emergency release tests and full Windows PCM24-linked 192-program QA in CI.
- Windows VST3/CLAP compilation, real WebView2 MIDI/host testing, CPU profile and Bitwig playback **pending**, do not present CP53 as installed/released.

## Release gate

1. Build Windows CLAP/VST3 from this branch; run all CTest audio and UI suites.
2. Verify all 192 preset sounds with embedded 32-material PCM24 bank, no peak clipping; CTest report artifact.
3. Test host note-on/off for 12-note chords, retrigger, pedal release on channels 1–16, transport Stop, program switching and preset import; confirm no notes stuck.
4. Test at 44.1/48/88.2/96/176.4/192 kHz and 64/128/256/512-sample block sizes; measure realtime load especially with 12 voices. Check both VST3 and CLAP in Bitwig.
5. Publish as CP53 HiFi candidate only after successful native tests; preserve CP51 and CP52 as immutable rollback.

## CP53 CPU / sine-cache refinement (2026-11-21)

- JUCE HiFi uses a precomputed **16,384-sample double sine LUT** (128 KiB per synth instance) with linear interpolation; the legacy non-CP53 branch retains its original 4,096-point Catmull-Rom interpolation. The new table's theoretical maximum linear sine interpolation error is approximately 1.84e-8 (about -155 dBFS), below nominal 24-bit audio quantization noise.
- Precomputed per-oscillator unison detune ratios refreshed **once per audio processing chunk**, not `pow(2, detune)` inside every unison voice and sample; avoids redundant transcendental operations.
- A local 48 kHz, 12-voice, four-engine spectral worst-case render measured approximately 0.143 seconds previously and 0.091 seconds with the 16K LUT and cached unison ratios for 3,840 samples (**~36% faster** in that isolated test). This is not a host CPU benchmark and should be verified under actual Bitwig playback.
- Both optimizations preserved the existing 192-preset local sound/voice safety test result. Final Windows CI and host tests are still required.
