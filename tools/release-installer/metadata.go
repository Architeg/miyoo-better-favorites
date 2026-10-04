package main

import (
	"encoding/binary"
	"fmt"
	"path/filepath"
	"strings"
)

// Deliberately bounded. Larger or unfamiliar metadata is preserved as a conflict.
const metadataLimit = 4 * 1024 * 1024

func metadataName(p string) bool {
	n := filepath.Base(p)
	return n == ".DS_Store" || strings.HasPrefix(n, "._")
}
func metadataTarget(p string) string {
	n := filepath.Base(p)
	if !strings.HasPrefix(n, "._") || len(n) <= 2 || metadataName(n[2:]) || n[2:] == "." || n[2:] == ".." {
		return ""
	}
	return filepath.ToSlash(filepath.Join(filepath.Dir(p), n[2:]))
}
func appleDouble(b []byte) bool {
	if len(b) < 26 || len(b) > metadataLimit || binary.BigEndian.Uint32(b) != 0x00051607 || binary.BigEndian.Uint32(b[4:]) != 0x00020000 {
		return false
	}
	count := int(binary.BigEndian.Uint16(b[24:]))
	if count < 1 || count > 16 || 26+12*count > len(b) {
		return false
	}
	ids := map[uint32]bool{}
	type span struct{ start, end uint64 }
	var spans []span
	for i := 0; i < count; i++ {
		p := 26 + 12*i
		id := binary.BigEndian.Uint32(b[p:])
		start := uint64(binary.BigEndian.Uint32(b[p+4:]))
		size := uint64(binary.BigEndian.Uint32(b[p+8:]))
		end := start + size
		// Resource fork, Finder information, dates, and file-information entries.
		// Extended attributes from current macOS reside inside Finder information.
		if id < 2 || id > 15 || ids[id] || start < uint64(26+12*count) || end > uint64(len(b)) || (id == 9 && size < 32) {
			return false
		}
		ids[id] = true
		for _, old := range spans {
			if size != 0 && old.start != old.end && start < old.end && old.start < end {
				return false
			}
		}
		spans = append(spans, span{start, end})
	}
	return ids[2] || ids[9]
}

// Validate the Finder buddy allocator and DSDB root, without interpreting or
// applying Finder records. Unknown versions or truncated structures fail closed.
func finderStore(b []byte) bool {
	if len(b) < 36 || len(b) > metadataLimit || binary.BigEndian.Uint32(b) != 1 || string(b[4:8]) != "Bud1" {
		return false
	}
	offset := uint64(binary.BigEndian.Uint32(b[8:]))
	size := uint64(binary.BigEndian.Uint32(b[12:]))
	if offset != uint64(binary.BigEndian.Uint32(b[16:])) || offset < 32 || size < 8 || offset+4+size > uint64(len(b)) {
		return false
	}
	a := b[offset+4 : offset+4+size]
	count := uint64(binary.BigEndian.Uint32(a))
	if count < 2 || count > 65536 {
		return false
	}
	tableEnd := uint64(8) + ((count+255)/256)*1024
	if tableEnd+4 > uint64(len(a)) {
		return false
	}
	blocks := make([][]byte, count)
	for i := uint64(0); i < count; i++ {
		address := binary.BigEndian.Uint32(a[8+4*i:])
		if address == 0 {
			continue
		}
		exp := address & 31
		start := uint64(address & ^uint32(31))
		length := uint64(1) << exp
		if exp < 5 || start+4+length > uint64(len(b)) {
			return false
		}
		blocks[i] = b[start+4 : start+4+length]
	}
	if len(blocks[0]) == 0 {
		return false
	}
	pos := tableEnd
	names := uint64(binary.BigEndian.Uint32(a[pos:]))
	pos += 4
	if names > count {
		return false
	}
	found := false
	for i := uint64(0); i < names; i++ {
		if pos >= uint64(len(a)) {
			return false
		}
		n := uint64(a[pos])
		pos++
		if n == 0 || pos+n+4 > uint64(len(a)) {
			return false
		}
		name := string(a[pos : pos+n])
		pos += n
		id := uint64(binary.BigEndian.Uint32(a[pos:]))
		pos += 4
		if id >= count || len(blocks[id]) < 20 {
			return false
		}
		if name == "DSDB" {
			if found {
				return false
			}
			found = true
			root := uint64(binary.BigEndian.Uint32(blocks[id]))
			if root >= count || len(blocks[root]) < 8 || binary.BigEndian.Uint32(blocks[id][16:]) != 4096 {
				return false
			}
		}
	}
	// All 32 free lists must be structurally present and bounded.
	for i := 0; i < 32; i++ {
		if pos+4 > uint64(len(a)) {
			return false
		}
		n := uint64(binary.BigEndian.Uint32(a[pos:]))
		pos += 4
		if pos+4*n > uint64(len(a)) {
			return false
		}
		pos += 4 * n
	}
	return found
}
func validateMetadata(p string, b []byte, dirs map[string]bool, target func(string) error) error {
	valid := false
	if filepath.Base(p) == ".DS_Store" {
		valid = dirs[filepath.ToSlash(filepath.Dir(p))] && finderStore(b)
	} else if t := metadataTarget(p); t != "" {
		valid = appleDouble(b) && target(t) == nil
	}
	if !valid {
		return fmt.Errorf("ambiguous/invalid OS metadata preserved: %s", p)
	}
	return nil
}
