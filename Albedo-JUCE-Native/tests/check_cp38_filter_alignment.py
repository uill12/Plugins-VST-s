#!/usr/bin/env python3
"""CP38: non-destructive Filter A/B mirrored geometry and parameter ID QA."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[2]
s=(root/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
assert s.count('id="filterKnobsA"') == 1
assert s.count('id="filterKnobsB"') == 1
assert s.count('id="filterA"') == 1
assert s.count('id="filterB"') == 1
assert s.count('id="filterTypeA"') == 1
assert s.count('id="filterTypeB"') == 1
assert 'window.ALBEDO_CP31_MOTION' in s
assert "knob('filterAcut','CUTOFF'" in s and "knob('filterBcut','CUTOFF'" in s
assert "knob('filterAres','RESO'" in s and "knob('filterBres','RESO'" in s
assert "knob('filterAdrive','DRIVE'" in s and "knob('filterBdrive','DRIVE'" in s
style=s.split('/* CP38 — Corpus & Character / Filter A and Filter B mirror grid.',1)[1].split('</style>',1)[0]
assert "grid-template-rows:32px 45px 62px!important" in style
assert "grid-template-columns:repeat(3,minmax(0,1fr))!important" in style
assert "grid-template-rows:33px 12px 14px!important" in style
assert "width:33px!important;height:33px!important" in style
assert "min-height:33px!important;max-width:33px!important;max-height:33px!important" in style
assert "font-variant-numeric:tabular-nums!important" in style
assert style.count(".shape-block:nth-child(-n+2)")>=11
assert "document.body.innerHTML" not in style
assert "overflow:visible!important" in style
assert "grid-column:1!important;grid-row:1!important" in style
assert "grid-column:1!important;grid-row:2!important" in style
assert "grid-column:1!important;grid-row:3!important" in style
assert "/* CP37 — Motus & Cellular:" in s
assert s.count('id="motionKnobs"')==1
assert s.count('id="presetBrowser"')==1
print("PASS CP38 static: equal A/B rows, 33px square dials, identical three columns, no DSP changes")
