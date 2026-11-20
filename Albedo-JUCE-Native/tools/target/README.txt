Albedo CP51 — Reference Acquisition ONLY

Use Acquire_Reference.exe to select an isolated WAV note. For best character
extraction, select a dry tonal instrument without percussion, mastered effects,
or layered accompaniment. User-supplied references are analyzed locally.

Example:
  Acquire_Reference.exe "reference.wav" --artist "Reference" --track "Track" --instrument "tonal synthesizer"

Output:
  reference_Reference_Profile.json — frequency bands (sub, bass, low-mid,
  mid, upper-mid, presence, air), tonal estimate with confidence, spectral
  centroid/rolloff/flatness, dynamic envelope, RMS and peak, and stereo width.

No .albedopatch, synthesized WAV, waveform copies, preset matching or
selection, audio embedding, or parameter changes are produced.

The Morph Tree remains exclusively in the development-time sound-generation
pipeline and independently creates/compares the 192 factory presets.
This tool provides reference measurements for human-guided study, not
automatic reproduction of a recording or an artist's exact sound.

The old Match_Target entrypoint is deprecated and now aliases acquisition.
References should be lawfully supplied. Neither the uploaded source
recording nor any samples are included in the output JSON.

Requirements for Python source use: Python 3.12+, numpy, scipy.
