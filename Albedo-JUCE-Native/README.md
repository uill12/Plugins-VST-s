# Albedo CP34 — JUCE 8 + WebView2 HTML (native fallback)

This CP34 branch uses JUCE 8 `WebBrowserComponent` / WebView2 to render the original Albedo HTML.
It includes the previous JUCE-native editor only as a fallback if WebView2 is unavailable.
**It is not the JUCE No-WebView2 variant.** The standalone C++-drawn implementation lives on the
[albedo-juce-native-no-webview2](https://github.com/uill12/Plugins-VST-s/tree/albedo-juce-native-no-webview2) branch,
with separate VST3 and CLAP IDs. Do not distribute CP34 files under a Native NoWebView2 name.

**Renderer:** `juce-webview2-html` · **WebView2 required for HTML GUI:** yes · **native fallback:** yes.
The source of truth is the [variant metadata](Metadata/PLUGIN_IDENTITY.json); the CMake target explicitly sets `NEEDS_WEBVIEW2 TRUE`.

## Rendering / platform
- JUCE 8 with WebView2 / Chromium HTML rendering on Windows; original JUCE C++ editor available as fallback
- Fixed 3:2 editor aspect ratio; launch 1440×960 and resize 960×640 to 1920×1280
- Preserved exact original Albedo logo SVG, no font files bundled
- WebView2 is **enabled** (`NEEDS_WEBVIEW2 TRUE`, `JUCE_WEB_BROWSER=1`) and the HTML UI is embedded. This is a WebView2 runtime variant.
- Original DSP source and original 32-material PCM resource, reused without changes
- All 64 factory patches and 197 parameter IDs, with an advanced control inspector
- VST3 via JUCE; CLAP via clap-juce-extensions (unofficial adapter)

## Status and caveats
- CP34 WebView2 host-fit integration is tracked separately from the native-only implementation. Bitwig host QA remains a separate verification step.
- This branch retains its existing WebView2 VST3 code `AQ32`, bundle ID `org.vitriol.albedo.juce32` and CLAP ID `org.vitriol.opussonorum.juce32`. Do not reuse these IDs for JUCE Native NoWebView2.
- Final audiovisual visual-parity and host performance must be tested in Windows Bitwig at 48/88.2/96 kHz
- Source integration and CI build reports, when available, determine deliverability

## Build

The CP34 WebView2 build additionally requires the original HTML asset and WebView2 Windows SDK.
From the repository root, after reconstructing verified Albedo CP31 sources into
`Albedo-Quintessential-Synth`, download a pinned JUCE 8 checkout under
`Albedo-JUCE-Native/ext/JUCE` and the recursively initialized
`clap-juce-extensions` repository under `Albedo-JUCE-Native/ext/clap-juce-extensions`.

```
cmake -S Albedo-JUCE-Native -B build-juce -A x64 -DALBEDO_DSP_ROOT=%CD%/Albedo-Quintessential-Synth
cmake --build build-juce --config Release --target AlbedoNative_VST3 AlbedoNative_CLAP
```

## CP32 — Complete factory preset taxonomy

Every existing factory ID (0–63) has an explicit, independently editable metadata record in
`Metadata/factory_preset_taxonomy.json`. The generated constexpr header
`Source/AlbedoPresetTaxonomy.hpp` is read directly by the native JUCE Preset Atlas.
Each preset now has a material family, instrument role, sound character, movement,
suggested musical context, and descriptive phrase. The musical context is editorial,
not a claim that the sound is limited to that genre. The 8×8 factory matrix remains
bijective: one preset per family/role intersection. Search, filter, sort, matrix selection,
patch loading and host-program synchronization preserve the original factory indices.
No values, names, preset audio, DSP, animation cadence, or MIDI behavior were changed.

To regenerate and verify (does not require installing JUCE):

```sh
python tools/generate_preset_taxonomy.py
python tests/check_preset_taxonomy.py
c++ -std=c++20 -I Source -I ../base/include tests/preset_taxonomy_compile.cpp -o taxonomy_test
./taxonomy_test
```
