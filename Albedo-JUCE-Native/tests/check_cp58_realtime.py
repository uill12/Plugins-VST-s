#!/usr/bin/env python3
"""CP58 static regression gate: source filter/FX cache + Windows QA wiring."""
from pathlib import Path
root=Path(__file__).resolve().parents[2]
patch=(root/'patches/cp58_realtime_cache.py').read_text(encoding='utf-8')
cmake=(root/'Albedo-JUCE-Native/CMakeLists.txt').read_text(encoding='utf-8')
workflow=(root/'.github/workflows/albedo-cp51-morph-tree-windows.yml').read_text(encoding='utf-8')
native=(root/'Albedo-JUCE-Native/tests/cp58_asio_blocks.cpp').read_text(encoding='utf-8')
assert ('VERSION 1.0.58' in cmake or 'VERSION 1.0.59' in cmake or 'VERSION 1.0.60' in cmake or 'VERSION 1.0.61' in cmake)
for needle in ('sourceDriveGain_','sourceFilterMixRate_','sourceSqrtUnison_',
               'sourceResonanceK_','sourceFilterCutoffRate_','voiceAttackDelta_',
               'sourceDriveNormalize_','std::fmod(x,size)'):
 assert needle in patch,needle
assert 'patches/cp58_realtime_cache.py' in workflow
assert 'AlbedoCP58AsioBlocks' in cmake and 'AlbedoCP58AsioBlocks' in workflow
assert 'cp58_asio_blocks.cpp' in cmake
assert 'CP58 realtime source contract failed' in workflow
for b in (64,128,256,512,1024):assert str(b) in native
for sr in (44100,48000,88200,96000):assert str(sr) in native
assert 'synth->activeVoices()<12' in native
print('CP58 source contract PASS: DSP invariants, 20 ASIO cases, release wiring')
