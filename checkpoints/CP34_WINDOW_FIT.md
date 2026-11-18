# Albedo CP34 — WebView2 3:2 screen-fit correction

## Symptom

Bitwig screenshot on 18 Nov 2026 shows the CP33 WebView2 editor too large
for the desktop: right-hand modules are cut and the lower panels fall below
the usable screen area.

## Applied changes

- The old JUCE WebView2 editor opened at 1440 × 960 logical pixels.
- CP34's preferred size is **1152 × 768**, a **20% reduction in each dimension**
  while preserving the **3:2** aspect ratio.
- At startup, `AlbedoWindowFit.h` checks the current primary-display
  `userArea` and constrains maximum/minimum editor sizes to leave
  horizontal and vertical margins for Bitwig chrome and the Windows taskbar.
- JUCE's `WebBrowserComponent` receives all available editor bounds, and the
  HTML's CP31 1320×880 design canvas is uniformly scaled and centered within
  the actual viewport. Its new `Assets/cp34_window_fit.js` safeguard
  caps scale at 100%, checks `visualViewport`, and handles host-native
  resizes even if no standard browser resize event fires.
- Accidental Ctrl+wheel/keyboard browser zoom is blocked without interfering
  with ordinary mouse-wheel knob behavior.
- The non-WebView JUCE fallback keeps its previously verified internal
  1260×840 minimum, and is uniformly transformed to fit the outer editor
  instead of compressing its filter/envelope controls into overlaps.
- DSP, MIDI, factory indexes, artwork and Preset Atlas remain unmodified.

## Browser tests

All test frames completely fit without overflow or JavaScript errors:
900×600, 1024×683, 1152×768, 1440×960, 1600×900.
Device-scale-factor tests at 100%, 125%, and 150% also passed.
At 1152×768 the page fills exactly, with all 64 factory patches and the
four Arbor animated signal particles still present.

## Windows build

GitHub Actions workflow: `.github/workflows/albedo-cp33-webview2-windows.yml`
in this CP34 branch, with dedicated `check_cp34_screen_fit.py`.
The current build status must be read from that workflow; this checkpoint
document does **not** declare Bitwig GPU/UI host validation complete.

## Release/rollback

CP34 is an isolated candidate. The last verified CP33 runtime is not
overwritten. Before stable promotion, open CP34 in Bitwig at 1920×1080,
test window opening, resize handles, both DAW plugin formats, overlay
controls, WebView2 zoom, preset Atlas and DSP at 48, 88.2 and 96 kHz.
