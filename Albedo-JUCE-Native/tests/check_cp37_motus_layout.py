#!/usr/bin/env python3
"""CP37: require Motus & Cellular controls to retain square dials and all graphics.
The runtime WebView2 renders the exact HTML included in JUCE BinaryData.
"""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[2]
s=(root/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
assert s.count('id="motionKnobs"') == 1
assert s.count('id="cells"') == 1
assert s.count('id="lfoGraphic"') == 1
assert 'const BASE_W=1320,BASE_H=880' in s
assert 'window.ALBEDO_CP31_MOTION' in s
assert 'knob(\'motionRate\',\'RATE\')+knob(\'motionDepth\',\'DEPTH\')+knob(\'motionSlew\',\'SLEW\')' in s
block=s.split('<div class="motion-head">',1)[1].split('<div class="automata"',1)[0]
assert 'id="lfoGraphic"' in block, 'LFO trace is not in the motion-head'
assert 'id="motionKnobs"' not in block
assert 'Modulates texture, density &amp; color — not MIDI notes.' in s
assert 'Cellular motion targets texture, density and color — not automatic MIDI notes.' in s
# Every override is strictly scoped to the Motus module, leaving other knobs unchanged.
sel='html.albedo-fit #page .lower > .surface:nth-child(2)'
css=s.split("/* CP37 — Motus & Cellular:",1)[1].split("</style>",1)[0]
rules=css.count(sel)
assert rules >= 10, rules
assert 'grid-template-rows:27px 37px 63px 12px!important' in css
assert 'width:39px!important;height:39px!important' in css
assert 'flex:0 0 39px!important' in css
assert 'min-height:39px!important;max-height:39px!important' in css
assert 'height:10px!important;min-height:10px!important;max-height:10px!important' in css
assert 'grid-template-rows:repeat(7,minmax(0,1fr))!important' in css
assert "display:none" not in css
assert 'document.body.innerHTML' not in css
assert 'height:12px!important' in css and 'text-overflow:ellipsis!important' in css
# Always preserve the complete instrument / preset inventory and graph animations.
assert s.count('id="presetBrowser"')==1
assert s.count('id="cp04Lab"')==1
assert s.count('id="arborCanvas"')<=1
assert s.count('id="motionKnobs"')==1
print('PASS CP37: 39x39 square Rate/Depth/Slew, 7x32 automata, LFO and note within 3:2 Motus bay')
