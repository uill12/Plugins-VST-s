"""Round-trip an unknown-to-the-matcher edited patch through Windows binaries."""
from pathlib import Path
import subprocess,sys,tempfile,struct,json
import numpy as np
from scipy.io.wavfile import read,write
bundle=Path(sys.argv[1]).resolve()
def render(slot,edits):
    line=f"{slot} 57 0.8 21000 48000 0 "+' '.join(f'{k} {v:.10g}' for k,v in edits.items())+'\n'
    data=subprocess.check_output([bundle/'AlbedoTargetRenderer.exe'],input=line.encode())
    return np.frombuffer(data,np.float32).reshape(-1,2)
with tempfile.TemporaryDirectory() as directory:
    root=Path(directory);wav=root/'target.wav';out=root/'results'
    write(wav,48000,(render(17,{3000:920,3100:.035})*32767).astype(np.int16))
    subprocess.run([bundle/'Match_Target.exe',str(wav),'--note','57','--hold','.4375','--out',str(out)],check=True,timeout=480)
    data=(out/'Reconstructed.albedopatch').read_bytes()
    magic,slot,n=struct.unpack('<iii',data[:12])
    assert magic==0x414c4232 and 0<=slot<128 and len(data)==12+12*n
    settings=dict(struct.iter_unpack('<id',data[12:]))
    report=json.loads((out/'Recipe.json').read_text())
    assert report['finalDistance']<=report['closestInitialDistance']+.1
    assert report['construction']['layers'] and report['construction']['allowedTransformations']
    sr,saved=read(out/'Reconstructed.wav');assert sr==48000
    replay=render(slot,settings)[:len(saved)]
    assert np.max(np.abs(replay-saved/32767.))<.0002
