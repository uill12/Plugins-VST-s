#!/usr/bin/env python3
"""CP57 Edge regression: real DAW/FREE clock and musical division in CP56 stage."""
from pathlib import Path
from playwright.sync_api import sync_playwright

root=Path(__file__).resolve().parents[2]
html=root/'ui/Albedo_CP30_CP22_Landscape_3x2.html'
assets=root/'Albedo-JUCE-Native/Assets'

with sync_playwright() as p:
    browser=p.chromium.launch(channel='msedge')
    for width,height in ((1230,820),(900,600),(720,480)):
        page=browser.new_page(viewport={'width':width,'height':height})
        errors=[]
        page.on('pageerror',lambda err:errors.append(str(err)))
        page.goto(html.as_uri())
        for name in ('cp35_editor_geometry.js','cp52_nereida_browser.js',
                     'cp54_essential_sound_layout.js','cp56_sound_priority.js',
                     'cp57_tempo_controls.js'):
            page.add_script_tag(path=str(assets/name))
        page.wait_for_function('()=>!!window.ALBEDO_CP57_TEMPO')
        assert page.locator('#cp57Clock').is_visible(),(width,height,'clock')
        assert page.locator('#cp57Rate').is_visible(),(width,height,'rate')
        assert page.locator('#cp57Clock').input_value()=='1',(width,height,'initial DAW')
        assert page.locator('#cp57Rate').input_value()=='0.25',(width,height,'initial 1/16')
        page.locator('#cp57Clock').select_option('0')
        assert page.evaluate('window.ALBEDO_CP04.state.cp04[6030]')==0,(width,height,'FREE')
        page.locator('#cp57Clock').select_option('1')
        assert page.evaluate('window.ALBEDO_CP04.state.cp04[6030]')==1,(width,height,'DAW')
        page.locator('#cp57Rate').select_option('0.5')
        assert abs(page.evaluate('window.ALBEDO_CP04.state.cp04[6021]')-0.5)<1e-6,(width,height,'1/8')
        page.locator('#cp57Rate').select_option('0.25')
        rect=page.evaluate("""()=>{
            const a=document.querySelector('#cp57TempoControls').getBoundingClientRect();
            const b=document.querySelector('#cp56Arp .cp56-caption').getBoundingClientRect();
            return {ok:a.left>=b.left-3&&a.right<=b.right+3&&a.top>=b.top-3&&a.bottom<=b.bottom+3};
        }""")
        assert rect['ok'],(width,height,'caption overrun',rect)
        assert not errors,(width,height,errors)
        page.close()
    browser.close()
print('CP57 Edge PASS: BPM source, musical divisions, true DSP parameter IDs, 3 viewport sizes')
