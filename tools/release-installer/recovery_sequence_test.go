package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestReceiptRepairAfterAppReplacement(t *testing.T) {
	for _, scenario := range []string{"missing", "replaced-folder", "foreign", "manifest", "system", "backup"} {
		t.Run(scenario, func(t *testing.T) {
			card, dir, _ := completeFixture(t)
			os.Remove(filepath.Join(card, app+"welcome-pending"))
			os.Remove(filepath.Join(card, app+"home-integration.conf"))
			switch scenario {
			case "replaced-folder":
				if e := os.RemoveAll(filepath.Join(card, app)); e != nil {
					t.Fatal(e)
				} // isolated fixture only
				pkg, _, _, e := loadPackage(dir)
				if e != nil {
					t.Fatal(e)
				}
				for _, f := range pkg.Files {
					if strings.HasPrefix(f.Path, app) {
						b, e := read(dir, "payload/"+f.Path)
						if e != nil {
							t.Fatal(e)
						}
						mustWrite(t, card, f.Path, b)
					}
				}
				files, e := transportFiles(dir)
				if e != nil {
					t.Fatal(e)
				}
				for _, f := range files {
					b, e := transportData(dir, f.Path)
					if e != nil {
						t.Fatal(e)
					}
					mustWrite(t, card, app+f.Path, b)
				}
			case "foreign":
				replaceFixture(t, card, app+"home-integration.conf", []byte("foreign"))
			case "manifest":
				b, _ := read(card, homeManifest)
				replaceFixture(t, card, homeManifest, append(b, ' '))
			case "system":
				replaceFixture(t, card, system+"bin/"+names[0], []byte("foreign"))
			case "backup":
				rec, _ := activeRecovery(card)
				j, _ := loadRecovery(rec)
				for _, s := range j.Files {
					if strings.Contains(s.Path, "better-favorites-home-backup-") {
						replaceFixture(t, card, s.Path, []byte("foreign"))
						break
					}
				}
			}
			changes, _, err := prepareInstall(card, dir, true, true)
			if scenario != "missing" && scenario != "replaced-folder" {
				if err == nil {
					t.Fatal("conflict accepted")
				}
				return
			}
			if err != nil {
				t.Fatal(err)
			}
			found := false
			for _, c := range changes {
				if c.Path == app+"home-integration.conf" && c.Before == nil && len(c.After) > 0 {
					found = true
				}
				if c.Path == app+"welcome-pending" {
					t.Fatal("dismissed welcome recreated")
				}
			}
			if !found {
				t.Fatal("receipt not repaired")
			}
			if err := install(card, dir, filepath.Join(t.TempDir(), "repair"), true, true, nil); err != nil {
				t.Fatal(err)
			}
			rec, _ := activeRecovery(card)
			if err := completeUninstall(card, rec, dir, filepath.Join(t.TempDir(), "uninstall"), nil); err != nil {
				t.Fatal(err)
			}
			if err := install(card, dir, filepath.Join(t.TempDir(), "fresh"), true, true, nil); err != nil {
				t.Fatal(err)
			}
			if b, e := read(card, app+"welcome-pending"); e != nil || string(b) != "BetterFavoritesWelcome1\n" {
				t.Fatal("fresh onboarding missing", e)
			}
		})
	}
}
func TestRestoreThenMetadataChanges(t *testing.T) {
	for _, scenario := range []string{"changed", "generated", "removed-with-payload", "malformed", "foreign-json", "interrupted"} {
		t.Run(scenario, func(t *testing.T) {
			card, dir, _ := completeFixture(t)
			rec, _ := activeRecovery(card)
			side := system + "config/._better-favorites-home.json"
			if scenario != "generated" {
				replaceFixture(t, card, side, macMetadata(t))
			}
			host := filepath.Join(t.TempDir(), "archive")
			err := completeUninstall(card, rec, dir, host, func(phase, p string) error {
				if phase == "after" && p == homeManifest {
					b := append([]byte{}, macMetadata(t)...)
					b[8] ^= 1
					if scenario == "malformed" {
						b = []byte("not metadata")
					}
					replaceFixture(t, card, side, b)
					if scenario == "foreign-json" {
						replaceFixture(t, card, homeManifest, []byte("foreign"))
					}
				}
				if phase == "after" && p == app+"better-favorites" {
					if scenario == "removed-with-payload" {
						os.Remove(filepath.Join(card, app+"._better-favorites"))
					}
					if scenario == "interrupted" {
						return fmt.Errorf("injected interruption")
					}
				}
				return nil
			})
			if scenario == "malformed" || scenario == "foreign-json" {
				if err == nil {
					t.Fatal("conflict accepted")
				}
				return
			}
			if scenario == "interrupted" {
				if err == nil {
					t.Fatal("no interruption")
				}
				if err = completeUninstall(card, rec, dir, filepath.Join(t.TempDir(), "retry"), nil); err != nil {
					t.Fatal(err)
				}
				return
			}
			if err != nil {
				t.Fatal(err)
			}
			if _, err := os.Stat(filepath.Join(card, side)); !os.IsNotExist(err) {
				t.Fatal("sidecar retained", err)
			}
			if _, err := os.Stat(filepath.Join(host, "metadata-artifacts-before", side)); err != nil {
				t.Fatal("changed metadata not archived", err)
			}
			if err := verifyRestored(card, filepath.Join(host, "recovery"), dir); err != nil {
				t.Fatal(err)
			}
		})
	}
}

