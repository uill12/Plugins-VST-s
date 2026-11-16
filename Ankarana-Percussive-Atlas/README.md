# Ankarana Percussive Atlas — Windows CLAP + VST3

**Current release:** 0.8.1 (release date: 2026-11-14; Drive sync: 2026-11-16)  
**Checkpoints:** CP08R1 (CLAP), CP09 (VST3)  
**Formats:** Windows x64 CLAP and VST3  
**Repository:** [Plugins-VST-s](https://github.com/uill12/Plugins-VST-s)

## Official binary distribution

**[Ankarana Percussive Atlas — Google Drive project folder](https://drive.google.com/drive/folders/1ex26ucUkZWXvW35ZivrIXz9VL-43tBtK)**

The complete instrument binaries exceed the connected upload service's 100 MiB per-file limit. Their existing verified split archives were therefore uploaded in numbered ZIP parts, **five for CLAP** and **six for VST3**. All 11 parts are present in the project folder. The original complete binaries have *not* been uploaded as GitHub Release assets; do not confuse this repository manifest with a published binary release.

Download every part for the desired format and use the corresponding assembly tool supplied with the original Ankarana checkpoint, or review the [assembly helper](assemble_ankarana.py) here (generic fallback; check the final file format before installing). **Do not install individual part ZIPs as plug-ins.**

### CLAP — 0.8.1 / CP08R1
- 24 kits, 24 MIDI pad roles; 16 hardware-pad performance page + 8 additional voices.
- Stable plug-in ID `com.ankarana.percussive-atlas`.
- Same 96 kHz / 24-bit synthesis bank as CP08; no changes to sound since 0.8.0.
- Install the reconstructed `.clap` at `C:\Program Files\Common Files\CLAP\`.

### VST3 — 0.8.1 / CP09
- VST3 adapter using the same synthesis bank and 24 kits.
- Exposes 58 host parameters, including 24 pad MIDI mappings.
- Install the extracted `.vst3` bundle at `C:\Program Files\Common Files\VST3\`.

See [release manifest](RELEASE_MANIFEST.json) for the exact eleven Google Drive file IDs, sizes, and part order.

### Validation boundary
Earlier checkpoint logs report Windows x64 compilation, CLAP/VST3 ABI simulation, saved mapping state, 576 preset/pad renders, and processing at 88.2 kHz. **Actual loading, playback and GUI interaction in Windows Bitwig have not been independently certified.** Drive transfer has been verified by listing the eleven files and sizes after upload; the retrieved, reassembled downloads have not been re-hashed after transfer.

## Project status
This new folder tracks the **Ankarana** branch only. It is not a rename or replacement of the separate Rubedo Atlas Drumkit project.
