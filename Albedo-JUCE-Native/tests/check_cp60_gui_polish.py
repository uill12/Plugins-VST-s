#!/usr/bin/env python3
"""CP59 Edge regression: no clipped/overlapping text in the audited regions, readable captions, usable hit targets."""
from pathlib import Path
from playwright.sync_api import sync_playwright

root=Path(__file__).resolve().parents[2]
html=root/'ui/Albedo_CP30_CP22_Landscape_3x2.html'
assets=root/'Albedo-JUCE-Native/Assets'
SCRIPTS=('cp35_editor_geometry.js','cp52_nereida_browser.js','cp54_essential_sound_layout.js','cp56_sound_priority.js','cp57_tempo_controls.js')

AUDIT="""()=>{
 const out=[];
 const box=s=>{const e=document.querySelector(s);return e?e.getBoundingClientRect():null};
 const clipped=e=>e.scrollWidth>e.clientWidth+1;
 // 1. single-line captions must not be ellipsised or cut
 for(const s of ['.field-notes .visual-card p','.field-notes .visual-card .micro','.shape-head .section-caption','#editorTitle','#editorStatus'])
   document.querySelectorAll(s).forEach(e=>{if(clipped(e))out.push('clipped '+s+' "'+e.textContent.trim()+'"')});
 // 2. both filter selectors show their selected text in full
 for(const id of ['filterTypeA','filterTypeB']){const e=document.getElementById(id),c=document.createElement('canvas').getContext('2d'),cs=getComputedStyle(e);
   c.font=cs.fontSize+' '+cs.fontFamily;if(c.measureText(e.options[e.selectedIndex].text).width>e.clientWidth-14)out.push('select clipped '+id)}
 // 3. Source Inspector header: title sits below its eyebrow, not beside it, and status stays on one line
 const eb=box('.inspector-head .eyebrow'),tt=box('#editorTitle'),st=box('#editorStatus');
 if(!(tt.top>=eb.bottom-1))out.push('inspector title overlaps eyebrow');
 if(st.height>14*1.6)out.push('inspector status wraps');
 // 4. XY read-out must not overlap the decorative flow label
 const coords=box('.morph-panel .coords');
 document.querySelectorAll('.network svg text').forEach(t=>{const r=t.getBoundingClientRect(),v=getComputedStyle(t).display!=='none';
   if(v&&r.width&&coords&&r.left<coords.right&&r.right>coords.left&&r.top<coords.bottom&&r.bottom>coords.top)out.push('svg label overlaps coords: '+t.textContent)});
 // 5. hit targets
 for(const id of ['theme','help']){const w=document.getElementById(id).offsetWidth;if(w<24)out.push(id+' too narrow (layout px) '+w)}
 return out;}"""

with sync_playwright() as p:
    browser=p.chromium.launch(channel='msedge')
    for width,height in ((1230,820),(900,600),(720,480)):
        page=browser.new_page(viewport={'width':width,'height':height})
        errors=[]
        page.on('pageerror',lambda err:errors.append(str(err)))
        page.goto(html.as_uri())
        for name in SCRIPTS:page.add_script_tag(path=str(assets/name))
        page.wait_for_function('()=>!!window.ALBEDO_CP57_TEMPO')
        page.wait_for_timeout(300)
        for theme in range(2):
            problems=page.evaluate(AUDIT)
            assert not problems,(width,height,'dark' if theme else 'light',problems)
            if not theme:page.locator('#theme').click();page.wait_for_timeout(200)
        assert not errors,(width,height,errors)
        page.close()
    browser.close()
print('CP59 GUI polish PASS: clipping, overlap, header, hit targets; 3 viewports × light/dark')