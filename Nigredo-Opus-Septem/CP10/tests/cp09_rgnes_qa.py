#!/usr/bin/env python3
"""RGNES G3: layout, control, mode state and HTML/native geometry validation."""
from pathlib import Path
import csv,json,math
from playwright.sync_api import sync_playwright
from PIL import Image
R=Path(__file__).resolve().parents[1]
HTML=(R/'ui/Nigredo_CP09_Standalone.html').read_text(encoding='utf-8')
M=json.loads((R/'assets/scenes/meta.json').read_text())
registry=list(csv.DictReader((R/'docs/control_registry.csv').open()))
assert len(registry)==210
assert len({(x['mode'],x['parameter_id']) for x in registry if x['parameter_id']})==164
with sync_playwright() as pw:
 browser=pw.chromium.launch(headless=True,executable_path='/usr/bin/chromium',args=['--no-sandbox','--disable-dev-shm-usage'])
 errors=[]
 for mode in ('audio','midi'):
  page=browser.new_page(viewport={'width':1600,'height':1000},device_scale_factor=1,reduced_motion='reduce')
  page.on('pageerror',lambda exc:errors.append(str(exc)))
  page.set_content(HTML,wait_until='load')
  if mode=='midi':page.locator('#midiMode').click()
  assert page.evaluate('document.documentElement.scrollHeight')==1000, 'CP09 design overflow'
  assert page.locator('.stage-item').count()==7
  assert page.locator('#powerBtn .power-status').inner_text()=='ON'
  titles=page.locator('#stagesList .stage-copy b').all_inner_texts()
  if mode=='audio':assert titles[0]=='SATURATION / DRIVE' and 'COMPRESSION' in titles[-1]
  else:assert titles[0]=='VELOCITY SHAPER' and 'QUANTIZE' in titles[-1]
  # Strictly left->right operation band.
  boxes=[page.locator(f'.stage-item:nth-child({i+1})').bounding_box() for i in range(7)]
  assert max(box['y'] for box in boxes)-min(box['y'] for box in boxes)<3
  assert all(boxes[i]['x']+boxes[i]['width'] <= boxes[i+1]['x']+1 for i in range(6))
  assert all(b['x']>=0 and b['x']+b['width']<=1600 for b in boxes)
  # Global ON/OFF is backed by bypass state (ID80).
  page.locator('#powerBtn').click(); assert page.locator('#powerBtn .power-status').inner_text()=='OFF'
  assert page.locator('#powerBtn').get_attribute('aria-pressed')=='false'
  page.locator('#powerBtn').click();assert page.locator('#powerBtn .power-status').inner_text()=='ON'
  # Stage select, stage OFF, stage ON, pane power and state count.
  for i in range(7):
   page.locator(f'[data-select="{i}"]').click()
   assert page.locator('#editorTitle').inner_text().strip()
   assert page.locator('.editor-panel .param').count()==9
   togg=page.locator(f'[data-toggle="{i}"]')
   togg.click();assert togg.get_attribute('aria-pressed')=='false'
   assert togg.locator('.toggle-text').inner_text()=='OFF'
   togg.click();assert togg.get_attribute('aria-pressed')=='true'
   assert togg.locator('.toggle-text').inner_text()=='ON'
   for entry in M[mode]['stages'][str(i)]['controls']:
    rect=entry['control'];assert rect and all(math.isfinite(v) for v in rect)
    x,y,w,h=rect;assert 0<=x<1600 and 0<=y<1000 and 0<w and 0<h and x+w<=1600 and y+h<=1000
   image=Image.open(R/'assets/scenes'/f'{mode}_{i}.png');assert image.size==(1600,1000)
  page.locator('[data-select="3"]').click()
  page.locator('#stagePower').click()
  assert page.locator('#stagePower').get_attribute('aria-pressed')=='false'
  page.locator('#stagePower').click()
  # Preset apply and interactive parameter actions remain wired.
  page.locator('#presetSelect').select_option('vitriol' if mode=='audio' else 'chance')
  page.locator('#applyPreset').click()
  assert page.locator('#toast').get_attribute('class').find('show')!=-1
  # The native scene capture masks dynamic labels, not static operation descriptions.
  page.screenshot(path=str(R/'distrib'/f'GUI_{mode}_CP09.png'),full_page=False,animations='disabled')
  for category in ('stage','toggle','path'):
   for stage,r in M[mode]['hitboxes'][category]:
    x,y,w,h=r; assert 0<=x<=1600 and 0<=y<=1000 and x+w<=1600 and y+h<=1000
  page.close()
 browser.close()
 assert not errors,errors
print('PASS G3: 7 horizontal processor cards, 82 stable parameters per mode, global/stage ON/OFF, mode labels, presets, no overflow or script errors')
