#!/usr/bin/env python3
"""CP54: primary controls only, all deep controls accessible, no redraw on edits.
The CP51 192-program selection is injected in CI before this browser test.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright

root=Path(__file__).resolve().parents[2]
html=root/"ui/Albedo_CP30_CP22_Landscape_3x2.html"
assets=root/"Albedo-JUCE-Native/Assets"
out=root/"qa-cp54-screenshots"
out.mkdir(exist_ok=True)

with sync_playwright() as playwright:
    browser=playwright.chromium.launch(channel="msedge")
    for width,height in [(1080,720),(900,600),(720,480)]:
        page=browser.new_page(viewport={"width":width,"height":height})
        page.goto(html.as_uri())
        page.add_script_tag(path=str(assets/"cp35_editor_geometry.js"))
        page.add_script_tag(path=str(assets/"cp52_nereida_browser.js"))
        page.add_script_tag(path=str(assets/"cp54_essential_sound_layout.js"))
        page.wait_for_function("() => !!window.ALBEDO_CP54_ESSENTIAL && !!window.ALBEDO_CP52_BROWSER")
        assert page.evaluate("window.ALBEDO_CP52_BROWSER.total")==192
        for selector,visible in [
            ("#engineKnobs>.param",2),("#macros>.param",4),
            ("#filterKnobsA>.param",2),("#filterKnobsB>.param",2),
            ("#envKnobs>.param",2)]:
            count=page.locator(selector).evaluate_all("nodes => nodes.filter(n=>getComputedStyle(n).display!=='none').length")
            assert count==visible,(width,height,selector,count)
        for selector in [".lower>.surface:not(#cellArp):first-child", "#page .utility"]:
            assert page.locator(selector).first.evaluate("el=>getComputedStyle(el).display==='none'"),selector
        assert page.locator("#page #cellArp .steps").is_visible()
        assert page.locator("#page #fx").is_visible()
        assert page.locator("#page #xy").is_visible()
        page.screenshot(path=str(out/f"Albedo_CP54_{width}x{height}_primary.png"))
        page.locator("#cp54Open").click()
        popup=page.locator("#cp54Popup")
        assert popup.is_visible()
        assert page.locator('#cp54Popup .dial[data-key="engine:2"]').count()==1
        assert page.locator('#cp54Popup .dial[data-key="macro:memory"]').count()==1
        assert page.locator('#cp54Popup .dial[data-key="envDecay"]').count()==1
        assert page.locator('#cp54Popup .dial[data-key="filterAdrive"]').count()==1
        for selector in ["#cp54Popup #routes","#cp54Popup #addRoute",
                         "#cp54Popup .arp-options","#cp54Popup .arp-lattice",
                         "#cp54Popup .utility","#cp54Popup #pitchMode"]:
            assert page.locator(selector).count()==1,selector
        dimensions=popup.locator(".cp54-panel").evaluate("""el=>{
            const r=el.getBoundingClientRect();
            return {left:r.left,right:r.right,top:r.top,bottom:r.bottom};
        }""")
        assert dimensions["left"]>=0 and dimensions["top"]>=0
        assert dimensions["right"]<=width+2 and dimensions["bottom"]<=height+2
        before=page.evaluate("window.ALBEDO_CP04.state.engines[0].knobs[2]")
        dial=page.locator('#cp54Popup .dial[data-key="engine:2"]')
        dial.scroll_into_view_if_needed()
        dial.hover()
        page.mouse.wheel(0,-140)
        after=page.evaluate("window.ALBEDO_CP04.state.engines[0].knobs[2]")
        assert after!=before,(width,height,before,after)
        before_routes=page.locator('#cp54Popup #routes .route-row').count()
        page.locator('#cp54Popup #addRoute').click()
        assert page.locator('#cp54Popup #routes .route-row').count()==before_routes+1
        page.screenshot(path=str(out/f"Albedo_CP54_{width}x{height}_details.png"))
        page.locator("#cp54Close").click()
        assert not popup.is_visible()
        assert page.locator("#page #routes .route-row").count()==before_routes+1
        assert page.locator("#page .utility").count()==1
        page.locator("#presetBrowseOpen").click()
        assert page.locator("#presetBrowser").is_visible()
        page.locator("#presetBrowserClose").click()
        assert not page.locator("#presetBrowser").is_visible()
        page.close()
    browser.close()
print("CP54 PASS: 192 programs; decisive controls only; 3 viewports; details interactive; original routing/restored DOM; browser intact")
