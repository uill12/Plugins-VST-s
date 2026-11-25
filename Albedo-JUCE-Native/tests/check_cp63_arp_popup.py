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
  # Native step telemetry must repaint one existing button rather than recreate 16.
  untouched=page.evaluate("""()=>{
    const first=document.querySelector('#steps .step');
    window.__cp63StepProbe=first;
    window.ALBEDO_CP04.setArpStep(3);
    return first===document.querySelector('#steps .step') &&
           document.querySelector('#steps [data-step="3"]').classList.contains('current');
  }""")
  assert untouched,(w,h,"non-destructive step highlighting")
  # Count outgoing parameter events; detailed controls must not dispatch twice.
  page.evaluate("""()=>{
    window.__cp63ArpSent=[];
    window.ALBEDO_NATIVE_CP27={onExtraParam:(id,value)=>__cp63ArpSent.push([id,value])};
  }""")
  page.locator('#cp54Popover #cp57Clock').select_option("0")
  assert page.locator('#cp54Popover [data-cp54-id="6029"]').is_visible()
  page.locator('#cp54Popover [data-arp="6025"]').select_option("2")
  assert page.evaluate('window.ALBEDO_CP04.state.cp04[6025]')==2,(w,h,"octave state")
  page.locator('#cp54Popover [data-arp="6032"]').select_option("4")
  page.wait_for_function("()=>window.ALBEDO_CP04.state.cp04[6032]===4 && document.querySelector('#pulsusMeter').value==='4'")
  page.locator('#cp54Popover [data-arp="6031"]').select_option("6")
  page.wait_for_function("()=>window.ALBEDO_CP04.state.cp04[6031]===6 && document.querySelector('#cpArpPattern').value==='9'")
  counts=page.evaluate("""()=>{
    const ids=[6025,6031,6032];
    return Object.fromEntries(ids.map(id=>[id,window.__cp63ArpSent.filter(x=>x[0]===id).length]));
  }""")
  assert all(counts[k]==1 for k in (6025,6031,6032)),(w,h,"exactly-once MIDI/host param bridge",counts)
  # Simulated DAW automation must repaint one existing dial in-place.
  refreshed=page.evaluate("""()=>{
    const dial=document.querySelector('#cp54Popover [data-cp54-id="6023"]');
    if(!dial)return false;
    window.ALBEDO_CP04.state.cp04[6023]=41;
    window.ALBEDO_CP54_ESSENTIAL.refresh();
    return dial===document.querySelector('#cp54Popover [data-cp54-id="6023"]') &&
           Number(dial.getAttribute('aria-valuenow'))===41;
  }""")
  assert refreshed,(w,h,"live automation without DOM replacement")
  assert page.locator('#cp54Popover .arp-internal-tempo').is_hidden(),(w,h,"one internal BPM control")
  page.locator("#cp54Close").click()
  assert not page.locator("#cp54Popover").is_visible()
  for selector in ("#steps","#cp57TempoControls","#cpArpPattern",".arp-options"):
   assert page.locator("#cellArp "+selector).count()==1,(w,h,"restore",selector)
  assert not errors,(w,h,errors)
  page.close()
 browser.close()
print("CP63 Edge PASS: responsive arpeggiator, live controls, original-node reparent/restore")
