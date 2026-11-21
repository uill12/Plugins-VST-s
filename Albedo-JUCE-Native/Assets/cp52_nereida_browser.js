/* CP52 — Nereida-style library browser for the existing Albedo factory IDs.
   Categorises source sounds by library, family and instrument role; this is
   NOT the offline Morph Tree sound-generation process.
   The original HTML atlas remains the sole loader and owns all preset IDs. */
(()=>{
  'use strict';
  const atlas=window.ALBEDO_PRESET_ATLAS;
  const dialog=document.getElementById('presetBrowser');
  const result=document.getElementById('pbResultContainer');
  const panel=dialog?.querySelector('.preset-browser-panel');
  if(!atlas||!dialog||!result||!panel||document.getElementById('albedo-cp52-workspace'))return;

  const style=document.createElement('style');
  style.id='albedo-cp52-browser-style';
  style.textContent=[
    'html.albedo-fit #presetBrowser .preset-browser-panel,#presetBrowser .preset-browser-panel{width:min(1040px,90vw,calc(100vw - 28px))!important;height:min(730px,90dvh,calc(100dvh - 28px))!important;max-height:calc(100dvh - 28px)!important;min-width:0!important;min-height:0!important;box-sizing:border-box!important}',
    '#presetBrowser .preset-browser-collections{flex:0 0 auto!important;min-height:33px!important;flex-wrap:wrap!important;white-space:normal!important}',
    '#presetBrowser .preset-browser-heading h2{font-size:23px!important}',
    '#presetBrowser .pb-filter-stack{display:none!important}',
    '#presetBrowser #albedo-cp52-workspace{display:flex;flex:1 1 auto;min-height:0;min-width:0;gap:0;overflow:hidden}',
    'html.albedo-fit #presetBrowser.preset-browser-backdrop{position:fixed!important;left:0!important;top:0!important;right:0!important;bottom:0!important;width:100vw!important;height:100dvh!important;max-width:100vw!important;max-height:100dvh!important;box-sizing:border-box!important;overflow:hidden!important;padding:7px!important}',
    'html.albedo-fit #presetBrowser .preset-browser-panel{width:min(1040px,90vw,calc(100vw - 28px))!important;height:min(730px,90dvh,calc(100dvh - 28px))!important;max-height:calc(100dvh - 28px)!important;min-width:0!important;min-height:0!important;box-sizing:border-box!important}',
    '#presetBrowser .cp52-window-controls{display:flex;align-items:center;flex:0 0 auto;gap:4px;margin-left:auto}',
    '#presetBrowser .cp52-window-controls button{display:flex;align-items:center;justify-content:center;min-width:32px;height:29px;padding:0 7px;border:1px solid #afbcac;border-radius:4px;background:linear-gradient(#faf5e6,#d8e5d8);color:#365748;font:800 11px Arial,sans-serif;cursor:pointer}',
    '#presetBrowser .cp52-window-controls button:hover{background:#fff5df;border-color:#a28a5d}',
    '#presetBrowser .cp52-window-controls button[aria-label^="Keep preset"]{min-width:91px;font-size:9px;letter-spacing:.03em}',
    '#presetBrowser .cp52-window-size{font:700 10px Georgia,serif;min-width:79px;text-align:center;white-space:nowrap;color:#587569}',
    '#presetBrowser .preset-browser-titlebar{min-width:0;gap:7px!important}',
    '#presetBrowser .preset-browser-heading{min-width:0;flex:1}',
    '#presetBrowser .preset-browser-titlebar #presetBrowserClose{flex:0 0 auto;min-width:56px}',
    '#presetBrowser #albedo-cp52-navigation{flex:0 0 clamp(166px,21%,240px);min-width:0;overflow-y:auto;overscroll-behavior:contain;border-right:1px solid #aab5a3;background:linear-gradient(145deg,#d8e3d6,#e8e7d6);padding:10px 8px;scrollbar-color:#a1b6a5 #d6e2d6}',
    '#presetBrowser .cp52-section{margin:0 0 14px}',
    '#presetBrowser .cp52-section h3{font:800 10px Arial,sans-serif;letter-spacing:.13em;color:#507160;padding:3px 6px 8px;margin:0}',
    '#presetBrowser .cp52-category{display:flex;align-items:center;justify-content:space-between;gap:7px;width:100%;text-align:left;border:1px solid transparent;border-radius:5px;padding:7px 9px;margin:1px 0;background:transparent;color:#3c584c;cursor:pointer;font:700 11px Arial,sans-serif;min-height:29px}',
    '#presetBrowser .cp52-category>span{min-width:0;overflow:hidden;white-space:nowrap;text-overflow:ellipsis}',
    '#presetBrowser .cp52-category small{flex:0 0 auto;font:700 10px Georgia,serif;opacity:.70}',
    '#presetBrowser .cp52-category:hover{border-color:#a2b6a5;background:#ecf0df}',
    '#presetBrowser .cp52-category[aria-pressed="true"]{background:linear-gradient(90deg,#507c69,#668675);border-color:#4e715e;color:#fff8e9;box-shadow:inset 3px 0 0 #d6b478}',
    '#presetBrowser .cp52-category:focus-visible{outline:2px solid #ba9b64;outline-offset:1px}',
    '#presetBrowser #pbResultContainer{flex:1 1 auto!important;min-width:0!important;min-height:0!important;overflow-y:auto!important;overflow-x:hidden!important;padding:7px 10px 13px!important}',
    '#presetBrowser #pbItems:not([hidden]){display:flex!important;flex-direction:column!important;grid-template-columns:none!important;grid-template-rows:none!important;grid-auto-rows:auto!important;gap:4px!important;width:100%!important;height:auto!important;overflow:visible!important}',
    '#presetBrowser .pb-item{position:relative!important;display:grid!important;grid-template-columns:minmax(140px,2fr) minmax(100px,1.1fr) minmax(85px,1fr) 40px!important;gap:10px!important;align-items:center!important;flex:0 0 auto!important;width:100%!important;min-width:0!important;min-height:45px!important;height:auto!important;padding:7px 11px!important;margin:0!important;border-radius:5px!important}',
    '#presetBrowser .pb-item-name{display:block!important;-webkit-line-clamp:unset!important;white-space:nowrap!important;overflow:hidden!important;text-overflow:ellipsis!important;font:700 14px Georgia,serif!important}',
    '#presetBrowser .pb-item-family,#presetBrowser .pb-item-role{display:block!important;font:700 10px Arial,sans-serif!important;white-space:nowrap!important;overflow:hidden!important;text-overflow:ellipsis!important}',
    '#presetBrowser .pb-item-index{position:static!important;text-align:right!important;font:700 10px Georgia,serif!important;opacity:.78!important}',
    '#presetBrowser .preset-browser-summary{flex:0 0 auto!important;min-height:32px!important}',
    '#presetBrowser .preset-browser-footer{flex:0 0 auto!important;min-height:25px!important}',
    '#presetBrowser #pbMatrix:not([hidden]){width:100%;height:100%;overflow:auto!important}',
    '#presetBrowser .pb-empty{padding:22px 12px!important}',
    '#presetBrowser .pb-column-headers{display:none!important}',
    '.page.nigredo~#presetBrowser #albedo-cp52-navigation{background:linear-gradient(135deg,#283f36,#345447);border-color:#597c68}',
    '.page.nigredo~#presetBrowser .cp52-section h3{color:#d2c49a}',
    '.page.nigredo~#presetBrowser .cp52-category{color:#e3e5d1}',
    '.page.nigredo~#presetBrowser .cp52-category:hover{background:#486c56}',
    '@media(max-width:810px){#presetBrowser #albedo-cp52-navigation{flex-basis:155px!important;padding:7px 5px!important}#presetBrowser .cp52-category{padding:5px!important;font-size:10px!important}#presetBrowser .pb-item{grid-template-columns:minmax(100px,2fr) minmax(75px,1fr) minmax(70px,1fr) 32px!important;gap:5px!important}#presetBrowser .pb-item-name{font-size:12px!important}#presetBrowser .pb-item-family,#presetBrowser .pb-item-role{font-size:9px!important}}',
    '@media(max-width:570px){#presetBrowser #albedo-cp52-navigation{flex-basis:120px!important}#presetBrowser .pb-item{display:flex!important;flex-wrap:wrap;gap:4px!important}#presetBrowser .pb-item-name{width:100%}#presetBrowser .pb-item-index{margin-left:auto}}'
  ].join('\n');
  document.head.appendChild(style);

  const workspace=document.createElement('div');
  workspace.id='albedo-cp52-workspace';
  const navigation=document.createElement('nav');
  navigation.id='albedo-cp52-navigation';
  navigation.setAttribute('aria-label','Preset library, families and instrument roles');
  workspace.append(navigation,result);
  panel.insertBefore(workspace,panel.querySelector('.preset-browser-footer'));
  // CP52: explicit resizer controls stay visible while the modal is open.
  // The original CP35 corner grip is hidden by an active modal, so it cannot
  // be the user's only way to negotiate window size with Bitwig.
  const titlebar=panel.querySelector('.preset-browser-titlebar');
  const close=document.getElementById('presetBrowserClose');
  const sizing=document.createElement('div');
  sizing.className='cp52-window-controls';
  sizing.setAttribute('role','group');
  sizing.setAttribute('aria-label','Resize Albedo plugin window');
  const sizeLabel=document.createElement('output');
  sizeLabel.className='cp52-window-size';
  sizeLabel.textContent='WINDOW';
  sizing.appendChild(sizeLabel);
  function requestSize(delta){
    const controller=window.ALBEDO_CP35_WINDOW;
    if(!controller?.requestSize){sizeLabel.textContent='HOST RESIZE';return;}
    const g=controller.geometry;
    const width=Math.max(720,Math.min(1500,Math.round((g.nativeWidth||g.width||1080)+delta)));
    controller.requestSize(delta===0?1080:width);
  }
  for(const [label,title,step] of [['−','Reduce plugin window',-72],['FIT','Comfortable 1080 × 720 window',0],['+','Enlarge plugin window',72]]){
    const button=document.createElement('button');
    button.type='button';button.textContent=label;button.title=title;
    button.setAttribute('aria-label',title);
    button.addEventListener('click',()=>requestSize(step));
    sizing.appendChild(button);
  }
  titlebar?.insertBefore(sizing,close);
  // Preset A/B workflow: clicking any sound loads it while leaving the
  // browser open, so musicians can audition via MIDI without modal reopening.
  window.ALBEDO_CP52_KEEP_BROWSER_OPEN=true;
  const audition=document.createElement('button');
  audition.type='button';audition.textContent='AUDITION ON';
  audition.title='Keep browser open while loading presets to compare timbres';
  audition.setAttribute('aria-pressed','true');
  audition.setAttribute('aria-label','Keep preset browser open while auditioning');
  audition.addEventListener('click',()=>{
    window.ALBEDO_CP52_KEEP_BROWSER_OPEN=!window.ALBEDO_CP52_KEEP_BROWSER_OPEN;
    audition.textContent=window.ALBEDO_CP52_KEEP_BROWSER_OPEN?'AUDITION ON':'AUDITION OFF';
    audition.setAttribute('aria-pressed',String(window.ALBEDO_CP52_KEEP_BROWSER_OPEN));
  });
  sizing.appendChild(audition);
  dialog.addEventListener('click',event=>{
    const picked=event.target.closest?.('[data-pb-choose]');
    if(!picked)return;
    const key=picked.dataset.pbChoose;
    queueMicrotask(()=>{
      for(const button of result.querySelectorAll('[data-pb-choose]')){
        button.classList.toggle('selected',button.dataset.pbChoose===key);
      }
    });
  });
  function refreshWindowSize(){
    const g=window.ALBEDO_CP35_WINDOW?.geometry;
    if(g&&g.width>0&&g.height>0)sizeLabel.textContent=Math.round(g.nativeWidth||g.width)+' × '+Math.round(g.nativeHeight||g.height);
  }
  window.addEventListener('resize',refreshWindowSize,{passive:true});
  window.__JUCE__?.backend?.addEventListener?.('albedoViewport',refreshWindowSize);
  refreshWindowSize();

  const family=document.getElementById('pbFamily');
  const role=document.getElementById('pbRole');
  const search=document.getElementById('pbSearch');
  const sort=document.getElementById('pbSort');
  const libraryCounts=()=>[atlas.catalog,atlas.creative,atlas.newSounds||[]].reduce((a,b)=>a+b.length,0);
  function currentItems(){
    if(atlas.collection==='Factory')return atlas.catalog;
    if(atlas.collection==='Creative')return atlas.creative;
    if(atlas.collection==='New')return atlas.newSounds||[];
    return [];
  }
  function selectCategory(field,value){
    const select=field==='family'?family:role;
    if(!select)return;
    select.value=value;
    select.dispatchEvent(new Event('change',{bubbles:true}));
    window.ALBEDO_CP41_ATLAS?.refreshChips?.();
    result.scrollTop=0;
    refresh();
  }
  function addSection(title,field,items,selected){
    const counts=new Map();
    for(const item of items){
      const key=String(item[field]||'Unassigned');
      counts.set(key,(counts.get(key)||0)+1);
    }
    const section=document.createElement('section');
    section.className='cp52-section';
    const h=document.createElement('h3');
    h.textContent=title;
    section.appendChild(h);
    const labels=[['','ALL '+title,items.length],...[...counts].sort((a,b)=>a[0].localeCompare(b[0],undefined,{sensitivity:'base'})).map(([name,count])=>[name,name,count])];
    for(const [value,name,count] of labels){
      const button=document.createElement('button');
      button.type='button';
      button.className='cp52-category';
      button.setAttribute('aria-pressed',String(value===selected));
      button.title=name+' · '+count+' presets';
      const text=document.createElement('span');
      text.textContent=name;
      const number=document.createElement('small');
      number.textContent=String(count);
      button.append(text,number);
      button.addEventListener('click',()=>selectCategory(field,value));
      section.appendChild(button);
    }
    navigation.appendChild(section);
  }
  function refresh(){
    refreshWindowSize();
    const items=currentItems();
    navigation.replaceChildren();
    const heading=document.createElement('div');
    heading.className='cp52-section';
    const h=document.createElement('h3');
    h.textContent='LIBRARY / '+(atlas.collection==='New'?'MORPHOGENIC FIELDS':atlas.collection.toUpperCase());
    heading.appendChild(h);
    navigation.appendChild(heading);
    if(atlas.collection==='User'){
      const note=document.createElement('p');
      note.textContent='Imported session patches';
      note.style.padding='0 6px';
      navigation.appendChild(note);
    }else{
      addSection('FAMILIES','family',items,family?.value||'');
      addSection('ROLES','role',items,role?.value||'');
    }
    const footer=panel.querySelector('.preset-browser-footer');
    if(footer){
      const first=footer.querySelector('span');
      if(first)first.textContent=libraryCounts()+' ACTIVE SOUNDS · LIBRARY × FAMILY × INSTRUMENT ROLE';
    }
    const title=document.getElementById('presetBrowserTitle');
    if(title)title.textContent='Preset Browser';
    const hint=document.getElementById('presetBrowserHint');
    if(hint)hint.textContent='Choose a library, family and instrument role; search and sort the full sound bank.';
  }
  dialog.querySelectorAll('[data-collection]').forEach(tab=>{
    tab.addEventListener('click',()=>queueMicrotask(refresh));
  });
  document.getElementById('presetBrowseOpen')?.addEventListener('click',()=>queueMicrotask(refresh));
  document.getElementById('pbClear')?.addEventListener('click',()=>queueMicrotask(refresh));
  search?.addEventListener('input',()=>{
    // The original Atlas sorter is also its search renderer.
    sort?.dispatchEvent(new Event('change',{bubbles:true}));
  });
  family?.addEventListener('change',()=>queueMicrotask(refresh));
  role?.addEventListener('change',()=>queueMicrotask(refresh));
  if(window.__JUCE__?.backend?.addEventListener){
    window.__JUCE__.backend.addEventListener('albedoProgram',program=>{
      if(dialog.hidden)return;
      const n=Number(program);
      if(!Number.isInteger(n))return;
      for(const item of result.querySelectorAll('[data-pb-choose]')){
        const pair=String(item.dataset.pbChoose||'').split(':');
        item.classList.toggle('selected',Number(pair[1])===n);
      }
    });
  }
  window.ALBEDO_CP52_BROWSER={
    get total(){return libraryCounts()},
    get library(){return atlas.collection},
    get visible(){return atlas.filtered.length},
    refresh
  };
  refresh();
})();