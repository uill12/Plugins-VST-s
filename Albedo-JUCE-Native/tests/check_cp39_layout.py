#!/usr/bin/env python3
"""CP39 non-rendering source contract: header containment, filter geometry and host scaling.
A Windows/Bitwig visual test is still required before release promotion.
"""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
html=(root.parent/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
geometry=(root/"Assets/cp35_editor_geometry.js").read_text(encoding="utf-8")
fit=(root/"Source/AlbedoWindowFit.h").read_text(encoding="utf-8")
editor=(root/"Source/AlbedoWebViewEditor.cpp").read_text(encoding="utf-8")
assert html.count('id="cp37Gain"')==html.count('id="cp37Master"')==1
assert html.count('id="filterKnobsA"')==html.count('id="filterKnobsB"')==1
assert '<style id="cp39-header-filter-fix">' in html
css=html.split('<style id="cp39-header-filter-fix">',1)[1].split('</style>',1)[0]
for text in (
    "header .cp37-level-strip label", "grid-template-rows:10px 18px 12px",
    "header .cp37-level-strip output", "height:12px!important",
    "#filterKnobsA .param", "#filterKnobsB .param",
    "grid-template-rows:34px 13px 14px",
    "justify-self:center!important",
):
    assert text in css, text
assert "w/canvas.offsetWidth" in html
assert "transform-origin:0 0!important" in geometry
assert "const scale=Math.min(sx,sy)" in geometry
assert "g.minWidth=std::min(720,g.maxWidth)" in fit
assert "g.initialHeight=g.initialWidth*2/3" in fit
assert "g.initialWidth=std::min(1080,g.maxWidth)" in fit
assert "ask(1200)" in geometry
assert "setResizable(true,false)" in editor
assert "requestHostResize(requestedWidth)" in editor
assert "safe->setSize(clipped,height)" in editor
assert "albedoViewport" in geometry and "resize|" in geometry
assert "document.body.innerHTML" not in geometry and "location.reload" not in geometry
print("CP39 source contract PASS: header, dual-filter grids, 1500x1000 canvas and 3:2 host requests")