func replaceFixture(t *testing.T, root, p string, b []byte) {
	t.Helper()
	if e := os.WriteFile(filepath.Join(root, p), b, 0600); e != nil {
		t.Fatal(e)
	}
}

func TestQuarantinedObsoleteToolNotRecreated(t *testing.T) {
	old := os.Getenv("BF_OLD_RELEASE_PACKAGE")
	if old == "" {
		t.Skip("old and new reviewed packages required")
	}
	current := os.Getenv("BF_RELEASE_PACKAGE")
	t.Setenv("BF_RELEASE_PACKAGE", old)
	card, _, _ := completeFixture(t)
	p := app + "computer/better-favorites-dispatch-windows-386.exe"
	if e := os.Remove(filepath.Join(card, p)); e != nil {
		t.Fatal(e)
	}
	mustWrite(t, card, app+"computer/._better-favorites-dispatch-windows-386.exe", macMetadata(t))
	t.Setenv("BF_RELEASE_PACKAGE", current)
	if e := install(card, current, filepath.Join(t.TempDir(), "update"), true, true, nil); e != nil {
		t.Fatal(e)
	}
	if _, e := os.Stat(filepath.Join(card, p)); !os.IsNotExist(e) {
		t.Fatal("quarantined tool recreated", e)
	}
	rec, e := activeRecovery(card)
	if e != nil {
		t.Fatal(e)
	}
	if e := completeUninstall(card, rec, current, filepath.Join(t.TempDir(), "other-computer"), nil); e != nil {
		t.Fatal(e)
	}
}

// Windows and Mac entry scripts use this same backend. Move the fixture to a
// different host path, fully uninstall, then copy the package as Finder would.
func TestCrossComputerReceiptRoundtrip(t *testing.T) {
	card, dir, _ := completeFixture(t)
	if err := install(card, dir, filepath.Join(t.TempDir(), "update"), true, true, nil); err != nil {
		t.Fatal(err)
	}
	rec, err := activeRecovery(card)
	if err != nil {
		t.Fatal(err)
	}
	journal, err := loadRecovery(rec)
	if err != nil {
		t.Fatal(err)
	}
	found := false
	for _, s := range journal.Files {
		if s.Path == app+"home-integration.conf" {
			found = true
			if s.Before == "absent" || s.Stock != "absent" || !s.Integration {
				t.Fatal("updated receipt restoration bookkeeping", s)
			}
		}
	}
	if !found {
		t.Fatal("missing receipt record")
	}
	host := filepath.Join(t.TempDir(), "other-computer-archive")
	if err = completeUninstall(card, rec, dir, host, nil); err != nil {
		t.Fatal(err)
	}
	if _, err = os.Lstat(filepath.Join(card, app)); !os.IsNotExist(err) {
		t.Fatal("app retained", err)
	}
	if err = verifyRestored(card, filepath.Join(host, "recovery"), dir); err != nil {
		t.Fatal(err)
	}
	pkg, _, _, err := loadPackage(dir)
	if err != nil {
		t.Fatal(err)
	}
	for _, f := range pkg.Files {
		if strings.HasPrefix(f.Path, app) {
			b, e := read(dir, "payload/"+f.Path)
			if e != nil {
				t.Fatal(e)
			}
			mustWrite(t, card, f.Path, b)
		}
	}
	files, err := transportFiles(dir)
	if err != nil {
		t.Fatal(err)
	}
	for _, f := range files {
		b, e := transportData(dir, f.Path)
		if e != nil {
			t.Fatal(e)
		}
		mustWrite(t, card, app+f.Path, b)
	}
	if _, err = os.Lstat(filepath.Join(card, app+"home-integration.conf")); !os.IsNotExist(err) {
		t.Fatal("copied ZIP contains receipt")
	}
	if err = install(card, dir, filepath.Join(t.TempDir(), "mac-install"), true, true, nil); err != nil {
		t.Fatal(err)
	}
}

