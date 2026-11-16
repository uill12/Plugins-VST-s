from pathlib import Path
R=Path(__file__).resolve().parents[1]
html=R/'ui/index.html'
s=html.read_text(encoding='utf-8')
s=s.replace('CP07 Alchemical GUI','CP09 RGNES Horizontal GUI').replace('class="build-version">CP07','class="build-version">CP09').replace('GUI CP07 · CLAP / VST3 ENGINE','GUI CP09 · RGNES NATIVE EDITOR')
old='<button id="powerBtn" type="button" class="main-power" aria-label="Toggle complete effects chain" aria-pressed="true" title="Toggle complete effects chain"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="M12 2v11"/><path d="M6.3 5.5a9 9 0 1 0 11.4 0"/></svg></button>'
new='<button id="powerBtn" type="button" class="main-power" aria-label="Turn effects chain off" aria-pressed="true" title="Toggle all seven effects stages"><span class="power-caption">FX CHAIN</span><span class="power-status" aria-live="polite">ON</span><span class="power-track" aria-hidden="true"><i class="power-thumb"></i></span></button>'
assert old in s
s=s.replace(old,new)
s=s.replace('THE OPERATIONS</span><span>ORDO · VII','PROCESSING CHAIN · VII OPERATIONS</span><span id="chainStatusLabel">7 PROCESSORS · ACTIVE').replace('Select, enable, and reorder the processes.','Audio and MIDI processors · select, switch or reorder each effect.')
html.write_text(s,encoding='utf-8')
js=R/'ui/app.js';j=js.read_text(encoding='utf-8')
anchor="const scopes = ['HARMONIC HEAT'"
assert anchor in j
segment="""const operationFunctions = {
 audio:['SATURATION / DRIVE','STEREO DELAY / DIFFUSION','MULTIBAND / TRANSIENT','HARMONIC BLEND','MODULATION / FEEDBACK','SPECTRAL EQ','COMPRESSION / LIMITER'],
 midi:['VELOCITY SHAPER','TIMING / LEGATO','NOTE SPLIT / ROUTING','CHORD HARMONIZER','PROBABILITY / MUTATION','SCALE / NOTE FILTER','QUANTIZE / GATE']
};
"""
j=j.replace(anchor,segment+anchor)
old="$('powerBtn').setAttribute('aria-pressed',String(!state().bypass));$('powerBtn').classList.toggle('off',state().bypass);$('cosmogram').classList.toggle('unlit',state().bypass||!state().enabled[state().selected]);"
new="$('powerBtn').setAttribute('aria-pressed',String(!state().bypass));$('powerBtn').classList.toggle('off',state().bypass);$('powerBtn').querySelector('.power-status').textContent=state().bypass?'OFF':'ON';$('powerBtn').setAttribute('aria-label',state().bypass?'Turn effects chain on':'Turn effects chain off');$('cosmogram').classList.toggle('unlit',state().bypass||!state().enabled[state().selected]);$('chainStatusLabel').textContent=state().bypass?'CHAIN BYPASSED':`${state().enabled.filter(Boolean).length} PROCESSORS · ACTIVE`;"
assert old in j
j=j.replace(old,new)
old='''const op=operations[i],selected=s.selected===i,enabled=s.enabled[i];
  return `<div class="stage-item ${selected?'selected':''} ${!enabled?'muted':''}" style="--stage:${hues[i]}" data-stage="${i}">
     <button type="button" class="stage-select" data-select="${i}" aria-pressed="${selected}" aria-label="Select ${op.name}">
      <span class="number">${roman[i]}</span><span class="stage-glyph">${glyphs[i]}</span>
      <span class="stage-copy"><b>${op.name}</b><small>${op.latin}</small></span></button>
     <button type="button" class="stage-toggle ${enabled?'':'off'}" data-toggle="${i}" aria-label="${enabled?'Disable':'Enable'} ${op.name}" title="${enabled?'Disable':'Enable'} ${op.name}" aria-pressed="${enabled}"></button>
    </div>`;'''
