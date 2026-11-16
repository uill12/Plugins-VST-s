# Rubedo Atlas Drumkit — CP12R1 VST3 implementation resources and recovery plan

**Research date:** 2026-11-16
**Scope:** complete the CP12R1 VST3 from ChatGPT-accessible source and private CP12R1 audio/GUI assets, with CLAP/VST3 functional parity and realistic Windows Bitwig verification.
**Status:** research and source/reference audit only. Not a completed VST3 runtime build. The local ChatGPT Python/container execution service was unavailable in this session.

## A. Resources already available

| Item | Actual location | Verified role |
| --- | --- | --- |
| CP12R1 released CLAP (13 native .7z volumes) | [Drive CP12R1](https://drive.google.com/drive/folders/1IwAehMK6LZMmoJFXn8o5Dd0LYkIHMfmw) | Definitive embedded CP12R1 bitmap and 2,304-hit PCM resource source |
| CP12R1 CLAP source + QA archive | Same Drive folder, `Rubedo_CP12R1_Source_QA.zip` | Source and test files; raw archive access via connected Google Drive `fetch(download_raw_file=true, include_base64=false)` was confirmed |
| CP12R1 CLAP source checkpoint | `../CP12R1_Pad_Maps/source/` | Canonical DSP/mapping/GUI structs and state |
| Native VST3 source candidate | `source/` in this folder | Source-only controller/processor port; 377 parameter IDs as of the latest parity patch |
| Compiled Windows x64 VST3 core | [GitHub Actions](https://github.com/uill12/Plugins-VST-s/actions/runs/43965388822) | PE32+ and factory-export compilation only; no embedded asset region, NOT installable |
| Existing parity audit | [FORMAT_PARITY_GATES.md](FORMAT_PARITY_GATES.md) | Blockers and cross-format feature matrix |
| CP11 SkinSync original card artwork | `../CP11_SkinSync/rubedo_blank_card.inc` | Approved texture/typography baseline |
| CP12R1 mapping file | `../CP12R1_Pad_Maps/Default_36_59.rbmap` | Portable mapping regression fixture |

Known preservation checks from the existing CP12 QA report:
- released CP12R1 CLAP plugin SHA-256: `65668f7a244c0e311741d6545b71d603e188ea27eed2e9b57c963da1edf28e1a`.
- CP11/CP12 audio bank: 373,714,116 bytes; SHA-256 `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454`.
- approved GUI artwork: 1512×1009 pixels; 24 kit families/mappings and 2,304 hits. Never regenerate or resynthesize this data for a port.

## B. Verified external dependencies (official preferred)

1. **[Steinberg VST3 SDK](https://github.com/steinbergmedia/vst3sdk)** — official source, current README describes the 3.8 family; CMake, examples, test host and validator. SDK repository specifies MIT licensing; preserve its license obligations and any other dependency notices.
2. **[Official Steinberg VST3 C API](https://github.com/steinbergmedia/vst3_c_api)** — generated C header, with interface definitions for `Steinberg_Vst_IComponent`, `Steinberg_Vst_IAudioProcessor`, `Steinberg_Vst_IEditController`, `Steinberg_Vst_IConnectionPoint`, `Steinberg_Vst_IDataExchangeHandler/Receiver`, etc. This permits preserving Rubedo's existing C DSP and native GDI renderer instead of rewriting sound code in C++. Verify and retain the [C API's own license text](https://github.com/steinbergmedia/vst3_c_api/blob/master/LICENSE.txt), not an assumed license.
3. **[Steinberg Data Exchange](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Data%2BExchange/Index.html)** and **[tutorial](https://steinbergmedia.github.io/vst3_dev_portal/pages/Tutorials/Data%2BExchange.html)** — host-controlled audio-to-editor RT-safe data queue, with SDK compatibility layer for hosts without native Data Exchange support. Must test Bitwig's actual supported path instead of assuming it implements native Data Exchange.
4. **[Steinberg API communication](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/API%2BDocumentation/Index.html)** — `IConnectionPoint/IMessage` for non-RT private communication and host-driven parameter/state sync, `outputParameterChanges` for output automation. Do not directly send IMessage from the audio callback.
5. **[Steinberg plugin bundle structure](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Locations%2BFormat/Plugin%2BFormat.html)** — Windows `Rubedo Atlas Drumkit.vst3/Contents/x86_64-win/...` binary and `Contents/Resources/` for resource files. Consider putting the exact original PCM bank and art there rather than patching a 378 MB PE overlay, **but this is an architectural option that requires changing the resource loader and verifying actual output parity**, not a drop-in copy.
6. **[VST validator](https://steinbergmedia.github.io/vst3_dev_portal/pages/What%2Bis%2Bthe%2BVST%2B3%2BSDK/Validator.html)**, **[VST3PluginTestHost](https://steinbergmedia.github.io/vst3_dev_portal/pages/What%2Bis%2Bthe%2BVST%2B3%2BSDK/Plug-in%2BTest%2BHost.html)** and **[Tracktion pluginval](https://github.com/Tracktion/pluginval)** — independent ABI, lifecycle, editor, parameter and state testing. Follow with in-host Windows Bitwig tests.

## C. Recommended technical architecture

```text
                   Rubedo canonical CP12R1 engine (C)
                    | samples / parameters / maps |
              +-----+-----------------------------+------+
              |                                            |
       CLAP wrapper (C)                        VST3 wrapper (official C API)
              |                                   | audio processor (RT)
       CLAP editor                          native SkinSync editor (UI)
                                                  |
                                 host parameter + IConnectionPoint
                                                  |
                                 Data Exchange UI feedback queue
              +----------------- verified .rbmap codec ---------+
```

- A single authoritative 24-pad note map drives MIDI input, audio pad selection, note number and GUI highlight. Duplicate learned notes must use the canonical swap function (never silent duplicate assignments).
- A **bounded audio→UI event packet** can carry `sequence, pad, note, velocity, sampleOffset, learnChange, mapGeneration`. The controller consumes packets via Data Exchange or its SDK-provided compatibility path; UI updates are rate-limited/coalesced, not per sample.
- Capturing MIDI Learn happens from the note events at the audio processor. Communicate the accepted note+pad to controller, then perform canonical host parameter edits and dirty-state updates off the real-time thread. Verify save/reload and two simultaneous instances.
- GUI-to-audio requests: send host parameter changes or a supported private command; do not directly invoke a global processor pointer from the GUI or bypass the host on a separate-process editor.
- Apply 24-map preset loads as a validated atomic logical transaction. If the host requires individual automatable note params, publish the final consistent snapshot to the processor at an appropriate block boundary and test concurrent note arrivals.
- CP12R1 V3 editor state includes pad gain/attack/release, decay and FM enable flags, four pattern banks and active variation. Preserve stable plugin IDs and previously allocated host param IDs. Distinguish state from automation; expose only useful controls as host parameters if refactoring from the interim 377-ID source port.
- Rendering: preserve CP11 SkinSync bitmap, gold typography, correct Core/Extended labels, restrained status area, real hit-testing, no generic overlays.
- Real-time DSP: process note events at their VST3 `sampleOffset`; no audio-thread locks, allocation, disk access or GUI calls. Include timing and CPU tests across 32/64/128/512/1024 block sizes and 44.1/48/88.2/96 kHz.

## D. Recovering and packaging exact private resources

1. Read Drive CP12R1 manifest and get all 13 `.7z.001`–`.013` files using an **authorized byte-transfer interface**. The connected Google Drive `fetch` can stream a file as a scoped `file_uri`, and the CP12R1 `Source_QA.zip` has been returned this way in the current session. However, ChatGPT's local Python/container execution failed, so no extraction or binary build happened here.
2. Once local execution is functional, verify each volume against the existing SHA-256 manifest, decode the full native 7z archive, and compare extracted CLAP SHA-256 to `65668f7a...` exactly.
3. Parse the CLAP's `RBOVR103` footer, recover the **original** image and PCM bank bytes, validate bank SHA and all 2,304 regions. This is authorized project reuse, not asset resynthesis.
4. Two packaging options:
   - **Minimum-change:** append original overlay to the VST3 PE, adjust footer offset, validate same 373,714,116-byte bank and original artwork.
   - **Long-term maintainable:** bundle identical PCM and artwork under `Contents/Resources/`, add a resource-manifest-aware, memory-mapped or preloaded loader, check all hashes, and retest startup latency and MIDI note response.
5. Do not put private runtime audio bytes into ordinary Git history or a public CI artifact without the user's separate distribution authorization. An approved workflow could transport scoped assets to a Windows CI runner, but no cross-connector or permanent GitHub-to-Drive secret bridge has been demonstrated.

## E. Parity release gate — no exceptions

- Same 24 kits, 24 pads, velocity layers, round robins, note assignments and `.rbmap` file codec.
- MIDI Learn note-on event → canonical assignment → controller feedback → pad label/highlight → Bitwig save/reopen in **both** plugin formats.
- Same per-pad envelope, FM, effects, kit selection, transport and four pattern variations; saved V1/V2/V3 session restores.
- Same on-screen approved appearance (fonts, parchment, gold, embossing, Core and Extended).
- Matched deterministic offline MIDI→audio renders at 44.1, 48, 88.2 and 96 kHz, including variable `sampleOffset` and overlapping voices; check onset offset and numerical differences.
- VST3 validator + VST3PluginTestHost + pluginval + actual Bitwig Windows x64 scan, GUI lifecycle and separate-process tests. A passing Windows compile is *not* VST3 runtime certification.
- Release only after the full `.vst3` bundle is verified, packed as native 7-Zip volumes, Drive-uploaded with hashes, and the previous CLAP baseline remains available for rollback.

## F. Practical prioritization

1. **Resource recovery and reproducible local build** (unlock source and bank in one environment).
2. **Official C API migration/ABI validation**, retaining C audio engine and SkinSync GUI.
3. **Processor-to-controller MIDI Learn + live pad/event feedback** with Data Exchange/fallback, plus transactional note-map loading.
4. **V3 parameter/pattern state parity, audio timing and performance**.
5. **Independent validators, Bitwig check and runtime Drive distribution**.

**Research conclusion:** The necessary vendor APIs and source-building toolchain are publicly available. The main remaining material blocker is access to an executing build environment with the **exact private CP12R1 embedded PCM/GUI bank**. The VST3 core currently compiled by CI lacks those bytes, and an equivalent working host bridge is not yet verified.
