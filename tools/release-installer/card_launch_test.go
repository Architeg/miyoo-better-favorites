package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

func TestCardLocation(t *testing.T) {
	r := root(t)
	mustWrite(t, r, system+"onionVersion/version.txt", []byte("v4.3.1-1\n"))
	mustWrite(t, r, system+"runtime.sh", []byte("#!/bin/sh\n"))
	os.MkdirAll(filepath.Join(r, system+"bin"), 0700)
	elf := make([]byte, 64)
	copy(elf, []byte{0x7f, 'E', 'L', 'F', 1, 1, 1})
	elf[18] = 40
	mustWrite(t, r, system+"bin/"+names[0], elf)
	p := filepath.Join(r, strings.TrimSuffix(app, "/"))
	os.MkdirAll(p, 0700)
	got, source, e := deriveCard(p)
	if e != nil || got != r || source != p {
		t.Fatal(got, source, e)
	}
	for _, p := range []string{r, filepath.Join(r, "BetterFavoritesTest"), filepath.Join(r, "App/Other")} {
		if _, _, e := deriveCard(p); e == nil {
			t.Fatal("wrong layout", p)
		}
	}
	if runtime.GOOS != "windows" {
		alias := filepath.Join(t.TempDir(), "alias")
		if e = os.Symlink(filepath.Dir(r), alias); e != nil {
			t.Fatal(e)
		}
		if _, _, e = deriveCard(filepath.Join(alias, filepath.Base(r), strings.TrimSuffix(app, "/"))); e == nil {
			t.Fatal("ancestor alias accepted")
		}
	}
	os.WriteFile(filepath.Join(r, system+"runtime.sh"), []byte("unknown"), 0600)
	if _, _, e = deriveCard(p); e == nil {
		t.Fatal("volume name/layout alone accepted")
	}
}
func TestPortableRecoveryMovedCard(t *testing.T) {
	r, dir, host := completeFixture(t)
	p, e := activeRecovery(r)
	if e != nil {
		t.Fatal(e)
	}
	before, _ := read(p, "files/"+system+"runtime.sh")
	// Losing the installation computer must not disable card recovery.
	if e = os.RemoveAll(host); e != nil {
		t.Fatal(e)
	}
	moved := filepath.Join(t.TempDir(), "SD card – ポケット")
	if e = os.Rename(r, moved); e != nil {
		t.Fatal(e)
	}
	selected, e := discoverRecovery(moved, dir)
	if e != nil || !strings.HasPrefix(selected, moved) {
		t.Fatal("portable", selected, e)
	}
	after, _ := read(selected, "files/"+system+"runtime.sh")
	if !bytes.Equal(before, after) {
		t.Fatal("stock original changed")
	}
	if e = completeUninstall(moved, selected, dir, filepath.Join(t.TempDir(), "archive"), nil); e != nil {
		t.Fatal(e)
	}
	if _, e = os.Stat(filepath.Join(moved, app)); !errors.Is(e, os.ErrNotExist) {
		t.Fatal("app remains", e)
	}
}
func TestIndexTamperPreserved(t *testing.T) {
	r, dir, _ := completeFixture(t)
	mustWrite(t, r, "keep", []byte("foreign"))
	os.WriteFile(filepath.Join(r, installationIndex), []byte(`{"format":1,"recovery":"../foreign","sha256":"x"}`), 0600)
	if _, e := discoverRecovery(r, dir); e == nil {
		t.Fatal("unsafe index accepted")
	}
	if e := install(r, dir, filepath.Join(t.TempDir(), "retry"), true, true, nil); e == nil {
		t.Fatal("index conflict replaced")
	}
	b, _ := read(r, "keep")
	if string(b) != "foreign" {
		t.Fatal("foreign changed")
	}
}

func TestCapturedSupportTarget(t *testing.T) {
	root, dir := t.TempDir(), t.TempDir()
	for _, args := range [][]string{{"--sd-root", root, "--package=" + dir}, {"--powered-off"}} {
		if e := capturedTarget(root, dir, args); e != nil {
			t.Fatal(e)
		}
	}
	for _, args := range [][]string{{"--sd-root", t.TempDir()}, {"-sd-root=" + t.TempDir()}, {"--package=" + t.TempDir()}, {"--sd-root"}} {
		if e := capturedTarget(root, dir, args); e == nil {
			t.Fatal("redirect accepted", args)
		}
	}
}

