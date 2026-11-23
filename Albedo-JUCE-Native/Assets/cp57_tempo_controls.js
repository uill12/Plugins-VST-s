/* CP57: musical clock selectors bound to the existing JUCE/DSP parameters.
   The original detailed ARP popup remains unchanged. */
(()=>{
'use strict';
const api=window.ALBEDO_CP04,arp=document.querySelector('#cp56Arp');
if(!arp||!api||document.querySelector('#cp57Rate'))return;
const caption=arp.querySelector('.cp56-caption');
if(!caption)return;
const bar=document.createElement('span');bar.id='cp57TempoControls';
function select(id,label,options){
 const s=document.createElement('select');s.id=id;s.setAttribute('aria-label',label);s.title=label;
 for(const [v,t] of options){const o=document.createElement('option');o.value=String(v);o.textContent=t;s.append(o);}
 return s;
}
const clock=select('cp57Clock','Tempo source',[[1,'Sync'],[0,'Internal']]);
const meter=select('pulsusMeter','Time signature',['4/4','2/4','3/4','5/4','7/8','6/8','9/8','12/8'].map((v,i)=>[i,v]));
clock.onchange=()=>commit(6030,Number(clock.value));
meter.onchange=()=>commit(6032,Number(meter.value));
const rate=select('cp57Rate','Musical arpeggiator division',[[1,'1/4'],[.75,'1/8 dotted'],[.5,'1/8'],[.375,'1/16 dotted'],[1/3,'1/8 triplet'],[.25,'1/16'],[1/6,'1/16 triplet'],[.125,'1/32'],[.0625,'1/64']]);
const pattern=select('cpArpPattern','Pulsus pattern',[
 'Oblique','Symmetrical','Cyclical','Asymmetrical','30 · Chaos','90 · Fractal',
 '110 · Glider','150 · Weave','30 · Sparse','90 · Mirror','110 · Domain','150 · Braid'
].map((name,i)=>[i,name]));
for(const id of [6029,6030]){
 const field=document.querySelector('[data-arp="'+id+'"]');
 field?.closest('label')?.remove();
}
const controls=document.querySelector('#cellArp .pattern-controls');
controls.querySelector('#division').hidden=true;
controls.append(pattern);
pattern.onchange=()=>{
 const choice=Number(pattern.value);
 if(choice<4)commit(6024,choice);
 commit(6031,choice<4?0:choice-3);
};
const bpm=document.createElement('output');
bpm.setAttribute('aria-label','Effective tempo');
controls.append(bpm);
let hostTempo=0;
window.__JUCE__?.backend?.addEventListener?.('albedoHostTempo',v=>{hostTempo=Number(v);refresh();});
bar.append(clock,meter,rate);caption.appendChild(bar);
const style=document.createElement('style');style.id='cp57-tempo-style';
style.textContent=`
html.albedo-fit #page #cp56Arp .cp56-caption{display:flex!important;justify-content:space-between!important;align-items:center!important;gap:3px!important}
html.albedo-fit #page #cp57TempoControls{display:inline-flex;gap:3px;align-items:center;flex:0 0 auto}
html.albedo-fit #page #cp57TempoControls select{font-size:8.5px;font-weight:700;height:19px;line-height:17px;max-width:80px;padding:1px 2px;border:1px solid #a0b09e;border-radius:4px;color:#355347;background:#eaf0e4;cursor:pointer}
#page.nigredo #cp57TempoControls select{background:#253d33;color:#dbe6da;border-color:#567263}
`;style.textContent+=`
html.albedo-fit #page #cp56Arp:not(.internal) [data-cp56-id="6029"]{display:none!important}
html.albedo-fit #page #cp56Arp.internal .cp56-grid{grid-template-columns:repeat(7,minmax(0,1fr))}
#cellArp #division{display:none!important}
#cellArp .pattern-controls output{font:inherit;font-variant-numeric:tabular-nums;white-space:nowrap}
#page .cp-arp-modes{display:flex;justify-content:center;align-items:center;gap:4px;grid-column:1/-1}
#page .cp-arp-modes select{min-width:0;font:700 10px Arial;color:inherit}
#page .cp56-param:not(.cp56-fx-param)>.cp56-label{align-self:stretch;text-align:center}
html.albedo-fit #presetBrowser.preset-browser-backdrop:popover-open{position:fixed!important;inset:0 auto auto 0!important;margin:0!important;width:var(--albedo-browser-w,100vw)!important;height:var(--albedo-browser-h,100dvh)!important;transform:none!important;border:0;display:grid!important;grid-template: minmax(0,1fr) / minmax(0,1fr);place-items:center;padding:7px!important;contain:strict}
html.albedo-fit #presetBrowser:popover-open .preset-browser-panel{position:relative!important;inset:auto!important;transform:none!important;margin:0!important;width:min(1040px,96%)!important;height:min(730px,96%)!important;max-width:100%!important;max-height:100%!important;contain:size layout}
#presetBrowser #pbResultContainer{scrollbar-gutter:stable;overflow-anchor:none}
`;document.head.appendChild(style);
const browser=document.querySelector('#presetBrowser');
if(browser && typeof browser.showPopover==='function'){
 browser.setAttribute('popover','manual');
 const syncBrowser=()=>{if(browser.hidden){if(browser.matches(':popover-open'))browser.hidePopover();}else if(!browser.matches(':popover-open'))browser.showPopover();};
 new MutationObserver(syncBrowser).observe(browser,{attributes:true,attributeFilter:['hidden']});syncBrowser();
}
function get(id){const p=api.registry.parameters.find(x=>x.id===id);return Number(api.state.cp04[id]??p?.default??0);}
function refresh(){
 const ext=get(6031);pattern.value=String(ext?ext+3:get(6024));
 const synced=get(6030)>.5;
 clock.value=synced?'1':'0';
 meter.value=String(get(6032));
 arp.classList.toggle('internal',!synced);
 const useHost=synced && hostTempo>0;
 bpm.textContent=Number((useHost?hostTempo:synced?120:get(6029)).toFixed(1))+" BPM";
 bpm.title=useHost?"Host tempo":"Internal tempo";
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
rate.addEventListener('change',()=>commit(6021,Number(rate.value)));
document.addEventListener('change',e=>{if(e.target.matches?.('[data-arp="6030"],[data-arp="6021"]'))queueMicrotask(refresh);});
window.__JUCE__?.backend?.addEventListener?.('albedoUpdate',()=>queueMicrotask(refresh));
window.__JUCE__?.backend?.addEventListener?.('albedoProgram',refresh);
window.addEventListener('albedo:gui-patch-imported',refresh);
refresh();
window.ALBEDO_CP57_TEMPO={version:'CP57',refresh,get clock(){return clock.value},get division(){return Number(rate.value)}};
})();