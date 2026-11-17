# Rubedo Atlas Drumkit — CP12R1 VST3 port (engineering checkpoint)

**Current recommended HostQA package (ABI fix):** [CP12R1 VST3 ABI-Fix Windows x64 — Google Drive](https://drive.google.com/drive/folders/1LIk9LjSZHe7w-NPSZSAMq1Zmb63yPgoo). The source-only 2026-11-17 GitHub Actions run [44028606392](https://github.com/uill12/Plugins-VST-s/actions/runs/44028606392) passed **47/47 official Steinberg VST3 validator tests on the synthetic diagnostic**, and a new full-resource VST3 binary using the same core was independently repackaged and uploaded. The old 2026-11-17 VST3 candidate remains archived for comparison but is superseded; the CP12R1 CLAP rollback is unchanged. **Windows Bitwig real-host validation is still required.** [Detailed ABI-Fix QA](CP12R1_VST3_ABIFIX_HOSTQA.md).

## Current status — COMPLETE-ASSET VST3 CANDIDATE, NOT BITWIG-VERIFIED

On **2026-11-17**, ChatGPT's active container retrieved the **exact original CP12R1 CLAP** from its Drive multipart archive, verified the CLAP, PCM and GUI SHA-256s, and combined the original resource overlay with the Windows x64 source-only VST3 core from [CI run 43975514483](https://github.com/uill12/Plugins-VST-s/actions/runs/43975514483). A full native multi-volume 7z with the VST3 binary was independently extracted and the complete SHA-256 matched.

**[Download VST3 HostQA candidate — Google Drive](https://drive.google.com/drive/folders/120_M3KXxH4HyoMFCAp2BOhyn60m-oCeR)** · [Full build evidence and remaining tests](CP12R1_VST3_REAL_ASSETS_HOSTQA.md).

**Do not confuse source-only CI artifacts with the assembled full-resource VST3 candidate.** The previous CLAP CP12R1 remains untouched as the known rollback. The full VST3 must still pass Bitwig real-host scan, GUI, MIDI Learn, state and CPU validation before promotion.

## Source contract

- Original VST3 component/controller CIDs preserved.
- VST3 IDs 0–135 preserved; MIDI mapping parameters 136–159 are 24 note numbers 0–127. Host parameter IDs **160–376** cover per-pad gain/attack/release, sequencer patterns, and activity flags. New transient parameters 377–380 provide MIDI Learn targeting and processor-to-editor feedback. Existing IDs remain stable.
- Audio processor consumes MIDI notes using the CP12R1 mapping table; controller receives and sends mapping parameter changes through VST3 host transactions.
- Save/Load Map Windows file dialogs are enabled in the **editor-only** VST3 GUI model; successful Load Map dispatches 24 mapping parameter edits to the controller/host.
- Native Save Map/Load Map uses the same `.rbmap` text format as CP12R1 CLAP. It does not change the kit or PCM timbres.
- VST3 processor state version 3 includes both CP12R1 extra-state blocks; version 1/2 reader is retained. Controller reconstructs mapping parameters on component-state restore.
- Rubedo CP11 embossed SkinSync appearance and approved CP12 Save/Load buttons retained; no generic overlays or typography changes.

## CLAP/VST3 parity gate

The [CP12R1 format-parity audit](FORMAT_PARITY_GATES.md) is mandatory before distributing the VST3 as a matching CP12R1 release. The VST3 source now forwards previously local-only per-pad envelope and sequencer controls, uses collision-safe MIDI assignments, and reports matching CP12R1 version metadata; the [latest Windows source compile](https://github.com/uill12/Plugins-VST-s/actions/runs/43965028126) passed. This **does not imply host parity** and does not solve the missing embedded-bank/runtime or processor-to-controller note-feedback bridge.

## Explicit feature limitation

**MIDI Learn and live pad feedback have now been source-implemented** using VST3 host `outputParameterChanges`, with event ids 377–380. The source-only Windows QA compiles and passes tests for note capture, note-map updates, repeat triggering, unmapped notes, multiple simultaneous hits, and controller parameter receipt. **This is NOT yet a Bitwig-verified processor-to-controller bridge:** host support for output feedback parameters and saved state must be tested; if Bitwig does not deliver this route, implement the official VST3 Data Exchange compatibility path instead.

This is an **engineering checkpoint**, not full feature parity. Native VST3 ABI, actual Bitwig host integration, controller separate-process behavior, note response, 88.2 kHz output, sample-bank loading, preset state and file dialogs are not yet host-verified.

## Source files

- [VST3 controller/processor implementation](source/rubedo_vst3.c)
- [CP12 SkinSync editor port](source/rubedo_gui.c)
- [Windows CI workflow](../../.github/workflows/rubedo-cp12r1-vst3-source.yml) — repository-root path is `.github/workflows/rubedo-cp12r1-vst3-source.yml`
- [Resource assembly helper](AssembleRubedoCP12R1VST3.ps1) — **optional local recovery**, requires a verified CP12R1 CLAP file and the fresh VST3 CI core. Creates a self-contained VST3 bundle locally; does not install or certify it.

Other shared source dependencies are in [CP12R1 source](../CP12R1_Pad_Maps/source) and the exact [CP11 card texture](../CP11_SkinSync/rubedo_blank_card.inc). The sinc table is generated by [generate_sinc_coeffs.py](../../generate_sinc_coeffs.py).

## Verified Windows CI compilation

The [CP12R1 VST3 Windows compilation workflow](https://github.com/uill12/Plugins-VST-s/actions/runs/43847931116) completed **successfully** on GitHub Actions `windows-2022`. It compiled the CP12R1 VST3 controller, CP12 engine, SkinSync GUI, sinc resources, and emitted a PE32+ Windows x64 DLL core with `GetPluginFactory` exported. The latest [Windows CI run](https://github.com/uill12/Plugins-VST-s/actions/runs/43971155669) also passes the actual processor/mock-host and wire-codec regression tests. It produces a source-only diagnostic artifact named `Rubedo_CP12R1_VST3_SourceOnly_NotInstallable`. It is **not** an installable plugin: it has no embedded PCM bank or GUI artwork overlay.

Optional Windows local assembly, when the exact CP12R1 CLAP file is already installed:

1. Download and extract the workflow's source-only artifact into this directory.
2. Use [RUN_ASSEMBLE_VST3.cmd](RUN_ASSEMBLE_VST3.cmd) or invoke [AssembleRubedoCP12R1VST3.ps1](AssembleRubedoCP12R1VST3.ps1) with the exact CLAP and VST3 core paths.
3. The tool verifies the CP12R1 CLAP SHA-256 and the resource footer, copies the image+PCM overlay to the new VST3 core and rewrites the bank's absolute offset. It outputs `Rubedo Atlas Drumkit.vst3/Contents/x86_64-win/Rubedo Atlas Drumkit.vst3` and its SHA-256 report.
4. This is a **local recovery path**, not a GitHub or Google Drive upload. Back up existing plugins and check the new VST3 inside Bitwig before relying on it.

## Historical source-only limitation (resolved by 2026-11-17 local assembly)

The originally identified missing-bank constraint has been resolved: the active ChatGPT container recovered the approved private resources, created the full VST3 candidate and independently validated full archive extraction. **The outstanding blocker is real Bitwig host certification**, not missing assets. See [HostQA report](CP12R1_VST3_REAL_ASSETS_HOSTQA.md).

## QA gates

1. **PASSED offline:** Windows x64 source compile with VST3 `GetPluginFactory` exported, MIDI Learn/feedback wire codec, real-processor mock-host callbacks and edit-controller synchronization.
2. Attach image+PCM overlay and update footer offsets; confirm embedded bank start and correct `RBDK24` header.
3. **NOT TESTED:** Load in Bitwig Studio Windows, including VST3 controller/processor lifecycle and actual output-parameter feedback.
4. Save/Load `.rbmap` and verify all 24 pad triggers, displayed numbers and host save/reopen.
5. Verify no regression to CLAP, visual SkinSync or CPU/note response.
6. **PASSED:** Full VST3 package assembled from real CP12R1 resources, extracted independently, distributed in 13 native `.7z.001` volumes and all 15 Drive folder items read back. **Host test still outstanding.**
