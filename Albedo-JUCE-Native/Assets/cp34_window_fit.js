/* CP34 screen-fit safeguard for the JUCE WebView2 editor.
   Renderer/layout only: no audio thread, parameter writes, or DOM reconstruction. */
(()=>{
 'use strict';
 const root=document.documentElement;
 const page=document.getElementById('page');
 if(!page)return;
 const BASE_W=1320,BASE_H=880;
 let lastW=-1,lastH=-1,lastScale=-1;
 function fit(){
   const cw=Math.max(1,root.clientWidth);
   const ch=Math.max(1,root.clientHeight);
   const vv=window.visualViewport;
   const w=vv?Math.min(cw,Math.max(1,vv.width)):cw;
   const h=vv?Math.min(ch,Math.max(1,vv.height)):ch;
   // Never enlarge an already readable 1320x880 design to fill a huge
   // browser viewport. Always shrink it to the actual visible client area.
   // This also repairs stale CP31 scale after a DAW host resizes its WebView.
   const scale=Math.max(.1,Math.min(1,w/BASE_W,h/BASE_H));
   if(Math.abs(scale-lastScale)>.0005||w!==lastW||h!==lastH){
     lastW=w;lastH=h;lastScale=scale;
     root.style.setProperty('--fit-scale',String(scale));
   }
 }
 window.addEventListener('resize',fit,{passive:true});
 window.visualViewport?.addEventListener('resize',fit,{passive:true});
 if(typeof ResizeObserver==='function'){
   const observer=new ResizeObserver(fit);
   observer.observe(root);
   // Kept for the lifetime of this editor; WebView2 disposal owns teardown.
   window.__ALBEDO_CP34_VIEWPORT_OBSERVER=observer;
 }
 // A composition resize need not trigger an HTML window.resize event in some
 // DAW hosts; an idle geometry check repairs only changed viewport values.
 const watcher=window.setInterval(()=>{if(!document.hidden)fit()},750);
 window.addEventListener('pagehide',()=>window.clearInterval(watcher),{once:true});
 // Do not accidentally magnify an instrument UI via Ctrl+wheel/+/−.
 document.addEventListener('wheel',event=>{if(event.ctrlKey)event.preventDefault()},
                           {capture:true,passive:false});
 document.addEventListener('keydown',event=>{
   if(event.ctrlKey && ['+','=','-','0'].includes(event.key))
     event.preventDefault();
 },{capture:true});
 fit();
 window.ALBEDO_CP34_SCREEN_FIT={
   get dimensions(){return{width:lastW,height:lastH,scale:lastScale}},
   recalculate:fit,
   baseWidth:BASE_W,baseHeight:BASE_H
 };
})();
