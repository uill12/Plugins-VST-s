#!/usr/bin/env python3
"""CP59 WaveEdit 64x256 table selection and concept-morphed granular material.
Reads a CC0 WaveEdit repository checkout, validates 24 original WAV SHA-256,
outputs compact, mip-bandlimited ALBWT59 sample resource. Zero network/audio-thread IO.
"""
from pathlib import Path
import argparse, json,hashlib,struct,wave
import numpy as np
p=argparse.ArgumentParser()
p.add_argument('--samples',type=Path,required=True)
p.add_argument('--selection',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
p.add_argument('--report',type=Path)
a=p.parse_args()
manifest=json.loads(a.selection.read_text(encoding='utf-8'))
entries=manifest['selected'];assert len(entries)==24
waves=[];audit=[]
for i,d in enumerate(entries):
 f=a.samples/d['file'];raw=f.read_bytes();assert hashlib.sha256(raw).hexdigest()==d['sha256'],f'Mismatched source: {f}'
 with wave.open(str(f),'rb') as wf:
  assert (wf.getnchannels(),wf.getsampwidth(),wf.getframerate(),wf.getnframes())==(1,2,44100,16384),f
  samples=np.frombuffer(wf.readframes(wf.getnframes()),dtype='<i2').astype(np.float64).reshape(64,256)/32768.
 samples-=samples.mean(axis=1,keepdims=True)
 # CP59: remove periodic boundary steps before mip FFT/grain generation.
 # A 256-sample linear endpoint correction is pitch-neutral and prevents
 # single-cycle clicks at oscillator phase wrap.
 samples-=(samples[:,-1]-samples[:,0])[:,None]*np.linspace(0.,1.,256)[None,:]
 peak=np.max(np.abs(samples))
 assert peak>.06
 # Preserve relative dynamics of all 64 frames, avoid preset gain spikes.
 samples=np.clip(samples*(.82/peak),-1,1)
 waves.append(samples)
 audit.append({'index':i,'file':d['file'],'family':d['family'],'role':d['role'],'sha256':d['sha256'],'peak':round(float(peak),5),'tail_seam':round(float(np.max(np.abs(samples[:,0]-samples[:,-1]))),4)})
# Original 64 frame morph bank, eight anti-aliasing mipmaps with spectral truncation.
with a.output.open('wb') as o:
 o.write(struct.pack('<8sIIIII',b'ALBWT59\0',24,64,256,8,12288))
 for w in waves:
  frequency=np.fft.rfft(w,axis=1)
  for mip in range(8):
   allowed=max(1,128>>mip)
   bins=frequency.copy();bins[:,allowed+1:]=0
   y=np.fft.irfft(bins,n=256,axis=1)
   o.write(np.rint(np.clip(y,-1,1)*32767).astype('<i2').tobytes())
 # One evolving, *periodic* texture per table. 48 cycles/12288 samples.
 # Every cycle has exact musical pitch; adjacent frames plus a similar-sibling
 # wavetable are morphed without time stretching or artificial pitch jitter.
 for i,w in enumerate(waves):
  sibling=waves[int(entries[i]['morph_with'])]
  tx=np.zeros((48,256),dtype=np.float64)
  for cycle in range(48):
   frac=cycle/48.
   frame=frac*63
   k=min(62,int(frame));t=frame-k
   src=w[k]*(1-t)+w[k+1]*t
   other=sibling[k]*(1-t)+sibling[k+1]*t
   mix=.12+.23*(.5-.5*np.cos(2*np.pi*frac))
   tx[cycle]=src*(1-mix)+other*mix
  # Apply circular seam smoothing via 3-cycle crossfade to the opening.
  for k in range(3):
   t=(k+1)/4.
   tx[-3+k]=tx[-3+k]*(1-t)+tx[k]*t
  txx=tx.ravel();txx-=txx.mean();peak=max(.0001,np.max(np.abs(txx)))
  txx*=.76/peak
  o.write(np.rint(np.clip(txx,-1,1)*32767).astype('<i2').tobytes())
expected=28+2*(24*8*64*256+24*12288)
assert a.output.stat().st_size==expected,(a.output.stat().st_size,expected)
report={'version':'CP59','source':'CC0 1.0 public WaveEdit Online','evaluated':705,'selected':audit,'selected_count':24,'morph_grains':24,'frames_per_bank':64,'samples_per_frame':256,'mip_levels':8,'grain_cycles':48,'grain_samples':12288,'resource_bytes':expected,'resource_sha256':hashlib.sha256(a.output.read_bytes()).hexdigest()}
if a.report:a.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('CP59 WAVETABLE + GRAIN RESOURCE PASS',report['resource_bytes'],'bytes',report['resource_sha256'])