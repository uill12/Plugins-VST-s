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
                page.wait_for_function("""([w,h])=>{
                    const r=document.querySelector('#page').getBoundingClientRect(),d=devicePixelRatio;
                    return Math.abs(r.right*d-w)<3 && Math.abs(r.bottom*d-h)<3
                        && Math.abs(r.left*d)<3 && Math.abs(r.top*d)<3;
                }""",arg=[w,h],timeout=10000)
                page.locator('#presetBrowseOpen').click()
                assert page.locator('#presetBrowser').is_visible()
                page.keyboard.press('Escape')
                print('Compiled CLAP physical client and editor match',w,h)
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
            for control,pid,options in [('oversample',9002,[('2×',2),('4×',3)]),
                                        ('filterRoute',3012,[('SERIAL A-B',1),('SERIAL B-A',2)]),
                                        ('division',6021,[('125',.125),('500',.5)]),
                                        ('tuning',3112,[('19-TET',3),('24-TET',4)])]:
                for option,value in options:
                    page.locator('#'+control).select_option(option,force=True)
                    check_value(pid,value)
            page.locator('#play').click()
            check_value(6020,1)
            page.locator('#play').click()
            check_value(6020,0)
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
            print('CP47 compiled parameter round trips and coupling drag passed')
    finally:
        proc.kill()
        proc.wait()

if __name__=='__main__': main()
