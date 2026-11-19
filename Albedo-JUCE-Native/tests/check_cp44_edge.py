from pathlib import Path
from playwright.sync_api import sync_playwright

root = Path(__file__).resolve().parents[2]
html = root / 'ui/Albedo_CP30_CP22_Landscape_3x2.html'
geometry = root / 'Albedo-JUCE-Native/Assets/cp35_editor_geometry.js'
with sync_playwright() as playwright:
    browser = playwright.chromium.launch(channel='msedge')
    for dpi in (1, 1.25, 1.5):
        page = browser.new_page(device_scale_factor=dpi, viewport={'width': 1300, 'height': 867})
        page.goto(html.as_uri())
        first = page.locator('#page').bounding_box()
        assert abs(first['x']) < 1.5 and abs(first['width']-1300) < 1.5, first
        page.add_script_tag(path=str(geometry))
        for w,h in ((1300,867),(720,480),(1440,960),(1050,760),(1200,800)):
            page.set_viewport_size({'width':w,'height':h})
            page.evaluate('() => new Promise(r => requestAnimationFrame(() => requestAnimationFrame(r)))')
            rect = page.locator('#page').bounding_box()
            scale = min(w/1320,h/880)
            for key, expected in [('x',(w-1320*scale)/2),('y',(h-880*scale)/2),('width',1320*scale),('height',880*scale)]:
                assert abs(rect[key]-expected)<1.5,(dpi,w,h,key,rect,expected)
            page.locator('#presetBrowseOpen').click(timeout=3000)
            assert page.locator('#presetBrowser').is_visible()
            page.keyboard.press('Escape')
        page.close()
    browser.close()
