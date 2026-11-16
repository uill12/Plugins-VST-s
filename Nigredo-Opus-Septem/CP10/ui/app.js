
const P = (label,def=50,min=0,max=100,unit='%',step=1,short='') => ({label,def,min,max,unit,step,short});
const operations = [
 {name:'Calcination',latin:'CALCINATIO',symbol:'♄',color:'#45413e',tone:'#b6a99b',motto:'IGNIS · FIRE',
  audio:{desc:'Excitation by fire. A harmonic forge for confident, saturated material without needless aliasing.',method:'DESIGN TARGET · oversampled, band-limited nonlinear drive with level compensation.',
   macro:[P('HEAT',40),P('DRIVE',27),P('MIX',55)],character:[P('HARMONICS',40),P('BIAS',50),P('BODY',60)],refine:[P('PRE-FILTER',65),P('TRANSIENT',42),P('TRIM',0,-12,12,' dB',.5)]},
  midi:{desc:'Transform note energy and articulation. Dynamics and gate behavior, never an audio tone generator.',method:'DESIGN TARGET · incoming note velocity and note-on/off transformation.',
   macro:[P('ACCENT',34),P('VELOCITY',70),P('GATE',78)],character:[P('CURVE',48),P('MIN VEL',12,1,127,'',1),P('EXPRESSION',65)],refine:[P('VELOCITY LIMIT',120,1,127,'',1),P('GATE BIAS',50),P('SAFE NOTES',100)]}},
 {name:'Dissolution',latin:'SOLUTIO',symbol:'☽',color:'#8c5b88',tone:'#d6b1d0',motto:'AQUA · WATER',
  audio:{desc:'Loosen the source into flowing space. Diffusion, trails, and soft temporal displacement.',method:'DESIGN TARGET · decorrelated stereo diffusion, carefully damped delay lines.',
   macro:[P('DIFFUSION',44),P('DECAY',35),P('MIX',25)],character:[P('PREDELAY',26,0,160,' ms'),P('SPREAD',60),P('DAMPING',55)],refine:[P('EARLY',48),P('MOD RATE',.25,.01,2,' Hz',.01),P('TAIL FILTER',66)]},
  midi:{desc:'Dissolve rhythmic rigidity without producing new sound. Natural timing and overlap of incoming notes.',method:'DESIGN TARGET · bounded timestamp variation and note-length reshaping.',
   macro:[P('HUMANIZE',24),P('TIME SPREAD',12,0,75,' ms'),P('LEGATO',22)],character:[P('SWING',0,-50,50,'%'),P('DURATION',60),P('CLUSTER',20)],refine:[P('SEED',42,0,999,'',1),P('LATE BIAS',50),P('SAFETY',100)]}},
 {name:'Separation',latin:'SEPARATIO',symbol:'♂',color:'#b65a54',tone:'#e8a097',motto:'DISCRIMEN · DIVIDE',
  audio:{desc:'Reveal structure within a complex signal. Separate bands, stereo placement, and transients.',method:'DESIGN TARGET · phase-aware crossover and complementary reconstruction.',
   macro:[P('CROSSOVER',1200,100,8000,' Hz',10),P('FOCUS',55),P('BALANCE',50)],character:[P('MID/SIDE',50),P('TRANSIENT',45),P('LOW BODY',52)],refine:[P('SLOPE',24,6,48,' dB',6),P('PHASE',50),P('LINK',80)]},
  midi:{desc:'Divide incoming notes by range, velocity, or musical role while preserving note identity.',method:'DESIGN TARGET · MIDI channel/register masks, reliable note-off mapping.',
   macro:[P('SPLIT NOTE',60,24,96,'',1),P('LOW ROUTE',50),P('HIGH ROUTE',50)],character:[P('VEL SPLIT',64,1,127,'',1),P('OCTAVE RANGE',3,1,6,' oct',1),P('DENSITY',80)],refine:[P('HYSTERESIS',0,0,12,' st',1),P('MPE SAFE',100),P('NOTE TRACK',100)]}},
 {name:'Conjunction',latin:'CONIUNCTIO',symbol:'☉',color:'#dbb447',tone:'#f3d988',motto:'UNIO · UNITE',
  audio:{desc:'Unite separated materials. Enhance harmony, depth, and cohesion through controlled recombination.',method:'DESIGN TARGET · phase-stable parallel harmonics and level-matched blends.',
   macro:[P('UNISON',45),P('HARMONIC',39),P('BLEND',46)],character:[P('WIDTH',55),P('COHERENCE',65),P('FUNDAMENTAL',53)],refine:[P('DE-TUNE',4,0,40,' ct'),P('CROSSFEED',15),P('CEILING',-1,-12,0,' dB',.1)]},
  midi:{desc:'Unite incoming notes into chords, intervals, and meaningful relationships.',method:'DESIGN TARGET · scale-aware voice generation with bounded voice counts.',
   macro:[P('INTERVAL',7,-12,12,' st',1),P('VOICES',2,1,6,'',1),P('BLEND',65)],character:[P('INVERSION',0,-3,3,'',1),P('SPREAD',40),P('CHORD CHANCE',70)],refine:[P('VOICE LEADING',80),P('DUPLICATE GUARD',100),P('CHANNEL',1,1,16,'',1)]}},
 {name:'Fermentation',latin:'FERMENTATIO',symbol:'♀',color:'#3e987d',tone:'#97c2a4',motto:'VITA · BECOME',
  audio:{desc:'Allow matter to evolve slowly. Living movement, spectral drift, and controlled feedback.',method:'DESIGN TARGET · smoothed modulation, stable bounded feedback, no zipper noise.',
   macro:[P('MOVEMENT',58),P('GROWTH',36),P('TEXTURE',54)],character:[P('RATE',.18,.01,5,' Hz',.01),P('FEEDBACK',22),P('STEREO DRIFT',61)],refine:[P('SEED',33,0,999,'',1),P('DAMPING',70),P('CHAOS LIMIT',35)]},
  midi:{desc:'Evolve a phrase organically. Conditional note mutations respond to the notes actually received.',method:'DESIGN TARGET · deterministic-seed stochastic edits; repeatable session recall.',
   macro:[P('MUTATION',22),P('PROBABILITY',38),P('EVOLUTION',64)],character:[P('PITCH RANGE',3,0,12,' st',1),P('RHYTHM VAR',35),P('REPEAT',42)],refine:[P('SEED',110,0,999,'',1),P('CHANGE CAP',18),P('REST CHANCE',8)]}},
 {name:'Distillation',latin:'DISTILLATIO',symbol:'☿',color:'#c98b42',tone:'#edbe77',motto:'SUBTILITAS · PURIFY',
  audio:{desc:'Clarify the essence. Selective spectral cleaning and transparent contour refinement.',method:'DESIGN TARGET · low-noise EQ with smooth gain updates and stable filter topology.',
   macro:[P('PURITY',56),P('AIR',48),P('WEIGHT',45)],character:[P('CENTER',2600,200,12000,' Hz',10),P('RESONANCE',25),P('DYNAMIC EQ',35)],refine:[P('Q',.8,.2,8,'',.1),P('TILT',0,-12,12,' dB',.1),P('SOFTNESS',72)]},
  midi:{desc:'Retain the essential musical information. Filter notes by scale, density, and role.',method:'DESIGN TARGET · scale masks, note thinning and stable note-on/off bookkeeping.',
   macro:[P('SCALE ROOT',0,0,11,' st',1),P('SCALE STRENGTH',75),P('DENSITY',74)],character:[P('REGISTER',60,24,96,'',1),P('SIMPLICITY',38),P('DEDUPE',100)],refine:[P('NEAR NOTE',65),P('FORCED ROOT',0),P('NOTE PRIORITY',60)]}},
 {name:'Coagulation',latin:'COAGULATIO',symbol:'☾',color:'#c9baa0',tone:'#f0e5cc',motto:'CORPUS · FIX',
  audio:{desc:'Give the result durable form. Transient definition, dynamics control, and finished output.',method:'DESIGN TARGET · transparent linked dynamics, look-ahead option, true-peak ceiling.',
   macro:[P('GLUE',45),P('IMPACT',43),P('OUTPUT',-1,-18,6,' dB',.5)],character:[P('ATTACK',18,0,120,' ms',1),P('RELEASE',120,10,800,' ms',10),P('RATIO',2,1,12,':1',.5)],refine:[P('SOFT KNEE',45),P('LOOKAHEAD',2,0,10,' ms',.5),P('CEILING',-1,-12,0,' dB',.1)]},
  midi:{desc:'Fix musical ideas into a deliberate grid and articulation. Make expressive notes performance-ready.',method:'DESIGN TARGET · transport-aware quantization and safe gate termination.',
   macro:[P('QUANTIZE',72),P('DURATION',62),P('STRENGTH',83)],character:[P('GRID',16,1,32,'th',1),P('GATE',72),P('RETRIGGER',40)],refine:[P('OFFSET',0,-50,50,' ms',1),P('HOLD',5,0,100,'%'),P('STUCK NOTE SAFE',100)]}}
];
/* NIGREDO — OPUS SEPTEM, GUI CP04. All visual fields are synthetic browser previews. */
(() => {
'use strict';
const $ = id => document.getElementById(id);
const roman = ['I','II','III','IV','V','VI','VII'];
const glyphs = ['✺','☽','⫽','⋈','✣','☿','⬡'];
const hues = ['#bd513b','#548595','#9a5b74','#c69838','#56845a','#6473a0','#a77e50'];
const phrases = [
 'Intensity reveals hidden structure.', 'Let boundaries become fluid.',
 'Difference makes detail visible.', 'Unite unlike materials into one coherent whole.',
 'Allow small changes to become living form.', 'Keep only what serves the material.',
 'Give movement a stable final shape.'
];
const operationFunctions = {
 audio:['SATURATION / DRIVE','STEREO DELAY / DIFFUSION','MULTIBAND / TRANSIENT','HARMONIC BLEND','MODULATION / FEEDBACK','SPECTRAL EQ','COMPRESSION / LIMITER'],
 midi:['VELOCITY SHAPER','TIMING / LEGATO','NOTE SPLIT / ROUTING','CHORD HARMONIZER','PROBABILITY / MUTATION','SCALE / NOTE FILTER','QUANTIZE / GATE']
};
const scopes = ['HARMONIC HEAT','DIFFUSION / FLOW','SPECTRAL DIVISION','HARMONIC FUSION','EVOLVING MATERIAL','SELECTIVE CLARITY','DYNAMIC STRUCTURE'];
const presets = {
 audio: [
  {id:'prima',name:'Prima Materia',mods:[1,1,1,1,1,1,1]},
  {id:'vitriol',name:'Vitriolic Etch',mods:[1.55,.65,1.3,1.08,1.36,1.1,1.2]},
  {id:'mercurial',name:'Mercurial Bloom',mods:[.55,1.6,.7,1.2,1.6,1.1,.75]},
  {id:'lapis',name:'Lapis / Polished',mods:[.68,.72,1.25,1,.8,1.4,1.3]},
  {id:'tabula',name:'Tabula Rasa',mods:[0,0,0,0,0,0,0]}
 ],
 midi: [
  {id:'prima',name:'Prima Nota',mods:[1,1,1,1,1,1,1]},
  {id:'canon',name:'Hermetic Canon',mods:[.76,.45,1.12,1.48,.5,1.2,1.3]},
  {id:'chance',name:'Chance & Order',mods:[1.12,1.2,.85,1.15,1.73,1.08,1.28]},
  {id:'ritual',name:'Ritual Measure',mods:[1.33,.8,1.05,1.33,1.22,.92,1.5]},
  {id:'tabula',name:'Tabula Rasa',mods:[0,0,0,0,0,0,0]}
 ]
};
const masterDefs = {
 audio: [
  {key:'mix', ...P('DRY / WET',100,0,100,'%',1)},
  {key:'output', ...P('OUTPUT',0,-18,12,' dB',.5)},
  {key:'quality', ...P('QUALITY',4,1,8,'×',1)}
 ],
 midi: [
  {key:'strength', ...P('FX AMOUNT',100,0,100,'%',1)},
  {key:'transpose', ...P('TRANSPOSE',0,-24,24,' st',1)},
  {key:'voicecap', ...P('VOICE CAP',16,1,32,'',1)}
 ]
};
const makeState = kind => ({selected:3,order:[0,1,2,3,4,5,6],enabled:[true,true,true,true,true,true,true],bypass:false,values:{},master:Object.fromEntries(masterDefs[kind].map(v=>[v.key,v.def])),preset:'prima'});
const states = {audio:makeState('audio'),midi:makeState('midi')};
let mode='audio', drag=null, toastTimeout=null;
const isReducedMotion = matchMedia('(prefers-reduced-motion: reduce)').matches;
const state = () => states[mode];
const paramKey = (i,layer,j)=>`${i}:${layer}:${j}`;
const specFor = (i,layer,j) => operations[i][mode][layer][j];
const valueFor = (i,layer,j) => state().values[paramKey(i,layer,j)] ?? specFor(i,layer,j).def;
const clamp = (n,min,max) => Math.max(min,Math.min(max,n));
const roundToStep = (value,spec) => {
 const n=clamp(value,spec.min,spec.max);
 const snapped = spec.min+Math.round((n-spec.min)/spec.step)*spec.step;
 return clamp(Number(snapped.toFixed(5)),spec.min,spec.max);
};
const format = (value,spec) => {
 const dec = spec.step < .1 ? 2 : spec.step < 1 ? 1 : 0;
 return `${Number(value).toFixed(dec)}${spec.unit||''}`;
};
const colorToRgb = hex => [1,3,5].map(n=>parseInt(hex.slice(n,n+2),16)).join(',');
function toast(message){const el=$('toast');el.textContent=message;el.classList.add('show');clearTimeout(toastTimeout);toastTimeout=setTimeout(()=>el.classList.remove('show'),2700);}
function setAccent(){const c=hues[state().selected];document.documentElement.style.setProperty('--accent',c);document.documentElement.style.setProperty('--accent-rgb',colorToRgb(c));}
function renderModes(){
 $('audioMode').classList.toggle('active',mode==='audio');$('midiMode').classList.toggle('active',mode==='midi');
 $('audioMode').setAttribute('aria-pressed',String(mode==='audio'));$('midiMode').setAttribute('aria-pressed',String(mode==='midi'));
 $('powerBtn').setAttribute('aria-pressed',String(!state().bypass));$('powerBtn').classList.toggle('off',state().bypass);$('powerBtn').querySelector('.power-status').textContent=state().bypass?'OFF':'ON';$('powerBtn').setAttribute('aria-label',state().bypass?'Turn effects chain on':'Turn effects chain off');$('cosmogram').classList.toggle('unlit',state().bypass||!state().enabled[state().selected]);$('chainStatusLabel').textContent=mode==='audio'?'AUDIO · 7 MODULES':'MIDI · 7 MODULES';
 const audio=mode==='audio';
 $('pathHeader').textContent=audio?'STEREO IN → STEREO OUT':'MIDI IN → MIDI OUT';
 $('pathSubtitle').textContent=audio?'HOST AUDIO · NO OSCILLATOR':'HOST NOTE EVENTS · NO KEYBOARD';
 $('fieldIdentity').textContent=audio?'AUDIO / SIGNAL STUDY':'MIDI / NOTE STUDY';
 $('chainTypeLabel').textContent=audio?'AUDIO · SEVEN STAGES':'NOTE EVENTS · SEVEN STAGES';
 $('inputName').textContent=audio?'AUDIO IN':'MIDI IN';$('inputSpec').textContent=audio?'STEREO · L / R':'HOST NOTE EVENTS';
 $('outputName').textContent=audio?'AUDIO OUT':'MIDI OUT';$('outputSpec').textContent=audio?'STEREO · L / R':'MODIFIED NOTE EVENTS';
 $('safetyLabel').textContent=audio?'HOST AUDIO · VISUAL ROUTING':'MIDI NOTES · VISUAL ROUTING';
}
function renderStages(){
 const s=state();
 $('stagesList').innerHTML=s.order.map((i,position)=>{
  const op=operations[i],selected=s.selected===i,enabled=s.enabled[i];
  const functionName=operationFunctions[mode][i];
  return `<div class="stage-item ${selected?'selected':''} ${!enabled?'muted':''}" style="--stage:${hues[i]}" data-stage="${i}">
     <button type="button" class="stage-select" data-select="${i}" aria-pressed="${selected}" aria-label="Edit ${functionName}, ${op.name}">
      <span class="number">${roman[i]}</span><span class="stage-glyph">${glyphs[i]}</span>
      <span class="stage-copy"><b>${functionName}</b><small>${op.name.toUpperCase()} · ${op.latin}</small></span></button>
     <button type="button" class="stage-toggle ${enabled?'':'off'}" data-toggle="${i}" aria-label="${enabled?'Disable':'Enable'} ${functionName}" title="${enabled?'Disable':'Enable'} ${functionName}" aria-pressed="${enabled}"><span class="toggle-text">${enabled?'ON':'OFF'}</span></button>
    </div>`;
 }).join('');
 $('stagePath').innerHTML=s.order.map(i=>`<button type="button" class="path-node ${s.selected===i?'selected':''} ${s.enabled[i]?'':'off'}" style="--stage:${hues[i]}" data-path="${i}" title="${operations[i].name}" aria-label="Select ${operations[i].name}"><span class="node-symbol">${glyphs[i]}</span><b>${roman[i]}</b><small>${operations[i].latin}</small></button>`).join('');
}
function kindOf(spec,context){
 const name=spec.label.toUpperCase();
 if(/SAFE NOTES|MPE SAFE|NOTE TRACK|DUPLICATE GUARD|DEDUPE|FORCED ROOT|STUCK NOTE SAFE/.test(name))return 'switch';
 if(/GRID|SLOPE|CHANNEL|QUALITY|SCALE ROOT|VOICES|OCTAVE RANGE/.test(name))return 'choice';
 if(/MIX|BLEND|DRY \/ WET|OUTPUT|TRIM|CEILING|CROSSOVER|CENTER|RATIO|SPLIT NOTE|TIME SPREAD|ATTACK|RELEASE|PREDELAY|LOOKAHEAD/.test(name))return 'fader';
 return 'knob';
}
function knobMarkup(spec,value,context,attrs={}){
 const kind=kindOf(spec,context),details=Object.entries(attrs).map(([k,v])=>`data-${k}="${v}"`).join(' '),base=`data-context="${context}" ${details}`;
 const progress=(value-spec.min)/(spec.max-spec.min||1),angle=clamp(progress,0,1)*270;
 let ctl='';
 if(kind==='switch')ctl=`<button type="button" class="param-switch ${value>=50?'armed':''}" aria-pressed="${value>=50}" ${base} aria-label="Toggle ${spec.label}"><i class="switch-gem">✥</i><small>${value>=50?'ENGAGED':'RELEASED'}</small></button>`;
 else if(kind==='choice'){
  const opts=spec.label==='QUALITY'?[1,2,4,8]:spec.label==='GRID'?[1,2,4,8,16,32]:spec.label==='SLOPE'?[6,12,18,24,30,36,42,48]:Array.from({length:Math.floor((spec.max-spec.min)/spec.step)+1},(_,i)=>Number((spec.min+i*spec.step).toFixed(3)));
  const pitch=['C','C♯','D','D♯','E','F','F♯','G','G♯','A','A♯','B'];
  ctl=`<select class="param-choice" ${base} aria-label="${spec.label}">${opts.map(n=>`<option value="${n}" ${n===value?'selected':''}>${spec.label==='SCALE ROOT'?pitch[n]:format(n,spec)}</option>`).join('')}</select>`;
 }else if(kind==='fader')ctl=`<div class="fader-track"><input type="range" class="param-fader" ${base} aria-label="${spec.label}" min="${spec.min}" max="${spec.max}" step="${spec.step}" value="${value}" style="--ratio:${progress*100}%"><span class="fader-runes">◁ ┈ ┈ ┈ ✧ ┈ ┈ ┈ ▷</span></div>`;
 else ctl=`<button type="button" class="knob" role="slider" tabindex="0" ${base} aria-label="${spec.label}" aria-valuemin="${spec.min}" aria-valuemax="${spec.max}" aria-valuenow="${value}" aria-valuetext="${format(value,spec)}" style="--angle:${angle}deg;--angle-turn:${angle-135}deg" title="Drag / mouse wheel / arrow keys · Double-click to reset"><span class="dial-inscription">☼</span></button>`;
 return `<div class="param param-${kind}">${ctl}<span class="param-name">${spec.label}</span><span class="param-val">${format(value,spec)}</span></div>`;
}
function renderInspector(){
 const i=state().selected,op=operations[i],p=op[mode],s=state();
 $('editorIndex').textContent=`${roman[i]} / VII`;
 $('fieldRoman').textContent=roman[i];$('fieldName').textContent=op.name;
 $('fieldLatin').textContent=op.latin;$('fieldMotto').textContent=op.motto;
 $('scopeLabel').textContent=`${window.NigredoHost?.active?'HOST ':'SIMULATED '}${mode==='audio'?'SIGNAL':'NOTE FIELD'} / ${scopes[i]}`;$('scopeFlow').textContent=String(s.order.indexOf(i)+1).padStart(2,'0');
 $('editorTitle').textContent=op.name;$('editorLatin').textContent=op.latin;
 $('editorGlyph').textContent=glyphs[i];$('editorDesc').textContent=p.desc;
 $('stagePower').textContent=s.enabled[i]?'ON':'OFF';$('stagePower').classList.toggle('off',!s.enabled[i]);$('stagePower').classList.toggle('on',s.enabled[i]);$('stagePower').setAttribute('aria-pressed',String(s.enabled[i]));
 $('stagePhrase').textContent=`“${phrases[i]}”`;
 $('stageMethod').textContent=p.method.replace(/^DESIGN TARGET\s*·\s*/,'');
 $('cosmogram').dataset.stage=String(i);$('cosmogram').classList.toggle('unlit',s.bypass||!s.enabled[i]);
 $('parameters').innerHTML=['macro','character','refine'].map((layer,n)=>{
  const arr=p[layer];return `<div class="param-group"><div class="param-heading"><span>0${n+1} / ${layer.toUpperCase()}</span><i>${['SHAPE','MOVEMENT','FINISH'][n]}</i></div><div class="knob-row">${arr.map((spec,j)=>knobMarkup(spec,valueFor(i,layer,j),'param',{stage:i,layer,slot:j})).join('')}</div></div>`;
 }).join('');
 const index=s.order.indexOf(i);$('moveEarlier').disabled=index===0;$('moveLater').disabled=index===6;
}
function renderMaster(){
 $('masterControls').innerHTML=masterDefs[mode].map(s=>knobMarkup(s,state().master[s.key],'master',{master:s.key})).join('');
 const options=presets[mode];
 $('presetSelect').innerHTML=(state().preset==='custom'?'<option value="custom" selected>Custom / Unsaved</option>':'')+options.map(p=>`<option value="${p.id}" ${p.id===state().preset?'selected':''}>${p.name}</option>`).join('');
}
function render(){setAccent();renderModes();renderStages();renderInspector();renderMaster();}
function updateStage(){setAccent();renderStages();renderInspector();}
function switchMode(next){if(mode===next)return;mode=next;render();}
function selectStage(i){state().selected=i;updateStage();if(window.chrome?.webview?.postMessage)window.chrome.webview.postMessage(`P|81|${i}`);}
function toggleStage(i){state().enabled[i]=!state().enabled[i];if(window.chrome?.webview?.postMessage)window.chrome.webview.postMessage(`P|${63+i}|${state().enabled[i]?1:0}`);renderStages();if(state().selected===i)renderInspector();renderModes();}
function moveStage(delta){const s=state(),p=s.order.indexOf(s.selected),t=p+delta;if(t<0||t>6)return;[s.order[p],s.order[t]]=[s.order[t],s.order[p]];renderStages();renderInspector();sendAllState();toast(`${operations[s.selected].name} → stage position ${t+1}`);}
function applyPreset(presetId){
 const selected=presets[mode].find(x=>x.id===presetId);if(!selected)return;
 const s=state();s.values={};s.order=[0,1,2,3,4,5,6];
 s.enabled=selected.mods.map(n=>n!==0);
 for(let i=0;i<7;i++) for(const layer of ['macro','character','refine']) operations[i][mode][layer].forEach((sp,j)=>{
   const m=selected.mods[i];
   if(m===1)return;
   if(m===0){s.values[paramKey(i,layer,j)]=sp.def;return;}
   if(/SAFE|LIMIT|CAP|CEILING|DEDUPE|NOTE TRACK|MPE/.test(sp.label))return;
   const scaled=sp.min+(sp.def-sp.min)*m;
   s.values[paramKey(i,layer,j)]=roundToStep(scaled,sp);
 });
 s.preset=presetId;render();sendAllState();toast(`Formula applied · ${selected.name}`);
}
function exportPreset(){
 const data={format:'nigredo-opus-septem-gui',version:4,created:new Date().toISOString(),kind:mode,...structuredClone(state())};
 const blob=new Blob([JSON.stringify(data,null,2)],{type:'application/json'});
 const url=URL.createObjectURL(blob),a=document.createElement('a');
 a.href=url;a.download=`Opus_Septem_${mode}_${state().preset}_${Date.now()}.json`;
 a.click();setTimeout(()=>URL.revokeObjectURL(url),2000);toast('GUI settings exported as JSON');
}
function importPreset(file){
 if(!file)return;const reader=new FileReader();
 reader.onload=()=>{try{
  const parsed=JSON.parse(reader.result);
  if(parsed.format!=='nigredo-opus-septem-gui'||!['audio','midi'].includes(parsed.kind))throw Error('Unsupported preset format');
  const kind=parsed.kind,s=states[kind];
  const order=parsed.order,enabled=parsed.enabled;
  if(!Array.isArray(order)||order.length!==7||new Set(order).size!==7||!order.every(i=>Number.isInteger(i)&&i>=0&&i<7)||!Array.isArray(enabled)||enabled.length!==7||!enabled.every(x=>typeof x==='boolean'))throw Error('Invalid stage configuration');
  s.order=[...order];s.enabled=[...enabled];s.selected=clamp(Number(parsed.selected)||0,0,6);s.bypass=Boolean(parsed.bypass);s.values={};
  for(let i=0;i<7;i++)for(let layer of ['macro','character','refine'])for(let j=0;j<3;j++){
   const key=paramKey(i,layer,j),n=parsed.values?.[key],spec=operations[i][kind][layer][j];
   if(typeof n==='number'&&Number.isFinite(n))s.values[key]=roundToStep(n,spec);
  }
  for(const spec of masterDefs[kind]){
   const n=parsed.master?.[spec.key];if(typeof n==='number'&&Number.isFinite(n))s.master[spec.key]=roundToStep(n,spec);
  }
  s.preset=presets[kind].some(x=>x.id===parsed.preset)?parsed.preset:'prima';
  mode=kind;render();sendAllState();toast('GUI settings loaded · '+(kind==='audio'?'Audio FX':'MIDI Note FX'));
 }catch(e){toast(`Import failed: ${e.message}`);} $('presetFile').value='';};
 reader.readAsText(file);
}
function descriptor(el){
 if(el.dataset.context==='master'){
  const spec=masterDefs[mode].find(x=>x.key===el.dataset.master);
  return {spec,current:()=>state().master[spec.key],set:n=>{state().master[spec.key]=roundToStep(n,spec);}};
 }
 const i=Number(el.dataset.stage),layer=el.dataset.layer,j=Number(el.dataset.slot),spec=specFor(i,layer,j);
 return {spec,current:()=>valueFor(i,layer,j),set:n=>{state().values[paramKey(i,layer,j)]=roundToStep(n,spec);}};
}
function hostValueChanged(el,val){
 if(!window.chrome?.webview?.postMessage||window.__nigHostLoading)return;
 let id;
 if(el.dataset.context==='master'){id=77+masterDefs[mode].findIndex(s=>s.key===el.dataset.master);}
 else id=Number(el.dataset.stage)*9+['macro','character','refine'].indexOf(el.dataset.layer)*3+Number(el.dataset.slot);
 if(id>=0)window.chrome.webview.postMessage(`P|${id}|${val}`);
}
function sendAllState(){
 if(!window.chrome?.webview?.postMessage||window.__nigHostLoading)return;
 for(let i=0;i<7;i++)for(let layer of ['macro','character','refine'])for(let j=0;j<3;j++){
  const idx=i*9+['macro','character','refine'].indexOf(layer)*3+j;
  window.chrome.webview.postMessage(`P|${idx}|${valueFor(i,layer,j)}`);
 }
 const s=state();for(let i=0;i<7;i++){
  window.chrome.webview.postMessage(`P|${63+i}|${s.enabled[i]?1:0}`);
  window.chrome.webview.postMessage(`P|${70+i}|${s.order.indexOf(i)}`);
 }
 masterDefs[mode].forEach((sp,i)=>window.chrome.webview.postMessage(`P|${77+i}|${s.master[sp.key]}`));
 window.chrome.webview.postMessage(`P|80|${s.bypass?1:0}`);
 window.chrome.webview.postMessage(`P|81|${s.selected}`);
}
function updateKnob(el,value,fromHost=false){
 const d=descriptor(el);d.set(value);const v=d.current(),angle=270*(v-d.spec.min)/(d.spec.max-d.spec.min||1);
 el.style.setProperty('--angle',`${angle}deg`);el.style.setProperty('--angle-turn',`${angle-135}deg`);
 el.style.setProperty('--ratio',`${100*(v-d.spec.min)/(d.spec.max-d.spec.min||1)}%`);
 el.setAttribute('aria-valuenow',String(v));el.setAttribute('aria-valuetext',format(v,d.spec));
 const display=el.closest('.param');if(display){display.querySelector('.param-val').textContent=format(v,d.spec);
 if(el.classList.contains('param-switch')){el.classList.toggle('armed',v>=50);el.setAttribute('aria-pressed',String(v>=50));el.querySelector('small').textContent=v>=50?'ENGAGED':'RELEASED';}
 }
 if(!fromHost){hostValueChanged(el,v);state().preset='custom';
 if($('presetSelect').querySelector('option[value="custom"]')===null){const opt=document.createElement('option');opt.value='custom';opt.textContent='Custom / Unsaved';$('presetSelect').prepend(opt);}
 $('presetSelect').value='custom';}
}
window.NigredoHost={
 active:false,
 apply:(payload)=>{ // Host-synchronized state. CP07 browser rendering and embedded WebView2 use the SAME UI code.
   const msg=typeof payload==='string'?JSON.parse(payload):payload;if(!msg||msg.t!=='state'||!Array.isArray(msg.p))return;
   const next=msg.kind==='midi'?'midi':'audio';window.__nigHostLoading=true;
   if(mode!==next){mode=next;}
   $('audioMode').disabled=next!=='audio';$('midiMode').disabled=next!=='midi';
   $('audioMode').title=next==='audio'?'Audio FX plugin':'This native plugin is MIDI FX only';
   $('midiMode').title=next==='midi'?'MIDI FX plugin':'This native plugin is Audio FX only';
   const s=state();
   for(let id=0;id<63;id++)if(typeof msg.p[id]==='number'&&Number.isFinite(msg.p[id])){
     const i=Math.floor(id/9),j=id%9,layer=['macro','character','refine'][Math.floor(j/3)];
     const sp=specFor(i,layer,j%3);s.values[paramKey(i,layer,j%3)]=roundToStep(msg.p[id],sp);
   }
   for(let i=0;i<7;i++)s.enabled[i]=msg.p[63+i]>=.5;
   const order=[0,1,2,3,4,5,6];order.sort((a,b)=>(msg.p[70+a]??a)-(msg.p[70+b]??b));s.order=order;
   masterDefs[mode].forEach((sp,i)=>{if(typeof msg.p[77+i]==='number')s.master[sp.key]=roundToStep(msg.p[77+i],sp);});
   s.bypass=msg.p[80]>=.5;s.selected=clamp(Math.round(msg.p[81]??s.selected),0,6);render();
   window.__nigHostLoading=false;window.NigredoHost.active=true;
   document.body.classList.add('native-plugin');$('audioMode').disabled=next==='midi';$('midiMode').disabled=next==='audio';
   $('scopeLabel').textContent=(mode==='audio'?'HOST AUDIO / ':'HOST MIDI / ')+scopes[s.selected];
   $('hostNotice').textContent='NATIVE PLUGIN CONNECTED · AUTOMATABLE PARAMETERS';
   document.querySelector('.master-footer').lastElementChild.textContent='DSP / MIDI ENGINE LINKED · SCOPE IS SYMBOLIC';
 }
};
if(window.chrome?.webview?.addEventListener){
  window.chrome.webview.addEventListener('message',e=>{try{window.NigredoHost.apply(e.data);}catch(error){console.warn('Host sync:',error);}});
  window.chrome.webview.postMessage('R');
}
$('audioMode').addEventListener('click',()=>{if(!window.chrome?.webview?.postMessage)switchMode('audio');});
$('midiMode').addEventListener('click',()=>{if(!window.chrome?.webview?.postMessage)switchMode('midi');});
$('powerBtn').addEventListener('click',()=>{state().bypass=!state().bypass;renderModes();if(window.chrome?.webview?.postMessage)window.chrome.webview.postMessage(`P|80|${state().bypass?1:0}`);toast(state().bypass?'FX chain bypassed':'FX chain engaged');});
$('stagePower').addEventListener('click',()=>toggleStage(state().selected));
$('stagesList').addEventListener('click',e=>{const toggle=e.target.closest('[data-toggle]');if(toggle){toggleStage(Number(toggle.dataset.toggle));return;}const select=e.target.closest('[data-select]');if(select)selectStage(Number(select.dataset.select));});
$('stagePath').addEventListener('click',e=>{const node=e.target.closest('[data-path]');if(node)selectStage(Number(node.dataset.path));});
$('resetStage').addEventListener('click',()=>{const i=state().selected;for(let layer of ['macro','character','refine'])for(let j=0;j<3;j++)delete state().values[paramKey(i,layer,j)];renderInspector();sendAllState();toast(`Reset ${operations[i].name} controls`);});
$('moveEarlier').addEventListener('click',()=>moveStage(-1));$('moveLater').addEventListener('click',()=>moveStage(1));
$('applyPreset').addEventListener('click',()=>applyPreset($('presetSelect').value));
$('savePreset').addEventListener('click',exportPreset);
$('loadPreset').addEventListener('click',()=>$('presetFile').click());
$('presetFile').addEventListener('change',e=>importPreset(e.target.files?.[0]));
window.addEventListener('change',e=>{
 const ctl=e.target.closest('.param-choice');if(ctl)updateKnob(ctl,Number(ctl.value));
});
window.addEventListener('input',e=>{
 const ctl=e.target.closest('.param-fader');if(ctl)updateKnob(ctl,Number(ctl.value));
});
window.addEventListener('click',e=>{
 const ctl=e.target.closest('.param-switch');if(ctl){const sp=descriptor(ctl).spec;updateKnob(ctl,descriptor(ctl).current()>=50?sp.min:sp.max);}
});
window.addEventListener('pointerdown',e=>{const el=e.target.closest('.knob');if(!el||e.button!==0)return;
 const d=descriptor(el);drag={el,id:e.pointerId,initial:d.current(),x:e.clientX,y:e.clientY,spec:d.spec};el.setPointerCapture?.(e.pointerId);e.preventDefault();});
window.addEventListener('pointermove',e=>{if(!drag||e.pointerId!==drag.id)return;
 const dy=drag.y-e.clientY,dx=e.clientX-drag.x;
 const range=drag.spec.max-drag.spec.min;
 updateKnob(drag.el,drag.initial+(dy+dx*.35)*range/220);
});
window.addEventListener('pointerup',e=>{if(drag&&e.pointerId===drag.id)drag=null;});
window.addEventListener('pointercancel',()=>drag=null);
window.addEventListener('wheel',e=>{const el=e.target.closest('.knob');if(!el)return;e.preventDefault();const d=descriptor(el),step=Math.max(d.spec.step,(d.spec.max-d.spec.min)/200);
 updateKnob(el,d.current()+(e.deltaY<0?1:-1)*step*(e.shiftKey?5:1));},{passive:false});
window.addEventListener('keydown',e=>{const el=e.target.closest?.('.knob');if(!el)return;const d=descriptor(el),n=d.current();let next=n;const step=d.spec.step;
 switch(e.key){case'ArrowUp':case'ArrowRight':next+=step;break;case'ArrowDown':case'ArrowLeft':next-=step;break;case'PageUp':next+=step*10;break;case'PageDown':next-=step*10;break;case'Home':next=d.spec.min;break;case'End':next=d.spec.max;break;default:return;}e.preventDefault();updateKnob(el,next);});
window.addEventListener('dblclick',e=>{const el=e.target.closest('.knob');if(!el)return;updateKnob(el,descriptor(el).spec.def);});
const canvas=$('scope'),ctx=canvas.getContext('2d',{alpha:true});let cw=600,ch=300,dpr=1,time=0,lastTime=0;
function measure(){const b=canvas.getBoundingClientRect();if(!b.width||!b.height)return;dpr=Math.min(window.devicePixelRatio||1,2);cw=b.width;ch=b.height;canvas.width=Math.max(1,Math.floor(cw*dpr));canvas.height=Math.max(1,Math.floor(ch*dpr));ctx.setTransform(dpr,0,0,dpr,0,0);}
function line(x1,y1,x2,y2,color,width=1){ctx.strokeStyle=color;ctx.lineWidth=width;ctx.beginPath();ctx.moveTo(x1,y1);ctx.lineTo(x2,y2);ctx.stroke();}
function drawGrid(){
 ctx.clearRect(0,0,cw,ch);
 const xs=14,ys=8;
 for(let n=0;n<=xs;n++)line(0+(cw/xs)*n,0,(cw/xs)*n,ch,'rgba(139,181,169,.07)');
 for(let n=0;n<=ys;n++)line(0,(ch/ys)*n,cw,(ch/ys)*n,'rgba(139,181,169,.055)');
 line(0,ch*.5,cw,ch*.5,'rgba(162,194,175,.19)');
 for(let i=0;i<7;i++){const x=cw*(i+.5)/7;line(x,0,x,ch,'rgba(205,183,129,.08)');ctx.fillStyle='rgba(190,191,162,.2)';ctx.fillRect(x-1,ch*.5-1,2,2);}
}
function drawAudio(ts){
 const s=state(),base=hues[s.selected],i=s.selected;
 let movement=state().enabled[i]&&!state().bypass?1:.12;
 let complexity=operations[i].audio.macro.map((p,j)=>{
  const v=valueFor(i,'macro',j);return (v-p.min)/(p.max-p.min||1);
 });
 const amt=movement*(.28+.36*complexity[0]);
 const center=ch*.51;
 const width=cw, height=ch;
 for(let k=0;k<17;k++){
  const opacity=(k===8?.85:.06+Math.max(0,.26-Math.abs(k-8)*.025))*movement;
  const yBase=center+(k-8)*5.2;
  const [r,g,b]=base.match(/[a-f\d]{2}/gi).map(v=>parseInt(v,16));
  ctx.strokeStyle=`rgba(${r},${g},${b},${opacity})`;
  ctx.lineWidth=k===8?2.1:.8;ctx.beginPath();
  for(let x=0;x<=width;x+=3){
   const u=x/width;
   const envelope=.27+.73*Math.pow(Math.sin(Math.PI*u),1.5);
   const wave=Math.sin(u*20+(ts*.0004)*(1+i*.12)+k*.35)*.5+Math.sin(u*(51+i*6)-ts*.00026+k*.41)*.27+Math.sin(u*97+ts*.0002+k*.83)*.15;
   const spread=(k-8)*1.2;
   const y=yBase+wave*height*.18*amt*envelope+spread*Math.sin(u*9+ts*.00023)*amt;
   x===0?ctx.moveTo(x,y):ctx.lineTo(x,y);
  }
  ctx.stroke();
 }
 ctx.strokeStyle=`rgba(230,217,173,${movement*.15})`;ctx.beginPath();
 for(let x=0;x<=width;x+=3){const u=x/width;const y=center+Math.sin(u*19+ts*.00055)*height*.045*amt+Math.sin(u*71-ts*.0006)*height*.012*amt;x===0?ctx.moveTo(x,y):ctx.lineTo(x,y);}ctx.stroke();
 const px=(ts*.018)%width;const grad=ctx.createLinearGradient(px-34,0,px+20,0);grad.addColorStop(0,'rgba(220,215,172,0)');grad.addColorStop(.7,'rgba(226,220,179,.08)');grad.addColorStop(1,'rgba(223,224,195,0)');ctx.fillStyle=grad;ctx.fillRect(px-34,0,54,height);
}
function drawMidi(ts){
 const s=state(),base=hues[s.selected];const [r,g,b]=base.match(/[a-f\d]{2}/gi).map(v=>parseInt(v,16));
 const rows=10,cols=24,left=cw*.08,top=ch*.20,gridW=cw*.85,gridH=ch*.62;
 const stepX=gridW/cols,stepY=gridH/rows,seed=s.selected+2;
 for(let n=0;n<=cols;n++)line(left+n*stepX,top,left+n*stepX,top+gridH,n%4===0?'rgba(204,191,157,.16)':'rgba(123,145,140,.06)');
 for(let n=0;n<=rows;n++)line(left,top+n*stepY,left+gridW,top+n*stepY,'rgba(137,158,151,.07)');
 let on=state().enabled[s.selected]&&!state().bypass;
 for(let n=0;n<28;n++){
  const beat=((n*11+seed*7)%cols),pitch=((n*7+seed*3+Math.floor(n/5))%rows);
  const w=(n%3===0?3:1.65)*stepX-3;
  const x=left+beat*stepX+1.5,y=top+(rows-pitch-1)*stepY+4;
  ctx.fillStyle=`rgba(${r},${g},${b},${on?(.26+(n%5)*.11):.10})`;
  ctx.fillRect(x,y,w,Math.max(3,stepY-9));
  if(n%5===0){ctx.fillStyle=`rgba(231,222,189,${on?.7:.14})`;ctx.fillRect(x,y,3,Math.max(3,stepY-9));}
 }
 const x=left+(ts*.03%gridW);ctx.fillStyle=`rgba(215,203,169,${on?.75:.20})`;ctx.fillRect(x,top,1,gridH);
}
function frame(ts){if(ts-lastTime<32){requestAnimationFrame(frame);return;}lastTime=ts;time=ts;drawGrid();if(mode==='audio')drawAudio(isReducedMotion?1000:ts);else drawMidi(isReducedMotion?1000:ts);requestAnimationFrame(frame);}
window.addEventListener('resize',measure);if(typeof ResizeObserver!=='undefined')new ResizeObserver(measure).observe($('scopeWrap'));
render();measure();requestAnimationFrame(frame);
})();

