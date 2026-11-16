#!/usr/bin/env python3
"""CP20: migrate only native-window-size QA expectations after audited UI change.

The base CP18/CP19 DSP, sample, factory-preset, and original artwork hashes
remain untouched and continue to be checked by verify_source_assets.py.
"""
from pathlib import Path
import hashlib
import json

root = Path("Albedo-Quintessential-Synth")
assert root.is_dir()
target = root / "include/vitriol_native_layout.hpp"
layout = target.read_text(encoding="utf-8")
assert "kDefaultEditorWidth=1265, kDefaultEditorHeight=900" in layout

def replace_exact(path, before, after):
    body = path.read_text(encoding="utf-8")
    if body.count(before) != 1:
        raise AssertionError(f"CP20 contract anchor mismatch: {path}, {before!r}")
    path.write_text(body.replace(before, after), encoding="utf-8", newline="\n")
    return hashlib.sha256(path.read_bytes()).hexdigest()

cpp = "tests/native_viewport_parity.cpp"
fixes = [
    (root / cpp,
     "if(startup.first!=900||startup.second!=640)return 18;",
     "if(startup.first!=1265||startup.second!=900)return 18;"),
    (root / cpp,
     "if(kDefaultEditorWidth>1024||kDefaultEditorHeight>728)return 17;",
     "if(kDefaultEditorWidth>1600||kDefaultEditorHeight>950)return 17;"),
]
for path, before, after in fixes:
    replace_exact(path, before, after)

changed = [cpp]
for name in ("check_cp15_resize_contract.py", "check_cp18_visual_contract.py"):
    relative = "tests/" + name
    replace_exact(root / relative,
        "kDefaultEditorWidth=900, kDefaultEditorHeight=640",
        "kDefaultEditorWidth=1265, kDefaultEditorHeight=900")
    changed.append(relative)

# Source contracts may originate in earlier checkpoint manifests. Update only
# the three explicitly migrated QA files, never the DSP, artwork, or preset keys.
updated = []
for path in sorted(root.glob("CP*_SOURCE_MANIFEST.json")):
    manifest = json.loads(path.read_text(encoding="utf-8"))
    signatures = manifest.get("changed_source_sha256")
    if not isinstance(signatures, dict):
        continue
    fields = []
    for relative in changed:
        if relative in signatures:
            signatures[relative] = hashlib.sha256((root / relative).read_bytes()).hexdigest()
            fields.append(relative)
    if fields:
        path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n",
                        encoding="utf-8", newline="\n")
        updated.append((path.name, fields))

editor = (root / "src/editor_native_win.cpp").read_text(encoding="utf-8")
assert "TextCaptureScope capture(liveText)" in editor
assert "StretchBlt(dc,ox,oy,sx,sy,backing" in editor
assert "text(dc,px,py,pw,textHeight,cmd.t" in editor
print("CP20 FHD window regression migrated:", changed)
print("Exact historical QA checksum records updated:", updated)
