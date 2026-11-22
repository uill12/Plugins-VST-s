/* CP57: musical clock selectors bound to the existing JUCE/DSP parameters.
   The original detailed ARP popup remains unchanged. */
(()=>{
'use strict';
const api=window.ALBEDO_CP04,arp=document.querySelector('#cp56Arp');
if(!arp||!api||document.querySelector('#cp57Clock'))return;
const caption=arp.querySelector('.cp56-caption');
if(!caption)return;
const bar=document.createElement('span');bar.id='cp57TempoControls';
function select(id,label,options){
 const s=document.createElement('select');s.id=id;s.setAttribute('aria-label',label);s.title=label;
 for(const [v,t] of options){const o=document.createElement('option');o.value=String(v);o.textContent=t;s.append(o);}
 return s;
}
const clock=select('cp57Clock','Arpeggiator clock mode',[[1,'DAW SYNC'],[0,'FREE']]);
const rate=select('cp57Rate','Musical arpeggiator division',[[1,'1/4'],[.5,'1/8'],[.25,'1/16'],[.125,'1/32'],[.0625,'1/64']]);
const rule=select('cpArpRule','Arp Wolfram Character',['Legacy', 'Rule 30 · Chaos', 'Rule 90 · Fractal', 'Rule 110 · Glider', 'Rule 150 · Weave', 'Rule 30 · Sparse', 'Rule 90 · Mirror', 'Rule 110 · Domain', 'Rule 150 · Braid'].map((name,i)=>[i,name]));
const pattern=select('cpArpPattern','Arpeggiation type',['Oblique','Symmetrical','Cyclical','Asymmetrical'].map((name,i)=>[i,name]));
const modes=document.createElement('div');modes.className='cp-arp-modes';modes.append(rule,pattern);caption.after(modes);
rule.onchange=()=>commit(6031,Number(rule.value));
pattern.onchange=()=>commit(6024,Number(pattern.value));
bar.append(clock,rate);caption.appendChild(bar);
const style=document.createElement('style');style.id='cp57-tempo-style';
style.textContent=`
html.albedo-fit #page #cp56Arp .cp56-caption{display:flex!important;justify-content:space-between!important;align-items:center!important;gap:3px!important}
html.albedo-fit #page #cp57TempoControls{display:inline-flex;gap:3px;align-items:center;flex:0 0 auto}
html.albedo-fit #page #cp57TempoControls select{font-size:8.5px;font-weight:700;height:19px;line-height:17px;max-width:80px;padding:1px 2px;border:1px solid #a0b09e;border-radius:4px;color:#355347;background:#eaf0e4;cursor:pointer}
#page.nigredo #cp57TempoControls select{background:#253d33;color:#dbe6da;border-color:#567263}
`;style.textContent+=`
#page .cp-arp-modes{display:flex;justify-content:center;align-items:center;gap:4px;grid-column:1/-1}
#page .cp-arp-modes select{min-width:0;font:700 10px Arial;color:inherit}
#page .cp56-param:not(.cp56-fx-param)>.cp56-label{align-self:stretch;text-align:center}
html.albedo-fit #presetBrowser.preset-browser-backdrop:popover-open{inset:0!important;margin:0!important;width:auto!important;height:auto!important;transform:none!important;border:0;display:grid!important;place-items:center}
html.albedo-fit #presetBrowser:popover-open .preset-browser-panel{position:relative!important;inset:auto!important;transform:none!important;margin:0!important;max-width:100%!important;max-height:100%!important}
`;document.head.appendChild(style);
const browser=document.querySelector('#presetBrowser');
if(browser && typeof browser.showPopover==='function'){
 browser.setAttribute('popover','manual');
 const syncBrowser=()=>{if(browser.hidden){if(browser.matches(':popover-open'))browser.hidePopover();}else if(!browser.matches(':popover-open'))browser.showPopover();};
 new MutationObserver(syncBrowser).observe(browser,{attributes:true,attributeFilter:['hidden']});syncBrowser();
}
function get(id){const p=api.registry.parameters.find(x=>x.id===id);return Number(api.state.cp04[id]??p?.default??0);}
function refresh(){
 rule.value=String(get(6031));pattern.value=String(get(6024));
 clock.value=get(6030)>.5?'1':'0';
 const v=get(6021),candidate=[...rate.options].find(o=>Math.abs(Number(o.value)-v)<1e-4);
 if(candidate)rate.value=candidate.value;
 else{
  let custom=rate.querySelector('[data-custom]');
  if(!custom){custom=document.createElement('option');custom.dataset.custom='1';custom.textContent='CUSTOM';rate.append(custom);}
  custom.value=String(v);rate.value=custom.value;
 }
}
function commit(id,v){
 api.setExtraParam?.(id,v);
 const old=document.querySelector('[data-arp="'+id+'"]');
 if(old){old.value=String(v);old.dispatchEvent(new Event('change',{bubbles:true}));}
 refresh();
}
clock.addEventListener('change',()=>commit(6030,Number(clock.value)));
rate.addEventListener('change',()=>commit(6021,Number(rate.value)));
document.addEventListener('change',e=>{if(e.target.matches?.('[data-arp="6030"],[data-arp="6021"]'))queueMicrotask(refresh);});
window.__JUCE__?.backend?.addEventListener?.('albedoUpdate',refresh);
window.__JUCE__?.backend?.addEventListener?.('albedoProgram',refresh);
window.addEventListener('albedo:gui-patch-imported',refresh);
refresh();
const panel=document.createElementNS('http://www.w3.org/2000/svg','svg');
panel.id='nereidaArp';panel.setAttribute('viewBox','373 578 725 219');
panel.setAttribute('aria-label','Arpeggiator');arp.before(panel);
const css=document.createElement('style');css.textContent=`
#nereidaArp{width:100%;height:auto;display:block;color:#29473d;touch-action:none;background:#e9ecdf;border-radius:7px}
#nereidaArp text{font: bold 10px Arial;fill:currentColor;pointer-events:none}
#nereidaArp [tabindex]{cursor:pointer}#nereidaArp [aria-pressed=true] rect{fill:#d5ba83}
html.albedo-fit #page #cellArp .arp-performance>#steps,html.albedo-fit #page #cellArp .arp-performance>.pattern-controls,html.albedo-fit #page #cp56Arp>.cp56-grid,html.albedo-fit #page #cp56Arp>.cp-arp-modes{display:none!important}
dialog.arpZoom{width:min(90vw,900px);padding:8px;background:#e9ecdf}dialog.arpZoom::backdrop{background:#16251dcc}
`;document.head.append(css);
function shape(tag,attrs,parent=panel,caption=''){
 const el=document.createElementNS(panel.namespaceURI,tag);
 for(const [key,val] of Object.entries(attrs))el.setAttribute(key,val);
 el.textContent=caption;parent.append(el);return el;
}
function label(x,y,t,parent=panel){return shape('text',{x,y,'text-anchor':'middle'},parent,t);}
function button(x,y,w,h,name,action){
 const g=shape('g',{role:'button',tabindex:0,'aria-label':name});
 shape('rect',{x,y,width:w,height:h,rx:5,fill:'#dce3d6',stroke:'#9bab9b'},g);
 label(x+w/2,y+h/2+3,name,g);
 g.onclick=action;g.onkeydown=e=>{if(['Enter',' '].includes(e.key)){e.preventDefault();action();}};
 return g;
}
const dialog=document.createElement('dialog');dialog.className='arpZoom';document.body.append(dialog);
const detail=button(876,586,207,26,'ARP DETAIL  >',()=>{if(dialog.open)dialog.close();else{dialog.append(panel);dialog.showModal();} detail.lastChild.textContent=dialog.open?'CLOSE':'ARP DETAIL  >';});
dialog.onclose=()=>{arp.before(panel);detail.lastChild.textContent='ARP DETAIL  >';detail.focus();};
label(448,603,'04   ARPEGGIATOR');
const power=button(429,625,83,38,'ARP OFF',()=>{api.setExtraParam(6020,get(6020)>.5?0:1);paint();});
const names=['OBLIQUE','SYMMETRY','CYCLIC','ASYMMETRY','30 CHAOS','90 FRACTAL','110 GLIDER','150 WEAVE','30 SPARSE','90 MIRROR','110 DOMAIN','150 BRAID'];
const choices=names.map((n,i)=>button(520+i%6*88,626+Math.floor(i/6)*32,85,28,n,()=>{
 if(i<4)api.setExtraParam(6024,i);api.setExtraParam(6031,i<4?0:i-3);paint();
}));
const dials=[[6029,'BPM'],[6023,'GATE'],[6025,'OCTAVES'],[6027,'STEPS'],[6026,'SWING'],[6022,'CA ACCENT'],[6028,'ROTATE']].map(([id,name],i)=>{
 const p=api.registry.parameters.find(p=>p.id===id),x=488+i*83;
 const g=shape('g',{role:'slider',tabindex:0,'aria-label':name,'aria-valuemin':p.min,'aria-valuemax':p.max});
 shape('rect',{x:x-35,y:689,width:70,height:72,fill:'transparent'},g);label(x,702,name,g);
 shape('circle',{cx:x,cy:723,r:12,fill:'#f4f0df',stroke:'#718a77'},g);
 const needle=shape('line',{x1:x,y1:723,x2:x,y2:714,stroke:'#506f60','stroke-width':2},g),value=label(x,751,'',g);
 function set(v){api.setExtraParam(id,Math.min(p.max,Math.max(p.min,v)));paint();}
 let origin=0,start=0;g.onpointerdown=e=>{origin=e.clientY;start=get(id);g.setPointerCapture(e.pointerId);};
 g.onpointermove=e=>{if(g.hasPointerCapture(e.pointerId))set(start+(origin-e.clientY)*(p.max-p.min)/100);};
 g.onwheel=e=>{e.preventDefault();set(get(id)-Math.sign(e.deltaY));};
 g.onkeydown=e=>{if(['ArrowUp','ArrowDown'].includes(e.key)){e.preventDefault();set(get(id)+(e.key==='ArrowUp'?1:-1));}};
 return ()=>{const v=get(id);g.setAttribute('aria-valuenow',v);value.textContent=String(Math.round(v*10)/10);needle.setAttribute('transform','rotate('+(-135+270*(v-p.min)/(p.max-p.min))+' '+x+' 723)');};
});
const lights=Array.from({length:16},(_,i)=>shape('rect',{x:452+i*36,y:774,width:28,height:9}));
function paint(){
 const ext=get(6031),mode=ext?ext+3:get(6024),on=get(6020)>.5;
 power.lastChild.textContent=on?'ARP ON':'ARP OFF';power.setAttribute('aria-pressed',on);
 choices.forEach((b,i)=>b.setAttribute('aria-pressed',i===mode));dials.forEach(f=>f());
 let cells=ext?0x10001:0xa5c33c5b;
 const rule=[30,90,110,150][ext?(mode-4)&3:get(4003)];
 lights.forEach((l,i)=>{let next=0;
  for(let j=0;j<32;j++){const pattern=((cells>>>((j+31)&31)&1)<<2)|((cells>>>j&1)<<1)|(cells>>>((j+1)&31)&1);next|=((rule>>>pattern)&1)<<j;}
  cells=next||1<<((i*7+11)&31);const k=(i*7+get(6028)*3)&31,w=(cells>>>k)|(cells<<((32-k)&31)),a=w&1,b=w&2,c=w&4;
  const hit=[i%8!==7,true,true,i*5%8<5,a||b||i%4===0,a||i%2===0,a||b||i%4!==3,true,a&&(b||i%4===0),a||c||i%4!==3,a||b||i%3===0,a||i%4!==3][mode];
  l.setAttribute('fill',i<get(6027)&&hit?(ext?a:cells>>>i&1)?'#b99551':'#577d69':'#c3ccbf');
 });
}
for(const event of ['albedoUpdate','albedoProgram'])window.__JUCE__?.backend?.addEventListener?.(event,()=>queueMicrotask(paint));
document.addEventListener('change',()=>queueMicrotask(paint));window.addEventListener('albedo:gui-patch-imported',paint);paint();

window.ALBEDO_CP57_TEMPO={version:'CP57',refresh,get clock(){return clock.value},get division(){return Number(rate.value)}};
})();