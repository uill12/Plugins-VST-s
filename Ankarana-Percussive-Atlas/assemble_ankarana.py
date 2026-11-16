#!/usr/bin/env python3
"""Reassemble Ankarana CP08R1/CP09 split ZIP payloads from Google Drive.

Requires Python 3.9+. Put all downloaded numbered ZIP parts for ONE format
beside this script, then run:
    python assemble_ankarana.py clap
    python assemble_ankarana.py vst3

This is a convenience reconstruction helper, not a signed installer.
Inspect the produced file before installing it.
"""
from __future__ import annotations
import hashlib
import pathlib
import shutil
import sys
import zipfile

ROOT = pathlib.Path(__file__).resolve().parent
FORMATS = {
    "clap": [f"Ankarana_CLAP_v0.8.1_Part_{i:02d}.zip" for i in range(1, 6)],
    "vst3": [f"Ankarana_VST3_CP09_Part_{i:02d}.zip" for i in range(1, 7)],
}
PREFIX = {"clap": "Ankarana_v0.8.1_CP08R1", "vst3": "Ankarana_v0.8.1_CP09"}

def select_payload(archive: zipfile.ZipFile) -> zipfile.ZipInfo:
    files = [x for x in archive.infolist() if not x.is_dir()
             and not x.filename.startswith("__MACOSX/")]
    substantial = [x for x in files if x.file_size > 1024 * 1024]
    if len(substantial) == 1:
        return substantial[0]
    if not substantial:
        raise RuntimeError("Part ZIP has no substantial binary payload: "
                           + ", ".join(x.filename for x in files))
    strongest = sorted(substantial, key=lambda x: x.file_size, reverse=True)
    if len(strongest) > 1 and strongest[1].file_size > strongest[0].file_size * .8:
        raise RuntimeError("Ambiguous multiple large payloads in part ZIP: "
                           + ", ".join(x.filename for x in substantial))
    return strongest[0]

def assemble(fmt: str) -> None:
    missing = [p for p in FORMATS[fmt] if not (ROOT / p).is_file()]
    if missing:
        raise FileNotFoundError("Missing all required parts: " + ", ".join(missing))
    temp = ROOT / (PREFIX[fmt] + ".assembling")
    sha = hashlib.sha256()
    size = 0
    try:
        with temp.open("wb") as dst:
            for i, fname in enumerate(FORMATS[fmt], 1):
                with zipfile.ZipFile(ROOT / fname) as z:
                    bad = z.testzip()
                    if bad:
                        raise RuntimeError(f"Corrupt member in {fname}: {bad}")
                    member = select_payload(z)
                    print(f"Part {i}/{len(FORMATS[fmt])}: {fname} -> {member.filename} "
                          f"({member.file_size:,} bytes)")
                    with z.open(member) as src:
                        while True:
                            chunk = src.read(1024 * 1024)
                            if not chunk:
                                break
                            dst.write(chunk)
                            sha.update(chunk)
                            size += len(chunk)
        with temp.open("rb") as f:
            header = f.read(4)
        if header.startswith(b"PK\x03\x04"):
            extension = ".zip"
        elif header.startswith(b"MZ"):
            extension = ".clap" if fmt == "clap" else ".vst3"
        else:
            raise RuntimeError(f"Unrecognized assembled file header: {header!r}. "
                               "Do not install; use the original checkpoint assembler.")
        target = ROOT / (PREFIX[fmt] + extension)
        temp.replace(target)
        if extension == ".zip":
            with zipfile.ZipFile(target) as z:
                if z.testzip():
                    raise RuntimeError("Reconstructed ZIP failed CRC validation")
        print(f"OUTPUT: {target.name} ({size:,} bytes)")
        print("SHA-256:", sha.hexdigest())
        print("Next: verify against the release's original SHA-256 if available, "
              "then install manually and test in Bitwig.")
    finally:
        if temp.exists():
            temp.unlink()

if __name__ == "__main__":
    if len(sys.argv) != 2 or sys.argv[1].lower() not in FORMATS:
        sys.exit("Usage: python assemble_ankarana.py clap|vst3")
    assemble(sys.argv[1].lower())
