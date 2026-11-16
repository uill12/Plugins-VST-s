#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)";cd "$root"
mkdir -p distrib
libs=(winlibs/user32_extended.lib winlibs/kernel32_web.lib winlibs/kernel32.lib winlibs/user32.lib winlibs/stub.lib winlibs/kernel32_add.lib winlibs/user32_add.lib)
for mode in 0 1;do
 if [ "$mode" = 0 ];then kind=Audio;else kind=MIDI;fi
 for entry in Clap Vst3;do
  if [ "$entry" = Clap ];then ext=clap;else ext=vst3;fi
  clang-cl -target x86_64-pc-windows-msvc -fuse-ld=lld /nologo /LD /O2 /GS- /GR- /EHs-c- /DNIG_MODE=$mode "src/$entry.cpp" src/WinCrt.cpp /link /NODEFAULTLIB /ENTRY:DllMain "/OUT:distrib/Nigredo_Opus_Septem_${kind}_FX_Windows_x64.$ext" "${libs[@]}"
 done
done
