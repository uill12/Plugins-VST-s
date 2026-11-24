# Albedo CP59 — WaveEdit CC0 Concept Tree + MIDI lifecycle

Parent: CP58R9 (albedo/cp58r9-voicing), commit 60191753961c5684c096da949bb77102d46cbf67.
New branch: albedo/cp59-waveedit-concepttree-note-safety.
Plugin: Windows x64 CLAP / VST3, version 1.0.59, JUCE 8 and WebView2.
Status: development. Do not claim Windows verification until GitHub Actions passes.

## Reference and licensing
User-provided wavedit-online-main.zip, 705 mono 16-bit / 44,100 Hz WAVs, each 16,384 samples (64 single cycles of 256).
The uploaded archive README declares CC0 1.0. Original project: https://github.com/smpldsnds/wavedit-online, commit 1a8d80f7a062f36eca212f92f24f2ca6329b88ff.
Every selected source is re-validated with a pinned SHA-256 before use; original source never loaded on the DAW audio thread.
Source bundle may be retrieved from that pinned public revision; if unavailable, do not silently substitute different material.

## Concept Tree
Acquire reference → classify by timbral function (bass, pad, keys, lead, pluck, digital, organic) → parameterize by spectral material and envelope/role → permutative pairing based on contrast and timbral continuity → generate new sounds → validate distinctness and retain categories.
24 selected WaveEdit 64-frame wavetables were chosen from 705 candidates by continuity, distinct spectral material, energy, and categorical utility. Each produces a 48-cycle stable-pitch periodic granular texture via morphology with an independently selected sibling.
A separate offline manifest records exact 24 names, hashes, pairings and role metadata. It is not a preset filter.
Preserve the original 32 PCM24 materials (IDs 0..31), append new WaveEdit IDs 32..55. Maintain previous 192 program IDs, and revoice 63 via actual source/roles. No non-WaveEdit preset forced to enable arp.

## DSP architecture
Separate Windows RCDATA 103 includes 24 tables, 8 FFT-mip levels, 24 48-cycle granular sources. RCDATA 102 remains the full original PCM24 sample bank.
Synth loads and parses resources during prepare only, not audio-thread process. Wave Edit morphology uses host MIDI note frequency, no deliberate pitch-jitter or unbounded voice growth. CPU remains subject to ASIO validation.

## Stuck note / transport corrections
- Force full DSP and FX reset when ASIO callback resumes after >2 seconds, transport stops, and CC120 / MIDI STOP.
- Maintain per-channel MIDI held-note ownership; CC123 uses a channel-local release.
- Handle CC121 reset all controllers, including sustain pedal.
- For arpeggiated notes, release a mismatched-channel note-off ONLY when the held pitch has a unique owner; do not steal notes from other channels.

## QA gates
Local C++ DSP tests: 40 rendered cases (methods 1 and 3, rates 44.1/48/88.2/96 kHz, blocks 64/128/256/512/1024), 12-note chords, finite output, clean CC121 sustained release, mismatched-channel note-off, no leaked voice. 24 source hashes validated. 63/192 revoicing from Concept Tree validated locally.
Windows Actions still required: build resource embedding, 192-preset audio QA, 18,336 output comparisons, GUI regression, PE exports and WAV byte integrity. Bitwig manual ASIO4ALL test remains pending even if CI passes.

## Rollback
CP58R9 build CI https://github.com/uill12/Plugins-VST-s/actions/runs/45278215565 (passed). Preserve last known working binaries; no overwrites or binary claims prior to QA.
