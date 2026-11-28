#!/usr/bin/env python3
"""CP66 experimental artifact metadata, run after strict native binary QA.

Keep the original CP65R1 release untouched. This changes only provenance JSON
for newly built CP66 artifacts and NEVER declares audition or DAW tests passed.
"""
import argparse,json
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument("directory",type=Path)
args=p.parse_args()
root=args.directory
legacy=root/"CP65R1_PACKAGE_QA.json"
assert legacy.exists(),"CP61 strict binary metadata gate must run first"
data=json.loads(legacy.read_text(encoding="utf-8"))
assert data["version"]=="1.0.66",data["version"]
data.update({
    "checkpoint":"Albedo CP66 — experimental root-first quality-candidate synthesis",
    "gui_checkpoint":"CP66 / existing stable interactive layout and synchronized material ranges",
    "build_date":"2026-11-28",
    "sources":"192 preset IDs, eight root types, 24 licensed WaveEdit sources, 1536 parameter candidates",
    "new_bank_display_label":"ALBEDO · BIOLUMINAL ATLAS",
    "new_bank_internal_key":"Single unified factory library",
    "source_level_and_gui_semantic_audit":"required before native binary packaging",
    "sample_rate_and_audio_render_validation":"not completed",
    "bitwig_manual_validation":"pending",
    "asio4all_manual_validation":"pending",
    "sonic_distinctness":"unverified",
    "audible_quality":"unverified",
    "release_status":"EXPERIMENTAL — not a replacement for the user-confirmed CP64R1",
    "user_confirmed_stable_rollback":"CP64R1",
    "development_predecessor":"CP65R1"
})
out=root/"CP66_EXPERIMENTAL_PACKAGE_QA.json"
out.write_text(json.dumps(data,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
legacy.unlink()
print("CP66 experimental metadata and truthful QA status recorded")
if __name__=="__main__":pass
