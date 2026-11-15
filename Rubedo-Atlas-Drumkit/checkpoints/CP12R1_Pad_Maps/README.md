# Rubedo Atlas Drumkit — CP12R1 Pad Map Presets

**Version:** `0.9.7-cp12r1` | **Build:** Windows x64 CLAP | **Date:** 2026-11-15.

**Status:** compiled, offline-verified, uploaded to Drive. **Not yet validated inside Windows Bitwig.**

## Feature

The approved CP11 SkinSync GUI now has two embossed **SAVE MAP** / **LOAD MAP** buttons above the pad grid. The misleading static `MIDI CONNECT` label is replaced; MIDI routing is still handled by Bitwig.

- **SAVE MAP** invokes the native Windows Save As dialog and exports exactly 24 MIDI note assignments to a portable, named `.rbmap` text file.
- **LOAD MAP** invokes the Open dialog, validates all 24 note assignments and their uniqueness, and only then replaces the active map.
- **No sound or preset changes.** MIDI maps affect note-to-pad routing and dynamic GUI labels, not kits, sound-bank PCM, timing, envelopes or DSP.
- **Bitwig project save** continues to save the current mapping automatically through the existing CLAP state v3 format, preserving legacy v1/v2/v3 compatibility.
- On a successful load, the GUI cancels MIDI Learn, applies canonical mapping values and requests a host state-dirty notification; invalid files do not change current assignments.
- The final CP12R1 corrects the Win32 classic file-dialog 3-character default-extension limitation so basename-only Save As entries are stored with the actual `.rbmap` suffix.

## Format

```text
RUBEDO_PADMAP_V1
PAD01=036
PAD02=037
...
PAD24=059
```

The included [default mapping](Default_36_59.rbmap) is a working example. Unique MIDI notes 0–127 only; supports CRLF/UTF-8 BOM, strict ordered pad IDs, and rejects malformed files.

## Downloads

**[Google Drive — CP12R1 Pad Map SaveFix](https://drive.google.com/drive/folders/1IwAehMK6LZMmoJFXn8o5Dd0LYkIHMfmw)**

Contains 13 native `.7z.001`–`.013` volumes, `Rubedo_CP12R1_Source_QA.zip` (full source), checksum manifest, installation notes and an example mapping.

Download all 13 volumes into one folder and open only **`.7z.001`** with 7-Zip. Back up the previous working CLAP before replacing `Rubedo_Atlas_Drumkit.clap`.

## QA

See [CP12 QA](qa/CP12_QA.md) and [map codec regression tests](qa/test_map12.c).

- 24 default mappings and custom note assignments round trip; invalid and duplicate note maps are rejected.
- Win64 CLAP compiles/links with nonzero entrypoint and `clap_entry`; common file dialogs are linked through `comdlg32.dll`.
- PCM bank is byte-identical to CP11: 373,714,116 bytes, SHA-256 `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454`.
- Repaired binary SHA-256 `65668f7a244c0e311741d6545b71d603e188ea27eed2e9b57c963da1edf28e1a`; full 7z extraction matches exactly.
- Actual Windows Save/Open dialogs, Bitwig state-dirty behavior and in-host performance require user validation.

This checkpoint **does not** attempt the separate, previously requested note-response/CPU optimization. VST3 has not been updated.
