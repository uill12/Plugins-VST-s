"""Expand the original mid channel into deterministic 96 kHz PCM24 mid/side materials."""
import struct,sys
from pathlib import Path
import numpy as np
from scipy.signal import butter,sosfilt
p=Path(sys.argv[1]);data=p.read_bytes()
assert data[:8]==b'VTRB1\0\0\0'
count,sr,n=struct.unpack('<III',data[8:20])
raw=np.frombuffer(data[20:],np.uint8).reshape(count,n,3).astype(np.int32)
x=(raw[:,:,0]|raw[:,:,1]<<8|raw[:,:,2]<<16)
x=((x^0x800000)-0x800000)/8388608.
def pack(x):
    v=np.rint(np.clip(x,-.999999,.999999)*8388607).astype(np.int32)
    return np.stack([v&255,v>>8&255,v>>16&255],axis=-1).astype('u1').tobytes()
out=bytearray(b'VTRB2\0\0\0'+data[8:20])
for i,original in enumerate(x):
    # CP51: oversampled gentle harmonic enrichment, band-controlled low-mid body.
    # PCM24 96k source stays untouched in the input; no external copyrighted audio.
    # Oversample the NONLINEAR stage before bringing it back to 96 kHz.
    from scipy.signal import resample_poly
    up=resample_poly(original,4,1,window=('kaiser',8.6))
    harmonic=resample_poly(np.tanh(up*(1.30+.12*(i%5))),1,4,window=('kaiser',8.6))[:n]
    lowmid=sosfilt(butter(3,(88,370+i%7*30),fs=sr,btype='bandpass',output='sos'),original)
    mid=.79*original+.21*harmonic+.10*lowmid
    # Widen only the upper harmonics; sub and bass fundamentals remain mono-safe.
    upper=sosfilt(butter(3,300+i%7*24,fs=sr,btype='highpass',output='sos'),mid)
    side=np.zeros_like(mid)
    for ms,g in [(4.7+i%3,.52),(10.3+i%7,-.39),(19.1+i%11,.26)]:
        dt=round(ms*.001*sr);side[dt:]+=g*upper[:-dt]
    taper=np.minimum(1,np.arange(n)/144)*np.minimum(1,np.arange(n-1,-1,-1)/288)
    mid*=taper;side*=taper
    peak=max(np.max(abs(mid+side)),np.max(abs(mid-side)))
    gain=.89/max(peak,1e-6)
    out.extend(pack(mid*gain));out.extend(pack(side*gain))
p.write_bytes(out)
