#!/usr/bin/env python3
"""Verify portable bundle file integrity. Python 3.10+, no dependencies."""
from pathlib import Path
import hashlib,json,sys
root=Path(__file__).resolve().parent
manifest=root/'SHA256SUMS.txt'
assert manifest.exists(),'No SHA256SUMS.txt in archive root'
count=0
for line in manifest.read_text(encoding='utf-8').splitlines():
    if not line.strip():continue
    expected,relative=line.split('  ',1)
    p=(root/relative).resolve()
    if not p.is_relative_to(root.resolve()) or not p.is_file():
        raise RuntimeError('Missing or unsafe path: '+relative)
    observed=hashlib.sha256(p.read_bytes()).hexdigest()
    if observed!=expected:raise RuntimeError('SHA-256 mismatch: '+relative)
    count+=1
manifestj=json.loads((root/'DEPENDENCIES_LOCK.json').read_text(encoding='utf-8'))
assert manifestj['cp58_release_commit']=='3481912073e23cef753521e3a64cceafb19257ac'
assert (root/'ReconstructedDSP/src/vitriol_dsp.cpp').is_file()
assert (root/'Binaries/CP58/Albedo_CP58_ASIO_Optimized_WebView2.clap').is_file()
print(f'PASS: {count} bundled files SHA-256 verified, CP58 source and binaries available')
