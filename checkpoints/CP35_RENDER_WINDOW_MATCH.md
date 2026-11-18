# Albedo CP35 — Render matches host window, 3:2 resize & proportional Atlas

## Confirmed user-visible problem

The Bitwig screenshots from 18 November 2026 show the CP34 window at about
1100×770 screen pixels with CP22/CP31's 1320×880 logical artwork still drawn
at close to 1:1. This clipped the far-right oscillator/filter modules and
the lower modules. The Preset Atlas similarly extended beyond its viewport.
JUCE's built-in native corner resizer was obstructed by WebView2's native
child window (WebView2 HWND always appears above JUCE component overlays).

## Fix applied to CP35 source

1. **Host initial size:** 1320×880, exactly equal to the HTML canvas at 1×.
   A constrained 3:2 fallback is used only if primary-display userArea is
   too small after allowing for host chrome and Windows taskbar.
2. **Single source of truth:** JUCE sends the actual editor bounds (not the
   requested bounds) to WebView2 as `albedoViewport`. The web renderer
   applies the minimum of actual host W/H and browser viewport W/H when
   calculating visual scale, thereby handling mismatched WebView2 DPI reports.
   CP34's conflicting periodic fit script was removed from the built assets.
3. **Resize input that works above the browser:** a native HTML resize
   control at the lower-right of the WebView2 surface, with drag/compact and
   reset-to-original functions. This issues `resize|W|H` to JUCE. The
   corresponding `AudioProcessorEditor::setSize` is scheduled on the next
   message-loop turn, outside the WebView2 callback, and the host-granted
   dimensions are sent back to JS. JUCE host-resizability is still enabled.
4. **Atlas proportions:** viewport-synchronous backdrop centered on the
   actual host surface, with a maximum 1016×628 panel, approximately
   the golden ratio 1.618. The six category filters and 8×8 factory matrix
   stay intact; only results list scrolls inside the panel.
5. **No unnecessary renderer rebuild:** original CP22/CP31 DOM/SVG/Arbor,
   64 factory patches, audio DSP/MIDI and the incremental JUCE parameter
   bridge are preserved. No full-page reload or GDI/bitmap typography overlay.

## Verification gates

- CP35 source and JS syntax contracts passed in GitHub Actions.
- Headless Chromium was run on the exact exported HTML and JS resources:
  five viewport/host-size pairs passed including a deliberately mismatched
  browser viewport 1320×880 reporting host window 1152×768.
  Entire 1320×880 main page and the proportional Preset Atlas were contained.
- Resize controls emitted `resize|1224|816` and `resize|1320|880`.
  Arbor animation: 4 particles, independent of pointer movement.
  Atlas: 64 factory presets; category metadata loaded, modal open/close,
  no browser JavaScript exceptions.
- Windows CI run 44240548675 completed successfully, produced CLAP/VST3
  packages, and passed binary export tests. Artifact SHA-256:
  `e0d8eca7ce282bc46e16c2432988de9640e7b75b09d5bf1eb66a46a046fbc88f`.
  VST3 binary SHA-256:
  `d53254285d318cb0e029230b5206ce944f06a72ab065c907e4451613888cf228`.
  CLAP binary SHA-256:
  `94d6cad8af0a4ecaab36ab22bb7c416c16c9ef711cd0ed8792c317a4e3a9cd85`.
- Bitwig live WebView2 GPU presentation, actual host resize negotiation and
  48/88.2/96 kHz DAW sample-rate QA remain **pending**.

## Rollback

CP34 and CP33 binary artifacts are retained in their earlier branches/runs.
Do not overwrite the last known Bitwig-verified build with CP35 until its
runtime behaviour is checked.
