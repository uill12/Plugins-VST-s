# Rubedo Atlas Drumkit — CP12R3 CLAP Preset Sync

**Windows x64 CLAP · v0.9.8-cp12r3 · 2026-11-18**

**Status:** corrected code, Windows loader smoke PASS and Linux CLAP host-feedback/24-kit PCM tests PASS. The full archive is independently extracted and checksummed. **Bitwig Windows GUI interaction remains to be confirmed by the user.**

## Download

**[CP12R3 Google Drive — 13-part installable CLAP](https://drive.google.com/drive/folders/1h-vMi0_MartSxMXvMoPKKZ_N1XYV7Odg)**

The folder has `Rubedo_CP12R3_CLAP_PresetSync_Windows_x64.7z.001` through `.013`, `VOLUMES_SHA256.json`, `LEIA_PRIMEIRO.txt` and the complete source+QA ZIP. Source changes are committed in `source/` under this checkpoint.

## Confirmed defects in prior builds

1. The lower-left preset dropdown hitbox `x=38..170, y=739..775` in `rubedo_gui.c` **incorrectly highlighted a different pad** instead of selecting a kit. It was a nonfunctional visual control.
2. The `gui_parent` CLAP integration had called `rb_gui_attach(...,x,0)` and **left the kit callback unconnected**. Param feedback for the 24-kit selector wasn't delivered to the Bitwig host.
3. Earlier GUI handlers used `kit/23.0`, although the host param `KIT / ATLAS` expects **plain stepped values 0..23**.
4. `par_flush` used to return immediately on a null input queue, dropping pending GUI→host events.
5. Host-side CLAP `clap.params` `request_flush` wasn't implemented for GUI changes, and no parameter event was emitted through `process`/`flush`.

The original CLAP error 127 `MessageBoxA` imported from `kernel32.dll` is also kept fixed: CI verifies imports under `USER32.dll`.

## CP12R3 fix

- Native Windows popup of **all 24 kits** from the lower-left selector, the displayed kit name, and `ALL 24`.
- Four family buttons show only their **six matching kits** (ETHEREAL, VITRIOLIC, SCIENTIFIC, TRANSMUTATION).
- Prev/next arrows, keyboard arrow keys and the mouse wheel over the kit selectors.
- Plain-value `KIT / ATLAS` host parameter event, sent with `CLAP_EVENT_PARAM_VALUE` in `process` or `params.flush`; host `request_flush` and main-thread `mark_dirty`. Lock-free 34-param pending mask, with retry when output queue rejects an event.
- Existing `CLAP state v3` including pad mappings and per-pad controls is retained. External host automation/state updates refresh the kit label on the existing GUI timer.
- **No DSP redesign, no bank regeneration, no new GUI bitmap.** The 96 kHz PCM24 bank (2,304 regions) and SkinSync bitmap are **byte-identical**.

The kits remain one plugin's `KIT / ATLAS` parameter; they are **not separate preset files exposed through the optional CLAP preset-discovery factory**. Adding Bitwig native preset browsing requires that separate extension and is not claimed here.

## QA

[GitHub Actions — run 44425095726](https://github.com/uill12/Plugins-VST-s/actions/runs/44425095726) **success**:

- Windows 2022 x64: source core compiles; `LoadLibraryW`, `GetProcAddress("clap_entry")`, `FreeLibrary` pass; `MessageBoxA` and `TrackPopupMenu` are imported from `USER32.dll`.
- Linux: complete CLAP factory and host parameter mock test; 34-param queue; plain kit number, queue backpressure/retry, host flush callback, incoming automation and backwards-compatible v3 state save/load.
- Local test using the original, unaltered PCM24 bank: 24/24 kits render nonzero and individually different waveform signatures for MIDI note 36.
- Binary uses the **exact source-only core** output by that Windows CI run, with the original bitmap, PCM bank and corrected `RBOVR103` footer.
- Independent 7-Zip extraction returns exactly the packaged full CLAP's SHA256.
- All 13 volumes were uploaded to Drive and total size independently enumerated.

| Artifact | Bytes | SHA-256 |
|---|---:|---|
| CI-tested Windows x64 CLAP core | 226,304 | `1d4de0a970d7ed1968d13c7ead9bf74ce983a0232658b2a86f2ab2856b3f4667` |
| **Full Rubedo_Atlas_Drumkit.clap** | 378,517,276 | `34a7ae92127a058bc44d4a5fcc8836516815afcec0e03d701ef45f0f6652ed9e` |
| Native 7-Zip archive | 378,517,410 | `e0b205fbc293cd610f419ee7e428b431b4f527e92602f843c09af5f83389a9e7` |
| PCM bank, unchanged | 373,714,116 | `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454` |
| Original GUI bitmap, unchanged | 4,576,824 | `a6db03fc0c28dcd269bc0750e2fddac1d03e5faa8d8055c834dc7f1b077df9bb` |

## Install / rollback

1. Exit Bitwig; back up `C:\Program Files\Common Files\CLAP\Rubedo_Atlas_Drumkit.clap`.
2. Download all 13 volumes in the same directory; open **`.7z.001` only** in 7-Zip and extract the full `Rubedo_Atlas_Drumkit.clap`.
3. Copy that CLAP to the previous location, restart Bitwig and rescan.
4. Test lower-left preset chooser, family filters, arrows and `KIT / ATLAS` automation; trigger the same MIDI note after each change (previously playing notes can retain the old kit until retriggered).
5. Reopen the Bitwig project and verify that the selected kit is restored; test 88.2 kHz, pad MIDI mapping, SAVE/LOAD MAP and playback stability.

**Rollback:** [CP12R2 Windows loader fix](https://drive.google.com/drive/folders/18ns_0iampAAsHGGhFGfz-RfC28NEW1x1). Keep the previous verified installation until Bitwig QA is complete; neither archive was overwritten.

**Remaining host QA:** the actual Windows Bitwig UI popup selection/clickability, state/automation under real transport, multi-instance behavior, audio at 44.1/48/88.2/96k and performance have not been tested in a live Bitwig host.
