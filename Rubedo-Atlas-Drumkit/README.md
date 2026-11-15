# Rubedo Atlas Drumkit — CP08 Hi-Fi

Windows x64 CLAP/VST3 drum instrument developer checkpoint (2026-11-14).

## Status

This directory contains the CP08 native C sources and audit documentation. **It is not a downloadable, complete Windows plugin release yet.** The full embedded CP07 sound bank (~378 MB as PCM24 in each DLL) and approved GUI raster resource are not committed to Git; publishing them needs a binary release or a separate asset-distribution channel.

- 24 kits × 24 voices × 4 articulations = 2,304 hits.
- Families: **Ethereal / Vitriolic / Scientific / Transmutation**.
- Nominal native-bank format: 96 kHz signed 24-bit PCM, original sound-bank FLAC 96 kHz/24-bit.
- MIDI note defaults: 36–59; 16 main pads + 8 extended voices.
- CP08 focus: band-limited resampling on high-step playback and stereo-linked soft limiter to reduce dense-hit hard clipping.
- Intended host: Bitwig Studio on Windows x64.

## Files

- [`source/`](source/) — real CP08 C source and ABI headers (CLAP/VST3), not pseudocode.
- [`generate_sinc_coeffs.py`](generate_sinc_coeffs.py) — regenerates the omitted 467 KB resampler table with NumPy (`python generate_sinc_coeffs.py`); output was checked byte-for-byte against the CP08 source table.
- [`docs/CP08_HIFI_QA.md`](docs/CP08_HIFI_QA.md) — offline test data, measurements, limitations.
- [`qa/CP08_Release_Manifest.json`](qa/CP08_Release_Manifest.json) — original recorded build manifest (some original ZIP checksums are superseded by later repackaging).
- [`qa/INSTALL_CP08.txt`](qa/INSTALL_CP08.txt) — installation instructions and known limitations.
- [`qa/audit_truepeaks_summary.json`](qa/audit_truepeaks_summary.json) — sample-bank true peak summary.

## Critical limitations

**GUI is not verified to be responsive inside Windows Bitwig.** It inherits the earlier CP05 editor. Offline Linux DSP tests and Windows x64 compilation/export checks do not certify actual Bitwig interoperability, saved state handling, or host timing. No independently confirmed Windows-in-host validation is claimed.

The source depends on the large *embedded* `rubedo_bank`, bitmap resources, and Windows toolchain import libraries. A repository checkout without these binary data components is not a standalone build. First run `pip install numpy` and `python generate_sinc_coeffs.py` to generate the native filter table. See `docs/BUILD_INPUTS.md`.

No audio from Max Cooper or other artists is copied: the timbres are original electronic interpretations of percussion behavior.