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
            for w,h in ((900,600),(720,480),(1020,680)):
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
    finally:
        proc.kill()
        proc.wait()

if __name__=='__main__': main()
