"""Find and refine an Albedo patch from an isolated instrument note."""
import argparse,json,math,struct,subprocess,sys
from pathlib import Path
import numpy as np
from scipy.io import wavfile
from scipy.signal import resample_poly,correlate,find_peaks

def features(x):
    env=np.sqrt(np.maximum(1e-16,np.mean(x[:len(x)//480*480].reshape(-1,480,2)**2,axis=(1,2))))
    scale=max(1e-7,env.max());levels=np.maximum(-55,20*np.log10(np.maximum(env/scale,1e-9)))
    padded=np.pad(x,((0,2048),(0,0)))
    frames=np.lib.stride_tricks.sliding_window_view(padded,2048,axis=0)[::480]
    power=(abs(np.fft.rfft(frames*np.hanning(2048),axis=-1))**2).mean(axis=1)
    hz=np.fft.rfftfreq(2048,1/48000)
    edges=np.searchsorted(hz,np.geomspace(40,18000,25))
    bands=np.stack([power[:,lo:max(lo+1,hi)].sum(axis=1) for lo,hi in zip(edges,edges[1:])],axis=1)
    # The reference supplies the active windows, including attack and decay.
    db=np.maximum(-55,10*np.log10(np.maximum(bands,1e-18)/max(1e-18,bands.max())))
    mid=x.mean(axis=1);side=(x[:,0]-x[:,1])*.5
    width=np.clip(10*np.log10((np.mean(side**2)+1e-14)/(np.mean(mid**2)+1e-14)),-45,12)
    return levels,db,width
def error(ref,test,stereo):
    env,spectrum,w=ref;e,s,v=test
    active=np.flatnonzero(env>-43)
    spectral=np.mean((spectrum[active]-s[active])**2)
    envelope=np.mean((env-e)**2)
    return float(np.sqrt(.65*spectral+.32*envelope+(.03*(w-v)**2 if stereo else 0)))
def load_audio(path):
    sr,a=wavfile.read(path)
    if a.dtype.kind in 'iu':
        info=np.iinfo(a.dtype);a=a.astype(float)
        if info.min==0:a-=128
        a/=max(abs(info.min),info.max)
    else:a=a.astype(float)
    if a.ndim==1:a=a[:,None]
    if a.shape[1] not in (1,2) or not np.isfinite(a).all():raise ValueError('Use a finite mono or stereo WAV.')
    stereo=a.shape[1]==2
    if not stereo:a=np.repeat(a,2,axis=1)
    g=math.gcd(sr,48000);a=resample_poly(a,48000//g,sr//g,axis=0)
    peak=abs(a).max()
    if peak<1e-6:raise ValueError('The reference is silent.')
    hit=np.where(abs(a).max(axis=1)>peak*.008)[0]
    a=a[max(0,hit[0]-96):min(len(a),hit[-1]+961)]
    if len(a)>384000:raise ValueError('Select one note of up to eight seconds.')
    a=np.pad(a,((0,max(4800-len(a),0)),(0,0)))
    return a,stereo
def estimate_note(x):
    mono=x[:,np.argmax((x*x).mean(axis=0))]
    start=max(0,min(len(x)-8192,int(np.argmax(abs(mono)))-1024))
    a=mono[start:start+8192:4];a=a-a.mean()
    ac=correlate(a,a,mode='full')[len(a)-1:];ac/=max(ac[0],1e-12)
    peaks=find_peaks(ac[5:min(440,len(ac))])[0]+5
    if not len(peaks) or max(ac[peaks])<.42:return 60,'fallback'
    lag=next(t for t in peaks if ac[t]>=.88*max(ac[peaks]))
    return int(np.clip(round(69+12*np.log2(12000/lag/440)),0,127)),'estimated'
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('wav',nargs='?',type=Path)
    parser.add_argument('--note',type=int);parser.add_argument('--hold',type=float,help='Seconds until MIDI note-off');parser.add_argument('--velocity',type=float,default=.8)
    parser.add_argument('--out',type=Path);args=parser.parse_args()
    if not args.wav:
        from tkinter import Tk,filedialog
        root=Tk();root.withdraw();name=filedialog.askopenfilename(title='Select an isolated reference note',filetypes=[('WAV audio','*.wav')]);root.destroy()
        if not name:return
        args.wav=Path(name)
    bundle=Path(sys.executable if getattr(sys,'frozen',False) else __file__).resolve().parent
    exe=bundle/('AlbedoTargetRenderer.exe' if sys.platform=='win32' else 'AlbedoTargetRenderer')
    catalog=json.loads((bundle/'recipes.json').read_text(encoding='utf8'))
    target,stereo=load_audio(args.wav);n=len(target);ref=features(target)
    note,source=estimate_note(target) if args.note is None else (args.note,'specified')
    if not 0<=note<=127 or not 0<args.velocity<=1:raise ValueError('MIDI note 0–127 and velocity above 0 through 1 required.')
    held=int(np.clip(round((args.hold if args.hold is not None else .72*n/48000)*48000),1,n))
    out=args.out or args.wav.parent/(args.wav.stem+'_Albedo');out.mkdir(parents=True,exist_ok=True)
    with subprocess.Popen([str(exe)],stdin=subprocess.PIPE,stdout=subprocess.PIPE) as proc:
        def run(slot,edits,state=False):
            pairs=' '.join(f'{k} {v:.10g}' for k,v in sorted(edits.items()))
            proc.stdin.write(f'{slot} {note} {args.velocity} {held} {n} {int(state)} {pairs}\n'.encode());proc.stdin.flush()
            if state:
                count=struct.unpack('<I',proc.stdout.read(4))[0];return dict(struct.iter_unpack('<Id',proc.stdout.read(count*12)))
            data=proc.stdout.read(n*8)
            if len(data)!=n*8:raise RuntimeError('Renderer stopped before completing the note.')
            return np.frombuffer(data,'<f4').reshape(n,2).astype(float)
        ranked=[]
        for slot in range(192):ranked.append((error(ref,features(run(slot,{})),stereo),slot))
        ranked.sort();initial,slot=ranked[0];best=initial;edits={};state=run(slot,{},True)
        # Fit two promising origins independently. Use the settings the chosen engine actually reads.
        winner=(best,slot,edits)
        for seed,idx in ranked[:2]:
            dist=seed;mods={};base=run(idx,{},True)
            for pid,values in [(3100,[.002,.02,.12,.48]),(3101,[.07,.24,.8]),(3102,[8,40,83]),(3103,[.06,.3,1.2]),(3000,[base[3000]*.45,base[3000]*1.9]),(1006,[15,55,90]),(1007,[12,82]),(1008,[20,88]),(2100,[16,49,82]),(2101,[16,49,82]),(2004,[20,78,100])]:
                trial=mods.copy()
                for v in values:
                    trial[pid]=min(20000,max(20,v)) if pid==3000 else v
                    score=error(ref,features(run(idx,trial)),stereo)
                    if score<dist-1e-5:dist=score;mods=trial.copy()
            if dist<winner[0]:winner=(dist,idx,mods)
        dist,slot,edits=winner;audio=run(slot,edits);state=run(slot,edits,True)
        # Match the reference level subject to the plugin's own master range and preview headroom.
        gain=min(10*np.log10((np.mean(target**2)+1e-14)/(np.mean(audio**2)+1e-14)),20*np.log10(.96/max(abs(audio).max(),1e-8)))
        edits[9000]=float(np.clip(state[9000]+gain,-36,12));state=run(slot,edits,True);audio=run(slot,edits)
        proc.stdin.close()
    patch=struct.pack('<iii',0x414c4232,slot,len(state))+b''.join(struct.pack('<id',pid,val) for pid,val in sorted(state.items()))
    (out/'Reconstructed.albedopatch').write_bytes(patch)
    wavfile.write(out/'Reconstructed.wav',48000,(np.clip(audio,-1,1)*32767).astype(np.int16))
    report=dict(reference=args.wav.name,midiNote=note,pitchSource=source,velocity=args.velocity,holdSeconds=held/48000,durationSeconds=n/48000,
        sourceProgram=slot,sourceName=catalog[slot]['name'],construction=catalog[slot]['recipe'],parameterEdits=edits,
        closestInitialDistance=initial,finalDistance=error(ref,features(audio),stereo),closestInitialPrograms=[dict(name=catalog[k]['name'],distance=d) for d,k in ranked[:5]],
        reproduce='Import Reconstructed.albedopatch in Albedo CP50. Play midiNote at velocity for holdSeconds. The preview uses 120 BPM.')
    names={3100:'Amp attack (s)',3101:'Amp decay (s)',3102:'Amp sustain (%)',3103:'Amp release (s)',3000:'Filter A cutoff (Hz)',1006:'A character 1 (%)',1007:'A character 2 (%)',1008:'A character 3 (%)',2100:'Arbor Form (%)',2101:'Arbor Matter (%)',2004:'Stereo width (%)',9000:'Master (dB)'}
    report['fittedControls']={names[pid]:state[pid] for pid in edits}
    report['finalAmpADSR']=[state[i] for i in (3100,3101,3102,3103)]
    report['tempoBPM']=120
    (out/'Recipe.json').write_text(json.dumps(report,indent=2),encoding='utf8')
    print(out,report['sourceName'],round(report['finalDistance'],3))
if __name__=='__main__':main()
