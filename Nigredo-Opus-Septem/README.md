# Nigredo — Opus Septem (CP10)

**Windows x64 · Audio FX + MIDI Note FX · CLAP and VST3**

## Current checkpoint

- CP10 (0.9.1-cp10), build dated 2026-11-15.
- Two independent effect modules: Audio FX and MIDI Note FX.
- Seven-stage horizontal alchemical GUI, RGNES reference-to-native workflow.
- Native Win32/GDI/GDI+ editor; **no WebView2 requirement**.
- CP10 resolves CP09 duplicate text overlays and excessive full-editor redraw by caching the manuscript scene and invalidating dynamic regions.
- Same stable plugin identities, 82 parameters per format, DSP and MIDI event engine as CP09.

## Downloads (verified Google Drive)

- [CLAP — Audio and MIDI FX](https://drive.google.com/file/d/1NWJqklI4x6JCu9LHm0sHvUlPbVuD3R2c/view)
- [VST3 — Audio and MIDI FX](https://drive.google.com/file/d/1_JLb1hNfE92HuSzWCZ-l102dV7xsZ-YR/view)
- [Latest runtime folder](https://drive.google.com/drive/folders/1abY1N1LYPjssziLypJUT54y2EaeaXEnp)

### ZIP SHA-256

- CLAP: `92ecfa5de87f15257322642494ff0a285ac67b9b98fa7fe18824052b72f579c2`
- VST3: `f25791dea2b97d886d13a172161822f18eb17d10e072b96c0839363b3b47c924`

## Repository content

The `CP10/` directory contains the **textual** source snapshot, HTML reference, build scripts, RGNES registry, QA tests and release metadata. The 14 large PNG GUI scene resources and Windows binary import libraries are not included in GitHub text-file commits; the complete editable ZIP retained in the original ChatGPT checkpoint is needed for a bit-for-bit rebuild. The packaged Windows plugins on Drive already embed the required artwork and are installable.

## Verification status

**Compiled and offline-tested, not yet Windows/Bitwig host-verified.** Smoke tests passed on Linux; all four Windows DLL-format plugin files cross-compiled and passed static entrypoint, resource and WebView2-dependency inspections. Actual Windows editor repaint behavior, automation feedback and host resizing require testing. See [CP10 RGNES QA](CP10/release/CP10_RGNEs_HOTFIX_QA.md).

Previous checkpoint CP09 is retained as rollback in the ChatGPT project artifacts; it is not replaced by this source commit.
