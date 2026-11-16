import json
from pathlib import Path
r=Path(__file__).resolve().parents[1]
m=json.loads((r/'assets/scenes/meta.json').read_text())
def ir(a):
    if not a:return '{0,0,0,0}'
    return '{%s}'%(','.join(str(round(v)) for v in a))
def type_of(s):
    name=s['label'].upper()
    import re
    if re.search(r'SAFE NOTES|MPE SAFE|NOTE TRACK|DUPLICATE GUARD|DEDUPE|FORCED ROOT|STUCK NOTE SAFE',name):return 3
    if re.search(r'GRID|SLOPE|CHANNEL|QUALITY|SCALE ROOT|VOICES|OCTAVE RANGE',name):return 2
    if re.search(r'MIX|BLEND|DRY / WET|OUTPUT|TRIM|CEILING|CROSSOVER|CENTER|RATIO|SPLIT NOTE|TIME SPREAD|ATTACK|RELEASE|PREDELAY|LOOKAHEAD',name):return 1
    return 0
out=['#pragma once','// Generated from the actual CP10 RGNES browser DOM at fixed 1600 x 1000 viewport.','struct NGuiR {int x,y,w,h;};','struct NGuiParam {NGuiR control,value;int style;};','static const NGuiParam NGUI_CONTROLS[2][7][9] = {']
for kind in ('audio','midi'):
    out.append(' {')
    for i in range(7):
        entries=m[kind]['stages'][str(i)]['controls']
        out.append('  {'+', '.join('{'+ir(e['control'])+','+ir(e.get('value'))+','+str(type_of(e['spec']))+'}' for e in entries)+'},')
    out.append(' },')
out.append('};')
out.append('static const NGuiR NGUI_STAGE[2][7] = {')
for kind in ('audio','midi'):
    out.append(' { '+','.join(ir(x[1]) for x in m[kind]['hitboxes']['stage'])+'},')
out.append('};')
for tag,key in [('TOGGLE','toggle'),('PATH','path')]:
    out.append(f'static const NGuiR NGUI_{tag}[2][7] = {{')
    for kind in ('audio','midi'):out.append(' { '+','.join(ir(x[1]) for x in m[kind]['hitboxes'][key])+'},')
    out.append('};')
out.append('static const NGuiParam NGUI_MASTERS[2][3] = {')
for kind in ('audio','midi'):
    vals=m[kind]['hitboxes']['master']
    # master DOM .param itself may not match input, handle later custom
    out.append(' { '+','.join('{'+ir(x['control'])+','+ir(x['value'])+','+str(([1,1,2] if kind=='audio' else [0,0,2])[j])+'}' for j,x in enumerate(vals))+'},')
out.append('};')
for name,key in [('POWER','power'),('RESET','reset'),('EARLIER','earlier'),('LATER','later'),('STAGEPOWER','stagePower'),('PRESET','preset'),('APPLY','apply'),('EXPORT','export'),('IMPORT','import')]:
    out.append(f'static const NGuiR NGUI_{name}[2] = {{'+','.join(ir(m[kind]['hitboxes'][key]) for kind in ('audio','midi'))+'};')
(r/'src/GuiSceneData.h').write_text('\n'.join(out)+'\n')
print('header lines',len(out))
