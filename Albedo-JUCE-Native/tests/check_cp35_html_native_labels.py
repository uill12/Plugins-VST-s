"""CP35 source-level label-fidelity gate: native JUCE versus canonical CP31 HTML.

This verifies the original CP31 synthesis-method, knob and alchemical names are
not silently replaced in the native editor. It does not assert pixel parity.
"""
from __future__ import annotations

import ast
import re
from pathlib import Path

root = Path(__file__).resolve().parents[2]
html = (root / "ui" / "Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
cpp = (root / "Albedo-JUCE-Native" / "Source" / "AlbedoEditor.cpp").read_text(encoding="utf-8")
hdr = (root / "Albedo-JUCE-Native" / "Source" / "AlbedoEditor.h").read_text(encoding="utf-8")


def body(source: str, pattern: str) -> str:
    result = re.search(pattern, source, re.DOTALL)
    assert result is not None, f"Required source table missing: {pattern}"
    return result.group(1)


def js_array(key: str):
    return ast.literal_eval(body(html, rf"const {key}=(\[[^\n]+?\]);"))


def cpp_array(key: str, count: int):
    raw = body(cpp, rf"{re.escape(key)}\[{count}\]\s*=\s*\{{(.*?)\}};")
    return ast.literal_eval("[" + raw.replace("{", "[").replace("}", "]") + "]")


methods = js_array("METHODS")
palette = js_array("PALETTE")
engines = js_array("ENGINES") if "const ENGINES=" in html else []
knobs = ast.literal_eval("{" + body(html, r"const KNOBS=\{(.*?)\};") + "}")

assert len(methods) == 10 and len(knobs) == 10
assert cpp_array("kSynthesisMethods", 10) == methods
assert [x.upper() for x in engines] == cpp_array("kEngineMatter", 4)
assert cpp_array("kMethodKnobs", 10) == [knobs[name] for name in methods]
assert cpp_array("kMacroNames", 7) == [entry[1] for entry in palette]

sections = [
    "Prima Materia", "Source Inspector", "Arbor Resonantia",
    "Corpus & Character", "Correspondentiae", "Motus & Cellular",
    "Spatium & Transform", "Pulsus",
]
for section in sections:
    assert section in html and section in cpp, f"Missing CP31 module: {section}"
for phrase in [
    "FOUR-ENGINE OSCILLATOR RACK", "SELECTED SOURCE",
    "HARMONIC MATERIAL", "ARTICULATION & FORM",
    "VISITA INTERIORA TERRAE", "STRUCTURE BECOMES SOUND",
]:
    assert phrase in cpp, f"Native UI has not retained HTML phrase: {phrase}"

for code_contract in [
    "sourceMethod(processor, activeSource)",
    "originalHtmlLabel(id, p->getName(100), processor)",
    "engineMethodLabels[(size_t)i].setText",
    "engineMethodLabels[(size_t)i].setBounds",
    "slider.setTooltip(name",
    "JUCE NATIVE · NO WEBVIEW2",
]:
    assert code_contract in cpp, f"Missing runtime label contract: {code_contract}"
assert "std::array<juce::Label,4> engineMethodLabels;" in hdr
assert "AlbedoWebViewEditor" not in cpp
print("CP35 PASS: native JUCE label tables match the canonical CP31 HTML.")
print("Host screenshots, exact glyph metrics and sound remain to be verified separately.")
