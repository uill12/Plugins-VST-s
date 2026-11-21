# Albedo Quintessential Synth — CP54 Sound-First Interface

**Branch:** `albedo/cp54-essential-sound-layout`  
**Parent:** CP53 Hi-Fi and MIDI safety checkpoint (Windows CLAP/VST3 CI passed)  
**Version:** 1.0.54 (host identifiers unchanged)  
**Status:** **Windows x64 CLAP/VST3 compilation and automated native DSP, WebView2 host, 192-preset and interface QA PASSED**, source run 44869174353 (commit `1418c18a2528d0fa72b4b00ce582116cef8ebacc`). Strict-VST3-JSON corrected distribution repackaged from identical compiled binaries in GitHub run 44870367506. **Manual Bitwig Studio QA is still pending; do not claim a Bitwig-tested release.**

## Principal decision

The parameters with the greatest direct impact on the sound remain **permanently visible** on the Albedo stage. Their editing must never depend on entering a mode, popup, submenu, or Advanced/Lab screen. Contextual popups are for less influential secondary sound-shaping and performance configuration.

### Fixed sound stage

- **Prima Materia:** Four independent oscillator/source cards always show synthesis method, level and bypass. The source inspector always shows the two foremost character parameters. Material and Unison become permanently visible selectors in the inspector whenever the chosen synthesis method supports them.
- **Quintessentia:** XY morph and first four macros (Brilliance, Exchange, Contour, Impact) remain visible. Width, Growth and Memory move to a contextual Macro popup.
- **Corpus:** Filter A/B types, cutoff, resonance and **Drive** remain on stage. Amplitude **Attack, Sustain and Release** stay on stage. The slower-changing envelope Decay and specialist filter envelope/voice settings move to the Corpus popup.
- **Syzygia:** Reciprocal engine couplings remain visible. Detailed source-target symbolic modulation routes live in a local Routes popup.
- **Pulsus:** Arpeggiator enable, timing and step pattern remain visible. Cellular rules, additional LFO and rhythmic articulation options open beside the Arpeggiator.
- **Spatium:** Each effect's bypass and wet/dry Mix remain permanent. Effect damping, size, chorus rate and bias reside in the FX popup.
- **System:** Tuning, oversampling and tonal guard are accessible via a small contextual System popup rather than continuously occupying the main synthesis surface.

### Local contextual popups

Seven small, anchored and scrollable popups: Source, Macro, Corpus, Routes, Rhythm, FX and System. Each is reachable from the module that uses it, placed inside the actual WebView viewport, dismissible with Escape or outside click, and keyboard accessible. Dialog contents use the existing delegated dial handlers and, for CP04 source/FX details, the single original `setExtraParam` function.

The legacy **Synthesis Lab / Advanced mode** launcher and F3 shortcut are retired. The old parameter registry (197 original parameter IDs plus CP50/CP53 resources) remains available for factory presets, automation and host sessions. No DSP parameter or factory sound is removed. This is a layout and interaction change.

### Stability

- Permanent controls are never moved into an overlay. The page canvas is not rerendered on mouse movement or every parameter change.
- Only secondary symbolic routes, cellular detail and utility controls are temporarily moved while their popup is visible, and are restored to their original DOM parents on close.
- Source material/unison displays synchronize on engine/method and native preset changes without marking unmodified factory presets as edited.
- CP52 Nereida-style browser, 90% compact size, and the 192 CP51 factory program IDs remain intact.
- CP53 high-fidelity output and MIDI voice safety are retained unchanged.

## Evidence and release gates

- [CP54 real Edge QA, seven local popups across 1080×720, 900×600 and 720×480 — PASS](https://github.com/uill12/Plugins-VST-s/actions/runs/44849571720). Covers permanent high-impact controls, state-changing native extra controls, source material/unison, original route restoration and preset browser compatibility.
- [CP53 native Windows CLAP/VST3 and DSP QA — PASS](https://github.com/uill12/Plugins-VST-s/actions/runs/44847934279). This verifies the inherited audio base, not the new CP54 GUI binary.
- [CP54 final native Windows CLAP/VST3 build — PASS](https://github.com/uill12/Plugins-VST-s/actions/runs/44869174353): CTest 8/8, real compiled host WebView2, MIDI parameter bridge, exports, embedded PCM24 and 192-preset output checks.
- [CP54 final contextual Microsoft Edge QA — PASS](https://github.com/uill12/Plugins-VST-s/actions/runs/44869174350): original synthesis GUI with 7 popups, ARP bridge event delegation and small-window geometry.
- Perform Bitwig Studio Windows manual host testing for 192 presets, window resizing, source controls and popups, mousewheel behavior, MIDI chords/release and 44.1/48/88.2/96k sample rates.
- Keep CP51/CP52/CP53 source and binary artifacts intact as rollback. Do not declare CP54 a verified installed release before native and host QA.

## Final CP54 Windows build and packaging (2026-11-21)

- Native Windows CI source: `1418c18a2528d0fa72b4b00ce582116cef8ebacc` (no sound/midi source modified by the subsequent packaging-only workflow).
- [Windows CLAP/VST3 build and QA](https://github.com/uill12/Plugins-VST-s/actions/runs/44869174353): **success**, including binary exports, embedded 32-material PCM24 bank, 8/8 CTest, real compiled WebView2 host resize, full MIDI/ARP parameter echo, 192/192 audible presets, finite audio, six sample rates and 18,336 output pair comparisons.
- [GitHub final strict-metadata package](https://github.com/uill12/Plugins-VST-s/actions/runs/44870367506): **success**, published as `Albedo_CP54_Final_HiFi_CLAP_VST3_Windows_x64`. The VST3 `moduleinfo.json` generated by the underlying build had invalid trailing commas; the packaging step now validates the JSON strictly and repairs only this metadata resource. Native CLAP and VST3 binary bytes remain unchanged.
- [Google Drive CP54 folder](https://drive.google.com/drive/folders/1ESvvrFMIIlEPsgDGWmlec_tvX9NKv8z_): verified plugin ZIP uploaded without overwriting older checkpoints. Distribution contains CLAP, complete VST3 folder, corrected strict JSON metadata, CP54 provenance and SHA256 checksums.
- Compiled binary hashes, SHA256:
  - `Albedo_CP54_SoundFirst_HiFi_JUCE_WebView2.clap`: `9189a2423efc01279cde895e44892979126e1584090510ec53e750a6f9c0c7cd`
  - `Albedo Quintessential Synth JUCE.vst3/Contents/x86_64-win/Albedo Quintessential Synth JUCE.vst3`: `de7661c30edb3894cfb7300b3c043121d33b373da4c51c708e7b9a884b88bc06`
- Bitwig Studio **manual** host validation still required (especially sustained chords, program switching, sleep/transport, multiple sample rates and VST3/CLAP popup interaction). This is a CI-verified distribution, not a Bitwig-verified installation.
- CP51, CP52, CP53 rollback branches, artifacts and Drive folders are untouched.
