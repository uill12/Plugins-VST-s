"""Balance a fixed, role-specific performance and retain the before/after measurements."""
from pathlib import Path
import sys, json, hashlib
import numpy as np
from scipy.io.wavfile import write
root=Path("Albedo-JUCE-Native")
dest=Path("dist-cp48-qa")
dest.mkdir(exist_ok=True)
rows=sum((json.loads((root/"Metadata"/n).read_text(encoding="utf8")) for n in ("material_factory_bank.json","recursive_forms_bank.json")),[])
targets={"Bass":-20,"Lead":-21,"Pad":-23,"Keys":-22,"Percussion":-21,"Pluck":-22,"Drone":-24,"Texture":-24}
def measure(stage, index):
    audio=np.fromfile(Path("audio")/stage/f"{index}.f32",dtype="<f4").reshape(-1,2)
    assert np.isfinite(audio).all() and len(audio)==124800
    blocks=audio[:124800].reshape(-1,4800,2).astype(float)
    power=np.mean(blocks**2,axis=(1,2))
    active=10*np.log10(max(1e-15,np.percentile(power,85)))
    peak=20*np.log10(max(1e-10,np.max(abs(audio))))
    return audio,round(float(active),3),round(float(peak),3)
if sys.argv[1]=="trim":
    trims=[]
    for i,p in enumerate(rows):
        _,level,peak=measure("raw",i)
        trims.append(float(np.clip(min(targets[p["role"]]-level,-2.5-peak),-15,12)))
    header="#pragma once\n#include <array>\ninline constexpr std::array<float,128> albedo_program_trim{{\n"+",\n".join(f"    {v:.4f}f" for v in trims)+"\n}};\n"
    for target in [root/"Source"/"AlbedoProgramTrim.hpp",dest/"AlbedoProgramTrim.hpp"]:
        target.write_text(header,encoding="ascii")
else:
    records=[]
    clips=[]
    fingerprints=set()
    for i,p in enumerate(rows):
        old,before,oldpeak=measure("old",i)
        final,after,peak=measure("final",i)
        digest=hashlib.sha256(final.tobytes()).hexdigest()
        assert not np.array_equal(old,final) and digest not in fingerprints and peak<=0.001 and after>-85,(i,after,peak)
        fingerprints.add(digest)
        records.append(dict(slot=i,name=p["name"],role=p["role"],old_active_dBFS=before,new_active_dBFS=after,peak_dBFS=peak,target_dBFS=targets[p["role"]]))
        if i in [0,18,39,64,77,98,123]:
            for a in [old,final]:
                a=a.copy()
                a[-960:]*=np.linspace(1,0,960)[:,None]
                clips.extend([a,np.zeros((12000,2))])
    write(dest/"Albedo_CP48_AB.wav",48000,(np.clip(np.concatenate(clips),-1,1)*32767).astype(np.int16))
    (dest/"CP48_Audition.json").write_text(json.dumps(records,indent=2),encoding="utf8")
