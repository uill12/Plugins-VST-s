from pathlib import Path
from playwright.sync_api import sync_playwright
import json
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
P=ROOT/'ui'/'Nigredo_CP10_Standalone.html'
OUT=ROOT/'assets'/'scenes'; OUT.mkdir(parents=True,exist_ok=True)
meta={}
with sync_playwright() as pw:
    browser=pw.chromium.launch(headless=True,executable_path='/usr/bin/chromium',args=['--disable-dev-shm-usage','--no-sandbox'])
    page=browser.new_page(viewport={'width':1600,'height':1000},device_scale_factor=1,reduced_motion='reduce')
    page.set_content(P.read_text(encoding='utf8'),wait_until='load')
    page.wait_for_timeout(150)
    style='''
    * {animation:none!important;transition:none!important;caret-color:transparent!important}
    
    .power-status, .toggle-text, #stagePower {color:transparent!important;text-shadow:none!important}
    #presetSelect {color:transparent!important;text-shadow:none!important}
    .power-thumb {visibility:hidden!important}
    .param-val {visibility:hidden!important}
    .knob:after {background-image:radial-gradient(circle at 32% 26%,#fff1cb,#caa66e 80%)!important;background-position:center!important;background-size:100%!important;transform:none!important}
    .knob:before {background:#e0c49a!important;}
    .param-fader {visibility:hidden!important}
    .param-choice {color:transparent!important;}
    .param-switch .switch-gem,.param-switch small {visibility:hidden!important}
    .param-switch.armed {background:linear-gradient(#ddd0ad,#c3a57c)!important;box-shadow:inset 0 2px 6px #74503066,0 2px 2px #fff2d7!important}
    '''
    page.add_style_tag(content=style)
    params=page.evaluate('({a:operations.map(x=>x.audio),m:operations.map(x=>x.midi)})')
    def rect(node):
        return node.evaluate('e=>{const r=e.getBoundingClientRect();return [r.x,r.y,r.width,r.height]}')
    for kind in ('audio','midi'):
        k=0 if kind=='audio' else 1
        specs=page.evaluate('kind=>operations.flatMap(x=>[...x[kind].macro,...x[kind].character,...x[kind].refine])',kind)
        masters=([{'def':100},{'def':0},{'def':4}] if kind=='audio' else [{'def':100},{'def':0},{'def':16}])
        p=[s['def'] for s in specs]+[1]*7+list(range(7))+[s['def'] for s in masters]+[0,3]
        for stage in range(7):
            p[81]=stage
            page.evaluate('arg=>window.NigredoHost.apply(arg)',{'t':'state','kind':kind,'p':p})
            page.wait_for_timeout(90)
            path=OUT/f'{kind}_{stage}.png'
            # Fixed viewport capture - backdrop stays consistent in native layout.
            page.screenshot(path=str(path),full_page=False,animations='disabled')
            # Exactly 1600x1000 screenshot
            entries=[]
            for j,n in enumerate(page.locator('#parameterFields .param').all() if page.locator('#parameterFields').count() else page.locator('.editor-panel .param').all()):
                inp=n.locator('[data-layer]')
                if not inp.count(): inp=n.locator('[data-kind]')
                spec=specs[stage*9+j] if j<9 else {}
                entries.append({'rect':rect(n),'control':rect(inp.first) if inp.count() else None,'value':rect(n.locator('.param-val')) if n.locator('.param-val').count() else None,'spec':spec})
            if stage==0:
                # Main hit boxes are consistent across stages, except for stage controls.
                selectors={
                    'stage':[[int(x.get_attribute('data-select')),rect(x)] for x in page.locator('[data-select]').all()],
                    'toggle':[[int(x.get_attribute('data-toggle')),rect(x)] for x in page.locator('[data-toggle]').all()],
                    'path':[[int(x.get_attribute('data-path')),rect(x)] for x in page.locator('[data-path]').all()],
                    'master':[{'rect':rect(x),'control':rect(x.locator('[data-context]')),'value':rect(x.locator('.param-val'))} for x in page.locator('.master-controls .param').all()],
                    'power':rect(page.locator('#powerBtn')),
                    'reset':rect(page.locator('#resetStage')),
                    'earlier':rect(page.locator('#moveEarlier')),
                    'later':rect(page.locator('#moveLater')),
                    'stagePower':rect(page.locator('#stagePower')),
                    'preset':rect(page.locator('#presetSelect')),
                    'apply':rect(page.locator('#applyPreset')),
                    'export':rect(page.locator('#savePreset')),
                    'import':rect(page.locator('#loadPreset')),
                    'mode':rect(page.locator('#audioMode' if kind=='audio' else '#midiMode')),
                }
                meta[kind]={'hitboxes':selectors,'stages':{}}
            meta[kind]['stages'][str(stage)]={'controls':entries}
            print(kind,stage,path.stat().st_size,'controls',len(entries))
    (OUT/'meta.json').write_text(json.dumps(meta,indent=2),encoding='utf8')
    browser.close()
