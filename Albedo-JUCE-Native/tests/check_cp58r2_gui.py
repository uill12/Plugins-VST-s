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
        for mode,count in [('1',6),('0',7),('1',6)]:
            page.locator('#cp57Clock').select_option(mode)
            assert page.locator('#cp56Arp [data-cp56-id="6029"]').is_visible()==(mode=='0')
            tiles=page.locator('#cp56Arp .cp56-param:visible')
            assert tiles.count()==count
            previous=None
            for tile in tiles.all():
                dial,label,value=[tile.locator(sel).bounding_box() for sel in ('.cp56-rotary','.cp56-label','output')]
                center=dial['x']+dial['width']/2
                for box in (label,value):
                    assert abs(box['x']+box['width']/2-center)<2
                if previous:
                    assert dial['x']>previous['x']+previous['width'] and abs(dial['y']-previous['y'])<1
                previous=dial
        page.locator('#pulsusMeter').select_option('4')
        assert page.evaluate('window.ALBEDO_CP04.state.cp04[6032]')==4
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
        assert page.locator('#steps').is_visible()
        for choice in range(12):
            page.locator('#cpArpPattern').select_option(str(choice))
            assert page.evaluate('window.ALBEDO_CP04.state.cp04[6031]')==max(0,choice-3)
            if choice<4:
                assert page.evaluate('window.ALBEDO_CP04.state.cp04[6024]')==choice
        page.evaluate("document.querySelector('#presetBrowser').hidden=false")
        page.wait_for_selector('#presetBrowser:popover-open')
        box=page.locator('#presetBrowser .preset-browser-panel').bounding_box()
        assert box['x']>=-1 and box['y']>=-1 and box['x']+box['width']<=width+1 and box['y']+box['height']<=height+1,box
        for query in ('silk','zz-no-results',''):
            page.locator('#pbSearch').fill(query)
            page.wait_for_timeout(60)
            assert page.locator('#presetBrowser .preset-browser-panel').bounding_box()==box
        for tab in ('New','Creative','Factory'):
            item=page.locator('[data-collection="'+tab+'"]')
            if item.count():item.click()
            assert page.locator('#presetBrowser .preset-browser-panel').bounding_box()==box
        page.close()
    browser.close()
print('CP57 Edge PASS: BPM source, musical divisions, true DSP parameter IDs, 3 viewport sizes')
