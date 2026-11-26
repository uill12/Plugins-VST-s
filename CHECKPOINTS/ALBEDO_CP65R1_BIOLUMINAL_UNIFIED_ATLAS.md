# Albedo Quintessential Synth — CP65R1 / Bioluminal Atlas

**Source base:** user-confirmed stable CP64R1 (commit `b614eea22173e0db984e0a37e030240b487daed3`)  
**Development branch:** `albedo/cp65-bioluminal-unified-bank`  
**Target version:** `1.0.65` (stable CLAP/VST3 plugin identifiers unchanged)  
**Reference rollback:** `albedo/cp64r1-windows-ci-verified` — **do not overwrite**

## Brief and sonic design

Revoice all 192 sounds with a cohesive **bio-cyber / fluorescent / expanding / recursive / elastic / neo-Renaissance alternative-electronica** vocabulary. Preserve intentional distinctions between bass, lead, pad, pluck, keys, pitched percussion, drone and texture. The goal is a recognizable shared material palette without timbral homogenization or arbitrary extremes.

### Development-time TNSP recipe

- **Shared partial invariant:** original primary sound exciter and MIDI-key tonal root, original musical role.
- **Branch permutations:** supporting synthesis engines, lower/upper octave bodies, harmonics, resonant architecture, envelope gesture and microtexture.
- **Octave harmonization:** secondary source layers configured across -12, +7, +12 and +24 semitones while primary root stays centered and tracking remains enabled.
- **Character axes:** elastic pulse, botanical growth, fluorescent filigree, recursive breath, resonant Renaissance, prismatic emergence.
- **Gain/FX safeguards:** smaller supporting-source levels, role-aware chorus/reverb, preserve MIDI timing and gate patterns, 12-note polyphony, output guard and existing factory master trims.
- **No new copyrighted audio:** reuse licensed CP59 WaveEdit CC0, synthesis resources and the existing Albedo source methods.
- **No FM source revival:** remain within the ten approved synthesis methods.

The generation script is `patches/cp65/retimbre_bioluminal.py`. It runs after CP59 source variation and CP60 FX balancing, changes >=25 real DSP parameters per patch, and records before/after parameter hashes for all 192. **An offline parameter generation pass is not a listening audition.**

### Renaming

Renames all indices **128–191** from morphological category patterns into 64 original, concise creative titles (e.g. *Velvet Mycelium*, *Renaissance Circuit*, *Blackwater Organ*, *Neon Fresco*, *Dawn Weaving*). Factory indices 0–127 retain names for continuity, though their sound parameters are revoiced.

### One bank without broken host sessions

- The **single visible sound bank** is `ALBEDO · BIOLUMINAL ATLAS` containing all 192 program slots.
- The palette preserves the existing `Factory`, `Creative` and `New` *internal program source keys* solely to maintain native Program ID dispatch and prior session compatibility.
- Top-level preset browsing remains Name × Family × Instrument Role; these are category filters, **not** three separate collections.
- User-imported patches and archives remain separate from the one factory bank.
- The browser integration runs `patches/cp65/unify_atlas.py` **after** the historical `patches/cp51/patch_atlas_catalog.py`; it does not replace or fork the authoritatively loaded HTML.

## Verification ledger

- **CP65 fast generation & catalog test:** passed in Windows-independent GitHub CI run `45729166342` on `aa29c1e6398bad4262ce8858eeca1035ee5bbfc3`; 192/192 revoiced, 64 names, single bank, legacy CP51/52 tests.
- **Windows CLAP/VST3 native compile, DSP safety and render-pair comparison:** follow `.github/workflows/albedo-cp51-morph-tree-windows.yml` on this branch. Do **not** claim verified until the actual run succeeds at the final source commit.
- **Target audio QA:** 192/192 audible, 17 native MIDI/DSP regressions, sample rates 44.1 / 48 / 88.2 / 96 kHz, stereo/mono compatibility, ASIO 64–1024 buffers, 18,336 preset-pair spectral/envelope comparisons, coherent loudness and oscillator balance.
- **User-in-host approval:** required before CP65 replaces the user-confirmed CP64R1 stable baseline.

## Source and delivery

- Source: repo `uill12/Plugins-VST-s`, CP65 branch and scripts.
- QA artifacts: `Albedo_CP65R1_Bioluminal_192_Presets_and_Browser`, and future native test/renders.
- Distribution: only completed and validated Windows x64 .clap/.vst3 artifacts may be promoted to Drive. Keep the CP64R1 release untouched.
