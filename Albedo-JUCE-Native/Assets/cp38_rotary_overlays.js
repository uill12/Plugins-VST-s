/* CP38: continuous controls as rotary surfaces; stable original range inputs
   still carry all DSP IDs, events and DAW parameter automation. */
(()=>{
'use strict';
const css=document.createElement('style');css.id='cp38-knob-geometry';css.textContent="\n#page .cp38-wrap,#cp04Lab .cp38-wrap{display:flex;align-items:center;justify-content:center;box-sizing:border-box;min-width:0}\n#page input.cp38-native,#cp04Lab input.cp38-native{position:absolute!important;opacity:0!important;width:1px!important;height:1px!important;pointer-events:none!important;margin:0!important;padding:0!important;clip-path:inset(50%)!important}\n.cp38-dial{--knob-size:28px;--angle:-135deg;position:relative;display:block;flex:0 0 var(--knob-size);width:var(--knob-size);height:var(--knob-size);box-sizing:border-box;border-radius:50%;touch-action:none;user-select:none;cursor:ns-resize;outline-offset:3px;border:2px solid #819c89;background:radial-gradient(circle at 36% 26%,#fff9e3,#e0dec6 54%,#aac1b1 83%,#53796c);box-shadow:inset 0 0 0 3px #ecedde,0 2px 4px #203e3360}\n.cp38-dial::after{content:\"\";position:absolute;top:12%;left:calc(50% - 1.5px);width:3px;height:30%;border-radius:3px;background:#284f42;transform-origin:50% 125%;transform:rotate(var(--angle));pointer-events:none}\n.cp38-dial::before{content:\"\";position:absolute;inset:1px;border:2px solid transparent;border-top-color:#c09a62;border-right-color:#c09a62;border-radius:50%;transform:rotate(var(--angle));opacity:.9}\n.cp38-dial:focus-visible{outline:2px solid #bb9856}\n#page .source-card .mini-track .cp38-dial{--knob-size:24px}\n#page .cp37-level-strip .cp38-dial{--knob-size:24px}\n#page .cp37-level-strip .cp38-wrap{height:24px}\n#page .route-row{grid-template-columns:minmax(0,1fr) 13px minmax(0,1fr) 34px 19px!important}\n#page .route-row .cp38-dial{--knob-size:27px}\n#page .couple-item{display:grid!important;grid-template-columns:minmax(0,1fr) 30px;align-items:center;gap:3px;padding:3px 6px!important;min-height:33px!important}\n#page .couple-item .subrow{grid-column:1;grid-row:1;margin:0;display:flex;flex-direction:column;align-items:flex-start;gap:0}\n#page .couple-item .subrow strong{font-size:11px!important}\n#page .couple-item .subrow output{font-size:10px!important}\n#page .couple-item small{display:none!important}\n#page .couple-item .cp38-wrap{grid-column:2;grid-row:1}\n#page .couple-item .cp38-dial{--knob-size:26px}\n#cp04Lab .cp38-wrap{min-height:51px}\n#cp04Lab .cp38-dial{--knob-size:48px}\n#cp38-visual[hidden]{display:none!important}\n#cp38-visual{pointer-events:none;position:fixed;z-index:12400;width:344px;height:154px;box-sizing:border-box;padding:9px 12px;border:1px solid #ba9667bb;border-radius:9px;color:#2c5649;background:linear-gradient(145deg,#edf2e3ee,#c5ddd5d9);backdrop-filter:blur(12px);-webkit-backdrop-filter:blur(12px);box-shadow:0 8px 24px #173e315b,inset 0 1px #fff9}\n#cp38-visual header{display:flex;justify-content:space-between;align-items:baseline;gap:9px;font:700 11px 'Segoe UI',Arial,sans-serif}\n#cp38-visual header output{font:500 11px Georgia,serif;color:#9d7951}\n#cp38-visual svg{display:block;width:100%;height:100px}\n#cp38-visual footer{font:500 10px 'Segoe UI',Arial,sans-serif;color:#527a70}\n";document.head.appendChild(css);
const pop=document.createElement('div');pop.id='cp38-visual';pop.hidden=true;pop.setAttribute('aria-hidden','true');
pop.innerHTML='<header><strong>ALBEDO / CHARACTER</strong><output id="cp38-value">0</output></header><svg viewBox="0 0 320 100" preserveAspectRatio="none"><path d="M8 25H312 M8 50H312 M8 75H312" fill="none" stroke="#67877c55" stroke-dasharray="4 4"/><path id="cp38-area" fill="#c8ab7260"/><path id="cp38-wave" fill="none" stroke="#aa7e50" stroke-width="2.5" stroke-linecap="round"/></svg><footer id="cp38-kind">LIVE · SONIC SHAPING</footer>';
document.body.appendChild(pop);
const map=new WeakMap();let count=0,redraws=0,active=null,hideTimer=0,raf=0;
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
  dial.setPointerCapture(e.pointerId);notify('pointerdown',e);open(input,dial);
 });
 dial.addEventListener('pointermove',e=>{if(!drag||e.pointerId!==drag.id)return;const change=(drag.y-e.clientY+(e.clientX-drag.x)*.12)*drag.range*(e.shiftKey?.0007:.0035);write(input,drag.value+change);drawSoon()});
 const stop=e=>{if(!drag||e?.pointerId!==undefined&&e.pointerId!==drag.id)return;drag=null;notify('pointerup',e);input.dispatchEvent(new Event('change',{bubbles:true}));hideSoon()};
 dial.addEventListener('pointerup',stop);dial.addEventListener('pointercancel',stop);dial.addEventListener('lostpointercapture',stop);
 dial.addEventListener('wheel',e=>{e.preventDefault();e.stopPropagation();const b=bounds(input),step=Math.max(b.step||.01,(b.max-b.min)*(e.shiftKey?.001:.008));write(input,Number(input.value)+(e.deltaY<0?step:-step));open(input,dial);hideSoon()},{passive:false});
 dial.addEventListener('keydown',e=>{const b=bounds(input),step=Math.max(b.step||.01,(b.max-b.min)*(e.shiftKey?.001:.01));let v=Number(input.value);if(e.key==='ArrowRight'||e.key==='ArrowUp')v+=step;else if(e.key==='ArrowLeft'||e.key==='ArrowDown')v-=step;else if(e.key==='PageUp')v+=10*step;else if(e.key==='PageDown')v-=10*step;else if(e.key==='Home')v=b.min;else if(e.key==='End')v=b.max;else return;e.preventDefault();e.stopPropagation();write(input,v);open(input,dial);hideSoon()});
 dial.addEventListener('dblclick',e=>{e.preventDefault();write(input,Number(input.defaultValue));open(input,dial);hideSoon()});
 dial.addEventListener('focus',()=>open(input,dial));dial.addEventListener('blur',hideSoon);
 input.addEventListener('input',()=>{reflect(input);if(active===input)drawSoon()});
}
function scan(node){if(!(node instanceof Element))return;if(node.matches('input[type=range]'))decorate(node);node.querySelectorAll('input[type=range]').forEach(decorate)}
scan(document.getElementById('page'));scan(document.getElementById('cp04Lab'));
const observer=new MutationObserver(records=>{for(const r of records)for(const n of r.addedNodes)scan(n)});
for(const id of ['page','cp04Lab']){const el=document.getElementById(id);if(el)observer.observe(el,{subtree:true,childList:true})}
window.__ALBEDO_CP38_KNOB_OBSERVER=observer;
function kind(source){
 if(source.dataset?.key?.startsWith('env'))return 'adsr';
 if(source.dataset?.couple!==undefined)return 'COUPLING / ENGINE EXCHANGE';
 if(source.dataset?.route!==undefined)return 'CORRESPONDENTIAE / ROUTING';
 if(source.dataset?.level!==undefined||source.dataset?.key?.startsWith('engine:'))return 'SOURCE / SPECTRAL CHARACTER';
 if(source.dataset?.key?.startsWith('filter'))return 'CORPUS / FILTER RESPONSE';
 if(source.dataset?.key?.startsWith('motion'))return 'MOTUS / CELLULAR';
 if(source.dataset?.key?.startsWith('macro:'))return 'QUINTESSENTIA / SPECTRAL BALANCE';
 if(source.dataset?.key?.startsWith('fx:'))return 'SPATIUM / EFFECT MORPH';
 if(source.id==='cp37Gain'||source.id==='cp37Master')return 'OUTPUT / HEADROOM';
 return 'SYNTHESIS / MATERIAL CHARACTER';
}
function open(source,anchor){if(kind(source)==='adsr')return;active=source;clearTimeout(hideTimer);const rect=(anchor||source).getBoundingClientRect(),vw=document.documentElement.clientWidth,vh=document.documentElement.clientHeight,w=Math.min(344,vw-16),h=154;pop.style.width=w+'px';pop.style.left=Math.round(clamp(rect.left+rect.width/2-w/2,8,Math.max(8,vw-w-8)))+'px';let y=rect.top-h-10;if(y<8)y=rect.bottom+10;pop.style.top=Math.round(clamp(y,8,Math.max(8,vh-h-8)))+'px';pop.hidden=false;drawSoon()}
function hideSoon(){clearTimeout(hideTimer);hideTimer=setTimeout(()=>{active=null;pop.hidden=true},950)}
function drawSoon(){if(!raf)raf=requestAnimationFrame(draw)}
function draw(){raf=0;if(pop.hidden||!active)return;const v=Number(active.value??active.getAttribute('aria-valuenow'))||0,k=kind(active),t=Math.abs(v)/100;pop.querySelector('strong').textContent=k;pop.querySelector('#cp38-value').textContent=v.toFixed(1);pop.querySelector('footer').textContent=k+' · LIVE';let d='M8 50';for(let i=0;i<60;i++){const x=8+i*5.1;const amp=k.includes('FILTER')?((i<30+t*20)?22:5):18+22*t;const y=50+amp*Math.sin(i*.55+v*.015)*(k.includes('CELLULAR')?Math.cos(i*.13):1);d+=' L'+x.toFixed(1)+' '+y.toFixed(1)}pop.querySelector('#cp38-wave').setAttribute('d',d);pop.querySelector('#cp38-area').setAttribute('d',d+' L312 91 L8 91 Z');redraws++}
document.addEventListener('pointerdown',e=>{const d=e.target.closest?.('.dial[data-key],.cp-knob[data-cp-id]');if(d&&kind(d)!=='adsr')open(d,d)},true);
document.addEventListener('wheel',e=>{const d=e.target.closest?.('.dial[data-key],.cp-knob[data-cp-id]');if(d&&kind(d)!=='adsr'){open(d,d);hideSoon()}},{capture:true,passive:true});
document.addEventListener('pointerup',hideSoon);document.addEventListener('pointercancel',hideSoon);
window.__JUCE__?.backend?.addEventListener?.('albedoUpdate',()=>{document.querySelectorAll('input.cp38-native').forEach(reflect);if(!pop.hidden)drawSoon()});
window.ALBEDO_CP38_KNOBS={get converted(){return count},get redraws(){return redraws},get visible(){return !pop.hidden},get fullyRotary(){return document.querySelectorAll('input[type=range]:not(.cp38-native)').length===0}};
})();