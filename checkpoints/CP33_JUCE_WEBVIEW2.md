# Albedo Quintessential Synth — CP33 JUCE 8 + WebView2

## Purpose

The approved CP22 landscape GUI (readability/Arbor fixes consolidated in CP31) is
once again the primary view. JUCE 8 is the plug-in framework; Windows' WebView2 is
the GPU-backed HTML/CSS/SVG/JavaScript renderer. **WebView2 is a graphical surface
only**. All MIDI, voices, DSP, sample-rate handling, factory programs, DAW state,
and parameter automation are handled by the original C++ synthesizer via JUCE.

## Geometry, identity and animations

- Native editor aspect ratio 3:2; start 1440×960; resize 990×660 to 1920×1280.
- Source HTML and its original Albedo SVG logo are retained unmodified at
  `ui/Albedo_CP30_CP22_Landscape_3x2.html` (CP31 visual checkpoint).
- Live Arbor signal particles continue on RAF independently of pointer movement.
- Source waves, filters, XY controls, enlarged graphs, cellular patterns, FX,
  dynamic texts and popups remain real HTML elements, not captured images.
- No full-screen DOM re-render or page reload when parameters change.

## Native bridge

- JUCE `WebBrowserComponent::Options::Backend::webview2`,
  `withNativeIntegrationEnabled`, `withEventListener`, and
  `withResourceProvider`, backed by JUCE `BinaryData`.
- On resource load, a single exact fragment in the historical CP27 bridge
  redirects its `postMessage` into `window.__JUCE__.backend.emitEvent`.
  The underlying editor markup, layout and animations stay untouched.
- `ready`, `begin`, `param`, `end`, and `preset` messages map to existing
  JUCE host parameters, gestures and 64 factory program IDs.
- Host automation is returned as deltas at 12 Hz, with a complete initial
  snapshot; no host-thread waiting or audio-thread UI calls.
- Historical program IDs and the CP28 12-voice polyphony rule remain unchanged.

## Factory Atlas

The 64 original factory timbres retain all names and indexes. The previous
8×8 family/role matrix is augmented in the same WebView2 HTML with the complete
CP32 metadata: tonal character, movement/articulation and suggested musical
context. Filtering does not mutate audio parameters or program numbering.

## Safety and rollback

The JUCE-native CP32 editor remains compiled. When WebView2 is not available
it is opened directly; if the resource provider fails or the JS bridge has
not signaled ready by the startup watchdog, it is displayed as a fallback.
CP31 and CP32 branches are not overwritten. Native WebView2 host compositing,
GPU-black-screen regression, GUI interactions and CPU at 48/88.2/96 kHz must
be verified in Bitwig before this checkpoint is promoted to a stable release.

## Build

The Windows workflow `.github/workflows/albedo-cp33-webview2-windows.yml`
regenerates the authenticated DSP bank, imports the verified CP31 sources,
runs the 64-preset/GUI contracts, installs the pinned Microsoft WebView2 SDK
and builds VST3 and CLAP via JUCE 8 and clap-juce-extensions.

## Source of API design

JUCE 8 Feature Overview: WebView UIs — https://juce.com/blog/juce-8-feature-overview-webview-uis/

## Status

CP33 source integrated. Windows build, Steinberg validator and Bitwig host
verification are separate release gates; see CI for current results.
