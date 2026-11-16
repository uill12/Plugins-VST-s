# Plugins-VST-s

Repositório de projetos de instrumentos e efeitos de áudio.

## Ankarana Percussive Atlas

**Current version:** `v0.8.1` (2026-11-14), Windows x64 CLAP (CP08R1) + VST3 (CP09).

- [Release details, verified Google Drive binary parts, and installation notes](Ankarana-Percussive-Atlas/README.md)
- [Binary-part manifest (Drive IDs and sizes)](Ankarana-Percussive-Atlas/RELEASE_MANIFEST.json)
- [Split-archive reconstruction helper](Ankarana-Percussive-Atlas/assemble_ankarana.py)

**Distribution:** the plug-in binaries are stored in eleven downloadable parts in the existing Ankarana Google Drive folder. GitHub tracks release metadata and tooling; the large binaries are not GitHub Release assets. Bitwig Windows playback is not yet host-certified.

## Rubedo Atlas Drumkit

- [Rubedo Atlas Drumkit — CP08 Hi-Fi (Windows x64, CLAP / VST3)](Rubedo-Atlas-Drumkit/README.md)
- [Relatório de áudio e limitações](Rubedo-Atlas-Drumkit/docs/CP08_HIFI_QA.md)
- [Código-fonte nativo](Rubedo-Atlas-Drumkit/source/)
- [Manifesto dos artefatos](Rubedo-Atlas-Drumkit/CP08_MANIFEST.json)

**Estado de distribuição:** o código e documentação são publicados aqui; os binários de ~378 MB **não** estão disponíveis como assets de GitHub Releases neste momento. Não confundir código exportado com uma release instalável. A GUI CP05/CP08 ainda não foi validada dentro do Bitwig Windows.

O projeto mantém 24 kits, 24 vozes por kit, 4 articulações, MIDI 36–59 e a identidade visual aprovada.

## Nereida IntraSynth

**Versão atual:** `v0.40.1-dev / CP-40` (Windows x64, CLAP e VST3).

- [Manifesto da versão, tamanhos e SHA-256](Nereida-IntraSynth/CP40_MANIFEST.json)
- [Histórico e estado de distribuição](Nereida-IntraSynth/README.md)

**Estado da entrega:** os dois pacotes binários foram verificados e enviados à pasta particular `Google Drive / PROJETOS / Nereida IntraSynth`. Este repositório contém o manifesto, **não os binários CLAP/VST3 nem um GitHub Release instalável**. Não alegar publicação dos executáveis no GitHub sem assets verificados.


## Nigredo Opus Septem

- [Nigredo Opus Septem — CP10 (Audio FX e MIDI Note FX, Windows x64, CLAP/VST3)](Nigredo-Opus-Septem/README.md)
- [Código-fonte nativo e interface RGNES](Nigredo-Opus-Septem/CP10/src/)
- [Relatório de testes e limitações](Nigredo-Opus-Septem/CP10/release/CP10_RGNEs_HOTFIX_QA.md)

**Distribuição:** binários CLAP e VST3 ficam na pasta Nigredo Opus Septem do Google Drive; este GitHub contém código-fonte, scripts e QA. CP10 foi compilado e testado offline, mas ainda não foi homologado no Bitwig Windows.
