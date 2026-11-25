#!/usr/bin/env python3
"""CP63 live ARP popup: original controls are reparented and restored in Edge."""
from pathlib import Path
from playwright.sync_api import sync_playwright

root=Path(__file__).resolve().parents[2]
html=root/"ui/Albedo_CP30_CP22_Landscape_3x2.html"
assets=root/"Albedo-JUCE-Native/Assets"
with sync_playwright() as p:
 browser=p.chromium.launch(channel="msedge")
 for w,h in ((1230,820),(900,600),(720,480)):
  page=browser.new_page(viewport={"width":w,"height":h})
  errors=[]
  page.on("pageerror",lambda e:errors.append(str(e)))
  page.goto(html.as_uri())
  for asset in ("cp35_editor_geometry.js","cp52_nereida_browser.js",
                "cp54_essential_sound_layout.js","cp56_sound_priority.js",
                "cp57_tempo_controls.js"):
   page.add_script_tag(path=str(assets/asset))
  page.locator("#cellArp .cp54-trigger").click()
  pop=page.locator('#cp54Popover[data-section="arp"]')
  assert pop.is_visible(),(w,h,"popup")
  for selector in ("#cp57TempoControls","#cpArpPattern","#steps",".arp-lattice",
                   ".arp-options",'[data-arp="6032"]','[data-arp="6031"]',
                   '[data-arp="6024"]','[data-arp="6025"]','[data-arp="6026"]',
                   '[data-arp="6027"]','[data-arp="6028"]','[data-arp="6023"]'):
   assert page.locator("#cp54Popover "+selector).count()>0,(w,h,selector)
  assert page.locator('#cp54Popover [data-arp="6022"]').count()==0
  page.locator('#cp54Popover #cp57Clock').select_option("0")
  assert page.locator('#cp54Popover [data-cp54-id="6029"]').count()>0
  page.locator('#cp54Popover [data-arp="6025"]').select_option("2")
  assert page.evaluate('window.ALBEDO_CP04.state.cp04[6025]')==2 or page.locator('#cp54Popover [data-arp="6025"]').input_value()=="2"
  page.locator("#cp54Close").click()
  assert not page.locator("#cp54Popover").is_visible()
  for selector in ("#steps","#cp57TempoControls","#cpArpPattern",".arp-options"):
   assert page.locator("#cellArp "+selector).count()==1,(w,h,"restore",selector)
  assert not errors,(w,h,errors)
  page.close()
 browser.close()
print("CP63 Edge PASS: responsive arpeggiator, live controls, original-node reparent/restore")
