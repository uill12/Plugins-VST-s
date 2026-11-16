#!/usr/bin/env python3
"""Build a deliberately NON-RELEASE Rubedo VST3 diagnostic bundle.

The original 2304-hit CP12R1 bank and approved SkinSync art are *not* used.
This serves only to exercise Win64 PE resource lookup, VST3 factory and
component initialization in an independent process.

Never distribute this as the Rubedo instrument.
"""
import hashlib
import pathlib
import struct
import sys

KIT_COUNT = 24
PAD_COUNT = 24
VAR_COUNT = 4
REGIONS = KIT_COUNT * PAD_COUNT * VAR_COUNT
WIDTH, HEIGHT, PIXEL_BYTES = 1512, 1009, 3
PICTURE_BYTES = WIDTH * HEIGHT * PIXEL_BYTES
FRAMES = 4096
OUTPUT = pathlib.Path("Rubedo_CP12R1_DIAGNOSTIC_ONLY") / "Rubedo Atlas Drumkit.vst3"
BINARY_NAME = "Rubedo Atlas Drumkit.vst3"

def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def make_bank() -> bytes:
    bank_header = b"RBDK24\0\0" + struct.pack("<IIII", 1, 96000, REGIONS, 0)
    frame_offset = 24 + REGIONS * 8
    table = b"".join(struct.pack("<II", frame_offset, FRAMES)
                     for _ in range(REGIONS))
    frames = bytearray(FRAMES * 3)
    for i in range(FRAMES):
        # Small stable signed PCM24 diagnostic signal, NOT real samples.
        sample = (((i * 53) & 2047) - 1024) * 6000
        frames[3*i:3*i+3] = struct.pack("<i", sample)[:3]
    bank = bank_header + table + bytes(frames)
    assert len(bank_header) == 24
    assert len(bank) == frame_offset + FRAMES * 3
    return bank

def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit("Usage: python create_synthetic_bank_diagnostic.py <source-only.vst3>")
    core = pathlib.Path(sys.argv[1]).read_bytes()
    if not core.startswith(b"MZ") or len(core) < 1024:
        raise SystemExit("Missing valid compiled Windows PE core")
    picture = bytes(PICTURE_BYTES)  # Placeholder artwork; not Rubedo skin
    bank = make_bank()
    bank_offset = len(core) + len(picture)
    footer = b"RBOVR103" + struct.pack("<QQQ", bank_offset, len(bank), len(picture))
    assert len(footer) == 32
    binary = OUTPUT / "Contents" / "x86_64-win" / BINARY_NAME
    binary.parent.mkdir(parents=True, exist_ok=True)
    with binary.open("wb") as file:
        file.write(core)
        file.write(picture)
        file.write(bank)
        file.write(footer)
    result = binary.read_bytes()
    assert result[-32:-24] == b"RBOVR103"
    assert int.from_bytes(result[-24:-16], "little") == bank_offset
    assert result[bank_offset:bank_offset+8] == b"RBDK24\0\0"
    report = (
        "RUBEDO CP12R1 VST3 -- SYNTHETIC DIAGNOSTIC ONLY\n"
        "NOT A RELEASE: NO ORIGINAL SOUNDS OR APPROVED ARTWORK\n"
        "DO NOT INSTALL THIS IN PLACE OF WORKING RUBEDO CLAP\n"
        f"BINARY_SHA256={sha256(result)}\n"
        f"BINARY_SIZE={len(result)}\n"
        f"PCM_REGIONS={REGIONS}\n"
        f"PCM_FAKE_SHA256={sha256(bank)}\n"
        "PURPOSE=Windows LoadLibrary, InitDll, GetPluginFactory, IComponent.initialize\n"
    )
    pathlib.Path("DIAGNOSTIC_ONLY.txt").write_text(report, encoding="utf-8")
    print(report, end="")

if __name__ == "__main__":
    main()
