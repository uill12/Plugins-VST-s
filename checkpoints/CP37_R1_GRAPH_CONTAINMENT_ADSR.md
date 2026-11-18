# CP37 R1 — Albedo JUCE/WebView2 Graph Containment, Live ADSR

## User screenshot and correction

In the Bitwig screenshot on 2026-11-18, the source waveforms A/B/C/D
were outside their individual colored source module boundaries and not
vertically/visually aligned. There was no translucent, contextual ADSR
response popup while manipulating Attack/Decay/Sustain/Release.

### Root cause

CP31 styled `.source-card` with `height:100%` and nonzero padding,
while its source-row grid was aligned inside a smaller rack height.
The content-box default made the actual card height exceed the rack
and mispositioned the waveforms despite their equal SVG shapes.

### Implemented CP37 R1 UI fix

- `.sources` uses four `minmax(0,1fr)` columns and bounded rack dimensions.
- Each card uses `box-sizing:border-box`, `height:100%`,
  `max-height:100%`, `min-width:0`, `overflow:hidden`,
  explicit 22px heading + 26px waveform/control rows and internal
  4px/7px padding. This fixes the geometry, not just masks an overflow.
- Each waveform gets the width of its own first grid cell. Level slider and
  percentage have independent smaller cells; all are measured inside
  their own colored module. The actual waveform SVG remains live.
- Filters A/B share the same three-knob centerlines. Motus & Cellular
  knobs occupy evenly spaced three columns; the Rule 30/90/110/150
  32-cell animation continues via individual DOM cell updates.
- Gain ID9003 (pre-FX) and Master ID9000 (post-FX) are separate,
  smoothed host-controlled parameters in the available header region.
- ADSR contextual popup: a one-time-created SVG body-appended overlay,
  with translucent gradient/backdrop blur, curve, four phase markers and
  readouts. Opens on mouse, wheel or keyboard interaction, tracks real
  state values and updates SVG attributes only (no full-page re-render).
  Pointer-events are disabled so it never blocks knob interaction.
- Existing standalone Graph Detail popup, Arbor particle animation,
  Preset Atlas, CP36 double precision DSP and JUCE/WebView2 bridge retained.

## Browser regression evidence

Exact GitHub WebView2 source (not a reconstructed mock of design) tested
in Linux Chromium in viewport sizes 1320×880, 1152×768 and 1024×683:
**4/4** source cards, waveforms, sliders and numbers lie within the rack
and their individual card bounds. A/B filter knob Y positions match.
Cellular spacing remains equidistant on those three sizes and 900×600.
ADSR popup appears on interaction and fits entirely inside all three
tested viewports; wheel adjustment changes the SVG curve.
Cellular frames advance without mouse input. Zero browser JavaScript errors.
The actual screenshots/measurement JSON are in the distributed GUI QA ZIP.

## Build QA

CI workflow: `.github/workflows/albedo-cp33-webview2-windows.yml`
on `albedo-cp37-gui-grid-master-cellular`.
The workflow applies CP36 high-precision DSP and CP37 gain stages after
rebuilding the verified sources, checks source/audio/UI contracts and
compiles Windows JUCE CLAP/VST3. Source + HTML syntax CI passed.
Build/runtime and Bitwig-specific QA remain distinct.

RGNES lesson: never use 100%-height module cards with content-box and
nonzero padding inside a bounded flex/grid rack. Check child rectangles
against card and rack rectangles at multiple actual host viewports.
Transient graph overlays should update live SVG elements incrementally,
retain the original font/texture, have pointer-events:none, and keep
input focus on the controls they describe.
