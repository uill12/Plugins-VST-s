#!/usr/bin/env python3
"""CP28: normalize and authenticate native/HTML/DSP source, then update source manifest."""
from pathlib import Path
import hashlib
import json
root=Path("Albedo-Quintessential-Synth")
expected={
  "CMakeLists.txt": "273611665650e1cb062648a1b5824af28bc147fbba2b88286124ab8a963e5432",
  "include/albedo_webview_win.hpp": "622947d967bb8d2949dfa985e6bb174b32c4ec386694b39e5ca583e0a5624125",
  "include/editor_native_win.hpp": "58696c516514adb72944840c735a473ec2bc6ef949a4ae38e594ae5ee5fd4c11",
  "include/vitriol_dsp.hpp": "c7cf93d5e763b2b460aebc08b76951a261f0c68f976371e4a30cd6da60f1709c",
  "resources/Albedo_CP27_5x3_embedded.html": "7bbd88c98d2fe00ebdc4461d8a783e51d7030172317b9c4384cc9ea8114447c1",
  "src/albedo_webview_win.cpp": "1418149cfb3b837a4908887f4cb6214fff3d753b759c784815f4e91c1e888eba",
  "src/editor_native_win.cpp": "9ba2c63ad04f4f397bdcf133d8d1f75c463e689bdaecf170284dc0bab8a226e6",
  "src/vitriol_dsp.cpp": "dbdeb44fdc28c41f2a9d55b5865a1df7a53cc47e8ad6297d36921d1e9f3e17a5",
  "tests/cp28_poly48_factory.cpp": "953a35a438173e9af7200880d016ac5ecbd77c705beeb8c919385f43d04d9e50",
  "tests/cp28_recovery_polyphony_contract.py": "62a986b3431507ff240a44ac93f399a9592bf689a477d87e938ea169ff8b794f"
}
for name in expected:
    path=root/name
    path.write_bytes(path.read_bytes().replace(b"\r\n",b"\n"))
actual={name:hashlib.sha256((root/name).read_bytes()).hexdigest() for name in expected}
mismatch={name:{"actual":actual[name],"expected":sha} for name,sha in expected.items() if actual[name]!=sha}
if mismatch:
    raise SystemExit("CP28 source SHA mismatch: "+json.dumps(mismatch,indent=2))
manifest=root/"CP19_SOURCE_MANIFEST.json"
data=json.loads(manifest.read_text(encoding="utf-8"))
data["changed_source_sha256"].update(expected)
data["cp28_recovery_polyphony"]={
    "audio":"Independent 12-note MIDI keys, 48kHz AUTO x2, cached source filters",
    "renderer":"WebView2 process/navigation failures => Win32 GDI fallback",
    "factory":"64 patches, 12-key polyphony QA",
    "host_certification":"Pending actual Bitwig verification"
}
manifest.write_text(json.dumps(data,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print("CP28 exact GUI/DSP/preset/test file hashes verified; manifest updated")
