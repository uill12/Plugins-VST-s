# GUI reference-to-native: release gate checklist

Build / project / date / host / format: ____________________

- [ ] Original reference archived, versioned and hashed
- [ ] Static decoration separated from dynamic labels and controls
- [ ] Control registry: every clickable affordance has an action, parameter/state owner and test
- [ ] Geometry derived from one layout specification: paint and hit-test agree at all supported scales
- [ ] HTML reference prototype approved and recorded as design lock
- [ ] Native screenshot parity at default/min/max size and 100/125/150/200% DPI
- [ ] No overlapping or stale labels, debug UI, dead hit regions, or invisible focus targets
- [ ] Knobs, buttons, popup controls, sequencer and pad gestures function correctly
- [ ] No GUI thread touching live DSP voices unsafely; bounded event queues tested
- [ ] CLAP/VST3 exports, native DLL entry point, resource offsets and ABI structures verified
- [ ] Default and custom MIDI maps trigger the same pad, note label, selected sound and highlight
- [ ] MIDI Learn/current target/conflicts/cancel/unmapped note cases tested
- [ ] Saved preset state and note map survive close/reopen, including migration from prior checkpoint
- [ ] Host automation and dirty-state notifications correct for each advertised parameter
- [ ] 44.1/48/88.2/96 kHz; variable block sizes; polyphony and clipping/NaN checks
- [ ] Windows Bitwig scan, instantiate, GUI open/close, preset switch, resize and multiple instances tested
- [ ] Distinguish host-tested CLAP from host-tested VST3; do not imply untested format is certified
- [ ] Previous host-confirmed build preserved as rollback
- [ ] Artifact has version/date, source, release notes, checksums and explicit limitations
- [ ] Native split 7z volumes verified through full extraction and Drive readback

**Evidence:** screenshots, logs, unit-test output, plugin SHA-256, archive SHA-256, host version and reviewer.