package main

import (
	"bytes"
	"errors"
	"os"
	"path/filepath"
	"runtime"
	"testing"
)

func TestInstallerLogExportBounded(t *testing.T) {
	h := t.TempDir()
	t.Setenv("HOME", h)
	if runtime.GOOS == "windows" {
		t.Setenv("USERPROFILE", h)
	}
	dir := filepath.Join(h, "BetterFavorites-Logs")
	os.Mkdir(dir, 0700)
	for _, name := range []string{"installer-20261005T010101.000000000Z.log", "installer-20261005T010102.000000000Z.log", "installer-20261005T010103.000000000Z.log", "private.log"} {
		os.WriteFile(filepath.Join(dir, name), bytes.Repeat([]byte("x"), 150*1024), 0600)
	}
	got := map[string][]byte{}
	missing := map[string]string{}
	if e := exportInstallerLogs(root(t), func(n string, b []byte) error { got[n] = b; return nil }, missing); e != nil {
		t.Fatal(e)
	}
	if len(got) != 2 || len(got["installer/session-1.log"]) != 128*1024 {
		t.Fatal("limit", len(got))
	}
	want := errors.New("output failed")
	if e := exportInstallerLogs(root(t), func(string, []byte) error { return want }, missing); !errors.Is(e, want) {
		t.Fatal(e)
	}
	if runtime.GOOS != "windows" {
		newest := filepath.Join(dir, "installer-20261005T010103.000000000Z.log")
		os.Remove(newest)
		os.Symlink(filepath.Join(dir, "private.log"), newest)
		got = map[string][]byte{}
		missing = map[string]string{}
		if e := exportInstallerLogs(root(t), func(n string, b []byte) error { got[n] = b; return nil }, missing); e != nil {
			t.Fatal(e)
		}
		if got["installer/session-1.log"] != nil || missing["installer/session-1.log"] == "" {
			t.Fatal("followed symlink")
		}
	}
}