new='''const op=operations[i],selected=s.selected===i,enabled=s.enabled[i];
  const functionName=operationFunctions[mode][i];
  return `<div class="stage-item ${selected?'selected':''} ${!enabled?'muted':''}" style="--stage:${hues[i]}" data-stage="${i}">
     <button type="button" class="stage-select" data-select="${i}" aria-pressed="${selected}" aria-label="Edit ${functionName}, ${op.name}">
      <span class="number">${roman[i]}</span><span class="stage-glyph">${glyphs[i]}</span>
      <span class="stage-copy"><b>${functionName}</b><small>${op.name.toUpperCase()} · ${op.latin}</small></span></button>
     <button type="button" class="stage-toggle ${enabled?'':'off'}" data-toggle="${i}" aria-label="${enabled?'Disable':'Enable'} ${functionName}" title="${enabled?'Disable':'Enable'} ${functionName}" aria-pressed="${enabled}"><span class="toggle-text">${enabled?'ON':'OFF'}</span></button>
    </div>`;'''
assert old in j
j=j.replace(old,new)
j=j.replace("if(state().selected===i)renderInspector();}","if(state().selected===i)renderInspector();renderModes();}")
# update any other render() of disabled operation: use global chain state only in on/off
j=j.replace("toast(state().bypass?'Chain bypassed · visual preview':'Chain engaged · visual preview');","toast(state().bypass?'FX chain bypassed':'FX chain engaged');")
js.write_text(j,encoding='utf-8')
css=R/'ui/styles.css'
c=css.read_text(encoding='utf-8')
add='''
/* CP09 · RGNES design lock. Horizontal signal chain; semantic processor titles. */
.main-power{display:flex!important;align-items:center;justify-content:space-around;gap:7px;width:160px!important;height:36px!important;min-width:160px;border-radius:22px!important;padding:4px 9px!important;cursor:pointer;font-family:Georgia,serif!important;background:linear-gradient(180deg,#658a5c,#36583f)!important;border:1px solid #49684b!important;box-shadow:inset 0 1px 2px #faedc875,0 1px 5px #8d5b3d44!important;color:#fff5d1!important}
.main-power .power-caption{font-size:10px;font-weight:bold;letter-spacing:.095em;white-space:nowrap;}
.main-power .power-status{font-weight:700;letter-spacing:.08em;font-size:11px;min-width:24px;text-align:center;}
.main-power .power-track{width:29px;height:15px;border:1px solid #eddeb2;border-radius:12px;background:#284732;display:flex;align-items:center;padding:1px;flex-shrink:0;}
.main-power .power-thumb{display:block;width:11px;height:11px;border-radius:50%;background:radial-gradient(circle at 35% 30%,#fff8d5,#e5bb67);box-shadow:0 0 3px #ffd3a0;transform:translateX(13px);transition:transform .14s ease;}
.main-power.off{background:linear-gradient(180deg,#9b8271,#6d5f55)!important;border-color:#977b69!important;color:#fff1df!important;}
.main-power.off .power-thumb{transform:translateX(0)}
.main-power:focus-visible,.stage-toggle:focus-visible,.stage-select:focus-visible{outline:2px solid #714d84;outline-offset:2px}
.stage-toggle{color:#4a6d4c}
.stage-toggle .toggle-text{font:bold 9px Georgia,serif;letter-spacing:.04em;display:block;position:relative;z-index:4;}
.stage-toggle.off .toggle-text{color:#a3564c}
.stage-select .stage-copy b{font-family:Georgia,serif;letter-spacing:.04em}
.stage-select .stage-copy small{letter-spacing:.045em}
@media(min-width:1121px){
 .topbar{min-height:58px!important;gap:12px!important}.header-tools{gap:7px!important}
 .intro{min-height:44px!important;margin:5px 0 6px!important}.trinity{margin-bottom:6px!important}
 .workbench{display:grid!important;grid-template-columns:minmax(0,1.14fr) minmax(0,.86fr)!important;grid-template-rows:auto minmax(0,1fr);column-gap:10px!important;row-gap:8px!important;align-items:stretch}
 .stages-panel{grid-column:1 / 3;grid-row:1;min-height:0!important;overflow:hidden;}
 .stages-panel .panel-heading{min-height:28px!important;padding:5px 12px!important}
 .ledger-caption{padding:2px 10px 3px!important;text-align:left!important;font-size:10px!important;font-style:italic;color:#86684b;line-height:1.2!important;}
 .stages-list{display:grid!important;grid-template-columns:repeat(7,minmax(0,1fr))!important;gap:6px!important;padding:4px 9px 7px!important;}
 .stage-item{height:98px!important;min-height:98px!important;display:flex!important;align-items:center!important;flex-direction:row!important;border:1px solid #bc8f68!important;border-top:3px solid var(--stage)!important;background:linear-gradient(180deg,#f7e8c5,#ead4ab)!important;position:relative!important;box-shadow:inset 0 0 0 1px #fdf1d4,0 1px 2px #8e624633!important;border-radius:4px!important;}
 .stage-item.selected{background:linear-gradient(180deg,#fff0ce,#e5c997)!important;border-color:#a46a42!important;box-shadow:inset 0 0 0 1px #fff0c7,0 0 0 2px #c79b59!important;}
 .stage-item.muted{opacity:.63}
 .stage-select{display:grid!important;grid-template-columns:22px 1fr!important;grid-template-rows:25px 1fr!important;width:100%;height:100%;justify-items:center;align-items:center!important;gap:0!important;text-align:center!important;padding:4px 8px 9px!important;}
 .stage-item .number{grid-column:1;grid-row:1;font-size:17px!important;line-height:1;color:var(--stage);align-self:start;justify-self:start;}
 .stage-item .stage-glyph{grid-column:2;grid-row:1;height:27px!important;width:27px!important;align-self:start;justify-self:center;}
 .stage-item .stage-copy{grid-column:1/3;grid-row:2;width:100%;min-width:0;text-align:center;display:flex;flex-direction:column;justify-content:flex-start;gap:4px;padding-top:4px;}
 .stage-item .stage-copy b{font-size:12px!important;line-height:1.03!important;min-height:24px;display:flex;align-items:center;justify-content:center;word-break:normal;overflow-wrap:normal;}
 .stage-item .stage-copy small{font-size:8px!important;line-height:1.1!important;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;display:block;width:100%;}
 .stage-toggle{position:absolute!important;right:7px!important;top:6px!important;width:40px!important;height:21px!important;min-width:40px;margin:0!important;border:1px solid #9f7949!important;border-radius:10px!important;background:#b5d0a4!important;display:flex!important;align-items:center;justify-content:center;padding:0!important;cursor:pointer;}
 .stage-toggle.off{background:#dfb3a0!important}
 .stage-toggle:before,.stage-toggle:after{display:none!important}
 .stages-footer{display:none!important}
 .field-panel{grid-column:1;grid-row:2;min-height:0!important}
 .editor-panel{grid-column:2;grid-row:2;min-height:0!important}
 .field-top{padding:5px 14px 0!important}
 .cosmogram-shell{padding:1px 8px 0!important}
 .cosmogram{width:clamp(275px,22vw,355px)!important}
 .path-node{width:44px!important;height:44px!important}
 .wheel-caption{margin:-2px 0 2px!important}
 .scope-block{min-height:72px!important;margin-bottom:6px!important}.scope-wrap{height:72px!important;min-height:72px!important}
 .bottom-altar{margin-top:6px!important;gap:8px!important}
 .altar-panel{padding:5px 11px!important}
 .site-footer{margin-top:3px!important}
}
@media(min-width:761px) and (max-width:1120px){
 .stages-list{display:grid!important;grid-template-columns:repeat(7,minmax(0,1fr))!important;gap:3px!important;}
 .stage-item .stage-copy b{font-size:10px;line-height:1.1}.stage-item .stage-copy small{font-size:7px}
}
@media(max-width:760px){
 .main-power{min-width:135px;width:135px!important;}
 .stages-list{grid-template-columns:repeat(2,minmax(0,1fr))!important}
 .stage-item{min-height:85px!important}
 .stage-copy b{font-size:11px!important}
}
'''
c+="\n"+add
css.write_text(c,encoding='utf-8')
print('CP09 HTML, JS, CSS updated')
