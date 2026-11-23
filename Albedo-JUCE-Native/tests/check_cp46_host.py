import ctypes
from ctypes import wintypes
import os
from pathlib import Path
import socket
import subprocess
import sys
import time
from urllib.request import urlopen
from playwright.sync_api import sync_playwright

def main():
    with socket.socket() as sock:
        sock.bind(('127.0.0.1',0))
        port=sock.getsockname()[1]
    env=dict(os.environ, WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS=f'--remote-debugging-port={port}')
    proc=subprocess.Popen([sys.argv[1],str(Path(sys.argv[2]).resolve())],env=env)
    try:
        endpoint=f'http://127.0.0.1:{port}'
        for _ in range(180):
            if proc.poll() is not None: raise RuntimeError(f'CLAP host exit {proc.returncode}')
            try:
                with urlopen(endpoint+'/json/version',timeout=1): break
            except OSError: time.sleep(.25)
        user=ctypes.windll.user32
        user.FindWindowW.restype=wintypes.HWND
        hwnd=user.FindWindowW('Static','Albedo viewport fixture')
        assert hwnd
        user.SetWindowPos.argtypes=[wintypes.HWND,wintypes.HWND,ctypes.c_int,ctypes.c_int,ctypes.c_int,ctypes.c_int,wintypes.UINT]
        with sync_playwright() as p:
            browser=p.chromium.connect_over_cdp(endpoint)
            ctx=browser.contexts[0]
            page=ctx.pages[0] if ctx.pages else ctx.wait_for_event('page')
            page.wait_for_function('() => !!window.ALBEDO_CP35_WINDOW',timeout=45000)
            user.GetPropW.argtypes=[wintypes.HWND,wintypes.LPCWSTR]
            user.GetPropW.restype=wintypes.HANDLE
            maximum=user.GetPropW(hwnd,'AlbedoFixtureMaxWidth')
            assert maximum
            for requested in (900,720,1020):
                w=min(requested,maximum)//6*6
                h=w*2//3
                assert user.SetWindowPos(hwnd,None,0,0,w,h,0x0014)
                # Verify the actual WebView2 client canvas. On Windows the HWND
                # outer coordinates and Edge CSS pixels need not share a DPR unit.
                # An exact comparison to SetWindowPos's outer width can falsely
                # reject a correctly fitted client on mixed-DPI host displays.
                page.wait_for_function("""()=>{
                    const root=document.documentElement;
                    const rect=document.querySelector('#page').getBoundingClientRect();
                    const g=window.ALBEDO_CP35_WINDOW?.geometry;
                    return g && root.clientWidth>0 && root.clientHeight>0
                       && Math.abs(rect.left)<3 && Math.abs(rect.top)<3
                       && Math.abs(rect.right-root.clientWidth)<3
                       && Math.abs(rect.bottom-root.clientHeight)<3
                       && Math.abs(g.width-root.clientWidth)<3
                       && Math.abs(g.height-root.clientHeight)<3;
                }""",timeout=20000)
                visible=page.evaluate("""()=>({
                    viewport:[document.documentElement.clientWidth,document.documentElement.clientHeight],
                    page:(()=>{const x=document.querySelector('#page').getBoundingClientRect();return [x.width,x.height,x.left,x.top]})(),
                    scale:window.ALBEDO_CP35_WINDOW.geometry.scale
                })""")
                print('Compiled CLAP WebView2 client/render geometry',requested,visible,flush=True)
                page.locator('#presetBrowseOpen').click()
                assert page.locator('#presetBrowser').is_visible()
                page.keyboard.press('Escape')
                print('Compiled CLAP physical client and editor match',w,h)
            # CP52: use the compiled WebView2 -> JUCE -> CLAP host path,
            # rather than only checking mocked browser resize events.
            page.wait_for_function('() => !!window.ALBEDO_CP52_BROWSER',timeout=15000)
            page.locator('#presetBrowseOpen').click()
            assert page.locator('#presetBrowser').is_visible()
            def viewport_width():
                return page.evaluate('document.documentElement.clientWidth')
            before=viewport_width()
            page.locator('.cp52-window-controls button[aria-label="Reduce plugin window"]').click()
            page.wait_for_function(
                'before => document.documentElement.clientWidth <= before-24',
                arg=before,timeout=20000)
            smaller=viewport_width()
            page.locator('.cp52-window-controls button[aria-label="Enlarge plugin window"]').click()
            page.wait_for_function(
                'smaller => document.documentElement.clientWidth >= smaller+24',
                arg=smaller,timeout=20000)
            page.locator('.cp52-window-controls button[aria-label="Comfortable 1230 × 820 window"]').click()
            page.wait_for_function("""() => {
                const box=document.querySelector('#presetBrowser .preset-browser-panel').getBoundingClientRect();
                return box.left >= -2 && box.top >= -2
                    && box.right <= document.documentElement.clientWidth+2
                    && box.bottom <= document.documentElement.clientHeight+2;
            }""",timeout=10000)
            # The three CP51 library boundaries must reach the compiled
            # processor through the real indexed factory-program loader.
            page.evaluate("""() => {
                window.cp52ProgramEvents=[];
                window.__JUCE__.backend.addEventListener('albedoProgram',p=>
                    window.cp52ProgramEvents.push(Number(p)));
            }""")
            for library,selector,program in [
                ('Factory','Factory:1',1),
                ('Creative','Creative:127',127),
                ('New','New:191',191)
            ]:
                page.locator('[data-collection="'+library+'"]').click()
                page.locator('#pbItems [data-pb-choose="'+selector+'"]').click()
                page.wait_for_function('program => window.cp52ProgramEvents.includes(program)',
                                       arg=program,timeout=10000)
                assert page.locator('#presetBrowser').is_visible(), (
                    'CP52 audition browser must remain open',library)
            page.keyboard.press('Escape')
            assert not page.locator('#presetBrowser').is_visible()
            print('CP52 compiled CLAP resize buttons, modal clipping, 3 bank boundaries passed')
            page.evaluate("""() => {
                window.cp47Echo = {};
                window.__JUCE__.backend.addEventListener('albedoUpdate',csv=>{
                    for(const pair of csv.split(',')){
                        const [id,v]=pair.split(':');window.cp47Echo[id]=Number(v);
                    }
                });
            }""")
            def check_value(pid, value):
                page.wait_for_function("([id,v])=>Math.abs(window.cp47Echo[id]-v)<0.001",arg=[pid,value],timeout=5000)
            # CP54 deliberately hides secondary controls from the solid main
            # stage. Test their real, visible contextual popups, not the old
            # hidden controls or a retired global Advanced/Synthesis Lab.
            def open_context(context):
                page.locator('[data-cp54-context="'+context+'"]').click()
                page.wait_for_function(
                    'context => window.ALBEDO_CP54_ESSENTIAL?.activeContext === context',
                    arg=context,timeout=10000)
                assert page.locator('#cp54Popover').is_visible()
            def close_context():
                page.locator('#cp54Close').click()
                assert page.locator('#cp54Popover').is_hidden()
            for control,pid,options,context in [
                    ('oversample',9002,[('2×',2),('4×',3)],'settings'),
                    ('filterRoute',3012,[('SERIAL A-B',1),('SERIAL B-A',2)],'filter'),
                    ('cp57Rate',6021,[('0.125',.125),('0.5',.5)],None),
                    ('tuning',3112,[('19-TET',3),('24-TET',4)],'settings')]:
                if context:open_context(context)
                # Filter routing uses a visible proxy that dispatches change
                # to the original native-bound selector.
                locator=(page.locator('#cp54Popover .cp54-choice select')
                    if control=='filterRoute'
                    else page.locator('#'+control))
                if locator.input_value()==options[0][0]:
                    options=list(reversed(options))
                for option,value in options:
                    locator.select_option(option)
                    print('Checking',control,pid,value,flush=True)
                    check_value(pid,value)
                if context:close_context()
            page.locator('#cellArp #play').click()
            check_value(6020,1)
            page.locator('#cellArp #play').click()
            check_value(6020,0)
            page.locator('#cpArpPattern').select_option('11')
            check_value(6031,8)
            page.locator('#cpArpPattern').select_option('2')
            check_value(6031,0)
            dial=page.locator('.couple-item .cp38-dial').first
            box=dial.bounding_box()
            before=float(dial.get_attribute('aria-valuenow'))
            x,y=box['x']+box['width']/2,box['y']+box['height']/2
            page.mouse.move(x,y)
            page.mouse.down()
            direction=1 if before<50 else -1
            page.mouse.move(x,y-40*direction,steps=5)
            page.mouse.up()
            after=float(dial.get_attribute('aria-valuenow'))
            assert abs(after-before)>=25,(before,after)
            check_value(2200,after)
            assert page.locator('.couple-item output').first.inner_text()==str(round(after))+'%'
            def sweep_knob(selector,pid):
                knob=page.locator(selector).first
                knob.focus()
                knob.press('Home')
                knob.press('End')
                page.wait_for_function("id=>Number.isFinite(cp47Echo[id])",arg=pid)
            sweep_knob('.dial[data-key="engine:0"]',1006)
            check_value(1006,100)
            sweep_knob('.dial[data-key="macro:bright"]',2000)
            check_value(2000,100)
            # The Nereida-style engine has four characters. Secondary ARP
            # articulation is intentionally moved to the RHYTHM popup.
            open_context('arp')
            for pid,valid in ((6024,['0','1','2','3']),(6025,['1','2','3']),
                              (6028,[str(n) for n in range(8)])):
                control=page.locator('#cp54Popover [data-arp="'+str(pid)+'"]')
                current=control.input_value()
                option=next(v for v in valid if v!=current)
                control.select_option(option)
                check_value(pid,int(option))
            assert page.locator('#cp54Popover [data-arp="6029"]').count()==0
            close_context()
            xy=page.locator('#xy')
            rect=xy.bounding_box()
            for nx,ny in ((.35,.66),(.71,.28)):
                page.mouse.click(rect['x']+nx*rect['width'],rect['y']+ny*rect['height'])
                page.wait_for_function("([x,y])=>Math.abs(cp47Echo[2100]-x)<4 && Math.abs(cp47Echo[2101]-y)<4",arg=[100*nx,100*ny])
            assert page.locator('#cp04LabOpen').is_hidden()
            open_context('filter')
            sweep_knob('#cp54Popover .cp54-rotary[data-cp54-id="3120"]',3120)
            check_value(3120,100)
            frame=page.locator('#cp54Popover').bounding_box()
            assert frame['width']<=page.evaluate('innerWidth')*.94
            assert frame['height']<=page.evaluate('innerHeight')*.98
            close_context()
            open_context('mod')
            page.locator('#cp54Popover #addRoute').click()
            count=page.locator('#cp54Popover #routes .route-row').count()
            check_value(8300,count)
            close_context()
            print('CP54 native WebView2 contextual source/Corpus/ARP/Routes echo and bounds passed')
            print('CP47 compiled parameter round trips and coupling drag passed')
    finally:
        proc.kill()
        proc.wait()

if __name__=='__main__': main()
