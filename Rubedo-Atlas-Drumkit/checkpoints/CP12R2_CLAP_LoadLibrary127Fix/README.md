# Rubedo Atlas Drumkit — CP12R2 CLAP Windows LoadLibrary 127 fix

**Version:** `0.9.7-cp12r2` · Windows x64 CLAP · 2026-11-18  
**Status:** full-resource binary + Windows source-only core `LoadLibrary` smoke **PASS**; real Bitwig hosting remains unverified.

## Current recommended download

**[CP12R2 CLAP WindowsLoadLibrarySmoke — Google Drive](https://drive.google.com/drive/folders/18ns_0iampAAsHGGhFGfz-RfC28NEW1x1)**

Download all 13 split 7-Zip files named `Rubedo_CP12R2_CLAP_WindowsVerified.7z.001` through `.013` to one folder, open only `.001` in 7-Zip and extract `Rubedo_Atlas_Drumkit.clap`. Verify checksums against the accompanying `VOLUMES_SHA256.json` and `QA_CI_Windows_LoadLibrary.json`.

**Preserved rollback:** [CP12R1 Pad Map SaveFix CLAP](https://drive.google.com/drive/folders/1IwAehMK6LZMmoJFXn8o5Dd0LYkIHMfmw). The initial locally rebuilt CP12R2 candidate was superseded by this package based on the **exact GitHub Windows-smoke-tested core**. Neither predecessor was overwritten.

## Root cause and fix

In the originally distributed `0.9.7-cp12r1` CLAP, the PE import table assigned `MessageBoxA` to **`kernel32.dll`** instead of its actual provider **`user32.dll`**. Windows therefore returns `ERROR_PROC_NOT_FOUND (127)` when Bitwig calls `LoadLibrary`, even though the archive, embedded 2,304-hit audio bank and GUI assets pass file checksums.

The CP12R2 build uses explicit correct Win32 import libraries, preserving the original C DSP, 96 kHz 24-bit PCM, 1512×1009 SkinSync bitmap, MIDI map/save APIs, fixed CLAP ID `com.rubedo.atlas-drumkit`, and state compatibility. Only the binary core and metadata version were recompiled; no sonic redesign or remapping.

## Windows smoke gate

The [Windows x64 GitHub Actions run 44409138760](https://github.com/uill12/Plugins-VST-s/actions/runs/44409138760) completed successfully. It generated explicit import libraries with `MessageBoxA` in `USER32.dll`, compiled the CP12 engine/editor into a PE32+ CLAP core, and executed on Windows:

- `LoadLibraryW(core)` — PASS, no 127.
- `GetProcAddress(core, "clap_entry")` — PASS.
- `FreeLibrary(core)` — PASS.
- Import table assertion: `MessageBoxA` must exist under `USER32.dll` and must not exist under `KERNEL32.dll` — PASS.

The full binary in Drive was assembled using this **exact core SHA-256**, verified byte-for-byte:

| Artifact | Bytes | SHA-256 |
|---|---:|---|
| Windows smoke-tested core (no bank) | 224,768 | `646984f0bfa3ff5f31e7f9bd5ac9cedc76cf651ee298b542488ccfe8838ce58c` |
| Full CP12R2 CLAP with verified original resources | 378,515,740 | `a1a27a1edaff750c2a83f233255455f29a95827095914ff388d700e9067ac1f0` |
| Native 7-Zip distribution | 378,518,747 | `edc2686ce89985c3e35baf633d770c3e5d38c146c8a73bb8b782913ed7db5ea5` |
| Original 2,304-hit PCM24 bank retained | 373,714,116 | `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454` |
| Original 1512×1009 GUI bitmap retained | 4,576,824 | `a6db03fc0c28dcd269bc0750e2fddac1d03e5faa8d8055c834dc7f1b077df9bb` |

The reconstructed full binary was independently extracted from 7-Zip and the final SHA-256 matched. All 13 volume records plus 3 sidecar documents were verified in Drive.

**Scope of test:** Windows `LoadLibrary` was executed on the core without appended real PCM data. The packaged full binary uses precisely that tested core with resources appended and the original resource footer offset updated. Real Bitwig plugin discovery, GUI, 88.2 kHz audio, multi-instance lifecycle, state and save-map interaction remain to be tested in the user's Windows host.

## Install

1. Exit Bitwig completely and back up `C:\Program Files\Common Files\CLAP\Rubedo_Atlas_Drumkit.clap`.
2. Extract the 13-part CP12R2 archive, then copy the resulting `Rubedo_Atlas_Drumkit.clap` to the same CLAP directory (administrator rights might be required).
3. Optionally run [`VERIFY_WINDOWS_CLAP_CP12R2.ps1`](VERIFY_WINDOWS_CLAP_CP12R2.ps1) to compare the installed hash and perform an independent loader test.
4. Restart Bitwig and rescan CLAP plugins. Test pad triggers 36–59, MIDI Learn, `.rbmap` Save/Load, 44.1/48/88.2/96 kHz playback, sample switching and GUI response.

Do not use the VST3 file as a substitute for a `.clap` in the CLAP directory. The VST3 is a separate installation in `Common Files\VST3`.
