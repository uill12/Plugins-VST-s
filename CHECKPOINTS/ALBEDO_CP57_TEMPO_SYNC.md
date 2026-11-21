# Albedo CP57 — DAW Tempo Sync
Date: 2026-11-21
Branch: `albedo/cp57-daw-tempo-sync`
Parent: CP56 Sound-Impact Density (rollback preserved).
Version: `1.0.57` (Windows builds still need verification).

## Functional behavior
- `6030=1` selects DAW BPM + PPQ position when the host supplies transport. First arpeggio note played off-beat waits for the next musical grid position. Swing shifts odd steps while even pair anchors remain fixed.
- `6030=0` uses the independent BPM parameter `6029`, preserving free-running rhythm.
- `6021` supports visible 1/4, 1/8, 1/16, 1/32 and 1/64 choices; arbitrary saved values remain intact and display as CUSTOM.
- Host seeks, loops and BPM changes recalibrate clock phase at block boundaries. Active note gate releases on a transport discontinuity.
- Factory presets with `6020=1` and no explicit `6030` value default to DAW sync. Explicit FREE presets stay free; non-arpeggiated factory presets are not forced into sync.
- No new automatable parameter IDs; stable VST3/CLAP IDs and saved DAW sessions preserved.
- The original detailed ARP popup and CP56 compact layout remain intact.

## QA
- `Albedo-JUCE-Native/tests/check_cp57_tempo_sync.py` checks PPQ integration, real parameter binding, preset behavior, assets.
- Extended `patches/cp50/arp_test.cpp` tests off-beat note onset, rephasing on seek and transport stop at 44.1/48/88.2/96 kHz.
- CP41/CP50 test version allowlists updated for 1.0.57; full Windows DSP and Bitwig QA pending.
