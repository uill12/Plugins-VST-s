# Albedo CP37 — Motus & Cellular fitted controls

Parent baseline: CP36 R1, HiFi DSP, JUCE 8 + WebView2, 3:2 canvas.
This is a **UI-only** alignment/containment change, without synthesizer/DSP
algorithm, factory preset, parameter or media modifications.

## What was wrong

Bitwig screenshot on 2026-11-18 showed the Motus & Cellular Rate/Depth/Slew
knobs squeezed vertically. The actual CP36 browser measurements confirmed
that at 1320×880 the knob `.dial` width was 29 px, but rendered height was
**19 px**, due to inherited `flex-shrink` and bottom-row constraints.
The LFO strip and note were outside the `bodypad` and visibly clipped.

## Changes

The canonical HTML `ui/Albedo_CP30_CP22_Landscape_3x2.html` now directly
contains the CP37 CSS correction:

- Restrict the correction to the second lower module only. Original
  1320×880 canvas, 3:2 resize, font files, artwork and knob interactions
  remain unchanged elsewhere.
- Convert internal `bodypad` to a measured 4-row grid:
  27 px heading/LFO, 37 px 7×32 automata grid, 63 px controls and 12 px
  note, with three 1 px row gaps, inside the existing 154 px body.
- Make all three `.dial` controls 39×39 square and
  `flex:0 0 39px`; give labels and values fixed non-shrinking heights.
- Keep the dynamically drawn SVG `#lfoGraphic` alive inside the motion
  header rather than cropped below the panel.
- Retain all 224 cells `#cells` and change the note to a concise line
  with the original full wording available in its tooltip.

## Measured Chromium QA

| Editor viewport | Dial width × height | All module content inside |
|---|---|---|
| 1320×880 | 39×39 | Yes |
| 1152×768 | 34.0×34.0 | Yes |
| 1080×720 | 31.9×31.9 | Yes |
| 900×600 | 26.6×26.6 | Yes |

No JavaScript exceptions in four scenarios. The 7×32 cellular state,
LFO SVG (two paths) and keyboard `ArrowUp` knob interaction remain working.
Standalone QA screenshots and preview are packaged separately.

## Windows build / rollback

GitHub branch `albedo-cp37-motus-cellular-layout`;
Windows workflow `.github/workflows/albedo-cp33-webview2-windows.yml`,
with `Albedo-JUCE-Native/tests/check_cp37_motus_layout.py` source contract.
JUCE plugin project version 1.0.37; Windows CLAP and VST3 built independently.

CP36 R1 remains the rollback until CP37 is observed inside Bitwig.
Windows CI success does not replace DAW host QA, 48/88.2/96k playback,
actual resizing and visual validation on the user's hardware.
