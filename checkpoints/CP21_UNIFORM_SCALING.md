# Albedo Quintessential Synth — CP21 uniform UI scaling

Development candidate based on the user-confirmed stable CP20.

## Display and behavior

- Same 1265 × 900 startup window on the user's 15.6-inch / 1920 × 1080 display.
- Original 1920 × 1366 bitmap artwork retained; vector UI controls and high-resolution text are now drawn in final HWND pixel space, not embedded in a downsampled graphics bitmap.
- Knobs, sliders, cursor/gesture target coverage, pitch/FX toggles, pulse-grid numbers, source values, major section titles and group labels enlarged consistently.
- Preset Atlas dialog controls and fonts scaled by 1.12, with the same 64 factory indices and filtering logic.
- Original Segoe UI + Georgia font families, color palette, source identities and horizontal morphology preserved.
- This changes the UI only. No changes to DSP, voice scheduling, sample-rate handling, CLAP, VST3 MIDI, or preset parameter values.

## Artifacts and verification

- `CP21_GUI_SCALING.patch`: exact delta after CP20 source (not a binary).
- `Albedo_CP21_FHD_15_6in_Uniform_GUI.html`: editable companion visual reference; *not* a live WebView resource.
- `CP21_QA.json`: precise GUI source hashes and saved CP20 launch size.
- `Albedo-Quintessential-Synth/*`: modified native source files to help review and diff.

The source patch passed an offline Git apply + SHA-256 round-trip and HTML render checks.
Windows CLAP/VST3 compilation and Bitwig host tests are pending unless verified separately.
Keep CP20 unchanged and usable as a rollback release.