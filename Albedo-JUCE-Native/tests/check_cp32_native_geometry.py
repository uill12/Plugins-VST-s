#!/usr/bin/env python3
"""CP32 legacy entry point delegates to CP33 responsive, host-safe geometry audit."""
from pathlib import Path
p=Path(__file__).with_name('check_cp33_host_fit.py')
assert p.is_file(), 'Missing native CP33 host-fit geometry contract'
exec(compile(p.read_text(encoding='utf-8'),str(p),'exec'),{'__file__':str(p)})
