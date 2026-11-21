# Albedo Quintessential Synth CP58 — complete source handoff

This portable development archive is for **continuing work**, not just installing the plugin.

**Sound/code baseline:** CP58 v1.0.58, commit \`3481912073e23cef753521e3a64cceafb19257ac\`.
**Windows CI:** https://github.com/uill12/Plugins-VST-s/actions/runs/44879814659 (72 steps success).
**GUI CI:** https://github.com/uill12/Plugins-VST-s/actions/runs/44879814690 (success).
**Next issue:** ASIO crackling at low buffer sizes; hardware Bitwig validation pending.
**Rollback:** CP57 branch \`albedo/cp57-daw-tempo-sync\`, commit \`1748a7e02650dd144e46274071231bebdb0c422f\`.

## Contents
- \`SourceRepo/\`: **all tracked project source files** at handoff packaging commit, including GUI assets, CMake, historical patches, tests, generator code, original source ZIP, and project checkpoints. Handoff commits do not change DSP or plugin behavior.
- \`ReconstructedDSP/\`: **actual patched CP58 native C++ DSP** from the successful Windows CI audit, not just scripts to regenerate it. The deterministic original 32-material PCM24 bank is generated and included in \`ReconstructedDSP/resources/\`.
- \`SelectedPresets/\`: verified CP51/CP58 192-program selection metadata and QA. These are the selected preset values, not candidates.
- \`Dependencies/\`: vendored JUCE 8.0.17-security source, CLAP JUCE extensions with CLAP SDK submodules, Microsoft WebView2 1.0.4258.31 NuGet SDK files and licenses. A companion \`DEPENDENCIES_LOCK.json\` records actual Git hashes.
- \`Binaries/CP58/\`: verified Windows x64 CLAP and VST3 and package manifests.
- \`QA/\`: low-buffer, 192-preset, comparison and source-audit QA.
- \`Build_Windows_CP58.ps1\`: rebuild from bundled source and dependencies, **without reconstructing CP13–CP58**.
- \`SHA256SUMS.txt\`: manifest of bundled files, verifiable with \`python verify_bundle.py\`.

## Quick Windows rebuild
Install Visual Studio 2022 with **Desktop Development with C++**, MSVC v143, Windows 10/11 SDK, CMake, Python 3.12, and optionally Git. Install Python requirements:
\`\`\`powershell
py -3.12 -m pip install -r requirements-build.txt
\`\`\`
Extract the archive (retain folder structure). Open PowerShell in its root:
\`\`\`powershell
py -3.12 verify_bundle.py
powershell -ExecutionPolicy Bypass -File .\Build_Windows_CP58.ps1
\`\`\`
Windows WebView2 Runtime is required to **display** the plugin UI. The build links to the bundled **WebView2 SDK** (a different component). Install an appropriate native ASIO driver to test within Bitwig Studio.

## Editing workflow
Edit the synth in \`SourceRepo/Albedo-JUCE-Native\` and the final DSP in \`ReconstructedDSP/\`. For permanent CI source changes, port DSP edits into source patch logic (\`SourceRepo/patches/\`) because the original CI **reconstructs** the DSP before building. Do **not** edit the CP13 source ZIP or assume it is the CP58 final DSP.
After edits, compile new checkpoint artifacts, compare sound and MIDI behavior, retain CP57/CP58 rollback, and sync verified releases to Drive.

## Validation and caveats
CI passed compilation, executable native tests, 20 buffer/sample-rate DSP checks and GUIs. It did **not** test Bitwig or user hardware ASIO. One 192-preset run had a worst-case estimated real-time ratio above 1 on the CI runner, which requires further profiling. Do not claim crackle-free hardware performance.
The repository and dependencies retain their respective original licenses. JUCE has particular license requirements for distributing closed-source audio plugins.
