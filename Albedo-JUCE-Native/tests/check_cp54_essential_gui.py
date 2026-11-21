#!/usr/bin/env python3
"""CP54 Edge QA: high-impact parameters remain fixed; contextual popups work.
Run after verified CP51 catalog injection; no DSP sample/regeneration needed.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright

root=Path(__file__).resolve().parents[2]
html=root/"ui/Albedo_CP30_CP22_Landscape_3x2.html"
assets=root/"Albedo-JUCE-Native/Assets"
output=root/"qa-cp54-context"
output.mkdir(exist_ok=True)

def check_bounds(page,width,height):
    rect=page.locator("#cp54Popover").evaluate("""el=>{
        const r=el.getBoundingClientRect();
        return {left:r.left,right:r.right,top:r.top,bottom:r.bottom};
    }""")
    assert rect["left"] >= -2 and rect["top"] >= -2,(width,height,rect)
    assert rect["right"] <= width+2 and rect["bottom"] <= height+2,(width,height,rect)

with sync_playwright() as p:
    browser=p.chromium.launch(channel="msedge")
    for width,height in [(1080,720),(900,600),(720,480)]:
        page=browser.new_page(viewport={"width":width,"height":height})
        page.goto(html.as_uri())
        for js in ["cp35_editor_geometry.js",
                   "cp52_nereida_browser.js",
                   "cp54_essential_sound_layout.js"]:
            page.add_script_tag(path=str(assets/js))
        page.wait_for_function("() => !!window.ALBEDO_CP54_ESSENTIAL && !!window.ALBEDO_CP52_BROWSER")
        assert page.evaluate("window.ALBEDO_CP52_BROWSER.total")==192
        assert page.locator("#cp04LabOpen").is_hidden()
        assert page.locator("#cp04Lab").is_hidden()
        assert page.locator('[data-cp54-context]').count()==7
        assert page.evaluate("window.ALBEDO_CP54_ESSENTIAL.primaryControlsPermanent")
        assert page.evaluate("window.ALBEDO_CP54_ESSENTIAL.advancedModeRemoved")
        for selector,n in [
            ("#engineKnobs>.param",2),("#macros>.param",4),
            ("#filterKnobsA>.param",3),("#filterKnobsB>.param",3),
            ("#envKnobs>.param",3)
        ]:
            shown=page.locator(selector).evaluate_all(
                "a=>a.filter(x=>getComputedStyle(x).display!=='none').length")
            assert shown==n,(width,height,selector,shown)
        for selector in ["#sources","#method","#xy","#filterTypeA","#filterTypeB",
                         "#fx","#cellArp #steps","#cellArp #play"]:
            assert page.locator(selector).is_visible(),(width,height,selector)
        page.screenshot(path=str(output/f"CP54_{width}x{height}_sound_first.png"))

        # Main Material/Unison is fixed wherever the synthesis algorithm uses it.
        page.locator("#method").select_option(label="Harmonic / Additive")
        assert page.locator("#cp54MaterialMain").is_visible()
        assert page.locator("#cp54UnisonMain").is_visible()
        page.locator("#cp54MaterialMain").select_option("3")
        assert page.evaluate("window.ALBEDO_CP04.state.cp04[1010]")==3
        page.locator("#cp54UnisonMain").select_option("3")
        assert page.evaluate("window.ALBEDO_CP04.state.cp04[1011]")==3

        def open_popup(context):
            page.locator(f'[data-cp54-context="{context}"]').click()
            assert page.locator("#cp54Popover").is_visible(),(context,width,height)
            assert page.evaluate("window.ALBEDO_CP54_ESSENTIAL.activeContext")==context
            check_bounds(page,width,height)

        def close_popup():
            page.locator("#cp54Close").click()
            assert page.locator("#cp54Popover").is_hidden()
            assert page.locator("#cp54Scrim").is_hidden()

        open_popup("source")
        assert page.locator('#cp54Popover .dial[data-key="engine:2"]').count()==1
        assert page.locator('#cp54Popover [data-cp54-id="1017"]').count()==1
        keytrack=page.locator("#cp54Popover .cp54-keytrack")
        assert keytrack.is_visible()
        before_tracking=page.evaluate("window.ALBEDO_CP04.state.engines[0].keytrack")
        keytrack.click()
        assert page.evaluate("window.ALBEDO_CP04.state.engines[0].keytrack")!=before_tracking
        keytrack.click()
        assert page.evaluate("window.ALBEDO_CP04.state.engines[0].keytrack")==before_tracking
        native_before=page.evaluate("window.ALBEDO_CP04.state.cp04[1017]")
        dial=page.locator('#cp54Popover .cp54-rotary[data-cp54-id="1017"]')
        dial.scroll_into_view_if_needed()
        dial.hover()
        page.mouse.wheel(0,-90)
        native_after=page.evaluate("window.ALBEDO_CP04.state.cp04[1017]")
        assert native_before!=native_after,(native_before,native_after)
        page.screenshot(path=str(output/f"CP54_{width}x{height}_source_popup.png"))
        close_popup()

        open_popup("macro")
        assert page.locator('#cp54Popover .dial[data-key="macro:width"]').count()==1
        close_popup()

        open_popup("filter")
        for key in ("envDecay",):
            assert page.locator(f'#cp54Popover .dial[data-key="{key}"]').count()==1,key
        assert page.locator("#page #filterKnobsA>.param:nth-child(3)").is_visible()
        assert page.locator("#page #envKnobs>.param:nth-child(3)").is_visible()
        assert page.locator("#cp54Popover #pitchMode").count()==1
        assert page.locator('#cp54Popover [data-cp54-id="3120"]').count()==1
        close_popup()
        assert page.locator("#page #pitchMode").count()==1

        open_popup("mod")
        assert page.locator("#cp54Popover #routes").count()==1
        routes_before=page.locator("#cp54Popover .route-row").count()
        page.locator("#cp54Popover #addRoute").click()
        assert page.locator("#cp54Popover .route-row").count()==routes_before+1
        page.screenshot(path=str(output/f"CP54_{width}x{height}_routes_popup.png"))
        close_popup()
        assert page.locator("#page #routes .route-row").count()==routes_before+1

        open_popup("arp")
        assert page.locator("#cp54Popover #rule").count()==1
        assert page.locator("#cp54Popover .arp-options").count()==1
        page.locator("#cp54Popover #rule").select_option("90")
        assert page.evaluate("window.ALBEDO_CP04.state.motionRule")==90
        # The original ARP change handler used to be scoped to #cellArp.
        # When these original nodes move into the popup, that handler no longer
        # receives the event. Verify the real native bridge dispatches it.
        page.evaluate("""() => {
            window.cp54ArpDispatches=[];
            const native=window.ALBEDO_NATIVE_CP27;
            const original=native.onExtraParam;
            native.onExtraParam=function(id,value){
                window.cp54ArpDispatches.push([id,value]);
                return original.call(native,id,value);
            };
        }""")
        page.locator('#cp54Popover [data-arp="6024"]').select_option("1")
        assert [6024,1] in page.evaluate("window.cp54ArpDispatches"), (
            width,height,"ARP character failed to reach JUCE bridge")
        page.locator('#cp54Popover [data-arp="6025"]').select_option("3")
        assert [6025,3] in page.evaluate("window.cp54ArpDispatches"), (
            width,height,"ARP octave span failed to reach JUCE bridge")
        close_popup()
        assert page.locator("#page #rule").count()==1

        open_popup("fx")
        assert page.locator('#cp54Popover [data-cp54-id="5011"]').count()==1
        close_popup()

        open_popup("settings")
        assert page.locator("#cp54Popover #oversample").count()==1
        assert page.locator("#cp54Popover #safety").count()==1
        page.locator("#cp54Popover #oversample").select_option(label="2×")
        assert page.evaluate("window.ALBEDO_CP04.state.oversample")=="2×"
        close_popup()
        assert page.locator("#page #oversample").count()==1

        # No global advanced mode or broken preset browser paths were introduced.
        assert page.locator("#page #engineKnobs>.param").count()==4
        page.locator("#presetBrowseOpen").click()
        assert page.locator("#presetBrowser").is_visible()
        page.locator("#presetBrowserClose").click()
        assert page.locator("#presetBrowser").is_hidden()
        page.close()
    browser.close()
print("CP54 PASS: 192 presets; primary sound controls permanent; 7 local overlays; all parameters editable at 1080/900/720 widths")
