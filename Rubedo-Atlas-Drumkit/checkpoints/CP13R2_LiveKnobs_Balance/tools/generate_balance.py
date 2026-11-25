"""CP13R2 controlled role/kit balance derived from real 96k PCM24 attack-energy tests.
Single gain per (kit,pad), not a destructive sample rewrite or per-hit automatic compressor.
Stat column 'rms' = averaged rms over first <=250ms of four source variants.
"""
from pathlib import Path
import pandas as pd
import numpy as np
import sys
src=Path(sys.argv[1]) if len(sys.argv)>1 else Path('rubedo_stats.csv'); data=pd.read_csv(src)
assert len(data)==24*24 and data.rms.min()>0
# Target preserves a distinct kit hierarchy: low frequencies anchor, backbeat present,
# hats lighter, long resonances restrained. No universal loudness target for all drums.
role=np.array([.92,.95,1.36,1.33,1.14,1.10,1.01,1.17,1.02,.94,.95,1.08,.92,1.16,1.13,1.00,.99,1.01,.98,1.05,.96,.96,1.18,.84])
median=data.groupby('pad').rms.median().to_numpy()
cal=np.empty((24,24));out=[]
for row in data.itertuples():
    f=(median[row.pad]/row.rms)**.44
    f=np.clip(f,.86,1.16)
    val=np.clip(role[row.pad]*f,.78,1.52)
    cal[row.kit,row.pad]=val
    out.append((row.kit,row.pad,row.rms,role[row.pad],f,val,row.rms*val))
full=pd.DataFrame(out,columns=['kit','pad','rms_250ms','role_trim','kit_trim','net_gain','rms_after']);full.to_csv(Path(sys.argv[2]) if len(sys.argv)>2 else Path('role_balance_qa.csv'),index=False)
# Header intentionally generated with fixed decimal precision: no math/library in audio callback.
lines=['/* CP13R2 — role-aware, kit-aware gain trims, computed from 2,304 original PCM24 regions. */',
       '/* Reproducible provenance: role_balance_qa.csv + generate_balance.py. */',
       'static const float rubedo_role_gain[RUBEDO_KITS][RUBEDO_PADS] = {']
for kit in range(24):
    lines.append('  {'+', '.join(f'{x:.6f}f' for x in cal[kit])+'},')
lines.append('};')
(Path(sys.argv[3]) if len(sys.argv)>3 else Path('source/rubedo_role_balance.inc')).write_text('\n'.join(lines)+'\n')
print('per-sample trim gain',cal.min(),cal.max(),'dynamic range changes')
for pad in [0,1,2,3,4,5,11,13,23]:
    group=full[full["pad"]==pad]
    print(f'{pad:2d}: raw_rms={group.rms_250ms.mean():.3f} ->{group.rms_after.mean():.3f}, trimmean={group.net_gain.mean():.3f}, kitspread={group.rms_after.max()/group.rms_after.min():.2f}')
print('role trim approx dB', np.around(20*np.log10(role),2))
