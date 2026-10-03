//go:build !windows

package main

import (
	"os"
	"syscall"
)

func openRegular(path string) (*os.File, error) {
	fd, e := syscall.Open(path, syscall.O_RDONLY|syscall.O_NOFOLLOW|syscall.O_NONBLOCK, 0)
	if e != nil {
		return nil, e
	}
	return os.NewFile(uintptr(fd), path), nil
}
func replaceFile(from, to string) error { return os.Rename(from, to) }
func unsafeAttributes(path string) (bool, error) {
	s, e := os.Lstat(path)
	if e != nil {
		return false, e
	}
	return s.Mode()&os.ModeSymlink != 0, nil
}
func flushDirectory(path string) error {
	f, e := os.Open(path)
	if e != nil {
		return e
	}
	defer f.Close()
	e = f.Sync()
	if e == syscall.EINVAL || e == syscall.ENOTSUP {
		return nil
	}
	return e
}
