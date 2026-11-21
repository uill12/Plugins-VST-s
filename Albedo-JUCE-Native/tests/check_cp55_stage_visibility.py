#!/usr/bin/env python3
"""CP55R1 real Edge stage-fit regression for the Bitwig clipping screenshot.

The native HTML contains a fixed 1500x1000 source layout. Assertions inspect
the RIGHTMOST source, Corpus and the LOWER ARP/FX elements, not merely #page.
The whole design must fit the granted host viewport, including when WebView2
overreports exactly one CSS axis. No preset browser redesign is involved.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright

root=Path(__file__).resolve().parents[2]
html=root/"ui/Albedo_CP30_CP22_Landscape_3x2.html"
assets=root/"Albedo-JUCE-Native/Assets"
out=root/"qa-cp55-stage-fit"
out.mkdir(exist_ok=True)

script="""({width,height,wrong}) => {
 const root=document.documentElement;
 const originalW=Object.getOwnPropertyDescriptor(root,'clientWidth');
 const originalH=Object.getOwnPropertyDescriptor(root,'clientHeight');
 const fakeW=wrong.includes('w')?Math.round(width*1.29):width;
 const fakeH=wrong.includes('h')?Math.round(height*1.29):height;
 try{
   Object.defineProperty(root,'clientWidth',{configurable:true,get:()=>fakeW});
   Object.defineProperty(root,'clientHeight',{configurable:true,get:()=>fakeH});
   window.ALBEDO_CP35_WINDOW.setHostGeometry(width,height);
   window.ALBEDO_CP35_WINDOW.fit();
   const page=document.querySelector('#page').getBoundingClientRect();
   const targets=[
      ['fourth-source','#page .engine-rack .source-card:nth-child(4)'],
      ['source-inspector','#page .source-panel .inspector'],
      ['morph-xy','#page .center .network'],
      ['corpus','#page .shape-panel'],
      ['envelope','#page .shape-panel #envKnobs'],
      ['arpeggiator','#page .lower #cellArp'],
      ['effects','#page .lower #fx']
   ];
   const measured=targets.map(([name,selector])=>{
      const el=document.querySelector(selector),r=el?.getBoundingClientRect();
      return {name,rect:r&&{left:r.left,right:r.right,top:r.top,bottom:r.bottom,
                            width:r.width,height:r.height},
               visible:!!r && r.width>6 && r.height>6};
   });
   return {page:{left:page.left,right:page.right,top:page.top,bottom:page.bottom},
           elements:measured,viewport:window.ALBEDO_CP35_WINDOW.geometry};
 } finally {
   if(originalW)Object.defineProperty(root,'clientWidth',originalW);
   else delete root.clientWidth;
   if(originalH)Object.defineProperty(root,'clientHeight',originalH);
   else delete root.clientHeight;
   window.ALBEDO_CP35_WINDOW.setHostGeometry(0,0);
 }
}"""

with sync_playwright() as playwright:
    browser=playwright.chromium.launch(channel="msedge")
    for width,height in ((1230,820),(1080,720),(900,600),(720,480)):
        page=browser.new_page(viewport={"width":width,"height":height})
        page.goto(html.as_uri())
        for asset in ("cp35_editor_geometry.js","cp52_nereida_browser.js",
                      "cp54_essential_sound_layout.js"):
            page.add_script_tag(path=str(assets/asset))
        page.wait_for_function("()=>!!window.ALBEDO_CP35_WINDOW && !!window.ALBEDO_CP54_ESSENTIAL")
        assert page.locator("#page .engine-rack .source-card").count()==4
        assert page.locator("#page .shape-panel").is_visible()
        assert page.locator("#page .lower #cellArp").is_visible()
        assert page.locator("#page .lower #fx").is_visible()
        for wrong in ("","w","h","wh"):
            d=page.evaluate(script,{"width":width,"height":height,"wrong":wrong})
            b=d["page"]
            assert b["left"]>=-2 and b["right"]<=width+2,(width,height,wrong,"page",b)
            assert b["top"]>=-2 and b["bottom"]<=height+2,(width,height,wrong,"page",b)
            for element in d["elements"]:
                r=element["rect"]
                assert element["visible"],(width,height,wrong,element)
                assert r["left"]>=b["left"]-2 and r["right"]<=b["right"]+2,(
                    width,height,wrong,element)
                assert r["top"]>=b["top"]-2 and r["bottom"]<=b["bottom"]+2,(
                    width,height,wrong,element)
                assert r["left"]>=-2 and r["right"]<=width+2,(width,height,wrong,element)
                assert r["top"]>=-2 and r["bottom"]<=height+2,(width,height,wrong,element)
        page.screenshot(path=str(out/f"CP55_{width}x{height}_all-four-sources-corpus-fx.png"))
        # The deep controls are contextual. All popups must remain within the
        # same host viewport, even when the main canvas is scaled down.
        for ctx in ("source","filter","arp","fx","settings"):
            page.locator(f'[data-cp54-context="{ctx}"]').click()
            box=page.locator("#cp54Popover").bounding_box()
            assert box is not None and box["width"]>150 and box["height"]>40,(ctx,box)
            assert box["x"]>=-2 and box["x"]+box["width"]<=width+2,(ctx,width,box)
            assert box["y"]>=-2 and box["y"]+box["height"]<=height+2,(ctx,height,box)
            page.locator("#cp54Close").click()
        page.close()
    browser.close()
print("CP55R1 PASS: 4 sources, inspector, XY, Corpus, ADSR, ARP, FX, popups, 4 sizes × 4 viewport mismatch cases")
