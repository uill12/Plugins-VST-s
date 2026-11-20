Albedo CP50 | Reproduce a reference note

Open Match_Target.exe and select an isolated mono or stereo WAV note, up to eight seconds. Keep AlbedoTargetRenderer.exe and recipes.json beside the matcher. Use a dry recording when you want to reproduce the instrument itself.

For a known performance, give the MIDI note, velocity (0 to 1), and seconds until note-off. Example:
Match_Target.exe "reference.wav" --note 57 --velocity 0.8 --hold 0.65

The matcher compares the envelope, evolving spectrum and stereo width against all 128 rendered instruments. It refines the closest two by changing the synth parameters, then matches level. Tonal inputs support automatic pitch estimation. For percussive or ambiguous targets use --note. The default hold estimate is 72 percent of the clip.

In the output folder, import Reconstructed.albedopatch using Albedo's IMPORT button. Play the note, velocity, hold time and tempo in Recipe.json. Reconstructed.wav is the synthesized result. Recipe.json lists the reference traits, source layers, transformations and fitted control settings. The patch contains the complete parameter state.

This is a single-note instrument matching tool. Different oscillator, filter and effect combinations can produce similar waveforms. The result is an approximation using Albedo's synthesis resources.
