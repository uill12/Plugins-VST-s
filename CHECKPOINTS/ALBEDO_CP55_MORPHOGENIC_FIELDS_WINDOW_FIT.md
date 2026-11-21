# Albedo Quintessential Synth — CP55 / Morphogenic Fields & Bitwig Window Fit

**Development branch:** `albedo/cp55-morphogenic-fields-viewport`  
**Based on:** CP54 source (Windows CLAP/VST3 CI-verified)  
**Plugin version:** `1.0.55` (CLAP/VST3 stable host IDs retained)  
**Status:** Source changes implemented, fast Microsoft Edge GUI QA passed. Full Windows CLAP/VST3 and compiled-host checks pending. Bitwig manual evaluation pending.

## Changes

### Morphogenic Fields — library name

Renamed the *displayed bank title*, formerly **New Sounds/New Bank**, to **Morphogenic Fields**. The name is grounded in the 64 existing sounds, grouped under Fractal Organisms, Velocity Without Drums, Subterranean Matter and Iridescent Systems, which yield Fractal, Kinetic, Subsoil and Prismatic instrument gestures (**Field** / **Motion**).

- Update the atlas tab and instrument dropdown optgroup through the canonical, deterministic `patches/cp51/patch_atlas_catalog.py` integration.
- Update the CP52 Nereida-style browser's category heading to `LIBRARY / MORPHOGENIC FIELDS`.
- **Do not change** the loader's internal collection key `New`, factory source key `NEW:128..191`, any preset names, parameter vectors, generated DSP/PCM data, or indices 128–191. The preceding Factory and Recursive Forms sets remain intact.
- Recheck 192 indexed sounds and 64 members in each collection.

### Bitwig window and HTML render

The user-provided Windows Bitwig screenshot shows the canonical 1500×1000 renderer visibly clipped inside a smaller plugin editor HWND (fourth source panel and right-hand Corpus, as well as lower FX zone). A layout must fit the *granted native viewport*, not be clipped from the outside.

- Default native editor now **1230×820** (3:2) when the available display area allows; on smaller screens it reduces proportionally within the original host constraints.
- CP35 browser geometry normally respects the actual root CSS viewport, but now detects and repairs a WebView2/embed anomaly when measured CSS root width or height over-reports the JUCE host dimensions by **more than 16%** and a consistent 3:2 native host rectangle is available.
- Do **not** always choose the smaller of native and CSS dimensions. Minor Windows DPI/host negotiation discrepancies remain handled by the verified CP40/CP52 path to avoid letterboxing.
- Existing responsive scale and aspect preservation, mouse-wheel knobs, seven contextual popups and full preset browser remain untouched. No page reload or audio-thread geometry work.

### RGNES — Tone Network Similar Permutation

RGNES Full DAW Engineer plugin updated to version **2.5.3** with `skills/tone-network-similar-permutation/SKILL.md`. This is a **development-time SOUND GENERATION graph**, not an interactive UI category or preset similarity filter. It can work forward from source/instrument/character/function, backwards from a target timbre, or by crossing two existing patches; only after generating candidate sounds does audition-based quality/diversity selection take place.

No change to the 192 already validated Albedo program parameter values is authorized by this label/UI checkpoint.

## QA and provenance

- [CP55 fast Edge GUI QA — PASS](https://github.com/uill12/Plugins-VST-s/actions/runs/44872791116): verifies displayed Morphogenic Fields, full three-library catalog and synthetic host/client viewport mismatch without clipping at 1080×720, 900×600 and 720×480; CP54 seven-popup regression unchanged.
- [CP55 full Windows CLAP/VST3 CI](https://github.com/uill12/Plugins-VST-s/actions/runs/44872852134): run the exact new source, CTest DSP/host, 192 preset checks and package strict VST3 metadata. **Do not promote until passed.**
- [CP54 final verified rollback build](https://github.com/uill12/Plugins-VST-s/actions/runs/44869174353) remains intact, including confirmed Windows CLAP/VST3 and MIDI/Hi-Fi DSP checks.

## Manual Bitwig acceptance remaining

Open both CLAP and VST3 in Bitwig Studio on Windows and verify screen-adaptive initial size, fourth source and entire Corpus, lower arpeggiator and effects controls all visible, seven popups in bounds, resizer/FIT behavior, source material/unison, full 192-preset browser, change/restore notes, sustained chords, transport stop, 88.2kHz and 48kHz audio. Only those tests can turn this from Windows-CI-verified into user-host-verified.

Keep CP51–CP54 as immutable rollback artifacts.
