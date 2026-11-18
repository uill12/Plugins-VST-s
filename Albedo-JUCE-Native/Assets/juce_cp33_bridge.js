/* Albedo CP33 JUCE 8 bridge: augments the original CP22/CP31 UI.
   Never rebuilds the synthesizer DOM or sends audio-thread messages. */
(async function () {
  'use strict';
  const backend=window.__JUCE__?.backend;
  if(!backend || !window.ALBEDO_CP04 || !window.ALBEDO_PRESET_ATLAS)return;

  // Keep the original CP27/CP31 receiving logic: it updates only affected
  // controls, and suppresses host-bound echo while replaying a host snapshot.
  backend.addEventListener('albedoUpdate', message=>{
    if(typeof message!=='string')return;
    const bridge=window.ALBEDO_NATIVE_CP27;
    if(!bridge)return;
    const changes=message.split(',').filter(Boolean);
    // Atomic initial snapshots and preset swaps; incremental automation otherwise.
    // The original receiveBatch calls renderAll and recreates hundreds of nodes.
    if(changes.length>24){bridge.receiveBatch?.(message);return;}
    for(const pair of changes){
      const sep=pair.indexOf(':');
      if(sep<1)continue;
      const id=Number(pair.slice(0,sep)),v=Number(pair.slice(sep+1));
      if(Number.isSafeInteger(id)&&Number.isFinite(v))bridge.receive?.(id,v);
    }
  });
  backend.addEventListener('albedoActionStatus', text=>{
    const toast=document.getElementById('toast');
    if(!toast)return;
    toast.textContent=String(text);
    toast.classList.add('show');
    clearTimeout(window.__albedoStatusTimeout);
    window.__albedoStatusTimeout=setTimeout(()=>toast.classList.remove('show'),3000);
  });
  backend.addEventListener('albedoProgram', index=>{
    const n=Number(index);
    const patch=[...window.ALBEDO_PRESET_ATLAS.catalog,
                 ...window.ALBEDO_PRESET_ATLAS.creative].find(p=>p.id===n);
    const select=document.getElementById('preset');
    if(patch && select && select.value!==patch.factoryKey)select.value=patch.factoryKey;
  });

  const dialog=document.getElementById('presetBrowser');
  const toolbar=dialog?.querySelector('.preset-browser-toolbar');
  const list=document.getElementById('pbItems');
  const matrix=document.getElementById('pbMatrix');
  if(!toolbar || !list || !matrix)return;
  let metadata;
  try {
    const response=await fetch('factory_preset_taxonomy.json',{cache:'force-cache'});
    if(!response.ok)throw new Error('taxonomy resource not available');
    metadata=await response.json();
    if(!Array.isArray(metadata)||metadata.length!==64||
       metadata.some((m,i)=>m.index!==i||!m.name||!m.family||!m.role||
         !m.character||!m.motion||!m.context))throw new Error('invalid factory taxonomy');
  }catch(error){
    console.warn('CP33 taxonomy unavailable; original 64-preset Atlas is preserved',error);
    return;
  }
  const byIndex=new Map(metadata.map(m=>[m.index,m]));
  // CP36: complete taxonomy remains for factory IDs, but character,
  // movement and context are no longer selectable filters.
  const css=document.createElement('style');
  css.textContent=`
    .pb-item.pb-cp33-no-match {display:none!important}
    .pb-matrix-cell.pb-cp33-no-match {visibility:hidden!important;pointer-events:none!important}
  `;
  document.head.appendChild(css);
  // Original factory filters/search/matrix remain authoritative. CP33 narrows
  // their result set without changing any indexed factory patch or DOM owner.
  function apply(){
    if(dialog.hidden)return;
    const factory=window.ALBEDO_PRESET_ATLAS.collection==='Factory';
    const base=new Set(window.ALBEDO_PRESET_ATLAS.filtered.map(p=>p.id));
    let shown=0;
    for(const item of list.querySelectorAll('[data-pb-choose]')){
      const match=item.dataset.pbChoose.match(/^Factory:(\d+)$/);
      if(!factory || !match){item.classList.remove('pb-cp33-no-match');continue;}
      const id=Number(match[1]),m=byIndex.get(id);
      const show=base.has(id);
      item.classList.toggle('pb-cp33-no-match',!show);
      if(show)shown++;
      if(m)item.title=m.name+' · '+m.character+' · '+m.motion+' · '+m.context;
    }
    for(const button of matrix.querySelectorAll('.pb-matrix-cell[data-pb-choose]')){
      const match=button.dataset.pbChoose.match(/^Factory:(\d+)$/);
      if(!match)continue;
      const id=Number(match[1]),m=byIndex.get(id);
      const show=base.has(id);
      button.classList.toggle('pb-cp33-no-match',!show);
      button.disabled=!show;
      if(m)button.title=m.name+' · '+m.character+' · '+m.motion+' · '+m.context;
    }
    if(factory){
      const count=document.getElementById('pbCount');
      const summary=document.getElementById('pbSummary');
      if(count)count.textContent=shown+' / 64';
      if(summary)summary.textContent='Name × Material × Role';
    }
  }
  let queued=false;
  function requestApply(){
    if(queued)return;
    queued=true;
    requestAnimationFrame(()=>{queued=false;apply()});
  }
  // CP41: don't live-filter while the user edits draft filter labels.
  // Apply/Enter triggers one render, then the list/matrix observers reconcile.
  for(const field of ['pbSearch','pbFamily','pbRole','pbSort'])
    document.getElementById(field)?.addEventListener('change',requestApply);
  for(const id of ['pbApply','pbClear','pbListMode','pbMatrixMode'])
    document.getElementById(id)?.addEventListener('click',requestApply);
  dialog.querySelectorAll('[data-collection]').forEach(tab=>tab.addEventListener('click',requestApply));
  new MutationObserver(requestApply).observe(list,{childList:true});
  new MutationObserver(requestApply).observe(matrix,{childList:true});
  // A visible Atlas is synchronized from host program IDs via its original API.
  window.ALBEDO_CP33_ATLAS={count:64,fields:['family','role'],
    apply:requestApply,get filtered(){return window.ALBEDO_PRESET_ATLAS.filtered.map(p=>p.id)},
    get metadata(){return metadata.map(x=>({...x}))}};
  requestApply();
})();
