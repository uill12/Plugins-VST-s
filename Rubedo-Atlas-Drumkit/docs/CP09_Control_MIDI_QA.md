# Rubedo Atlas Drumkit CP09 — Control & MIDI Repair

Build ID: `0.9.4-cp09` · Windows x64 CLAP · experimental / requires Bitwig host validation.

## Reported defects
- Overlapping cyan diagnostic labels over approved layout.
- Incoming MIDI notes do not highlight corresponding pads.
- MIDI Learn occasionally creates ambiguous/duplicate assignments.
- Several knobs don't change sound; sequencer variation and graph controls are incomplete.

## Changes
- Removed input diagnostic box and status labels from approved GUI. Replaced incorrect opaque text background `COLORREF` blue cast with matched warm parchment background.
- Corrected and retained exact Win64 `CREATESTRUCTA` layout and DLL `DllMain` entry point from working CP08R2.
- Native MIDI note-on updates a 24-pad atomic bitmask; editor polls every 35 ms and flashes all newly struck pads for ~220 ms. Selection and Core/Extended page follow last MIDI note.
- MIDI Learn captures the next incoming host note-on only, swaps conflicts instead of creating duplicate mappings; Escape or a second Learn click cancels. Mappings remain included in plugin state.
- Pad clicks queue auditions from GUI to DSP using bounded single-producer/single-consumer lock-free queue. No direct voice mutation on the GUI thread.
- All 16 right-hand knobs are independently wired to correct engine state: pitch, decay, color, FM, attack, drive, space, voice level, gain, master output, voice attack time, voice release, swing, scatter, cellular, chance. Per-pad attack/gain/release are new, neutral by default.
- Envelope graph supports drag regions; sequencer Generate generates a new beat, Clear resets, VAR cycles four stored 16-step patterns. Variation, per-pad controls and status are saved with version-3 state. Version-1/-2 CP08 state remains loadable.
- Existing PCM24 sample bank and approved background GUI artwork preserved byte for byte; original 34 CLAP parameter IDs and plugin identifier kept unchanged.

## Offline validation
- Windows x64 compile with Clang+LLD: `clap_entry` exported, nonzero Windows DLL entry point, 7z package supports extraction.
- C engine QA at host 88.2 kHz passed: MIDI Learn swap, event flashing state, audition queue, 4 sequencer variations, saved state V3, 8 extreme control combinations with finite output.
- PCM+artwork overlay copied byte-for-byte from CP08 source with footer offset adjusted for new PE section size.
- **Compatibility:** Untouched CP09 preset DSP output was byte-identical to CP08 in 1024-frame, three-voice 88.2-kHz offline render with representative global parameters (SHA-256 `886dd6ed263467e1163f91c85fef4e03d5c845a38872672568b8bbe2b3344a51`).

## Limitations
- No Windows or Bitwig executable testing from this environment; in-host MIDI routing, editor rendering, host state persistence and automation remain to be confirmed in Bitwig.
- UI requests are reflected in plugin state; some non-CLAP-automatable per-voice controls are saved in the V3 extra-state extension, not exposed as separate host automation parameters.
- MIDI hardware must be routed through Bitwig to the instrument track. The baked `MIDI CONNECT` label does not directly pair hardware.
- Changes to a playing sequencer pattern from the editor are not synchronized to host transport. Back up old sessions before installation.