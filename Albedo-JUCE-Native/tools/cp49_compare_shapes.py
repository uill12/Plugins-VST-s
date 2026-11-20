"""Compare stereo 48 kHz renders of MIDI 48 and 67. Lower distance means more similar."""
from pathlib import Path
import numpy as np
from scipy.spatial.distance import cdist
from scipy.signal.windows import hann

def output_shape(path):
    audio=np.fromfile(path,dtype='<f4').reshape(-1,2)
    assert audio.shape==(115200,2) and np.isfinite(audio).all(),path
    # Separate energy envelopes and spectra. Measuring channel powers avoids phase cancellation.
    power=np.mean(audio.reshape(-1,480,2).astype(float)**2,axis=(1,2))
    peak=max(1e-18,power.max())
    env=np.maximum(-55,10*np.log10(np.maximum(power/peak,1e-14)))
    fft_size=2048
    window=hann(fft_size,sym=False)
    hz=np.fft.rfftfreq(fft_size,1/48000)
    edges=np.geomspace(35,17000,24)
    split=np.searchsorted(hz,edges)
    spec=[]
    for note in (0,57600):
        for start,end in [(0,3840),(3840,15360),(15360,33280),(33280,40960),(40960,57600)]:
            block=audio[note+start:note+end]
            frames=np.lib.stride_tricks.sliding_window_view(block,fft_size,axis=0)[::1024]
            psd=(abs(np.fft.rfft(frames*window,axis=-1))**2).mean(axis=(0,1))
            bands=np.array([psd[lo:max(lo+1,hi)].sum() for lo,hi in zip(split,split[1:])])
            spec.extend(np.maximum(-55,10*np.log10(np.maximum(bands,1e-18)/max(1e-18,bands.max()))))
    # 65% harmonic evolution, 35% attack/sustain/release in squared distance.
    return np.r_[np.asarray(spec)*np.sqrt(.65/len(spec)),env*np.sqrt(.35/len(env))]

def distances(folder):
    n=len(list(folder.glob('*.f32')))
    assert n in (128,192)
    return cdist(*[np.stack([output_shape(folder/f'{i}.f32') for i in range(n)])]*2)

if __name__=='__main__':
    import argparse,csv
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('renders',type=Path)
    parser.add_argument('csv',type=Path)
    args=parser.parse_args()
    d=distances(args.renders)
    with args.csv.open('w',newline='') as stream:
        writer=csv.writer(stream);writer.writerow(['slot_a','slot_b','output_shape_distance_db'])
        writer.writerows(sorted(((i,j,round(d[i,j],3)) for i in range(d.shape[0]) for j in range(i+1,d.shape[0])),key=lambda r:r[2]))
