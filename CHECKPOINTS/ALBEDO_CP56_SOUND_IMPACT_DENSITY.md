# Albedo Quintessential Synth — CP56 / Sound-Impact Priority & Compact Density

**Branch:** `albedo/cp56-sound-priority-density`  
**Parent:** CP55 Morphogenic Fields / viewport fitting  
**Target:** Windows JUCE WebView2 CLAP + VST3; source checkpoint (CI and Bitwig host acceptance separately required).  
**DSP / preset compatibility:** unchanged host IDs; unchanged synthesis algorithms and factory values; 192 original factory program indices retained.

## Intent

Use the available stage surface for **audible changes first** rather than decorative whitespace. Keep main-page controls ordered by influence on tone and performance while preserving the seven CP54 contextual popups and original compact 3:2 host viewport scaling. Refinement controls share the remaining space in smaller rotary geometry.

## Primary stage — impact hierarchy

1. **Prima Materia — source identity:** four always-visible oscillator character knobs, method, material/unison selectors when appropriate, level/pitch/pan, and **four DSP-backed contextual refinements** for the active oscillator. Refinements prefer detune, spread, transient, fine tuning for applicable oscillators, additive partials/tilt for harmonic oscillators, and transient/sub/phase/fine pitch for other modes. Each rotary binds its original source offset within IDs 1000–1127, rather than introducing fake controls.
2. **Quintessentia — global timbral identity:** seven original sound macros on one row, reordered visually **Brilliance → Contour → Impact → Exchange → Width → Growth → Memory**. Existing dial nodes/handlers and persisted parameter values remain unchanged. Preserve XY morph and living arbor.
3. **Corpus — tonal body and articulation:** two filter types and cutoff/resonance/drive remain visible; **filter topology** parallel/serial is now on the main stage; all four amplitude ADSR knobs are continuously accessible. Add Filter A and B envelope amount plus filter envelope attack/decay (IDs 3120–3123) beside the ADSR. Remaining envelope parameters and voice behavior stay in the BODY popup.
4. **Pulsus — note articulation:** retain ARP power, 16-step gates and division, add compact live Gate, Cellular Accent, Swing, Octaves (IDs 6023,6022,6026,6025). Cellular rule and detailed motion remain available in the RHYTHM popup.
5. **Spatium — effects:** keep all four bypasses and MIX controls, and add immediately accessible Saturation Bias, Chorus Rate and Reverb Size (IDs 5012,5008,5011). Other effect properties remain in FX+ popup.

**Control hierarchy:** synthesis method and material/topology > character/morph/filter and envelope > global movement/coupling > detailed oscillator, rhythm and effects color > system. Priority is adapted by engine method; unused specialized controls are not exposed as inert stage knobs.

## Implementation contract

- New `Assets/cp56_sound_priority.js` runs after CP54, registers only real original DSP parameter IDs, updates a small dirty region instead of redrawing the page or rebuilding the HTML.
- `AlbedoWebViewEditor.cpp` serves and injects it; `CMakeLists.txt` includes it in the compiled binary resource. HTML and native plugin share one existing CP04/JUCE event bridge.
- Rotary controls support drag, wheel, keyboard and double click reset. Selected oscillator changes rebind parameter IDs without destabilizing the instrument. Body, ARP and FX controls follow factory preset loads. No audio-thread changes.
- The CP35/CP55 window/renderer fit, source rack and preset Atlas remain untouched.
- The Tone Network Similar Permutation algorithm remains a **development-time sound generation method**, not a new GUI preset filter.

## Acceptance

- `node --check Albedo-JUCE-Native/Assets/cp56_sound_priority.js`
- `python Albedo-JUCE-Native/tests/check_cp56_sound_priority.py` on Windows Edge, with the verified CP51 expanded catalog.
- Legacy CP54 popups, CP55 viewport fit and 192 factory preset list regressions.
- Manual Bitwig CLAP/VST3 3:2 scaling and audible parameter A/B, presets, chords/release, DAW sync and 44.1/48/88.2/96 kHz still required.
- Compiled Windows artifacts must not replace CP55/CP54 rollback until exact-source CI, audio and host tests pass.

**Status:** source committed on CP56 branch; Edge CI was triggered separately. Do not label binary/host QA passed without workflow evidence.
