//go:build windows

package main

import (
	"encoding/binary"
	"fmt"
	"syscall"
	"unsafe"
)

func detectWindows() (host, error) {
	var h host
	// OSVERSIONINFOEXW, fixed Windows ABI (284 bytes), not GetVersionEx.
	var version [284]byte
	binary.LittleEndian.PutUint32(version[:4], 284)
	rtl := syscall.NewLazyDLL("ntdll.dll").NewProc("RtlGetVersion")
	if e := rtl.Find(); e != nil {
		return h, e
	}
	status, _, _ := rtl.Call(uintptr(unsafe.Pointer(&version[0])))
	if status != 0 {
		return h, fmt.Errorf("RtlGetVersion status %#x", status)
	}
	if binary.LittleEndian.Uint32(version[16:20]) != 2 {
		return h, fmt.Errorf("not Windows NT")
	}
	h.major = binary.LittleEndian.Uint32(version[4:8])
	h.minor = binary.LittleEndian.Uint32(version[8:12])
	h.build = binary.LittleEndian.Uint32(version[12:16])
	kernel := syscall.NewLazyDLL("kernel32.dll")
	wow := kernel.NewProc("IsWow64Process2")
	if wow.Find() == nil {
		var process, native uint16
		current, _, _ := kernel.NewProc("GetCurrentProcess").Call()
		ok, _, e := wow.Call(current, uintptr(unsafe.Pointer(&process)), uintptr(unsafe.Pointer(&native)))
		if ok == 0 {
			return h, fmt.Errorf("IsWow64Process2: %v", e)
		}
		h.native = native
	} else {
		// Older x86/x64 Windows lacks IsWow64Process2. Native SYSTEM_INFO starts
		// with the same architecture WORD in either ABI; oversized buffer is safe.
		var info [64]byte
		p := kernel.NewProc("GetNativeSystemInfo")
		if e := p.Find(); e != nil {
			return h, e
		}
		p.Call(uintptr(unsafe.Pointer(&info[0])))
		switch binary.LittleEndian.Uint16(info[:2]) {
		case 0:
			h.native = 0x14c
		case 9:
			h.native = 0x8664
		default:
			return h, fmt.Errorf("unknown legacy native architecture")
		}
	}
	return h, nil
}
