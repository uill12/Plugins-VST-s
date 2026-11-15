# RGNES — account-wide CLAP/VST3 routing and release policy

**RGNES:** v1.4.0 · **Reviewed:** 2026-11-15 · [Open private plugin](https://chatgpt.com/plugins/plugins_6af97c4f3ef881919ff81384b9fad5ea)

## Automatic application

Apply the RGNES engineering method when useful to all current/future .clap and .vst3 synthesizers, instruments, audio effects, MIDI effects, drum machines and build/distribution work. Preserve each project's established architecture, DSP, presets, host identifiers, UI identity, and rollbacks. Choose HTML design-source edits for visual layout changes and native GUI edits for native/host defects rather than forcing a redesign.

## Two destinations — do not mix them

| Destination | Store | Do **not** store |
|---|---|---|
| **GitHub** `uill12/Plugins-VST-s` | Actual source, build scripts, project checkpoints, test results, release manifests, engineering history | Runtime .clap/.vst3 binaries as ordinary Git blobs |
| **Google Drive** `PROJETOS/<project>/<checkpoint>` | Installable .clap/.vst3 files, platform-specific runtime-only ZIP or complete multipart .7z distributions; optionally essential minimal install info inside distribution | Source archives, build trees, HTML prototypes, screenshots, separate QA bundles |

Pre-existing mixed historical Drive folders remain intact; do not destructively reorganize or delete them automatically.

### Project Drive folders

| Project | Drive |
|---|---|
| Nereida IntraSynth | [Folder](https://drive.google.com/drive/folders/1lDh4-h7WfZeEAYaLSlc4X9jk5iVDLDv0) |
| Rubedo Atlas Drumkit | [Folder](https://drive.google.com/drive/folders/1wjIN7tcz-17lD6ozlhZpxKogvBZoctMR) |
| Albedo Quintessential Synth | [Folder](https://drive.google.com/drive/folders/1nGl-deLK2UWgbE-gbs78T7Ivdfac0H42) |
| Nigredo Opus Septem | [Folder](https://drive.google.com/drive/folders/1qBm90J-S_LL8e1vBZ-iUuJydeSnQdghF) |
| Ankarana Percussive Atlas | [Folder](https://drive.google.com/drive/folders/1ex26ucUkZWXvW35ZivrIXz9VL-43tBtK) |

## Latest-release detection and sync behavior

During active development and during the configured daily RGNES check:

1. Search accessible ChatGPT Library/project files, GitHub checkpoints, and existing Google Drive releases for candidate versions. Never assume a search result proves that binary bytes exist or are transferable. Distinguish source-only, prototype, compiled, offline QA, host QA and user-validated evidence.
2. Compare **product version**, **checkpoint/revision**, **build date**, **platform**, **CLAP/VST3 formats**, internal plugin version/IDs and content hashes (when known). Track RGNES skill version separately.
3. If a new runtime-only distribution is truly accessible through a connector-supported binary file reference, transfer it to the matching Google Drive folder and verify every byte/part; do not perform the upload merely because a file name was indexed.
4. Commit newly available project **source/build files** to GitHub through real write tools or an established CI workflow. Do not claim source was committed if no files were obtained. GitHub Actions may build and hand resulting runtimes directly to Drive **only when project-specific build recipes and valid Drive credentials are configured**.
5. Only after the full distribution is verified on Drive, rename the previous `00_LATEST` folder back to its checkpoint name and mark the newer release `00_LATEST — <checkpoint>`. Preserve rollback releases.
6. Report separately: latest release **identified in ChatGPT**, latest **source/build committed in GitHub**, latest **runtime distribution confirmed in Drive**, and any transfer blocked by missing bytes or connector access.

**Critical integration limit:** GitHub Actions cannot read the user's private ChatGPT Library. ChatGPT Library file IDs, Files search snippets and sandbox paths are **not** Google Drive `file_uri` handles. A supported bridge or a new build in an accessible execution environment is required. The daily watcher is a scheduled best-effort check, not guaranteed synchronization. Never invent transfers or report blocked work as completed.

## Verified current Rubedo release and rollback

- **Latest verified runtime on Drive:** [Rubedo CP12R1 Pad Map SaveFix — v0.9.7-cp12r1](https://drive.google.com/drive/folders/1IwAehMK6LZMmoJFXn8o5Dd0LYkIHMfmw), all 13 CLAP multipart volumes present in folder and source/QA recorded on [GitHub](https://github.com/uill12/Plugins-VST-s/tree/main/Rubedo-Atlas-Drumkit/checkpoints/CP12R1_Pad_Maps). Windows Bitwig user testing not yet confirmed.
- **Preserved prior release:** [Rubedo CP11 SkinSync](https://drive.google.com/drive/folders/1F6tAkwEtkKyRgiSLVdE4gAd05eFAt3Ah). Older releases and host-tested rollback states must remain untouched.
- Other projects' latest ChatGPT candidates need a supported byte-transfer route before any Drive upload may be claimed.

## QA and release manifest invariants

For each format and platform separately, verify descriptor IDs, exports/entrypoints, version strings, stable parameter IDs, saved state/presets and automation, MIDI/audio routing, sample-rate behavior and render/host evidence. Manifest should include project, semver, CP, ISO build date, source commit, OS/arch, formats, internal IDs/versions, SHA-256 for each distributed artifact, compilation/host QA status, exact Drive and GitHub URLs, and rollback reference.

RGNES standard files are in `standards/`; it is an account plugin method applied during work, **not** retroactive recompilation of existing binaries.
