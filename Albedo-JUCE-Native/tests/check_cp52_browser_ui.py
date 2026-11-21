#!/usr/bin/env python3
"""CP52: real Windows Edge viewport and 192-preset GUI integration regression.

Run after CP51's selected morph_expansion_bank.json has been injected into the
canonical HTML by patches/cp51/patch_atlas_catalog.py. No DSP render needed.
The viewport fixture uses the same embedded JS resources as the JUCE editor.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright

root = Path(__file__).resolve().parents[2]
html = root / "ui/Albedo_CP30_CP22_Landscape_3x2.html"
assets = root / "Albedo-JUCE-Native/Assets"

with sync_playwright() as playwright:
    browser = playwright.chromium.launch(channel="msedge")
    for width, height in ((1080, 720), (900, 600), (720, 480)):
        page = browser.new_page(viewport={"width": width, "height": height})
        page.goto(html.as_uri())
        page.add_script_tag(path=str(assets / "cp35_editor_geometry.js"))
        page.add_script_tag(path=str(assets / "cp52_nereida_browser.js"))
        page.wait_for_function("() => !!window.ALBEDO_CP52_BROWSER && !!window.ALBEDO_CP35_WINDOW")
        assert page.evaluate("window.ALBEDO_CP52_BROWSER.total") == 192, "CP51 selected bank must contain 192 sounds"
        open_button = page.locator("#presetBrowseOpen")
        open_button.click()
        dialog = page.locator("#presetBrowser")
        assert dialog.is_visible()
        page.wait_for_function("() => document.querySelector('#albedo-cp52-navigation')?.children.length > 1")
        assert page.locator("#pbFactoryCount").inner_text() == "64"
        assert page.locator("#pbCreativeCount").inner_text() == "64"
        assert page.locator("#pbNewCount").inner_text() == "64"
        assert page.locator("#pbItems .pb-item").count() == 64

        dimensions = page.evaluate("""() => {
            const x=document.getElementById('presetBrowser').getBoundingClientRect();
            const p=document.querySelector('#presetBrowser .preset-browser-panel').getBoundingClientRect();
            const rows=document.getElementById('pbResultContainer');
            return {x:{left:x.left,top:x.top,right:x.right,bottom:x.bottom},
                    p:{left:p.left,top:p.top,right:p.right,bottom:p.bottom},
                    overflowX:rows.scrollWidth-rows.clientWidth,
                    scrollable:rows.scrollHeight>rows.clientHeight}
        }""")
        for box in ("x", "p"):
            bounds = dimensions[box]
            assert bounds["left"] >= -2 and bounds["top"] >= -2, (width,height,box,dimensions)
            assert bounds["right"] <= width+2 and bounds["bottom"] <= height+2, (width,height,box,dimensions)
        assert dimensions["overflowX"] <= 3, (width,height,dimensions)
        assert dimensions["scrollable"], "Preset list must scroll vertically"
        # CP52 compact modal: leave approximately 5% breathing room on every
        # side. The synthesizer behind the browser must remain visually evident.
        panel_width = dimensions["p"]["right"] - dimensions["p"]["left"]
        panel_height = dimensions["p"]["bottom"] - dimensions["p"]["top"]
        assert 0.85 <= panel_width/width <= 0.915, (width,height,dimensions)
        assert 0.85 <= panel_height/height <= 0.915, (width,height,dimensions)
        # The compact size must not hide the top-level controls.
        control_bounds = page.evaluate("""() => {
            const selectors=['#presetBrowserClose','#pbSearch',
              '#presetBrowser [data-collection="New"]',
              '.cp52-window-controls button[aria-label="Reduce plugin window"]',
              '.cp52-window-controls button[aria-label="Keep preset browser open while auditioning"]'];
            const panel=document.querySelector('#presetBrowser .preset-browser-panel').getBoundingClientRect();
            return selectors.map(selector=>{
                const element=document.querySelector(selector);
                const r=element?.getBoundingClientRect();
                return {selector,visible:!!r && r.width>0 && r.height>0,
                  inside:!!r && r.left>=panel.left-2 && r.right<=panel.right+2 &&
                    r.top>=panel.top-2 && r.bottom<=panel.bottom+2};
            });
        }""")
        assert all(v["visible"] and v["inside"] for v in control_bounds), (width,height,control_bounds)
        shots = root / "qa-cp52-screenshots"
        shots.mkdir(exist_ok=True)
        page.screenshot(path=str(shots / f"Albedo_CP52_{width}x{height}_browser.png"))

        # Window controls are available even while the preset browser is open.
        page.evaluate("""() => {
            window.cp52ResizeRequests=[];
            window.ALBEDO_CP35_WINDOW.requestSize=(w)=>cp52ResizeRequests.push(w);
        }""")
        page.locator('.cp52-window-controls button[aria-label="Enlarge plugin window"]').click()
        page.locator('.cp52-window-controls button[aria-label="Reduce plugin window"]').click()
        page.locator('.cp52-window-controls button[aria-label="Comfortable 1080 × 720 window"]').click()
        reqs = page.evaluate("window.cp52ResizeRequests")
        assert len(reqs) == 3 and reqs[-1] == 1080, (width,height,reqs)

        # Normal selection stays open in AUDITION mode.
        page.locator('#pbItems .pb-item').first.click()
        assert dialog.is_visible()
        assert page.locator('#pbItems .pb-item.selected').count() >= 1
        page.get_by_role("tab", name="NEW SOUNDS").click()
        assert page.locator("#pbItems .pb-item").count() == 64
        page.locator("#albedo-cp52-navigation .cp52-category").filter(has_text="Bass").first.click()
        count = page.locator("#pbItems .pb-item").count()
        assert 0 < count < 64, (width,height,count)

        # With audition disabled, original close-after-load behavior returns.
        page.locator('.cp52-window-controls button[aria-label="Keep preset browser open while auditioning"]').click()
        page.locator('#pbItems .pb-item').first.click()
        assert not dialog.is_visible()
        page.close()
    browser.close()
print("CP52 PASS: 192 indexed sounds, sidebar, scroll, modal fit, resize buttons, continuous audition in Edge")
