#!/usr/bin/env python3
"""CP27-R3: migrate historical CP18 GUI viewport expectation to 5:3."""
from pathlib import Path
import json,hashlib
r=Path("Albedo-Quintessential-Synth")
p=r/"tests/check_cp18_visual_contract.py"
s=p.read_text(encoding="utf-8")
old="kDefaultEditorWidth=1265, kDefaultEditorHeight=900"
new="kDefaultEditorWidth=1440, kDefaultEditorHeight=864"
assert s.count(old)>=1,"CP18 viewport legacy assertion unexpectedly absent"
p.write_text(s.replace(old,new),encoding="utf-8",newline="\n")
text=(r/"include/vitriol_native_layout.hpp").read_text(encoding="utf-8")
assert new in text and "constexpr double aspect=5.0/3.0;" in text
sha=hashlib.sha256(p.read_bytes()).hexdigest()
manifest=r/"CP19_SOURCE_MANIFEST.json"
j=json.loads(manifest.read_text(encoding="utf-8"))
if "tests/check_cp18_visual_contract.py" in j.get("changed_source_sha256",{}):
 j["changed_source_sha256"]["tests/check_cp18_visual_contract.py"]=sha
j["cp27_legacy_visual_contract"]="CP18 visual test retains original fonts/artwork; expected editor startup migrated to CP27 1440x864."
manifest.write_text(json.dumps(j,ensure_ascii=False,indent=2)+"\n",encoding="utf-8",newline="\n")
print("CP27-R3 legacy CP18 visual contract adapted to 5:3: "+sha)
