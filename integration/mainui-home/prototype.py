#!/usr/bin/env python3
"""Offline generator, NOT an installer. Refuse every unrecognized input hash."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

HASHES = {
    'MainUI-283-clean': '6b01276a6292fd7061e0b2576322a52ada65b755562f97bf7656b174d475866f',
    'MainUI-283-expert': '6948b5310dda6513b9e8fa2519d90c5287fc1fd06b4f668a7f2f205406281d28',
    'MainUI-354-clean': '98c85f6c573bdeabd3762e8d9b596f354014e666cc873d0f758cbf3752620c94',
    'MainUI-354-expert': '3bd1fef7fd9bd215bb9e335b6be1101fdff510590ba0d9ca0a9707edc5d9718a',
}
BASE = 0x190000
HOOKS = ((0x27860, 0xe3a03000, 'bf_activation_hook'),
         (0x17fc0, 0xe92d4800, 'bf_writer_hook'),
         (0x188d0, 0xe51b3088, 'bf_publication_hook'),
         (0x15a30, 0xe3a0b000, 'bf_startup_hook'))

def digest(data):
    return hashlib.sha256(data).hexdigest()

def align(n, a):
    return (n+a-1)//a*a

def word(data, off):
    return struct.unpack_from('<I', data, off)[0]

class Elf:
    def __init__(self, data):
        self.data = data
        if data[:7] != b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H', data, 18)[0] != 40:
            raise ValueError('require little-endian ELF32 ARM')
        self.phoff, self.shoff = struct.unpack_from('<II', data, 28)
        _, self.phsize, self.phnum, self.shsize, self.shnum, self.shstr = struct.unpack_from('<6H', data, 40)
        if self.phsize != 32 or self.shsize != 40:
            raise ValueError('unexpected ELF header sizes')
        self.ph = [list(struct.unpack_from('<8I', data, self.phoff+i*32)) for i in range(self.phnum)]
        raw = [list(struct.unpack_from('<10I', data, self.shoff+i*40)) for i in range(self.shnum)]
        names = raw[self.shstr]
        names = data[names[4]:names[4]+names[5]]
        self.sections = {}
        for i, sh in enumerate(raw):
            end = names.find(b'\0', sh[0])
            name = names[sh[0]:end].decode()
            self.sections[name] = (i, sh)
    def section(self, name):
        sh = self.sections[name][1]
        return self.data[sh[4]:sh[4]+sh[5]]
    def symbols(self):
        result = {}
        for _, sh in self.sections.values():
            if sh[1] != 2:
                continue
            stringsh = struct.unpack_from('<10I', self.data, self.shoff+sh[6]*40)
            strings = self.data[stringsh[4]:stringsh[4]+stringsh[5]]
            for off in range(sh[4], sh[4]+sh[5], sh[9]):
                name, value, size, info, other, index = struct.unpack_from('<IIIBBH', self.data, off)
                if index:
                    result[strings[name:strings.find(b'\0', name)].decode()] = value
        return result

def prel_target(value, place):
    delta = value & 0x7fffffff
    if delta & 0x40000000:
        delta -= 0x80000000
    return place+delta

def prel(value, place):
    d = value-place
    if not -(1<<30) <= d < (1<<30):
        raise ValueError('PREL31 overflow')
    return d & 0x7fffffff

def unwind_entries(elf):
    _, sh = elf.sections['.ARM.exidx']
    data = elf.section('.ARM.exidx')
    entries = []
    for i in range(0, len(data), 8):
        a, b = struct.unpack_from('<II', data, i)
        place = sh[3]+i
        entries.append((prel_target(a, place), b if b == 1 or b&0x80000000 else prel_target(b, place+4), b == 1 or bool(b&0x80000000)))
    return entries

def patch(original, payload):
    name = next((n for n, h in HASHES.items() if digest(original) == h), None)
    if name is None:
        raise ValueError('unsupported MainUI hash')
    elf, code = Elf(original), Elf(payload)
    symbols = code.symbols()
    if elf.phnum != 9:
        raise ValueError('unexpected program headers')
    ph = [p[:] for p in elf.ph]
    rw = next(p for p in ph if p[0] == 1 and p[6] == 6)
    if rw[2]+rw[5] != 0x188a58:
        raise ValueError('unexpected native BSS extent')
    bss = code.sections['.bss'][1]
    if bss[3] != 0x188a60 or align(bss[3]+bss[5], 0x10000) > BASE:
        raise ValueError('payload data overlaps RX on 64K pages')
    rw[5] = bss[3]+bss[5]-rw[2]
    segment = bytearray(512) # relocated 10-entry PHDR table occupies first 320 bytes
    for section, (_, sh) in code.sections.items():
        if sh[2]&2 and sh[1] != 8:
            off = sh[3]-BASE
            if off < 512:
                raise ValueError('payload overlaps relocated PHDR table')
            end = off+sh[5]
            segment.extend(bytes(max(0, end-len(segment))))
            segment[off:end] = payload[sh[4]:sh[4]+sh[5]]
    tag = name.encode()+b'\0'
    if len(tag)>24: raise ValueError('variant tag too long')
    tagoff=symbols['bf_variant']-BASE
    segment[tagoff:tagoff+24]=tag.ljust(24,b'\0')
    entries = sorted(unwind_entries(elf)+unwind_entries(code))
    if len({e[0] for e in entries}) != len(entries):
        raise ValueError('duplicate unwind range')
    tableoff = align(len(segment), 8)
    segment.extend(bytes(tableoff-len(segment)))
    for i, (address, value, compact) in enumerate(entries):
        place = BASE+tableoff+i*8
        segment.extend(struct.pack('<II', prel(address, place), value if compact else prel(value, place+4)))
    # Older Onion kernels derive AT_PHDR from first-load bias + e_phoff.
    # Keep that identical to the actual relocated PT_PHDR virtual address.
    fileoff = BASE-0x10000
    if fileoff < len(original) or fileoff%0x10000:
        raise ValueError('program-header placement invalid')
    for p in ph:
        if p[0] == 6: # PT_PHDR retained, points to relocated table
            p[1:6] = [fileoff, BASE, BASE, 320, 320]
        if p[0] == 0x70000001:
            p[1:6] = [fileoff+tableoff, BASE+tableoff, BASE+tableoff, len(entries)*8, len(entries)*8]
    ph.append([1, fileoff, BASE, BASE, len(segment), len(segment), 5, 0x10000])
    for i, p in enumerate(ph):
        struct.pack_into('<8I', segment, i*32, *p)
    result = bytearray(original)
    struct.pack_into('<I', result, 28, fileoff)
    struct.pack_into('<H', result, 44, 10)
    # Existing section metadata follows the relocated exidx for inspection tools.
    idx, sh = elf.sections['.ARM.exidx']
    struct.pack_into('<III', result, elf.shoff+idx*40+12, BASE+tableoff, fileoff+tableoff, len(entries)*8)
    hookmap = []
    for address, expected, target in HOOKS:
        off = address-0x10000
        if word(original, off) != expected:
            raise ValueError('displaced instruction mismatch')
        delta = symbols[target]-address-8
        if delta%4 or not -(1<<25) <= delta < (1<<25):
            raise ValueError('ARM branch overflow')
        replacement = 0xea000000 | ((delta//4)&0xffffff)
        struct.pack_into('<I', result, off, replacement)
        hookmap.append(dict(address=hex(address), file_offset=hex(off), original=hex(expected), replacement=hex(replacement), target=hex(symbols[target]), symbol=target))
    changed = []
    i = 0
    while i < len(original):
        if original[i] == result[i]:
            i += 1
            continue
        start = i
        while i < len(original) and original[i] != result[i]:
            i += 1
        changed.append(dict(offset=hex(start), size=i-start, before=original[start:i].hex(), after=result[start:i].hex()))
    result.extend(bytes(fileoff-len(result)))
    result.extend(segment)
    finalelf = Elf(result)
    assert unwind_entries(finalelf) == entries
    assert finalelf.phoff+0x10000 == BASE
    for page in (4096, 65536):
        ranges = []
        for p in finalelf.ph:
            if p[0] != 1:
                continue
            assert p[1]%p[7] == p[2]%p[7]
            assert p[6] != 7
            ranges.append((p[2]//page*page, align(p[2]+p[5], page), p[6]))
        for a, b in zip(ranges, ranges[1:]):
            assert a[1] <= b[0]
    report = dict(variant=name, original_sha256=digest(original), patched_sha256=digest(result), original_size=len(original), patched_size=len(result), payload_elf_sha256=digest(payload), payload_rx_bytes=len(segment), payload_bss_bytes=bss[5], bss_growth=rw[5]-elf.ph[4][5], rx_mapped_4k=align(len(segment),4096), rx_mapped_64k=align(len(segment),65536), rw_extra_mapped_4k=align(bss[3]+bss[5],4096)-align(0x188a58,4096), rw_extra_mapped_64k=align(bss[3]+bss[5],65536)-align(0x188a58,65536), unwind_entries=len(entries), hooks=hookmap, changed_original_bytes=changed, append=dict(offset=hex(len(original)), size=len(result)-len(original), sha256=digest(result[len(original):])), symbols=symbols)
    return bytes(result), report

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input', type=Path, required=True)
    parser.add_argument('--payload', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    build = Path(__file__).resolve().parents[2]/'build'
    if build not in output.parents and not str(output).startswith(('/tmp/','/private/tmp/')):
        parser.error('host-only output must be a fresh directory under repository build or host tmp')
    data, report = patch(args.input.read_bytes(), args.payload.read_bytes())
    args.output.mkdir(parents=True, exist_ok=False)
    (args.output/report['variant']).write_bytes(data)
    (args.output/'byte-map.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({k:v for k,v in report.items() if k not in ('symbols','changed_original_bytes')}, indent=2))

if __name__ == '__main__':
    main()
