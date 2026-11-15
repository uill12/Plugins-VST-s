#!/usr/bin/env python3
"""CP15 Windows resize rounding hotfix, deterministic and fail-closed."""
from pathlib import Path
import hashlib, json
root=Path(__file__).resolve().parents[1]/"Albedo-Quintessential-Synth"
changes={
 "include/vitriol_native_layout.hpp":{
  "before":"e41302fef72df40dcf53d8ed552decbd2e8ed1d2805766b889d872851f4c3615",
  "after":"98ba7cab8516da94ad246bdf8a121516b3be0585819a3a087f43252866fc2644",
  "edits":[
   ("const int minWidth=int(std::ceil(minHeight*aspect));","const int minWidth=int(std::lround(minHeight*aspect));"),
   ("const int maxWidth=int(std::floor(maxHeight*aspect));","const int maxWidth=int(std::lround(maxHeight*aspect));"),
   ("const int hh=std::clamp(requestedHeight,minHeight,maxHeight);",
    "// An initial host size request must not shift the default by one pixel.\n if(requestedWidth==kDefaultEditorWidth&&requestedHeight==kDefaultEditorHeight)\n  return {kDefaultEditorWidth,kDefaultEditorHeight};\n const int hh=std::clamp(requestedHeight,minHeight,maxHeight);")
  ]},
 "tests/native_viewport_parity.cpp":{
  "before":"87bb1cb0860f89e0fc18ed1eb8eab38fa743e4a3c7e00f0d4b3a4bb7baa09304",
  "after":"3e3dda793c3ea119c4969542005ba7ff62861425e4a9d949aad248d51d69aa46",
  "edits":[
   ("if(constrained.first<int(std::ceil(600.0*kArtWidth/kArtHeight)))return 15;",
    "if(constrained.first<int(std::lround(600.0*kArtWidth/kArtHeight)))return 15;"),
   ("static_assert(kDefaultEditorWidth==1280&&kDefaultEditorHeight==910);",
    "static_assert(kDefaultEditorWidth==1280&&kDefaultEditorHeight==910);\n const auto startup=constrainNativeEditorSize(kDefaultEditorWidth,kDefaultEditorHeight);\n if(startup.first!=1280||startup.second!=910)return 18;")
  ]}
}
manifest_file=root/"CP15_SOURCE_MANIFEST.json"
manifest=json.loads(manifest_file.read_text(encoding="utf-8"))
for name,step in changes.items():
 p=root/name
 raw=p.read_bytes()
 actual=hashlib.sha256(raw).hexdigest()
 if actual!=step["before"]:raise SystemExit(f"Refusing unexpected source revision: {name} {actual}")
 s=raw.decode("utf-8")
 for old,new in step["edits"]:
  if s.count(old)!=1:raise SystemExit(f"Unique patch anchor absent in {name}")
  s=s.replace(old,new)
 p.write_text(s,encoding="utf-8",newline="\n")
 after=hashlib.sha256(p.read_bytes()).hexdigest()
 if after!=step["after"]:raise SystemExit(f"Patched source SHA mismatch: {name} {after}")
 manifest["changed_source_sha256"][name]=after
manifest_file.write_text(json.dumps(manifest,indent=2)+"\n",encoding="utf-8",newline="\n")
print("CP15 viewport minimum-width rounding and initial exact 1280x910 validated.")
