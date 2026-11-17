# Albedo Quintessential Synth — CP23 native alignment

Date: 2026-11-17. Target display: 15.6-inch FHD 1920 × 1080, Bitwig Studio on Windows x64.

## Screenshot findings
The CP22 renderer used pixel-native minimum font sizes but expanded text rectangles beyond their logical row bounds. On a 1265×900 first-open editor, stacked labels, headers and values could overlap knobs, adjacent modules and the lower secondary headers.

## CP23 changes
- Device-space text is measured, sized and clipped **inside** its assigned cell; Georgia and Segoe UI remain the native font families.
- Source, macro, filter, ADSR and cellular dials now have consistently centered, non-overlapping label columns.
- Two-row labels remain where vertical space allows. The Source Level/Tune/Pan and envelope ADSR groups use one readable inline label/value where two rows would collide.
- Bottom-row titles at logical art Y=959 have a 29-pixel logical band; the cellular and pulse subheadings move to Y=993, leaving space before control Y=1013.
- Title rendering retains the approved Albedo material identity and palette. Ampersands are rendered literally.
- CP22 atomic offscreen repaint, targeted damage regions and graph enlargement are retained.

## Compatibility
- Original bitmap design coordinate system 1920×1366; start window 1265×900, preserving existing Bitwig host behavior.
- Same control IDs, MIDI, 64 factory presets, DSP algorithms, PCM soundbank, CLAP/VST3 identity and audio state.
- The CP22 HTML landscape reference is **not** live HTML/WebView inside the native plugin. CP23 fixes the native screenshot's alignment without claiming complete CSS/native reflow.

## Verification
- Exact source patch SHA-256: `1ee6c0c24b887e655e913be98310c2e6d8c2053eacc3d61e9c2703743e77e8e3`
- Native C++ SHA-256 after CP22 MSVC hotfix: `9ea9c5ac546ce1d8c1863bd9597084c812b183181fe027bccd28cd69d8d29826`
- Local static spacing QA: PASS; patch reverse-application and integrity checks: PASS.
- Windows CI: link from `.github/workflows/albedo-cp23-windows.yml`; distinguish source QA from actual CLAP/VST3 compile/test results.
- Bitwig visual inspection: still required. Keep CP22 rollback until the CP23 runtime is compiled and in-host validated.

## Source
- `patches/cp23_native_alignment.b64` is a base64(gzip(git patch)) applied after CP22 and its Win64 LONG-to-int hotfix.
- Full Windows CI rebuilds source from the original authenticated archive and applies CP14→CP23 patches in order.
