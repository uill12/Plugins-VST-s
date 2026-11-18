# Plugins-VST-s

## Albedo Quintessential Synth — variants (Windows CLAP/VST3)

- **JUCE Native — No WebView2:** [source, metadata, build instructions](Albedo-JUCE-Native/README.md), [plugin identity manifest](Albedo-JUCE-Native/Metadata/PLUGIN_IDENTITY.json). GitHub branch: `albedo-juce-native-no-webview2`. This is the **C++-drawn native GUI** and uses unique VST3 / CLAP IDs. Artefatos previstos: `Albedo_JUCE_Native_NoWebView2_v1.0.32_Windows_x64_*`.
- **JUCE + WebView2:** previous CP33/CP34 branch `albedo-cp34-window-fit-3x2`, with a WebBrowserComponent rendering HTML and independent source history. **Do not name this variant NoWebView2.** This branch preserves its previous plugin identity and compiled artifacts.

**Build status:** native variant metadata changed on this branch. It is **not** considered Windows-verified or Bitwig-certified until the new CI job and host test are checked. Do not overwrite the previous functional release or mix the binaries from these two variants.


Repositório de projetos de instrumentos e efeitos de áudio.

## Rubedo Atlas Drumkit

- [Rubedo Atlas Drumkit — CP08 Hi-Fi (Windows x64, CLAP / VST3)](Rubedo-Atlas-Drumkit/README.md)
- [Relatório de áudio e limitações](Rubedo-Atlas-Drumkit/docs/CP08_HIFI_QA.md)
- [Código-fonte nativo](Rubedo-Atlas-Drumkit/source/)
- [Manifesto dos artefatos](Rubedo-Atlas-Drumkit/CP08_MANIFEST.json)

**Estado de distribuição:** o código e documentação são publicados aqui; os binários de ~378 MB **não** estão disponíveis como assets de GitHub Releases neste momento. Não confundir código exportado com uma release instalável. A GUI CP05/CP08 ainda não foi validada dentro do Bitwig Windows.

O projeto mantém 24 kits, 24 vozes por kit, 4 articulações, MIDI 36–59 e a identidade visual aprovada.
