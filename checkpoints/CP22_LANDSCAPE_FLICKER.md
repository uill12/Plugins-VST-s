# Albedo Quintessential Synth — CP22 landscape GUI and native repaint
Date: 2026-11-17 | Windows x64 | 15.6-inch FHD 1920 × 1080

## Scope

CP22 has two distinct, deliberately separated deliverables.

1. **Interactive HTML layout reference:** landscape redistribution of module cards and their controls, enlarged synthesis network, spectra, envelopes, filter responses and cellular/LFO graphics. Layout tested at 1265, 1540 and 1680 CSS pixels with zero JavaScript exceptions, zero passive-pointer DOM mutations, and graph zoom open/close checks passing.
2. **Compiled native CLAP/VST3 candidate:** based on the fully verified CP21 sources. Reuses the original bitmap artwork, module positions, fonts and host controls; increases graphs within existing native panels, fixes dirty-rect coverage of the filter graph, and draws clipped dirty regions into a reusable offscreen GDI buffer before one-window BitBlt. Pointer motion without dragging no longer requests explicit invalidation. This binary does **not** yet implement the HTML grid's module reflow.

## Build verification

- Branch: `albedo-cp22-landscape-flicker`
- GitHub Actions: https://github.com/uill12/Plugins-VST-s/actions/runs/44034635510
- Final workflow: **success**
- CTest: **27 / 27 passed** (includes CP19 partial repaint test updated for passive mouse and resize)
- Steinberg VST3 validator: **47 / 47 passed**
- PE export checks: `clap_entry` for CLAP and `GetPluginFactory` for VST3
- Runtime artifact: `Albedo_CP22_Windows_x64_RuntimeOnly`
- Artifact ZIP checksum (SHA-256): `4ecb7ca0b85f4d1a43f6c39bf4b789ec4d4ae4d91dc0b9cc2aa54f8a11422068`
- Runtime ZIP manifest: 6 individual files had matching SHA-256 values; ZIP CRC passed.
- Internal release metadata still reports **CP19R5 / 1.0.6-cp19r5** because it is inherited from the native build baseline. The CP22 label represents the tested source checkpoint, not a new plug-in ID or version string.

## Distribution and rollback

- New isolated Google Drive candidate folder: https://drive.google.com/drive/folders/1owwcelc3skFyI5UYb5aM7LKRlXOUHF77
- Installable runtime ZIP on Drive: https://drive.google.com/file/d/1C8pNffSWussUrjJ6nN_O22EY4tuaKUFt/view?usp=drivesdk
- Preserve prior CP21/CP20 releases unchanged.

## Limits and next native work

The native test proves offline Windows build integrity and requested paint-region behavior, **not** visual flicker elimination inside Bitwig under real host parameter traffic. That requires on-device GUI/automation regression testing at native and changed DPI scales.

The native plugin still uses a fixed underlying bitmap layout. To fully migrate the **HTML's horizontal module repositioning**, the original theme art, control hit rectangles, dirty maps and module placement need to be rebuilt together; do not treat HTML as embedded WebView source or claim identical native layout yet.

No DSP, note scheduling, sample-rate logic, CLAP/VST3 MIDI, or factory preset parameters changed in CP22.
