#!/usr/bin/env python3
"""CP63R7: STEPS count controls availability of all 16 ARP gate buttons.

Keep inactive gate states intact for preset compatibility. No DOM rebuild on
knob/host automation, and a disabled step cannot dispatch a MIDI parameter.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright

root=Path(__file__).resolve().parents[2]
html=root/"ui/Albedo_CP30_CP22_Landscape_3x2.html"
assets=root/"Albedo-JUCE-Native/Assets"
out=root/"qa-cp63r7-steps"
out.mkdir(exist_ok=True)

def assert_window(page,count,width):
    rows=page.locator("#steps .step")
    assert rows.count()==16,(width,"gate count")
    info=page.eval_on_selector_all("#steps [data-step]","""
     items=>items.map((b,i)=>({
       disabled:b.disabled, aria:b.getAttribute('aria-disabled'),
       classOff:b.classList.contains('step-unavailable'),
       opacity:Number(getComputedStyle(b).opacity)
     }))
    """)
    for i,item in enumerate(info):
        blocked=i>=count
        assert item["disabled"]==blocked,(width,count,i,item)
        assert item["aria"]==str(blocked).lower(),(width,count,i,item)
        assert item["classOff"]==blocked,(width,count,i,item)
        if blocked:
            assert item["opacity"]<.5,(width,count,i,"not faded",item)

with sync_playwright() as p:
    browser=p.chromium.launch(channel="msedge")
    for width,height in ((1230,820),(900,600),(720,480)):
        page=browser.new_page(viewport={"width":width,"height":height})
        errors=[]
        page.on("pageerror",lambda err:errors.append(str(err)))
        page.goto(html.as_uri())
        for asset in ("cp35_editor_geometry.js","cp52_nereida_browser.js",
                      "cp54_essential_sound_layout.js","cp56_sound_priority.js",
                      "cp57_tempo_controls.js"):
            page.add_script_tag(path=str(assets/asset))
        page.wait_for_function("()=>!!window.ALBEDO_CP04 && !!window.ALBEDO_CP54_ESSENTIAL")
        assert_window(page,8,width)
        page.evaluate("""()=>{
          window.__stepProbe=[...document.querySelectorAll('#steps [data-step]')];
          window.__arpSent=[];
          window.ALBEDO_NATIVE_CP27={onExtraParam:(id,v)=>window.__arpSent.push([id,v])};
          window.ALBEDO_CP04.state.steps[10]=true;
          window.ALBEDO_CP04.refreshStepAvailability();
        }""")
        # Directly adjusting the real STEPS knob must not replace button nodes.
        page.evaluate("window.ALBEDO_CP04.setExtraParam(6027,4)")
        assert_window(page,4,width)
        assert page.evaluate("window.__stepProbe.every((n,i)=>n===document.querySelectorAll('#steps [data-step]')[i])"),width
        # Programmatically dispatched invalid clicks are rejected, even when
        # the host or a script bypasses the native disabled click behavior.
        unchanged=page.evaluate("""()=>{
          const before=window.ALBEDO_CP04.state.steps[10];
          const sent=window.__arpSent.length;
          document.querySelector('#steps [data-step="10"]').dispatchEvent(
              new MouseEvent('click',{bubbles:true,cancelable:true}));
          return window.ALBEDO_CP04.state.steps[10]===before && window.__arpSent.length===sent;
        }""")
        assert unchanged,(width,"disabled event guard")
        # CP64: sculpted Albedo gate/buttons, and fill only the available steps.
        assert page.locator("#arpFill").is_visible(),(width,"fill available")
        page.locator("#arpFill").click()
        assert page.evaluate("window.ALBEDO_CP04.state.steps.slice(0,4).every(Boolean)")
        assert page.locator("#arpFill").is_disabled(),(width,"fill should be complete")
        material=page.evaluate("""()=>{
          const b=document.querySelector('#steps [data-step="1"]');
          const s=getComputedStyle(b);
          const fill=getComputedStyle(document.querySelector('#regen'));
          return {background:s.backgroundImage,shadow:s.boxShadow,fillShadow:fill.boxShadow};
        }""")
        assert "gradient" in material["background"] and material["shadow"]!="none" and material["fillShadow"]!="none",(width,"albedo material styling",material)
        # A normal click edits one active gate and emits one native parameter event.
        sent=page.evaluate("window.__arpSent.length")
        page.locator('#steps [data-step="2"]').click()
        assert page.evaluate('window.__arpSent.slice(-1)[0][0]')==6002,(width,"gate event ID")
        assert page.evaluate("window.__arpSent.length")==sent+1,(width,"single gate event")
        page.locator("#cellArp .cp54-trigger").click()
        assert page.locator('#cp54Popover[data-section="arp"]').is_visible()
        # Move real selector from 4 to 12, then to 16. Hidden gate 11 retained.
        page.locator('#cp54Popover [data-arp="6027"]').select_option("12")
        assert_window(page,12,width)
        page.locator('#cp54Popover [data-arp="6027"]').select_option("16")
        assert_window(page,16,width)
        assert page.locator('#steps [data-step="10"]').get_attribute("aria-pressed")=="true"
        assert page.evaluate("window.__stepProbe.every((n,i)=>n===document.querySelectorAll('#steps [data-step]')[i])"),(width,"popup node identity")
        page.screenshot(path=str(out/f"CP63R7_{width}x{height}_sixteen_steps.png"))
        page.locator("#cp54Close").click()
        # Simulated DAW value update uses the same in-place public refresh hook,
        # and must not emit a UI-originating parameter event.
        before=page.evaluate("window.__arpSent.length")
        page.evaluate("""()=>{
          window.ALBEDO_CP04.state.cp04[6027]=6;
          window.ALBEDO_CP04.refreshStepAvailability();
        }""")
        assert_window(page,6,width)
        assert page.evaluate("window.__arpSent.length")==before,(width,"no automation echo")
        assert page.evaluate("window.__stepProbe.every((n,i)=>n===document.querySelectorAll('#steps [data-step]')[i])"),(width,"automation node identity")
        assert not errors,(width,height,errors)
        page.close()
    browser.close()
print("CP63R7 Edge PASS: 4/6/8/12/16 steps, disabled/faded gates, preserved masks, no DOM rebuild, native gate dispatch, popup and host automation")
