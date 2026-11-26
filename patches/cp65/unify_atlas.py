#!/usr/bin/env python3
"""CP65 runtime catalog: ONE visible 192-sound Albedo bank, stable native IDs.

Run after the standard CP51 patch_atlas_catalog.py so the native Factory /
Creative / New dispatch keys remain backwards-compatible with project files.
Families and roles remain filters; TNSP is not exposed as a GUI mode.
"""
from pathlib import Path
import re,sys
if len(sys.argv)!=2:raise SystemExit("usage: unify_atlas.py ui/Albedo*.html")
file=Path(sys.argv[1]);s=file.read_text(encoding="utf-8")
def change(before,after):
 global s
 n=s.count(before)
 if n!=1:raise RuntimeError("CP65 unexpected generated Atlas anchor ("+str(n)+"): "+before[:95])
 s=s.replace(before,after,1)

change(
 "const catalog=Object.freeze(JSON.parse(document.getElementById('albedoFactoryCatalog').textContent));",
 "const factory=Object.freeze(JSON.parse(document.getElementById('albedoFactoryCatalog').textContent));")
needle="const newSounds=Object.freeze(JSON.parse(document.getElementById('albedoNewSoundsCatalog').textContent));"
change(needle,needle+"\nconst catalog=Object.freeze([...factory,...creative,...newSounds]);")
change("const nativeValid=catalog.every((p,i)=>p.id===i&&p.factoryKey===api.presets?.[i]?.name);",
       "const nativeValid=factory.every((p,i)=>p.id===i&&p.factoryKey===api.presets?.[i]?.name);")
# The quick selector must list 192 names ONCE, not one 192 set plus
# duplicate 64+64 optgroups. Native preset index dispatch is still canonical.
start=s.index("if(q){const current=q.value;q.innerHTML=")
end=s.index("q.addEventListener('change'",start)
replacement="""if(q){const current=q.value;
q.innerHTML='<option value="">UNSAVED / MODIFIED</option>'+
 '<optgroup label="ALBEDO · BIOLUMINAL ATLAS · 192">'+
 [...catalog].sort((a,b)=>collator.compare(a.name,b.name)).map(p=>'<option value="'+escapeText(p.factoryKey)+'">'+escapeText(label(p))+'</option>').join('')+'</optgroup>';
q.value=current;
"""
segment=s[start:end]
if segment.count("q.value=current;")!=1:raise RuntimeError("quick-selection version changed")
s=s[:start]+replacement+s[end:]
# The catalog's 192 entries are immutable and contain source dispatch keys
# for each original index. We unify the visible bank, not host preset IDs.
change("get('pbSummary').textContent=state.collection==='Factory'?'Original 64 · Name × Family × Role'",
       "get('pbSummary').textContent=state.collection==='Factory'?'192 tones · Name × Family × Instrument Role'")
change('data-collection="Factory" aria-selected="true" class="on">FACTORY <span id="pbFactoryCount">64</span>',
       'data-collection="Factory" aria-selected="true" class="on">BIOLUMINAL ATLAS <span id="pbFactoryCount">192</span>')
change('data-collection="Creative" aria-selected="false">RECURSIVE FORMS',
       'data-collection="Creative" aria-selected="false" hidden aria-hidden="true" tabindex="-1">RECURSIVE FORMS')
change('data-collection="New" aria-selected="false">MORPHOGENIC FIELDS',
       'data-collection="New" aria-selected="false" hidden aria-hidden="true" tabindex="-1">MORPHOGENIC FIELDS')
change("const p=[...creative,...newSounds].find(p=>p.factoryKey===q.value);",
       "const p=catalog.find(p=>p.factoryKey===q.value);")
# After combining the full bank, source-specific click handlers retain their
# semantics. No 'morph tree' UI filter and no renumbering.
css="""
<style id="cp65-unified-bank-style">
#presetBrowser #pbCreative,
#presetBrowser #pbNew{display:none!important}
#presetBrowser #pbFactory{
 color:#395e53;border-color:#a29a76;
 background:linear-gradient(130deg,#e7f0e0,#f7edda 58%,#ead5bb);
 letter-spacing:.07em;white-space:nowrap
}
#presetBrowser .preset-browser-provenance{font-size:9px}
</style>
"""
change("</head>",css+"</head>")
if s.count("const catalog=Object.freeze([...factory,...creative,...newSounds]);")!=1:
 raise RuntimeError("unified catalog missing")
if "state.collection==='Factory'?catalog" not in s:raise RuntimeError("factory traversal changed")
file.write_text(s,encoding="utf-8",newline="\n")
print("CP65 PASS: one visible Bioluminal Atlas bank, 192 stable native programs, original family/role filters")
