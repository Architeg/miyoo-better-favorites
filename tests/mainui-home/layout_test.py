#!/usr/bin/env python3
"""Read-only exact-binary fixture tests; artifacts stay in caller's host folder."""
import importlib.util
from pathlib import Path
import struct
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prototype', ROOT/'integration/mainui-home/prototype.py')
p = importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)
CARD, PAYLOAD, OUTPUT = map(Path, sys.argv[1:4])
sys.argv = [sys.argv[0]]

class Layout(unittest.TestCase):
    def test_all_audited_variants(self):
        payload = PAYLOAD.read_bytes()
        for name in p.HASHES:
            with self.subTest(name=name):
                original = (CARD/name).read_bytes()
                data, report = p.patch(original, payload)
                old, new = p.Elf(original), p.Elf(data)
                self.assertEqual(new.phnum, 10)
                self.assertEqual(new.phoff+old.ph[3][2], p.BASE)
                for i in (1,2,5,6,7,8): # PHDR special, all other original properties preserved
                    if old.ph[i][0] != 6:
                        self.assertEqual(old.ph[i], new.ph[i])
                self.assertEqual(new.ph[3],old.ph[3])
                self.assertEqual(new.ph[4][:5],old.ph[4][:5])
                self.assertEqual(new.ph[4][6:],old.ph[4][6:])
                # Stock Home row/icon construction, X/Y handler, state saver untouched.
                for low,high in ((0x37784,0x37a6c),(0x1533c4,0x1533f4),(0x36a00,0x37400),(0x1c544,0x1c75c),(0x2edac,0x2efcc)):
                    self.assertEqual(original[low-0x10000:high-0x10000],data[low-0x10000:high-0x10000])
                for hook in report['hooks']:
                    address = int(hook['address'],16)
                    ins = p.word(data,address-0x10000)
                    delta=ins&0xffffff
                    if delta&0x800000: delta-=1<<24
                    self.assertEqual(address+8+delta*4,int(hook['target'],16))
                allowed = {28,29,30,31,44,45}
                for a,_,_ in p.HOOKS: allowed.update(range(a-0x10000,a-0x10000+4))
                idx=old.sections['.ARM.exidx'][0]
                allowed.update(range(old.shoff+idx*40+12,old.shoff+idx*40+24))
                self.assertTrue(all(a in allowed for a in range(len(original)) if original[a]!=data[a]))
                corrupt = bytearray(original); corrupt[0x8000]^=1
                with self.assertRaisesRegex(ValueError,'unsupported MainUI hash'): p.patch(corrupt,payload)
                with self.assertRaisesRegex(ValueError,'unsupported MainUI hash'): p.patch(data,payload)
                out=OUTPUT/name
                out.mkdir(parents=True,exist_ok=False)
                (out/name).write_bytes(data)
                import json
                (out/'byte-map.json').write_text(json.dumps(report,indent=2)+'\n')
    def test_source_hook_replays(self):
        text=(ROOT/'integration/mainui-home/hooks.S').read_text()
        for replay in ('mov r3,#0','push {r11,lr}','ldr r3,[r11,#-136]'):
            self.assertIn(replay,text)
        for policy in ('bf_writer_is_owned','0x188d0','native_launch_cleanup','native_activation_return'):
            self.assertIn(policy,text)

if __name__=='__main__': unittest.main()
