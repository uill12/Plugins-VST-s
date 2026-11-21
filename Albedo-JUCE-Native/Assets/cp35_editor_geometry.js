/* CP35 — JUCE host window and HTML render use one 3:2 geometry.
   Replaces CP34's conflicting viewport fit writer. No DOM reconstruction,
   no audio-thread calls, and no mouse-move repaint except active resizing. */
(()=>{
 'use strict';
 const root=document.documentElement;
 const page=document.getElementById('page');
 if(!page)return;
 let nativeW=0,nativeH=0,lastW=-1,lastH=-1,lastScale=-1;
 let lastScaleX=-1,lastScaleY=-1;
 let frame=0,physicalWidth=0,physicalHeight=0;
 const clampWidth=w=>Math.max(720,Math.min(1500,Math.round(w/6)*6));
 const send=(w,h)=>{
   const width=clampWidth(w),height=width*2/3;
   const payload='resize|'+width+'|'+height;
   window.__JUCE__?.backend?.emitEvent?.('albedoMessage',{payload});
 };
 function measured(){
   // CP40: root.clientWidth/clientHeight are the actual WebView2 CSS viewport.
   // JUCE HWND logical dimensions and visualViewport can report different units
   // under Windows DPI scaling / host embedding. Never take their minimum:
   // it created a reduced canvas with empty borders in Bitwig.
   let w=Math.max(1,Math.floor(root.clientWidth||window.innerWidth||1));
   let h=Math.max(1,Math.floor(root.clientHeight||window.innerHeight||1));
   // CP55: a real Bitwig screenshot showed the 1500x1000 design canvas clipped
   // inside a significantly smaller granted host window. In unusual WebView2
   // embed/DPI cases root.clientWidth can describe the content layout instead
   // of the host client rectangle. Use the JUCE-reported host dimensions ONLY
   // for large, unambiguous over-reporting (>16% on either axis).
   // Small disagreements are deliberately left alone: CP40 established that
   // always taking min(css, native) causes whitespace on normal Windows DPI.
   const hostValid=nativeW>=480 && nativeH>=320;
   const mismatch=hostValid&&(w>nativeW*1.16 || h>nativeH*1.16);
   if(mismatch){
     const hostRatio=nativeW/nativeH;
     if(hostRatio>1.35 && hostRatio<1.65){
       w=Math.min(w,nativeW);h=Math.min(h,nativeH);
     }
   }
   return {w,h};
 }
 function fit(){
   const {w,h}=measured();
   const pw=Math.max(1,page.offsetWidth),ph=Math.max(1,page.offsetHeight);
   const sx=Math.max(.1,w/pw),sy=Math.max(.1,h/ph);
   const scale=Math.min(sx,sy);
   // Fill the granted WebView2 client rectangle, including small DPI / Bitwig
   // geometry disagreements. Preserve undistorted content for large ratio gaps.
   const mismatch=Math.abs(sx-sy)/Math.max(.1,scale);
   const scaleX=mismatch<=.075?sx:scale,scaleY=mismatch<=.075?sy:scale;
   if(w===lastW&&h===lastH&&Math.abs(lastScaleX-scaleX)<.0002
      &&Math.abs(lastScaleY-scaleY)<.0002)return;
   lastW=w;lastH=h;lastScale=scale;lastScaleX=scaleX;lastScaleY=scaleY;
   root.style.setProperty('--albedo-viewport-w',w+'px');
   root.style.setProperty('--albedo-viewport-h',h+'px');
   root.style.setProperty('--albedo-offset-x',Math.max(0,(w-pw*scaleX)/2)+'px');
   root.style.setProperty('--albedo-offset-y',Math.max(0,(h-ph*scaleY)/2)+'px');
   // CP36: the Atlas uses most of the available editor area. Removing the
   // secondary filter row adds room for readable factory-patch list entries.
   const atlasW=Math.min(960,Math.floor(w*.96),Math.max(320,w-22));
   const atlasH=Math.min(642,Math.floor(h*.96),Math.max(260,h-18));
   root.style.setProperty('--albedo-atlas-width',atlasW+'px');
   root.style.setProperty('--albedo-atlas-height',atlasH+'px');
   root.style.setProperty('--fit-scale',String(scale));
   root.style.setProperty('--fit-scale-x',String(scaleX));
   root.style.setProperty('--fit-scale-y',String(scaleY));
   const sizeDisplay=document.getElementById('albedo-cp35-size-value');
   if(sizeDisplay)sizeDisplay.textContent=Math.round((nativeW||w))+'×'+Math.round(nativeH||h);
 }
 function enqueue(){
   if(frame)return;
   frame=requestAnimationFrame(()=>{frame=0;fit()});
 }
 const style=document.createElement('style');
 style.id='albedo-cp35-style';
 style.textContent=`
 html.albedo-fit body #page.page{
   position:fixed!important;margin:0!important;transform-origin:0 0!important;
   transform:scale(var(--fit-scale-x,var(--fit-scale,1)),var(--fit-scale-y,var(--fit-scale,1)))!important;
   left:var(--albedo-offset-x,0px)!important;
   top:var(--albedo-offset-y,0px)!important;
 }
 html.albedo-fit .preset-browser-backdrop{
   position:fixed!important;inset:0 auto auto 0!important;
   width:var(--albedo-viewport-w,100vw)!important;
   height:var(--albedo-viewport-h,100vh)!important;
   max-width:none!important;max-height:none!important;
   padding:10px!important;box-sizing:border-box!important;
   display:flex;align-items:center;justify-content:center;
   overflow:hidden!important;
 }
 html.albedo-fit .preset-browser-backdrop[hidden]{display:none!important}
 html.albedo-fit .preset-browser-panel{
   width:var(--albedo-atlas-width,1200px)!important;
   max-width:calc(var(--albedo-viewport-w,1320px) - 20px)!important;
   height:var(--albedo-atlas-height,795px)!important;
   max-height:calc(var(--albedo-viewport-h,880px) - 20px)!important;
   min-height:0!important;
   box-sizing:border-box!important;
   display:flex!important;flex-direction:column!important;overflow:hidden!important;
 }
 html.albedo-fit .preset-browser-titlebar{
   flex:0 0 auto!important;min-height:54px!important;
   padding:7px 14px!important;gap:10px!important;
 }
 html.albedo-fit .preset-browser-mark{
   width:38px!important;height:38px!important;flex-basis:38px!important;font-size:22px!important;
 }
 html.albedo-fit .preset-browser-heading h2{
   font-size:22px!important;margin:2px 0!important;line-height:1.06!important;
 }
 html.albedo-fit .preset-browser-heading p{
   font-size:10px!important;line-height:1.25!important;
 }
 html.albedo-fit .preset-browser-collections{
   padding:6px 12px!important;flex:0 0 auto!important;min-height:0!important;
 }
 html.albedo-fit .preset-browser-toolbar{
   display:grid!important;grid-template-columns:minmax(0,1.5fr) repeat(3,minmax(0,1fr))!important;
   gap:6px!important;padding:7px 12px!important;
   flex:0 0 auto!important;
 }
 html.albedo-fit .preset-browser-toolbar label{min-width:0!important;gap:3px!important}
 html.albedo-fit .preset-browser-toolbar input,
 html.albedo-fit .preset-browser-toolbar select{
   height:35px!important;min-height:35px!important;
   font-size:12px!important;padding:5px 8px!important;
 }
 html.albedo-fit .preset-browser-toolbar label span{font-size:9px!important;letter-spacing:.07em!important}
 html.albedo-fit .preset-browser-summary{
   flex:0 0 auto!important;padding:5px 12px!important;gap:6px!important;
   min-height:30px!important;
 }
 html.albedo-fit .preset-browser-results{
   flex:1 1 auto!important;min-height:0!important;overflow-y:auto!important;
   padding:0 12px 7px!important;overscroll-behavior:contain;
 }
 html.albedo-fit .pb-column-headers,html.albedo-fit .pb-item{
   grid-template-columns:minmax(0,2.3fr) minmax(0,1fr) minmax(0,1fr) 30px!important;
   gap:8px!important;
 }
 html.albedo-fit .pb-item{
   min-height:43px!important;margin-bottom:4px!important;padding:7px 10px!important;
 }
 html.albedo-fit .pb-item-name{font-size:14px!important;line-height:1.2!important}
 html.albedo-fit .pb-item-family,html.albedo-fit .pb-item-role{font-size:12px!important}
 html.albedo-fit .preset-browser-footer{
   flex:0 0 auto!important;min-height:34px!important;
   padding:6px 12px!important;flex-wrap:nowrap!important;gap:8px!important;overflow:hidden!important;
 }
 html.albedo-fit .preset-browser-footer *{font-size:10px!important}
 html.albedo-fit .pb-matrix-scroller{
   max-height:100%!important;min-height:0!important;overflow:auto!important;
 }
 html.albedo-fit #presetBrowser #pbResultContainer{overflow:auto!important}
 html.albedo-fit #presetBrowser #pbItems:not([hidden]){display:block!important;height:auto!important;overflow:visible!important}
 html.albedo-fit #presetBrowser .pb-item, html.albedo-fit #presetBrowser .pb-column-headers{
   display:grid!important;grid-template-columns:minmax(0,2fr) repeat(2,minmax(0,1fr)) 32px!important;
   align-items:center;gap:8px!important;text-align:left;padding:5px 10px!important;
 }
 html.albedo-fit #presetBrowser .pb-item{height:auto!important;min-height:32px!important;border-radius:0!important}
 html.albedo-fit #presetBrowser .pb-item-name{display:block;font-size:14px!important}
 html.albedo-fit #presetBrowser .pb-item-family,html.albedo-fit #presetBrowser .pb-item-role{font-size:12px!important}
 html.albedo-fit #presetBrowser .pb-item-index{position:static;font-size:11px!important}
 html.albedo-fit #presetBrowser :is(#pbListMode,#pbMatrixMode){display:none!important}
 #albedo-cp35-controls{
   margin-right:max(0px,calc(100vw - var(--albedo-viewport-w,100vw)));
   margin-bottom:max(0px,calc(100vh - var(--albedo-viewport-h,100vh)));
   position:fixed;right:8px;bottom:8px;left:auto;top:auto;
   width:153px;height:29px;z-index:12000;
   display:flex;align-items:center;gap:3px;
   padding:2px 4px;box-sizing:border-box;
   border:1px solid #829b87;border-radius:5px;
   background:#edf0dc;box-shadow:0 2px 5px #0003;
   color:#2b5148;font:10px 'Segoe UI',sans-serif;
 }
 #albedo-cp35-controls button{
   width:25px;height:23px;flex:0 0 auto;cursor:pointer;
   border:1px solid #adbbac;border-radius:3px;background:#dae6d8;color:#22463e;
   font:bold 12px 'Segoe UI',sans-serif;padding:0;
 }
 #albedo-cp35-controls button:focus-visible{outline:2px solid #487c86}
 #albedo-cp35-size-value{flex:1;text-align:center;font-size:10px;white-space:nowrap}
 #albedo-cp35-resize-grip{width:22px!important;cursor:nwse-resize!important;touch-action:none;
   background:repeating-linear-gradient(135deg,#c9dbca 0 3px,#315b53 3px 4px,#c9dbca 4px 7px)!important}
 body:has(.preset-browser-backdrop:not([hidden])) #albedo-cp35-controls{display:none}
 @media(max-width:760px){
   html.albedo-fit .preset-browser-toolbar{grid-template-columns:repeat(2,minmax(0,1fr))!important}
   html.albedo-fit .preset-browser-footer{flex-wrap:wrap!important}
 }
 @media(max-height:640px){
   html.albedo-fit .preset-browser-titlebar{min-height:45px!important;padding:4px 9px!important}
   html.albedo-fit .preset-browser-heading h2{font-size:18px!important}
   html.albedo-fit .preset-browser-heading p{display:none!important}
   html.albedo-fit .preset-browser-collections{padding:3px 9px!important}
   html.albedo-fit .preset-browser-summary{min-height:24px!important;padding:3px 9px!important}
 }
 `;
 document.head.appendChild(style);
 const controls=document.createElement('div');
 controls.id='albedo-cp35-controls';
 controls.setAttribute('aria-label','Resize the Albedo plugin window');
 controls.innerHTML='<button type="button" id="albedo-cp35-smaller" title="Smaller 3:2 window">−</button>'+
   '<span id="albedo-cp35-size-value">1080×720</span>'+
   '<button type="button" id="albedo-cp35-reset" title="Request comfortable 1080×720 (limited by available screen area)">↔</button>'+
   '<button type="button" id="albedo-cp35-resize-grip" title="Drag to resize the 3:2 plugin window">◢</button>';
 document.body.appendChild(controls);
 function ask(width){
   const w=clampWidth(width);
   send(w,w*2/3);
 }
 controls.querySelector('#albedo-cp35-smaller').addEventListener('click',()=>{
   ask((nativeW||measured().w)-36);
 });
 controls.querySelector('#albedo-cp35-reset').addEventListener('click',()=>ask(1080));
 const grip=controls.querySelector('#albedo-cp35-resize-grip');
 let start=null;
 grip.addEventListener('pointerdown',event=>{
   if(event.button!==0)return;
   event.preventDefault();
   const w=nativeW||measured().w;
   start={x:event.clientX,y:event.clientY,w,requested:w,pointerId:event.pointerId};
   grip.setPointerCapture?.(event.pointerId);
 });
 grip.addEventListener('pointermove',event=>{
   if(!start || event.pointerId!==start.pointerId)return;
   const dx=event.clientX-start.x,dy=(event.clientY-start.y)*1.5;
   const delta=Math.abs(dx)>=Math.abs(dy)?dx:dy;
   start.requested=clampWidth(start.w+delta*.7);
   const sizeDisplay=document.getElementById('albedo-cp35-size-value');
   if(sizeDisplay)sizeDisplay.textContent=start.requested+'×'+(start.requested*2/3);
   // Do not move the host's HWND while its WebView2 pointer capture is active.
   // A single request on release survives Bitwig's native window negotiation.
 });
 grip.addEventListener('pointerup',event=>{
   if(!start || event.pointerId!==start.pointerId)return;
   const width=start.requested;
   start=null;
   if(grip.hasPointerCapture?.(event.pointerId))grip.releasePointerCapture(event.pointerId);
   ask(width);
 });
 grip.addEventListener('pointercancel',event=>{
   if(!start || event.pointerId!==start.pointerId)return;
   start=null;fit();
 });
 if(window.__JUCE__?.backend?.addEventListener){
   window.__JUCE__.backend.addEventListener('albedoViewport',payload=>{
     const [w,h,pw,ph]=String(payload).split(',').map(Number);
     if(Number.isFinite(w)&&Number.isFinite(h)&&w>=320&&h>=240){
       nativeW=w;nativeH=h;physicalWidth=pw||0;physicalHeight=ph||0;enqueue();
     }
   });
 }
 window.addEventListener('resize',enqueue,{passive:true});
 window.visualViewport?.addEventListener('resize',enqueue,{passive:true});
 if(typeof ResizeObserver==='function'){
   const observer=new ResizeObserver(enqueue);
   observer.observe(root);
   observer.observe(page);
   window.__ALBEDO_CP35_RESIZE_OBSERVER=observer;
 }
 // ResizeObserver, window events and host viewport events update fit without polling.
 fit();
 window.ALBEDO_CP35_WINDOW={
   get geometry(){return{width:lastW,height:lastH,scale:lastScale,nativeWidth:nativeW,nativeHeight:nativeH}},
   setHostGeometry(w,h){nativeW=w;nativeH=h;fit()},
   requestSize:ask,
   fit
 };
})();
