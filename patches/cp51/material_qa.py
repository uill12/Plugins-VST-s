#!/usr/bin/env python3
"""CP51 96 kHz PCM24 material QA: bass/body and stereo are measured, not inferred."""
from pathlib import Path
import json,sys,struct
import numpy as np
p=Path(sys.argv[1]);data=p.read_bytes()
assert data[:8]==b"VTRB2\0\0\0","Expected enriched 24-bit mid/side bank"
count,sr,n=struct.unpack("<III",data[8:20])
assert count==32 and sr==96000 and n>4096,(count,sr,n)
raw=np.frombuffer(data,dtype=np.uint8,offset=20)
assert len(raw)==count*2*n*3,(len(raw),count,n)
frame=raw.reshape(count,2,n,3).astype(np.int32)
pcm=frame[...,0]|(frame[...,1]<<8)|(frame[...,2]<<16)
pcm=(pcm^0x800000)-0x800000
x=pcm.astype(np.float64)/8388608.
hz=np.fft.rfftfreq(n,1/sr)
rows=[]
for i in range(count):
    mid,side=x[i]
    peak=max(np.max(abs(mid+side)),np.max(abs(mid-side)))
    total=float(np.mean(mid*mid))
    side_power=float(np.mean(side*side))
    power=abs(np.fft.rfft(mid*np.hanning(n)))**2
    freqs=[(35,180),(180,450),(450,1600),(1600,7000),(7000,19000)]
    bands=[float(power[(hz>=lo)&(hz<hi)].sum()) for lo,hi in freqs]
    fractions=np.array(bands)/max(sum(bands),1e-18)
    assert np.isfinite(x[i]).all() and .0001<peak<=.901, (i,peak)
    assert total>1e-8, ("silent material",i)
    assert side_power<=total*.80, ("stereo side overpowered the source",i)
    rows.append(dict(index=i,peak=round(float(peak),5),rms=round(float(np.sqrt(total)),5),
        side_to_mid_db=round(float(10*np.log10((side_power+1e-18)/(total+1e-18))),2),
        band_percent=[round(float(v*100),3) for v in fractions]))
# The source pool must span very different spectral categories; *each* sample
# is not required to have as much bass as a kick or a long-pad oscillator.
assert len({tuple(np.asarray(r["band_percent"]).argmax().reshape(1)) for r in rows})>=3,"Low spectral diversity"
report=dict(bank=str(p),format="PCM24 96k mid/side",materials=count,
            frequency_bands_hz=["35-180","180-450","450-1600","1600-7000","7000-19000"],
            checks=["finite","-0.9..0.9 peak","mid louder than side","nonzero body","diverse dominant bands"],
            rows=rows)
out=Path(sys.argv[2]);out.parent.mkdir(parents=True,exist_ok=True)
out.write_text(json.dumps(report,indent=1)+"\n",encoding="utf8")
print("CP51 original-source spectral enrichment PASS",count,"materials; dominant-band classes",
      len({np.argmax(r["band_percent"]) for r in rows}))
