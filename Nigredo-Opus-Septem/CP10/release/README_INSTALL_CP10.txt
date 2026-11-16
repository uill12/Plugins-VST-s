NIGREDO OPUS SEPTEM — CP10 RGNES NATIVE RENDERER HOTFIX
2026-11-15 | 0.9.1-cp10 | Audio FX + MIDI Note FX, both CLAP and VST3, Windows x64

This is a GUI hotfix for CP09's overpainting/full-window-refresh behavior.
Visual design, 16:10 window, DSP, MIDI processing, plugin identities and parameter IDs are unchanged.
No WebView2 runtime is required; renderer uses built-in Windows GDI/GDI+.

INSTALL
1. Close Bitwig Studio.
2. Extract all contents of the COMPLETE package (do not run directly inside ZIP).
3. In PowerShell at the extracted root:
   powershell -NoProfile -ExecutionPolicy Bypass -File .\Install_Nigredo_CP10.ps1
4. Restart Bitwig and rescan plugins.
5. Verify the editor by dragging controls, switching stages, changing presets and resizing.

CLAP files are installed in %%LOCALAPPDATA%%\Programs\Common\CLAP by default;
VST3 in %%LOCALAPPDATA%%\Programs\Common\VST3. -Scope AllUsers uses CommonProgramFiles.
Old files are renamed with .backup-before-cp10-... suffix by the installer.
Preserve CP09 separately for rollback. CLAP/VST3 editor identifiers follow CP10.

RGNES STATUS: Windows x64 cross-compiled and static/offline tested.
Windows Bitwig runtime GUI/resize/automation behavior has not been validated here.
