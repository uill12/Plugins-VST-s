# Rubedo Atlas Drumkit — CP12R1 VST3 ABI-Fix HostQA (2026-11-17)

**Status: compiled full-resource Windows x64 VST3 candidate, independently extracted and checked; NOT certified inside Bitwig.**

## Downloads and sources

- **Full installable-candidate VST3 (13 multipart 7z volumes):** [Google Drive — CP12R1_VST3_ABIFix_HostQA_2026-11-17](https://drive.google.com/drive/folders/1LIk9LjSZHe7w-NPSZSAMq1Zmb63yPgoo)
- **Windows x64 CI:** [run 44028606392](https://github.com/uill12/Plugins-VST-s/actions/runs/44028606392)
- **Preserved CLAP CP12R1 rollback:** [Google Drive](https://drive.google.com/drive/folders/1IwAehMK6LZMmoJFXn8o5Dd0LYkIHMfmw)
- The full detailed per-volume SHA-256 list and full binary manifest are inside the new Drive folder.

## ABI issue fixed

The initial full-resource VST3 HostQA package used an older 239,616-byte VST3 core. Official Steinberg VST3 validator found one failed test: missing mandatory `IProcessContextRequirements` (46 tests passed, 1 failed). We corrected the processor interface, vtable dispatch, and regression coverage in three source commits:

- `e0d0681dac964951fbd26762177c5898986c22ac` — mandatory interface, COM-style queryInterface/refcount lifecycle, correct object address.
- `8719c8375af89620f4d1203f0109cf9f11869153` — query/roundtrip regression.
- `572f5252676221118319c6ce47838ba3a3318fb2` — official Steinberg ABI offset assertion.

The completed Windows x64 CI run **44028606392** then passed both source/check jobs, and the genuine Steinberg validator on a **synthetic-bank diagnostic VST3** reported **47 tests passed, 0 tests failed**. This validator result is not a real-bank or Bitwig host test.

## Full-resource binary assembly, checksum audit

The core produced by the fixed CI run was combined locally with the exact original CP12R1 VST3 candidate's embedded bitmap and 2,304-region PCM24 bank. This assembly preserved `RBOVR103` and rewrote its absolute bank offset for the new core length. The generated native 7z archive was independently extracted using libarchive; the extracted binary's SHA-256 matched exactly. Each .7z.001–.013 volume was SHA-256 checked locally before upload and all 16 Drive items were read back.

| Component | Bytes | SHA-256 |
|---|---:|---|
| Fixed CI core | 240,128 | `371af76e37628e1028fad33491915e06fd6d958679338464ed58a38d2269acd5` |
| Original SkinSync bitmap | 4,576,824 | `a6db03fc0c28dcd269bc0750e2fddac1d03e5faa8d8055c834dc7f1b077df9bb` |
| Original 2,304-region PCM bank | 373,714,116 | `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454` |
| Corrected full VST3 PE binary | 378,531,100 | `dc377fd77dd4c27e103462ef5ca9eb585f62f89d5049cf73c7c569f21ad14123` |
| Native 7-Zip package | 378,533,645 | `6d6f2e9847e0736c7b6993b7a099d6ba8d4ecb46f29e8df3162c4cfcf85b9992` |

**Distribution:** 13 volumes with part names `Rubedo_CP12R1_VST3_ABIFix_HostQA.7z.001`–`.013`, plus `VOLUMES_SHA256.json`, `QA_MANIFEST.json`, and `LEIA_PRIMEIRO.txt`. Download **all** into one folder; open only `.7z.001` in 7-Zip. Install the extracted **folder** `Rubedo Atlas Drumkit.vst3` into `C:\\Program Files\\Common Files\\VST3\\` only after backing up earlier candidate and closing Bitwig. Do not overwrite the CLAP version.

## Outstanding host gates (blocking final release)

- Bitwig Windows x64 plugin scan, unload, multi-instance lifecycle and GUI render.
- All 24 MIDI pads 36–59, note swaps, MIDI Learn activity feedback, `.rbmap` Save/Load, automation and project restore.
- 44.1/48/88.2/96 kHz real-bank audio playback and CPU/voice stability.
- Genuine Steinberg validator (and pluginval if available) **on this full original-bank binary**.

**This is a complete-resource HostQA candidate, not a certified production release.** Do not infer Bitwig success from synthetic-bank validator tests.
