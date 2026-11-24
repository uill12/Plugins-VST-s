#!/usr/bin/env python3
"""CP62: remove velocity accents from the native Albedo arp, preserving legacy state ID.
Apply after the CP58R4/CP60/CP61 integrations. Fail closed if the expected
architecture has changed; no edit is made on a mismatched source tree.
"""
from pathlib import Path
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--root", type=Path, default=Path("Albedo-Quintessential-Synth"))
args = parser.parse_args()
arp = args.root / "include" / "AlbedoCellArp.inc"
s = arp.read_text(encoding="utf-8")
variants = (
    """   const double meterWeight=arpMeterWeight(arpIndex_,division,int(get(6032)));
   const double accent=clamp(get(6022)*.01,0.,1.);
   const double velocityScale=1.-accent*((emphasis?0.:.25)+(.15*(1.-meterWeight)));
   voiceOn(note,clamp(key.vel*velocityScale,.001,1.),key.ch,-1);""",
    """   // Separate CA lattice shapes accents only; no pitch drift or random transpose.
   const bool energetic=((arpBits_>>step)&1)!=0;
   const double accent=clamp(get(6022)*.01,0.,1.);
   const double velocityScale=energetic?1.:(1.-.70*accent);
   voiceOn(note,clamp(key.vel*velocityScale,.001,1.),key.ch,-1);""",
)
replacement = """   // CP62: velocity is inherited from the held key, independent of lattice
   // emphasis or meter. Preserve gate, rests, pattern and original expressivity.
   // Deprecated ID 6022 stays readable in saved states; it is no longer audible.
   voiceOn(note,clamp(key.vel,.001,1.),key.ch,-1);"""
matches = [x for x in variants if s.count(x) == 1]
if len(matches) != 1:
    raise RuntimeError("CP62: expected exactly one recognized arp accent block")
if "CP62: velocity is inherited" in s:
    raise RuntimeError("CP62 already applied")
s = s.replace(matches[0], replacement)
arp.write_text(s, encoding="utf-8")
print("CP62 native accent removal: parameter 6022 legacy-compatible, velocity accents disabled")
