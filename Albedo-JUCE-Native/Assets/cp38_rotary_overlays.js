/* CP38: continuous controls as rotary surfaces; stable original range inputs
   still carry all DSP IDs, events and DAW parameter automation. */
(()=>{
'use strict';
const css=document.createElement('style');css.id='cp38-knob-geometry';css.textContent="\n#page .cp38-wrap,#cp04Lab .cp38-wrap{display:flex;align-items:center;justify-content:center;box-sizing:border-box;min-width:0}\n#page input.cp38-native,#cp04Lab input.cp38-native{position:absolute!important;opacity:0!important;width:1px!important;height:1px!important;pointer-events:none!important;margin:0!important;padding:0!important;clip-path:inset(50%)!important}\n.cp38-dial{--knob-size:28px;--angle:-135deg;position:relative;display:block;flex:0 0 var(--knob-size);width:var(--knob-size);height:var(--knob-size);box-sizing:border-box;border-radius:50%;touch-action:none;user-select:none;cursor:ns-resize;outline-offset:3px;border:2px solid #819c89;background:radial-gradient(circle at 36% 26%,#fff9e3,#e0dec6 54%,#aac1b1 83%,#53796c);box-shadow:inset 0 0 0 3px #ecedde,0 2px 4px #203e3360}\n.cp38-dial::after{content:\"\";position:absolute;top:12%;left:calc(50% - 1.5px);width:3px;height:30%;border-radius:3px;background:#284f42;transform-origin:50% 125%;transform:rotate(var(--angle));pointer-events:none}\n.cp38-dial::before{content:\"\";position:absolute;inset:1px;border:2px solid transparent;border-top-color:#c09a62;border-right-color:#c09a62;border-radius:50%;transform:rotate(var(--angle));opacity:.9}\n.cp38-dial:focus-visible{outline:2px solid #bb9856}\n#page .source-card .mini-track .cp38-dial{--knob-size:24px}\n#page .cp37-level-strip .cp38-dial{--knob-size:24px}\n#page .cp37-level-strip .cp38-wrap{height:24px}\n#page .route-row{grid-template-columns:minmax(0,1fr) 13px minmax(0,1fr) 34px 19px!important}\n#page .route-row .cp38-dial{--knob-size:27px}\n#page .couple-item{display:grid!important;grid-template-columns:minmax(0,1fr) 30px;align-items:center;gap:3px;padding:3px 6px!important;min-height:33px!important}\n#page .couple-item .subrow{grid-column:1;grid-row:1;margin:0;display:flex;flex-direction:column;align-items:flex-start;gap:0}\n#page .couple-item .subrow strong{font-size:11px!important}\n#page .couple-item .subrow output{font-size:10px!important}\n#page .couple-item small{display:none!important}\n#page .couple-item .cp38-wrap{grid-column:2;grid-row:1}\n#page .couple-item .cp38-dial{--knob-size:26px}\n#cp04Lab .cp38-wrap{min-height:51px}\n#cp04Lab .cp38-dial{--knob-size:48px}\n";document.head.appendChild(css);
// CP40: contextual visual popups removed. ADSR popup is owned by CP37 HTML.
const map=new WeakMap();let count=0;
const clamp=(v,a,b)=>Math.min(b,Math.max(a,v));
const bounds=i=>({min:Number(i.min||0),max:Number(i.max||100),step:i.step==='any'?0:(Number(i.step)||1)});
function reflect(input){const dial=map.get(input);if(!dial)return;const b=bounds(input),v=clamp(Number(input.value)||0,b.min,b.max);dial.style.setProperty('--angle',(-135+270*(v-b.min)/(b.max-b.min||1))+'deg');dial.setAttribute('aria-valuenow',String(v));dial.title=(dial.getAttribute('aria-label')||'Control')+' '+v.toFixed(2)}
function write(input,value){const b=bounds(input);let v=clamp(value,b.min,b.max);if(b.step)v=b.min+Math.round((v-b.min)/b.step)*b.step;v=Number(clamp(v,b.min,b.max).toFixed(5));if(Number(input.value)===v)return;input.value=String(v);input.dispatchEvent(new Event('input',{bubbles:true}));reflect(input)}
function decorate(input){
 if(map.has(input)||!input.isConnected||input.type!=='range')return;
 const shell=document.createElement('span'),dial=document.createElement('span');
 shell.className='cp38-wrap';dial.className='cp38-dial';dial.tabIndex=0;
 dial.setAttribute('role','slider');dial.setAttribute('aria-label',input.getAttribute('aria-label')||input.id||'Parameter');
 dial.setAttribute('aria-valuemin',input.min||'0');dial.setAttribute('aria-valuemax',input.max||'100');
 input.before(shell);shell.appendChild(dial);shell.appendChild(input);
 input.classList.add('cp38-native');input.setAttribute('aria-hidden','true');input.tabIndex=-1;
 map.set(input,dial);count++;reflect(input);
 let drag=null;
 function notify(type,event){input.dispatchEvent(new PointerEvent(type,{bubbles:true,pointerId:event?.pointerId||1}))}
 dial.addEventListener('pointerdown',e=>{
  if(e.button!==0)return;e.preventDefault();e.stopPropagation();
  const b=bounds(input);drag={x:e.clientX,y:e.clientY,id:e.pointerId,value:Number(input.value),range:b.max-b.min};
  dial.setPointerCapture(e.pointerId);notify('pointerdown',e);
 });
 dial.addEventListener('pointermove',e=>{if(!drag||e.pointerId!==drag.id)return;const change=(drag.y-e.clientY+(e.clientX-drag.x)*.12)*drag.range*(e.shiftKey?.001:.008);write(input,drag.value+change)});
 const stop=e=>{if(!drag||e?.pointerId!==undefined&&e.pointerId!==drag.id)return;drag=null;notify('pointerup',e);input.dispatchEvent(new Event('change',{bubbles:true}))};
 dial.addEventListener('pointerup',stop);dial.addEventListener('pointercancel',stop);dial.addEventListener('lostpointercapture',stop);
 dial.addEventListener('wheel',e=>{e.preventDefault();e.stopPropagation();const b=bounds(input),step=Math.max(b.step||.01,(b.max-b.min)*(e.shiftKey?.004:.025));write(input,Number(input.value)+(e.deltaY<0?step:-step))},{passive:false});
 dial.addEventListener('keydown',e=>{const b=bounds(input),step=Math.max(b.step||.01,(b.max-b.min)*(e.shiftKey?.001:.01));let v=Number(input.value);if(e.key==='ArrowRight'||e.key==='ArrowUp')v+=step;else if(e.key==='ArrowLeft'||e.key==='ArrowDown')v-=step;else if(e.key==='PageUp')v+=10*step;else if(e.key==='PageDown')v-=10*step;else if(e.key==='Home')v=b.min;else if(e.key==='End')v=b.max;else return;e.preventDefault();e.stopPropagation();write(input,v)});
 dial.addEventListener('dblclick',e=>{e.preventDefault();write(input,Number(input.defaultValue))});
 input.addEventListener('input',()=>reflect(input));
}
function scan(node){if(!(node instanceof Element))return;if(node.matches('input[type=range]'))decorate(node);node.querySelectorAll('input[type=range]').forEach(decorate)}
scan(document.getElementById('page'));scan(document.getElementById('cp04Lab'));
const observer=new MutationObserver(records=>{for(const r of records)for(const n of r.addedNodes)scan(n)});
for(const id of ['page','cp04Lab']){const el=document.getElementById(id);if(el)observer.observe(el,{subtree:true,childList:true})}
window.__ALBEDO_CP38_KNOB_OBSERVER=observer;
window.__JUCE__?.backend?.addEventListener?.('albedoUpdate',()=>{document.querySelectorAll('input.cp38-native').forEach(reflect)});
window.ALBEDO_CP38_KNOBS={get converted(){return count},get redraws(){return 0},get visible(){return false},get fullyRotary(){return document.querySelectorAll('input[type=range]:not(.cp38-native)').length===0}};
})();