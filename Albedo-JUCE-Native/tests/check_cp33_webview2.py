#!/usr/bin/env python3
"""CP33 WebView2 integration source checks, independent of Windows/host runtime."""
import json
from pathlib import Path

r=Path(__file__).resolve().parents[1]
editor=(r/"Source/AlbedoWebViewEditor.cpp").read_text(encoding="utf-8")
header=(r/"Source/AlbedoWebViewEditor.h").read_text(encoding="utf-8")
cmake=(r/"CMakeLists.txt").read_text(encoding="utf-8")
processor=(r/"Source/AlbedoProcessor.cpp").read_text(encoding="utf-8")
script=(r/"Assets/juce_cp33_bridge.js").read_text(encoding="utf-8")
html=(r.parent/"ui/Albedo_CP30_CP22_Landscape_3x2.html").read_text(encoding="utf-8")
taxonomy=json.loads((r/"Metadata/factory_preset_taxonomy.json").read_text(encoding="utf-8"))

required=[
 "withBackend(Options::Backend::webview2)",
 "withNativeIntegrationEnabled()",
 "withEventListener(\"albedoMessage\"",
 "withResourceProvider(",
 "withKeepPageLoadedWhenBrowserIsHidden()",
 "withUserDataFolder(profileFolder())",
 "getResourceProviderRoot()",
 "emitEventIfBrowserIsVisible(\"albedoUpdate\"",
 "emitEventIfBrowserIsVisible(\"albedoProgram\"",
 "juceHost",
 "nativeFallback=std::make_unique<AlbedoEditor>",
 "pageLoadHadNetworkError",
]
for token in required:assert token in editor,token
for token in ["static bool isAvailable();","bool pageAboutToLoad","bool pageLoadHadNetworkError"]:
 assert token in header,token
for token in ["NEEDS_WEBVIEW2 TRUE","JUCE_WEB_BROWSER=1",
              "JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1",
              "juce::juce_gui_extra","Assets/juce_cp33_bridge.js",
              "factory_preset_taxonomy.json",
              "../ui/Albedo_CP30_CP22_Landscape_3x2.html"]:
 assert token in cmake,token
assert "return new AlbedoWebViewEditor(*this)" in processor
assert "return new AlbedoEditor(*this)" in processor
assert len(taxonomy)==64
assert [x["index"] for x in taxonomy]==list(range(64))
assert len({(x["family"],x["role"]) for x in taxonomy})==64
assert "w/canvas.offsetWidth" in html
assert "window.ALBEDO_CP31_MOTION" in html
assert html.count('id="presetBrowser"')==1
assert html.count('id="cp04Lab"')==1
assert "const host=window.chrome && window.chrome.webview;" in html
for name in ["albedoUpdate","albedoProgram","Name × Material × Role",
             "factory_preset_taxonomy.json","MutationObserver(requestApply)"]:
 assert name in script,name
assert "document.body.innerHTML=" not in script
print("CP33 source contract PASS: JUCE/WebView2, 3:2 live HTML, Arbor, 64x taxonomy, bridge, fallback")
