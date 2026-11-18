#!/usr/bin/env python3
"""RGNES: validate and normalize JUCE VST3 moduleinfo.json to strict JSON.
Packaging-only; never modifies the compiled PE binary or preset bank.
Usage:
  python standards/tools/rgnes_vst3_moduleinfo_strict_json.py path/to/Name.vst3
"""
from pathlib import Path
import argparse, json, re

def normalize(bundle: Path) -> None:
    target=bundle/"Contents/Resources/moduleinfo.json"
    if not target.exists():
        raise SystemExit(f"VST3 metadata not found: {target}")
    raw=target.read_text(encoding="utf-8-sig")
    # JUCE 8's emitted moduleinfo.json uses trailing commas. Strip only those
    # that immediately precede structural closers, outside quoted strings.
    # Full state machine avoids corrupting metadata string contents.
    pieces=[]
    quoted=False
    escaped=False
    for ch in raw:
        if quoted:
            pieces.append(ch)
            if escaped: escaped=False
            elif ch=="\\": escaped=True
            elif ch=='"': quoted=False
        elif ch=='"':
            quoted=True
            pieces.append(ch)
        elif ch in "]}":
            # Move backwards past whitespace to remove the last trailing comma.
            cursor=len(pieces)-1
            while cursor>=0 and pieces[cursor].isspace():cursor-=1
            if cursor>=0 and pieces[cursor]==",":pieces.pop(cursor)
            pieces.append(ch)
        else:pieces.append(ch)
    fixed="".join(pieces)
    info=json.loads(fixed)
    assert info.get("Name") and info.get("Version")
    assert len(info.get("Classes",[]))>=1
    target.write_text(fixed,encoding="utf-8")
    print(f"Strict moduleinfo.json OK: {target} ({info['Version']})")

if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("bundle",type=Path)
    normalize(parser.parse_args().bundle)
