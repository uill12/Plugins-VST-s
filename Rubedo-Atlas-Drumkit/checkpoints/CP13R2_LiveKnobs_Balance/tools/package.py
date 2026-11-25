#!/usr/bin/env python3
"""Package exact PE x64 core + CP13R2 live-knob skin + original PCM24 bank.
This does not modify source audio, MIDI map state, or CLAP identity.
"""
import argparse, ctypes, ctypes.util, hashlib, json, os, struct
from pathlib import Path

def sha(p):
 h=hashlib.sha256()
 with open(p,'rb') as f:
  for block in iter(lambda:f.read(8<<20),b''):h.update(block)
 return h.hexdigest()

def main():
 ap=argparse.ArgumentParser();ap.add_argument('core',type=Path);ap.add_argument('--base',type=Path,default=Path('/mnt/data/Rubedo_CP13R1_Release/Rubedo_Atlas_Drumkit.clap'));ap.add_argument('--skin',type=Path,default=Path('/mnt/data/cp13r2/rubedo_art_live_knobs.bgr'));ap.add_argument('--out',type=Path,default=Path('/mnt/data/Rubedo_CP13R2_Release'))
 args=ap.parse_args();args.out.mkdir(parents=True,exist_ok=True)
 core=args.core.read_bytes();assert core[:2]==b'MZ'
 with args.base.open('rb') as old:
  old.seek(-32,os.SEEK_END);foot=old.read(32);assert foot[:8]==b'RBOVR103'
  old_bank,bank_len,old_pic_len=struct.unpack_from('<QQQ',foot,8)
  assert bank_len==373714116 and old_pic_len==1512*1009*3
  old.seek(old_bank-old_pic_len);orig_skin_sha=hashlib.sha256(old.read(old_pic_len)).hexdigest()
  assert orig_skin_sha=='a6db03fc0c28dcd269bc0750e2fddac1d03e5faa8d8055c834dc7f1b077df9bb',orig_skin_sha
 skin=args.skin.read_bytes();assert len(skin)==old_pic_len
 full=args.out/'Rubedo_Atlas_Drumkit.clap'
 banksha=hashlib.sha256()
 with full.open('wb') as out,args.base.open('rb') as old:
  out.write(core);out.write(skin)
  old.seek(old_bank)
  left=bank_len
  while left:
   block=old.read(min(left,8<<20));assert block
   out.write(block);banksha.update(block);left-=len(block)
  out.write(b'RBOVR103'+struct.pack('<QQQ',len(core)+len(skin),bank_len,len(skin)))
 assert banksha.hexdigest()=='968aa26a3fe0b7abf89e68ff6354977c2e24d30b270e0036219e2906dc856454'
 with full.open('rb') as f:
  f.seek(-32,os.SEEK_END);footer=f.read(32);assert footer[:8]==b'RBOVR103'
  a,b,c=struct.unpack_from('<QQQ',footer,8);assert (a,b,c)==(len(core)+len(skin),bank_len,len(skin))
  f.seek(a);assert f.read(6)==b'RBDK24'
 report={'project':'Rubedo Atlas Drumkit','checkpoint':'CP13R2_LiveKnobs_Balance','version':'0.9.10-cp13r2','scope':'Linux/local DSP and MIDI tests; Windows source-only LoadLibrary CI if available; Bitwig host GUI manual QA pending','core':{'bytes':len(core),'sha256':hashlib.sha256(core).hexdigest()},'full_clap':{'bytes':full.stat().st_size,'sha256':sha(full)},'skin':{'bytes':len(skin),'sha256':hashlib.sha256(skin).hexdigest(),'source':'CP13R2 raster neutralizes 16 static dial needles/arcs/numbers; GDI draws animated state'},'pcm24_bank':{'bytes':bank_len,'sha256':banksha.hexdigest(),'changed':False},'sample_rate':96000,'raw_audio_regions':2304,'trim_table':'24 x 24 role-aware level trims; tested 24 kits x 9 pad roles + six-hit layers'}
 print(json.dumps(report,indent=2),flush=True)
 readme=f'''RUBEDO ATLAS DRUMKIT CP13R2 - LIVE KNOBS + ROLE-AWARE BALANCE
Windows x64 CLAP, v0.9.10-cp13r2

INSTALLATION:
1) Close Bitwig Studio and back up the prior Rubedo_Atlas_Drumkit.clap.
2) Keep all 13 .7z.001-.013 parts in the same directory; open ONLY .001 with 7-Zip.
3) Copy the extracted Rubedo_Atlas_Drumkit.clap into
   C:\\Program Files\\Common Files\\CLAP\\Rubedo_Atlas_Drumkit.clap
4) Restart Bitwig and rescan plugins. Compare same note across kits.

CHANGES:
- 16 animated knob pointers and changing gold position arcs, no fixed needles.
- Accurate current numeric values drawn below knobs, updated on drag/wheel,
  kit/pad change, Bitwig automation and state restore.
- Native role-aware per-kit gain compensation (snares/claps more defined;
  kick/sub remains foundation, long resonances held back).
- Bank and samples UNCHANGED, no added DSP latency or pre-sample delay.
- Existing CLAP state v3 and Kit/Atlas parameters retained.

EXPECTED SHA-256 of full CLAP: {report['full_clap']['sha256']}
Original bank SHA-256: {report['pcm24_bank']['sha256']}
GUI image SHA-256: {report['skin']['sha256']}
Windows Bitwig in-host GUI and hardware monitoring tests STILL require QA.
'''
 text_file=args.out/'LEIA_PRIMEIRO_CP13R2.txt';text_file.write_text(readme)
 lib=ctypes.CDLL(ctypes.util.find_library('archive'))
 ptr=ctypes.c_void_p
 def f(name,restype,argtypes):
  fn=getattr(lib,name);fn.restype=restype;fn.argtypes=argtypes;return fn
 W=f('archive_write_new',ptr,[]);fmt=f('archive_write_set_format_7zip',ctypes.c_int,[ptr]);opt=f('archive_write_set_format_option',ctypes.c_int,[ptr,ctypes.c_char_p,ctypes.c_char_p,ctypes.c_char_p]);filt=f('archive_write_add_filter_none',ctypes.c_int,[ptr]);op=f('archive_write_open_filename',ctypes.c_int,[ptr,ctypes.c_char_p]);header=f('archive_write_header',ctypes.c_int,[ptr,ptr]);en=f('archive_entry_new',ptr,[]);pname=f('archive_entry_set_pathname',None,[ptr,ctypes.c_char_p]);psize=f('archive_entry_set_size',None,[ptr,ctypes.c_int64]);pperm=f('archive_entry_set_perm',None,[ptr,ctypes.c_int]);ptype=f('archive_entry_set_filetype',None,[ptr,ctypes.c_uint]);wdat=f('archive_write_data',ctypes.c_ssize_t,[ptr,ptr,ctypes.c_size_t]);close=f('archive_write_close',ctypes.c_int,[ptr]);free=f('archive_write_free',None,[ptr]);efree=f('archive_entry_free',None,[ptr]);err=f('archive_error_string',ctypes.c_char_p,[ptr]);
 archive=args.out/'Rubedo_CP13R2_CLAP_LiveKnobs_RoleBalance_Windows_x64.7z'
 a=W();assert a and fmt(a)==0 and filt(a)==0 and opt(a,b'7zip',b'compression',b'copy')==0
 assert op(a,os.fsencode(archive))==0,err(a)
 for item in [full,text_file]:
  e=en();pname(e,os.fsencode(item.name));psize(e,item.stat().st_size);ptype(e,0o100000);pperm(e,0o644);assert header(a,e)==0,err(a)
  with item.open('rb') as src:
   while True:
    blob=src.read(8<<20)
    if not blob:break
    buf=ctypes.create_string_buffer(blob)
    assert wdat(a,buf,len(blob))==len(blob),err(a)
  efree(e)
 assert close(a)==0,err(a);free(a)
 parts=args.out/'DriveParts';parts.mkdir(exist_ok=True)
 regs=[]
 with archive.open('rb') as inp:
  i=0
  while True:
   data=inp.read(28<<20)
   if not data:break
   i+=1;dest=parts/(archive.name+f'.{i:03d}');dest.write_bytes(data)
   regs.append({'file':dest.name,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()})
 assert len(regs)==13
 report['archive']={'name':archive.name,'bytes':archive.stat().st_size,'sha256':sha(archive),'volumes':regs}
 (parts/'VOLUMES_SHA256.json').write_text(json.dumps(report,indent=2)+'\n')
 (parts/text_file.name).write_text(readme)
 (args.out/'QA_MANIFEST.json').write_text(json.dumps(report,indent=2)+'\n')
 print('Package',archive,'parts',len(regs),'bytes',archive.stat().st_size,flush=True)
if __name__=='__main__':main()
