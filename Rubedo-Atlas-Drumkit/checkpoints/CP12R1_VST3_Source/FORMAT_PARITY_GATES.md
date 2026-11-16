# Rubedo CP12R1 — CLAP / VST3 equivalence gate

**Audit:** 2026-11-16. **Status: NOT EQUIVALENT YET — VST3 release blocked.** This is a factual comparison of the actual source at the two linked checkpoints. The ChatGPT local compilation/container runtime was unavailable during this audit; authoring was performed through the connected GitHub source tools. Windows compilation checks below are GitHub Actions (external), *not a native build executed inside ChatGPT*.

## Canonical baseline

- CLAP: `checkpoints/CP12R1_Pad_Maps/source/` with the released self-contained Windows x64 CLAP in Google Drive folder `00_LATEST — CP12R1_Pad_Map_SaveFix`.
- VST3 candidate: `checkpoints/CP12R1_VST3_Source/source/` (not installable). Both compile against the exact **CP12R1 engine C/H** and the CP11 embossed skin-card source.
- Exact CP12R1 full CLAP binary (SHA-256): `65668f7a244c0e311741d6545b71d603e188ea27eed2e9b57c963da1edf28e1a`. The VST3 assembly helper checks this hash before reading the private GUI/PCM overlay.
- Original CP11/CP12 PCM bank region should be identical in both formats if the assembly succeeds: 373,714,116 bytes, SHA-256 `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454`. **Not yet checked on a completed VST3.**

## Source parity changes completed (not host tested)

