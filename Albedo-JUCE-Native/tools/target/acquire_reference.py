#!/usr/bin/env python3
"""Albedo CP51 — acquisition of isolated audio REFERENCES, not sound synthesis.

Converts a user-selected WAV into measured spectral/temporal/stereo descriptors
to inform a later OFFLINE Morph Tree generation pass. Does NOT load presets,
match a factory patch, render samples, export audio, or construct synth states.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import math
from pathlib import Path

import numpy as np
from scipy.io import wavfile
from scipy.signal import resample_poly

TARGET_RATE = 48000
BANDS = (
    ("sub", 25, 90), ("bass", 90, 220), ("low_mid", 220, 700),
    ("mid", 700, 2000), ("upper_mid", 2000, 5000),
    ("presence", 5000, 10000), ("air", 10000, 20000),
)

def load_wav(path: Path) -> tuple[np.ndarray, int, int]:
    rate, data = wavfile.read(path)
    if not (8000 <= int(rate) <= 192000):
        raise ValueError("Supported sample rates are 8 to 192 kHz.")
    if data.ndim not in (1, 2) or (data.ndim == 2 and data.shape[1] not in (1, 2)):
        raise ValueError("Provide one mono or stereo WAV.")
    channels = 1 if data.ndim == 1 else data.shape[1]
    if data.dtype.kind == "u":
        info = np.iinfo(data.dtype)
        mid = (info.max + 1) / 2
        samples = (data.astype(np.float64) - mid) / mid
    elif data.dtype.kind == "i":
        info = np.iinfo(data.dtype)
        samples = data.astype(np.float64) / max(abs(int(info.min)), int(info.max))
    elif data.dtype.kind == "f":
        samples = data.astype(np.float64)
    else:
        raise ValueError("Unsupported WAV encoding.")
    if not np.isfinite(samples).all():
        raise ValueError("Reference WAV has non-finite samples.")
    if samples.ndim == 1:
        samples = samples[:, None]
    duration = len(samples) / rate
    if not (0.13 <= duration <= 30):
        raise ValueError("Use one isolated reference event lasting 0.13–30 seconds.")
    if abs(samples).max() <= 1e-7:
        raise ValueError("Reference is silent.")
    divisor = math.gcd(int(rate), TARGET_RATE)
    samples = resample_poly(samples, TARGET_RATE // divisor,
                            rate // divisor, axis=0).astype(np.float64)
    return samples, int(rate), channels

def estimate_fundamental(x: np.ndarray) -> dict:
    """FFT autocorrelation: advisory only, never transposition or pitch edit."""
    if len(x) < 8192:
        x = np.pad(x, (0, 8192 - len(x)))
    # Search a representative steady window by 0.17 s energy.
    starts = np.linspace(0, max(0, len(x) - 8192), 7, dtype=int)
    pos = int(max(starts, key=lambda i: np.mean(x[i:i+8192] ** 2)))
    sample = x[pos:pos+8192] * np.hanning(8192)
    sample = sample - sample.mean()
    power = abs(np.fft.rfft(sample, n=16384)) ** 2
    ac = np.fft.irfft(power, n=16384)[:8192]
    if ac[0] < 1e-10:
        return {"frequency_hz": None, "midi_note_estimate": None,
                "confidence": 0, "method": "advisory_fft_autocorrelation"}
    ac /= ac[0]
    lo, hi = int(TARGET_RATE / 1400), min(len(ac)-1, int(TARGET_RATE / 35))
    selected = ac[lo:hi]
    idx = int(np.argmax(selected)) + lo
    confidence = float(np.clip(ac[idx], 0, 1))
    if confidence < 0.28:
        return {"frequency_hz": None, "midi_note_estimate": None,
                "confidence": round(confidence, 4),
                "method": "advisory_fft_autocorrelation"}
    hz = TARGET_RATE / idx
    note = round(69 + 12 * math.log2(hz / 440))
    return {"frequency_hz": round(hz, 2),
            "midi_note_estimate": int(max(0, min(127, note))),
            "confidence": round(confidence, 4),
            "method": "advisory_fft_autocorrelation"}

def summarize(path: Path, artist: str = "", track: str = "",
              instrument: str = "", notes: str = "") -> dict:
    samples, original_rate, channels = load_wav(path)
    if channels == 1:
        left = right = samples[:, 0]
    else:
        left, right = samples[:, 0], samples[:, 1]
    mid = (left + right) * .5
    side = (left - right) * .5
    peak = float(np.max(np.abs(samples)))
    rms = float(np.sqrt(np.mean(samples ** 2) + 1e-18))
    mid_energy = float(np.mean(mid**2) + 1e-18)
    side_energy = float(np.mean(side**2) + 1e-18)
    correlation = float(np.corrcoef(left,right)[0,1]) if channels == 2 and min(np.std(left),np.std(right)) > 1e-8 else 1.0
    # Temporal profile in 10ms frames; do not retain the waveform.
    window = 480
    cut = len(mid)//window*window
    env = np.sqrt(np.mean(mid[:cut].reshape(-1,window)**2,axis=1) + 1e-18)
    max_env = max(float(env.max()), 1e-12)
    env_normalized = env/max_env
    first_above = np.flatnonzero(env_normalized > .10)
    onset = int(first_above[0]) if len(first_above) else 0
    max_frame = int(np.argmax(env))
    attack_sec = max(0,(max_frame-onset)*.01)
    segment_count = 12
    seg = [float(np.mean(chunk)) if len(chunk) else 0.0
           for chunk in np.array_split(env_normalized,segment_count)]
    # Sample 24 temporal FFT windows evenly, to prevent long recordings
    # from overwhelming low-frequency or attack characteristics.
    fft = 8192
    lengths = np.pad(mid,(0,max(0,fft-len(mid))))
    positions = np.linspace(0,max(0,len(lengths)-fft),24,dtype=int)
    spectra = np.stack([abs(np.fft.rfft(lengths[i:i+fft] *
                          np.hanning(fft)))**2 for i in positions])
    spectrum = spectra.mean(axis=0)
    freqs = np.fft.rfftfreq(fft, 1/TARGET_RATE)
    band_energy = [float(spectrum[(freqs>=lo)&(freqs<hi)].sum())
                   for _,lo,hi in BANDS]
    denominator = max(sum(band_energy),1e-20)
    proportions = {name:round(100*e/denominator,3)
                   for (name,_,_),e in zip(BANDS,band_energy)}
    active = freqs > 25
    freq = freqs[active]
    power = spectrum[active]
    total = max(float(power.sum()),1e-20)
    centroid = float(np.dot(freq,power)/total)
    cumulative = np.cumsum(power)
    rolloff = float(freq[min(len(freq)-1,int(np.searchsorted(cumulative,cumulative[-1]*.85)))])
    flatness = float(np.exp(np.mean(np.log(power+1e-16))) / max(np.mean(power),1e-16))
    profile = {
        "format":"albedo.reference.acquisition.v1",
        "purpose":"Reference characterization for a later Morph Tree generation pass only",
        "source":{"filename":path.name,
                  "sha256":hashlib.sha256(path.read_bytes()).hexdigest(),
                  "original_sample_rate":original_rate,
                  "channels":channels,
                  "seconds":round(len(samples)/TARGET_RATE,3)},
        "reference_labels":{"artist":artist, "track":track,
                            "instrument_hint":instrument, "notes":notes},
        "frequency_distribution_percent":proportions,
        "spectral":{"centroid_hz":round(centroid,1),
                    "rolloff_85_hz":round(rolloff,1),
                    "flatness":round(flatness,5)},
        "temporal":{"rms_dbfs":round(20*math.log10(max(rms,1e-12)),2),
                    "peak_dbfs":round(20*math.log10(max(peak,1e-12)),2),
                    "crest_db":round(20*math.log10(max(peak/rms,1e-12)),2),
                    "attack_seconds_estimate":round(attack_sec,3),
                    "relative_energy_12_segments":[round(v,4) for v in seg]},
        "stereo":{"correlation":round(correlation,4),
                  "side_to_mid_db":round(10*math.log10(side_energy/mid_energy),2),
                  "mono_compatibility_warning":bool(correlation < -.15)},
        "pitch":estimate_fundamental(mid),
        "generation_boundary":{"creates_preset":False,
                               "exports_synth_patch":False,
                               "produces_reconstructed_audio":False,
                               "imports_audio_into_synth":False,
                               "retains_original_audio":False,
                               "workflow":"Reference acquisition only; sound generation is a separate Morph Tree pipeline."},
    }
    return profile

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("wav",nargs="?",type=Path,
                        help="A dry, isolated WAV reference (user-provided)")
    parser.add_argument("--out",type=Path,
                        help="JSON output file (default: WAV stem + '_Reference_Profile.json')")
    parser.add_argument("--artist",default="",help="Optional reference attribution/label")
    parser.add_argument("--track",default="",help="Optional track label")
    parser.add_argument("--instrument",default="",help="Optional instrument label")
    parser.add_argument("--notes",default="",help="Optional creative notes")
    args = parser.parse_args(argv)
    if args.wav is None:
        from tkinter import Tk,filedialog
        root = Tk();root.withdraw()
        name=filedialog.askopenfilename(title="Select an isolated reference WAV",
                                         filetypes=[("WAV", "*.wav")])
        root.destroy()
        if not name:return 0
        args.wav=Path(name)
    result=summarize(args.wav,args.artist,args.track,args.instrument,args.notes)
    out=args.out or args.wav.with_name(args.wav.stem+"_Reference_Profile.json")
    out.parent.mkdir(parents=True,exist_ok=True)
    out.write_text(json.dumps(result,ensure_ascii=False,indent=2)+"\n",encoding="utf8")
    print("Saved reference descriptors only:",out)
    return 0

if __name__=="__main__":
    raise SystemExit(main())
