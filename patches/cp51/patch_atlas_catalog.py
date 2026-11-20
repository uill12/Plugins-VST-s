#!/usr/bin/env python3
"""Expose winning sound categories, not the generation tree, in Preset Atlas."""
import json,sys
from pathlib import Path

html_path,bank_path,bridge_path=map(Path,sys.argv[1:4])
s=html_path.read_text(encoding="utf8")
rows=json.loads(bank_path.read_text(encoding="utf8"))
assert len(rows)==64 and [p["index"] for p in rows]==list(range(128,192))
def item(p):
    return dict(id=p["index"],name=p["name"],family=p["family"],role=p["role"],
        character=p["character"],baseIndex=p["baseIndex"],
        factoryKey=f"MORPH:{p['index']}",source="Morph",
        idea=f"{p['role']} · {p['morphTree']['color']} · {p['morphTree']['chosenTransformation']}",
        trait=p["morphTree"]["instrument"])
catalog=json.dumps([item(p) for p in rows],ensure_ascii=False,separators=(",",":"))
script='<script type="application/json" id="albedoMorphCatalog">'+catalog+'</script>'
assert 'id="albedoMorphCatalog"' not in s
pos=s.index('<script type="application/json" id="albedoCreativeCatalog">')
end=s.index('</script>',pos)+len('</script>')
s=s[:end]+'\n'+script+s[end:]
def replace(old,new):
    global s
    if old not in s:raise RuntimeError('Atlas integration anchor missing: '+old[:110])
    s=s.replace(old,new,1)
replace("const creative=Object.freeze(JSON.parse(document.getElementById('albedoCreativeCatalog').textContent));",
        "const creative=Object.freeze(JSON.parse(document.getElementById('albedoCreativeCatalog').textContent));\n"
        "const morph=Object.freeze(JSON.parse(document.getElementById('albedoMorphCatalog').textContent));")
replace("const p=creative.find(p=>p.factoryKey===q.value);",
        "const p=[...creative,...morph].find(p=>p.factoryKey===q.value);")
replace("+'</optgroup>';q.value=current;",
        "+'</optgroup>'+'<optgroup label=\"NEW SOUNDS · 64\">'+[...morph].map(p=>'<option value=\"'+escapeText(p.factoryKey)+'\">'+escapeText(label(p))+'</option>').join('')+'</optgroup>';q.value=current;")
old="state.collection==='Creative'?creative:state.collection==='User'?state.user:[];"
replace(old,"state.collection==='Creative'?creative:state.collection==='Morph'?morph:state.collection==='User'?state.user:[];")
replace(old,"state.collection==='Creative'?creative:state.collection==='Morph'?morph:state.collection==='User'?state.user:[];")
replace("get('pbCreativeCount').textContent=creative.length;",
        "get('pbCreativeCount').textContent=creative.length;get('pbMorphCount').textContent=morph.length;")
replace("state.collection==='Creative'?creative.length:state.collection==='User'?state.user.length:0;",
        "state.collection==='Creative'?creative.length:state.collection==='Morph'?morph.length:state.collection==='User'?state.user.length:0;")
replace("state.collection==='Creative'?'24 new polyphonic voices · Analog reconstruction + rhythmic iteration':",
        "state.collection==='Creative'?'Recursive Forms · 64 sounds':state.collection==='Morph'?'64 timbres · functional sound design, not conceptual duplicates':")
replace("p.source==='Creative'?'R'+String(p.id-63).padStart(2,'0'):'U'",
        "p.source==='Creative'?'R'+String(p.id-63).padStart(2,'0'):p.source==='Morph'?'N'+String(p.id-127).padStart(2,'0'):'U'")
replace("else if(p.source==='Creative'){","else if(p.source==='Creative'||p.source==='Morph'){")
replace("(source==='Factory'?catalog:source==='Creative'?creative:state.user)",
        "(source==='Factory'?catalog:source==='Creative'?creative:source==='Morph'?morph:state.user)")
replace("get creative(){return creative.map(p=>({...p}));},",
        "get creative(){return creative.map(p=>({...p}));},get morph(){return morph.map(p=>({...p}));},")
replace('<button type="button" role="tab" id="pbUser" data-collection="User"',
        '<button type="button" role="tab" id="pbMorph" data-collection="Morph" aria-selected="false">NEW SOUNDS <span id="pbMorphCount">64</span></button>\n'
        '      <button type="button" role="tab" id="pbUser" data-collection="User"')
b=bridge_path.read_text(encoding="utf8")
old="...window.ALBEDO_PRESET_ATLAS.creative].find(p=>p.id===n);"
assert old in b
b=b.replace(old,"...window.ALBEDO_PRESET_ATLAS.creative,...window.ALBEDO_PRESET_ATLAS.morph].find(p=>p.id===n);")
html_path.write_text(s,encoding="utf8")
bridge_path.write_text(b,encoding="utf8")
print("CP51 Atlas: 64 winning timbres in fifth compact tab, without any morph-tree navigation")
