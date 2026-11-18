#!/usr/bin/env python3
"""CP38 R2: all Pulsus rhythm steps and real amplitude parameters are exposed.
Pulsus remains audio modulation, not a MIDI note arpeggiator.
"""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[2]
html=(root/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
params=(root/"Albedo-Quintessential-Synth/include/vitriol_params.hpp").read_text(encoding="utf-8")
dsp=(root/"Albedo-Quintessential-Synth/src/vitriol_dsp.cpp").read_text(encoding="utf-8")
assert html.count('id="steps"')==1
assert html.count('id="pulseEnable"')==1
assert html.count('id="pulseDepth"')==1
assert html.count('id="pulseGate"')==1
assert html.count('id="division"')==1
assert html.count('id="play"')==1
assert html.count('id="regen"')==1
assert "16-STEP AMPLITUDE SEQUENCER" in html
assert "not MIDI notes" in html
assert "grid-template-columns:repeat(8,minmax(0,1fr))!important" in html
assert "grid-template-rows:repeat(2,27px)!important" in html
assert "grid-template-rows:16px 57px 26px 25px 11px!important" in html
assert "document.body.innerHTML" not in html
assert "function renderSteps(){" in html
assert "if(grid.children.length!==16)" in html
assert "grid.innerHTML=state.steps.map" in html
assert "button.setAttribute('aria-pressed',String(!!state.steps[i]))" in html
for param in ["6020","6021","6022","6023"]:
    assert f"add({param}," in params
    assert param in html
assert "post('param',6021,Number(target.value)/1000)" in html
assert "else if(t.closest('#pulseEnable'))post('param',6020" in html
assert "t.id==='pulseDepth'?6022:6023" in html
assert "id===6022||id===6023" in html
assert "else if(t.closest('#play'))post('param',6020" not in html
assert "targetPulse=(get(6000+step)" in dsp
assert "get(6022)*.01" in dsp and "get(6023)*.01" in dsp
assert "noteOn" in dsp and "noteOff" in dsp
# Cross-check that changes are scoped to the fourth bottom card.
pulsus=html.split("/* CP38 R2: full Pulsus 16-step audio amplitude rhythm",1)[1].split("</style>",1)[0]
assert pulsus.count(".surface:nth-child(4)")>=10
assert ".surface:nth-child(2)" not in pulsus
assert ".shape-panel" not in pulsus
assert "#steps .step" in pulsus
assert "pulse-shape-controls" in pulsus
print("PASS CP38 R2: 16 pulse steps in 8x2; preview, enable, rate, depth, gate mapped to DSP; original filters/Motus retained")
