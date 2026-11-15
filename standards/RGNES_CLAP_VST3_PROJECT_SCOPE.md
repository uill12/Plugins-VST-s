# RGNES — CLAP and VST3 project-wide application

Applied workflow standard: **RGNES v1.3.0** (private ChatGPT plugin `rgnes-gui-engineering`).
Last registry review: **2026-11-15**.

## Scope

This is a **shared engineering policy**, not a claim that any project binary has been rebuilt or changed.
Apply RGNES to every current and future DAW project that produces `.clap`, `.vst3`, or both:
instruments/synthesizers, drum machines, audio FX and MIDI FX. Include a new project automatically when it enters active development. Preserve its own aesthetics, parameter IDs, plugin identifiers, states and DSP.

### Project registry

| Project | CLAP | VST3 | Drive destination |
| --- | --- | --- | --- |
| Nereida IntraSynth / Tonal Architecture | In scope | In scope | [Nereida](https://drive.google.com/drive/folders/1lDh4-h7WfZeEAYaLSlc4X9jk5iVDLDv0) |
| Rubedo Atlas Drumkit | In scope; existing CP11 CLAP distribution verified in Drive | In scope when built; no VST3 distribution verified here | [Rubedo](https://drive.google.com/drive/folders/1wjIN7tcz-17lD6ozlhZpxKogvBZoctMR) |
| Albedo Quintessential Synth | In scope | In scope | [Albedo](https://drive.google.com/drive/folders/1nGl-deLK2UWgbE-gbs78T7Ivdfac0H42) |
| Nigredo Opus Septem / Alchemical Effects | In scope | In scope | [Nigredo](https://drive.google.com/drive/folders/1qBm90J-S_LL8e1vBZ-iUuJydeSnQdghF) |
| Ankarana Percussive Atlas | In scope where actively maintained | In scope where actively maintained | [Ankarana](https://drive.google.com/drive/folders/1ex26ucUkZWXvW35ZivrIXz9VL-43tBtK) |

**Note:** "In scope" means apply the standard upon edits/builds, not that a compiled artifact was inspected, uploaded, or tested in the host. Check each project's actual source and distribution before stating format support.

## Required procedure per project revision

1. Resolve the latest *actual* source/build and prior working rollback. Never infer the newest from filename alone.
2. Decide UI ownership: canonical HTML/CSS/SVG for approved design changes, native renderer for native/host issues, or both for foundational redesigns. Preserve approved visual identity.
3. Preserve CLAP plugin ID, VST3 class IDs, stable parameter IDs, preset/state serialization and backwards compatibility unless explicitly changing them through a migration.
4. Test each delivered format independently: CLAP entrypoint/descriptor, VST3 factory/bundle, GUI life cycle, state, automation, MIDI or audio events, rate/latency, audio quality, resize/DPI and Bitwig host behavior where test access exists. Distinguish compiled/offline/host/user confirmation.
5. Keep product `version`, `checkpoint`, ISO `build_date`, binary-reported version, `platform`, `formats`, hashes, QA evidence and distribution filenames aligned. Record rollback target; track RG NES skill version separately from product version.
6. **Automatically attempt to upload only new installable distributions produced directly during the active ChatGPT workflow** to their project Drive folder. Use the Drive connector only if the actual package has a supported file reference; an inaccessible Library file or a sandbox path alone is not sufficient. Do not claim autonomous background sync. Do not upload isolated source/HTML/QA unless explicitly asked.
7. After every volume is present and verified, highlight exactly one project's latest verified Drive checkpoint as `00_LATEST — <checkpoint>`. If transfer fails or is unavailable, retain the older verified latest marker and report `Drive upload pending`. Preserve earlier versions and rollback files.

## Verified existing release marker

- Rubedo Atlas Drumkit: [`00_LATEST — CP11_SkinSync_GUI`](https://drive.google.com/drive/folders/1F6tAkwEtkKyRgiSLVdE4gAd05eFAt3Ah), CLAP Windows x64 v0.9.6-cp11, existing 13-part `.7z` distribution in Drive.
- Host validation remains unverified based on the existing release notes. No additional projects were built or synchronized merely by creating this registry.

RGNES plugin: https://chatgpt.com/plugins/plugins_6af97c4f3ef881919ff81384b9fad5ea
