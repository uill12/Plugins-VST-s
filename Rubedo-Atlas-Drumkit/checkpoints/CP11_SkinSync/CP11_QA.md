# Rubedo Atlas Drumkit CP11 — SkinSync GUI Regression Fix

**Version:** 0.9.6-cp11 · Windows x64 CLAP · 2026-11-15.

## Reported regression

CP10 pad and MIDI overlays obscured the approved Rubedo art. Recreated cards were flat Windows rectangles and their note text used the default GDI font. Older opaque info labels and hover value rectangles looked like foreign colored highlights.

## Changes

- Retained the approved 1512×1009 background outside the active/dynamic text regions and the original selected pad card. Core pad 02–16 art, embossing, labels, and shading remain pixel-identical to CP10.
- Replaced the permanently baked selection of Core pad 01 with the exact-dimension nonselected embossed blank card; active selection now paints an embossed **selected card template** copied from the original artwork with text removed.
- Core: retain the approved pad cards and names instead of redrawing every card as a generic rectangle. Only the dynamically selected card is reconstructed with matching textures and typography.
- Extended: native card sprites matching the same original embossing; pads 17–24 are visible and eight disabled slots have no misleading labels.
- Dynamic assigned MIDI numbers shown subtly at the lower right of pads, styled in warm gold Georgia; all 24 numbers are read directly from the same `engine->mappings[]` table as the MIDI input resolver.
- Dynamic instrument/kit/note readouts use the existing parchment regions and an explicit Georgia/Arial font hierarchy; they no longer use opaque Windows TextOutA rectangles. The right-side voice title follows the selected pad.
- MIDI Learn/unmapped-note status appears in the existing selected-instrument heading line, not as an unrelated floating label or panel. Dynamic changes do not occlude the Learn button.
- Removed the generic VALUE XX% hover popup. Knob adjustment remains preserved; focus is shown with a fine ring.
- Added 7 per-editor Win32 font handles, cached with resize-aware heights and destroyed on GUI detach. Original GUI, MIDI, automation, and audio parameter contracts retained.
- Unique native window class CP11; CLAP version 0.9.6-cp11; unchanged plugin ID `com.rubedo.atlas-drumkit`.

## Checked

- Original CP10 sound bank: **373,714,116 bytes identical** in CP11, SHA-256 `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454`.
- Rubedo PCM bank and 24 kits, 24 pads, four articulations per voice unchanged.
- Source code of `rubedo_engine.c` and `rubedo_engine.h` matches CP10 by SHA-256.
- CP10 MIDI QA executable: **PASS** (default MIDI 36–59, Core/Extended, Learn, swaps, unmapped notes, 88.2-kHz render).
- CP10 CLAP state executable: **PASS** (state dirty, note mapping save/load and legacy duplicate repair).
- Windows x64 GUI compiled with Clang/MSVC ABI and linked via LLD; PE entry point nonzero and `clap_entry` export present; `CreateFontA` import verified.
- Full 7-Zip independent extraction and binary SHA-256 verification: see release manifest after packaging.
- Static preview Core/Extended generated from exact CP11 embedded art and equivalent fonts; not a real Windows screenshot.

## Known limitations

**This checkpoint has NOT been executed inside Windows Bitwig.** Native font metrics, GDI compositing and host DPI behavior need in-host confirmation. CP08R2 is the last user-confirmed working rendering checkpoint, while CP10 preserves the later mapping fixes. CP11 does not include VST3; no VST3 compatibility claim. The right-side knob numeric labels are still from the reference raster and may not reflect automation; unlike CP10, CP11 no longer displays a generic hover-value popup. A later design-locked control-value phase should synchronize those values rather than layering new text over static labels.