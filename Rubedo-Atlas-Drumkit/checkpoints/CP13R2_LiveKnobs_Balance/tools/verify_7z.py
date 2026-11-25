import ctypes,ctypes.util,hashlib,json,os
from pathlib import Path
lib=ctypes.CDLL(ctypes.util.find_library('archive'))
ptr=ctypes.c_void_p;C=ctypes.c_char_p;I=ctypes.c_int
f=lambda name,r,a: (lambda x: (setattr(x,'restype',r),setattr(x,'argtypes',a),x)[-1])(getattr(lib,name))
new=f('archive_read_new',ptr,[]);fm=f('archive_read_support_format_all',I,[ptr]);fi=f('archive_read_support_filter_all',I,[ptr]);openfn=f('archive_read_open_filename',I,[ptr,C,ctypes.c_size_t]);nexthead=f('archive_read_next_header',I,[ptr,ctypes.POINTER(ptr)]);pathname=f('archive_entry_pathname',C,[ptr]);size=f('archive_entry_size',ctypes.c_int64,[ptr]);data=f('archive_read_data',ctypes.c_ssize_t,[ptr,ptr,ctypes.c_size_t]);close=f('archive_read_close',I,[ptr]);free=f('archive_read_free',I,[ptr]);err=f('archive_error_string',C,[ptr]);
a=new();assert fm(a)==0 and fi(a)==0
archive=b'/mnt/data/Rubedo_CP13R2_Release/Rubedo_CP13R2_CLAP_LiveKnobs_RoleBalance_Windows_x64.7z'
assert openfn(a,archive,8192)==0,err(a)
results={}
while True:
 ent=ptr();status=nexthead(a,ctypes.byref(ent))
 if status==1:break
 assert status==0,(status,err(a))
 name=os.fsdecode(pathname(ent));h=hashlib.sha256();count=0
 buf=ctypes.create_string_buffer(8*1024*1024)
 while True:
  n=data(a,buf,len(buf))
  assert n>=0,(n,err(a))
  if not n:break
  h.update(buf.raw[:n]);count+=n
 assert count==size(ent),(name,count,size(ent));results[name]={'size':count,'sha256':h.hexdigest()}
close(a);free(a)
print(json.dumps(results,indent=2))
m=json.loads(Path('/mnt/data/Rubedo_CP13R2_Release/DriveParts/VOLUMES_SHA256.json').read_text());assert results['Rubedo_Atlas_Drumkit.clap']['sha256']==m['full_clap']['sha256'];print('INDEPENDENT libarchive verification PASS')
