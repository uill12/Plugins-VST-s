/* CP54 — fixed high-impact Albedo controls + local contextual popups.
   One authoritative HTML/CP04/native parameter bridge; no global Advanced mode.
   No DSP changes, polling, page reloads or parameter-by-parameter rerenders. */
(()=>{
'use strict';
const page=document.getElementById('page'),api=window.ALBEDO_CP04;
if(!page||!api||document.getElementById('cp54Popover'))return;
const $=s=>document.querySelector(s), $$=s=>[...document.querySelectorAll(s)];
const registry=new Map(api.registry.parameters.map(p=>[p.id,p]));
const style=document.createElement('style');
style.id='cp54-context-style';
style.textContent=[
'html.albedo-fit #page #engineKnobs>.param:nth-child(n+3),',
'html.albedo-fit #page #layerKnobs,',
'html.albedo-fit #page .source-panel .inspector-body>.divider,',
'html.albedo-fit #page .source-panel .inspector-body>.callrow,',
'html.albedo-fit #page #macros>.param:nth-child(n+5),',
'html.albedo-fit #page #envKnobs>.param:nth-child(2),',
'html.albedo-fit #page .shape-panel .shape-block:nth-child(3) .callrow,',
'html.albedo-fit #page .lower>.surface:first-child:not(#cellArp),',
'html.albedo-fit #page #cellArp .arp-lattice,',
'html.albedo-fit #page #cellArp .arp-options,',
'html.albedo-fit #page .utility{display:none!important}',
'html.albedo-fit #page .source-panel .inspector-body>.section-caption:has(+ #layerKnobs){display:none!important}',
'html.albedo-fit #page #engineKnobs{grid-template-columns:repeat(2,minmax(0,1fr))!important}',
'html.albedo-fit #page #macros{grid-template-columns:repeat(4,minmax(0,1fr))!important}',
'html.albedo-fit #page #filterKnobsA,html.albedo-fit #page #filterKnobsB,html.albedo-fit #page #envKnobs{grid-template-columns:repeat(3,minmax(0,1fr))!important}',
'html.albedo-fit #page .lower{grid-template-columns:minmax(0,1.25fr) minmax(0,1fr)!important;gap:9px!important}',
'html.albedo-fit #page #cellArp .arp-body{display:block!important;grid-template-columns:none!important}',
'html.albedo-fit #page #cellArp .arp-performance{width:100%!important}',
'html.albedo-fit #page #cellArp .pattern-controls{display:flex!important;justify-content:space-between;align-items:center;gap:6px}',
'/* CP55R1 main-stage readability: enlarge only the permanently visible, high-impact sound controls. */',
'html.albedo-fit #page #engineKnobs .dial{width:46px!important;height:46px!important;flex-basis:46px!important}',
'html.albedo-fit #page #macros .dial{width:43px!important;height:43px!important;flex-basis:43px!important}',
'html.albedo-fit #page #filterKnobsA .dial,html.albedo-fit #page #filterKnobsB .dial,html.albedo-fit #page #envKnobs .dial{width:39px!important;height:39px!important;flex-basis:39px!important}',
'html.albedo-fit #page #engineKnobs .dial:after{height:14px!important;top:4px!important;transform-origin:50% 19px!important}',
'html.albedo-fit #page #macros .dial:after{height:13px!important;top:4px!important;transform-origin:50% 17.5px!important}',
'html.albedo-fit #page #filterKnobsA .dial:after,html.albedo-fit #page #filterKnobsB .dial:after,html.albedo-fit #page #envKnobs .dial:after{height:12px!important;top:4px!important;transform-origin:50% 15.5px!important}',
'html.albedo-fit #page #engineKnobs>.param{min-height:72px!important}',
'html.albedo-fit #page #macros>.param{min-height:67px!important}',
'html.albedo-fit #page #filterKnobsA>.param,html.albedo-fit #page #filterKnobsB>.param,html.albedo-fit #page #envKnobs>.param{min-height:67px!important}',
'html.albedo-fit #page #engineKnobs .param-label,html.albedo-fit #page #macros .param-label,html.albedo-fit #page #filterKnobsA .param-label,html.albedo-fit #page #filterKnobsB .param-label,html.albedo-fit #page #envKnobs .param-label{font-size:11.4px!important;line-height:1.08!important;max-height:24px!important}',
'html.albedo-fit #page #engineKnobs .param-value,html.albedo-fit #page #macros .param-value,html.albedo-fit #page #filterKnobsA .param-value,html.albedo-fit #page #filterKnobsB .param-value,html.albedo-fit #page #envKnobs .param-value{font-size:11.4px!important;min-height:14px!important}',
'#cp04LabOpen,#cp04Lab{display:none!important}',
'#page .cp54-trigger{display:inline-flex!important;align-items:center;justify-content:center;flex:0 0 auto;min-height:21px;padding:2px 7px;margin-left:5px;vertical-align:middle;border:1px solid #ac9c79;border-radius:4px;background:linear-gradient(#f1ecda,#dae6d8);color:#436651;letter-spacing:.035em;white-space:nowrap;font:800 9px/1 Arial,sans-serif;cursor:pointer}',
'#page .cp54-trigger:hover,#page .cp54-trigger[aria-expanded=true]{background:#f9efd4;border-color:#b18d57;color:#315743}',
'#page .cp54-trigger:focus-visible{outline:2px solid #bd9456;outline-offset:2px}',
'#page .cp54-header-title{display:flex!important;align-items:center;justify-content:space-between;gap:4px}',
'#page .cp54-header-title>.cp54-trigger{margin-left:auto}',
'#page .macro-caption>.cp54-trigger{margin-left:auto;min-height:18px;font-size:8px}',
'#page .cp54-strong-selectors{display:flex;align-items:center;gap:5px;margin:5px 0;flex-wrap:wrap;min-height:23px}',
'#page .cp54-strong-selectors label{display:inline-flex;align-items:center;gap:4px;flex:1 1 85px;max-width:100%;font:800 9px Arial,sans-serif;color:#597865}',
'#page .cp54-strong-selectors select{flex:1;min-width:42px;max-width:100%;padding:3px 5px;border:1px solid #a5b69e;border-radius:4px;background:#ecebd7;color:#365c49;font:700 10px Arial,sans-serif}',
'#page .cp54-strong-selectors label[hidden]{display:none!important}',
'#cp54Scrim[hidden],#cp54Popover[hidden]{display:none!important}',
'#cp54Scrim{position:fixed;inset:0;z-index:95000;background:#112c254c}',
'#cp54Popover{position:fixed;z-index:95001;display:flex;flex-direction:column;box-sizing:border-box;width:min(480px,calc(100vw - 20px));max-height:calc(100dvh - 20px);min-height:0;padding:0;border:1px solid #a3b79f;border-radius:9px;background:linear-gradient(140deg,#eeecdb,#dce8dd 62%,#f1e9d8);color:#3b5d4a;box-shadow:0 12px 35px #0b29219c,inset 0 1px 0 #fff9e7;overflow:hidden;font-family:Arial,sans-serif}',
'#cp54Popover .cp54-head{display:flex;align-items:center;gap:8px;min-height:51px;padding:8px 12px;border-bottom:1px solid #adc0a8;background:linear-gradient(90deg,#dce7d9,#efe9d7);flex:0 0 auto}',
'#cp54Popover .cp54-sign{width:28px;height:28px;display:grid;place-items:center;border:1px solid #c5a570;border-radius:50%;background:#f7eed6;color:#ad8c58;font-size:19px}',
'#cp54Popover .cp54-headtext{flex:1;min-width:0}',
'#cp54Popover .cp54-head h2{font:700 18px/1.1 Georgia,serif;margin:1px 0;color:#335846}',
'#cp54Popover .cp54-head small{display:block;font:800 8px/1.2 Arial,sans-serif;letter-spacing:.1em;color:#67816e}',
'#cp54Popover .cp54-close{flex:0 0 auto;border:1px solid #a4b89f;border-radius:5px;background:#f5efd9;color:#355943;padding:7px 9px;font:800 10px Arial,sans-serif;cursor:pointer}',
'#cp54Popover .cp54-body{flex:1;min-height:0;overflow-y:auto;overflow-x:hidden;padding:10px 12px 14px;overscroll-behavior:contain;scrollbar-color:#97ad9b #dce7d9}',
'#cp54Popover .cp54-intro{font:500 10px/1.35 Arial,sans-serif;color:#68806e;margin:0 0 9px}',
'#cp54Popover .cp54-subhead{font:800 9px/1.2 Arial,sans-serif;letter-spacing:.07em;color:#58735c;margin:11px 0 7px;padding-bottom:5px;border-bottom:1px solid #c5d2bc}',
'#cp54Popover .cp54-dials{display:grid;grid-template-columns:repeat(auto-fit,minmax(87px,1fr));gap:8px 4px;min-width:0;justify-items:center}',
'#cp54Popover .cp54-dials .param{display:flex;flex-direction:column;align-items:center;justify-content:start;min-width:0;min-height:82px;gap:4px;padding:0 2px;text-align:center}',
'#cp54Popover .cp54-dials .param-label{font:700 10px/1.1 Arial,sans-serif;color:#476d56}',
'#cp54Popover .cp54-dials .param-value{font:600 11px/1.1 Georgia,serif;color:#496a55}',
'#cp54Popover .dial{position:relative;display:block;width:43px;height:43px;flex:0 0 43px;border:2px solid #819d88;border-radius:50%;box-sizing:border-box;touch-action:none;cursor:ns-resize;outline-offset:2px;background:radial-gradient(circle at 33% 24%,#fff8e0,#dfe1cc 53%,#a8beac 82%,#6c8d78);box-shadow:inset 0 0 0 3px #f3edd9,0 2px 5px #36574748}',
'#cp54Popover .dial::after{content:"";position:absolute;left:calc(50% - 2px);top:5px;width:4px;height:13px;background:var(--dialcolor,#b99762);transform-origin:50% 16px;transform:rotate(calc(-135deg + var(--fill,0.5)*270deg));border-radius:2px}',
'#cp54Popover .dial:focus-visible{outline:2px solid #bc9455}',
'#cp54Popover .cp54-extra-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(107px,1fr));gap:7px;margin-bottom:7px}',
'#cp54Popover .cp54-native{display:flex;flex-direction:column;align-items:stretch;gap:4px;min-width:0;padding:6px 7px;border:1px solid #bacdb8;border-radius:5px;background:#e4ebdfad}',
'#cp54Popover .cp54-native>label{font:700 10px/1.3 Arial,sans-serif;color:#40624f;min-height:25px}',
'#cp54Popover .cp54-native output{font:600 10px Georgia,serif;color:#4a6c55;text-align:center}',
'#cp54Popover .cp54-rotary{position:relative;display:block;margin:0 auto;width:39px;height:39px;border:2px solid #85a18c;border-radius:50%;touch-action:none;cursor:ns-resize;outline-offset:2px;background:radial-gradient(circle at 34% 22%,#fff6dd,#c8dac9 65%,#7d9e86)}',
'#cp54Popover .cp54-rotary::after{content:"";position:absolute;left:calc(50% - 2px);top:4px;width:4px;height:11px;background:#866a45;border-radius:2px;transform-origin:50% 15px;transform:rotate(var(--cp54-angle,-135deg))}',
'#cp54Popover .cp54-rotary:focus-visible{outline:2px solid #b99258}',
'#cp54Popover .cp54-native select{width:100%;max-width:100%;padding:5px;border:1px solid #aebfaa;border-radius:4px;background:#f7f2e4;color:#365a48;font:700 10px Arial,sans-serif}',
'#cp54Popover .cp54-original{min-width:0;margin:6px 0 9px}',
'#cp54Popover .cp54-original .surface{display:block!important;position:static!important;min-height:0!important;max-height:none!important;height:auto!important;width:auto!important;overflow:visible!important;padding:0!important;margin:0!important;border:0!important;box-shadow:none!important;background:transparent!important}',
'#cp54Popover .cp54-original .surface-title{display:none!important}',
'#cp54Popover .cp54-original .bodypad{height:auto!important;min-height:0!important;max-height:none!important;padding:0!important;overflow:visible!important}',
'#cp54Popover .cp54-original .route-row{display:flex;align-items:center;min-height:31px;gap:5px;margin:3px 0}',
'#cp54Popover .cp54-original .route-row select{flex:1;min-width:0;border:1px solid #acbca7;background:#eff0e1;font:600 10px Arial,sans-serif}',
'#cp54Popover .cp54-original .route-row .cp38-wrap{flex:0 0 28px}',
'#cp54Popover .cp54-original .arp-lattice{display:block!important}',
'#cp54Popover .cp54-original .arp-options{display:grid!important;grid-template-columns:repeat(3,minmax(0,1fr));gap:6px}',
'#cp54Popover .cp54-original .arp-options label{display:flex;flex-direction:column;gap:4px;min-width:0;font:700 9px Arial,sans-serif}',
'#cp54Popover .cp54-original .arp-options select,#cp54Popover .cp54-original .arp-options input{box-sizing:border-box;max-width:100%;min-width:0;padding:3px;border:1px solid #aabbab;border-radius:4px;background:#f7f2e4;color:#365a47}',
'#cp54Popover .cp54-original .utility{display:flex!important;flex-direction:column;gap:6px}',
'#cp54Popover .cp54-original .utility .surface{padding:6px!important;background:#e6ecdfa8!important;border:1px solid #bcccb7!important}',
'#cp54Popover .cp54-original .utility .subrow{display:flex;align-items:center;justify-content:space-between;gap:7px}',
'#cp54Popover .cp54-original .utility p{margin:1px 0;font:600 10px Arial,sans-serif;color:#476450}',
'#cp54Popover .cp54-original .utility select{padding:4px;max-width:130px}',
'#cp54Popover .cp54-original .callrow{display:flex!important;align-items:center;flex-wrap:wrap;min-height:28px;gap:8px;margin:7px 0;color:#43654f}',
'#cp54Popover .cp54-original .callrow label{font:700 10px Arial,sans-serif}',
'#cp54Popover .cp54-original .motion-controls{display:flex;justify-content:space-around;gap:6px}',
'#cp54Popover button.tiny-button,#cp54Popover button.add-route{border:1px solid #a5b8a2;border-radius:4px;padding:5px 7px;background:#e8edde;color:#3d5f4c;cursor:pointer}',
'#cp54Popover .cp54-keytrack{display:block;margin:9px 0;padding:7px 11px;font:800 10px Arial,sans-serif;letter-spacing:.04em}',
'#cp54Popover .cp54-choice{display:flex;align-items:center;gap:8px;margin:7px 0}',
'#cp54Popover .cp54-choice label{min-width:88px;font:700 10px Arial,sans-serif;color:#466653}',
'#cp54Popover .cp54-choice select{flex:1;min-width:0;padding:5px;border:1px solid #adbea8;border-radius:4px;background:#f7f2e2;color:#375b46}',
'@media(max-width:690px){#cp54Popover .cp54-extra-grid{grid-template-columns:repeat(2,minmax(0,1fr))}#page .cp54-trigger{padding:2px 5px;font-size:8px}}'
].join('\n');
document.head.appendChild(style);

// Dedicated, fixed main-stage selectors for the largest oscillator timbre changes.
const method=$('#method'),inspector=method?.parentElement;
let primary=null,syncPrimary=()=>{};
if(method&&inspector){
 primary=document.createElement('div');primary.className='cp54-strong-selectors';
 primary.innerHTML='<label id="cp54MaterialWrap">MATERIAL<select id="cp54MaterialMain" aria-label="Source tonal material"></select></label><label id="cp54UnisonWrap">UNISON<select id="cp54UnisonMain" aria-label="Unison voice count"></select></label>';
 method.after(primary);
 const mat=primary.querySelector('#cp54MaterialMain');
 api.materials.forEach((name,i)=>mat.add(new Option(name,String(i))));
 const uni=primary.querySelector('#cp54UnisonMain');
 for(let v=1;v<=8;v++)uni.add(new Option(String(v),String(v)));
 const synchronize=()=>{
  const base=1000+32*api.state.selected;
  const chosen=api.state.engines[api.state.selected].method.split(' / ')[0];
  const material=['Harmonic','Granular','Sample'].includes(chosen);
  const unison=['Analog','Wavetable','Harmonic','Phase Distortion','Generative'].includes(chosen);
  primary.querySelector('#cp54MaterialWrap').hidden=!material;
  primary.querySelector('#cp54UnisonWrap').hidden=!unison;
  primary.hidden=!material&&!unison;
  mat.value=String(Math.round(api.state.cp04[base+10]??0));
  uni.value=String(Math.round(api.state.cp04[base+11]??1));
 };
 syncPrimary=synchronize;
 mat.addEventListener('change',()=>api.setExtraParam?.(1010+32*api.state.selected,+mat.value));
 uni.addEventListener('change',()=>api.setExtraParam?.(1011+32*api.state.selected,+uni.value));
 method.addEventListener('change',()=>queueMicrotask(synchronize));
 document.addEventListener('click',e=>{
  if(e.target.closest('[data-select],.node,[data-pb-choose]'))queueMicrotask(synchronize);
 });
 window.__JUCE__?.backend?.addEventListener?.('albedoProgram',()=>queueMicrotask(synchronize));
 synchronize();
}

// Small local openers; primary knobs never move or disappear when using popups.
const configs=[
 ['source','.source-panel .surface-title','SOURCE','Source — materials and articulation','Source material, detail, stereo, pitch and transient refinement'],
 ['macro','.macro-deck .macro-caption','MORE','Quintessentia — fine character','Macro dimensions beyond the four essential sound transformations'],
 ['filter','.shape-panel .surface-title','BODY','Corpus — filter and envelope details','Filter drive, envelope curvature, pitch behavior and resonance architecture'],
 ['mod','.coupling-head','ROUTES','Syzygia — modulation assignments','Original symbolic routing and LFO-specific configuration'],
 ['arp','#cellArp .surface-title','RHYTHM','Pulsus — cellular rhythm refinements','Rule, motion and articulation of the note-event patterns'],
 ['fx','#fx', 'FX +','Spatium — processing detail','Effect algorithms, space size, damping and bias'],
 ['settings','#page header .actions','SYSTEM','Claritas — performance and output','Tuning, oversampling, tonal guard and voice stability']
];
const buttons=new Map();
for(const [key,selector,label,title,description] of configs){
 const anchor=$(selector);
 if(!anchor)continue;
 const button=document.createElement('button');
 button.type='button';button.className='cp54-trigger';button.textContent='◇ '+label;
 button.title=title;button.dataset.cp54Context=key;
 button.setAttribute('aria-haspopup','dialog');button.setAttribute('aria-controls','cp54Popover');
 button.setAttribute('aria-expanded','false');
 if(key==='fx')anchor.closest('.surface')?.querySelector('.surface-title')?.appendChild(button);
 else if(key==='settings')anchor.insertBefore(button,anchor.querySelector('#help'));
 else anchor.appendChild(button);
 buttons.set(key,button);
}
const scrim=document.createElement('div');
scrim.id='cp54Scrim';scrim.hidden=true;
const popup=document.createElement('section');
popup.id='cp54Popover';popup.hidden=true;
popup.setAttribute('role','dialog');popup.setAttribute('aria-modal','true');
popup.setAttribute('aria-label','Albedo contextual sound parameters');
popup.innerHTML='<header class="cp54-head"><span class="cp54-sign">◇</span><div class="cp54-headtext"><small>ALBEDO · SOUND ARCHITECTURE</small><h2 id="cp54Title"></h2></div><button type="button" class="cp54-close" id="cp54Close">✕ CLOSE</button></header><div class="cp54-body" id="cp54Body"></div>';
document.body.append(scrim,popup);
const body=popup.querySelector('#cp54Body');
let moved=[],activeKey=null,activeButton=null,lastFocus=null,mouseDrag=null;

function heading(label){
 const h=document.createElement('h3');h.className='cp54-subhead';h.textContent=label;body.appendChild(h);
}
function cloned(selectors){
 const holder=document.createElement('div');holder.className='cp54-dials';
 for(const selector of selectors){const node=$(selector);if(node)holder.appendChild(node.cloneNode(true))}
 body.appendChild(holder);
}
function moveOriginal(selector){
 const node=$(selector);if(!node)return;
 const marker=document.createComment('CP54 restore original node');
 node.parentNode.insertBefore(marker,node);
 const holder=document.createElement('div');holder.className='cp54-original';
 holder.appendChild(node);body.appendChild(holder);
 moved.push([node,marker]);
}
function proxyOriginal(selector,label){
 const target=$(selector);if(!target)return;
 const row=document.createElement('div');row.className='cp54-choice';
 const caption=document.createElement('label');caption.textContent=label;
 const mirror=target.cloneNode(true);
 mirror.removeAttribute('id');mirror.value=target.value;
 mirror.addEventListener('change',()=>{
  target.value=mirror.value;target.dispatchEvent(new Event('change',{bubbles:true}));
 });
 row.append(caption,mirror);body.appendChild(row);
}
function getValue(id){const p=registry.get(id);return +(api.state.cp04[id]??p?.default??0)}
function steppedValues(id,p){
 if((id-1000)%32===10&&id>=1010&&id<=1118)return api.materials.map((name,i)=>[i,name]);
 if((id-1000)%32===11&&id>=1011&&id<=1119)return Array.from({length:8},(_,i)=>[i+1,String(i+1)]);
 if((id-1000)%32===15&&id>=1015&&id<=1123)return ['HANN','SINE','TRAPEZOID','GAUSSIAN'].map((s,i)=>[i,s]);
 if(id===4011||id===4012)return ['SINE','TRIANGLE','RAMP','SQUARE','COMPLEX'].map((s,i)=>[i,s]);
 const entries=[];for(let v=p.min;v<=p.max&&entries.length<=48;v++)entries.push([v,String(v)]);
 return entries;
}
function displayValue(id){
 const p=registry.get(id),value=getValue(id);
 if(!p)return '';
 if((id-1000)%32===10&&id>=1010&&id<=1118)return api.materials[Math.round(value)]||String(Math.round(value));
 if(p.stepped)return String(Math.round(value));
 return (Math.abs(value)<10?value.toFixed(2):value.toFixed(1))+(p.unit&&p.unit!=='x'?' '+p.unit:'');
}
function updateOne(id){
 const p=registry.get(id);if(!p)return;
 for(const c of body.querySelectorAll('[data-cp54-id="'+id+'"]')){
  if(c.matches('select'))c.value=String(Math.round(getValue(id)));
  else {
   const frac=Math.max(0,Math.min(1,(getValue(id)-p.min)/Math.max(.00001,p.max-p.min)));
   c.style.setProperty('--cp54-angle',(-135+frac*270)+'deg');
   c.setAttribute('aria-valuenow',String(getValue(id)));
  }
 }
 for(const o of body.querySelectorAll('[data-cp54-value="'+id+'"]'))o.textContent=displayValue(id);
}
function editExtra(id,v){
 const p=registry.get(id);
 if(!p||!Number.isFinite(v))return;
 api.setExtraParam?.(id,v);updateOne(id);
}
function nativeControls(ids){
 const grid=document.createElement('div');grid.className='cp54-extra-grid';
 for(const id of ids){
  const p=registry.get(id);if(!p)continue;
  const tile=document.createElement('div');tile.className='cp54-native';
  const label=document.createElement('label');label.textContent=p.name;
  const output=document.createElement('output');output.dataset.cp54Value=String(id);
  let widget;
  if(p.stepped){
   widget=document.createElement('select');
   widget.setAttribute('aria-label',p.name);
   for(const [value,name] of steppedValues(id,p))widget.add(new Option(name,String(value)));
   widget.addEventListener('change',()=>editExtra(id,+widget.value));
  }else{
   widget=document.createElement('div');widget.className='cp54-rotary';
   widget.setAttribute('role','slider');widget.tabIndex=0;
   widget.setAttribute('aria-label',p.name);widget.setAttribute('aria-valuemin',String(p.min));
   widget.setAttribute('aria-valuemax',String(p.max));
  }
  widget.dataset.cp54Id=String(id);
  tile.append(label,widget,output);grid.appendChild(tile);
 }
 body.appendChild(grid);
 for(const id of ids)updateOne(id);
}
function sectionSource(){
 heading('Selected oscillator character');
 cloned(['#engineKnobs>.param:nth-child(3)','#engineKnobs>.param:nth-child(4)',
 '#layerKnobs>.param:nth-child(1)','#layerKnobs>.param:nth-child(2)','#layerKnobs>.param:nth-child(3)']);
 // Keyboard tracking is a voice-critical switch. Keep one authoritative
 // HTML/native state and make it accessible beside the other source refinements.
 const keytrack=$('#keytrack');
 if(keytrack){
  const toggle=document.createElement('button');
  toggle.type='button';toggle.className='tiny-button cp54-keytrack';
  toggle.setAttribute('aria-label','Keyboard tracking for selected oscillator');
  const sync=()=>{
   const enabled=api.state.engines[api.state.selected].keytrack;
   toggle.textContent=enabled?'KEYTRACK ON':'FREE PITCH';
   toggle.setAttribute('aria-pressed',String(enabled));
  };
  sync();
  toggle.addEventListener('click',()=>{keytrack.click();sync()});
  body.appendChild(toggle);
 }
 const base=1000+32*api.state.selected;
 heading('Synthesis-specific options');
 const method=api.state.engines[api.state.selected].method.split(' / ')[0];
 let offsets=[14,19,20,21];
 if(['Analog','Wavetable','Harmonic','Phase Distortion','Generative'].includes(method))offsets.push(12,13);
 if(method==='Granular')offsets.push(15);
 if(method==='Harmonic')offsets.push(16,17);
 if(method==='Noise')offsets.push(18);
 nativeControls(offsets.sort((a,b)=>a-b).map(i=>base+i));
}
function sectionMacro(){
 heading('Additional material dimensions');
 cloned(['#macros>.param:nth-child(5)','#macros>.param:nth-child(6)','#macros>.param:nth-child(7)']);
}
function sectionFilter(){
 heading('Body shaping and envelope character');
 cloned(['#envKnobs>.param:nth-child(2)']);
 heading('Pitch and filter architecture');
 proxyOriginal('#filterRoute','Filter route');
 moveOriginal('#pitchMode'); // Segment retains its existing click handler on the document.
 moveOriginal('#voiceMode'); // Keep original identifier for host parameter linkage.
 heading('Resonant envelope and glide');
 nativeControls([3120,3121,3122,3123,3124,3125,3130,3131]);
}
function sectionModulation(){
 heading('Reciprocal modulation routes');
 moveOriginal('#page .lower>.surface:first-child:not(#cellArp)');
 heading('LFO configuration');
 nativeControls([4010,4011,4012,4013,4014]);
}
function sectionArp(){
 heading('Cellular motion');
 moveOriginal('#cellArp .arp-lattice');
 heading('Arpeggiator pattern options');
 moveOriginal('#cellArp .arp-options');
}
function sectionFx(){
 heading('Effect color and spatial parameters');
 nativeControls([5008,5009,5010,5011,5012]);
}
function sectionSettings(){
 heading('Tuning, oversampling and master safety');
 moveOriginal('#page .utility');
}
const sections={source:sectionSource,macro:sectionMacro,filter:sectionFilter,
 mod:sectionModulation,arp:sectionArp,fx:sectionFx,settings:sectionSettings};
function place(button){
 const box=button.getBoundingClientRect();
 const margin=10,w=Math.min(480,innerWidth-margin*2);
 // Prefer adjacent placement; fall back to opposite side and finally clamp.
 let x=box.right+8;if(x+w>innerWidth-margin)x=box.left-w-8;
 if(x<margin)x=(innerWidth-w)/2;
 const h=Math.min(popup.scrollHeight,innerHeight-2*margin);
 let y=box.top;
 if(y+h>innerHeight-margin)y=Math.max(margin,innerHeight-h-margin);
 popup.style.left=Math.round(Math.max(margin,Math.min(x,innerWidth-w-margin)))+'px';
 popup.style.top=Math.round(Math.max(margin,y))+'px';
 popup.style.width=Math.round(w)+'px';
}
function open(key){
 if(!sections[key]||!buttons.has(key))return;
 close(false);
 activeKey=key;activeButton=buttons.get(key);
 lastFocus=document.activeElement;
 const config=configs.find(x=>x[0]===key);
 popup.querySelector('#cp54Title').textContent=config[3];
 body.replaceChildren();
 const intro=document.createElement('p');intro.className='cp54-intro';intro.textContent=config[4];
 body.appendChild(intro);
 sections[key]();
 popup.hidden=scrim.hidden=false;
 activeButton.setAttribute('aria-expanded','true');
 place(activeButton);
 popup.querySelector('#cp54Close').focus();
}
function close(restoreFocus=true){
 if(activeKey===null)return;
 for(let i=moved.length-1;i>=0;--i){
  const [node,marker]=moved[i];
  if(marker.parentNode)marker.parentNode.insertBefore(node,marker);
  marker.remove();
 }
 moved=[];
 popup.hidden=scrim.hidden=true;
 popup.style.top=popup.style.left='';
 body.replaceChildren();
 activeButton?.setAttribute('aria-expanded','false');
 activeKey=null;activeButton=null;mouseDrag=null;
 if(restoreFocus&&lastFocus?.isConnected)lastFocus.focus();
}
for(const [key,button] of buttons)button.addEventListener('click',()=>activeKey===key?close():open(key));
popup.querySelector('#cp54Close').addEventListener('click',()=>close());
scrim.addEventListener('pointerdown',()=>close());
window.addEventListener('resize',()=>{if(activeKey)place(activeButton)},{passive:true});
window.__JUCE__?.backend?.addEventListener?.('albedoProgram',()=>{
 close(false);
 queueMicrotask(syncPrimary)
});
document.addEventListener('keydown',e=>{
 if(activeKey===null)return;
 if(e.key==='Escape'){e.preventDefault();e.stopImmediatePropagation();close();return}
 if(e.key!=='Tab')return;
 const nodes=$$('#cp54Popover button:not([disabled]),#cp54Popover select:not([disabled]),#cp54Popover [tabindex="0"]')
  .filter(x=>x.getClientRects().length);
 if(!nodes.length)return;
 if(e.shiftKey&&document.activeElement===nodes[0]){e.preventDefault();nodes[nodes.length-1].focus()}
 else if(!e.shiftKey&&document.activeElement===nodes[nodes.length-1]){e.preventDefault();nodes[0].focus()}
},true);
popup.addEventListener('pointerdown',e=>{
 const c=e.target.closest('.cp54-rotary');if(!c)return;
 e.preventDefault();
 const id=+c.dataset.cp54Id,p=registry.get(id);
 mouseDrag={id,pointerId:e.pointerId,startY:e.clientY,start:getValue(id),range:p.max-p.min};
 c.setPointerCapture(e.pointerId);
});
popup.addEventListener('pointermove',e=>{
 if(!mouseDrag||mouseDrag.pointerId!==e.pointerId)return;
 const d=mouseDrag;
 editExtra(d.id,d.start+(d.startY-e.clientY)*d.range*(e.shiftKey?.001:.008));
});
popup.addEventListener('pointerup',()=>mouseDrag=null);
popup.addEventListener('pointercancel',()=>mouseDrag=null);
popup.addEventListener('wheel',e=>{
 const c=e.target.closest('.cp54-rotary');if(!c)return;
 e.preventDefault();
 const id=+c.dataset.cp54Id,p=registry.get(id);
 editExtra(id,getValue(id)-Math.sign(e.deltaY)*(p.max-p.min)*(e.shiftKey?.004:.025));
},{passive:false});
popup.addEventListener('dblclick',e=>{
 const c=e.target.closest('.cp54-rotary');if(c)editExtra(+c.dataset.cp54Id,registry.get(+c.dataset.cp54Id).default);
});
popup.addEventListener('keydown',e=>{
 const c=e.target.closest('.cp54-rotary');if(!c)return;
 const id=+c.dataset.cp54Id,p=registry.get(id),step=(p.max-p.min)*(e.shiftKey?.003:.019);
 let value=getValue(id);
 if(e.key==='ArrowUp'||e.key==='ArrowRight')value+=step;
 else if(e.key==='ArrowDown'||e.key==='ArrowLeft')value-=step;
 else if(e.key==='Home')value=p.min;
 else if(e.key==='End')value=p.max;
 else return;
 e.preventDefault();editExtra(id,value);
});
window.ALBEDO_CP54_ESSENTIAL={
 open,close,
 get activeContext(){return activeKey},
 get buttons(){return [...buttons.keys()]},
 get isOpen(){return activeKey!==null},
 get primaryControlsPermanent(){return true},
 get advancedModeRemoved(){return true},
 version:'CP54-contextual'
};
})();