func TestObsoleteReceiptRequiresStockAndRecovery(t *testing.T) {
	for _, scenario := range []string{"owned", "foreign", "no-recovery", "modified-system"} {
		t.Run(scenario, func(t *testing.T) {
			card, dir, _ := completeFixture(t)
			marker, err := read(card, app+"home-integration.conf")
			if err != nil {
				t.Fatal(err)
			}
			rec, err := activeRecovery(card)
			if err != nil {
				t.Fatal(err)
			}
			if err = restore(card, rec, dir, true, nil); err != nil {
				t.Fatal(err)
			}
			// Reproduce a stale receipt after stock restoration, without assuming
			// this was the historical Windows cause (that log is unavailable).
			mustWrite(t, card, app+"home-integration.conf", marker)
			switch scenario {
			case "foreign":
				replaceFixture(t, card, app+"home-integration.conf", []byte("foreign"))
			case "no-recovery":
				os.Remove(filepath.Join(card, installationIndex))
				os.RemoveAll(rec)
			case "modified-system":
				replaceFixture(t, card, system+"bin/"+names[0], []byte("foreign"))
			}
			if scenario == "no-recovery" { // An unindexed, matching mirror is legitimate evidence;
				// remove only isolated fixture mirrors to test genuinely absent evidence.
				entries, _ := os.ReadDir(filepath.Join(card, system+"config"))
				for _, entry := range entries {
					if strings.HasPrefix(entry.Name(), "better-favorites-recovery-") {
						os.RemoveAll(filepath.Join(card, system+"config", entry.Name()))
					}
				}
			}
			changes, _, err := prepareInstall(card, dir, true, true)
			if scenario != "owned" {
				if err == nil {
					t.Fatal("unverified receipt accepted")
				}
				return
			}
			if err != nil {
				t.Fatal(err)
			}
			found := false
			for _, c := range changes {
				if c.Path == app+"home-integration.conf" {
					found = true
					if !same(c.Before, marker) || !same(c.After, marker) || c.Stock != nil {
						t.Fatal("receipt replacement bookkeeping")
					}
				}
			}
			if !found {
				t.Fatal("receipt reconciliation absent")
			}
			if err = install(card, dir, filepath.Join(t.TempDir(), "reconciled"), true, true, nil); err != nil {
				t.Fatal(err)
			}
		})
	}
}

func TestReceiptRecoveryAlwaysGenerated(t *testing.T) {
	card := root(t)
	marker := []byte("fixture installed receipt")
	for _, path := range []string{app + "home-integration.conf", legacyApp + "home-integration.conf"} {
		// Simulate an older caller incorrectly treating the prior receipt as stock.
		changes := []change{{Path: path, Before: marker, After: marker, Stock: marker, Mode: 0600}}
		host := filepath.Join(t.TempDir(), "recovery")
		if _, err := saveRecovery(card, host, changes, Package{Version: "test"}, nil); err != nil {
			t.Fatal(err)
		}
		journal, err := loadRecovery(host)
		if err != nil {
			t.Fatal(err)
		}
		if journal.Files[0].Stock != "absent" || !journal.Files[0].Integration {
			t.Fatal("receipt saved as original")
		}
		if _, err = os.Stat(filepath.Join(host, "files", path)); !os.IsNotExist(err) {
			t.Fatal("receipt original retained", err)
		}
	}
}
