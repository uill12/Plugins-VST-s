#!/usr/bin/env python3
"""RGNES Windows CLAP/VST3 PE64 and export/bundle release gate (stdlib only)."""
from __future__ import annotations
import argparse, hashlib, json, mmap, os, shutil, struct, sys
from pathlib import Path

class VerificationError(Exception):
    pass

def _u16(b, off):
    if off < 0 or off + 2 > len(b): raise VerificationError('truncated PE header')
    return struct.unpack_from('<H', b, off)[0]

def _u32(b, off):
    if off < 0 or off + 4 > len(b): raise VerificationError('truncated PE header')
    return struct.unpack_from('<I', b, off)[0]

def parse_pe(path: Path):
    with path.open('rb') as f:
        with mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ) as buf:
            if len(buf) < 0x100 or buf[:2] != b'MZ': raise VerificationError('not a Windows PE executable (MZ missing)')
            pe = _u32(buf, 0x3c)
            if pe + 24 > len(buf) or buf[pe:pe+4] != b'PE\0\0': raise VerificationError('PE signature missing')
            machine, sections = _u16(buf, pe+4), _u16(buf, pe+6)
            opt_len = _u16(buf, pe+20)
            opt = pe + 24
            if opt + opt_len > len(buf) or _u16(buf, opt) != 0x20b:
                raise VerificationError('not PE32+ (64-bit Windows)')
            if machine != 0x8664: raise VerificationError(f'wrong machine: 0x{machine:04x}, expected AMD64 0x8664')
            size_headers = _u32(buf, opt+60)
            n_data_dirs = _u32(buf, opt+108)
            table = opt + opt_len
            regions = []
            for i in range(sections):
                so = table + i*40
                if so+40 > len(buf): raise VerificationError('truncated section headers')
                virtual_size = _u32(buf, so+8)
                virtual_address = _u32(buf, so+12)
                raw_size = _u32(buf, so+16)
                raw_offset = _u32(buf, so+20)
                regions.append((virtual_address, max(virtual_size, raw_size), raw_offset, raw_size))
            def offset(rva, needed=1):
                if rva < size_headers and rva + needed <= len(buf): return rva
                for vaddr, vsize, roff, rsize in regions:
                    if vaddr <= rva < vaddr + vsize:
                        delta = rva - vaddr
                        if delta + needed <= rsize and roff + delta + needed <= len(buf):
                            return roff + delta
                raise VerificationError(f'RVA 0x{rva:x} cannot be mapped into file')
            exports=[]
            if n_data_dirs and opt + 120 <= opt + opt_len:
                export_rva, export_size = _u32(buf, opt+112), _u32(buf, opt+116)
                if export_rva and export_size:
                    ed = offset(export_rva, 40)
                    n_names = _u32(buf, ed+24)
                    names_rva = _u32(buf, ed+32)
                    if n_names > 100000: raise VerificationError('suspicious export name count')
                    name_table = offset(names_rva, 4*n_names) if n_names else None
                    for j in range(n_names):
                        name_rva = _u32(buf, name_table + 4*j)
                        name_off = offset(name_rva)
                        end = buf.find(b'\0', name_off, min(len(buf), name_off+4096))
                        if end < 0: raise VerificationError('unterminated export name')
                        exports.append(buf[name_off:end].decode('ascii', 'replace'))
            return {'machine':'AMD64','pe_format':'PE32+','exports':exports}

def hash_file(path):
    h=hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda:f.read(2**20),b''):h.update(block)
    return h.hexdigest()

