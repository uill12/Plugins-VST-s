#!/usr/bin/env python3
"""RGNES CP10 offline validation; does not claim Windows host GUI runtime testing."""
from pathlib import Path
import io,struct,subprocess,hashlib,json,zipfile
from PIL import Image
R=Path(__file__).resolve().parents[1]
w=(R/'src/WinGui.h').read_text()
required=['compatibleDC','compatibleBitmap','NG.blit(v->frontDC','NG.blit(dc,rr.left','invalidateZone','invalidateParam','v->compositeDirty','v->cacheStage!=stage','WM_ERASE_','return 1;','dropBackbuffers(v)','offsetof(NCreateStructA,createParams)==0']
for token in required:assert token in w,token
pv=w[w.index('static void pvEdit'):w.index('static void paintText')]
assert 'invalidateParam(v,id);' in pv and 'InvalidateRect(v->hwnd,nullptr,0)' not in pv
sync=w[w.index('static void syncNative'):w.index('static int hitParam')]
assert 'invalidateParam(v,i)' in sync and 'InvalidateRect(v->hwnd,nullptr,0)' not in sync
assert 'pvRead(v,id)>=(sp.lo+sp.hi)*.5' in w
assert 'NGUI_MASTERS' in (R/'src/GuiSceneData.h').read_text()
assert '0,0,2}' not in '' # syntax sanity
assert '0,0,2' in (R/'scripts/build_scene_header.py').read_text()
res=[]
for kind in ('Audio','MIDI'):
 for ext in ('clap','vst3'):
  f=R/'distrib'/f'Nigredo_Opus_Septem_{kind}_FX_Windows_x64.{ext}'
  data=f.read_bytes();assert data[:2]==b'MZ' and data[-120:-116]==b'NIG7'
  assert struct.unpack_from('<I',data,len(data)-116)[0]==7
  assert b'WebView2Loader' not in data and b'libcef' not in data
  assert f'NigredoOpusSeptemCP10_{"CLAP" if ext=="clap" else "VST3"}_{kind}_PaintFixed'.encode() in data
  if ext=='clap':assert b'0.9.1-cp10' in data
  else:assert '0.9.1-cp10' in (R/'src/Vst3.cpp').read_text()
  for stage in range(7):
   off,ln=struct.unpack_from('<QQ',data,len(data)-112+stage*16)
   assert ln>100000 and off+ln<=len(data)-120
   png=data[off:off+ln]
   im=Image.open(io.BytesIO(png));im.verify()
   assert Image.open(io.BytesIO(png)).size==(1600,1000)
   file_png=(R/'assets/scenes'/f'{kind.lower()}_{stage}.png').read_bytes()
   assert file_png==png
  pe=subprocess.check_output(['llvm-objdump','-p',str(f)],text=True)
  assert 'DLL Name: USER32.dll' in pe
  assert 'DLL Name: WebView2' not in pe and 'DLL Name: libcef' not in pe
  assert ('clap_entry' if ext=='clap' else 'GetPluginFactory') in pe
  res.append({'file':f.name,'sha256':hashlib.sha256(data).hexdigest(),'bytes':len(data),'scenes':7,'status':'compiled + static-verified; Windows Bitwig not tested'})
  print('PASS',kind,ext,'native cached render, seven PNGs, PE entrypoint, no WebView2')
# RGNES HTML reference + masks: only mutable labels are removed from baked imagery.
gen=(R/'scripts/generate_scenes.py').read_text()
for token in ('.param-val {visibility:hidden', '#stagePower {color:transparent', '#presetSelect {color:transparent', '.power-thumb {visibility:hidden'):
 assert token in gen,token
manifest={'checkpoint':'CP10','date':'2026-11-15','evidence':'compiled + offline-test only; no Windows host GUI tests','binaries':res}
(R/'release/CP10_SHA256.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('PASS CP10 RGNES G2/G4/G5 static checks (host lifecycle & redraw require Windows validation)')
