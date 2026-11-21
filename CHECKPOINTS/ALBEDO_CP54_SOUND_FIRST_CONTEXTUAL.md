# Albedo Quintessential Synth — CP54 Sound-First Interface

**Branch:** `albedo/cp54-essential-sound-layout`  
**Parent:** CP53 Hi-Fi and MIDI safety checkpoint (Windows CLAP/VST3 CI passed)  
**Version:** 1.0.54 (host identifiers unchanged)  
**Status:** Source implemented, Edge visual QA passed on the sound-priority controls and contextual overlays. Native Windows CLAP/VST3 build for the exact CP54 commit in progress. Bitwig Studio manual host QA pending.

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
- [CP54 Windows CLAP/VST3 build](https://github.com/uill12/Plugins-VST-s/actions/runs/44849699541): check final status and artifact integrity before promotion.
- [CP54 final contextual Edge run](https://github.com/uill12/Plugins-VST-s/actions/runs/44849699605): check latest commit.
- Perform Bitwig Studio Windows manual host testing for 192 presets, window resizing, source controls and popups, mousewheel behavior, MIDI chords/release and 44.1/48/88.2/96k sample rates.
- Keep CP51/CP52/CP53 source and binary artifacts intact as rollback. Do not declare CP54 a verified installed release before native and host QA.
