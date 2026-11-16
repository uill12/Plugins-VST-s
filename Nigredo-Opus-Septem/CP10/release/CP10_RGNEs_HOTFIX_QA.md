# Nigredo Opus Septem — CP10 native editor repaint / overlay hotfix

**Build:** 0.9.1-cp10 · 2026-11-15 · Windows x64 Audio FX & MIDI Note FX / CLAP & VST3.

**Evidence tier:** **Cross-compiled + offline-tested**; **not tested in Windows Bitwig**. User previously reported CP09 overlapping overlays and a visible full-screen refresh on each action. CP09 is retained as the untouched rollback checkpoint.

## Root cause in CP09

- `pvEdit()` issued `InvalidateRect(hwnd,nullptr,0)` on every control update; the 100-ms parameter timer also invalidated the entire editor. Every `WM_PAINT` decoded/scaled and repainted the high-resolution manuscript PNG directly to the visible HDC before drawing all GDI controls. In a host this creates unnecessary full-window repaints and can visibly flash.
- Native ON/OFF and formula labels were painted on top of already baked text or opaque GDI rectangles. The static-scene/native-control separation was incomplete.
- MIDI master knobs were miscategorized as linear faders in the generated native control atlas (two drawn lines over rotary controls).

## CP10 changes (visual design lock preserved)

1. **Cached base scene:** GDI+ renders the selected 1600×1000 illustration to an offscreen compatible bitmap only when the selected stage changes or the window is resized.
2. **Atomic foreground buffer:** live knobs, values, ON/OFF and order are composited into a second memory DC; the completed frame is shown by `BitBlt`, rather than partially painting onto the host's visible HDC.
3. **Region-based invalidation:** edits, host automation, stage enables, power, stage ordering and preset name invalidate bounded design-space rectangles projected through the same 16:10 transform used by painting and hit testing. Selecting a different stage or resizing still legitimately updates the full image.
4. **No conflicting baked text:** static PNGs suppress mutable `#stagePower` text, `#presetSelect` text and the top power thumb in addition to the existing numeric and per-stage ON/OFF masks. The native layer draws the single authoritative state value. Removed opaque fills from native choice/switch/preset overlays.
5. **Control-type correction:** MIDI master controls 1 and 2 render as rotary rather than fader indicators. Boolean threshold uses its actual parameter range.
6. **Compatibility:** stable plugin IDs, 82 parameter IDs per plugin, host state version (5), DSP, MIDI event processing and 16:10 layout are not changed. No WebView2.

## Offline checks completed

- Four Windows x64 binaries built from current source with LLVM `clang-cl`/LLD.
- CLAP factory entry/export and VST3 `GetPluginFactory`, Win64 native class ABI and native artwork footer validated statically.
- Each of four binaries has seven loadable 1600×1000 PNG scenes at verified offsets. No imports or embedded references to WebView2/CEF.
- RG NES HTML behavior: seven horizontal stage selections, mode labels, global bypass, each stage toggle and control coordinates pass Chromium test.
- Linux CLAP/VST3 smoke tests and audio/MIDI stress at 44.1, 48, 88.2, 96 and 192 kHz pass (finite output, matched MIDI note on/off); DSP unchanged from CP09.
- **Not validated:** Windows WM_PAINT frequency/latency, actual flicker elimination in Bitwig, mouse input under host scaling, automation state on simultaneous instances. This needs a user-side or host-side run.

## Required Windows Bitwig regression checks

Open CLAP Audio and MIDI individually, then VST3; hold/drag knobs continuously; watch the artwork for flashes and the controls for ghosted text; use wheel/fader/choices; switch seven stages (full illustration change is expected); toggle every stage and FX Chain; load presets repeatedly; test host automation/undo; resize to 1120×700 and 1600×1000; close/reopen editor; load two instances and verify independent state; save/reopen project and verify parameter values.

**Release acceptance:** keep CP10 as an offline-tested fix until its Windows host behavior is confirmed. If blank, flicker or overlay recurs, retain the screenshot and Bitwig version and restore CP09 from its original archive; do not repackage CP09 as CP10.
