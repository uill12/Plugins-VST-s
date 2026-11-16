from pathlib import Path
from playwright.sync_api import sync_playwright
from PIL import Image
import json
R=Path(__file__).resolve().parents[1]
page_html=(R/'ui/Nigredo_CP07_Standalone.html').read_text(encoding='utf8')
meta=json.loads((R/'assets/scenes/meta.json').read_text(encoding='utf8'))
with sync_playwright() as p:
 browser=p.chromium.launch(headless=True,executable_path='/usr/bin/chromium',args=['--disable-dev-shm-usage','--no-sandbox'])
 page=browser.new_page(viewport={'width':1600,'height':1000},device_scale_factor=1,reduced_motion='reduce')
 errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
 page.set_content(page_html,wait_until='load');page.wait_for_timeout(200)
 assert page.evaluate('document.documentElement.scrollHeight') == 1000, 'Desktop canvas overflow'
 assert page.locator('.cosmogram').count()==1
 assert page.locator('.param').count() >= 9
 assert page.locator('#stagesList [data-select]').count()==7
 for kind in ('audio','midi'):
  if kind=='midi': page.locator('#midiMode').click()
  for stage in range(7):
   page.locator(f'#stagesList [data-select="{stage}"]').click()
   assert page.locator('#editorTitle').inner_text().strip()
   assert page.locator('.editor-panel .param').count()==9
   assert Image.open(R/'assets/scenes'/f'{kind}_{stage}.png').size==(1600,1000)
   for item in meta[kind]['stages'][str(stage)]['controls']:
    x,y,w,h=item['control']
    assert 0<=x and 0<=y and x+w<=1600 and y+h<=1000,(kind,stage,x,y,w,h)
  for ctrl in meta[kind]['hitboxes']['stage'] + meta[kind]['hitboxes']['toggle'] + meta[kind]['hitboxes']['path']:
   x,y,w,h=ctrl[1];assert x>=0 and y>=0 and x+w<=1600 and y+h<=1000,(kind,ctrl)
  assert all(0<=x<=1600 for r in meta[kind]['hitboxes']['master'] for x in [r['control'][0]])
 assert not errors,errors
 browser.close()
print('PASS: 1600x1000 full-frame desktop, 14 stage scenes, 126 parameter rects, all stage/toggle/path hitboxes, both HTML modes, no JS errors')
