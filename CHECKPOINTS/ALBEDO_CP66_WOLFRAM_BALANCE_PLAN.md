# Albedo CP66 — Wolfram Preset Generation and Four-Engine Balance

Development branch: `albedo/cp66-wolfram-preset-arp`; parent: verified `albedo/cp65r1-windows-ci-verified`. Do not overwrite CP65R1.

## Revised sonic specification
- Every one of the 192 presets passes through a deterministic elementary Wolfram cellular automaton (Rule 30, 90, 110, or 150), seeded per preset. The lattice shapes harmonic partials and organic spectral/modulation motion. TNSP remains the sound-generation concept tree, **not merely a filtering algorithm**.
- Four synth generators must not become an always-dominant main oscillator with inaudible support. Role profiles balance four source *parameter levels* and vary all four supported generator methods. Original primary engine identity and MIDI root are preserved.
- The 8 existing Wolfram arpeggiator characters (6031) can be assigned per preset, without automatically enabling the arp. Character, tempo, gates, octaves and 4–16 step layout remain functional and independent. If 6031 is reset by program application, ensure it is explicitly included in generated JSON and native program headers.
- Use the existing GUI's Wolfram Character (6031), Cellular Rule (4003), four generator controls, and cellular motion view. A future compact four-engine contribution display must use measured per-engine signal energy rather than guessing energy from UI values.
- No new UI tab, redraw loop, JS page reload or dynamic editor resizing. Keep the RGNES responsive horizontal layout, existing fonts, names/roles and single bank.
- Nereida Intrasynth provides the reference for *event clock, timed note-off and held-note ownership*; Wolfram changes must not disrupt host sync, free clock, sustain or stopped transport handling.

## Reproducible generation sequence
1. Retrieve original CP51 192 program input as in CP65 workflow; run CP59 and CP60 processing.
2. Run CP65 retimbre / one-bank catalog.
3. Run `python patches/cp66/wolfram_generate.py --metadata Albedo-JUCE-Native/Metadata --report CP66_Wolfram_QA.json`.
4. Run `python patches/cp66/balance_generators.py --metadata Albedo-JUCE-Native/Metadata --report CP66_Generator_Balance_QA.json`.
5. Regenerate native program headers and HTML catalogs **from the same final JSON**; do not ship an HTML-only update.
6. Run native Windows CLAP/VST3 builds and audio QA across 44.1/48/88.2/96 kHz and DAW/ASIO buffer sizes.

## Required audio verification
- Measure isolated per-engine RMS/short-term loudness, peak, spectral centroid and contribution to full mix. The nominal percentages in the generator are **not** calibrated energy. A target is that no engine dominates unintentionally; deliberate single-engine characters are still permitted.
- Test against 192×192 perceptual feature similarities; compare envelope, spectral character, pitch/register, modulation and dynamics; regenerate near-duplicates through the idea-permutation tree.
- Check role differentiation vs shared bio-cyber, fluorescent, recursive, bouncy neo-Renaissance palette.
- Explicit stuck-note, idle-resume, arpeggiator OFF, long-held chord, sustain, timing at 110 BPM / 1/16 and 12-voice tests.

**Status:** CP66 generator/balance source added to the development branch. Its outputs, GUI integration, binary compilation and listening validation are not yet verified. CP65R1 remains the usable rollback.
