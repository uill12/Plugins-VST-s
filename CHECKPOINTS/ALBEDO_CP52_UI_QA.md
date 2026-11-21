# Albedo Quintessential Synth — CP52 UI validation checkpoint

**Checkpoint date:** 2026-11-21  
**Source branch:** `albedo/cp52-render-fill-nereida-browser`  
**Base rollback branch:** `albedo/cp51-morph-tree-sound-generation`  
**Status:** Windows Edge HTML/UI validation **PASS**. Full native CLAP/VST3 + Bitwig host validation **PENDING**. Do not overwrite CP51.

## Why this checkpoint exists

User's real Bitwig screenshot showed the old Preset Atlas clipped by the plugin window, with controls off the right edge and no accessible resize while the dialog was open. This prevented practical evaluation of the CP51 192-sound library.

## Implemented

- Native 3:2 editor start geometry reduced to **1080 × 720**, respecting screen working area.
- WebView2 main render fits the actual CSS client viewport. Minor host/DPI aspect disagreements can fill the client; significant deviations remain undistorted.
- Nereida-style browser with collection tabs, **Family** and **Instrument Role** navigation, search, sort, a vertically scrollable list, and stable host preset identifiers.
- **− / FIT / +** host resize commands remain visible in the modal, rather than relying on a corner control concealed by the overlay.
- **AUDITION ON/OFF** permits repeated selection of presets without closing the browser and restores the original close-after-load behavior when disabled.
- CP51 Morph Tree remains an **offline sound-generation process**, not a preset filter or user-facing tree.
- CP51 soundbank, indexed preset programs, DSP and prior releases are preserved.

## Verified evidence

- [Windows Edge fast GUI CI — run 44842834932](https://github.com/uill12/Plugins-VST-s/actions/runs/44842834932): **success**.
- CP51 selected source metadata obtained from the previously successful [CP51 run 44835756139](https://github.com/uill12/Plugins-VST-s/actions/runs/44835756139), preserving the actual 192-preset catalog without rerendering audio.
- Static browser source, native resource loading, 192 IDs, and viewport controller passed.
- Playwright in Microsoft Edge confirmed **1080×720, 900×600, 720×480**: no modal escape/cutoff, no horizontal list overflow, vertical scrolling, three 64-preset libraries, Family/Role filters, host resize control dispatch, and continuous audition toggling.
- Reference screenshots in `Albedo_CP52_Edge_GUI_Window_Reference` artifact of that run.

## Remaining gates

1. Finish the Windows native CLAP/VST3 build with the latest source commit and preserve its build SHA.
2. Run compiled plugin host resize/WebView2 client-geometry test, including preset catalog and native events.
3. Validate manually in Bitwig Studio on Windows: actual host size negotiation; no clipped controls; mouse wheel navigation; MIDI notes during switching; no notes stuck; presets 0–191 reachable.
4. Promote source/binaries and upload to the Albedo project release folders only after passing compiled and host QA. Keep CP51 rollback intact.

**Never label CP52 a binary release before compiled artifacts and host QA are verified.**
