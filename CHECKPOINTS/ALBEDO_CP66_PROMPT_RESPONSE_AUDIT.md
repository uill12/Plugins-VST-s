# Albedo CP66 — Prompt-to-response audit and rectification

Status: **EXPERIMENTAL SOURCE CHECKPOINT**. No verified CP66 CLAP/VST3, 192-preset
listening pass or Bitwig/ASIO4ALL approval is implied by these commits.
Stable rollback remains CP65R1 (user-reported stable core: CP64R1).

This document compares the user's *actual* recent requests against what was
committed. The previous assistant described several architectural intentions
as though they were finished audio results. Those claims are corrected below.

| User intent | Prior reply's implied completion | Reality / correction |
|---|---|---|
| Wolfram processing for all presets and functioning Nereida-inspired arp | Wolfram-enabled 192 bank | CP66 metadata now sets cellular rule and character for each of 192 presets; the CP58R4 DSP patch actually handles ID 6031. CP66 still needs live MIDI, host sync, idle-note and 88.2kHz tests. |
| Four engines contribute without one dominating | Generator levels balanced | Levels are uncalibrated percentages. Preset power flags were incorrectly left ON even when the engine level was zero. CP66 now sets source power flags coherently. Measured acoustic energy contribution and masking QA **remain unverified**. |
| Significant Concept Tree branching to unique instrument endpoints | 192 distinct audible timbres | 192 distinct provenance IDs/parameter configurations exist, **not verified 192 perceptually different sounds**. Distinctness needs rendered audio/MFCC+envelope and listening. |
| Conscious noise | Selectively placed stochastic source | Prior algorithm could allocate noise to a muted layer. CP66 now uses active C/D layers and a pitched primary for stochastic roots. Clipping/noise-floor tests remain undone. |
| Root material selected by sound type, spectral shape and role | Diverse sources | 24 previously license-reviewed WaveEdit tables selected from a catalog of 705, not 705 acquired sources and not independently rendered new materials. Root types now map to varied primary methods. |
| Simple and complex instruments, whichever sounds best | Four complexity types | Earlier code rigidly forced every fundamental source to stay minimal; removed. Minimal patches now power OFF unused generators and bypass their global saturation/space FX. |
| Prioritize good musical instruments over fidelity to source or maximal novelty | Quality-first presets complete | Candidate score is **parameter-only and uncalibrated**. CP66 now screens for plausible quality first, then considers structural novelty among near-equal proposals. Studio quality cannot be claimed without sound renders. |
| Add/remove/transform parameters where useful | Dynamic parameter system | Existing registered parameter values, source power, filters, modulation, FX enables are changed. **No new host parameter IDs were introduced or removed**; that would require coordinated DSP registration, host-state migration and GUI code. |
| GUI reflects source and plugin | Integrated WebView | HTML Material controls falsely advertised 0..31 while native DSP supports 0..55. Canonical HTML, idempotent GUI sync and a parameter-range audit now address this. No new GUI panels were designed. |
| Build Windows CLAP/VST3 | Build verified | CI workflow was edited to generate final JSON before native headers and the live HTML catalog. No CP66 run success/binary verification was observed. Output artifacts and version metadata are now labeled **CP66 Experimental 1.0.66** rather than CP65R1. |

## Executable checks now added

- `patches/cp66/audit_integrity.py` examines final 192 presets, intended
  1–4 active engines and actual ON flags, noise audibility, playable stochastic
  tonal roots, enabled FX semantics, CC0 source table presence, registered
  parameter bounds/step values, MIDI-polyphony safeguards and Wolfram selector.
- `patches/cp66/sync_ui_schema.py` protects four material selectors against
  native/HTML range drift and is safe on an already-correct source.
- Both the small Linux source-generation QA and the extended native Windows
  build invoke these checks (pending CI verification).
- Native CMake version and experimental artifact metadata were updated so
  CP65R1 is not accidentally replaced or mislabeled.

## Outstanding blockers — do not call CP66 stable

1. **CI execution**: obtain actual workflow run ID, logs and successful
   native build/artifact checks. A commit or green source assertion alone is
   insufficient.
2. **Actual candidate rendering**: produce WAVs from all 192 final presets,
   across representative registers and dynamics. Evaluate RMS/true peak,
   harmonic balance, transient, spectral diversity and noise levels. Compare
   root-to-leaf intent, leaf-to-leaf separability and the effect of each engine.
3. **Listening**: judge musical usefulness, body, clean vs complex sound,
   comparable perceived loudness, sustained notes and registration across
   octaves. Reject/regenerate weak timbres, not merely parameter duplicates.
4. **Safety/performance**: Bitwig MIDI note ownership, idle/resume stuck-note
   test with arp ON/OFF, ASIO4ALL short buffers/CPU/crackling, 44.1/48/88.2/96k
   and preset switching, plus GUl live sync/no redraw.
5. **Source breadth**: test whether 24 selected tables actually cover required
   tonal materials; broaden only with source-verified, legally acceptable audio.

**Promotion rule**: a quality-proxy score or successful compilation is not a
rendered sound-quality certificate. CP66 remains an experimental candidate
checkpoint until these gates are observed and documented.
