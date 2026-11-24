# Albedo CP62 — Arpeggiator articulation, GUI and removal of accents

Branch: `albedo/cp62-arp-gui-no-accent`; parent `albedo/cp61-midi-onset-note-ownership`.

## Implemented in repository
- Canonical HTML no longer exposes parameter 6022 (Cellular Accent); original state/registry ID remains for backwards compatibility.
- Visible CP56 stage has Gate, Swing, Octaves, Steps, Rotate and Internal Tempo only when clock mode is Internal.
- CP57 keeps Nereida-like character trajectories and Wolfram variants, while retaining Albedo nomenclature; duplicates in native tempo events are prevented by removing the redundant DOM change dispatch in `commit()`.
- Popup offers time signature 6032 and gets per-parameter host updates; meter enumeration now equals CP57's canonical 4/4, 2/4, 3/4, 5/4, 7/8, 6/8, 9/8, 12/8 ordering.
- Main stage keeps fixed geometry and avoids re-rendering on control edits.
- `patches/cp62/remove_arp_velocity_accents.py` is a guarded post-CP61 source transform; **apply it to the reconstructed native DSP before building**. It neutralizes 6022's audible effect, including restored presets, while keeping the existing parameter index and held-key velocity.

## Required remaining acceptance
1. Reconstruct full CP61 source from verified CP59/60/61 pipeline and apply CP62 DSP script, checking the strict anchor.
2. Run Edge GUI tests, especially `check_cp56_sound_priority.py` and `check_cp57_tempo_gui.py` at 1230x820/900x600/720x480.
3. Build Windows x64 CLAP and VST3 with MSVC/JUCE; verify both formats and automatic DAW clock, internal BPM, host transport start/stop, 6032 meter, no accented velocities.
4. Regression: ARP off has no onset delay; 1–12 concurrent notes, overlapping Note IDs and sustain, repeated program loads, ASIO4ALL small blocks, 44.1/48/88.2/96 kHz.
5. Do not label CP62 a verified binary release before passing host/build tests.

## Compatibility
Do not renumber 6022, 6023–6032 or overwrite previous functional checkpoint. New audio semantics ignore 6022 but preserve serialized values for old projects.
