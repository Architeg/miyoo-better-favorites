//go:build windows

package main

import (
	"os"
	"path/filepath"
	"testing"
	"time"
)

func TestWindowsDevicePathsBounded(t *testing.T) {
	for _, p := range []string{`\\.\pipe\bf-fixture`, `\\server\share\x`, `C:\NUL`, `C:\COM2`, `C:\LPT9`, `C:\CONOUT$`, `C:\file:stream`} {
		done := make(chan error, 1)
		go func(p string) { _, e := openRegular(p); done <- e }(p)
		select {
		case e := <-done:
			if e == nil {
				t.Fatal("device path accepted", p)
			}
		case <-time.After(time.Second):
			t.Fatal("device path blocked", p)
		}
	}
}
func TestWindowsReparseRefusal(t *testing.T) {
	r := t.TempDir()
	target := filepath.Join(r, "target")
	os.Mkdir(target, 0700)
	link := filepath.Join(r, "link")
	if e := os.Symlink(target, link); e != nil {
		t.Skip("Windows symlink rights unavailable; reparse qualification remains pending")
	}
	if _, e := join(r, "link/file"); e == nil {
		t.Fatal("reparse ancestor followed")
	}
}

func TestWindowsArchiveDriveSafety(t *testing.T) {
	for _, pair := range [][2]string{{`E:\`, `e:\backup`}, {`E:\`, `E:\folder\archive`}, {`E:\`, `\\server\share\archive`}} {
		if e := outsideCard(pair[0], pair[1]); e == nil {
			t.Fatal("unsafe archive destination", pair)
		}
	}
	if e := outsideCard(`E:\`, `C:\BetterFavorites-Test\archive`); e != nil {
		t.Fatal("different local drive refused", e)
	}
}
