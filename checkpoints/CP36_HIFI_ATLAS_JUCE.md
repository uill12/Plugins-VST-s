# Albedo CP36 — Hi-Fi Mathematics and Extended Preset Atlas

## Scope and source

Parent branch: `albedo-cp35-render-window-match`.
Current target: JUCE 8 + WebView2, Windows x64 CLAP and VST3.
Preserve CP35 verified runtime, original 32-material bank, 64 factory voices and
stable CLAP/VST3 manufacturer/parameter IDs. No FM engine added.

## Audio implementation

Reconstructed CP35 source `vitriol_dsp.cpp` receives deterministic patch
`patches/cp36_hifi_math.py` after the existing source integrity gates.

- True double host path: `processTyped<Sample>` template produces `double`
  directly for `process64`; removes temporary float32 output buffers.
- Four-sample cubic Catmull-Rom sine LUT and chorus/delay interpolation in
  double precision, without modifying the pre-existing Hermite helper.
- Oversampling FIR low-pass decimation raised from 32 to 64 Blackman taps,
  in already preallocated coefficient/history buffers; no new audio-thread
  heap allocation.
- All 4 synthesis sources, routing, DSP identity, parameter values and factory
  presets unchanged. At high sample rates the automatic 1x factor still means
  a separate anti-alias comparison remains to be done.

## Objective local evidence

Linux GCC C++ offline test: 4 sample rates (44.1/48/88.2/96k) × 3 synthesis
methods finite output, sustained signal and bounded peaks. The 64-bit result
retains nonzero sub-float32 residual and matches the 32-bit path within
roundoff at normal values. Held 12-note double-buffer test finite.

4096-LUT numerical interpolation max absolute sine error:
linear ≈ 2.94e-7 vs cubic ≈ 5.8e-11, an algorithmic accuracy metric.
FIR stopband at 0.56/os: 2× (32 taps ≈ -35dB / 64 taps ≈ -78dB);
4× (32 ≈ -16dB / 64 ≈ -36dB). These are **specific normalized sample-rate
frequency points**, not a broadband alias/noise-floor certification.

Indicative Linux CPU benchmark at 48 kHz, all sources enabled, 12 keys:
the new algorithm required roughly 20% more compute than CP35.
Results depend on toolchain and processor; do not assume real-time headroom
in Bitwig without host measurements.

Windows tests: CMake `AlbedoCP36HifiDspTest` and source-contract
`check_cp36_quality.py`, plus existing CLAP/VST3 runtime export QA.

## Atlas and GUI

The long-form factory Atlas is viewport centered and expands up to
1200×795 within a 1320×880 editor. Internal scroll list shows approximately
12 rows on that viewport (10 at 1152×768). Preset row titles/family/role
font sizes increased to 14px/12px. Character, movement and musical-context
**dropdowns are removed**; the metadata remain intact for all 64 voices
but are not shown as filters. Search, Family, Instrument Role, Sort, 8×8 matrix,
factory loading, Arbor animation, parameter bridge and JUCE resize unchanged.

Chromium browser test: 1320×880, 1152×768 and 900×600 with no browser errors,
no clipped panel, 64 factory entries, 8 families × 8 roles; includes
`Albedo_CP36_Atlas_1320x880.png` preview.

## RGNES governance

`standards/RGNES_JUCE8_STANDARD_CP36.md` records JUCE 8 as the preferred
framework for new/rebuilt plugins, choosing native JUCE or authorized WebView2
per project. The private RGNES plugin was separately updated to v2.3.0,
including a reusable JUCE 8 DAW skill; this does **not** mass-rebuild other
projects or certify their current plugin GUI.

## Remaining gates

The latest Windows CP36 workflow must complete; check
`actions/runs/44245872403` (or later branch run), retrieve binaries,
verify ZIP integrity, SHA/exports, and run Bitwig CLAP/VST3 at 48/88.2/96k
with 12 voices. Audio 'Hi-Fi' remains a goal until measurable anti-alias,
harmonic, gain and CPU benchmarks plus listening are approved. If Windows
build fails, preserve CP35 runtime rather than promoting this source-only
checkpoint.

## Verified CP36 R1 distribution

- Windows Actions run `44245872403` finished successfully.
- CTest: **2/2** (`cp32_factory_atlas_64_complete`,
  `cp36_hifi_double_precision_polyphony`).
- PE32+ AMD64 exports: `clap_entry` and `GetPluginFactory`.
- Embedded original PCM bank: found in both audio binaries.
- Release ZIP: `Albedo_CP36_R1_HiFi_LongAtlas_Windows_x64_CLAP_VST3.zip`.
- R1 is **packaging-only** compared with the CI binary: its VST3
  `Contents/Resources/moduleinfo.json` trailing commas were normalized to
  strict JSON; CLAP/VST3 PE binary SHA-256 hashes are unchanged. Reproducible
  helper: `standards/tools/rgnes_vst3_moduleinfo_strict_json.py`.
- R1 Google Drive runtime: https://drive.google.com/file/d/1-CcARggTBSLYu-81mdwHheRJZCabxceK/view
- SHA-256 for both binaries and ZIP: delivered in
  `Albedo_CP36_R1_Runtime_SHA256.json`; independent ZIP CRC succeeded.
- Installed private RGNES Full DAW Engineer plugin version: **2.3.0**,
  including the new `juce8-native-daw` skill. Source standard:
  `standards/RGNES_JUCE8_STANDARD_CP36.md`.
- **Bitwig host audio/GUI confirmation remains pending**. CP35 retained.
