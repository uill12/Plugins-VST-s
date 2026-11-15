# Build inputs and publication constraints

The CP08 source snapshot is supplied for inspection and native integration. It does **not** include the complete CP07 sample bank or the GUI bitmap resource in this Git history, since the file payloads are too large for the current connected GitHub text-write workflow.

The previously built Windows x64 plugin contains 2,304 embedded PCM24 regions, plus the approved GUI artwork. A complete rebuild requires:

1. All C/ABI source files in `source/`, including `rubedo_sinc_coeffs.inc`.
2. The exact CP07 sample-bank data (24 kits × 24 pads × 4 hits, 96 kHz mono PCM24), checksummed SHA-256 `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454` as recorded for the source bank.
3. The approved CP07 GUI raster, SHA-256 `0714527cfd768349daa8698e4b5044a53b12117c76e43a8a1bc7ae3d082cea41`.
4. Native PE/COFF x64 toolchain and Windows import libraries; CLAP/VST3 entrypoint code is provided in `rubedo_clap.c` and `rubedo_vst3.c`.

**Warning:** the original CP08 ZIP manifest refers to packages later found to have extraction problems. Verify any downloaded artifact by its own current SHA-256 and open it in an independent ZIP extractor. This source tree is not a replacement for a downloadable full plugin binary.

The GitHub connector available here can commit UTF-8 text sources, but cannot upload the existing >300 MB plugin binary as a GitHub Releases asset. To publish a complete executable release, supply the binary using GitHub Releases in a browser or an authenticated `gh release upload` workflow outside this integration, then verify its hash.