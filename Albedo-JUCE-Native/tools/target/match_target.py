"""Compatibility entrypoint: acquire reference descriptors only.

The former target-matching/reconstructed-patch feature is intentionally
disabled. Use Acquire_Reference.exe to capture source characteristics;
Morph Tree sound generation runs separately and never automatically matches
or copies a reference's audio.
"""
from acquire_reference import main

if __name__=="__main__":
    raise SystemExit(main())
