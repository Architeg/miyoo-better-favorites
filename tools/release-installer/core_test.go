package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
	"runtime"
	"testing"
)

func root(t *testing.T) string {
	t.Helper()
	p := t.TempDir()
	real, e := filepath.EvalSymlinks(p)
	if e != nil {
		t.Fatal(e)
	}
	return real
}
func mustWrite(t *testing.T, r, p string, d []byte) {
	t.Helper()
	if e := writeNew(r, p, d, 0700); e != nil {
		t.Fatal(e)
	}
}
func TestTransactionFailures(t *testing.T) {
	for _, kind := range []string{"before", "after", "foreign", "stage", "directory"} {
		t.Run(kind, func(t *testing.T) {
			r := root(t)
			mustWrite(t, r, "a", []byte("old-a"))
			mustWrite(t, r, "b", []byte("old-b"))
			c := []change{{"a", []byte("old-a"), []byte("new-a"), 0700, nil, false}, {"b", []byte("old-b"), []byte("new-b"), 0700, nil, false}}
			if kind == "stage" {
				c[1].Path = "a/child"
			}
			if kind == "directory" {
				os.Remove(filepath.Join(r, "b"))
				os.Mkdir(filepath.Join(r, "b"), 0700)
			}
			e := transact(r, c, func(phase, p string) error {
				if p == "b" && phase == kind {
					return errors.New("injected disk/write failure")
				}
				if p == "b" && phase == "after" && (kind == "foreign" || kind == "after") {
					if kind == "foreign" {
						os.WriteFile(filepath.Join(r, "a"), []byte("foreign"), 0600)
					}
					return errors.New("injected failure")
				}
				return nil
			})
			if e == nil {
				t.Fatal("expected failure")
			}
			a, _ := read(r, "a")
			want := "old-a"
			if kind == "foreign" {
				want = "foreign"
			}
			if string(a) != want {
				t.Fatalf("unrelated change/rollback: %q", a)
			}
		})
	}
}
func TestPathRefusal(t *testing.T) {
	r := root(t)
	for _, p := range []string{"../x", "/tmp/x", "a/../x", "a\\x", "E:x"} {
		if _, e := join(r, p); e == nil {
			t.Fatal(p)
		}
	}
	if runtime.GOOS != "windows" {
		os.Symlink(t.TempDir(), filepath.Join(r, "link"))
		if _, e := join(r, "link/file"); e == nil {
			t.Fatal("link followed")
		}
	}
	os.Mkdir(filepath.Join(r, "directory"), 0700)
	if _, e := regular(filepath.Join(r, "directory")); e == nil {
		t.Fatal("directory read")
	}
}
func TestActualAuditedOutputs(t *testing.T) {
	repo := os.Getenv("BF_FIXTURE_REPO")
	if repo == "" {
		t.Skip("read-only audited fixtures not supplied")
	}
	d := func(p string) []byte {
		b, e := os.ReadFile(filepath.Join(repo, p))
		if e != nil {
			t.Fatal(e)
		}
		return b
	}
	var h HomeSpec
	json.Unmarshal(d("integration/mainui-home/package.json"), &h)
	payload := d("build/m6-home-review-final/adapter.elf")
	for _, n := range names {
		original := d("build/m6-accepted-fixture/.tmp_update/bin/" + n)
		patched, e := patchHome(original, payload, n)
		if e != nil || digest(patched) != h.Patched[n] {
			t.Fatalf("%s: %v %s", n, e, digest(patched))
		}
	}
	var r ReturnSpec
	json.Unmarshal(d("integration/onion-return/hashes.json"), &r)
	original := d("build/m6-accepted-fixture/.tmp_update/config/better-favorites-return-backup/runtime.sh")
	patched, e := patchRuntime(original, d("integration/onion-return/runtime.patch"))
	if e != nil || digest(patched) != r.Patched {
		t.Fatalf("runtime: %v %s", e, digest(patched))
	}
}
func TestActualPackage(t *testing.T) {
	dir := os.Getenv("BF_RELEASE_PACKAGE")
	repo := os.Getenv("BF_FIXTURE_REPO")
	if dir == "" || repo == "" {
		t.Skip("extracted actual package and audited private fixtures not supplied")
	}
	pkg, h, r, e := loadPackage(dir)
	if e != nil {
		t.Fatal(e)
	}
	seed := func(t *testing.T) string {
		root := root(t)
		mustWrite(t, root, system+"onionVersion/version.txt", []byte("v4.3.1-1\n"))
		for _, n := range names {
			data, e := os.ReadFile(filepath.Join(repo, "build/m6-accepted-fixture/.tmp_update/bin/"+n))
			if e != nil {
				t.Fatal(e)
			}
			mustWrite(t, root, system+"bin/"+n, data)
		}
		data, e := os.ReadFile(filepath.Join(repo, "build/m6-accepted-fixture/.tmp_update/config/better-favorites-return-backup/runtime.sh"))
		if e != nil {
			t.Fatal(e)
		}
		mustWrite(t, root, system+"runtime.sh", data)
		for _, p := range []string{"settings.conf", "browser-preferences.conf", "browser-state", "home-entry.conf"} {
			mustWrite(t, root, app+p, []byte("personal-"+p))
		}
		return root
	}
	t.Run("roundtrip-update-uninstall-reinstall", func(t *testing.T) {
		root := seed(t)
		recovery := filepath.Join(t.TempDir(), "recovery")
		if e = install(root, dir, recovery, true, true, nil); e != nil {
			t.Fatal(e)
		}
		for _, n := range names {
			b, _ := read(root, system+"bin/"+n)
			if digest(b) != h.Patched[n] {
				t.Fatal(n)
			}
		}
		for _, p := range []string{"settings.conf", "browser-preferences.conf", "browser-state", "home-entry.conf"} {
			b, _ := read(root, app+p)
			if string(b) != "personal-"+p {
				t.Fatal("settings changed")
			}
		}
		b, _ := read(root, system+"runtime.sh")
		if digest(b) != r.Patched {
			t.Fatal("runtime mismatch")
		}
		for _, f := range pkg.Files {
			if stringsHasApp(f.Path) {
				b, _ = read(root, f.Path)
				if digest(b) != f.SHA {
					t.Fatal("app payload mismatch")
				}
			}
		}
		second := filepath.Join(t.TempDir(), "second")
		if e = install(root, dir, second, true, true, nil); e != nil {
			t.Fatal(e)
		}
		if e = restore(root, second, dir, false, nil); e != nil {
			t.Fatal(e)
		}
		for _, n := range names {
			b, _ = read(root, system+"bin/"+n)
			if digest(b) != h.Original[n] {
				t.Fatal("stock not restored")
			}
		}
		if e = restore(root, second, dir, true, nil); e != nil {
			t.Fatal("repeat restore:", e)
		}
		third := filepath.Join(t.TempDir(), "third")
		if e = install(root, dir, third, true, true, nil); e != nil {
			t.Fatal("reinstall:", e)
		}
	})
	t.Run("unsupported-before-mutation", func(t *testing.T) {
		root := seed(t)
		os.WriteFile(filepath.Join(root, system+"bin/"+names[0]), []byte("foreign"), 0700)
		before, _ := read(root, system+"runtime.sh")
		recovery := filepath.Join(t.TempDir(), "recovery")
		if e = install(root, dir, recovery, true, true, nil); e == nil {
			t.Fatal("unknown MainUI accepted")
		}
		after, _ := read(root, system+"runtime.sh")
		if !bytes.Equal(before, after) {
			t.Fatal("changed runtime")
		}
		if _, e = os.Stat(recovery); !os.IsNotExist(e) {
			t.Fatal("backup written before preflight")
		}
	})
	t.Run("rollback-foreign-interrupted-restore", func(t *testing.T) {
		root := seed(t)
		recovery := filepath.Join(t.TempDir(), "recovery")
		e = install(root, dir, recovery, true, true, func(phase, p string) error {
			if phase == "after" && p == system+"runtime.sh" {
				return errors.New("injected failure")
			}
			return nil
		})
		if e == nil {
			t.Fatal("expected error")
		}
		b, _ := read(root, system+"runtime.sh")
		if digest(b) != r.Original {
			t.Fatal("rollback failed")
		}
		if e = restore(root, recovery, dir, true, nil); e != nil {
			t.Fatal("interrupted restore:", e)
		}
		recovery = filepath.Join(t.TempDir(), "other")
		if e = install(root, dir, recovery, true, true, nil); e != nil {
			t.Fatal(e)
		}
		os.WriteFile(filepath.Join(root, system+"bin/"+names[0]), []byte("foreign"), 0700)
		if e = restore(root, recovery, dir, true, nil); e == nil {
			t.Fatal("foreign overwritten")
		}
		b, _ = read(root, system+"bin/"+names[0])
		if string(b) != "foreign" {
			t.Fatal("foreign changed")
		}
	})
	t.Run("export-does-not-enable", func(t *testing.T) {
		root := seed(t)
		mustWrite(t, root, app+"better-favorites.log", bytes.Repeat([]byte{'a'}, 70000))
		out := filepath.Join(t.TempDir(), "diagnostics.zip")
		if e = export(root, dir, out); e != nil {
			t.Fatal(e)
		}
		marker, _ := optional(root, app+"home-diagnostics.conf")
		if marker != nil {
			t.Fatal("tracing enabled")
		}
	})
}
func stringsHasApp(p string) bool { return len(p) >= len(app) && p[:len(app)] == app }

func TestEmptyConflictAndLowSpace(t *testing.T) {
	r := root(t)
	changes := []change{{"new", nil, []byte("ours"), 0600, nil, false}}
	e := transact(r, changes, func(phase, p string) error {
		if phase == "before" {
			return os.WriteFile(filepath.Join(r, p), []byte{}, 0600)
		}
		return nil
	})
	if e == nil {
		t.Fatal("empty unrelated file overwritten")
	}
	b, e := read(r, "new")
	if e != nil || len(b) != 0 {
		t.Fatal("foreign empty file changed")
	}
	r = root(t)
	mustWrite(t, r, "old", []byte("old"))
	changes = []change{{"old", []byte("old"), []byte("new"), 0600, nil, false}}
	e = transact(r, changes, func(phase, p string) error {
		if phase == "stage" {
			return errors.New("simulated ENOSPC")
		}
		return nil
	})
	if e == nil {
		t.Fatal("low space not reported")
	}
	b, _ = read(r, "old")
	if string(b) != "old" {
		t.Fatal("replacement before all staging")
	}
}
