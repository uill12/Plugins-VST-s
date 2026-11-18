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

- Source contract `tests/check_cp35_render_window.py` and JS syntax check
  execute in the Windows CI workflow.
- Windows x64 CLAP/VST3 build and package validation: see latest workflow
  attached to `albedo-cp35-render-window-match` branch.
- Bitwig live GPU presentation, actual resize grip and DAW sample-rate QA
  are **not** certified by CI and require host testing.

## Rollback

CP34 and CP33 binary artifacts are retained in their earlier branches/runs.
Do not overwrite the last known Bitwig-verified build with CP35 until its
runtime behaviour is checked.
