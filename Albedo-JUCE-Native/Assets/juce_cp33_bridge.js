/* Albedo CP33 JUCE 8 bridge: augments the original CP22/CP31 UI.
   Never rebuilds the synthesizer DOM or sends audio-thread messages. */
(async function () {
  'use strict';
  const backend=window.__JUCE__?.backend;
  if(!backend || !window.ALBEDO_CP04 || !window.ALBEDO_PRESET_ATLAS)return;

  // Keep the original CP27/CP31 receiving logic: it updates only affected
  // controls, and suppresses host-bound echo while replaying a host snapshot.
  backend.addEventListener('albedoUpdate', message=>{
    if(typeof message==='string')
      window.ALBEDO_NATIVE_CP27?.receiveBatch?.(message);
  });
  backend.addEventListener('albedoProgram', index=>{
    const n=Number(index);
    const patch=window.ALBEDO_PRESET_ATLAS.catalog.find(p=>p.id===n);
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
  const extra=document.createElement('div');
  extra.className='pb-juce-extra';
  const selectors={};
  for(const [field,title] of [['character','SOUND CHARACTER'],
                              ['motion','MOVEMENT'],['context','MUSICAL CONTEXT']]){
    const label=document.createElement('label');
    const heading=document.createElement('span');heading.textContent=title;
    const select=document.createElement('select');
    select.id='pbJuce'+field[0].toUpperCase()+field.slice(1);
    select.setAttribute('aria-label','Filter by '+title.toLowerCase());
    const none=new Option('ALL '+title+'S','');
    select.appendChild(none);
    for(const value of [...new Set(metadata.map(m=>m[field]))].sort())
      select.appendChild(new Option(value,value));
    label.append(heading,select);
    extra.appendChild(label);
    selectors[field]=select;
  }
  toolbar.insertAdjacentElement('afterend',extra);
  const css=document.createElement('style');
  css.textContent=`
    .pb-juce-extra{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));
      gap:10px;margin:8px 12px 10px;}
    .pb-juce-extra label{display:flex;flex-direction:column;min-width:0;
      gap:4px;font-size:11px;letter-spacing:.035em;color:var(--ink,#38504b)}
    .pb-juce-extra select{height:32px;min-width:0;width:100%;font-size:13px;
      background:var(--ground,#e8e8d6);color:var(--ink,#28463b);
      border:1px solid rgba(53,88,80,.35);border-radius:3px;padding:4px 7px}
    .pb-item .pb-cp33-tags{display:block;font:10px/1.2 'Segoe UI',sans-serif;
      font-weight:500;opacity:.7;margin-top:2px;white-space:nowrap;
      overflow:hidden;text-overflow:ellipsis}
    .pb-item.pb-cp33-no-match,.pb-matrix-cell.pb-cp33-no-match{display:none!important}
    .pb-matrix-cell.pb-cp33-no-match{pointer-events:none!important}
    @media(max-width:700px){.pb-juce-extra{grid-template-columns:1fr;gap:5px}}
  `;
  document.head.appendChild(css);
  const hasExtraFilter=()=>Object.values(selectors).some(s=>s.value);
  function matches(m){
    return !!m && Object.entries(selectors).every(([key,element])=>
      !element.value || element.value===m[key]);
  }
  // Original factory filters/search/matrix remain authoritative. CP33 narrows
  // their result set without changing any indexed factory patch or DOM owner.
  function apply(){
    if(dialog.hidden)return;
    const factory=window.ALBEDO_PRESET_ATLAS.collection==='Factory';
    extra.hidden=!factory;
    const base=new Set(window.ALBEDO_PRESET_ATLAS.filtered.map(p=>p.id));
    let shown=0;
    for(const item of list.querySelectorAll('[data-pb-choose]')){
      const match=item.dataset.pbChoose.match(/^Factory:(\d+)$/);
      if(!factory || !match){item.classList.remove('pb-cp33-no-match');continue;}
      const id=Number(match[1]),m=byIndex.get(id);
      const show=base.has(id) && matches(m);
      item.classList.toggle('pb-cp33-no-match',!show);
      if(show)shown++;
      const name=item.querySelector('.pb-item-name');
      if(name && m && !name.querySelector('.pb-cp33-tags')){
        const tag=document.createElement('small');
        tag.className='pb-cp33-tags';
        tag.textContent=m.character+' · '+m.motion+' · '+m.context;
        name.appendChild(tag);
      }
      if(m)item.title=m.name+' · '+m.character+' · '+m.motion+' · '+m.context;
    }
    for(const button of matrix.querySelectorAll('.pb-matrix-cell[data-pb-choose]')){
      const match=button.dataset.pbChoose.match(/^Factory:(\d+)$/);
      if(!match)continue;
      const id=Number(match[1]),m=byIndex.get(id);
      const show=base.has(id)&&matches(m);
      button.classList.toggle('pb-cp33-no-match',!show);
      button.disabled=!show;
      if(m)button.title=m.name+' · '+m.character+' · '+m.motion+' · '+m.context;
    }
    if(factory){
      const count=document.getElementById('pbCount');
      const summary=document.getElementById('pbSummary');
      if(count)count.textContent=shown+' / 64';
      if(summary)summary.textContent='Name × Material × Role × Character × Motion × Context';
    }
  }
  let queued=false;
  function requestApply(){
    if(queued)return;
    queued=true;
    requestAnimationFrame(()=>{queued=false;apply()});
  }
  for(const select of Object.values(selectors))
    select.addEventListener('change',requestApply);
  for(const field of ['pbSearch','pbFamily','pbRole','pbSort'])
    document.getElementById(field)?.addEventListener('input',requestApply);
  dialog.addEventListener('click',requestApply);
  document.getElementById('pbClear')?.addEventListener('click',()=>{
    Object.values(selectors).forEach(s=>{s.value=''});
    requestApply();
  });
  new MutationObserver(requestApply).observe(list,{childList:true});
  new MutationObserver(requestApply).observe(matrix,{childList:true});
  // A visible Atlas is synchronized from host program IDs via its original API.
  window.ALBEDO_CP33_ATLAS={count:64,fields:['family','role','character','motion','context'],
    apply:requestApply,get filtered(){return metadata.filter(matches).map(x=>x.index)},
    get metadata(){return metadata.map(x=>({...x}))}};
  requestApply();
})();
