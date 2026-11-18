"""Fail-closed identity gate for the Albedo JUCE native / No WebView2 variant.

Run: python Albedo-JUCE-Native/tests/check_plugin_identity.py
Only static source/metadata checks. A passing result is NOT a compiled or Bitwig-tested plugin.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

PROJECT = Path(__file__).resolve().parents[1]
manifest_path = PROJECT / "Metadata" / "PLUGIN_IDENTITY.json"
metadata = json.loads(manifest_path.read_text(encoding="utf-8"))
cmake = (PROJECT / "CMakeLists.txt").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


require(metadata["variant_id"] == "juce-native-no-webview2", "Incorrect variant")
require(metadata["renderer"] == "JUCE 8 native C++ components", "Incorrect renderer")
for field in ("requires_webview2", "embeds_html_runtime", "juce_web_browser"):
    require(metadata[field] is False, f"{field} must be false")

assertions = {
    "product": f'PRODUCT_NAME "{metadata["product_name"]}"',
    "bundle": f'BUNDLE_ID "{metadata["vst3"]["bundle_id"]}"',
    "manufacturer": f'PLUGIN_MANUFACTURER_CODE {metadata["vst3"]["manufacturer_code"]}',
    "VST3 plugin code": f'PLUGIN_CODE {metadata["vst3"]["plugin_code"]}',
    "CLAP plugin ID": f'CLAP_ID "{metadata["clap"]["plugin_id"]}"',
    "native browser flag": "JUCE_WEB_BROWSER=0",
    "native renderer flag": "ALBEDO_GUI_NATIVE_JUCE=1",
    "disabled WebView2 flag": "ALBEDO_GUI_WEBVIEW2=0",
}
for label, token in assertions.items():
    require(token in cmake, f"Missing {label} metadata: {token}")

require(metadata["vst3"]["plugin_code"] != metadata["compatibility"]["existing_webview2_product_code"],
        "Native VST3 and WebView2 VST3 must use different plugin codes")
require(metadata["clap"]["plugin_id"] != metadata["compatibility"]["existing_webview2_clap_id"],
        "Native CLAP and WebView2 CLAP must have different IDs")
require(re.fullmatch(r"[A-Za-z0-9]{4}", metadata["vst3"]["plugin_code"]) is not None,
        "VST3 plugin code must be 4 ASCII letters/digits")

for banned in (
    r"NEEDS_WEBVIEW2\s+TRUE",
    r"JUCE_WEB_BROWSER\s*=\s*1",
    r"juce::juce_webview2\b",
    r"juce::juce_gui_extra\b",
    r"Source/AlbedoWebViewEditor\.(cpp|h)",
    r"Assets/juce_cp33_bridge\.js",
    r"\bWebBrowserComponent\s*[(<:]",
):
    require(re.search(banned, cmake) is None, f"Browser dependency in CMake: {banned}")

sources = list((PROJECT / "Source").glob("*.cpp")) + list((PROJECT / "Source").glob("*.h"))
require(sources, "Native C++ source tree missing")
for file in sources:
    source = file.read_text(encoding="utf-8")
    for banned in (r"#include\s*[<\"][^>\"]*WebView", r"juce::WebBrowserComponent",
                   r"new\s+AlbedoWebViewEditor\b"):
        require(re.search(banned, source) is None, f"Browser UI in {file.name}: {banned}")
require("Source/AlbedoEditor.cpp" in cmake and "Source/AlbedoProcessor.cpp" in cmake,
        "Native JUCE source entrypoints missing")
require("AlbedoWebViewEditor" not in cmake, "WebView editor included in native target")
require("No WebView2" in metadata["product_name"], "The host-visible name must distinguish this variant")
print("PASS: JUCE native No WebView2 source and metadata identity; build/Bitwig QA not asserted")
