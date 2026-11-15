# CP08R1 — CLAP initialization hotfix (Windows x64)

**Reported host error:** Bitwig Studio: `Could not create CLAP instance: init() returned false`.

## Root cause found in CP08

PE32+ DLL header of the CP08 `.clap` had `AddressOfEntryPoint = 0`. Windows therefore did not invoke the `DllMain` that assigns `rubedo_module = module` on process attach. The bank loader calls `GetModuleFileNameA(rubedo_module, path, ...)`; when `rubedo_module` is null, Windows resolves the **host executable** rather than the actual Rubedo plug-in DLL. The loader then cannot find the `RBOVR103` bank footer and `rubedo_load_bank()` returns 0; this propagates to `cb_init() == false`.

## Corrective build flag

The CLAP Windows x64 DLL must link with **`/entry:DllMain`** and `/dll`, not `/noentry`.

```text
lld-link /dll /machine:x64 /entry:DllMain /subsystem:windows /out:Rubedo_Atlas_Drumkit.clap clap.obj engine.obj gui.obj fltused.obj kernel32.lib user32.lib gdi32.lib /opt:ref /opt:icf
```

After linking, append the original CP08 GUI image, 96kHz PCM24 bank, and trailing `RBOVR103` footer to the PE file. If the PE size changes, adjust the bank start offset in the footer. The CP08R1 rebuilt PE core is the exact same byte length as the CP08 PE core, so the original overlay and its footer were appended **bit for bit** without modifying any bank metadata.

## Verification

- The repaired PE header has `AddressOfEntryPoint = 0x1070`.
- Export `clap_entry` is present.
- All PCM bank and GUI overlay bytes identical to CP08.
- The embedded bank header is `RBDK24\0\0` with 2,304 96kHz regions.
- Original and repaired plugin have the same length: 378,449,180 bytes.
- Repaired `.clap` SHA-256: `9cc5a03a032c72132e926cf928ff2a02845c59e3c5db2d3670d91f73c47cb859`.
- Native 7z archive SHA-256: `5a9416a3a96d241e5993ed2a55827367672f62b07787953b7ddc999167bc713a`.
- Full 7z file was successfully decoded by an independent libarchive reader.

## Distribution

The CP08R1 fixed CLAP is available in Google Drive at `PROJETOS/Rubedo Atlas Drumkit/CP08R1_InitFix`, split into 13 native `.7z.001`–`.7z.013` volumes. Download all to the same directory and extract **only `.7z.001`** with 7-Zip; no scripts necessary.

**Scope:** CP08R1 fixes the identified load-time DLL initialization defect. It has **not** been run inside Bitwig Studio for Windows. GUI responsiveness remains unverified and may still need further fixes. VST3 was not rebuilt for CP08R1.
