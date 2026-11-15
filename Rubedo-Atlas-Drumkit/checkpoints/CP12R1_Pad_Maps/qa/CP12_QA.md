# Rubedo Atlas Drumkit CP12 — Portable Pad Map Presets

**Version:** 0.9.7-cp12r1 · Windows x64 CLAP · 2026-11-15.

## User request

Save current MIDI note assignments independently from Bitwig session state, and recall those assignments in other projects. Preserve the confirmed CP11 SkinSync GUI rendering and CP10 mapping behavior.

## Implemented

- Two native **SAVE MAP** / **LOAD MAP** controls, beside Core 16 / Extended 8, using parchment-embossed button art derived from CP11. The old misleading "MIDI CONNECT" artwork was replaced.
- Windows common Save/Open file dialogs launched only on the GUI thread; `.rbmap` default extension is appended explicitly for basename-only Save As entries (Win32 classic dialogs do not reliably append a five-character extension). Existing files are protected by an overwrite confirmation.
- Portable, versioned **text** file format: `RUBEDO_PADMAP_V1` then 24 ordered `PAD01=036` … `PAD24=059` entries. Examples included.
- Strict transactional load: validate all 24 pad indices, valid MIDI notes 0–127, and uniqueness *before* updating any live mapping. Supports UTF-8 BOM or CRLF when edited in a text editor. Reject corrupted, duplicate and incomplete files.
- Save writes to a sibling `.tmp`, flushes, and atomically renames/replaces target on the same volume; failed writes remove the temporary file.
- Mapping file contains **only** the 24 MIDI assignments, not the kit/audio presets or DSP settings. CLAP plugin state still saves the current mapping automatically with the Bitwig project, preserving prior v1/v2/v3 load compatibility.
- On successful load, cancel MIDI Learn, update the canonical atomic note map, request a host main-thread callback and state-dirty notification, and refresh the same MIDI labels/trigger mapping.
- Success/failure feedback stays in the existing selected-instrument parchment heading. No new floating overlays.
- Existing CP11 UI bitmap outside the two small button regions, 2,304-hit sound bank, 96 kHz PCM, DSP and canonical mapping semantics are unchanged.

## Offline validation

- Windows x64 CLAP code compiled with clang and lld-link. PE64 DLL entrypoint nonzero; `clap_entry` exported; `comdlg32.dll` imports include `GetSaveFileNameA`, `GetOpenFileNameA`, and `CommDlgExtendedError`.
- Map codec test: default 24-note roundtrip, custom MIDI Learn assignments, swaps, text re-import, invalid/duplicate/out-of-range rejection, BOM/CRLF acceptance and DSP-state preservation: **PASS**.
- CP08/CP11 PCM bank: 373,714,116 bytes retained; SHA-256 `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454`.
- Art derived from CP11 `art_skin_core.png`, with only the Save/Load button area inpainted/replaced; no UI redesign.
- Full native 7-Zip validation and plugin hash recorded in release manifest after packaging.

## Validation still required in Bitwig Windows

1. Load the CP12 CLAP plugin and check Core/Extended pad rendering.
2. Learn a custom MIDI note for pad 21; use **SAVE MAP** to create a file.
3. Learn a different note for pad 21; **LOAD MAP** from that file and verify pad 21's display, note triggering and sound are restored.
4. Save/close/reopen the Bitwig project and confirm map persistence; use Load Map in a second project and confirm portability.
5. Cancel a file dialog; load an invalid file and ensure mappings do not change; save to a read-only folder and check error status.
6. Verify no lost note events, stuck mouse capture or GUI painting regressions after file dialogs. Check simultaneous MIDI notes during/after map load.

**Important:** this checkpoint is compiled and offline-tested, not Windows-in-host certified. It is CLAP only; VST3 source has not been updated for the CP12 controls.