#!/usr/bin/env python3
"""CP28-R1: preserve CP17 high-rate bounded-voice contract; keep 12-key 48k polyphony."""
from pathlib import Path
import hashlib,json
r=Path("Albedo-Quintessential-Synth")
p=r/"src/vitriol_dsp.cpp"
src=p.read_text(encoding="utf-8")
old="return 12; // CP28: stable 12 independent keys at 48 kHz and higher, bounded x2 AUTO CPU cost."
new="return fs_>=176400.?8:12; // CP28: 12 independent keys through 96 kHz, legacy 8-voice cap at 176.4/192 kHz."
assert src.count(old)==1,"CP28 voiceBudget anchor no longer uniquely matches"
p.write_text(src.replace(old,new),encoding="utf-8",newline="\n")
t=r/"tests/cp28_recovery_polyphony_contract.py"
txt=t.read_text(encoding="utf-8")
before="assert 'return 12; // CP28' in dsp"
after="assert 'return fs_>=176400.?8:12; // CP28' in dsp"
assert txt.count(before)==1,"CP28 polyphony contract anchor changed"
t.write_text(txt.replace(before,after),encoding="utf-8",newline="\n")
expected={
 "src/vitriol_dsp.cpp":"914d77017336c9ff6f3ee05a18af0cc114b46d53023d369e2407aff44be15f1f",
 "tests/cp28_recovery_polyphony_contract.py":"d14324705b2287a1e6a65ac90d7f045069367ab34fb15a8e2cff638408815e9b"
}
actual={f:hashlib.sha256((r/f).read_bytes()).hexdigest() for f in expected}
assert expected==actual,("CP28-R1 source SHA mismatch",actual)
manifest=r/"CP19_SOURCE_MANIFEST.json"
doc=json.loads(manifest.read_text(encoding="utf-8"))
doc["changed_source_sha256"].update(expected)
doc["cp28_highrate_safety"]="12 keyboard notes at 44.1/48/88.2/96 kHz, eight voices at >=176.4 kHz; preserves CP17 note-storm bounds."
manifest.write_text(json.dumps(doc,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print("CP28-R1 48k 12-note polyphony and legacy high-rate note-storm cap verified")
