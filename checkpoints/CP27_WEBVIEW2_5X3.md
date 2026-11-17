# Albedo Quintessential Synth — CP27 WebView2 5:3

**Build date:** 2026-11-17  
**State:** Windows CI compiled and verified; Bitwig host visual/interaction QA remains pending.  
**Source branch:** `albedo-cp27-html-webview2`  
**Windows CI:** https://github.com/uill12/Plugins-VST-s/actions/runs/44053940675

## Implementation

- RGNES CP26 5:3 HTML preserved as an embedded RCDATA Win32 asset.
- Albedo original logo and the 64 factory presets (8 families x 8 roles) retained.
- Native CLAP/VST3 parameter bridge connects HTML controls and preset commands to the existing C++ DSP.
- WebView2 hosted in the plugin editor with stable browser document; no full DOM recreation for idle pointer movement.
- CP23 native Win32 renderer retained as fallback if WebView2 cannot initialize.
- Nominal startup editor: 1440 x 864 (5:3). HTML logical design: 1650 x 990.
- The current audio engine and MIDI note lifecycle, DSP resource bank, and original parameter IDs remain unchanged.
- WebView2 SDK pinned to Microsoft.Web.WebView2 1.0.4258.31; linked using WebView2LoaderStatic.lib. WebView2 Runtime must be installed for the HTML editor to appear.

## Verified results

- CTest 28/28 passed.
- Official Steinberg VST3 validator 47/47 passed; 51/51 selftests passed.
- CLAP and VST3 are genuine Windows PE x64 binaries.
- Runtime ZIP CRC and all 6 per-file SHA-256 checks passed.
- Runtime ZIP SHA256: `1d71f56bcb3f3714ba50c7468939318b25fcc8808f93fa3a6f85299afa607bc8`.
- Runtime ZIP length: 45,919,971 bytes.

## Artifacts

- GitHub Actions: https://github.com/uill12/Plugins-VST-s/actions/runs/44053940675
- Google Drive: https://drive.google.com/file/d/1TTxaFITkij2MR--290Radq28B8s3hsdC/view?usp=drivesdk
- Drive folder: https://drive.google.com/drive/folders/11jmSnrSExplk_vqFfkntnkKftM0xQ5xF

## Known gaps

- Not yet host-certified in Bitwig; check GUI start/resize, Preset Atlas, sustained chords, all sample rates, automation/state restore, and idle mouse movement after loading.
- The **internal plugin version metadata / release manifest still uses legacy CP19R5** even though CP27 GUI code is installed. Do not describe it as internally version-tagged CP27 until the next metadata-safe rebuild.
- WebView2 initialization is asynchronous; GDI is a working fallback.
- Keep CP23 and prior binaries intact; do not overwrite host-validated releases.