def collect(root:Path, formats):
    if not root.is_dir():raise VerificationError(f'artifact root is not a directory: {root}')
    files=[p for p in root.rglob('*') if p.is_file()]
    clap=[p for p in files if p.suffix.lower()=='.clap']
    vst3=[p for p in files if p.suffix.lower()=='.vst3']
    selected=[]; errors=[]; warnings=[]
    if 'clap' in formats:
        if not clap:errors.append('No .clap binary found in artifact root')
        for p in clap:selected.append((p,'clap',p))
    if 'vst3' in formats:
        if not vst3:errors.append('No VST3 binary found in artifact root')
        for p in vst3:
            parent=p.parent
            valid=parent.name.lower()=='x86_64-win' and parent.parent.name.lower()=='contents' and parent.parent.parent.suffix.lower()=='.vst3'
            if not valid: errors.append(f'Unbundled VST3: {p.relative_to(root)}; expected Name.vst3/Contents/x86_64-win/Name.vst3')
            selected.append((p,'vst3',parent.parent.parent if valid else p))
    seen=set();found=[]
    for binary,fmt,pkg in selected:
        relative=str(binary.relative_to(root)).replace(os.sep,'/')
        if relative in seen:continue
        seen.add(relative)
        try:
            meta=parse_pe(binary)
            required='clap_entry' if fmt=='clap' else 'GetPluginFactory'
            if required not in meta['exports']: errors.append(f'{relative}: missing export {required}')
            item={'format':fmt,'path':relative,'package':str(pkg.relative_to(root)).replace(os.sep,'/'),
                  'size_bytes':binary.stat().st_size,'sha256':hash_file(binary),'machine':meta['machine'],
                  'export_check':required in meta['exports'],'required_export':required}
            found.append(item)
        except (OSError, ValueError, VerificationError) as e:
            errors.append(f'{relative}: {e}')
    if 'vst3' in formats and vst3: warnings.append('Static export/PE validation does not prove VST3 factory ABI or DAW host acceptance')
    if 'clap' in formats and clap: warnings.append('Static export/PE validation does not prove CLAP init, DSP processing or DAW host acceptance')
    return {'schema':'rgnes-win64-check-v1','root':str(root),'required_formats':sorted(formats),
            'status':'passed' if not errors else 'failed','verified_binaries':found,
            'errors':errors,'warnings':warnings,'host_tested':False,'dsp_tested':False}

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root',required=True,help='Directory containing the NEWLY BUILT runtime binaries or bundles')
    ap.add_argument('--formats',default='clap,vst3',help='Comma separated required formats, default clap,vst3')
    ap.add_argument('--json-out',help='Write machine-readable verification report (not proof of host QA)')
    ap.add_argument('--stage-out',help='Copy ONLY verified runtime binary/bundles into a clean release staging directory')
    args=ap.parse_args()
    formats={v.strip().lower() for v in args.formats.split(',') if v.strip()}
    if not formats or formats-{'clap','vst3'}:ap.error('formats must be clap, vst3 or clap,vst3')
    root=Path(args.root).expanduser().resolve()
    try:report=collect(root,formats)
    except (OSError,VerificationError) as e:
        report={'schema':'rgnes-win64-check-v1','root':str(root),'required_formats':sorted(formats),'status':'failed','errors':[str(e)],'verified_binaries':[], 'warnings':[],'host_tested':False,'dsp_tested':False}
    if report['status']=='passed' and args.stage_out:
        dest=Path(args.stage_out).expanduser().resolve()
        if dest==root or root in dest.parents:
            report['errors'].append('stage-out must NOT be within the scanned root (avoid recursive artifacts)')
        elif dest.exists() and any(dest.iterdir()):
            report['errors'].append('stage-out must be empty to avoid stale binaries')
        else:
            dest.mkdir(parents=True,exist_ok=True)
            for item in report['verified_binaries']:
                src_pkg=root/item['package'];dst_pkg=dest/item['package']
                dst_pkg.parent.mkdir(parents=True,exist_ok=True)
                if src_pkg.is_dir():
                    if not dst_pkg.exists():shutil.copytree(src_pkg,dst_pkg)
                else: shutil.copy2(src_pkg,dst_pkg)
            report['staged_runtime_only']=str(dest)
        if report['errors']:report['status']='failed'
    output=json.dumps(report,indent=2,ensure_ascii=False)
    if args.json_out:
        out=Path(args.json_out).expanduser().resolve();out.parent.mkdir(parents=True,exist_ok=True)
        out.write_text(output+'\n',encoding='utf-8')
    print(output)
    return 0 if report['status']=='passed' else 2
if __name__=='__main__':sys.exit(main())
