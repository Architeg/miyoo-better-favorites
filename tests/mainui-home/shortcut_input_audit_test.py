#!/usr/bin/env python3
"""Read-only exact input-path evidence; not a shortcut implementation."""
import hashlib
import json
from pathlib import Path
import struct
import sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'integration/mainui-home'))
import prototype
originals,patched=map(Path,sys.argv[1:3])
expected={0x1775c:0xebfff5c2, # SDL_WaitEvent, own native input stream
          0x17944:0xe3530065,0x17948:0x0a000045, # L1 key-down branch
          0x17a64:0xe3a0300e, # L1 down -> action 14
          0x17bdc:0xe3530e13,0x17c48:0xe3a03003, # X up -> action 3
          0x17c08:0xe3530f4d,0x17c30:0xe3a03004} # Y up -> action 4
spec=json.loads((ROOT/'integration/mainui-home/package.json').read_text())
for name,sha in prototype.HASHES.items():
 old=(originals/name).read_bytes();new=(patched/name/name).read_bytes()
 assert hashlib.sha256(old).hexdigest()==sha
 assert hashlib.sha256(new).hexdigest()==spec['patched'][name]
 for address,word in expected.items():assert struct.unpack_from('<I',old,address-0x10000)[0]==word,(name,hex(address))
 assert old[0x17744-0x10000:0x17f20-0x10000]==new[0x17744-0x10000:0x17f20-0x10000]
print('Four audited variants: native input conversion intact; L1 down and ordinary X/Y release paths confirmed. No chord suppression established.')
