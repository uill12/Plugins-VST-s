#!/usr/bin/env python3
"""CP34 JUCE/WebView2 3:2 screen-size and immutable source contract."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
src=root/"Source"
fit=(src/"AlbedoWindowFit.h").read_text(encoding="utf-8")
web=(src/"AlbedoWebViewEditor.cpp").read_text(encoding="utf-8")
native=(src/"AlbedoEditor.cpp").read_text(encoding="utf-8")
script=(root/"Assets/cp34_window_fit.js").read_text(encoding="utf-8")
cmake=(root/"CMakeLists.txt").read_text(encoding="utf-8")
html=(root.parent/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
assert "initialWidth=1152" in fit and "initialHeight=768" in fit
assert "getPrimaryDisplay()" in fit and "userArea.getHeight()" in fit
assert "availableH-130" in fit and "availableW-96" in fit
assert "maxHeight=g.maxWidth*2/3" in fit
assert "initialHeight=g.initialWidth*2/3" in fit
assert "getDisplays()" in fit
assert '#include "AlbedoWindowFit.h"' in web
assert '#include "AlbedoWindowFit.h"' in native
assert "aspect.setSizeLimits(fit.minWidth,fit.minHeight,fit.maxWidth,fit.maxHeight)" in web
assert "constrainer.setSizeLimits(fit.minWidth,fit.minHeight,fit.maxWidth,fit.maxHeight)" in native
assert "setSize(fit.initialWidth,fit.initialHeight)" in web
assert "setSize(fit.initialWidth,fit.initialHeight)" in native
for s in (web,native):
    assert "setSize(1440,960)" not in s and "setSize(1290,860)" not in s
for token in ["cp34_window_fit.js","cp34_window_fit_js"]:
    assert token in web or token in cmake,token
assert "const BASE_W=1320,BASE_H=880" in html
assert 'id="presetBrowser"' in html
assert 'id="cp04Lab"' in html
assert "window.ALBEDO_CP31_MOTION" in html
for token in ["ResizeObserver","visualViewport","--fit-scale","Math.min(1,w/BASE_W,h/BASE_H)",
              "addEventListener('resize'","window.setInterval","window.ALBEDO_CP34_SCREEN_FIT"]:
    assert token in script, token
assert "innerHTML=" not in script and "replaceChildren(" not in script
assert "juce::juce_gui_extra" in cmake and "cp34_window_fit.js" in cmake
assert "Albedo_CP30_CP22_Landscape_3x2.html" in cmake
print("PASS CP34: display-aware 1152x768 startup, 3:2 min/max, no bitmap reload, original CP31 interface")
