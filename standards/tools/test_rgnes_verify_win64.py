"""Synthetic-header unit tests: never count these files as compiled plugins."""
import struct, tempfile, unittest
from pathlib import Path
import rgnes_verify_win64 as r

def fake_pe(name, machine=0x8664):
    b=bytearray(0x600)
    b[0:2]=b'MZ';struct.pack_into('<I',b,0x3c,0x80)
    b[0x80:0x84]=b'PE\0\0'
    struct.pack_into('<HH',b,0x84,machine,1)
    struct.pack_into('<H',b,0x80+4+16,0xF0)
    opt=0x80+24
    struct.pack_into('<H',b,opt,0x20b)
    struct.pack_into('<I',b,opt+60,0x200)
    struct.pack_into('<I',b,opt+108,16)
    struct.pack_into('<II',b,opt+112,0x1000,0x100)
    sec=opt+0xF0
    b[sec:sec+8]=b'.edata\0\0'
    struct.pack_into('<IIII',b,sec+8,0x200,0x1000,0x200,0x200)
    struct.pack_into('<I',b,0x200+24,1)
    struct.pack_into('<I',b,0x200+32,0x1030)
    struct.pack_into('<I',b,0x200+0x30,0x1040)
    b[0x200+0x40:0x200+0x40+len(name)+1]=name.encode('ascii')+b'\0'
    return bytes(b)
class TestRGNES(unittest.TestCase):
    def test_success_both(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp)
            (root/'Test.clap').write_bytes(fake_pe('clap_entry'))
            vst=root/'Test.vst3'/'Contents'/'x86_64-win'
            vst.mkdir(parents=True)
            (vst/'Test.vst3').write_bytes(fake_pe('GetPluginFactory'))
            result=r.collect(root,{'clap','vst3'})
            self.assertEqual(result['status'],'passed')
            self.assertEqual(len(result['verified_binaries']),2)
            self.assertFalse(result['host_tested'])
    def test_wrong_machine(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp);(root/'Test.clap').write_bytes(fake_pe('clap_entry',0x14c))
            result=r.collect(root,{'clap'})
            self.assertEqual(result['status'],'failed')
            self.assertIn('wrong machine', ' '.join(result['errors']))
    def test_missing_export(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp);(root/'Test.clap').write_bytes(fake_pe('wrong_entry'))
            result=r.collect(root,{'clap'})
            self.assertEqual(result['status'],'failed')
            self.assertIn('missing export clap_entry', ' '.join(result['errors']))
    def test_standalone_vst3_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp);(root/'Test.vst3').write_bytes(fake_pe('GetPluginFactory'))
            result=r.collect(root,{'vst3'})
            self.assertEqual(result['status'],'failed')
            self.assertIn('Unbundled VST3', ' '.join(result['errors']))
if __name__=='__main__':unittest.main()
