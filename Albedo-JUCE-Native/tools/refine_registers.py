"""Voice the four existing source layers for the register of each instrument."""
import json
import sys
from pathlib import Path

# Secondary body, supporting color, upper detail (semitones from the played key).
registers = {
    "Bass": (0, -12, 12), "Lead": (-12, 0, 12), "Pad": (0, -12, 12),
    "Keys": (0, 12, -12), "Pluck": (0, -12, 12), "Drone": (-12, 0, 12),
    "Texture": (0, -12, 12), "Percussion": (0, -12, 12),
}
for filename in ("material_factory_bank.json", "recursive_forms_bank.json", "morph_expansion_bank.json"):
    path = Path(sys.argv[1]) / filename
    rows = json.loads(path.read_text(encoding="utf-8"))
    assert len(rows) == 64
    for row in rows:
        p = row["overrides"]
        role = row["role"]
        body, color, detail = registers[role]
        # Keep selected fifths in the quiet detail voice for evolving instruments.
        old_detail = p["1099"]
        if role in ("Pad", "Texture", "Drone") and old_detail in (-5, 7):
            detail = old_detail + 12 if old_detail < 0 else old_detail
        for layer, pitch in enumerate((0, body, color, detail)):
            p[str(1003 + layer * 32)] = pitch
        # Move energy into the two body layers while retaining the original level sum.
        transfer = min(5, p["1066"] * .12, p["1098"] * .18, 100-p["1034"], 100-p["1002"])
        for key, delta in [(1002, transfer), (1034, transfer), (1066, -transfer), (1098, -transfer)]:
            p[str(key)] = round(p[str(key)] + delta, 5)
    path.write_text(json.dumps(rows, ensure_ascii=False, indent=1)+"\n", encoding="utf-8")
