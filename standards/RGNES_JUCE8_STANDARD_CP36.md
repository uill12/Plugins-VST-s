# RGNES v2.3.0 — JUCE 8 standard for Windows DAW plugins

## Evidence from Albedo CP22–CP36

The user confirmed that **JUCE is the standard plugin engineering framework**.
Albedo CP33–CP35 JUCE/WebView2 binaries load visually in Bitwig user screenshots;
the same screenshots also exposed real defects: editor frame/render mismatch,
nonfunctional native resize grip under WebView2 HWND, clipped lower modules,
overwide preset panels, unnecessary visual filters and redraw on automation.
Therefore JUCE *integration/display* is user-observed; full resizing and Hi-Fi
sound/host stability are **not yet user-confirmed**.

## Default standard

1. **Use JUCE 8 and CMake for new or substantially rebuilt CLAP/VST3 DAW plugins**.
   VST3 uses JUCE; CLAP uses a pinned compatible `clap-juce-extensions` adapter
   with separately checked exports. Preserve each project's own identity and
   stable manufacturer/plugin/parameter IDs. Do not mass-migrate existing
   stable instruments without versioned, rollback-safe approval.
2. **GUI rendering is project-specific within JUCE.** Native JUCE
   `AudioProcessorEditor`/Direct2D is the default when the original is native
   or raster-sprite based. JUCE `WebBrowserComponent` with WebView2 is preferred
   when an approved dynamic HTML/CSS/SVG source is canonical and the user permits
   the extra dependency. Never force WebView2 on native-only/no-browser projects.
3. **One source of truth for geometry:** author a logical canvas and set the host
   editor initial size, aspect ratio, scaling, bounds and hitboxes consistently.
   Check *actual host-granted* dimensions, not just requested dimensions. If
   WebView2's child HWND covers JUCE's corner widget, provide an HTML resize
   control that requests a JUCE `setSize` on the message thread, and echo actual
   editor dimensions to JavaScript. Test Windows scaling at 100/125/150%.
4. **Keep dynamic elements live:** preserve original font texture/logo, SVG
   animations, colors, graphs and clickable elements. Animate local paths with
   RAF or a component timer. Never call `renderAll` or rebuild hundreds of DOM
   nodes for every host parameter update; use *incremental* ID/value deltas and
   full snapshots only on startup/preset swaps.
5. **Preset Atlas:** preserve factory indices, names and 8×8 matrix; search,
   family and role are sufficient visible filters unless others requested.
   Keep curatorial tags in metadata but avoid cluttering the GUI; give list
   rows enough height and use remaining panel area for the scroll list.
6. **DSP Hi-Fi:** no claim is certified by `double`, a richer library or
   more taps alone. Verify numeric precision, aliasing/stopband, 48/88.2/96k,
   12-note polyphony, stable note-off, gain/phase and 12-voice CPU against
   baseline. Oversampling/high-order interpolation trade off CPU and latency.
   Reuse embedded licensed PCM and actual JUCE DSP facilities when they
   improve measured quality, not for cosmetic complexity.
7. **Checkpoint discipline:** source-implemented, CMake/CI-compiled,
   offline-audio-tested, live-Bitwig-tested and user-confirmed are distinct.
   Preserve the last good runtime and the original patch bank; test Windows
   CLAP and VST3 independently and publish SHA-256.

## Regression requirements learned from Albedo

- Default 1920×1080 Windows laptop: no clipped rack, filter, effects or lower
  modules at host startup. Atlas completely inside 3:2 editor and readable.
- Host changes size -> WebView2 viewport updates -> page/Atlas bounds follow
  without refresh or mouse-induced redraw.
- Open and close Atlas; factory search, family, role, sorting, matrix and
  preset load remain working. No filter cell collapse.
- A single automation update must not reconstruct the full DOM; patch
  save/import/mutate actually reaches the native processor.
- Audio follows host rate 44.1/48/88.2/96 with 12 held keys and correct
  note-off; no stuck notes, clipping or excessive xrun risk.
- Keep QA evidence, screenshots and actual Bitwig host results separate
  from JavaScript/Chromium/C++ offline tests.

This is a repeatable engineering standard and test contract, **not** an
assertion that all other project builds have already adopted JUCE.
