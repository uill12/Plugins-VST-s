# Rubedo Atlas Drumkit — CP08 Hi-Fi QA and sound fidelity

Date: 2026-11-14. **Windows Bitwig playback and UI not host-verified.**

## Original CP07 sample bank — retained (no timbral revoice)

- Full-bank PCM decoding check: **2304/2304** stereo? No: each file is MONO signed 24-bit FLAC, 96,000 Hz. All output files remain mono; plugin sums to stereo.
- Maximum file sample peak: **-2.452 dBFS**; maximum 4× interpolated true peak: **-2.417 dBFS** across **2304** files, **0** true-peak overs (4× Kaiser resampling, 384 kHz proxy).
- Maximum absolute DC offset: **-168.38 dBFS**; maximum ending-sample absolute value: **-85.01 dBFS**. No significant start/end discontinuities flagged by thresholds.
- Velocity-layer differences across **576** pairs: min **+3.36 dB**, median **+5.75 dB** for hard vs soft RMS.
- High-band energy above 22 kHz: maximum **1.59%**, median **0.0%**. Not a guarantee of perceptual balance; source material coloration and alternative sounds preserved.
- Representative 4th-variation native-rate conversion check: **576** files at 96→88.2 kHz without peak overs. All 2,304 files had 4× true-peak screening at source rate.

## Playback DSP: CP05 problem found and repaired in CP08

Native CP05 used cubic resampling for all source/host ratios and independently hard-clamped each channel at ±0.891.
On 163,840-frame 88.2 kHz offline stress renders from the exact CP07 bank:

| Simultaneous hits | CP05 frames touching hard ceiling | CP08 equivalent | CP08 max absolute output |
|---|---:|---:|---:|
| 1 | 0 | 0 | 0.5824 |
| 2 | 98 | 0 | 0.8193 |
| 4 | 110 | 0 | 0.8372 |
| 8 | 148 | 0 | 0.8515 |
| 16 | 1235 | 0 | 0.8671 |

CP08 uses a linked stereo soft-knee ceiling to prevent flat-topped peaks, with knee at 0.70 and asymptote 0.88. This is not a mastering limiter and may shape extremely dense overlapping drum transients, but it avoids the old horizontal hard-clipped waveform. Neutral/non-overloaded single hits remain unchanged at 88.2 kHz.

CP08 also selects a 24-tap, 64-phase Blackman-windowed low-pass sinc interpolator for high-rate source stepping; ordinary neutral 96→88.2k playback retains original cubic interpolation. It uses precomputed coefficients (no trig calls in audio processing).

Controlled 96 kHz synthetic tone test played at 48 kHz host, using otherwise neutral engine settings:
- 15 kHz (in-band) RMS: CP05 0.309706, CP08 0.295551 (**−0.41 dB**).
- 30 kHz (above 48k host Nyquist) RMS: CP05 0.309706, CP08 0.001348 (**−47.2 dB** compared with CP05).
- 45 kHz: CP05 0.309706, CP08 0.0000185 (**−84.5 dB** compared with CP05).

## Native and compatibility validation

- Offline CP08 C-engine, actual CP07 bank: **576/576** kit/pad combinations produce finite audible output at 88,200 Hz.
- Windows x64 CLAP and VST3 PE DLL compilation succeeded. Exports checked (`clap_entry`, `GetPluginFactory`, `InitDll`, `ExitDll`); archive integrity passed.
- **Current approved GUI styling and control scheme are preserved**; the image has only CP07 Vitriolic name updates (mean per-channel pixel difference ~0.11 from CP05 approved preview at 1512×1009). No new native GUI repair made in this checkpoint.
- 24 kits, 24 pads, 4 articulations, 34 existing engine parameters, 136 VST3 controller params, MIDI note mapping 36–59 and host state scheme unchanged. `RubedoVoice` and `RubedoEngine` ABI layout sizes remained identical in compile-time checks.

## Important limitations and workflow

- No real Windows Bitwig scan, plugin-host event, GUI hit-target, or playback test has been performed.
- CP05 input unresponsiveness previously reported by user may still occur; CP08 does not claim to repair the GUI.
- Both plugin release ZIPs embed exact CP07 sound bank, not merely a plugin shell. Prior CP05 install continues to use its original CP02 audio/engine unless replaced.
- No subjective blind listening test or professional mastering certification; hi-fi status here means verified PCM integrity/headroom, SRC and safer overlap dynamics, not an artistic preference judgment.
- If Bitwig reports clipping at its master, check later gain stages and other devices as well. The Rubedo output guard only limits its own sample output, not the entire signal chain.