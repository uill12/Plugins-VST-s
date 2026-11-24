#!/usr/bin/env python3
"""CP60 integration invariants; called after CP59 reconstructed DSP & patches."""
from pathlib import Path
import json,sys
root=Path('Albedo-Quintessential-Synth')
s=(root/'src/vitriol_dsp.cpp').read_text(encoding='utf-8')
h=(root/'include/vitriol_dsp.hpp').read_text(encoding='utf-8')
params=(root/'include/vitriol_params.hpp').read_text(encoding='utf-8')
cmake=Path('Albedo-JUCE-Native/CMakeLists.txt').read_text(encoding='utf-8')
ui=Path('ui/Albedo_CP30_CP22_Landscape_3x2.html').read_text(encoding='utf-8')
for marker in ('waveEditSample','waveEditGrainSample','loadWaveEditBankBytes','reverbTick','refreshFxCoefficients','lnCosh','sourceFilter', 'const double space=clamp(modFrame_[8],-1.,1.)','const size_t count=std::min<size_t>(32,rev_.clearRemaining)'):
 assert marker in s,marker
assert 'clearRemaining=rev_.mask+1' in s
assert 'for(auto& ll:rev_.line)std::fill(ll.begin(),ll.end(),0.0)' not in s
assert 'waveEditTables_' in h and 'struct Reverb' in h and 'clearRemaining' in h
assert 'add(b+10,"Material",g,0,55' in params
assert ('VERSION 1.0.60' in cmake or 'VERSION 1.0.61' in cmake)
assert 'AlbedoCP60DspQuality' in cmake and 'AlbedoCP60FxRealtimeTest' in cmake
assert 'cp60-polish' in ui and 'CRUCIBLE II' in ui
report=Path('CP60_FX_Preset_Balance_QA.json')
if report.is_file():
 j=json.loads(report.read_text(encoding='utf-8'))
 assert j['total_programs']==192 and j['revoiced_fx']==63 and j['untouched']==129
 assert j['original_pcm24_and_waveedit'] and j['parameter_ids_unchanged']
print('CP60 integrated DSP/GUI/source/preset contracts PASS')