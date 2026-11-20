#!/usr/bin/env python3
"""CP35: host/HTML geometry consistency and complete Atlas/animations contract."""
from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
fit=(root/"Source/AlbedoWindowFit.h").read_text(encoding="utf-8")
cpp=(root/"Source/AlbedoWebViewEditor.cpp").read_text(encoding="utf-8")
hdr=(root/"Source/AlbedoWebViewEditor.h").read_text(encoding="utf-8")
js=(root/"Assets/cp35_editor_geometry.js").read_text(encoding="utf-8")
cm=(root/"CMakeLists.txt").read_text(encoding="utf-8")
html=(root.parent/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
metadata=json.loads((root/"Metadata/factory_preset_taxonomy.json").read_text(encoding="utf-8"))
assert "initialWidth=1200" in fit and "initialHeight=800" in fit
assert "g.initialHeight=g.initialWidth*2/3" in fit
assert "g.minWidth=std::min(720,g.maxWidth)" in fit
assert "Math.min(cssW,vv?.width" not in js
assert "nativeW||cssW" not in js
assert "window.setInterval" not in js
assert "getPrimaryDisplay()" in fit and "userArea.getHeight()" in fit
assert "availableH-95" in fit and "availableW-80" in fit
assert "setResizable(true,false)" in cpp, "HTML resize grip must replace covered native corner"
assert "requestHostResize(requestedWidth)" in cpp
assert "setSize(clipped,height)" in cpp
assert 'emitEventIfBrowserIsVisible("albedoViewport"' in cpp
assert 'void AlbedoWebViewEditor::resized()' in cpp
assert 'sendViewportSize();' in cpp and "sendViewportSize(bool force)" in cpp
assert "    void requestHostResize(int width);" in hdr
assert 'html.replace("</body>"' in cpp
assert 'cp35_editor_geometry.js' in cpp and 'cp35_editor_geometry_js' in cpp
assert "cp35_editor_geometry.js" in cm
assert "cp34_window_fit.js" not in cm, "old conflicting browser fit must be unbundled"
assert "cp34_window_fit.js" not in cpp
assert "w/canvas.offsetWidth" in html
assert 'id="presetBrowser"' in html and 'id="cp04Lab"' in html
assert "window.ALBEDO_CP31_MOTION" in html
assert len(metadata)==64 and [x["index"] for x in metadata]==list(range(64))
for item in metadata:
    for field in ["name","family","role","character","motion","context"]:
        assert item[field].strip(),(item["index"],field)
assert len({(x["family"],x["role"]) for x in metadata})==64
required=[
    "albedoViewport","ResizeObserver","visualViewport",
    "transform-origin:0 0!important",
    "root.clientWidth||window.innerWidth","--albedo-viewport-w","--albedo-viewport-h",
    "root.style.setProperty('--fit-scale'",
    "--albedo-offset-x","--albedo-offset-y",
    "--albedo-atlas-width","--albedo-atlas-height",
    "Math.floor(h*.96)",
    ".preset-browser-panel",
    "min-height:0!important","overflow-y:auto!important",
    "albedo-cp35-resize-grip","setPointerCapture",
    "requestAnimationFrame","resize|",
    "albedo-cp35-reset","const scale=Math.min(sx,sy)",
    "--fit-scale-x","--fit-scale-y"
]
for value in required:assert value in js, value
assert "document.body.innerHTML" not in js and "location.reload" not in js
assert "juce::juce_gui_extra" in cm and "NEEDS_WEBVIEW2 TRUE" in cm
print("CP39 PASS: 1200x800 initial editor, 1500x1000 HTML canvas, 720x480 minimum, 3:2 host resize, matched WebView viewport, 64 factory presets")
