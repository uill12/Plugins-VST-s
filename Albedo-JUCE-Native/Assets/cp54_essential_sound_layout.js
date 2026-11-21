/* CP54: decisive controls on the stage; original refinements in one popup.
   No parameter removals, host-ID changes, DOM reload, or pointer-move rerender. */
(()=>{
'use strict';
const page=document.getElementById('page'),api=window.ALBEDO_CP04,lab=document.getElementById('cp04LabOpen');
if(!page||!api||!lab||document.getElementById('cp54Open'))return;
const $=s=>document.querySelector(s);
const css=document.createElement('style');css.id='cp54-essential-css';
css.textContent=[
'html.albedo-fit #page #engineKnobs>.param:nth-child(n+3),',
'html.albedo-fit #page #layerKnobs,',
'html.albedo-fit #page .source-panel .inspector-body>.divider,',
'html.albedo-fit #page .source-panel .inspector-body>.callrow,',
'html.albedo-fit #page #macros>.param:nth-child(n+5),',
'html.albedo-fit #page #filterKnobsA>.param:nth-child(3),',
'html.albedo-fit #page #filterKnobsB>.param:nth-child(3),',
'html.albedo-fit #page #envKnobs>.param:nth-child(2),',
'html.albedo-fit #page #envKnobs>.param:nth-child(3),',
'html.albedo-fit #page .shape-panel .shape-block:nth-child(3) .callrow,',
'html.albedo-fit #page .lower>.surface:first-child:not(#cellArp),',
'html.albedo-fit #page #cellArp .arp-lattice,',
'html.albedo-fit #page #cellArp .arp-options,',
'html.albedo-fit #page .utility{display:none!important}',
'html.albedo-fit #page #engineKnobs{grid-template-columns:repeat(2,minmax(0,1fr))!important}',
'html.albedo-fit #page #macros{grid-template-columns:repeat(4,minmax(0,1fr))!important}',
'html.albedo-fit #page #filterKnobsA,html.albedo-fit #page #filterKnobsB,html.albedo-fit #page #envKnobs{grid-template-columns:repeat(2,minmax(0,1fr))!important}',
'html.albedo-fit #page .lower{grid-template-columns:minmax(0,1.25fr) minmax(0,1fr)!important}',
'html.albedo-fit #page #cellArp .arp-body{display:block!important;grid-template-columns:none!important}',
'html.albedo-fit #page #cellArp .arp-performance{width:100%!important}',
'html.albedo-fit #page #cellArp .pattern-controls{display:flex;justify-content:space-between;gap:8px}',
'html.albedo-fit #page .source-panel #editorStatus{font-size:9px!important}',
'#cp54Open{background:linear-gradient(110deg,#deded0,#eee5d3)!important;border-color:#ae9a73!important;color:#426355!important;white-space:nowrap}',
'#cp54Popup[hidden]{display:none!important}',
'#cp54Popup{position:fixed;inset:0;z-index:99000;display:flex;align-items:center;justify-content:center;box-sizing:border-box;padding:9px;background:#142b23c6;backdrop-filter:blur(5px);font-family:Arial,sans-serif;color:#355746}',
'#cp54Popup .cp54-panel{display:flex;flex-direction:column;width:min(1020px,94vw);height:min(760px,92dvh);min-height:0;border:1px solid #a6b89e;border-radius:9px;background:linear-gradient(140deg,#e6e9d9,#d8e3d9 65%,#f1e9d8);box-shadow:0 14px 44px #152e258c;overflow:hidden}',
'#cp54Popup header{flex:0 0 auto;display:flex;align-items:center;gap:11px;padding:10px 14px;background:linear-gradient(90deg,#d7e4d8,#eee9da);border-bottom:1px solid #9eb29d}',
'#cp54Popup header .cp54-icon{border:1px solid #b99b68;background:#f4ebd2;border-radius:50%;width:35px;height:35px;display:grid;place-items:center;font-size:23px;color:#b28c58}',
'#cp54Popup header .cp54-label{flex:1;min-width:0}',
'#cp54Popup header small{display:block;color:#5b7d6a;letter-spacing:.12em;font-size:9px;font-weight:800}',
'#cp54Popup header h2{margin:1px 0;color:#345949;font:700 22px Georgia,serif}',
'#cp54Popup header p{font-size:10px;margin:0;color:#5a7163}',
'#cp54Popup button.cp54-close{border:1px solid #9bae9e;background:#f7efd9;border-radius:5px;color:#315544;padding:8px 10px;font-weight:800;cursor:pointer}',
'#cp54Popup .cp54-body{min-height:0;flex:1;overflow:auto;padding:12px 15px 22px;scrollbar-color:#8eaa99 #d8e4d7}',
'#cp54Popup .cp54-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:11px;align-items:start}',
'#cp54Popup .cp54-group{min-width:0;padding:11px;border-radius:7px;background:#f1f0e2a8;border:1px solid #b5c7b5}',
'#cp54Popup .cp54-group>h3{font:700 15px Georgia,serif;color:#3c6450;margin:0 0 5px}',
'#cp54Popup .cp54-group>p{font-size:10px;line-height:1.3;color:#628070;margin:0 0 9px}',
'#cp54Popup .cp54-dials{display:grid;grid-template-columns:repeat(auto-fit,minmax(88px,1fr));gap:10px}',
'#cp54Popup .param{display:flex;flex-direction:column;align-items:center;justify-content:start;gap:4px;min-height:84px;text-align:center}',
'#cp54Popup .param-label{font:700 10px Arial,sans-serif;color:#426655}',
'#cp54Popup .param-value{font:600 11px Georgia,serif;color:#486a56}',
'#cp54Popup .dial{display:block;position:relative;flex:0 0 43px;width:43px;height:43px;border:2px solid #849e88;border-radius:50%;box-sizing:border-box;touch-action:none;cursor:ns-resize;outline-offset:2px;background:radial-gradient(circle at 32% 24%,#fff9df,#dedfc9 50%,#9cb6a3 82%,#668671);box-shadow:inset 0 0 0 3px #f5efdb,0 3px 5px #385d4a40}',
'#cp54Popup .dial::after{content:"";position:absolute;left:calc(50% - 2px);top:5px;width:4px;height:13px;background:var(--dialcolor,#bb955d);border-radius:3px;transform-origin:50% 16px;transform:rotate(calc(-135deg + var(--fill,0.5)*270deg))}',
'#cp54Popup .dial:focus-visible{outline:2px solid #bd9756}',
'#cp54Popup .cp54-group .surface{display:block!important;position:static!important;width:auto!important;height:auto!important;min-height:0!important;max-height:none!important;background:transparent!important;border:0!important;box-shadow:none!important;padding:0!important;margin:0!important;overflow:visible!important}',
'#cp54Popup .surface-title{height:auto!important;min-height:0!important;padding:0!important;margin-bottom:5px!important}',
'#cp54Popup .surface-title h2{font:700 14px Georgia,serif!important;color:#436550!important}',
'#cp54Popup .bodypad{height:auto!important;min-height:0!important;overflow:visible!important;padding:0!important}',
'#cp54Popup #routes{max-height:240px;overflow-y:auto}',
'#cp54Popup .route-row{display:flex;align-items:center;gap:5px;min-height:30px}',
'#cp54Popup .route-row select{flex:1;min-width:0}',
'#cp54Popup .route-row .cp38-wrap{flex:0 0 auto}',
'#cp54Popup .arp-options{display:grid!important;grid-template-columns:repeat(3,minmax(0,1fr));gap:8px}',
'#cp54Popup .arp-options label{display:flex;flex-direction:column;gap:4px;min-width:0;font:700 10px Arial,sans-serif}',
'#cp54Popup .arp-options select,#cp54Popup .arp-options input,#cp54Popup .cp54-group select{box-sizing:border-box;max-width:100%;border:1px solid #aab9a4;border-radius:4px;background:#f7f2e4;color:#355b48;padding:4px;font:600 11px Arial,sans-serif}',
'#cp54Popup .arp-lattice{display:block!important}',
'#cp54Popup .motion-controls{display:flex;justify-content:space-around;gap:8px}',
'#cp54Popup .utility{display:flex!important;flex-direction:column;gap:7px}',
'#cp54Popup .utility .surface{background:#e1eade!important;border:1px solid #c0d0bb!important;padding:6px!important}',
'#cp54Popup .utility .subrow{display:flex;align-items:center;justify-content:space-between;gap:7px}',
'#cp54Popup .utility p{font-size:10px;margin:0}',
'#cp54Popup .cp54-choice{display:flex;align-items:center;gap:8px;margin:8px 0}',
'#cp54Popup .cp54-choice label{min-width:92px;font-size:10px;font-weight:800}',
'#cp54Popup .cp54-choice select{flex:1;min-width:0}',
'#cp54Popup button.tiny-button{border:1px solid #9daF9d;border-radius:4px;padding:6px;background:#e7eddf;color:#41644e;cursor:pointer}',
'#cp54Popup .callrow{display:flex!important;align-items:center;flex-wrap:wrap;gap:8px;margin:6px 0;min-height:28px;color:#426452}',
'#cp54Popup .callrow label{font-size:10px;font-weight:700}',
'@media(max-width:630px){#cp54Popup .cp54-grid{grid-template-columns:minmax(0,1fr)}#cp54Popup header h2{font-size:18px}#cp54Popup header{padding:8px}}'
].join('\n');document.head.appendChild(css);
const openButton=document.createElement('button');
openButton.id='cp54Open';openButton.type='button';openButton.className='control-button';
openButton.textContent='◇ DETAILS';openButton.title='Open lesser-impact sound parameters';
openButton.setAttribute('aria-haspopup','dialog');openButton.setAttribute('aria-controls','cp54Popup');
lab.before(openButton);
const popup=document.createElement('div');
popup.id='cp54Popup';popup.hidden=true;popup.setAttribute('role','dialog');
popup.setAttribute('aria-modal','true');popup.setAttribute('aria-label','Detailed Albedo sound parameters');
popup.innerHTML='<div class="cp54-panel"><header><span class="cp54-icon">◇</span><div class="cp54-label"><small>ALBEDO · DEEP SOUND EDITING</small><h2>Sound refinements</h2><p>Main stage contains the strongest sonic controls · all refinements stay automatable</p></div><button id="cp54Close" class="cp54-close" type="button">✕ CLOSE</button></header><div class="cp54-body"><div class="cp54-grid" id="cp54Grid"></div></div></div>';
document.body.appendChild(popup);
const grid=popup.querySelector('#cp54Grid');let relocated=[],lastFocus=null;
function group(title,help){
 const section=document.createElement('section');section.className='cp54-group';
 const h=document.createElement('h3');h.textContent=title;
 const p=document.createElement('p');p.textContent=help;section.append(h,p);
 grid.appendChild(section);return section;
}
function cloneDialControls(section,selectors){
 const row=document.createElement('div');row.className='cp54-dials';
 for(const selector of selectors){const element=$(selector);if(element)row.appendChild(element.cloneNode(true))}
 section.appendChild(row);
}
function moveControl(node,target){
 if(!node)return;
 const marker=document.createComment('CP54 restore original');
 node.parentNode.insertBefore(marker,node);target.appendChild(node);
 relocated.push([node,marker]);
}
function proxySelect(section,selector,text){
 const original=$(selector);if(!original)return;
 const row=document.createElement('div');row.className='cp54-choice';
 const label=document.createElement('label');label.textContent=text;
 const copy=original.cloneNode(true);copy.removeAttribute('id');copy.value=original.value;
 copy.addEventListener('change',()=>{original.value=copy.value;original.dispatchEvent(new Event('change',{bubbles:true}))});
 row.append(label,copy);section.appendChild(row);
}
function show(){
 if(!popup.hidden)return;
 lastFocus=document.activeElement;grid.replaceChildren();
 const source=group('Source character','Third and fourth character, tuning, pan and individual source articulation.');
 cloneDialControls(source,['#engineKnobs>.param:nth-child(3)','#engineKnobs>.param:nth-child(4)','#layerKnobs>.param:nth-child(1)','#layerKnobs>.param:nth-child(2)','#layerKnobs>.param:nth-child(3)']);
 const key=$('#keytrack');
 if(key){
  const button=document.createElement('button');button.type='button';button.className='tiny-button';
  button.textContent='KEYTRACK · '+key.textContent;
  button.addEventListener('click',()=>{key.click();button.textContent='KEYTRACK · '+key.textContent});
  source.appendChild(button);
 }
 const macros=group('Additional macro domains','Width, growth and memory remain accessible, but not on the primary stage.');
 cloneDialControls(macros,['#macros>.param:nth-child(5)','#macros>.param:nth-child(6)','#macros>.param:nth-child(7)']);
 const filters=group('Filter and envelope refinement','Drive, decay, sustain, pitch behavior and filter architecture.');
 cloneDialControls(filters,['#filterKnobsA>.param:nth-child(3)','#filterKnobsB>.param:nth-child(3)','#envKnobs>.param:nth-child(2)','#envKnobs>.param:nth-child(3)']);
 proxySelect(filters,'#filterRoute','Filter route');
 const pitchRow=$('#pitchMode')?.closest('.callrow');
 const voiceRow=$('#voiceMode')?.closest('.callrow');
 moveControl(pitchRow,filters);moveControl(voiceRow,filters);
 const routing=group('Symbolic assignments','Original modulation routing controls, including depth and add/remove.');
 moveControl($('#page .lower>.surface:not(#cellArp)'),routing);
 const motion=group('Cellular and rhythm detail','Rule, motion parameters and rhythmic refinements; main note events remain visible.');
 moveControl($('#cellArp .arp-lattice'),motion);
 moveControl($('#cellArp .arp-options'),motion);
 const system=group('Performance and output','Tuning, oversampling and safety controls.');
 moveControl($('#page .utility'),system);
 popup.hidden=false;openButton.setAttribute('aria-expanded','true');
 popup.querySelector('#cp54Close').focus();
}
function close(){
 if(popup.hidden)return;
 for(let i=relocated.length-1;i>=0;--i){
  const [node,marker]=relocated[i];
  if(marker.parentNode)marker.parentNode.insertBefore(node,marker);
  marker.remove();
 }
 relocated=[];grid.replaceChildren();popup.hidden=true;
 openButton.setAttribute('aria-expanded','false');
 if(lastFocus?.isConnected)lastFocus.focus();
}
openButton.addEventListener('click',show);
popup.querySelector('#cp54Close').addEventListener('click',close);
popup.addEventListener('pointerdown',e=>{if(e.target===popup)close()});
document.addEventListener('keydown',e=>{
 if(popup.hidden)return;
 if(e.key==='Escape'){e.preventDefault();close();return}
 if(e.key!=='Tab')return;
 const controls=[...popup.querySelectorAll('button:not([disabled]),select:not([disabled]),input:not([disabled]),[tabindex="0"]')]
  .filter(el=>el.getClientRects().length);
 if(!controls.length)return;
 if(e.shiftKey&&document.activeElement===controls[0]){e.preventDefault();controls[controls.length-1].focus()}
 else if(!e.shiftKey&&document.activeElement===controls[controls.length-1]){e.preventDefault();controls[0].focus()}
},true);
window.__JUCE__?.backend?.addEventListener?.('albedoProgram',close);
$('#method')?.addEventListener('change',()=>{if(!popup.hidden)close()});
window.ALBEDO_CP54_ESSENTIAL={open:show,close,get isOpen(){return !popup.hidden},primaryOnly:true};
})();