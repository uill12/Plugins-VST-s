#!/usr/bin/env python3
"""RGNES G1/G2: derive the native control contract from actual CP09 DOM and stable C++ registry."""
from pathlib import Path
import csv,hashlib,json,re
R=Path(__file__).resolve().parents[1];docs=R/'docs';docs.mkdir(exist_ok=True)
meta=json.loads((R/'assets/scenes/meta.json').read_text())
sp= (R/'src/SharedParams.h').read_text()
rx=r'\{"([^"\\]+)","([^"\\]*)",(-?\d+(?:\.\d+)?),(-?\d+(?:\.\d+)?),(-?\d+(?:\.\d+)?),(-?\d+(?:\.\d+)?),(\d)\}'
spec=[{'label':name,'unit':unit,'min':float(mi),'max':float(ma),'default':float(de),'step':float(step),'param_kind':int(kind)} for name,unit,mi,ma,de,step,kind in re.findall(rx,sp)]
assert len(spec)==164,f'Expected 164 parameters (82x2); found {len(spec)}'
names=['Calcination','Dissolution','Separation','Conjunction','Fermentation','Distillation','Coagulation']
functions={'audio':['Saturation / Drive','Stereo Delay / Diffusion','Multiband / Transient','Harmonic Blend','Modulation / Feedback','Spectral EQ','Compression / Limiter'], 'midi':['Velocity Shaper','Timing / Legato','Note Split / Routing','Chord Harmonizer','Probability / Mutation','Scale / Note Filter','Quantize / Gate']}
fields=['id','mode','group','kind','bounds_ref','draw_layer','label','value_field','parameter_id','state_key','input_gesture','range_min','range_max','default','taper','units','automatable','saved','engine_destination','thread_contract','test_id']
rows=[]
def add(mode,id,group,kind,rect,label,pid,minval='',maxval='',default='',step='',units='',automatable='true',saved='true',gesture='click',action=''):
 rows.append({'id':id,'mode':mode,'group':group,'kind':kind,'bounds_ref':json.dumps([round(x,2) for x in rect]),'draw_layer':'reactive-native' if action else 'dynamic-native','label':label,'value_field':f'params[{pid}]' if isinstance(pid,int) else action,'parameter_id':pid if isinstance(pid,int) else '', 'state_key':f'{mode}.{group}.{id}','input_gesture':gesture,'range_min':minval,'range_max':maxval,'default':default,'taper':('discrete' if kind in ('choice','switch','toggle','select') else 'linear'),'units':units,'automatable':automatable,'saved':saved,'engine_destination':action or f'Engine.h params[{pid}]','thread_contract':'GUI→host parameter→audio/MIDI render' if isinstance(pid,int) else 'GUI/local state','test_id':f'RGNES_{len(rows)+1:03d}'})
