package main

import (
	"bytes"
	"debug/elf"
	"encoding/binary"
	"fmt"
	"sort"
)

var le = binary.LittleEndian

const base = uint32(0x190000)

func align(n, a uint32) uint32 { return (n + a - 1) / a * a }
func words(data []byte, off, n int) []uint32 {
	out := make([]uint32, n)
	for i := range out {
		out[i] = le.Uint32(data[off+i*4:])
	}
	return out
}
func putWords(data []byte, off int, v []uint32) {
	for i, x := range v {
		le.PutUint32(data[off+i*4:], x)
	}
}

type unwind struct {
	address, value uint32
	compact        bool
}

func prelTarget(v, place uint32) uint32 {
	d := int32(v<<1) >> 1
	return uint32(int64(place) + int64(d))
}
func prel(v, place uint32) (uint32, error) {
	d := int64(v) - int64(place)
	if d < -(1<<30) || d >= 1<<30 {
		return 0, fmt.Errorf("PREL31 overflow")
	}
	return uint32(d) & 0x7fffffff, nil
}
func entries(f *elf.File) ([]unwind, error) {
	s := f.Section(".ARM.exidx")
	if s == nil {
		return nil, fmt.Errorf("missing unwind table")
	}
	data, e := s.Data()
	if e != nil {
		return nil, e
	}
	if len(data)%8 != 0 {
		return nil, fmt.Errorf("bad unwind size")
	}
	var out []unwind
	for i := 0; i < len(data); i += 8 {
		a, b := le.Uint32(data[i:]), le.Uint32(data[i+4:])
		p := uint32(s.Addr) + uint32(i)
		compact := b == 1 || b&0x80000000 != 0
		v := b
		if !compact {
			v = prelTarget(b, p+4)
		}
		out = append(out, unwind{prelTarget(a, p), v, compact})
	}
	return out, nil
}
func patchHome(original, payload []byte, name string) ([]byte, error) {
	f, e := elf.NewFile(bytes.NewReader(original))
	if e != nil {
		return nil, e
	}
	defer f.Close()
	code, e := elf.NewFile(bytes.NewReader(payload))
	if e != nil {
		return nil, e
	}
	defer code.Close()
	if f.Class != elf.ELFCLASS32 || f.Data != elf.ELFDATA2LSB || f.Machine != elf.EM_ARM || len(f.Progs) != 9 || len(original) < 52 {
		return nil, fmt.Errorf("unsupported ELF layout")
	}
	if code.Class != f.Class || code.Data != f.Data || code.Machine != f.Machine {
		return nil, fmt.Errorf("bad payload ELF")
	}
	symbols, e := code.Symbols()
	if e != nil {
		return nil, e
	}
	sym := map[string]uint32{}
	for _, s := range symbols {
		sym[s.Name] = uint32(s.Value)
	}
	phoff := int(le.Uint32(original[28:]))
	if phoff+9*32 > len(original) {
		return nil, fmt.Errorf("bad PHDR extent")
	}
	ph := make([][]uint32, 9)
	for i := range ph {
		ph[i] = words(original, phoff+i*32, 8)
	}
	bss := code.Section(".bss")
	if bss == nil || bss.Addr != 0x188a60 || align(uint32(bss.Addr+bss.Size), 0x10000) > base {
		return nil, fmt.Errorf("bad payload BSS")
	}
	rwFound := false
	for _, p := range ph {
		if p[0] == 1 && p[6] == 6 {
			if p[2]+p[5] != 0x188a58 {
				return nil, fmt.Errorf("bad native BSS")
			}
			p[5] = uint32(bss.Addr+bss.Size) - p[2]
			rwFound = true
		}
	}
	if !rwFound {
		return nil, fmt.Errorf("missing RW segment")
	}
	segment := make([]byte, 512)
	for _, s := range code.Sections {
		if s.Flags&elf.SHF_ALLOC == 0 || s.Type == elf.SHT_NOBITS {
			continue
		}
		if s.Addr < uint64(base)+512 || s.Addr+s.Size > uint64(base)+1024*1024 {
			return nil, fmt.Errorf("bad payload section")
		}
		off := int(s.Addr) - int(base)
		end := off + int(s.Size)
		if end > len(segment) {
			segment = append(segment, make([]byte, end-len(segment))...)
		}
		d, e := s.Data()
		if e != nil {
			return nil, e
		}
		copy(segment[off:end], d)
	}
	tagoff := int(sym["bf_variant"]) - int(base)
	if len(name) >= 24 || tagoff < 512 || tagoff+24 > len(segment) {
		return nil, fmt.Errorf("bad variant tag")
	}
	for i := tagoff; i < tagoff+24; i++ {
		segment[i] = 0
	}
	copy(segment[tagoff:], name)
	a, e := entries(f)
	if e != nil {
		return nil, e
	}
	b, e := entries(code)
	if e != nil {
		return nil, e
	}
	all := append(a, b...)
	sort.Slice(all, func(i, j int) bool { return all[i].address < all[j].address })
	tableoff := align(uint32(len(segment)), 8)
	segment = append(segment, make([]byte, int(tableoff)-len(segment))...)
	for i, x := range all {
		if i > 0 && x.address == all[i-1].address {
			return nil, fmt.Errorf("duplicate unwind range")
		}
		p := base + tableoff + uint32(i*8)
		a, e := prel(x.address, p)
		if e != nil {
			return nil, e
		}
		b := x.value
		if !x.compact {
			b, e = prel(b, p+4)
			if e != nil {
				return nil, e
			}
		}
		segment = le.AppendUint32(segment, a)
		segment = le.AppendUint32(segment, b)
	}
	fileoff := base - 0x10000
	if len(original) > int(fileoff) {
		return nil, fmt.Errorf("bad payload file placement")
	}
	for _, p := range ph {
		if p[0] == 6 {
			copy(p[1:6], []uint32{fileoff, base, base, 320, 320})
		}
		if p[0] == 0x70000001 {
			copy(p[1:6], []uint32{fileoff + tableoff, base + tableoff, base + tableoff, uint32(len(all) * 8), uint32(len(all) * 8)})
		}
	}
	ph = append(ph, []uint32{1, fileoff, base, base, uint32(len(segment)), uint32(len(segment)), 5, 0x10000})
	for i, p := range ph {
		putWords(segment, i*32, p)
	}
	result := append([]byte(nil), original...)
	le.PutUint32(result[28:], fileoff)
	le.PutUint16(result[44:], 10)
	shoff := int(le.Uint32(original[32:]))
	idx := -1
	for i, s := range f.Sections {
		if s.Name == ".ARM.exidx" {
			idx = i
		}
	}
	if idx < 0 || shoff+idx*40+24 > len(result) {
		return nil, fmt.Errorf("bad section table")
	}
	putWords(result, shoff+idx*40+12, []uint32{base + tableoff, fileoff + tableoff, uint32(len(all) * 8)})
	hooks := []struct {
		addr, word uint32
		name       string
	}{{0x27860, 0xe3a03000, "bf_activation_hook"}, {0x17fc0, 0xe92d4800, "bf_writer_hook"}, {0x188d0, 0xe51b3088, "bf_publication_hook"}, {0x15a30, 0xe3a0b000, "bf_startup_hook"}}
	for _, h := range hooks {
		off := h.addr - 0x10000
		if le.Uint32(original[off:]) != h.word {
			return nil, fmt.Errorf("displaced instruction mismatch")
		}
		target, ok := sym[h.name]
		d := int64(target) - int64(h.addr) - 8
		if !ok || d%4 != 0 || d < -(1<<25) || d >= 1<<25 {
			return nil, fmt.Errorf("ARM branch overflow")
		}
		le.PutUint32(result[off:], 0xea000000|uint32(d/4)&0xffffff)
	}
	result = append(result, make([]byte, int(fileoff)-len(result))...)
	result = append(result, segment...)
	// Final output is also checked against the audited per-variant SHA-256 by caller.
	return result, nil
}
