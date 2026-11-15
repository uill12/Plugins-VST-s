# Rubedo Atlas Drumkit — CP10 MIDI MapSync

**Build:** `0.9.5-cp10` — Windows x64 CLAP only. Date: 2026-11-15. **Not yet tested in Bitwig Windows.**

## User report

CP09's GUI MIDI pad section did not consistently match the notes being played. Specific regressions were verified in the code:

1. The Extended 8 page reused the baked first-16 pad-card labels and indices rather than displaying pads 17–24.
2. MIDI Learn could remain armed for an old pad after the user clicked another pad or switched Core/Extended page.
3. The baked `MIDI CONNECT` button could silently arm the Learn state although the host, not the CLAP plugin, routes the MIDI device.
4. Individual pad cards did not display their actual note assignments; the only dynamic mapping was in a small selected-voice field.
5. The host was not notified that learning a note changed session state, risking unsaved maps.
6. Legacy saved-state mappings were loaded without deduplication; old collisions could make pads unreachable.

## Fixes

- **Single MIDI map:** 24 atomic mapping entries, used by both the incoming note resolver and per-pad dynamic MIDI labels; defaults remain MIDI notes 36–59.
- **All 24 pad labels:** Core shows pads 01–16; Extended shows pads 17–24 and eight clearly disabled positions. GUI numbering, computer-key hints and MIDI note numbers derive from the actual pad index, not the static background.
- **Learn binding:** while the Learn mode is armed, choosing a different GUI pad or switching pages updates the target; the following MIDI note is assigned to that pad. Learn automatically disarms after assignment and can be canceled with Esc.
- **Duplicate avoidance:** learning a note already assigned to another pad swaps the two note assignments; importing malformed CP08/CP09 state repairs duplicates deterministically. Old state format (v1/v2/v3) and CLAP plugin ID stay compatible.
- **Host state update:** requests a CLAP host main-thread callback after a learned assignment and calls the host-state `mark_dirty` extension, when supported, so the project knows to save it.
- **Unmapped feedback:** incoming MIDI notes without a pad display `UNMAPPED MIDI <number>` temporarily; `MIDI CONNECT` now explains that MIDI routing is managed in Bitwig rather than silently arming learn.
- **State kit consistency:** explicit saved kit field is restored as the authoritative kit when loading legacy sessions.
- **Sound bank:** identical PCM24 and approved backdrop bytes, 24 kits ×24 pads ×4 articulations, no sound design changes.

## Verified offline tests

- `test_midi_map10`: defaults 36–59, Core/Extended boundary, last pad (MIDI 59), exact note routing, conflicting assignment swaps, successful learning of unused MIDI note, unmapped-note feedback, duplicate repair, audition and finite 88.2 kHz rendering — PASS.
- `test_clap_state10`: CLAP host `request_callback` and host-state `mark_dirty` requests, CP10 mapping persisted/restored in existing v3 format, malformed v1 state repaired — PASS.
- Native Windows DLL: PE32+ x86-64, nonzero `AddressOfEntryPoint`, `clap_entry` export present — PASS.
- Original overlay: every byte of GUI raster image and the 2,304-hit PCM bank identical to CP09; only the offset in trailing 32-byte footer changed for the new PE size — PASS.
- Unmodified DSP 88.2 kHz render: CP09 and CP10 8,192 audio bytes **bit-identical**, SHA-256 `886dd6ed263467e1163f91c85fef4e03d5c845a38872672568b8bbe2b3344a51` — PASS.

## Installation / validation in Bitwig

1. Download all `.7z.001` … `.7z.013` files into the same folder.
2. With 7-Zip, extract **only `.7z.001`**; all other volumes are discovered automatically.
3. Back up CP09 and close Bitwig. Install `Rubedo_Atlas_Drumkit.clap` under `C:\Program Files\Common Files\CLAP\`.
4. Open Bitwig, scan plugins, select Rubedo. Check Core pad 01 at MIDI 36 and Core 16 at MIDI 51. Select Extended 8: pad 17 must show MIDI 52, pad 24 MIDI 59.
5. Select pad 21 in Extended; click Learn Note; play a controller note (for example 72). GUI should display MIDI 72 on pad 21, and the same note must trigger/highlight that pad.
6. Save the Bitwig project, close and reopen, and verify pad 21 still maps to 72. Verify that the former mapping was reassigned appropriately if there was a conflict.

**Not verified:** actual Windows Bitwig host MIDI integration, editor paint appearance, controller-specific routing, and 7-Zip extraction on user Windows. The source-level test and package checks do not certify the in-host result.