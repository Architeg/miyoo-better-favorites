package main

import (
	"encoding/json"
	"errors"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"testing"
)

func TestLegacyPackageMigration(t *testing.T) {
	dir := os.Getenv("BF_RELEASE_PACKAGE")
	repo := os.Getenv("BF_FIXTURE_REPO")
	old := os.Getenv("BF_LEGACY_PACKAGE")
	if dir == "" || repo == "" || old == "" {
		t.Skip("actual previous package and private fixtures required")
	}
	_, home, ret, e := loadPackage(dir)
	if e != nil {
		t.Fatal(e)
	}
	for _, scenario := range []string{"off", "on", "write-failure", "foreign-old", "conflicting-preference", "unknown-new", "unknown-new-directory", "interrupted", "copied-only", "metadata"} {
		t.Run(scenario, func(t *testing.T) {
			card := root(t)
			mustWrite(t, card, system+"onionVersion/version.txt", []byte("v4.3.1-1\n"))
			for _, n := range names {
				b, e := os.ReadFile(filepath.Join(repo, "build/m6-accepted-fixture/.tmp_update/bin", n))
				if e != nil {
					t.Fatal(e)
				}
				mustWrite(t, card, system+"bin/"+n, b)
			}
			b, e := os.ReadFile(filepath.Join(repo, "build/m6-accepted-fixture/.tmp_update/config/better-favorites-return-backup/runtime.sh"))
			if e != nil {
				t.Fatal(e)
			}
			mustWrite(t, card, system+"runtime.sh", b)
			host := "darwin"
			arch := "amd64"
			if runtime.GOOS == "linux" {
				host = "linux"
			}
			if runtime.GOARCH == "arm64" {
				arch = "arm64"
			}
			tool := filepath.Join(old, "better-favorites-installer-"+host+"-"+arch)
			c := exec.Command(tool, "install", "--sd-root", card, "--powered-off", "--recovery", filepath.Join(t.TempDir(), "old-recovery"))
			if out, e := c.CombinedOutput(); e != nil {
				t.Fatalf("old package install: %v %s", e, out)
			}

			if scenario == "copied-only" {
				c := exec.Command(tool, "uninstall", "--sd-root", card, "--powered-off", "--archive", filepath.Join(t.TempDir(), "old-archive"))
				if out, e := c.CombinedOutput(); e != nil {
					t.Fatalf("old uninstall %v %s", e, out)
				}
				source := filepath.Join(filepath.Dir(old), "full", filepath.FromSlash(legacyApp))
				if e := filepath.Walk(source, func(p string, s os.FileInfo, e error) error {
					if e != nil {
						return e
					}
					if s.IsDir() {
						return nil
					}
					rel, _ := filepath.Rel(source, p)
					b, e := os.ReadFile(p)
					if e != nil {
						return e
					}
					return writeNew(card, legacyApp+filepath.ToSlash(rel), b, s.Mode().Perm())
				}); e != nil {
					t.Fatal(e)
				}
			}
			if scenario == "copied-only" {
				mustWrite(t, card, legacyApp+"._computer", directoryMetadata(t))
			}

			if scenario == "metadata" {
				mustWrite(t, card, legacyApp+"._launch.sh", macMetadata(t))
				mustWrite(t, card, legacyApp+".DS_Store", finderMetadata(t))
			}

			value := "0"
			if scenario == "on" {
				value = "1"
			}
			prefs := map[string][]byte{"settings.conf": []byte("BetterFavoritesSettings1\n" + value + "\n0123456789abcdef0123456789abcdef\n"), "home-entry.conf": []byte("BetterFavoritesHome1\n" + value + "\n"), "browser-state": []byte("BetterFavoritesBrowserState1\n0\n"), "browser-preferences.conf": []byte("BetterFavoritesBrowserPreferences1\n0\n0\n1\n")}
			for n, b := range prefs {
				mustWrite(t, card, legacyApp+n, b)
			}
			os.Remove(filepath.Join(card, legacyApp, "welcome-pending"))
			if scenario == "foreign-old" {
				mustWrite(t, card, legacyApp+"foreign", []byte("keep"))
			}
			if scenario == "conflicting-preference" {
				mustWrite(t, card, app+"home-entry.conf", []byte("different"))
			}
			if scenario == "unknown-new-directory" {
				os.MkdirAll(filepath.Join(card, app, "foreign-directory"), 0700)
			}
			if scenario == "unknown-new" {
				mustWrite(t, card, app+"foreign", []byte("keep"))
			}
			rec := filepath.Join(t.TempDir(), "migration")
			hook := func(phase, p string) error {
				if scenario == "write-failure" && phase == "after" && p == system+"bin/"+names[1] {
					return errors.New("injected")
				}
				return nil
			}

			if scenario == "interrupted" {
				changes, pkg, e := prepareInstall(card, dir, true, true)
				if e != nil {
					t.Fatal(e)
				}
				index, _ := read(card, installationIndex)
				var previous Installation
				json.Unmarshal(index, &previous)
				mirror, e := saveRecovery(card, rec, changes, pkg, &previous)
				if e != nil {
					t.Fatal(e)
				}
				journal, _ := read(card, mirror+"/recovery.json")
				pending := encode(Installation{Format: 1, Recovery: mirror, SHA: digest(journal), Pending: true})
				if e = transact(card, []change{{installationIndex, index, pending, 0600, nil, true}}, nil); e != nil {
					t.Fatal(e)
				}
				cut := 0
				for i, c := range changes {
					if c.Path == system+"bin/"+names[0] {
						cut = i + 1
						break
					}
				}
				if cut == 0 {
					t.Fatal("no MainUI publication")
				}
				if e = transact(card, changes[:cut], nil); e != nil {
					t.Fatal(e)
				}
				recovery, e := activeRecovery(card)
				if e != nil {
					t.Fatal(e)
				}
				if e = completeUninstall(card, recovery, dir, filepath.Join(t.TempDir(), "power-cut-recovery"), nil); e != nil {
					t.Fatal(e)
				}
				for n, h := range home.Original {
					b, _ := read(card, system+"bin/"+n)
					if digest(b) != h {
						t.Fatal("stock after interruption")
					}
				}
				return
			}
			e = install(card, dir, rec, true, true, hook)
			if scenario == "foreign-old" || scenario == "conflicting-preference" || (scenario == "unknown-new" || scenario == "unknown-new-directory") {
				if e == nil {
					t.Fatal("conflict accepted")
				}
				return
			}
			if scenario == "write-failure" {
				if e == nil {
					t.Fatal("failure not injected")
				}
				for n, b := range prefs {
					got, _ := read(card, legacyApp+n)
					if !same(got, b) {
						t.Fatal("rollback lost preference", n)
					}
				}
				e = install(card, dir, filepath.Join(t.TempDir(), "retry"), true, true, nil)
			}
			if e != nil {
				t.Fatal(e)
			}
			if _, e = os.Stat(filepath.Join(card, legacyApp)); !os.IsNotExist(e) {
				t.Fatal("old Apps entry remains", e)
			}
			for n, b := range prefs {
				got, _ := read(card, app+n)
				if !same(got, b) {
					t.Fatal("preference changed", n)
				}
			}
			if _, e = os.Stat(filepath.Join(card, app, "welcome-pending")); !os.IsNotExist(e) {
				t.Fatal("dismissed welcome restored")
			}
			for n, h := range home.Patched {
				b, _ := read(card, system+"bin/"+n)
				if digest(b) != h {
					t.Fatal("new MainUI mismatch")
				}
			}
			b, _ = read(card, system+"script/better_favorites_return.sh")
			if digest(b) != ret.Helper {
				t.Fatal("helper mismatch")
			}
			recovery, e := activeRecovery(card)
			if e != nil {
				t.Fatal(e)
			}
			if scenario == "copied-only" {
				// Model an interrupted cleanup leaving the directory sidecar alone.
				mustWrite(t, card, legacyApp+"._computer", directoryMetadata(t))
			}
			if e = completeUninstall(card, recovery, dir, filepath.Join(t.TempDir(), "other-computer-archive"), nil); e != nil {
				t.Fatal(e)
			}
			for n, h := range home.Original {
				b, _ := read(card, system+"bin/"+n)
				if digest(b) != h {
					t.Fatal("stock mismatch")
				}
			}
		})
	}
}
func TestLegacyCatalogueStockUnchanged(t *testing.T) {
	dir := os.Getenv("BF_RELEASE_PACKAGE")
	if dir == "" {
		t.Skip("package required")
	}
	_, h, _, e := loadPackage(dir)
	if e != nil {
		t.Fatal(e)
	}
	old, e := legacyHomeSpec(dir)
	if e != nil {
		t.Fatal(e)
	}
	a, _ := json.Marshal(h.Original)
	b, _ := json.Marshal(old.Original)
	if string(a) != string(b) {
		t.Fatal("stock guards changed")
	}
}
