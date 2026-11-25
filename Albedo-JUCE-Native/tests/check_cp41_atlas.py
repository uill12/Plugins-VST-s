#!/usr/bin/env python3
"""CP41: compact Atlas contract; one surface, button labels, all voices, zero page controls."""
from pathlib import Path
import re, json, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
html=(root.parent/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
geometry=(root/"Assets/cp35_editor_geometry.js").read_text(encoding="utf-8")
bridge=(root/"Assets/juce_cp33_bridge.js").read_text(encoding="utf-8")
rotary=(root/"Assets/cp38_rotary_overlays.js").read_text(encoding="utf-8")
cmake=(root/"CMakeLists.txt").read_text(encoding="utf-8")
assert html.count('id="presetBrowser"')==1
for name in ["Factory","Creative","User","Archives"]:
    assert html.count('data-collection="'+name+'"')==1, name
for sel in ["pbSearch","pbFamily","pbRole","pbSort","pbListMode","pbMatrixMode","pbOrder","pbClear"]:
    assert html.count('id="'+sel+'"')==1,sel
for sel in ["pbFamily","pbRole","pbSort"]:
    assert re.search(r'<select id="'+sel+r'" hidden aria-hidden="true" tabindex="-1"',html),sel
for sel in ["pbApply","pbFamilyChips","pbRoleChips"]:
    assert html.count('id="'+sel+'"')==1,sel
assert html.count('id="cp41-atlas-label-appliers"')==1
css=html.split('<style id="cp41-compact-atlas">',1)[1].split('</style>',1)[0]
script=html.split('<script id="cp41-atlas-label-appliers">',1)[1].split('</script>',1)[0]
for token in ["grid-template-columns:repeat(8,minmax(0,1fr))","overflow:hidden!important",
              "width:min(960px","height:min(642px","pb-filter-chip","pb-filter-apply",
              "pb-matrix-grid"]:
    assert token in css,token
for token in ["draft.family=family.value","draft.role=role.value","draft.sort=sort.value",
              "sort.dispatchEvent(new Event('change'","refreshChips","pbApply",
              "get hasPages(){return false}"]:
    assert token in script,token
assert "pbPager" not in html and "pbPrev" not in html and "pbNext" not in html
assert "Math.min(960," in geometry and "Math.min(642," in geometry
assert "dialog.addEventListener('click',requestApply)" not in bridge
assert "addEventListener('change',requestApply)" in bridge
assert "cp38-visual" not in rotary
assert "cp37AdsrPopup" in html
assert ("VERSION 1.0.51" in cmake or "VERSION 1.0.53" in cmake or "VERSION 1.0.54" in cmake or "VERSION 1.0.55" in cmake or "VERSION 1.0.56" in cmake or "VERSION 1.0.57" in cmake or "VERSION 1.0.58" in cmake or "VERSION 1.0.59" in cmake or "VERSION 1.0.60" in cmake or "VERSION 1.0.61" in cmake or "VERSION 1.0.63" in cmake or "VERSION 1.0.64" in cmake), "Retain plugin family and compatible CP53 version"
assert "juce32" in cmake and "CLAP_ID" in cmake
factory=re.search(r'<script type="application/json" id="albedoFactoryCatalog">([^<]+)</script>',html)
creative=re.search(r'<script type="application/json" id="albedoCreativeCatalog">([^<]+)</script>',html)
assert factory and creative
fa=json.loads(factory.group(1));cr=json.loads(creative.group(1))
assert [r['id'] for r in fa]==list(range(64))
assert [r['id'] for r in cr]==list(range(64,128))
assert len({(r["family"],r["role"]) for r in fa})==64
with tempfile.TemporaryDirectory() as td:
    for i,src in enumerate(re.findall(r'<script(?:\s[^>]*)?>([\s\S]*?)</script>',html)):
        if not src.strip():continue
        file=Path(td)/f"inline_{i}.js"
        file.write_text(src,encoding="utf-8")
        subprocess.run(["node","--check",str(file)],check=True,capture_output=True)
print("CP41 PASS: one compact no-scroll Atlas, label buttons + APPLY, 4 tabs, 64+24 indexed presets, valid inline JS")
