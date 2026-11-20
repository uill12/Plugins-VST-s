from pathlib import Path
import json,re,sys
import numpy as np
from scipy.spatial.distance import cdist
root=Path(__file__).resolve().parents[2];native=root/'Albedo-JUCE-Native'
sys.path.insert(0,str(native/'tools'))
from cp49_compare_shapes import output_shape
folder=Path(sys.argv[1]);options=sorted(x for x in folder.iterdir() if x.name.isdigit());k=len(options)
features=np.array([[output_shape(p/'audio'/f'{i}.f32') for p in options] for i in range(128)])
D=cdist(features.reshape(128*k,-1),features.reshape(128*k,-1)).reshape(128,k,128,k)
rng=np.random.default_rng(599)
best=None
for start in range(12):
    sel=rng.integers(0,k,128)
    for sweep in range(30):
        changed=False
        for i in rng.permutation(128):
            scores=D[i,:,np.arange(128),sel].T.copy();scores[:,i]=1000
            near=np.sort(scores,axis=1)[:,:8]
            cost=np.sum(np.exp(-1.4*(near-6)),axis=1)
            idx=cost.argmin()
            if idx!=sel[i]:sel[i]=idx;changed=True
        if not changed:break
    ds=D[np.arange(128)[:,None],sel[:,None],np.arange(128)[None,:],sel[None,:]][np.triu_indices(128,1)]
    key=(round(ds.min(),2),np.percentile(ds,5))
    if best is None or key>best[0]:best=(key,sel.copy(),ds.copy())
sel=best[1];print('minimum, median, under six:',best[2].min(),np.median(best[2]),sum(best[2]<6))
names=['material_factory_bank.json','recursive_forms_bank.json']
banks=[sum([json.loads((p/n).read_text()) for n in names],[]) for p in options]
rows=[banks[sel[i]][i] for i in range(128)]
trim=[float(x) for x in re.findall(r'(-?[0-9.]+)f',(native/'Source/AlbedoProgramTrim.hpp').read_text())]
out=folder/'selected';out.mkdir(exist_ok=True)
for i,p in enumerate(rows):
    a=np.fromfile(options[sel[i]]/'audio'/f'{i}.f32',np.float32)
    assert np.isfinite(a).all() and np.max(abs(a))>.0001
    power=np.mean(a.reshape(-1,960).astype(float)**2,axis=1)
    rms=10*np.log10(np.mean(power[power>power.max()*.1]))
    peak=20*np.log10(abs(a).max())
    db=p['overrides']['9000']
    trim[i]=float(np.clip(trim[i]+min(-22-rms,-3-peak),-36-db,12-db))
    if i>=64:p['character']=' · '.join(p['ideaPath'])
for n,b in zip(names,(rows[:64],rows[64:])):(out/n).write_text(json.dumps(b,indent=1,ensure_ascii=False)+'\n')
(out/'AlbedoProgramTrim.hpp').write_text('#pragma once\n#include <array>\ninline constexpr std::array<float,128> albedo_program_trim{{\n'+',\n'.join(f'    {v:.4f}f' for v in trim)+'\n}};\n')
