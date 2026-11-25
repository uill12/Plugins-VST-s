# Rubedo Atlas Drumkit — CP13R2 Live Knobs & Role-Aware Balance

**Version:** `0.9.10-cp13r2` • **Date:** 2026-11-25 • **Target:** Windows x64 CLAP for Bitwig Studio.

**Distribution:** [Google Drive — CP13R2 verified 13-part CLAP](https://drive.google.com/drive/folders/1UXMzU4HQmORz1zN7qogSVw9he4Rz7kOO)  
**CI:** [Windows LoadLibrary + Linux CLAP tests — run 45719839715](https://github.com/uill12/Plugins-VST-s/actions/runs/45719839715) — both jobs passed.  
**Rollback:** [CP13R1 CLAP](https://drive.google.com/drive/folders/1g_IOxx1FI3JZcEC5s7nPwlQk_toXJSCA), preserved and not overwritten.

## Root causes

- **Frozen knob positions:** The previous 1512×1009 artwork already contained permanently drawn knob needles, golden arcs and values. CP13R1 drew only tiny dots on top, so changing a DSP parameter left the principal visual pointer and numeric labels unchanged.
- **Uneven perceived drum balance:** The PCM24 source bank uses genuine different envelopes and loudness levels. A direct first-250ms RMS survey of 24 kits × 24 pads × four original variations found an average **15.7 dB** difference between the highest- and lowest-energy instrument roles per kit. This includes intentional kick/hat differences; equalizing all roles to one RMS target would destroy the kit hierarchy. The prior playback engine had no *kit-and-instrument specific* compensation.

## CP13R2 GUI changes

- Neutralized only the original fixed indicator/number portions of the artwork. The new raster is still the same 1512×1009 native GUI with original panel colors, controls and typography hierarchy.
- All **16 knobs** draw a live GDI pointer and progress arc using a precalculated 48-step angle table and the current engine value; no runtime trigonometric dependency.
- Real values replace the stale printed figures. Attack times are milliseconds, FM character is bipolar, output/gain are readable percentages; pitch displays playback-rate percentage instead of a false fixed Hz figure.
- The 35ms GUI timer checks the 16 live knob values and selected pad: state load, host automation and kit/pad selection trigger updates, as do GUI drag/wheel changes.
- Internal knobs notify the host state-dirty callback. Existing CLAP parameters, MIDI mapping, kit switching and state-v3 compatibility are retained.
- The GDI drawing adds no DSP sample latency; actual frame-exact MIDI rendering continues to pass at 44.1/48/88.2/96 kHz.

**Visual preview:** [new live-knob concept](https://drive.google.com/drive/folders/1UXMzU4HQmORz1zN7qogSVw9he4Rz7kOO) as `CP13R2_GUI_LiveKnobs_Preview.png`. The preview is a rendering of the corrected art and knob geometry, not a screenshot recorded from Bitwig; host GUI clickability still requires real Windows Bitwig validation.

## CP13R2 sound balance

- A deterministic **24×24 static gain-trim matrix** is derived from the original 2,304-sample, 96 kHz PCM24 source (first 250ms energy from each of four original articulations). Calibration respects subkick/fm-kick foundation, stronger snare/clap backbeats, lighter hats, carefully controlled long resonances/drone, and modest per-kit variations.
- Trim range **0.799× to 1.453×**, or approximately **−1.95 dB to +3.24 dB**. No per-hit loudness lookahead, sample alteration, new latency, spectral distortion or hard normalization.
- Mean first-250ms **source RMS after fixed trims** across kits: subkick `0.179→0.166`, snare `0.050→0.068`, clap `0.032→0.042`, drone `0.161→0.135`. These are PCM energy indicators, **not** LUFS or a guarantee of final subjective loudness across all contexts.
- Role-specific controls (`VOICE LEVEL` / pad `GAIN`) remain usable for intentional exceptions.
- Test `qa/test_role_balance.c` with the real PCM bank: 24 kits × 9 representative pad roles and 24 six-hit layered mixes generated nonzero finite audio, retained a kick/backbeat hierarchy and produced **zero samples above full scale**.

## QA and integrity

[GitHub Actions run 45719839715](https://github.com/uill12/Plugins-VST-s/actions/runs/45719839715) passed:

- Linux CLAP mock-host events, preset switching, sample-offset MIDI, GUI audition host wake, parameter events, v3 state save/load, independent instances and 0-algorithmic-sample latency report.
- Windows 2022 x64 PE core compile with new `MoveToEx`, `LineTo`, `SetTextAlign` GDI imports; `LoadLibraryW`, `GetProcAddress(clap_entry)` and `FreeLibrary` passed; error-127 `MessageBoxA` remains correctly imported from `USER32.dll`.
- The full distributable was assembled using **the exact Windows-CI-tested core bytes**; rebuilt 7-Zip was extracted with independent libarchive and the full CLAP hash matched.

| Resource | Bytes | SHA-256 |
|---|---:|---|
| **Complete Windows x64 CLAP** | 378,528,540 | `de9f656b357c1ca5a506a9d57763ae9118095ea19fe474c7915a92cf4cd6b203` |
| Windows LoadLibrary-tested core | 237,568 | `688c9e8413a47a39abcfff07c7ee5f3789c4b9afa95387c0648f2dd2554faf37` |
| Original PCM24 bank (unchanged) | 373,714,116 | `968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454` |
| Corrected 1512×1009 raw BGR skin | 4,576,824 | `fe7444bc6083223122e84e577c9f97039675b6af7a429e8f65451a6441fcb335` |
| Complete native 7-Zip archive | 378,530,020 | `93b81d3f91923ca4c2f382a2c4513206fa6aca67b9925febb9ba5fe36661622d` |

## Install

1. Close Bitwig. Back up `C:\Program Files\Common Files\CLAP\Rubedo_Atlas_Drumkit.clap` and keep the CP13R1 release folder intact.
2. Download **all 13 parts** `Rubedo_CP13R2_CLAP_LiveKnobs_RoleBalance_Windows_x64.7z.001` through `.013` into the **same directory**, then open `.001` in 7-Zip.
3. Extract and copy `Rubedo_Atlas_Drumkit.clap` to the CLAP directory, replacing the old file, then restart Bitwig and rescan plugins.
4. Compare the same MIDI note through two kits; drag each knob and confirm moving needle + number. Test automation/state reload, input pad control, 88.2 kHz, overlapping beats, pad mappings and GUI performance.

## Remaining validation

**Host QA still required:** this release has NOT been rendered/click-tested in the actual user's Windows Bitwig GUI. The GDI code is compiled and its API imports/loader pass on Windows, and the corrected visual preview renders with the same geometry, but the host's pointer response, font appearance and screen scaling are not yet physically tested. Perceived groove depth must be listened to in the target DAW and may benefit from user-directed final mixing. Do not treat this as an independently certified VST3 build; this checkpoint updates CLAP only.
