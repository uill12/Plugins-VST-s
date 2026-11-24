/* CP56 sound-first density. All controls map to existing JUCE/CP04 parameter IDs.
   Stage geometry remains CP35/CP55; no page redraw or polling. */
(()=>{
'use strict';
const page=document.getElementById('page'),api=window.ALBEDO_CP04;
if(!page||!api||!window.ALBEDO_CP54_ESSENTIAL||document.getElementById('cp56Source'))return;
const $=s=>document.querySelector(s), registry=new Map(api.registry.parameters.map(p=>[p.id,p]));
const css=[
'html.albedo-fit #page #engineKnobs{display:grid!important;grid-template-columns:repeat(4,minmax(0,1fr))!important;gap:2px!important;min-height:65px!important;margin:0!important}',
'html.albedo-fit #page #engineKnobs>.param:nth-child(n+3){display:flex!important}',
'html.albedo-fit #page #engineKnobs>.param{min-height:65px!important;max-height:67px!important;min-width:0!important}',
'html.albedo-fit #page #engineKnobs .dial{width:40px!important;height:40px!important;min-width:40px!important;flex-basis:40px!important}',
'html.albedo-fit #page #engineKnobs .dial:after{height:12px!important;top:4px!important;transform-origin:50% 16px!important}',
'html.albedo-fit #page #engineKnobs .param-label{font-size:9.4px!important;line-height:1.04!important;max-height:20px!important;margin-top:2px!important}',
'html.albedo-fit #page #engineKnobs .param-value{font-size:10px!important}',
'html.albedo-fit #page .source-panel .inspector-body>.divider,html.albedo-fit #page .source-panel .inspector-body>.section-caption{display:none!important}',
'html.albedo-fit #page #layerKnobs{display:grid!important;grid-template-columns:repeat(3,minmax(0,1fr))!important;gap:2px!important;min-height:62px!important;margin:0!important}',
'html.albedo-fit #page #layerKnobs>.param{min-height:62px!important;max-height:64px!important}',
'html.albedo-fit #page #layerKnobs .dial{width:34px!important;height:34px!important;min-width:34px!important;flex-basis:34px!important}',
'html.albedo-fit #page #layerKnobs .param-label{font-size:9.5px!important}',
'html.albedo-fit #page #layerKnobs .param-value{font-size:10px!important}',
'html.albedo-fit #page .source-panel .inspector-body{gap:3px!important;padding:5px 8px!important}',
'html.albedo-fit #page .source-panel .inspector-body>select{margin-bottom:1px!important}',
'html.albedo-fit #page .cp54-strong-selectors{margin:2px 0!important;gap:3px!important;min-height:22px!important}',
'html.albedo-fit #page .cp54-strong-selectors select{margin:0!important;height:23px!important;min-height:23px!important;padding:2px!important}',
'html.albedo-fit #page #macros{display:grid!important;grid-template-columns:repeat(7,minmax(0,1fr))!important;gap:2px!important}',
'html.albedo-fit #page #macros>.param:nth-child(n+5){display:flex!important}',
'html.albedo-fit #page #macros>.param{min-height:62px!important;max-height:64px!important;min-width:0!important}',
'html.albedo-fit #page #macros .dial{width:36px!important;height:36px!important;min-width:36px!important;flex-basis:36px!important}',
'html.albedo-fit #page #macros .dial:after{height:11px!important;top:4px!important;transform-origin:50% 15px!important}',
'html.albedo-fit #page #macros .param-label{font-size:9px!important;line-height:1.06!important;margin-top:2px!important}',
'html.albedo-fit #page #macros .param-value{font-size:10px!important}',
'html.albedo-fit #page #cp56Route{width:86px!important;max-width:86px!important;height:23px!important;min-height:23px!important;margin:0!important;padding:1px 3px!important;font:800 8.8px Arial,sans-serif;background:#e4ebdc;color:#3c654f;border:1px solid #9eb4a0;border-radius:4px}',
'html.albedo-fit #page .shape-panel .shape-block:nth-child(3) .shape-head{grid-template-columns:minmax(0,1fr) 86px!important}',
'html.albedo-fit #page #envKnobs{display:grid!important;grid-template-columns:repeat(4,minmax(0,1fr))!important;gap:1px!important;height:64px!important;max-height:64px!important}',
'html.albedo-fit #page #envKnobs>.param:nth-child(2){display:flex!important}',
'html.albedo-fit #page #envKnobs>.param{min-height:62px!important;max-height:64px!important}',
'html.albedo-fit #page #envKnobs .dial{width:34px!important;height:34px!important;min-width:34px!important;flex-basis:34px!important}',
'html.albedo-fit #page #envKnobs .param-label{font-size:9.2px!important}',
'html.albedo-fit #page #envKnobs .param-value{font-size:10px!important}',
'html.albedo-fit #page .cp56-grid{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:2px;min-width:0;align-items:start}',
'html.albedo-fit #page .cp56-param{min-width:0;min-height:59px;display:flex;flex-direction:column;align-items:center;justify-content:flex-start;gap:1px;text-align:center;color:#496b58}',
'html.albedo-fit #page .cp56-caption{font:800 8px/10px Arial,sans-serif;letter-spacing:.06em;color:#698473;margin:1px 0 2px}',
'html.albedo-fit #page .cp56-rotary{display:block;position:relative;box-sizing:border-box;width:32px;height:32px;flex:0 0 32px;border:2px solid #829e88;border-radius:50%;cursor:ns-resize;touch-action:none;outline-offset:1px;background:radial-gradient(circle at 30% 23%,#f9f3df,#d0decf 61%,#86a58f);box-shadow:inset 0 0 0 2px #f1edda,0 1px 3px #31513d3e}',
'html.albedo-fit #page .cp56-rotary::after{content:"";position:absolute;top:4px;left:calc(50% - 1.5px);width:3px;height:10px;background:#a77e4e;border-radius:2px;transform-origin:50% 12px;transform:rotate(var(--cp56-angle,-135deg))}',
'html.albedo-fit #page .cp56-rotary:focus-visible{outline:2px solid #b18c54}',
'html.albedo-fit #page .cp56-param .cp56-label{font:700 9px/10px Arial,sans-serif;white-space:nowrap;max-width:100%;overflow:hidden;text-overflow:ellipsis}',
'html.albedo-fit #page .cp56-param output{font:600 10px/11px Georgia,serif;font-variant-numeric:tabular-nums}',
'html.albedo-fit #page #cp56Source{flex:0 0 auto;min-height:68px!important}',
'html.albedo-fit #page #cp56Envelope{min-height:63px!important;margin-top:1px}',
'html.albedo-fit #page #cp56Envelope .cp56-caption,html.albedo-fit #page #cp56Arp .cp56-caption{margin:0 0 1px}',
'html.albedo-fit #page #cp56Envelope .cp56-param,html.albedo-fit #page #cp56Arp .cp56-param{min-height:54px}',
'html.albedo-fit #page #cp56Envelope .cp56-rotary,html.albedo-fit #page #cp56Arp .cp56-rotary{width:28px;height:28px;flex-basis:28px}',
'html.albedo-fit #page #cp56Envelope .cp56-label,html.albedo-fit #page #cp56Arp .cp56-label{font-size:8.5px}',
'html.albedo-fit #page #cp56Arp:not(.internal) [data-cp56-id="6029"]{display:none!important}',
'html.albedo-fit #page #cp56Arp .cp56-grid{grid-template-columns:repeat(5,minmax(0,1fr));gap:4px;padding:0 4px}',
'html.albedo-fit #page #cp56Arp .cp56-param{display:grid;grid-template-rows:28px 12px 13px;justify-items:center;align-items:center;align-content:start;justify-content:stretch;gap:2px}',
'html.albedo-fit #page #cp56Arp{margin-top:2px;min-height:59px}',
'html.albedo-fit #page #fx .fx-knob-area{display:flex!important;flex-direction:row!important;align-items:center!important;justify-content:space-around!important;gap:2px!important;overflow:hidden!important}',
'html.albedo-fit #page #fx .fx-knob-area>.param{min-width:0!important;flex:1 1 54%!important;max-width:56%!important}',
'html.albedo-fit #page #fx .cp56-fx-param{min-height:34px!important;max-height:34px;flex:1 1 44%;display:grid;grid-template-columns:27px minmax(0,1fr);grid-template-rows:17px 17px;gap:0 3px;justify-items:start;text-align:left}',
'html.albedo-fit #page #fx .cp56-fx-param .cp56-rotary{grid-row:1 / 3;grid-column:1;width:25px;height:25px;flex-basis:25px;align-self:center}',
'html.albedo-fit #page #fx .cp56-fx-param .cp56-rotary::after{height:8px;top:3px;transform-origin:50% 10px}',
'html.albedo-fit #page #fx .cp56-fx-param .cp56-label{grid-column:2;grid-row:1;align-self:end;font-size:8.5px}',
'html.albedo-fit #page #fx .cp56-fx-param output{grid-column:2;grid-row:2;align-self:start;font-size:9px}',
'#page.nigredo .cp56-param{color:#d9e7d6!important}',
'#page.nigredo .cp56-caption{color:#b5cdb9!important}',
'#page.nigredo .cp56-rotary{border-color:#87a798!important;background:radial-gradient(circle at 30% 23%,#8da89a,#466c5e 70%,#254438)!important}'
];
const style=document.createElement('style');style.id='cp56-sound-impact-style';style.textContent=css.join('\n');document.head.appendChild(style);
function dock(id,title,parent,after){
 const box=document.createElement('section');box.id=id;box.setAttribute('aria-label',title);
 const head=document.createElement('p');head.className='cp56-caption';head.textContent=title;
 const grid=document.createElement('div');grid.className='cp56-grid';box.append(head,grid);
 if(after)after.after(box);else parent?.appendChild(box);
 return box;
}
const source=dock('cp56Source','TIMBRE · TRANSIENT · FINE PITCH',null,$('#engineKnobs'));
const env=dock('cp56Envelope','FILTER ENVELOPE · DEPTH & MOVEMENT',null,$('#envKnobs'));
const arp=dock('cp56Arp','PULSUS · NOTE ARTICULATION',$('#cellArp .arp-performance'));
function value(id){const p=registry.get(id);return Number(api.state.cp04[id]??p?.default??0)}
const clamp=(v,a,b)=>Math.max(a,Math.min(b,v));
function format(p,v){
 if(p.unit==='s')return v<1?Math.round(v*1000)+' ms':v.toFixed(v>=10?1:2)+' s';
 if(p.unit==='ms')return Math.round(v)+' ms';
 if(p.unit==='Hz')return v>=1000?(v/1000).toFixed(1)+'k':v.toFixed(v<10?2:1);
 if(p.unit==='ct')return Math.round(v)+' ct';
 if(p.unit==='%'||p.unit==='BPM')return Math.round(v)+(p.unit==='%'?'%':' bpm');
 if(p.unit==='st')return (v>0?'+':'')+v.toFixed(1)+' st';
 return p.stepped?String(Math.round(v)):v.toFixed(Math.abs(v)<10?2:1);
}
function sync(tile){
 const id=+tile.dataset.cp56Id,p=registry.get(id);if(!p)return;
 const v=value(id),f=clamp((v-p.min)/Math.max(1e-7,p.max-p.min),0,1);
 const dial=tile.querySelector('.cp56-rotary');
 dial.style.setProperty('--cp56-angle',(-135+270*f)+'deg');
 dial.setAttribute('aria-valuenow',String(v));dial.setAttribute('aria-valuetext',format(p,v));
 tile.querySelector('output').textContent=format(p,v);
}
function bind(tile,id,short){
 const p=registry.get(id);if(!p)return;
 tile.dataset.cp56Id=String(id);
 const dial=tile.querySelector('.cp56-rotary'),label=tile.querySelector('.cp56-label');
 dial.setAttribute('aria-label',p.name);dial.setAttribute('aria-valuemin',String(p.min));
 dial.setAttribute('aria-valuemax',String(p.max));dial.title=p.name;
 label.textContent=short||p.name;label.title=p.name;sync(tile);
}
function create(id,short){
 if(!registry.has(id))return null;
 const tile=document.createElement('div');tile.className='cp56-param';
 const dial=document.createElement('div');dial.className='cp56-rotary';
 dial.tabIndex=0;dial.setAttribute('role','slider');
 const label=document.createElement('span');label.className='cp56-label';
 tile.append(dial,label,document.createElement('output'));bind(tile,id,short);return tile;
}
function insert(box,items){
 const grid=box.querySelector('.cp56-grid');
 grid.replaceChildren(...items.map(([id,short])=>create(id,short)).filter(Boolean));
}
const original=[[14,'TRANSIENT'],[19,'SUB LEVEL'],[20,'PHASE'],[21,'FINE TUNE']];
function syncSource(){
 const engine=api.state.engines[api.state.selected];if(!engine)return;
 const base=1000+32*api.state.selected,method=engine.method.split(' / ')[0];
 let items=original;
 if(['Analog','Wavetable','Phase Distortion','Generative'].includes(method))
   items=[[12,'DETUNE'],[13,'SPREAD'],[14,'TRANSIENT'],[21,'FINE TUNE']];
 else if(method==='Harmonic')
   items=[[16,'PARTIALS'],[17,'SPECTRAL TILT'],[14,'TRANSIENT'],[21,'FINE TUNE']];
 const grid=source.querySelector('.cp56-grid');
 if(grid.children.length!==items.length)insert(source,items.map(([off,label])=>[base+off,label]));
 else items.forEach(([off,label],i)=>bind(grid.children[i],base+off,label));
}
insert(env,[[3120,'A ENV ±'],[3121,'B ENV ±'],[3122,'ENV ATTACK'],[3123,'ENV DECAY']]);
insert(arp,[[6023,'GATE'],[6026,'SWING'],[6025,'OCTAVES'],[6027,'STEPS'],[6028,'ROTATE'],[6029,'TEMPO']]);
syncSource();
// Visual order is audible influence, not factory ID order. DOM nodes are reused:
// the original dial listeners and JUCE automation remain fully intact.
const macros=$('#macros');
function orderMacros(){
 if(!macros||macros.children.length!==7)return;
 const order=['bright','contour','impact','exchange','width','growth','memory'];
 const nodes=[...macros.children];
 const byKey=new Map(nodes.map(el=>[el.querySelector('.dial[data-key]')?.dataset.key,el]));
 const sorted=order.map(k=>byKey.get('macro:'+k));
 if(sorted.some(x=>!x)||sorted.every((x,i)=>x===nodes[i]))return;
 macros.append(...sorted);
}
orderMacros();
if(macros)new MutationObserver(orderMacros).observe(macros,{childList:true});
// Filter topology radically changes timbre: make this existing DSP selector primary.
const route=$('#filterRoute'),bodyHead=$('.shape-panel .shape-block:nth-child(3) .shape-head');
let routeView=null;
if(route&&bodyHead){
 routeView=document.createElement('select');routeView.id='cp56Route';
 routeView.setAttribute('aria-label','Filter routing');
 routeView.title='Filter routing · parallel / serial A→B / serial B→A';
 for(const option of route.options)routeView.add(option.cloneNode(true));
 routeView.value=route.value;
 routeView.addEventListener('change',()=>{
  route.value=routeView.value;
  route.dispatchEvent(new Event('change',{bubbles:true}));
 });
 bodyHead.appendChild(routeView);
 route.addEventListener('change',()=>{routeView.value=route.value});
}
const fx=$('#fx'),fxMap=[[5012,'BIAS'],[5008,'RATE'],[5011,'SIZE'],null];
function syncFx(){
 const cards=fx?[...fx.querySelectorAll('.fx-card')]:[];
 cards.forEach((card,i)=>{
  const cfg=fxMap[i],area=card.querySelector('.fx-knob-area');
  if(!cfg||!area)return;
  let tile=area.querySelector('.cp56-fx-param');
  if(!tile){tile=create(cfg[0],cfg[1]);if(!tile)return;tile.classList.add('cp56-fx-param');area.appendChild(tile)}
  else bind(tile,cfg[0],cfg[1]);
 });
}
syncFx();
if(fx)new MutationObserver(syncFx).observe(fx,{childList:true});
if($('#engineKnobs'))new MutationObserver(()=>queueMicrotask(syncSource)).observe($('#engineKnobs'),{childList:true});
function edit(id,v){
 const p=registry.get(id);if(!p||!Number.isFinite(v))return;
 const next=clamp(p.stepped?Math.round(v):v,p.min,p.max);
 if(Math.abs(next-value(id))<1e-8)return;
 api.setExtraParam?.(id,next);
 page.querySelectorAll('[data-cp56-id="'+id+'"]').forEach(sync);
}
let pending=0;
function refresh(){
 if(pending)return;
 pending=requestAnimationFrame(()=>{
  pending=0;syncSource();
  page.querySelectorAll('.cp56-param[data-cp56-id]').forEach(sync);
 });
}
document.addEventListener('change',e=>{if(e.target.id==='method')queueMicrotask(syncSource)});
document.addEventListener('click',e=>{if(e.target.closest?.('[data-select],.node,[data-pb-choose]'))queueMicrotask(syncSource)});
window.__JUCE__?.backend?.addEventListener?.('albedoUpdate',refresh);
window.__JUCE__?.backend?.addEventListener?.('albedoProgram',()=>{refresh();if(routeView)routeView.value=route.value;});
window.addEventListener('albedo:gui-patch-imported',refresh);
let drag=null;
page.addEventListener('pointerdown',e=>{
 const dial=e.target.closest?.('.cp56-rotary');if(!dial)return;
 e.preventDefault();dial.focus({preventScroll:true});
 const id=+dial.closest('.cp56-param').dataset.cp56Id,p=registry.get(id);
 if(!p)return;
 drag={dial,pid:e.pointerId,id,y:e.clientY,v:value(id),span:p.max-p.min};
 dial.setPointerCapture(e.pointerId);
});
page.addEventListener('pointermove',e=>{
 if(drag&&e.pointerId===drag.pid)
  edit(drag.id,drag.v+(drag.y-e.clientY)*drag.span*(e.shiftKey?.001:.008));
});
for(const type of ['pointerup','pointercancel','lostpointercapture'])
 page.addEventListener(type,e=>{if(drag&&e.pointerId===drag.pid)drag=null});
page.addEventListener('wheel',e=>{
 const dial=e.target.closest?.('.cp56-rotary');if(!dial)return;
 e.preventDefault();const id=+dial.closest('.cp56-param').dataset.cp56Id,p=registry.get(id);
 if(p)edit(id,value(id)-Math.sign(e.deltaY)*(p.max-p.min)*(e.shiftKey?.004:.022));
},{passive:false});
page.addEventListener('dblclick',e=>{
 const dial=e.target.closest?.('.cp56-rotary');if(!dial)return;
 const id=+dial.closest('.cp56-param').dataset.cp56Id,p=registry.get(id);if(p)edit(id,p.default);
});
page.addEventListener('keydown',e=>{
 const dial=e.target.closest?.('.cp56-rotary');if(!dial)return;
 const id=+dial.closest('.cp56-param').dataset.cp56Id,p=registry.get(id);if(!p)return;
 let v=value(id),step=(p.max-p.min)*(e.shiftKey?.003:.018);
 if(e.key==='ArrowUp'||e.key==='ArrowRight')v+=step;
 else if(e.key==='ArrowDown'||e.key==='ArrowLeft')v-=step;
 else if(e.key==='Home')v=p.min;
 else if(e.key==='End')v=p.max;
 else return;
 e.preventDefault();edit(id,v);
});
window.ALBEDO_CP56_PRIORITY={version:'CP56',
 get liveExtraControls(){return page.querySelectorAll('.cp56-param[data-cp56-id]').length},
 get sourceIds(){return [...source.querySelectorAll('[data-cp56-id]')].map(t=>+t.dataset.cp56Id)},
 refresh};
})();