func TestTransportManifestRefusal(t *testing.T) {
	for _, kind := range []string{"valid", "duplicate", "traversal", "mode", "checksum", "incomplete"} {
		t.Run(kind, func(t *testing.T) {
			source := t.TempDir()
			dir := filepath.Join(source, "computer")
			os.Mkdir(dir, 0700)
			m := Transport{Format: 1}
			for _, p := range []string{"Install-Windows.cmd", "Install-macOS.command", "Install-Linux.sh", "Install-Linux.desktop", "computer/package.json", "computer/HOST-BUILDS.json", "computer/a", "computer/b", "computer/c", "computer/d"} {
				b := []byte(p)
				mustWrite(t, source, p, b)
				m.Files = append(m.Files, PayloadFile{p, digest(b), 0644})
			}
			switch kind {
			case "duplicate":
				m.Files = append(m.Files, m.Files[0])
			case "traversal":
				m.Files[0].Path = "../foreign"
			case "mode":
				m.Files[0].Mode = 07777
			case "checksum":
				m.Files[0].SHA = "bad"
			case "incomplete":
				m.Files = m.Files[1:]
			}
			b, _ := json.Marshal(m)
			mustWrite(t, dir, "transport.json", b)
			_, e := transportFiles(dir)
			if (e == nil) != (kind == "valid") {
				t.Fatal(kind, e)
			}
		})
	}
}
func TestLateCleanupPreservesPortableRecovery(t *testing.T) {
	root, dir, _ := completeFixture(t)
	recovery, e := activeRecovery(root)
	if e != nil {
		t.Fatal(e)
	}
	hook := func(phase, p string) error {
		if phase == "after" && p == app+"better-favorites" {
			return os.Mkdir(filepath.Join(root, app+"foreign-empty-directory"), 0700)
		}
		return nil
	}
	if e = completeUninstall(root, recovery, dir, filepath.Join(t.TempDir(), "archive"), hook); e == nil {
		t.Fatal("late unknown directory accepted")
	}
	if _, e = activeRecovery(root); e != nil {
		t.Fatal("retry recovery removed", e)
	}
	if e = verifyStock(root, dir); e != nil {
		t.Fatal(e)
	}
	if e = os.Remove(filepath.Join(root, app+"foreign-empty-directory")); e != nil {
		t.Fatal(e)
	}
	if e = completeUninstall(root, recovery, dir, filepath.Join(t.TempDir(), "retry"), nil); e != nil {
		t.Fatal("retry", e)
	}
}

func TestInterruptedInstallRetryLineage(t *testing.T) {
	root, dir, _ := completeFixture(t)
	if e := install(root, dir, filepath.Join(t.TempDir(), "failed"), true, true, func(phase, p string) error {
		if phase == "after" && p == app+"better-favorites" {
			return errors.New("interrupt")
		}
		return nil
	}); e == nil {
		t.Fatal("injected failure lost")
	}
	if e := install(root, dir, filepath.Join(t.TempDir(), "retry"), true, true, nil); e != nil {
		t.Fatal("retry", e)
	}
	recovery, e := activeRecovery(root)
	if e != nil {
		t.Fatal(e)
	}
	if e = completeUninstall(root, recovery, dir, filepath.Join(t.TempDir(), "archive"), nil); e != nil {
		t.Fatal("owned attempt lineage", e)
	}
}
func TestUnrelatedValidRecoveryPreserved(t *testing.T) {
	root, dir, _ := completeFixture(t)
	recovery, _ := activeRecovery(root)
	foreign := filepath.Join(root, system+"config/better-favorites-recovery-unrelated")
	os.Mkdir(foreign, 0700)
	e := filepath.Walk(recovery, func(p string, s os.FileInfo, e error) error {
		if e != nil {
			return e
		}
		if s.IsDir() {
			return nil
		}
		rel, _ := filepath.Rel(recovery, p)
		b, e := os.ReadFile(p)
		if e != nil {
			return e
		}
		return writeNew(foreign, filepath.ToSlash(rel), b, 0600)
	})
	if e != nil {
		t.Fatal(e)
	}
	before, _ := read(root, system+"runtime.sh")
	if e = completeUninstall(root, recovery, dir, filepath.Join(t.TempDir(), "archive"), nil); e == nil {
		t.Fatal("unrelated valid recovery adopted")
	}
	after, _ := read(root, system+"runtime.sh")
	if !bytes.Equal(before, after) {
		t.Fatal("preflight mutated runtime")
	}
	if _, e = loadRecovery(foreign); e != nil {
		t.Fatal("unrelated recovery altered", e)
	}
}

func TestAppOnlyCannotDiscardInstalledRecovery(t *testing.T) {
	root, dir, _ := completeFixture(t)
	before, _ := read(root, installationIndex)
	runtimeBefore, _ := read(root, system+"runtime.sh")
	for _, flags := range [][2]bool{{false, false}, {false, true}, {true, false}} {
		if e := install(root, dir, filepath.Join(t.TempDir(), "refused"), flags[0], flags[1], nil); e == nil {
			t.Fatal("installed recovery discarded", flags)
		}
		after, _ := read(root, installationIndex)
		runtimeAfter, _ := read(root, system+"runtime.sh")
		if !bytes.Equal(before, after) || !bytes.Equal(runtimeBefore, runtimeAfter) {
			t.Fatal("refused update mutated card")
		}
	}
	recovery, _ := activeRecovery(root)
	if e := completeUninstall(root, recovery, dir, filepath.Join(t.TempDir(), "archive"), nil); e != nil {
		t.Fatal(e)
	}
}
