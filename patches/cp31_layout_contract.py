#!/usr/bin/env python3
"""CP31: verify HTML resource, 3:2 host geometry and animation without DSP edits."""
from pathlib import Path
import hashlib,json
r=Path('Albedo-Quintessential-Synth')
h=r/'resources/Albedo_CP30_CP22_3x2_embedded.html'
s=h.read_text(encoding='utf-8')
assert len(s)>250000,'CP31 embedded HTML truncated'
required=[
 'w/canvas.offsetWidth',
 "aspectRatio:'3:2'",
 'window.ALBEDO_CP31_MOTION',
 'cp31-signal-particle',
 'requestAnimationFrame(render)',
 "getPointAtLength(off)",
 'window.ALBEDO_CP04',
 'window.ALBEDO_NATIVE_CP27?.onDial(k,v)',
 'window.ALBEDO_NATIVE_CP27?.onPreset(name)',
 'id="albedoFactoryCatalog"',
 'id="presetBrowser"',
 'id="cp04Lab"'
]
assert all(x in s for x in required),[x for x in required if x not in s]
assert s.count('<div class="brand"><svg')==1
assert s.count('id="presetBrowser"')==1
assert s.count('id="cp04Lab"')==1
assert 'pointermove' in s
layout=(r/'include/vitriol_native_layout.hpp').read_text(encoding='utf-8')
assert 'kDefaultEditorWidth=1440, kDefaultEditorHeight=960' in layout
assert 'constexpr double aspect=3.0/2.0;' in layout
m=json.loads((r/'CP19_SOURCE_MANIFEST.json').read_text(encoding='utf-8'))
actual=hashlib.sha256(h.read_bytes()).hexdigest()
assert m['changed_source_sha256']['resources/Albedo_CP30_CP22_3x2_embedded.html']==actual
print('CP31 verified: HTML live UI + moving Arbor + factory Atlas + native bridge + 3:2')
print('CP31 resource SHA256:',actual)
