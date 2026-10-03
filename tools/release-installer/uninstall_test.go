package main

import (
	"bytes"
	"errors"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func completeFixture(t *testing.T) (string, string, string) {
	t.Helper()
	repo := os.Getenv("BF_FIXTURE_REPO")
	dir := os.Getenv("BF_RELEASE_PACKAGE")
	if repo == "" || dir == "" {
		t.Skip("actual audited package required")
	}
	root := root(t)
	mustWrite(t, root, system+"onionVersion/version.txt", []byte("v4.3.1-1\n"))
	for _, n := range names {
		b, e := os.ReadFile(filepath.Join(repo, "build/m6-accepted-fixture/.tmp_update/bin/"+n))
		if e != nil {
			t.Fatal(e)
		}
		mustWrite(t, root, system+"bin/"+n, b)
	}
	b, e := os.ReadFile(filepath.Join(repo, "build/m6-accepted-fixture/.tmp_update/config/better-favorites-return-backup/runtime.sh"))
	if e != nil {
		t.Fatal(e)
	}
	mustWrite(t, root, system+"runtime.sh", b)
	mustWrite(t, root, "Roms/favourite.json", []byte("favorite unchanged"))
	mustWrite(t, root, "Roms/recentlist.json", []byte("history unchanged"))
	mustWrite(t, root, "Saves/game.sav", []byte("save unchanged"))
	mustWrite(t, root, "Themes/shared", []byte("theme unchanged"))
	mustWrite(t, root, ".tmp_update/lib/shared.so", []byte("shared unchanged"))
	recovery := filepath.Join(t.TempDir(), "recovery")
	if e = install(root, dir, recovery, true, true, nil); e != nil {
		t.Fatal(e)
	}
	for n, b := range map[string]string{"settings.conf": "BetterFavoritesSettings1\n0\ngeneration\n", "browser-preferences.conf": "BetterFavoritesBrowserPreferences1\n1\n1\n0\n", "browser-state": "BetterFavoritesBrowserState1\n0\n", "home-entry.conf": "BetterFavoritesHome1\n0\n", "better-favorites.log": "startup\n"} {
		mustWrite(t, root, app+n, []byte(b))
	}
	return root, dir, recovery
}
func TestCompleteUninstall(t *testing.T) {
	for _, kind := range []string{"roundtrip", "missing-recovery", "corrupt-original", "foreign-app", "unknown-app", "foreign-system", "interrupted-cleanup", "partial-cleanup", "foreign-backup", "ambiguous-recovery", "updated-records", "integrations-only-reinstall"} {
		t.Run(kind, func(t *testing.T) {
			root, dir, recovery := completeFixture(t)
			archive := filepath.Join(t.TempDir(), "archive")
			before, _ := read(root, system+"runtime.sh")
			switch kind {
			case "updated-records", "integrations-only-reinstall":
				if kind == "integrations-only-reinstall" {
					if e := restore(root, recovery, dir, true, nil); e != nil {
						t.Fatal(e)
					}
				}
				recovery = filepath.Join(t.TempDir(), "update")
				if e := install(root, dir, recovery, true, true, nil); e != nil {
					t.Fatal(e)
				}

			case "missing-recovery":
				recovery = filepath.Join(t.TempDir(), "absent")
			case "corrupt-original":
				os.WriteFile(filepath.Join(recovery, "files/"+system+"runtime.sh"), []byte("foreign"), 0600)
			case "foreign-app":
				os.WriteFile(filepath.Join(root, app+"better-favorites"), []byte("foreign"), 0600)
			case "unknown-app":
				mustWrite(t, root, app+"keep.txt", []byte("foreign"))
			case "foreign-system":
				os.WriteFile(filepath.Join(root, system+"bin/"+names[0]), []byte("foreign"), 0600)
			case "foreign-backup":
				mustWrite(t, root, returnBackup+"keep.txt", []byte("foreign"))
			case "partial-cleanup":
				if e := restore(root, recovery, dir, true, nil); e != nil {
					t.Fatal(e)
				}
				os.Remove(filepath.Join(root, app+"better-favorites.log"))
				os.Remove(filepath.Join(root, app+"launch.sh"))
			case "ambiguous-recovery":
				second := filepath.Join(filepath.Dir(recovery), "recovery-other")
				if e := install(root, dir, second, true, true, nil); e != nil {
					t.Fatal(e)
				}
				if _, e := discoverRecovery(root, filepath.Dir(recovery)); e == nil {
					t.Fatal("ambiguous selection")
				}
				return
			}
			var hook func(string, string) error
			if kind == "interrupted-cleanup" {
				hook = func(phase, p string) error {
					if phase == "after" && p == app+"better-favorites" {
						return errors.New("interrupted cleanup")
					}
					return nil
				}
			}
			e := completeUninstall(root, recovery, dir, archive, hook)
			okay := kind == "roundtrip" || kind == "partial-cleanup" || kind == "updated-records" || kind == "integrations-only-reinstall"
			if (e == nil) != okay {
				t.Fatalf("%s: %v", kind, e)
			}
			if !okay {
				if _, e := os.Stat(filepath.Join(root, app+"settings.conf")); e != nil {
					t.Fatal("personal files removed on failure")
				}
				if kind != "interrupted-cleanup" {
					after, _ := read(root, system+"runtime.sh")
					if !bytes.Equal(before, after) {
						t.Fatal("preflight changed runtime")
					}
				} else {
					if e := verifyStock(root, dir); e != nil {
						t.Fatal(e)
					}
					if e := completeUninstall(root, recovery, dir, filepath.Join(t.TempDir(), "retry"), nil); e != nil {
						t.Fatal("retry", e)
					}
				}
				return
			}
			if _, e = os.Stat(filepath.Join(root, strings.TrimSuffix(app, "/"))); !os.IsNotExist(e) {
				t.Fatal("app remains", e)
			}
			if e = verifyStock(root, dir); e != nil {
				t.Fatal(e)
			}
			config, _ := os.ReadDir(filepath.Join(root, system+"config"))
			for _, p := range config {
				if strings.HasPrefix(p.Name(), "better-favorites-") {
					t.Fatal("artifact remains", p.Name())
				}
			}
			for p, want := range map[string]string{"Roms/favourite.json": "favorite unchanged", "Roms/recentlist.json": "history unchanged", "Saves/game.sav": "save unchanged", "Themes/shared": "theme unchanged", ".tmp_update/lib/shared.so": "shared unchanged"} {
				b, e := read(root, p)
				if e != nil || string(b) != want {
					t.Fatal("protected", p)
				}
			}
			if _, e = loadRecovery(filepath.Join(archive, "recovery")); e != nil {
				t.Fatal("host recovery", e)
			}
			if e = install(root, dir, filepath.Join(t.TempDir(), "reinstall"), true, true, nil); e != nil {
				t.Fatal("reinstall", e)
			}
		})
	}
}
func TestActionOnlyAndEOF(t *testing.T) {
	for _, action := range []string{"install", "uninstall", "remove-integrations", "export-diagnostics"} {
		t.Run(action, func(t *testing.T) {
			old := os.Stdin
			f, e := os.CreateTemp(t.TempDir(), "input")
			if e != nil {
				t.Fatal(e)
			}
			defer f.Close()
			os.Stdin = f
			defer func() { os.Stdin = old }()
			if e = run([]string{action}); e == nil || !strings.Contains(e.Error(), "input required") {
				t.Fatal("must prompt/fail on EOF", e)
			}
		})
	}
}
