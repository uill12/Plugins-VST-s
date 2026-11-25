#!/usr/bin/env python3
"""CP62 acceptance gate after CP61 reconstruction and native patch application."""
from pathlib import Path
root=Path(__file__).resolve().parents[2]
html=(root/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
arp=(root/"Albedo-Quintessential-Synth/include/AlbedoCellArp.inc").read_text(encoding="utf-8")
priority=(root/"Albedo-JUCE-Native/Assets/cp56_sound_priority.js").read_text(encoding="utf-8")
tempo=(root/"Albedo-JUCE-Native/Assets/cp57_tempo_controls.js").read_text(encoding="utf-8")
assert 'CA ACCENT %' not in html
assert 'data-arp="6022"' not in html
assert 'data-arp="6032"' in html
assert 'data-arp="6030"' in html
assert 'CP62: velocity is inherited' in arp
assert 'voiceOn(note,clamp(key.vel,.001,1.),key.ch,-1);' in arp
assert 'const double velocityScale=' not in arp
assert "[6022,'CA ACCENT']" not in priority
assert "[[6023,'GATE'],[6026,'SWING']" in priority
assert "old.dispatchEvent(new Event('change',{bubbles:true}))" not in tempo
assert "get(6000+step)" in arp, "Step gate mask unexpectedly removed"
assert "arpRelease()" in arp, "Step release unexpectedly removed"
print("CP62 PASS: accent-free native note velocity, event gate, timing bridge, backwards-compatible parameter ID")
