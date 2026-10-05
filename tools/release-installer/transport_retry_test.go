package main

import (
	"bytes"
	"errors"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestIdenticalComputerToolNotRepublished(t *testing.T) {
	r := root(t)
	p := app + "computer/better-favorites-dispatch-windows-386.exe"
	data := []byte("verified-tool")
	mustWrite(t, r, p, data)
	before, _ := os.Stat(filepath.Join(r, p))
	staged := false
	e := transact(r, []change{{p, data, data, 0700, nil, false}, {"owned", nil, []byte("new"), 0700, nil, false}}, func(phase, path string) error {
		if phase == "before" && path == p {
			fs, _ := filepath.Glob(filepath.Join(r, app, "computer/.bf-stage-*"))
			staged = len(fs) > 0
		}
		return nil
	})
	after, _ := os.Stat(filepath.Join(r, p))
	if e != nil || staged || !os.SameFile(before, after) {
		t.Fatal("identical input rewritten", e, staged)
	}
	if !unchangedComputerTool(change{p, data, data, 0700, nil, false}) || unchangedComputerTool(change{system + "runtime.sh", data, data, 0700, nil, true}) {
		t.Fatal("scope")
	}
}
func TestComputerToolLateDisappearanceRollback(t *testing.T) {
	for _, foreign := range []bool{false, true} {
		r := root(t)
		p := app + "computer/better-favorites-dispatch-windows-386.exe"
		data := []byte("verified-tool")
		mustWrite(t, r, p, data)
		mustWrite(t, r, "owned", []byte("old"))
		e := transact(r, []change{{p, data, data, 0700, nil, false}, {"owned", []byte("old"), []byte("new"), 0700, nil, false}}, func(phase, path string) error {
			if phase == "after" && path == "owned" {
				if foreign {
					os.WriteFile(filepath.Join(r, "owned"), []byte("foreign"), 0600)
				}
				return os.Remove(filepath.Join(r, p))
			}
			return nil
		})
		if e == nil || (!foreign && (!strings.Contains(e.Error(), "disappeared") || !errors.Is(e, os.ErrNotExist))) {
			t.Fatal(e)
		}
		want := []byte("old")
		if foreign {
			want = []byte("foreign")
		}
		got, _ := read(r, "owned")
		if !bytes.Equal(got, want) {
			t.Fatal("rollback", string(got))
		}
		if _, e = os.Stat(filepath.Join(r, p)); !errors.Is(e, os.ErrNotExist) {
			t.Fatal("restored antivirus-removed input", e)
		}
	}
}
func TestTransportEarlyReadError(t *testing.T) {
	r := root(t)
	e := os.ErrNotExist
	// With a malformed present manifest, preflight fails before any card publication.
	pkg := t.TempDir()
	mustWrite(t, pkg, "transport.json", []byte("bad"))
	if _, _, e = prepareInstall(r, pkg, true, true); e == nil || !strings.Contains(e.Error(), "transport") {
		t.Fatal(e)
	}
	if _, e = os.Stat(filepath.Join(r, system)); !errors.Is(e, os.ErrNotExist) {
		t.Fatal("preflight wrote card", e)
	}
}
