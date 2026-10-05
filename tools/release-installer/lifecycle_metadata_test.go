package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestConsumedWelcomeMetadata(t *testing.T) {
	for _, scenario := range []string{"existing", "consumed", "malformed", "orphan", "symlink"} {
		t.Run(scenario, func(t *testing.T) {
			card, dir, _ := completeFixture(t)
			marker := app + "welcome-pending"
			mustWrite(t, card, app+"._welcome-pending", macMetadata(t))
			if b, err := read(card, marker); err != nil || string(b) != "BetterFavoritesWelcome1\n" {
				t.Fatal("fixture marker", err)
			}
			if scenario != "existing" {
				if err := os.Remove(filepath.Join(card, marker)); err != nil {
					t.Fatal(err)
				}
			}
			switch scenario {
			case "malformed":
				if err := os.WriteFile(filepath.Join(card, app+"._welcome-pending"), []byte("invalid"), 0600); err != nil {
					t.Fatal(err)
				}
			case "orphan":
				mustWrite(t, card, app+"._unrelated-consumed", macMetadata(t))
			case "symlink":
				if err := os.Symlink("launch.sh", filepath.Join(card, marker)); err != nil {
					t.Skip(err)
				}
			}
			_, _, err := prepareInstall(card, dir, true, true)
			if scenario == "malformed" || scenario == "orphan" || scenario == "symlink" {
				if err == nil {
					t.Fatal("ambiguous sidecar accepted")
				}
				return
			}
			if err != nil {
				t.Fatal("preflight", err)
			}
			if err := install(card, dir, filepath.Join(t.TempDir(), "update"), true, true, nil); err != nil {
				t.Fatal("update", err)
			}
			rec, err := activeRecovery(card)
			if err != nil {
				t.Fatal(err)
			}
			journal, err := loadRecovery(rec)
			if err != nil || journal.Lifecycle[marker] == "" {
				t.Fatal("lifecycle evidence not retained", err)
			}
			if scenario == "consumed" {
				if _, err := os.Stat(filepath.Join(card, marker)); !os.IsNotExist(err) {
					t.Fatal("welcome recreated", err)
				}
			}
			// Portable journal, no host-specific metadata API; inject an interruption,
			// then retry from a different computer archive destination.
			if err := completeUninstall(card, rec, dir, filepath.Join(t.TempDir(), "interrupted"), func(phase, p string) error {
				if phase == "after" && p == app+"better-favorites" {
					return fmt.Errorf("interrupted")
				}
				return nil
			}); err == nil {
				t.Fatal("interruption missed")
			}
			if err := completeUninstall(card, rec, dir, filepath.Join(t.TempDir(), "other-computer"), nil); err != nil {
				t.Fatal("portable retry", err)
			}
		})
	}
}

