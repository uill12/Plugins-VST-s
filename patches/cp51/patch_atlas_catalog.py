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
        factoryKey=f"NEW:{p['index']}",source="New",
        idea=p.get("description",p["role"]),trait=p.get("tone",p["role"]))
catalog=json.dumps([item(p) for p in rows],ensure_ascii=False,separators=(",",":"))
script='<script type="application/json" id="albedoNewSoundsCatalog">'+catalog+'</script>'
assert 'id="albedoNewSoundsCatalog"' not in s
pos=s.index('<script type="application/json" id="albedoCreativeCatalog">')
end=s.index('</script>',pos)+len('</script>')
s=s[:end]+'\n'+script+s[end:]
def replace(old,new):
    global s
    if old not in s:raise RuntimeError('Atlas integration anchor missing: '+old[:110])
    s=s.replace(old,new,1)
replace("const creative=Object.freeze(JSON.parse(document.getElementById('albedoCreativeCatalog').textContent));",
        "const creative=Object.freeze(JSON.parse(document.getElementById('albedoCreativeCatalog').textContent));\n"
        "const newSounds=Object.freeze(JSON.parse(document.getElementById('albedoNewSoundsCatalog').textContent));")
replace("const p=creative.find(p=>p.factoryKey===q.value);",
        "const p=[...creative,...newSounds].find(p=>p.factoryKey===q.value);")
replace("+'</optgroup>';q.value=current;",
        "+'</optgroup>'+'<optgroup label=\"MORPHOGENIC FIELDS · 64\">'+[...newSounds].map(p=>'<option value=\"'+escapeText(p.factoryKey)+'\">'+escapeText(label(p))+'</option>').join('')+'</optgroup>';q.value=current;")
old="state.collection==='Creative'?creative:state.collection==='User'?state.user:[];"
replace(old,"state.collection==='Creative'?creative:state.collection==='New'?newSounds:state.collection==='User'?state.user:[];")
replace(old,"state.collection==='Creative'?creative:state.collection==='New'?newSounds:state.collection==='User'?state.user:[];")
replace("get('pbCreativeCount').textContent=creative.length;",
        "get('pbCreativeCount').textContent=creative.length;get('pbNewCount').textContent=newSounds.length;")
replace("state.collection==='Creative'?creative.length:state.collection==='User'?state.user.length:0;",
        "state.collection==='Creative'?creative.length:state.collection==='New'?newSounds.length:state.collection==='User'?state.user.length:0;")
replace("state.collection==='Creative'?'24 new polyphonic voices · Analog reconstruction + rhythmic iteration':",
        "state.collection==='Creative'?'Recursive Forms · 64 sounds':state.collection==='New'?'Morphogenic Fields · Fractal, Kinetic, Subsoil & Prismatic forms':")
replace("p.source==='Creative'?'R'+String(p.id-63).padStart(2,'0'):'U'",
        "p.source==='Creative'?'R'+String(p.id-63).padStart(2,'0'):p.source==='New'?'N'+String(p.id-127).padStart(2,'0'):'U'")
replace("else if(p.source==='Creative'){","else if(p.source==='Creative'||p.source==='New'){")
replace("(source==='Factory'?catalog:source==='Creative'?creative:state.user)",
        "(source==='Factory'?catalog:source==='Creative'?creative:source==='New'?newSounds:state.user)")
replace("get creative(){return creative.map(p=>({...p}));},",
        "get creative(){return creative.map(p=>({...p}));},get newSounds(){return newSounds.map(p=>({...p}));},")
replace('<button type="button" role="tab" id="pbUser" data-collection="User"',
        '<button type="button" role="tab" id="pbNew" data-collection="New" aria-selected="false">MORPHOGENIC FIELDS <span id="pbNewCount">64</span></button>\n'
        '      <button type="button" role="tab" id="pbUser" data-collection="User"')
b=bridge_path.read_text(encoding="utf8")
old="...window.ALBEDO_PRESET_ATLAS.creative].find(p=>p.id===n);"
assert old in b
b=b.replace(old,"...window.ALBEDO_PRESET_ATLAS.creative,...window.ALBEDO_PRESET_ATLAS.newSounds].find(p=>p.id===n);")
html_path.write_text(s,encoding="utf8")
bridge_path.write_text(b,encoding="utf8")
print("Albedo Morphogenic Fields: 64 indexed timbres and four sonic aesthetic families, no generator UI")
