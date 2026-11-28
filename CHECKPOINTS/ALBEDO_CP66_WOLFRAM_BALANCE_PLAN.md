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

## CP66 terminal-leaf Concept Tree (revised design)

User correction: **every preset must be the end of a significantly diverging branch**.
Concept Tree is a generative search, not a similarity-only filter.
The tree now follows:

```
Instrument role (8)
  -> Excitation architecture (6)
    -> Articulation / temporal gesture (4)
      -> Unique final preset leaf with Wolfram rule, generator arrangement,
         harmonic topology, filter structure and deliberate noise policy
```

Implemented in `patches/cp66/terminal_concept_tree.py`, run **after** `wolfram_generate.py` and `balance_generators.py`. Each leaf has a unique ID, structural lineage and source/ADSR/spectral definitions. Existing program indices, root/keytracking, musical clock and note safety are held constant.

### Conscious noise
The actual instrument registry identifies `6 = Noise / Stochastic`, not a generic wave method. Noise is not mandatory for every sound:
- dry tonal branches use NO additional noise;
- transient branches use a short attack excitation where appropriate;
- breath branches supply air and onset;
- friction branches add rasp and resonant friction;
- granular branches supply particles/texture;
- environmental-bed branches provide controlled air and diffuse texture.

Noise amplitude is bounded by instrument role, typically much lower than the pitched body, and it is restricted to an intentional source rather than global broadband hiss. Original primary noise sounds are retained and explicitly labeled as inherited stochastic excitation. This currently controls the method and its existing spectral/filter parameters; it does **not** claim independent colored-noise filters or measured signal-band energy that the synth doesn't implement.

**Important:** nominal generator percentages and metadata uniqueness cannot prove perceptually distinct sounds. The release gate must render all 192 programs, measure waveform/envelope/spectral/temporal features, detect highly similar endpoints (within and across roles), then regenerate those leaves through a different architecture or material path. The final GUI must reflect any new runtime control accurately; CP66 currently reuses existing method, oscillator-level, envelope, filter, cellular and Wolfram fields without inventing controls.

## CP66 root-first material diversity revision

The user identifies the shared spectral envelope and near-identical harmonic shapes as the critical source of timbral sameness, not simply the number of branches. The generator must now select **roots first**:

1. **Reference pool** — reuse 24 license-reviewed WaveEdit source tables (from 705 inspected) plus all Albedo's native synthesis methods; do not claim new sample acquisition.
2. **Sound-type roots** — fundamental, low-harmonic, vocal/formant, acoustic/resonant, metallic/spectral, digital/inharmonic, evolving/granular and stochastic.
3. **Source feature interpretation** — select by fundamental-vs-upper harmonic energy, frame evolution, role, and underused material; preserve original table SHA provenance.
4. **Primary synthesis method per root** — analog (0), wavetable (1), physical/modal (5), spectral (4), phase distortion (8), granular (3), or stochastic (6). Different methods *really are different source architectures*, not cosmetic preset labels. When appropriate, a different primary method still uses the chosen wavetable in a supporting generator.
5. **Complexity type** — minimal (one active generator), restrained (two), layered (three), dense (four). Simple presets may have no added motion, chorus, reverb or noise. Never force all sounds into a common thick/evolving timbre.
6. **Terminal Concept Tree** — per-role excitation branch, gesture, source selection and rule; preserve real MIDI note ownership, synchronized arp timing and 192 program IDs.
7. **Audio evaluation** — measure spectral centroid, rolloff, flatness, harmonic-to-noise ratio, MFCC/log-mel distance, onset/envelope and temporal variation. Near-duplicate leaves must be regenerated by choosing a *different root material/architecture*, not only adjusting oscillator brightness.

New script: `patches/cp66/root_materials.py`. Execution order after CP65 retimbre: `root_materials.py` → `wolfram_generate.py` → `balance_generators.py` → `terminal_concept_tree.py`. CI now checks Python syntax, 8 root families, 4 complexity classes, at least 6 primary methods and at least 16 selected source tables. The root spectral vectors are proxies from the CP59 catalogue, not proof of rendered timbre. Rebuild native preset headers and live GUI catalogue from final JSON before compiling or shipping.

## Timbre identity as the generator's possibility boundary

Each source root has a **timbre identity contract**: (a) source/provenance, (b) defining spectral or temporal traits, (c) allowed transformations, (d) transformations that destroy the identity, (e) intended instrument role. It is not enough to pick a source and then mutate everything: the resulting sound must remain perceptually related to its root if presented as a descendant of that source. When a transformation crosses the root identity boundary, branch into a **new root** and name its new identity explicitly.

Implementation: `patches/cp66/timbre_identity.py`, run after `terminal_concept_tree.py`; 192 contracts in `CP66_Timbre_Identity_QA.json`. The contract checks preservation of the assigned primary synthesis method, audible licensed wavetable selection when used, and complexity-dependent count of actually active generators. It annotates the intended spectral feature anchor and legitimate transformation dimensions. This is a **structural** validation gate, not a claim that sound is perceptually preserved. The user must be able to hear the root's timbral fingerprint in final output.

Design rule: **similarity within a lineage, diversity across root families**. Neither maximize divergence indiscriminately nor converge everything toward one bright, resonant, dense color. Native render evaluation must measure source-to-leaf timbral relatedness separately from leaf-to-leaf diversity, using MFCC/log-mel spectral envelopes, harmonic ratios, transient envelopes, crest/noise balance, and temporal change. Regenerate a leaf if (1) it is indistinguishable from another endpoint, (2) it loses root identity without an explicitly new root, or (3) it has unnecessary complexity/noise. Sample-rate and DAW/MIDI safety QA remains required before release.

## User correction — final governing principle (supersedes rigid identity preservation)

> **Diversify according to good-quality possibilities that can render good-quality instruments.**

The source root is a **creative search space** informed by timbral properties, not an identity-preserving prohibition. Tone and harmonic shape remain visible because they predict what transformations may be sonically successful, but any root may be radically reshaped when the result is a stronger instrument. No forced constraint requiring timbral similarity to the root. Avoid assuming maximal complexity, maximal difference, or maximal similarity is inherently desirable.

Generation sequence: source material/type -> plausible instrument roles and sound structures -> divergent candidate transformations (including simple, sparse, noisy only where musically justified) -> sound rendering -> **quality + playable function + tonal clarity + spectral diversity** evaluation -> selected terminal presets. The method must **generate possibilities**, not merely filter a pre-existing bank. Similarity analysis is a diagnostic (duplicate detection), never a substitute for creating good timbres.

Quality gate still requires actual audio rendering: usable register, accurate pitch stability, musical transient and sustain, sufficient body without unwanted masking, smooth DSP/no aliasing/crackling, consistent intentional output levels, and distinct timbral contribution. These are *evaluative goals* pending rendering, not presently verified results.
