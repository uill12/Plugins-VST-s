# Rubedo Atlas Drumkit — CP09 Control & MIDI Fix

Windows x64 CLAP developer checkpoint (CP09, 2026-11-15); VST3 source remains at its earlier experimental baseline.

## Status

This directory contains the current **CP09 CLAP native source** and archived CP08 audit documentation. The **installable CP09 CLAP binary** is stored in the connected Google Drive under `PROJETOS/Rubedo Atlas Drumkit/CP09_Control_MIDI_Fix`, as 13 standard multi-volume 7-Zip files (`.7z.001`–`.013`). All 13 volumes are verified on Drive. The 378 MB bank + approved GUI artwork remain embedded in the Drive-distributed binary, not committed to Git.

- 24 kits × 24 voices × 4 articulations = 2,304 hits.
- Families: **Ethereal / Vitriolic / Scientific / Transmutation**.
- Nominal native-bank format: 96 kHz signed 24-bit PCM, original sound-bank FLAC 96 kHz/24-bit.
- MIDI note defaults: 36–59; 16 main pads + 8 extended voices.
- CP08 focus: band-limited resampling on high-step playback and stereo-linked soft limiter to reduce dense-hit hard clipping.
- Intended host: Bitwig Studio on Windows x64.

## Files

- [`source/`](source/) — CP09 CLAP C source (with preserved ABI IDs); VST3 source is older and has not received the CP09 control fixes.
- [`docs/CP09_Control_MIDI_QA.md`](docs/CP09_Control_MIDI_QA.md) — current CP09 MIDI, input, compatibility and QA documentation.
- [`generate_sinc_coeffs.py`](generate_sinc_coeffs.py) — regenerates the omitted 467 KB resampler table with NumPy (`python generate_sinc_coeffs.py`); output was checked byte-for-byte against the CP08 source table.
- [`docs/CP08_HIFI_QA.md`](docs/CP08_HIFI_QA.md) — offline test data, measurements, limitations.
- [`qa/CP08_Release_Manifest.json`](qa/CP08_Release_Manifest.json) — original recorded build manifest (some original ZIP checksums are superseded by later repackaging).
- [`qa/INSTALL_CP08.txt`](qa/INSTALL_CP08.txt) — installation instructions and known limitations.
- [`qa/audit_truepeaks_summary.json`](qa/audit_truepeaks_summary.json) — sample-bank true peak summary.

## Critical limitations

**CP08R2 GUI PaintFix was reported working in Bitwig by the user. CP09 is newly built but has not yet been validated in Bitwig Windows.** Offline Linux control/DSP tests and Windows x64 compilation/export checks do not certify session restore, MIDI controller routing, automation or live host timing. Back up the working CP08R2 plugin before upgrading.

The source depends on the large *embedded* `rubedo_bank`, bitmap resources, and Windows toolchain import libraries. A repository checkout without these binary data components is not a standalone build. First run `pip install numpy` and `python generate_sinc_coeffs.py` to generate the native filter table. See `docs/BUILD_INPUTS.md`.

No audio from Max Cooper or other artists is copied: the timbres are original electronic interpretations of percussion behavior.