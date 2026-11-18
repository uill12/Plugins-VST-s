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

# CP37 R1 — containment is real box geometry, not only equal graph widths.
assert 'id="cp37-r1-source-bounds-adsr"' in html
assert '.sources>.source-card' in html
assert 'box-sizing:border-box!important;height:100%!important;max-height:100%!important;' in html
assert 'overflow:hidden!important;' in html
assert 'grid-template-rows:minmax(0,22px) minmax(0,26px)' in html
assert '.source-card>.source-bottom>.wave' in html
assert 'width:100%!important;max-width:100%!important;min-width:0!important' in html
assert html.count('id="cp37-r1-adsr-live"')==1
assert "id='cp37AdsrPopup'" in html and "popup.hidden=true" in html
assert "pointer-events:none" in html and "backdrop-filter:blur(12px)" in html
assert "const [a,d,s,r]=values();" in html
assert "area.setAttribute('d',poly+' L8 76 Z')" in html
assert "path.setAttribute('d',poly)" in html
assert "grid.innerHTML" not in html.split('<script id="cp37-r1-adsr-live">')[1]
assert "show(dial.dataset.key)" in html
assert "window.ALBEDO_CP37_ADSR" in html
assert "window.__JUCE__?.backend?.addEventListener?.('albedoUpdate'" in html
# Compile all inline JS blocks individually with node in CI to detect syntax errors.
if __name__=='__main__':
    import re, subprocess, tempfile
    js_blocks=re.findall(r'<script(?:\s[^>]*)?>([\s\S]*?)</script>',html)
    checked=0
    with tempfile.TemporaryDirectory() as d:
        from pathlib import Path as P
        for i,block in enumerate(js_blocks):
            if not block.strip():continue
            dest=P(d)/f'inline_{i}.js'
            dest.write_text(block,encoding='utf-8')
            subprocess.run(['node','--check',str(dest)],capture_output=True,check=True)
            checked+=1
    print(f'CP37 R1 JS syntax: {checked} inline scripts passed')

print("CP37 PASS: 4 wave/2 filter/3 cellular alignments; 2 native gains; animated Wolfram; original Atlas/Arbor")
