"""Compare final plugin resource 102 with the generated PCM24 bank."""
import hashlib
from pathlib import Path
import sys
import pefile

expected = Path(sys.argv[1]).read_bytes()
assert expected.startswith(b"VTRB2\0\0\0")
for module in sys.argv[2:]:
    pe = pefile.PE(module)
    def child(directory, ident):
        return next(entry.directory for entry in directory.entries if entry.id == ident)
    languages = child(child(pe.DIRECTORY_ENTRY_RESOURCE, 10), 102).entries
    assert languages
    for lang in languages:
        leaf = lang.data.struct
        payload = pe.get_data(leaf.OffsetToData, leaf.Size)
        assert payload == expected, module
    print(module, len(expected), hashlib.sha256(expected).hexdigest())