func TestOlderBackupLineage(t *testing.T) {
	for _, scenario := range []string{"retained", "empty", "foreign"} {
		t.Run(scenario, func(t *testing.T) {
			card, dir, old := completeFixture(t)
			journal, err := loadRecovery(old)
			if err != nil {
				t.Fatal(err)
			}
			var backups []string
			for _, s := range journal.Files {
				if strings.Contains(s.Path, "better-favorites-home-backup-") {
					backups = append(backups, s.Path)
				}
			}
			if len(backups) != 4 {
				t.Fatal("old originals missing")
			}
			// Model interrupted/rolled-back publication followed by stock restoration
			// and a new install: oldest journal retains ownership, latest omits paths.
			if err := restore(card, old, dir, true, nil); err != nil {
				t.Fatal(err)
			}
			if scenario == "foreign" {
				if err := os.WriteFile(filepath.Join(card, backups[0]), []byte("foreign"), 0600); err != nil {
					t.Fatal(err)
				}
				if _, _, err := prepareInstall(card, dir, true, true); err == nil {
					t.Fatal("update accepted foreign backup")
				}
				if err := completeUninstall(card, old, dir, filepath.Join(t.TempDir(), "refused"), nil); err == nil {
					t.Fatal("uninstall accepted foreign")
				}
				if b, _ := read(card, backups[0]); string(b) != "foreign" {
					t.Fatal("foreign overwritten")
				}
				return
			}
			if scenario == "retained" {
				if err := install(card, dir, filepath.Join(t.TempDir(), "interrupted-install"), true, true, func(phase, p string) error {
					if phase == "after" && p == system+"bin/"+names[0] {
						return fmt.Errorf("publication interrupted")
					}
					return nil
				}); err == nil {
					t.Fatal("interruption missed")
				}
			}
			if err := install(card, dir, filepath.Join(t.TempDir(), "new-install"), true, true, nil); err != nil {
				t.Fatal("reinstall", err)
			}
			if scenario == "empty" {
				for _, p := range backups {
					if err := os.Remove(filepath.Join(card, p)); err != nil {
						t.Fatal(err)
					}
				}
			}
			rec, err := activeRecovery(card)
			if err != nil {
				t.Fatal(err)
			}
			if err := completeUninstall(card, rec, dir, filepath.Join(t.TempDir(), "archive"), nil); err != nil {
				t.Fatal("lineage uninstall", err)
			}
			if _, err := os.Stat(filepath.Dir(filepath.Join(card, backups[0]))); !os.IsNotExist(err) {
				t.Fatal("old directory remains", err)
			}
		})
	}
}

func TestActualConsumedSidecarReadOnly(t *testing.T) {
	p := os.Getenv("BF_ACTUAL_WELCOME_SIDECAR")
	if p == "" {
		t.Skip("optional real Mac evidence")
	}
	b, err := regular(p)
	if err != nil || !appleDouble(b) {
		t.Fatal("real sidecar", err)
	}
	if digest(b) != "e6da019da34bf58d85311aaaa41b04ac222214846a474e00c0d7c20089906aed" {
		t.Fatal("evidence changed")
	}
}

// Optional real-card preflight is strictly read-only: no install/uninstall call.
func TestMountedLifecyclePreflightReadOnly(t *testing.T) {
	card := os.Getenv("BF_READONLY_CARD")
	if card == "" {
		t.Skip("optional mounted evidence")
	}
	dir := os.Getenv("BF_RELEASE_PACKAGE")
	if dir == "" {
		t.Fatal("package required")
	}
	rec, err := activeRecovery(card)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := prepareRestore(card, rec, dir, true); err != nil {
		t.Fatal("restore preflight", err)
	}
	if _, err := cleanupPlan(card, rec, dir); err != nil {
		t.Fatal("cleanup preflight", err)
	}
	if _, _, err := prepareInstall(card, dir, true, true); err != nil {
		t.Fatal("install preflight", err)
	}
}

func TestMalformedPreferencesRecordedWithoutRewrite(t *testing.T) {
	card, dir, _ := completeFixture(t)
	p := app + "browser-preferences.conf"
	if err := os.WriteFile(filepath.Join(card, p), []byte("malformed but personal"), 0600); err != nil {
		t.Fatal(err)
	}
	before, _ := os.Stat(filepath.Join(card, p))
	if err := install(card, dir, filepath.Join(t.TempDir(), "update"), true, true, nil); err != nil {
		t.Fatal(err)
	}
	after, _ := os.Stat(filepath.Join(card, p))
	if !os.SameFile(before, after) {
		t.Fatal("preference rewritten")
	}
	b, _ := read(card, p)
	if string(b) != "malformed but personal" {
		t.Fatal("preference changed")
	}
	rec, err := activeRecovery(card)
	if err != nil {
		t.Fatal(err)
	}
	if err := completeUninstall(card, rec, dir, filepath.Join(t.TempDir(), "archive"), nil); err != nil {
		t.Fatal(err)
	}
}
