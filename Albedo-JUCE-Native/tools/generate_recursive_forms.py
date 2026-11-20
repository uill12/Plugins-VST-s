#!/usr/bin/env python3
"""Generate a contiguous 64-program native bank from its sound-design metadata.
No copyrighted source audio, MIDI notes, chord progressions or sequences.
"""
from pathlib import Path
import argparse, json, math
p=argparse.ArgumentParser()
p.add_argument("--metadata",type=Path,default=Path("Albedo-JUCE-Native/Metadata/recursive_forms_bank.json"))
p.add_argument("--output",type=Path,default=Path("Albedo-JUCE-Native/Source/AlbedoCreativeBank.hpp"))
p.add_argument("--bank", choices=["creative","material","previous","morph"], default="creative")
args=p.parse_args()
namespace = {"creative":"albedo_creative","material":"albedo_material","previous":"albedo_previous","morph":"albedo_morph"}[args.bank]
first = {"material":0,"creative":64,"previous":64,"morph":128}[args.bank]
rows=json.loads(args.metadata.read_text(encoding="utf-8"))
assert len(rows)==64
assert [p["index"] for p in rows]==list(range(first, first+64))
assert len({p["name"] for p in rows})==64
if args.bank in ("creative","previous"): assert {p["family"] for p in rows}=={"Resynthesis Forms","Repetition Fields","Phase Studies"}
assert all(p["baseIndex"] in range(64) for p in rows)
for p in rows:
    assert len(p["overrides"])>=35
    assert all(math.isfinite(float(value)) for value in p["overrides"].values())
    assert p["overrides"]["3111"]==0  # every instrument remains polyphonic
    assert -36<=p["overrides"]["9000"]<=12
    assert -18<=p["overrides"]["9003"]<=12
    for i in range(4):
        b=1000+32*i
        assert 0<=p["overrides"][str(b)]<=9
        assert 0<=p["overrides"][str(b+2)]<=100
def escaped(v):return json.dumps(v,ensure_ascii=True)
def cfloat(v):
    n=float(v)
    s=format(n,'.8g')
    if "e" not in s and "." not in s:s+=".0"
    return s+"f"
out=["#pragma once",
     "// Generated from preset metadata. 64 sound slots per bank; index identity is stable.",
     "// No embedded artist samples; compiled changes refer only to internal DSP resources.",
     "#include <array>",
     "#include <cstddef>",
     "#include <cstdint>",
     "namespace "+namespace+" {",
     "struct Change { uint32_t id; float value; };",
     "struct Preset { int index; const char* name; const char* family;",
     " const char* role; int baseIndex; const Change* changes; std::size_t changeCount; };"]
for p in rows:
    out.append(f"inline constexpr Change changes_{p['index']}[] = {{")
    for id,val in sorted(p["overrides"].items(),key=lambda item:int(item[0])):
        out.append(" {"+str(int(id))+"u, "+cfloat(val)+"},")
    out.append("};")
out.append("inline constexpr std::array<Preset,64> patches{{")
for p in rows:
    i=p["index"]
    out.append(" {"+str(i)+", "+escaped(p["name"])+", "+escaped(p["family"])+", "+escaped(p["role"])+", "+str(p["baseIndex"])+f", changes_{i}, std::size(changes_{i})"+"},")
out+=["}};","static_assert(patches.size()==64);","} // namespace albedo_creative",""]
args.output.parent.mkdir(parents=True,exist_ok=True)
args.output.write_text("\n".join(out),encoding="utf-8")
print("Generated",args.bank,"bank with 64 indexed programs:",first,"through",first+63,"metadata:",args.metadata)

# Keep both program tables in sync for local builds that use the default entry point.
if args.bank=="creative" and args.metadata.name=="recursive_forms_bank.json":
    import subprocess, sys
    subprocess.run([sys.executable,__file__,"--bank","material","--metadata",str(args.metadata.with_name("material_factory_bank.json")),"--output",str(args.output.with_name("AlbedoMaterialBank.hpp"))],check=True)
