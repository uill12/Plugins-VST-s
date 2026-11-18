# Albedo Quintessential Synth — JUCE Native (No WebView2)

**Identidade desta variante:** `juce-native-no-webview2`. **Versão de projeto:** `1.0.32`.
Este ramo contém a GUI JUCE/C++ independente do navegador, e **não** a variante JUCE com HTML/WebView2.
A nomenclatura explícita identifica uma instalação separada no Bitwig e evita a colisão com os IDs anteriores.

| Metadado | JUCE Native (esta variante) | JUCE WebView2 (anterior) |
|---|---|---|
| GUI | JUCE 8 C++ nativo | JUCE + HTML/WebView2 |
| Dependência WebView2 | **Não** | **Sim** |
| Nome no DAW | Albedo Quintessential Synth - JUCE Native (No WebView2) | Albedo Quintessential Synth JUCE |
| VST3 `PLUGIN_CODE` | `AJNW` | `AQ32` |
| VST3 `BUNDLE_ID` | `org.vitriol.albedo.juce.native.noview2` | `org.vitriol.albedo.juce32` |
| CLAP `CLAP_ID` | `org.vitriol.albedo.juce.native.noview2` | `org.vitriol.opussonorum.juce32` |

**Arquivos identificáveis:** `Albedo_JUCE_Native_NoWebView2_v1.0.32_Windows_x64.clap` e bundle VST3 esperado `Albedo Quintessential Synth - JUCE Native (No WebView2).vst3`. O artefato de CI será `Albedo_JUCE_Native_NoWebView2_v1.0.32_Windows_x64_CLAP_VST3` **apenas após compilação bem-sucedida**.

A identificação é formalizada em [`Metadata/PLUGIN_IDENTITY.json`](Metadata/PLUGIN_IDENTITY.json) e validada por `tests/check_plugin_identity.py`.
**Atenção:** alterar os IDs cria uma instância de plugin diferente, deliberadamente. Sessões anteriores do Bitwig não migram automaticamente. O último binário verificado permanece preservado; esta revisão de identificação requer nova compilação Windows e QA no host.

**Fidelidade visual:** a interface nativa em C++ usa o HTML CP22/CP31 como referência, mas não o executa. Paridade de labels, efeitos, tipografia e microinterações continua exigindo comparação visual com o HTML de origem. Não alegar paridade pixel-a-pixel sem testes.

---

## Arquitetura e histórico — CP32 JUCE 8 Native Graphics Migration

This is an isolated native JUCE implementation, not an embedded HTML browser.
It uses the original CP22/CP31 landscape HTML as a design and behavior reference.
It draws interactive knobs, panels, module diagrams, parameter automation, MIDI,
and a searchable 64-preset Atlas as JUCE C++ components. The Arbor animation is
local to its graphic component and never redraws the whole editor on mouse motion.

## Rendering / platform
- JUCE 8.0.17-security, default Direct2D rendering on Windows; software renderer available as JUCE fallback
- Fixed 3:2 editor aspect ratio; launch 1440×960 and resize 960×640 to 1920×1280
- Preserved exact original Albedo logo SVG, no font files bundled
- No WebView2, Web2View, browsers, `WebBrowserComponent`, or HTML resources in runtime
- Original DSP source and original 32-material PCM resource, reused without changes
- All 64 factory patches and 197 parameter IDs, with an advanced control inspector
- VST3 via JUCE; CLAP via clap-juce-extensions (unofficial adapter)

## Status and caveats
- New wrapper and components are independently staged; not yet a replacement for verified CP31
- Metadata/parameter encoding differ from the legacy CLAP/VST3 and sessions should not be silently replaced
- Final audiovisual visual-parity and host performance must be tested in Windows Bitwig at 48/88.2/96 kHz
- Source integration and CI build reports, when available, determine deliverability

## Build

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
