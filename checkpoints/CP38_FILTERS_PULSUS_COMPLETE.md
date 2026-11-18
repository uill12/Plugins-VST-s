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
