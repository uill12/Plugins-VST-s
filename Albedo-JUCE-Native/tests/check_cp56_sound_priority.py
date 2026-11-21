#!/usr/bin/env python3
"""CP56 Windows Edge acceptance: real DSP parameters occupy priority-ranked,
non-overlapping positions inside the CP55 fixed 3:2 renderer. No audio claims.
Launch after CP51 192-program expansion and use the original Edge browser.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright

root = Path(__file__).resolve().parents[2]
html = root / "ui/Albedo_CP30_CP22_Landscape_3x2.html"
assets = root / "Albedo-JUCE-Native/Assets"
output = root / "qa-cp56-sound-priority"
output.mkdir(exist_ok=True)

def live(page, selector):
    return page.locator(selector).evaluate_all(
        "els=>els.filter(e=>getComputedStyle(e).display!=='none' && e.getClientRects().length).length"
    )

def fits(page, selector, parent):
    rects = page.evaluate("""([sel,par])=>{
      const a=document.querySelector(sel)?.getBoundingClientRect(),
            b=document.querySelector(par)?.getBoundingClientRect();
      return a&&b?{a:{left:a.left,right:a.right,top:a.top,bottom:a.bottom,
                         width:a.width,height:a.height},
                   b:{left:b.left,right:b.right,top:b.top,bottom:b.bottom}}:null;
    }""", [selector,parent])
    assert rects, (selector,parent)
    a,b = rects["a"],rects["b"]
    assert a["width"]>12 and a["height"]>8, (selector,a)
    assert a["left"]>=b["left"]-3 and a["right"]<=b["right"]+3,(selector,parent,rects)
    assert a["top"]>=b["top"]-3 and a["bottom"]<=b["bottom"]+3,(selector,parent,rects)

with sync_playwright() as p:
    browser = p.chromium.launch(channel="msedge")
    for width,height in ((1230,820),(1080,720),(900,600),(720,480)):
        page = browser.new_page(viewport={"width":width,"height":height})
        errors = []
        page.on("pageerror",lambda error: errors.append(str(error)))
        page.goto(html.as_uri())
        for js in (
            "cp35_editor_geometry.js",
            "cp52_nereida_browser.js",
            "cp54_essential_sound_layout.js",
            "cp56_sound_priority.js",
        ):
            page.add_script_tag(path=str(assets/js))
        page.wait_for_function("()=>!!window.ALBEDO_CP56_PRIORITY")
        assert live(page,"#engineKnobs>.param")==4,(width,height,"oscillator character")
        assert live(page,"#layerKnobs>.param")==3,(width,height,"level/pitch/pan")
        assert live(page,"#macros>.param")==7,(width,height,"macro")
        assert live(page,"#envKnobs>.param")==4,(width,height,"ADSR")
        assert live(page,"#cp56Source .cp56-param")==4,(width,height,"source detail")
        assert live(page,"#cp56Envelope .cp56-param")==4,(width,height,"filter env")
        assert live(page,"#cp56Arp .cp56-param")==4,(width,height,"arp")
        assert live(page,"#fx .cp56-fx-param")==3,(width,height,"effect color")
        assert page.locator("#cp56Route").is_visible()
        macro_order = page.locator("#macros .dial").evaluate_all(
            "els=>els.map(e=>e.dataset.key)")
        assert macro_order==[
            "macro:bright","macro:contour","macro:impact","macro:exchange",
            "macro:width","macro:growth","macro:memory"
        ],(width,height,macro_order)

        fits(page,"#cp56Source","#page .source-panel .inspector")
        fits(page,"#cp56Envelope","#page .shape-panel .shape-block:nth-child(3)")
        fits(page,"#cp56Arp","#page #cellArp .arp-performance")
        for selector in ("#page .engine-rack .source-card:nth-child(4)",
                         "#page .shape-panel","#page #fx"):
            fits(page,selector,"#page")
        # Real CP04 ID values, not visual placeholder values.
        source_dial=page.locator('#cp56Source [data-cp56-id="1012"] .cp56-rotary')
        before=page.evaluate("window.ALBEDO_CP04.state.cp04[1012]")
        source_dial.hover()
        page.mouse.wheel(0,-100)
        after=page.evaluate("window.ALBEDO_CP04.state.cp04[1012]")
        assert after>before,(width,height,"source detune wheel",before,after)
        assert page.locator('[data-cp56-id="1012"] output').inner_text().strip()
        # Filter envelope, ARP, and FX must update the canonical parameter state.
        for ident in (3120,6023,5011):
            d=page.locator(f'#page [data-cp56-id="{ident}"] .cp56-rotary')
            before=page.evaluate(f'window.ALBEDO_CP04.state.cp04[{ident}]')
            d.hover();page.mouse.wheel(0,-110)
            after=page.evaluate(f'window.ALBEDO_CP04.state.cp04[{ident}]')
            assert before!=after,(width,height,ident,before,after)
        page.locator("#cp56Route").select_option("SERIAL A-B")
        assert page.locator("#filterRoute").input_value()=="SERIAL A-B"
        page.locator("#method").select_option(label="Harmonic / Additive")
        page.wait_for_function("()=>!!document.querySelector('#cp56Source [data-cp56-id=\"1016\"]')")
        page.locator('#cp56Source [data-cp56-id="1016"] .cp56-rotary').hover()
        page.mouse.wheel(0,-110)
        assert page.evaluate("window.ALBEDO_CP04.state.cp04[1016]")>12
        # Switching source rebinds the same 4 controls to the new source ID group.
        page.locator('.source-card [data-select="1"]').click()
        page.wait_for_function("()=>window.ALBEDO_CP56_PRIORITY.sourceIds[0]>=1032")
        assert page.evaluate("window.ALBEDO_CP56_PRIORITY.sourceIds.every(id=>id>=1032&&id<=1063)")
        # The validated Nereida-style 3×64 bank is unchanged.
        assert page.evaluate("window.ALBEDO_CP52_BROWSER.total")==192
        page.screenshot(path=str(output/f"CP56_{width}x{height}_priority_sound_stage.png"))
        assert not errors,(width,height,errors)
        page.close()
    browser.close()
print("CP56 Edge PASS: impact-ranked visible controls, live IDs, method-dependent source binding, geometry at 1230/1080/900/720, 192 presets")
