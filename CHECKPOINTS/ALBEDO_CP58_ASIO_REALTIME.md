# Albedo CP58 — ASIO realtime DSP optimization

Date: 2026-11-21
Branch: `albedo/cp58-asio-realtime-dsp`
Parent: CP57 (bit-perfect rollback available)
Version: 1.0.58, Windows x64 CLAP + VST3.
GUI: unchanged CP56 + CP57 tempo controls.
Preset: 192 existing IDs, 32 embedded PCM24 source materials, no bank regeneration.

## User symptom
Crackling through ASIO reduces when buffer size is increased. This is **consistent**
with an overloaded audio deadline but does not distinguish CPU underruns from
driver/DPC instability without a measurement inside the user's Bitwig session.
CP57's 192-preset audit reported maximum rendering cost = 0.937003 times its
host-time budget in one 2048-sample chord scenario; this was not a low-buffer QA.

## Root cause candidates and fixes
1. `sourceFilter` previously called `db(drive)` (pow10) once per source per
   voice per internal audio sample whenever source filters were active.
   Cache the exact gain and gain normalization once per host/event chunk.
2. Source filter wet-bypass/cutoff smoothing rates, resonance k coefficient,
   unison sqrt denominators and envelope rates are invariant during each block.
   Reuse them from a block-level snapshot. Dynamic filter states, oversampling,
   modulations, resonances, nonlinear shape and voice count remain unchanged.
3. The `Delay::read` inner loop did `std::fmod` for every chorus/reverb/diffuse
   delay tap, despite all builtin tap lengths being smaller than the delay ring.
   Use one-step wrapping for normal taps and the original fmod for invalid or
   out-of-range taps. No resampling, modulation or algorithm changes.
4. Keep JUCE float64 processing, WebView2 and CP57 PPQ sync intact.

## Local engineering QA
- Reconstructed exact CP57 DSP from successful GitHub Actions source audit.
- Independently compiled baseline and optimized C++20 high-fidelity synth with
  `-O3 -DALBEDO_CP53_TRANSPARENT_OUTPUT` on Linux.
- 12 simultaneous voices; four source engines and source filters; chorus,
  saturation and reverb active. 15 pairs at 44.1/48/96 kHz, buffer 64/128/256/
  512/1024 samples: **15/15 binary-identical float64 PCM comparisons**.
- Repeated local wall-clock samples showed generally 8–16% lower processing
  load. OS scheduling produced outliers; measurements are not a claim of
  guaranteed Windows/ASIO throughput or a specific percent on the user's PC.

## Windows CI gates
1. Reconstruct and validate original PCM24; apply CP50, CP53, then CP58 patch.
2. Run CP58 source contract and CP41/CP50/CP57 regressions.
3. Build Windows CLAP, VST3 and `AlbedoCP58AsioBlocks` CTest program.
4. Report 20 scenarios (4 sample rates x 5 buffer sizes) with 12 notes.
5. Verify Windows native PE exports, embedded bank, metadata, SHA-256,
   192-preset HiFi QA and 18,336 preset-pair comparisons.
6. Publish separate versioned artifacts; do not overwrite CP57.

## Validation boundary
No physical ASIO hardware or Bitwig playback was available. After installation
test 44.1/48/88.2/96 kHz at buffer sizes 64–1024, solo and with other plugins,
MIDI chords and presets with all 4 sources. If crackling persists with CPU
headroom, investigate Windows DPC/ASIO driver. A 256/512 buffer is a temporary
safe playback setting; it is not the DSP fix.
