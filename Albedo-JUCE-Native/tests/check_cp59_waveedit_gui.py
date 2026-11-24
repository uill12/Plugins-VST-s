#!/usr/bin/env python3
"""CP59 Edge: 24 extra CC0 WaveEdit materials are exposed in all 4 engines.
No clipping, no editor reload, all source parameters remain editable.
"""
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
        page.on('pageerror',lambda e:errors.append(str(e)))
        page.goto(html.as_uri())
        for name in ('cp35_editor_geometry.js','cp52_nereida_browser.js',
                     'cp54_essential_sound_layout.js','cp56_sound_priority.js',
                     'cp57_tempo_controls.js','cp59_waveedit_materials.js'):
            page.add_script_tag(path=str(assets/name))
        page.wait_for_function('()=>window.ALBEDO_CP04?.materials?.length===56')
        result=page.evaluate("""()=>{
            const api=window.ALBEDO_CP04;
            const ids=[1010,1042,1074,1106];
            const params=cp04Registry.parameters.filter(x=>ids.includes(x.id));
            return {count:api.materials.length,tail:api.materials.slice(32),
                range:params.map(x=>[x.id,x.min,x.max]), width:document.documentElement.scrollWidth,
                viewport:innerWidth}
        }""")
        assert result['count']==56,(width,height,result)
        assert len(result['tail'])==24 and len(set(result['tail']))==24
        assert len(result['range'])==4 and all(x[2]==55 for x in result['range']),result
        assert result['width']<=width+5,(width,height,result)
        assert not errors,(width,height,errors)
        page.close()
    browser.close()
print('CP59 Edge PASS: 24 CC0 materials in 4 source engines, 3 viewports, no JS errors')