for modeindex,mode in enumerate(('audio','midi')):
 m=meta[mode]; specs=spec[modeindex*82:(modeindex+1)*82]
 for stage in range(7):
  entries=m['stages'][str(stage)]['controls'];assert len(entries)==9
  for j,entry in enumerate(entries):
   p=specs[stage*9+j]; label=p['label'];control=entry['control']; assert control
   style=('switch' if re.search(r'SAFE NOTES|MPE SAFE|NOTE TRACK|DUPLICATE GUARD|DEDUPE|FORCED ROOT|STUCK NOTE SAFE',label) else
          'choice' if re.search(r'GRID|SLOPE|CHANNEL|QUALITY|SCALE ROOT|VOICES|OCTAVE RANGE',label) else
          'fader' if re.search(r'MIX|BLEND|DRY / WET|OUTPUT|TRIM|CEILING|CROSSOVER|CENTER|RATIO|SPLIT NOTE|TIME SPREAD|ATTACK|RELEASE|PREDELAY|LOOKAHEAD',label) else 'knob')
   add(mode,f'{mode}_stage_{stage+1:02d}_param_{j+1:02d}',functions[mode][stage],style,control,label,stage*9+j,p['min'],p['max'],p['default'],p['step'],p['unit'],gesture=('drag|wheel|keyboard|reset' if style=='knob' else 'change|click|keyboard' if style in ('choice','switch') else 'drag|keyboard'))
 for i in range(7):
  p=specs[63+i];t=m['hitboxes']['toggle'][i][1]
  add(mode,f'{mode}_stage_{i+1:02d}_onoff',functions[mode][i],'toggle',t,f'{names[i]} ON/OFF',63+i,0,1,1,1,'','true','true','click|keyboard','EnableBase')
 for i in range(7):
  p=specs[70+i]
  add(mode,f'{mode}_stage_{i+1:02d}_order',functions[mode][i],'state',m['hitboxes']['stage'][i][1],f'{names[i]} position',70+i,0,6,i,1,'','true','true','earlier|later','OrderBase')
 for i in range(3):
  p=specs[77+i]
  add(mode,f'{mode}_global_{i+1}', 'Global', ('choice' if 'QUALITY' in p['label'] else 'fader' if any(a in p['label'] for a in ('DRY','OUTPUT')) else 'knob'),m['hitboxes']['master'][i]['control'],p['label'],77+i,p['min'],p['max'],p['default'],p['step'],p['unit'],gesture='drag|wheel|keyboard')
 add(mode,f'{mode}_fx_chain_power','Global','toggle',m['hitboxes']['power'],'FX CHAIN ON/OFF',80,0,1,0,1,'','true','true','click|keyboard','BypassId (0=on,1=off)')
 add(mode,f'{mode}_selected_stage','Global','state',m['hitboxes']['stage'][3][1],'Selected stage',81,0,6,3,1,'','true','true','click|keyboard','SelectedId')
 for i in range(7):
  add(mode,f'{mode}_stage_{i+1:02d}_select','Stage Navigation','select',m['hitboxes']['stage'][i][1],functions[mode][i],'','','','','','','false','false','click|keyboard',f'select stage {i}, updates selected ID81')
  add(mode,f'{mode}_stage_{i+1:02d}_radial','Cosmogram','select',m['hitboxes']['path'][i][1],names[i],'','','','','','','false','false','click|keyboard',f'select stage {i}, updates selected ID81')
 for tag,label in [('stagePower','Selected Stage ON/OFF'),('reset','Reset Stage'),('earlier','Move Earlier'),('later','Move Later'),('preset','Preset Selector'),('apply','Apply Preset'),('export','Export JSON'),('import','Import JSON')]:
  add(mode, f'{mode}_{tag}', 'Editor Actions','button',m['hitboxes'][tag],label,'','','','','','','false','false','click|keyboard',label)
 # Mode switch appears only in browser. In native, the plugin's type is immutable by design.
 add(mode,f'{mode}_mode_indicator','Plugin Format','display',m['hitboxes']['mode'],'This plugin mode','', automatable='false',saved='false',gesture='disabled in native',action='Separate plugins; no in-plugin type switching')
with (docs/'control_registry.csv').open('w',encoding='utf-8',newline='') as f:
 w=csv.DictWriter(f,fieldnames=fields);w.writeheader();w.writerows(rows)
with (docs/'layout_spec.json').open('w',encoding='utf-8') as f:
 json.dump({'standard':'RGNES 1.0','checkpoint':'CP09','date':'2026-11-15','design_canvas':[1600,1000],'default_native':[1440,900],'aspect':'16:10','layout':'seven horizontal operation cards above side-by-side cosmogram and editor','paint_and_hit_testing':'same 1600x1000 design-space transform with inverse viewport mapping','mode_specific_operation_labels':functions,'reference_atlas':meta,'overlap_policy':'stage toggle overlays each card selection; toggle hitbox has priority'},f,indent=2)
with (docs/'design_tokens.json').open('w',encoding='utf-8') as f:
 json.dump({'name':'Nigredo manuscript cosmology','standard':'RGNES 1.0','canvas':[1600,1000],'palette':{'parchment':'#f8e8c7','old_gold':'#c89b59','oxide_red':'#a94739','aged_ink':'#673a35','moss_green':'#56845a','indigo':'#29304c'},'font_families':['Georgia','serif'],'control_shapes':['fader','rotary_knob','choice','switch','global_chain_toggle','stage_toggle'],'minimum_native':[960,600],'live_not_baked':['power ON/OFF','stage switches ON/OFF','parameter numeric values','knob pointers','fader positions','stage order indices','preset name']},f,indent=2)
assets=[]
for f in sorted((R/'assets/scenes').glob('*.png')):
 assets.append({'path':str(f.relative_to(R)),'bytes':f.stat().st_size,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()})
with (docs/'resource_manifest.json').open('w') as f:json.dump({'original_images':['/mnt/data/1794699558648.jpg','/mnt/data/image.png'],'scenes':assets},f,indent=2)
print('G2 registry created:',len(rows),'controls & actions across two modes; 164 automatable params')
