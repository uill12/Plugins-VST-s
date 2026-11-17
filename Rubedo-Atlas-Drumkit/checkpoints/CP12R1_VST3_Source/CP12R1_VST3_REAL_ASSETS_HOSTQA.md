# Rubedo Atlas Drumkit — CP12R1 VST3 with exact original resources

> **Superseded as the recommended HostQA build (2026-11-17):** This document preserves evidence for the *original* VST3 candidate assembled from CI run 43975514483. Its old core fails the mandatory Steinberg process-context extension test. The **corrected full-resource VST3**, with the same PCM/GUI assets and a recompiled 47/47-validator-passing core, is available in [CP12R1_VST3_ABIFix_HostQA_2026-11-17](https://drive.google.com/drive/folders/1LIk9LjSZHe7w-NPSZSAMq1Zmb63yPgoo). See [updated QA, hashes and tests](CP12R1_VST3_ABIFIX_HOSTQA.md). Neither candidate is certified in Windows Bitwig yet.\n\n
**Assembly date:** 2026-11-17  
**Status:** REAL-RESOURCE WINDOWS X64 **HOST QA CANDIDATE**, not Bitwig-certified.  
**User-confirmed rollback:** CP12R1 CLAP in existing `00_LATEST — CP12R1_Pad_Map_SaveFix` folder — not changed.

## Download

**[Google Drive — CP12R1 VST3 Candidate HostQA](https://drive.google.com/drive/folders/120_M3KXxH4HyoMFCAp2BOhyn60m-oCeR)**

Contains the full VST3 bundle in 13 native multi-volume 7z files (`.7z.001` through `.7z.013`), `VOLUMES_SHA256.json`, and installation/testing instructions. All 15 items were uploaded and listed back in the Drive checkpoint. Download every part into the same folder and open **only** `.7z.001` using 7-Zip.

Bundle layout:

```
Rubedo Atlas Drumkit.vst3/
└── Contents/
    └── x86_64-win/
        └── Rubedo Atlas Drumkit.vst3
```

## Build provenance

1. **Exact original CLAP:** Retrieved all 13 CP12R1 volume files through the connected Google Drive files interface; verified all part sizes and SHA-256s against `MANIFEST_SHA256.json`, and reconstructed the native 7z file with the exact manifest SHA.
2. **Independent full extraction:** Extracted four archived entries using libarchive; the CLAP executable matched the originally published SHA-256 exactly.
3. **VST3 core:** Windows x64 compiled `Rubedo_CP12R1_VST3_CORE_ONLY.vst3` from [GitHub Actions run 43975514483](https://github.com/uill12/Plugins-VST-s/actions/runs/43975514483), source revision `25bf2a3042d69494f03c07ef4b7adaf008adbf0f`. That CI run passed the Steinberg C API ABI static checks, mock-host MIDI Learn/feedback tests, synthetic PCM wrapper audio parity at 44.1/48/88.2/96k and independent Windows synthetic-resource DLL initialization.
4. **ChatGPT local assembly:** Recovered the original 1512×1009 24-bit GUI raster and original 2,304-hit 96kHz PCM24 region from the verified CLAP's `RBOVR103` overlay, appended them to the actual Windows VST3 PE, and rewrote its absolute footer bank offset.
5. **Resource verification:** Compared the entire new VST3 image and PCM bank **byte for byte** with the released CLAP, re-read and validated the new `RBOVR103` footer and `RBDK24` bank signature. The whole bank hash matches the original CP12R1 manifest.
6. **Packaging:** Used libarchive's native 7z **copy codec** to create a VST3 bundle plus manifest/install notes; extracted the complete VST3 again independently, and confirmed the extracted full binary SHA-256 matches. Divided archive into 13×28 MiB max volumes; verified the concatenated part SHA-256 matches the full archive hash.
7. **Drive:** Uploaded all 13 volumes and 2 sidecars to a new `CP12R1_VST3_Candidate_HostQA` folder, verified all 15 remote records and their aggregate volume size. Did **not** replace the latest user-confirmed CLAP.

## SHA-256 evidence

| Artifact | Size (bytes) | SHA-256 |
|---|---:|---|
| Canonical CP12R1 CLAP | 378,516,252 | `65668f7a244c0e311741d6545b71d603e188ea27eed2e9b57c963da1edf28e1a` |
| Original CLAP 7z | 378,521,892 | `0fd56e33831c46a3133ccb05d3383c21ee899dea27a0d4351ca24d758a644496` |
| Windows VST3 compiled core | 239,616 | `6bf3f3a14951ee2c8628ec1d76a6755994bd9908ebd17c08f2e52c0c055225c3` |
| 2,304-hit PCM24 bank (shared) | 373,714,116 | `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454` |
| Approved CP12R1 SkinSync raster | 4,576,824 | `a6db03fc0c28dcd269bc0750e2fddac1d03e5faa8d8055c834dc7f1b077df9bb` |
| Complete Windows x64 VST3 bundle binary | 378,530,588 | `f9d63a7416a2ecd16f5d99df306441aa90b1081addff15eac87b17eb96dd4ca6` |
| Native VST3 7z distribution | 378,534,736 | `078a839b1e0b790ea3925febbf7da36e07e617b82263cdc299084732bc1961b8` |

## What is actually validated

- The VST3 is a real PE32+ Windows x64 DLL exporting `GetPluginFactory`; it is no longer a missing-bank core.
- Entire original 2,304-hit audio payload and original GUI asset are **byte-identical** to CP12R1 CLAP.
- Windows CI synthetic diagnostic independent loader successfully exercised `LoadLibrary`, factory creation, `IComponent::initialize` and bank resource loader (with a *synthetic bank*, **not** the exact original bank).
- Windows CI source-only mock host verified MIDI Learn, pad activity and controller receipt. Host mechanism uses `outputParameterChanges`.
- Windows CI synthetic PCM test compared the VST3 processor with the canonical CP12 engine, **not** the two installable plugin binaries, and matched samples at 44.1, 48, 88.2 and 96 kHz.
- Independent 7z extraction and exact VST3 SHA check passed in ChatGPT's active container.

## What remains UNVERIFIED

- **Actual Bitwig Studio Windows x64 VST3 scan/load and editor lifecycle**, including any separate-process hosting of edit controller.
- VST3 GUI appearance, button behavior, text readability and no overlays **inside Bitwig**.
- Host delivery of the transient VST3 `outputParameterChanges` for MIDI Learn and animated pad feedback; the real host might require Data Exchange fallback.
- Host save/reopen persistence for `.rbmap` mapping and CP12 state v1/v2/v3 migration, plus multiple instances.
- Full original PCM bank numerical CLAP-vs-VST3 audio comparisons and end-to-end MIDI note response/CPU under load.
- Formal Steinberg VST3 Validator and pluginval on the fully assembled original-bank bundle.

## Windows host QA steps

1. Back up any old VST3; close Bitwig.
2. Extract the .7z.001 as explained above. Install the **folder** `Rubedo Atlas Drumkit.vst3` to `C:\Program Files\Common Files\VST3\`.
3. Scan plugins in Bitwig and open the editor; check for black/blank screen, font differences and unwanted overlays.
4. Trigger 24 pads at defaults MIDI notes 36–59, including Core and Extended. Test fast repeated hits and simultaneous notes at 88.2kHz.
5. Arm MIDI Learn on pad 1; send note 72; confirm controller/gui note display and audio note-to-pad behavior. Swap against an already assigned note and confirm uniqueness.
6. Save `.rbmap`, load across projects and compare all pad assignments after close/reopen.
7. Test 44.1/48/88.2/96 kHz, kit swaps, pad audition, 2 plugin instances, automation, and device unload/reload.
8. If Bitwig fails plugin scan or becomes unstable, remove **only the VST3 candidate** and return to the unchanged CP12R1 CLAP.

**2026-11-17 VST3 ABI audit:** The existing downloadable HostQA package was assembled using the pre-fix core from GitHub Actions run `43975514483`. Later official Steinberg validation of that source family detected a missing mandatory `IProcessContextRequirements` interface (46 passed, 1 failed). The missing interface was implemented in source commit `e0d0681dac964951fbd26762177c5898986c22ac`, with regression checks in `8719c8375af89620f4d1203f0109cf9f11869153` and `572f5252676221118319c6ce47838ba3a3318fb2`. **These commits do not modify the VST3 candidate already on Drive. A new full-resource VST3 package and independent official validator rerun are required before replacing/promoting it.**

**Release promotion gate:** Do not mark this VST3 as user-confirmed until the real Bitwig host tests pass. Full byte-identical resources do not by themselves prove controller-host behavior.
