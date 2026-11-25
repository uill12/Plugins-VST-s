# Rubedo Atlas Drumkit — CP13R1 Realtime/GUI diagnostic and repair

**2026-11-25 · v0.9.9-cp13r1 · Windows x64 CLAP**

**Status:** [Windows CI run 45716212321](https://github.com/uill12/Plugins-VST-s/actions/runs/45716212321) PASS (both Windows loader and Linux CLAP integration tests); local unit/PCM24/timing tests pass. Distributed full binary embeds the exact Windows-CI-tested core. **A live Bitwig Windows installation is still needed to measure MIDI-to-speaker end-to-end latency and exercise every graphical control.** No unsupported claim that the audio driver or Bitwig has zero latency.

## Root cause analysis

The originally embedded 2,304 PCM24 96-kHz regions have **no meaningful start silence**. Across all 2,304 regions, the attack crossed 5% of the region's initial peak with median ~0.03 ms and maximum ~0.31 ms in the first 250 ms of sampled material. The per-event CLAP audio rendering honors a note event's `time` sample offset within each block. In a real PCM bank mock host, note-on at frame 37 generates its first nonzero output **at frame 37** at 44.1/48/88.2/96 kHz, across 24 kits.

The GUI pad audition formerly enqueued a hit on the UI thread without calling the CLAP host's `request_process`. When the host idled, this could defer audio until it next scheduled the plugin. CP13R1 now requests audio processing after enqueueing an audition. This fixes an identifiable host-wakeup omission in the GUI path, but cannot eliminate latency caused by a host audio buffer, MIDI controller, external audio interface, or monitoring chain.

## GUI/UX and state audit

| Area | Prior defect | CP13R1 repair |
| --- | --- | --- |
| GUI pad audition | Audition enqueued while idle host could be asleep | `host->request_process()` issued on GUI hit; audio starts on the audio thread, not GUI thread |
| MIDI input → UI | Incoming MIDI forcibly changed selected editing pad and Core/Extended bank | MIDI updates live hit flashes only; selected editor target/page no longer jump |
| Control concurrency | Audio-thread reads could race UI-thread writes for global and per-pad controls | C11 atomic parameter/control fields for 34 global parameters and per-pad continuous controls |
| Non-automatable controls | Tempo, swing, per-pad synthesis, patterns, variations not always marked dirty in Bitwig | Request `on_main_thread` state-dirty callback even for internal controls; no fake CLAP parameter IDs 58/59/135 |
| DSP-reported latency | No explicit latency extension | CLAP `clap.latency` returns **0 algorithmic samples**; independent host/device buffering is outside this extension |
| On-screen control values | Raster labels were static; no accurate readout of current knob | Live knob hover readout using actual parameter values; attack time shown in milliseconds when applicable |
| Output meter | Previously only a baked static meter and AUDIO IDLE label | Live audio-thread output peak, decayed on 35 ms UI timer; status text follows recent peak |
| Symbolic modes/routing | Mode/force/routing outlines could disagree with host automation | Outlines conditional on current underlying TONE/WIDTH parameters, no false highlights |
| CLAP loader | Earlier Windows error 127 because `MessageBoxA` bound to KERNEL32 | Correct USER32 import retained; local PE import table inspected |
| Visual identity | Risk of losing original SkinSync artwork | Original 1512×1009 RGB raster SHA256 unchanged; all updates drawn via GDI overlay |

**Important limit:** This is a CLAP repair. The VST3 candidate from CP12R1 was not silently changed. Macro controls still share existing sound parameters rather than independent host-automatable parameters; the MIDI sequencer's internal BPM does not claim automatic Bitwig transport synchronization. Those features require separate design work, not a hidden behavior change in this latency patch.

## Tests performed

- PCM24 source bank: 2,304/2,304 attack regions measured; byte-identical bank copied into CP13R1.
- `qa/test_realtime.c`: 24 kits × 4 sample rates at nonzero sample offset, CLAP NOTE_ON, raw MIDI 0x90, queued GUI audition + host wake, independent instances, output 32/64-bit, state dirty callback, 0-sample latency extension.
- `qa/test_kit_presets.c`: CLAP factory/param events, failed output push retry, host automation, state v3 save/load and independent kit sound signatures using real PCM samples (full local mode).
- Native Windows x64 `clang` / `lld-link` cross-compilation, valid PE32+ with exported `clap_entry`, KERNEL32/USER32/GDI32/COMDLG32 imports.
- Original PCM24 bank SHA256 `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454`; source GUI bitmap SHA256 `a6db03fc0c28dcd269bc0750e2fddac1d03e5faa8d8055c834dc7f1b077df9bb`.
- Native 7-Zip COPY archive and all 13 volumes verified, **independent libarchive read-extraction** matched full CLAP hash.

## Distribution

The released artifact is a full-resource `Rubedo_Atlas_Drumkit.clap` for **`C:\Program Files\Common Files\CLAP\`**, not a source-only CI core. All 13 `Rubedo_CP13R1_CLAP_CI_WindowsVerified.7z.001`–`.013` parts must be downloaded into one folder; open `.001` in 7-Zip. Back up the previously working CP12R3 CLAP before replacing it. Verify the SHA256 against `VOLUMES_SHA256.json`.

**Official Windows CI gate:** https://github.com/uill12/Plugins-VST-s/actions/runs/45716212321 — Windows and Linux jobs passed.

**Exact Windows-smoke-tested core SHA256:** `c0d238a2948e571e488188e3499fca013fd105eac51f0c9f2e216365dac4d6b1`

**Installable full CLAP SHA256:** `18b6a370cacddbcc285c8d08a410857b2f168cf0f2f76aba87c6c181f9cf4d0e` (378,522,396 bytes)

**Archive SHA256:** `7ff2f30d1fcbaa1224e6b55afbad638c116b9a85353458c46eab5ce799e0c53f` (378,523,378 bytes)

**Google Drive:** https://drive.google.com/drive/folders/1g_IOxx1FI3JZcEC5s7nPwlQk_toXJSCA

**Original rollback CP12R3:** https://drive.google.com/drive/folders/1h-vMi0_MartSxMXvMoPKKZ_N1XYV7Odg

## Remaining on-host QA

1. Measure physical finger/controller → Bitwig MIDI event → interface audio; compare with other zero-latency instruments under the same ASIO driver, sample rate, block size, routing and monitoring chain. The CLAP test proves internal sample-accurate triggering but cannot measure the external roundtrip.
2. Validate 16-pad M-VAVE SMK-37 Elite mapping and pad flashes, editor selection stability under simultaneous input.
3. Validate all 16 knobs, macro force/mode selection, 24 kits/family browser, 24 note mappings, Save/Load `.rbmap`, wheel/drag, sequencer steps, patterns, tempo, transport and extended-page disabled pads. Check Bitwig parameter automation and state persistence.
4. Verify host re-scan/load, multinstance lifecycle, no cracking or stuck notes, 44.1/48/88.2/96 kHz, project reload and GUI consistency when Bitwig automates controls.

Never overwrite the CP12R3 rollback until real Bitwig validation passes.