**2026-11-16 follow-up:** [Windows CI run 43970673570](https://github.com/uill12/Plugins-VST-s/actions/runs/43970673570) succeeded. It runs `qa/test_midi_feedback.c` (24 Learn targets; normalized mask, result and unmapped-note codec) and `qa/test_vst3_process_bridge.c` against the **compiled actual VST3 core** with simulated VST3 input events and host input/output parameter queues. The processor test passed: note 72 assigned to pad 1 via Learn, mapping and Learn result emitted, repeated pad activity emitted, unmapped note 100 reported, and simultaneous note 37+38 returned a two-pad mask. The compiled PE32+ core still **lacks** the private audio/GUI resource overlay and has not been loaded in Bitwig. Host support and complete VST3 audio parity are not established.

Current feedback contract, unchanged VST3 IDs 0–376: `377` Learn target; `378` bounded pad activity mask; `379` unmapped note; `380` Learn result. These event values are transient and not saved into the CP12 V3 state blob. Processor-to-editor feedback travels through the VST3 host; no direct global processor-pointer access from the editor. See [wire codec](source/rubedo_vst3_feedback.h) and [Windows source-only workflow](../../.github/workflows/rubedo-cp12r1-vst3-source.yml).



- Stable original VST3 CIDs and existing host parameter IDs 0–159 retained, with 24 note-map IDs 136–159.
- VST3 note-map host edits now use `rubedo_assign_note` (swap-on-collision) rather than direct atomic writes, matching the CLAP engine's collision policy.
- VST3 mapping parameters display MIDI note values 000–127 instead of percentages. Both VST3 factory metadata variants report CP12R1 rather than contradictory CP08 version text.
- VST3-only GUI forwards previously unconnected per-pad pitch, decay, FM, gain, attack, release, swing, scatter, chance and edits to all four sequencer patterns.
- VST3 controller/processor parameters extend to 377 (IDs 160–376 added; IDs 0–159 preserved). Extra parameters cover per-pad gain/attack/release, 4 saved 24-step patterns, selected variation and decay/FM activity flags.
- VST3 source now loads these per-pad and variation settings from the same CP12R1 V3 extra-state block, and initializes the editor model from the controller. These are source changes, not a host-verified guarantee.
- Save/Load `.rbmap` dialogs use the CP12 codec and embedded CP11 SkinSync style; Windows file I/O is on the GUI thread.
- Windows source-only compilation with PE32+ x64 and `GetPluginFactory` export passed in GitHub Actions: [run 43965028126](https://github.com/uill12/Plugins-VST-s/actions/runs/43965028126). **This compiled DLL does not contain the bank or artwork.**

## Blocking discrepancies (must be resolved before claiming VST3 equivalence)

| Gate | Expected equivalence | Current VST3 status |
| --- | --- | --- |
| B1 — Installable VST3 | Full CP12R1 GUI + 2,304-hit PCM bank embedded, valid PE+footer and proper VST3 bundle | **BLOCKED:** only source/compiled core; no final resource overlay |
| B2 — MIDI Learn | Bitwig MIDI note enters processor, captured target pad and updated map returned to editor and host state | **SOURCE IMPLEMENTED / MOCK-HOST PASS:** Uses VST3 outputParameterChanges IDs 377 and 380 with the canonical 24 note-map IDs; Bitwig support not yet verified. |
| B3 — MIDI note indication | Host MIDI notes drive Core/Extended pad flash, including simultaneous hits and unmapped notes | **SOURCE IMPLEMENTED / MOCK-HOST PASS:** Output parameters 378 (bounded 24-pad activity mask) and 379 (unmapped notes) update the editor model; actual host callbacks, repaint and timing remain unverified. |
| B4 — Parameter/state round trip | Host audio uses exactly the values displayed by GUI after automation and close/reopen, including V3 patterns, per-pad envelope and active flags | **SOURCE PORTED; HOST UNVERIFIED:** verify real transactions and state migration |
| B5 — Pad audition | Each clicked pad produces exactly the routed voice with reliable repeated trigger | **SOURCE PORTED; HOST UNVERIFIED:** uses host edit parameter 135; not a sample-accurate event bridge |
| B6 — Map transactional apply | Save/Load and host edits preserve all 24 distinct MIDI notes under update order and multi-instance load | **SOURCE LOGIC IMPROVED; HOST UNVERIFIED** |
| B7 — Sample-output parity | Same sounds, velocity/round robin, output levels, 88.2k playback and note onset timing | **UNTESTED:** no complete VST3 PCM build to null-test |
| B8 — Plugin host compatibility | Valid VST3 SDK ABI, preset persistence and multiple Bitwig instances | **UNTESTED:** hand-authored VST3 ABI; compiler success does not certify Bitwig |
| B9 — Runtime performance | No additional RT locks/allocations, bounded note response and GUI CPU; no stuck notes | **UNTESTED in host** |
| B10 — Distribution | Full VST3 `.vst3` directory, SHA-256, independent 7-Zip extraction, Drive readback | **NOT UPLOADED** |

### Release acceptance procedure

1. Compile **both** actual formats from the linked CP12R1 sources; confirm plugin IDs/CIDs and source revisions. VST3 source-only CI now passes the normalized feedback codec and mock-processor/host tests (see run below).
2. Copy *exact verified* CP12R1 PCM+GUI data into the VST3, adjust the trailing footer offset, and validate the extracted bank hashes. Do not replace or resynthesize missing user assets.
3. **In Bitwig**, verify the implemented VST3 `outputParameterChanges` channel for Learn/GUI note feedback. Test empty or unsupported output queues and use official Data Exchange/compatibility fallback if the host fails to deliver changes. No unmanaged cross-process pointers.
4. Use deterministic MIDI input (default notes 36–59, two custom map swaps, unused notes and simultaneously held notes); compare audible pad, dynamic label, selected pad and Learn outcome in CLAP and VST3.
5. Save a named .rbmap file in both formats; cross-load it; save/reopen a Bitwig project; compare all 24 notes and V3 settings.
6. Render a scripted 24-pad ×2 velocity ×2 round-robin test at 44.1/48/88.2/96 kHz with matched settings; compare timing and PCM output numerically (not merely both audible).
7. Validate scale, redraw, font rendering, no overlays, CPU/latency and multiple instances in Windows Bitwig for each format separately.
8. Only after all gates pass, archive an installable VST3 bundle as native `.7z.001` volumes, verify complete extraction and upload to its existing Drive CP12R1 folder. Keep prior working CLAP intact.

**Do not promote a CI core or checklist as an installable VST3 or imply release parity based solely on a passing PE/export check.**
