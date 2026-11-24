#!/usr/bin/env python3
"""Albedo CP50 Nereida-inspired arpeggiator source/GUI contract.
This is not a Bitwig performance claim; CTest exercises rendered native behavior.
"""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[2]
html=(root/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
src=(root/"patches/cp50/AlbedoCellArp.inc").read_text(encoding="utf-8")
apply=(root/"patches/cp50/apply.py").read_text(encoding="utf-8")
test=(root/"patches/cp50/arp_test.cpp").read_text(encoding="utf-8")
cmake=(root/"Albedo-JUCE-Native/CMakeLists.txt").read_text(encoding="utf-8")
processor=(root/"Albedo-JUCE-Native/Source/AlbedoProcessor.cpp").read_text(encoding="utf-8")
assert ("VERSION 1.0.51" in cmake or "VERSION 1.0.53" in cmake or "VERSION 1.0.54" in cmake or "VERSION 1.0.55" in cmake or "VERSION 1.0.56" in cmake or "VERSION 1.0.57" in cmake or "VERSION 1.0.58" in cmake or "VERSION 1.0.59" in cmake), "Maintain CP50 compatibility under CP53 metadata"
assert "BUNDLE_ID \"org.vitriol.albedo.juce32\"" in cmake
for i,character in enumerate(("Oblique","Symmetrical","Cyclical","Asymmetrical")):
    assert f'<option value="{i}"' in html and character in html
for pid in (6022,6023,6024,6025,6026,6027,6028):
    assert f'data-arp="{pid}"' in html, pid
    assert f'add({pid},' in apply or pid in (6022,6023), pid
for token in ("const bool obliqueGap","const bool asymmetricGap","2ull*cursor+cursor/2",
              "5ull*cursor+cursor/3","2*span-2","get(6028)",
              "arpHostValid_","++arpIndex_",
              "arpBits_","std::sort","arpPrune()","voicePanic()"):
    assert token in src or token in apply, token
assert "std::stable_sort" not in src
assert "std::vector" not in src and "new " not in src
assert "get(6000+step)" in src, "Legacy editable step mask must remain active"
assert "get(4003)" in src, "CA rule selection must remain"
assert "s->setParam(6025,1)" in test and "s->midiCC(64,127)" in test
for rate in (44100,48000,88200,96000):
    assert str(rate) in test
assert "syncArpClock()" in processor and "synth->setArpTempo(tempo)" in processor
assert "albedoArpStep" in html, "GUI must follow the actual synth step"
assert html.count('id="cellArp"')==1
assert "Pulse Depth" not in apply.split("p=params.read_text()")[1].split("params.write_text(p)")[1]
print("CP50 Nereida-style arp source contract PASS: four character modes, host/free BPM, CA accent, MIDI ownership, all step gates")
