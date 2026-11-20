#!/usr/bin/env python3
"""Acquire_Reference runtime tests: no synthesized output, no preset matching."""
from pathlib import Path
import importlib.util, json, math, subprocess, sys, tempfile
import numpy as np
from scipy.io import wavfile
root=Path(__file__).resolve().parents[1]
script=root/"tools/target/acquire_reference.py"
spec=importlib.util.spec_from_file_location("acquire_reference",script)
mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
with tempfile.TemporaryDirectory() as td:
    folder=Path(td);sr=96000
    t=np.arange(sr*2,dtype=float)/sr
    bass=(.34*np.sin(2*np.pi*55*t)+.15*np.sin(2*np.pi*110*t))
    air=(.25*np.sin(2*np.pi*1760*t)+.08*np.sin(2*np.pi*3520*t))
    samples=[("bass.wav",bass),("bright.wav",air)]
    profiles=[]
    for name,signal in samples:
        file=folder/name
        wavfile.write(file,sr,(signal*32767).astype(np.int16))
        output=folder/(name+".json")
        subprocess.run([sys.executable,str(script),str(file),
                        "--out",str(output),"--artist","reference","--instrument","tonal"],
                       check=True,capture_output=True)
        p=json.loads(output.read_text(encoding="utf8"))
        profiles.append(p)
        assert p["format"]=="albedo.reference.acquisition.v1"
        assert p["source"]["channels"]==1
        assert p["source"]["original_sample_rate"]==96000
        assert p["generation_boundary"]["creates_preset"] is False
        assert p["generation_boundary"]["produces_reconstructed_audio"] is False
        assert abs(sum(p["frequency_distribution_percent"].values())-100)<.5
        assert p["reference_labels"]["instrument_hint"]=="tonal"
        assert len(p["temporal"]["relative_energy_12_segments"])==12
        assert p["temporal"]["peak_dbfs"]<0
        assert p["pitch"]["method"]=="advisory_fft_autocorrelation"
    b,a=profiles
    assert b["frequency_distribution_percent"]["sub"]+b["frequency_distribution_percent"]["bass"] > 85
    assert a["frequency_distribution_percent"]["mid"] > 65
    assert b["spectral"]["centroid_hz"] < a["spectral"]["centroid_hz"]/3
    assert b["stereo"]["side_to_mid_db"] < -90
    assert len(list(folder.glob("*.wav")))==2, "Do not create rendered/derived WAVs"
    assert not list(folder.glob("*.albedopatch")), "Do not export patches"
    assert not list(folder.glob("*.nexopreset")), "Do not export presets"
src=script.read_text(encoding="utf8")
for banned in ("AlbedoTargetRenderer", "Reconstructed.albedopatch", "Reconstructed.wav",
               "match_target", "factory_bank", "setParam(", "presetFromReference"):
    assert banned not in src,(banned,"reference acquisition depends on DSP/reconstruction")
print("CP51 reference acquisition PASS: rich bass/bright spectral separation, pitch advisory, descriptors only, zero generated sounds")
