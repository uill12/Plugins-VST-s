#!/usr/bin/env python3
"""CP18 output-name repair: never upload an empty CP18 artifact from CP17 path."""
from pathlib import Path
import hashlib,json
root=Path('Albedo-Quintessential-Synth')
p=root/'Build_Windows_x64.ps1'
s=p.read_text(encoding='utf-8')
if s.count('dist_windows_CP17')!=2 or s.count('Albedo_Quintessential_Synth_Windows_x64_CP17.zip')!=1:
 raise SystemExit('Unexpected Windows dist script version: refusing blind output rename')
s=s.replace('dist_windows_CP17','dist_windows_CP18')
s=s.replace('Albedo_Quintessential_Synth_Windows_x64_CP17.zip','Albedo_Quintessential_Synth_Windows_x64_CP18.zip')
s=s.replace('CP16 Windows build outputs','CP18 Windows build outputs')
p.write_text(s,encoding='utf-8',newline='\n')
digest=hashlib.sha256(p.read_bytes()).hexdigest()
if digest!='77f628fbef2883bb01cac1644973469d553c9a758fb13abb57680424b25caf1a':
 raise SystemExit('Unexpected CP18 build source hash '+digest)
mf=root/'CP18_SOURCE_MANIFEST.json';m=json.loads(mf.read_text(encoding='utf-8'))
m['changed_source_sha256']['Build_Windows_x64.ps1']=digest
mf.write_text(json.dumps(m,indent=2,ensure_ascii=False)+'\n',encoding='utf-8',newline='\n')
print('CP18 build outputs configured for dist_windows_CP18 and CP18 plugin ZIP')
