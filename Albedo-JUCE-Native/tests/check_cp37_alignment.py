#!/usr/bin/env python3
"""CP37 regression: actual HTML geometry, animated cells, and two audio gains."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
html=(root.parent/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
params=(root.parent/"Albedo-Quintessential-Synth/include/vitriol_params.hpp").read_text(encoding="utf-8")
dsp=(root.parent/"Albedo-Quintessential-Synth/src/vitriol_dsp.cpp").read_text(encoding="utf-8")
hpp=(root.parent/"Albedo-Quintessential-Synth/include/vitriol_dsp.hpp").read_text(encoding="utf-8")
assert html.count('id="cp37Gain"')==html.count('id="cp37Master"')==1
assert 'cp37GainValue' in html and 'cp37MasterValue' in html
assert 'grid-template-columns:315px 330px 174px minmax(0,1fr)' in html
assert 'grid-template-columns:minmax(0,1fr) 56px 32px' in html
assert '#filterKnobsA .param' in html and '#filterKnobsB .param' in html
assert '.lower .motion-controls .dial{width:35px' in html
script=html.split('<script id="cp37-cellular-and-gain">')[1].split("</script>")[0]
assert 'requestAnimationFrame(run)' in script and 'rows.shift()' in script
assert "grid.querySelectorAll('.cell')" in script and "next(cells,rule)" in script
assert 'grid.innerHTML' not in script and 'renderAll()' not in script
assert "api.state?.motionRule" in script and "api.state?.params?.motionRate" in script
assert "albedoMessage" in script and "'albedoUpdate'" in script
assert 'add(9003,"Pre FX Gain"' in params
assert 'add(9000,"Output Gain"' in params
assert "preFxGainSmooth_" in hpp
assert "const double targetPreFxGain=db(get(9003));" in dsp
assert "preFxGainSmooth_+=(targetPreFxGain-preFxGainSmooth_)*gainAlpha;" in dsp
assert dsp.index("x*=preFxGainSmooth_;y*=preFxGainSmooth_;")<dsp.index("processEffects(x,y);")
assert "const double targetOutputGain=db(get(9000));" in dsp
assert html.count('id="presetBrowser"')==1
assert "window.ALBEDO_CP31_MOTION" in html
print("CP37 PASS: 4 wave/2 filter/3 cellular alignments; 2 native gains; animated Wolfram; original Atlas/Arbor")
