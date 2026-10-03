//go:build windows

package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"syscall"
	"unsafe"
)

var moveFileEx = syscall.NewLazyDLL("kernel32.dll").NewProc("MoveFileExW")

func openRegular(path string) (*os.File, error) {
	if strings.HasPrefix(path, `\\`) || len(filepath.VolumeName(path)) != 2 {
		return nil, fmt.Errorf("only local drive files are supported")
	}
	for _, part := range strings.Split(strings.TrimPrefix(path, filepath.VolumeName(path)), `\`) {
		base := strings.ToUpper(strings.Split(part, ".")[0])
		if strings.HasSuffix(part, " ") || strings.HasSuffix(part, ".") || strings.Contains(part, ":") || base == "CON" || base == "PRN" || base == "AUX" || base == "NUL" || (len(base) == 4 && (strings.HasPrefix(base, "COM") || strings.HasPrefix(base, "LPT")) && base[3] >= '1' && base[3] <= '9') || base == "CONIN$" || base == "CONOUT$" {
			return nil, fmt.Errorf("reserved Windows path refused")
		}
	}
	p, e := syscall.UTF16PtrFromString(path)
	if e != nil {
		return nil, e
	}
	h, e := syscall.CreateFile(p, syscall.GENERIC_READ, syscall.FILE_SHARE_READ, nil, syscall.OPEN_EXISTING, syscall.FILE_FLAG_OPEN_REPARSE_POINT, 0)
	if e != nil {
		return nil, e
	}
	var info syscall.ByHandleFileInformation
	e = syscall.GetFileInformationByHandle(h, &info)
	if e != nil || info.FileAttributes&(syscall.FILE_ATTRIBUTE_REPARSE_POINT|syscall.FILE_ATTRIBUTE_DIRECTORY) != 0 {
		syscall.CloseHandle(h)
		return nil, fmt.Errorf("unsafe file: %s", path)
	}
	return os.NewFile(uintptr(h), path), nil
}
func unsafeAttributes(path string) (bool, error) {
	p, e := syscall.UTF16PtrFromString(path)
	if e != nil {
		return false, e
	}
	a, e := syscall.GetFileAttributes(p)
	return a&syscall.FILE_ATTRIBUTE_REPARSE_POINT != 0, e
}
func replaceFile(from, to string) error {
	a, e := syscall.UTF16PtrFromString(from)
	if e != nil {
		return e
	}
	b, e := syscall.UTF16PtrFromString(to)
	if e != nil {
		return e
	}
	// Same-directory staging. Do NOT allow a cross-volume copy/delete fallback.
	ok, _, err := moveFileEx.Call(uintptr(unsafe.Pointer(a)), uintptr(unsafe.Pointer(b)), 1|8)
	if ok == 0 {
		return err
	}
	return nil
}

// Windows directory FlushFileBuffers is not portable across FAT/readers. Each
// staged file is flushed; safe removal remains necessary. No power-loss promise.
func flushDirectory(path string) error { return nil }
