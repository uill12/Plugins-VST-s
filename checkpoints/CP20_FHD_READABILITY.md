# Albedo Quintessential Synth — CP20 FHD readability (Windows x64)

**Date:** 2026-11-16  
**Target display:** 15.6-inch, 1920 × 1080; Bitwig Studio native plugin window.  
**Status:** Windows CI verified; Bitwig in-host visual and MIDI testing pending.

## Release

- CI: https://github.com/uill12/Plugins-VST-s/actions/runs/43980982965
- Compiled archive: GitHub Actions artifact `Albedo_CP20_Windows_x64_CLAP_VST3`.
- Install-only CLAP/VST3 archive: artifact `Albedo_CP20_Windows_x64_RuntimeOnly`.
- Verified Drive runtime: https://drive.google.com/file/d/1jF4GZiBoAf6g-fDF8RG56dIqbTMThnJD/view
- Previous CP19R5 checkpoint retained in Drive rollback folder.

## User-visible changes

- Native first-open plugin editor size **1265 × 900**, replacing **900 × 640**.
- Rescaling-independent GDI dynamic text on the final native window, with minimum readability sizes of approximately 11–14 pixels; no new font family or replacement of original Albedo artwork.
- Secondary instrument/knob labels get larger footprints and abbreviated role labels when the layout is tight.
- Preset browser native typography increased to **17px main / 14px small**, retaining the Name × Family × Instrument Role organization.
- Preserves the original horizontal macro/character/refinement modules, four source voices, symbolic coupling, hi-fi sound resources, and partial GUI invalidation on parameter change.
- No DSP, factory bank, MIDI voice allocation, CLAP parameter IDs, or VST3 processor changes.

## Verification

- Windows x64 CLAP compiled; VST3 compiled against official Steinberg SDK.
- **Steinberg validator: 47/47 pass**.
- **CTest: 27/27 pass**, including sustained eight-note chord and note-off regressions, native viewport hit regions, 64-preset organization, and partial redraw.
- Runtime-only GitHub Action ZIP SHA-256: `3096cba5416090838cdfc13e5d590f5a6ca149295551558d0f2e81bd5149fd8a`.
- Both generated binaries are unchanged from the CI runtime artifact when packaged locally with CP20 GUI release metadata; binary-integrity hashes are recorded in the ZIP.
- **Host caveat:** Bitwig scan, parameter interactions at user DPI scaling, in-host legibility, and project reopen have not yet been personally tested. Original small labels baked into the underlying 1920 × 1366 background bitmap may still need a separate artwork typography revision.
- **Version caveat:** the native binary's embedded version string remains `1.0.6-cp19r5` because CP20 is a GUI-only checkpoint based on the CP19R5 host/DSP code; the externally distributed package is identified as CP20.

This CP20 checkout is a development branch. Do not overwrite source or working CP19R5 backups before Bitwig validation.
