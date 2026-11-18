# Albedo CP38 R2 — Corpus A/B symmetry and complete Pulsus amplitude rhythm

Checkpoint date: 2026-11-18. Baseline: CP37, JUCE 8/WebView2 + CP36 HiFi DSP. Native CLAP and VST3 plugin IDs and the 64 factory patches are unchanged.

## Corpus & Character

Both filter modules share the same measured layout: 171px panel with a 32px heading/select row, 45px response graph, 62px row of 3 controls. Cutoff/Reso/Drive are centered in matched columns with 33px *square* interactive knobs and common label/value baselines; no GDI/text overlay.

Independent Chromium screenshots measured A/B graphs and all six dial y-coordinates identical at 1320×880, 1152×768, and 900×600 viewports.

## Pulsus: expose *all 16 steps* and real audio-pulse shaping

The 16 amplitude steps are arranged as **two banks of eight** to avoid individual 10px-wide buttons in the former single 16-column row. Pulsus fits the existing 190px lower panel in one 3:2 viewport:

- 16px module caption, 57px sequencer (2×27px steps), 26px preview/randomize/division row, 25px dedicated host-controlled pulse enable/depth/gate row, 11px explanation and 2px row gaps.
- Added dedicated **Pulse On**, **Depth**, **Gate** controls mapped to native DSP parameter IDs **6020**, **6022**, **6023**, while **Division** controls **6021**. The original 16 step values still map to **6000–6015**.
- Fixed division unit conversion: HTML options 125/250/500 *milliseconds* are emitted to host as 0.125/0.25/0.5 *seconds*. Host automation is decoded into the closest displayed option.
- The ▶ Preview button only animates the step cursor and is no longer mistaken for native audio-pulse enable.
- Optimized `renderSteps()` to keep 16 buttons stable and update their classes/ARIA rather than rebuilding the DOM on every visual beat.
- No MIDI note generation was added: Pulsus is a **16-step audio amplitude modulator, not a MIDI note arpeggiator**. This preserves the previous user-approved DSP and MIDI-polyphony architecture.

## Browser QA

The exact embedded HTML/JS was exported from GitHub Actions branch and tested with Chromium, with JavaScript-to-JUCE host message instrumentation.
* 1320×880: 16/16 steps, 8 per row; panel within viewport; no page errors.
* 1152×768: same, uniformly scaled and fully visible.
* 900×600: same, uniformly scaled and fully visible.
* UI events observed: `param|6020|1`, `param|6022|66`, `param|6023|83`, `param|6021|0.5`, `param|6015|1`. The parameters are real DSP controls.
* Filter A/B response graphs and knob/label/value vertical coordinates match at all three sizes.
* No interference with animated Arbor, Motus/Cellular, 64-preset Atlas or JUCE resize controls.

QA artifacts: `Albedo_CP38_R2_Pulsus_Filters_Interactive_GUI_QA.zip`, `Albedo_CP38_R2_Pulsus_GUI_QA.json` and screenshots. Source contract files: `check_cp38_filter_alignment.py` and `check_cp38_pulsus.py`.

## Release gates and rollback

Windows CI workflow: `.github/workflows/albedo-cp33-webview2-windows.yml`, targeting JUCE VST3 + CLAP, with the 64-factory-preset and HiFi DSP offline regressions. Latest Windows workflow status must be checked separately. Browser QA does *not* certify Windows Bitwig audio/GUI or real host resize. CP36 R1 remains available for recovery; do not overwrite functional builds.

## Windows CP38 R1 compiled distribution (2026-11-18)

- GitHub Actions Windows run **44271084225**: successful, including the source contracts, CTest, CLAP and VST3 binary export checks. Source/plugin version **1.0.38**.
- GitHub native artifact: `Albedo_CP38_JUCE_WebView2_AB_Pulsus_Windows_x64_CLAP_VST3`.
- Downloaded runtime ZIP: CRC verified. Both binary PE headers are x86_64 (0x8664), with `clap_entry` in CLAP and `GetPluginFactory` in VST3. Both binaries embed the new `pulseEnable`, `pulseGate` and `16-STEP AMPLITUDE SEQUENCER` source strings.
- **R1 distribution** only normalizes JUCE's VST3 `moduleinfo.json` to strict JSON, preserving byte-for-byte identical CLAP/VST3 binaries from CI. ZIP: `Albedo_CP38_R1_JUCE_WebView2_Corpus_Pulsus_Windows_x64_CLAP_VST3.zip`.
- R1 ZIP SHA-256: `61aa3abdcbfab326450cfb0a85bec122bc56b37b88ffc0b78bb764f741f6d87b`.
- VST3 binary SHA-256: `e8babe6e7631c8e66de9862eaff8c417db8d4857ea103e1fb303cc2eb5aec1ea`.
- CLAP binary SHA-256: `3571d1316ae017cdb64909cf2618172598583e6e162222503db2fdbb0fd3ad9a`.
- Drive CP38 folder: https://drive.google.com/drive/folders/1ycSj8nrz-a6Hgbeny1RNyB0sBiJhnjXt . Runtime file: https://drive.google.com/file/d/1dWxXlG2IymezmYpXmu_2ylOFVH9f8IVl/view .
- Browser test extra: 16/16 steps and all controls visible at 1320×880, 1152×768, 900×600; 0 JS exceptions; native host message IDs for enable/depth/gate/rate/step confirmed; host automation updates do not replace the step DOM. Filter A and B response plots and six knob/label/value rows match across all three viewports.
- CP37 & CP36 R1 remain preserved as rollback candidates. **Bitwig runtime playback and host GUI validation still pending**; CI and Chromium do not prove 48k polyphonic CPU/headroom or actual desktop WebView2 compositing.
