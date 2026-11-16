# RGNES — ChatGPT-first build and fallback policy (v2.2.0)

**Default build environment: the active ChatGPT execution environment.** This is a priority, not a promise that the ChatGPT host always contains a Windows toolchain, accessible project files, Windows desktop, or transferable Drive file references. Do as much actual engineering and verification as the current environment supports; use an external Windows runner only for tasks that cannot be completed locally.

## Mandatory execution order

1. **Acquire authoritative inputs.** Discover the latest real project source, licensed sample/bitmap resources, pinned SDK, build scripts and exact branch/version. Prefer attached/project files and authorized materialization, then source from connected GitHub. Inspect actual file bytes. ChatGPT Library metadata without authorized raw bytes is not build input.
2. **Probe the *current ChatGPT runtime*** before sending work elsewhere:
   ```sh
   python skills/full-daw-engineering/references/tools/rgnes_probe_build_env.py --project-root PATH_TO_PROJECT
   ```
   From this plugin, use the equivalent tool installed in the current source tree or `standards/tools/rgnes_probe_build_env.py` in GitHub. The report is tool detection, **not proof of a compiled plugin**.
3. **Local-first implementation and tests.** Prefer in-session ChatGPT tools (terminal/container/Python) for C/C++ source edits, resource generation, architecture and unit tests, deterministic DSP/offline audio renders, packaging, manifests, SHA-256, and CLAP/VST3 ABI source checks. Avoid exporting these steps to GitHub unnecessarily. Preserve logs and failures.
4. **Attempt a local Windows x64 build if technically feasible:**
   - Native Windows host: build with project-pinned MSVC/Windows SDK and project scripts/CMake/MSBuild.
   - Linux host: only if an actual x86_64 Windows cross compiler **and matching Windows sysroot, import libraries, ABI dependencies, resources and project-compatible setup** are present or can be legitimately installed. A bare Clang/LLVM linker does not prove Windows targeting works.
   - Generate a **fresh** Windows PE32+ AMD64 `.clap` and `.vst3` bundle, not a Linux ELF renamed `.clap`, and validate exports `clap_entry` / `GetPluginFactory` independently.
   - When feasible, install missing permissively licensed build tools in the sandbox (with provenance) and rerun. Do not download proprietary/unlicensed SDK/resource data or substitute placeholder samples.
5. **Fallback to external Windows build ONLY after the local attempt is blocked** by missing/unsupported target toolchain, Windows-only SDK, host requirements or other hard environmental constraints. The preferred fallback is an authorized GitHub Actions `windows-2022` runner with complete source inputs. Prepare source/build scripts/QA in ChatGPT first. Record the exact fallback reason, CI workflow/run and inputs, then retrieve and inspect actual build outputs if supported.
6. **Validate, package and deliver in ChatGPT when possible.** Validate actual artifact freshness, PE64 architecture, format exports, VST3 Windows bundle structure, SHA-256, full archive extraction, target sample rate / state and audio QA. Windows Bitwig runtime validation remains separate and may require an external Windows host even when a cross-compiled binary passes static checks.
7. **Publish only verified artifacts.** GitHub holds source, CI recipes, build logs, QA, checkpoint and metadata; runtime `.clap` / `.vst3` distributions go to the project Drive folder only when connector-accepted transferable bytes are available. Keep rollbacks. Never claim a sandbox download is automatically a Google Drive `file_uri`.

## Decision matrix

| Situation | Default | Fallback | Report |
| --- | --- | --- | --- |
| ChatGPT has complete inputs + compatible Windows toolchain | Compile and test **inside ChatGPT** | Windows Actions only on demonstrated local failure | `local-compiled` when binary verified |
| ChatGPT Linux has complete MinGW-w64 cross toolchain + compatible assets/headers | Cross-compile inside ChatGPT; PE/export QA | Windows Actions for missing Windows-only dependencies or host checks | `cross-compiled`, not native Windows-host-tested |
| ChatGPT has Clang/CMake but no Windows sysroot | Implement, run platform-neutral DSP tests in ChatGPT, explain missing toolchain | Windows GitHub Actions if setup complete | `local-source-tested / Windows build pending` |
| Only Library filename/metadata exists, raw bytes inaccessible | Inspect accessible metadata; don't fabricate source | Request supported source access or use verified GitHub source | `source unavailable` |
| GitHub workflow file exists but no successful run/output available | Do not call it a build | Launch authorized CI when feasible, inspect outputs | `CI not verified` |

## Required per-build evidence manifest

`project`, `source_sha`, `checkpoint`, `version`, `build_date`, `requested_formats`, `actual_formats`, `target_os_arch`, `execution_environment` (`chatgpt-native`, `chatgpt-cross`, `external-windows`, or `none`), `compiler+sdk`, `repro_command`, `local_probe_result`, `fallback_reason`, `build_log`, `artifact_files`, `sha256`, `static_validation`, `offline_audio_validation`, `host_validation`, `drive_upload`, `rollback`.

No successful compile without actual binary bytes and independent format validation. If CLAP succeeds but VST3 fails, record two distinct statuses. Do not force users into an external build when ChatGPT is capable of producing the target internally. No changes to a DAW source/binary are implied by updating this policy.
