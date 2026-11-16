#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)";cd "$root"
mkdir -p distrib build
for mode in 0 1;do
 if [ "$mode" = 0 ];then kind=Audio;else kind=MIDI;fi
 for entry in Clap Vst3;do
  if [ "$entry" = Clap ];then ext=clap;else ext=vst3;fi
  g++ -std=c++17 -fPIC -O2 -shared -DNIG_MODE="$mode" "src/$entry.cpp" -o "distrib/Nigredo_Opus_Septem_${kind}_FX_Linux_x64.$ext"
 done
done
g++ -O2 -std=c++17 tests/clap_smoke.cpp -ldl -o build/clap_smoke
g++ -O2 -std=c++17 tests/vst3_smoke.cpp -ldl -o build/vst3_smoke
g++ -O2 -std=c++17 tests/dsp_sweep.cpp -o build/dsp_sweep
for mode in Audio MIDI;do
 if [ "$mode" = Audio ];then k=audio;else k=midi;fi
 build/clap_smoke "distrib/Nigredo_Opus_Septem_${mode}_FX_Linux_x64.clap" "$k"
 build/vst3_smoke "distrib/Nigredo_Opus_Septem_${mode}_FX_Linux_x64.vst3" "$k"
done
build/dsp_sweep
