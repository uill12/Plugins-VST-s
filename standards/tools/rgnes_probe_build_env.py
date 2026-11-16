#!/usr/bin/env python3
"""RGNES local-first Windows x64 build capability probe. Does not compile or certify plugins.

Stdlib-only. Reports verified tools found on this runtime; missing tools are explicit.
Even when a suitable candidate toolchain is found, SDK/project compatibility must
be validated by an actual compile and Windows PE/export/host checks.
"""
import argparse
import json
import os
import platform
import shutil
import sys
from pathlib import Path

TOOL_NAMES = (
    "cl", "clang-cl", "clang", "clang++", "lld-link", "link", "cmake", "ninja",
    "msbuild", "MSBuild.exe", "x86_64-w64-mingw32-gcc", "x86_64-w64-mingw32-g++",
    "x86_64-w64-mingw32-windres", "llvm-rc", "rc", "wine", "python", "python3",
)


def probe(project_root=None):
    host = platform.system()
    tools = {name: shutil.which(name) for name in TOOL_NAMES}
    native_windows = host == "Windows"
    mingw_link = bool(tools["x86_64-w64-mingw32-g++"] or tools["x86_64-w64-mingw32-gcc"])
    llvm_link = bool(tools["clang-cl"] and tools["lld-link"])
    win_sdk = bool(os.getenv("WindowsSdkDir") or os.getenv("WindowsSDKVersion"))
    vc_tools = bool(os.getenv("VCToolsInstallDir"))
    native_compiler = native_windows and bool(tools["cl"] or tools["clang-cl"])
    cross_candidate = not native_windows and mingw_link
    # LLVM tools alone do NOT prove Windows SDK/headers, C runtime, or import libs.
    if native_compiler:
        candidate = "chatgpt-native-windows"
        blockers = []
    elif cross_candidate:
        candidate = "chatgpt-cross-mingw-candidate"
        blockers = ["Verify MinGW-w64 Windows sysroot, headers/import libs, and project ABI compatibility"]
    else:
        candidate = "external-windows-runner-needed"
        blockers = ["No verified local native Windows x64 compiler or MinGW-w64 cross-compiler"]
    if not (tools["cmake"] or native_windows and (tools["msbuild"] or tools["MSBuild.exe"])):
        blockers.append("No CMake/MSBuild build driver detected (project custom scripts may still exist)")
    result = {
        "schema": "rgnes.build-environment.v1",
        "host": {"os": host, "architecture": platform.machine(), "python": sys.version.split()[0]},
        "tools": tools,
        "windows_sdk_env_present": win_sdk,
        "visual_studio_tools_env_present": vc_tools,
        "llvm_windows_candidate": llvm_link,
        "mingw_x64_candidate": mingw_link,
        "recommended_first_attempt": candidate,
        "blockers_or_required_checks": blockers,
        "disclaimer": "A detected compiler is not evidence of a successful Windows CLAP/VST3 build; a fresh PE64 output and independent format checks are mandatory.",
    }
    if project_root is not None:
        root = Path(project_root).expanduser().resolve()
        if not root.is_dir():
            raise ValueError(f"Project directory not found: {root}")
        result["project"] = {
            "root": str(root),
            "cmake": (root / "CMakeLists.txt").is_file(),
            "windows_build_script": (root / "Build_Windows_x64.ps1").is_file(),
            "solution_files": [p.name for p in root.glob("*.sln")],
        }
        if not (result["project"]["cmake"] or result["project"]["windows_build_script"] or result["project"]["solution_files"]):
            result["blockers_or_required_checks"].append("No CMakeLists.txt, Build_Windows_x64.ps1 or .sln in project root; inspect repository-specific build instructions")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project-root", help="Optional source directory containing the actual project")
    parser.add_argument("--output", help="Optional JSON output file; stdout is always printed")
    args = parser.parse_args()
    try:
        info = probe(args.project_root)
    except ValueError as exc:
        parser.error(str(exc))
    output = json.dumps(info, indent=2, sort_keys=True) + "\n"
    print(output, end="")
    if args.output:
        path = Path(args.output)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(output, encoding="utf-8")


if __name__ == "__main__":
    main()
