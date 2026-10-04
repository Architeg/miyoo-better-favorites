package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"
)

func appPrefix(p string) string {
	for _, prefix := range []string{app, legacyApp} {
		if strings.HasPrefix(p, prefix) {
			return prefix
		}
	}
	return ""
}
func allowedRecoveryApp(p string) bool {
	prefix := appPrefix(p)
	if prefix == "" {
		return false
	}
	n := strings.TrimPrefix(p, prefix)
	switch n {
	case "settings.conf", "browser-preferences.conf", "browser-state", "home-entry.conf", "home-diagnostics.conf", "better-favorites.log", "better-favorites.previous.log", "home-diagnostics.log", "home-integration.conf":
		return true
	}
	return allowedAppFile(n) || allowedTransportPath(n)
}
func legacyHomeSpec(dir string) (HomeSpec, error) {
	var h HomeSpec
	b, e := read(dir, "payload/integration/mainui-home/legacy-package.json")
	if e != nil {
		return h, e
	}
	if json.Unmarshal(b, &h) != nil || h.Version != "M6Home1" {
		return h, fmt.Errorf("invalid legacy catalogue")
	}
	return h, nil
}
func acceptedHomeHash(dir, n, h string) bool {
	_, cur, _, e := loadPackage(dir)
	if e != nil {
		return false
	}
	old, e := legacyHomeSpec(dir)
	return e == nil && (h == cur.Patched[n] || h == old.Patched[n])
}
func acceptedReceipt(dir, h, p string) bool {
	_, cur, _, e := loadPackage(dir)
	if e != nil {
		return false
	}
	old, e := legacyHomeSpec(dir)
	return e == nil && (h == digest(receipt(cur)) || h == digest(receipt(old)) || (p == legacyApp+"home-integration.conf" && h == "absent"))
}

// Verified lineage authenticates legacy files. Before bytes permit retry of a
// rolled-back pending transaction; unrelated bytes never gain ownership.
func migrationOwnership(root string) (map[string]map[string]bool, error) {
	out := map[string]map[string]bool{}
	add := func(p, h string) {
		if out[p] == nil {
			out[p] = map[string]bool{}
		}
		out[p][h] = true
	}
	p, e := activeRecovery(root)
	if errors.Is(e, os.ErrNotExist) {
		return out, nil
	}
	if e != nil {
		return nil, e
	}
	mirrors, e := ownedRecoveryMirrors(root, p)
	if e != nil {
		return nil, e
	}
	for rel := range mirrors {
		r, e := loadRecovery(filepath.Join(root, filepath.FromSlash(rel)))
		if e != nil {
			return nil, e
		}
		for _, s := range r.Files {
			add(s.Path, s.After)
			add(s.Path, s.Before)
		}
	}
	return out, nil
}
func prepareMigration(root, dir string) ([]change, error) {
	old, e := join(root, strings.TrimSuffix(legacyApp, "/"))
	if e != nil {
		return nil, e
	}
	if _, e = os.Lstat(old); errors.Is(e, os.ErrNotExist) {
		return nil, nil
	}
	if e != nil {
		return nil, e
	}
	own, e := migrationOwnership(root)
	if e != nil {
		return nil, e
	}
	if len(own) == 0 {
		own, e = legacyCopiedOwnership(root, dir)
		if e != nil {
			return nil, e
		}
	}
	files, e := ownedTree(root, strings.TrimSuffix(legacyApp, "/"), func(p string, b []byte) error {
		n := strings.TrimPrefix(p, legacyApp)
		if !allowedRecoveryApp(p) && !personalFile(n, b) {
			return fmt.Errorf("unknown legacy file preserved: %s", p)
		}
		if !personalFile(n, b) && n != "settings.conf" && n != "home-entry.conf" && n != "browser-state" && n != "browser-preferences.conf" && !own[p][digest(b)] {
			return fmt.Errorf("modified legacy file preserved: %s", p)
		}
		if n == "home-integration.conf" && !acceptedReceipt(dir, digest(b), p) {
			return fmt.Errorf("legacy receipt conflict")
		}
		return nil
	})
	if e != nil {
		return nil, e
	}
	expected := map[string]bool{strings.TrimSuffix(legacyApp, "/"): true}
	for _, f := range files {
		for d := filepath.ToSlash(filepath.Dir(f.Path)); strings.HasPrefix(d, strings.TrimSuffix(legacyApp, "/")); d = filepath.ToSlash(filepath.Dir(d)) {
			expected[d] = true
		}
	}
	e = filepath.Walk(old, func(p string, s os.FileInfo, e error) error {
		if e != nil {
			return e
		}
		if s.IsDir() {
			rel, _ := filepath.Rel(root, p)
			if !expected[filepath.ToSlash(rel)] {
				return fmt.Errorf("unknown legacy directory preserved: %s", rel)
			}
		}
		return nil
	})
	if e != nil {
		return nil, e
	}
	var changes []change
	for _, f := range files {
		n := strings.TrimPrefix(f.Path, legacyApp)
		if personalFile(n, f.Before) || n == "settings.conf" || n == "home-entry.conf" || n == "browser-state" || n == "browser-preferences.conf" {
			target := app + n
			current, e := optional(root, target)
			if e != nil {
				return nil, e
			}
			if current != nil && !bytes.Equal(current, f.Before) {
				return nil, fmt.Errorf("conflicting saved app data preserved: %s", target)
			}
			if current == nil {
				changes = append(changes, change{target, nil, f.Before, f.Mode, nil, false})
			}
		}
		f.Integration = n == "home-integration.conf"
		changes = append(changes, f)
	}
	return changes, nil
}
func removeEmptyLegacy(root string) error {
	p, e := join(root, strings.TrimSuffix(legacyApp, "/"))
	if e != nil {
		return e
	}
	var dirs []string
	e = filepath.Walk(p, func(path string, s os.FileInfo, e error) error {
		if errors.Is(e, os.ErrNotExist) {
			return nil
		}
		if e != nil {
			return e
		}
		if s.IsDir() {
			dirs = append(dirs, path)
		}
		return nil
	})
	if e != nil {
		return e
	}
	sort.Slice(dirs, func(i, j int) bool { return len(dirs[i]) > len(dirs[j]) })
	for _, d := range dirs {
		if e = os.Remove(d); e != nil {
			return fmt.Errorf("legacy directory retained: %s: %w", d, e)
		}
	}
	return nil
}

func acceptedHelperHash(dir, h string) bool {
	_, _, r, e := loadPackage(dir)
	if e != nil {
		return false
	}
	var old ReturnSpec
	b, e := read(dir, "payload/integration/onion-return/legacy-hashes.json")
	return e == nil && json.Unmarshal(b, &old) == nil && (h == r.Helper || h == r.Previous || h == old.Helper || h == old.Previous)
}

func pendingRecovery(root, recovery string) bool {
	b, e := read(root, installationIndex)
	var m Installation
	if e != nil || json.Unmarshal(b, &m) != nil || !m.Pending {
		return false
	}
	p, e := activeRecovery(root)
	return e == nil && filepath.Clean(p) == filepath.Clean(recovery)
}
func cleanupOwned(root, recovery string, s Saved, b []byte) bool {
	expected, e := optional(recovery, "after/"+s.Path)
	if e != nil {
		return false
	}
	if hashOrAbsent(b) == s.After || ((s.Path == homeManifest || s.Path == returnBackup+"manifest.json") && ownJournal(b, expected)) {
		return true
	}
	if !pendingRecovery(root, recovery) {
		return false
	}
	before, e := optional(recovery, "before/"+s.Path)
	return e == nil && (hashOrAbsent(b) == s.Before || ((s.Path == homeManifest || s.Path == returnBackup+"manifest.json") && ownJournal(b, before)))
}
func legacyCleanup(root, recovery string) ([]change, error) {
	own, e := migrationOwnership(root)
	if e != nil {
		return nil, e
	}
	r, e := loadRecovery(recovery)
	if e != nil {
		return nil, e
	}
	for _, s := range r.Files {
		if own[s.Path] == nil {
			own[s.Path] = map[string]bool{}
		}
		own[s.Path][s.After] = true
		if pendingRecovery(root, recovery) {
			own[s.Path][s.Before] = true
		}
	}
	return ownedTree(root, strings.TrimSuffix(legacyApp, "/"), func(p string, b []byte) error {
		n := strings.TrimPrefix(p, legacyApp)
		if own[p][digest(b)] || personalFile(n, b) {
			return nil
		}
		return fmt.Errorf("unknown or changed legacy file preserved: %s", p)
	})
}

// A known extracted RC3 package never reached installation on the reported Mac.
// Its exact payload/transport catalogue can authenticate app bytes ONLY when all
// system binaries are verified stock and no integration is active.
func legacyCopiedOwnership(root, dir string) (map[string]map[string]bool, error) {
	_, h, r, e := loadPackage(dir)
	if e != nil {
		return nil, e
	}
	for n, want := range h.Original {
		b, e := read(root, system+"bin/"+n)
		if e != nil || digest(b) != want {
			return nil, fmt.Errorf("legacy patched installation needs verified portable recovery or matching development restoration")
		}
	}
	b, e := read(root, system+"runtime.sh")
	if e != nil || digest(b) != r.Original {
		return nil, fmt.Errorf("legacy runtime is not verified stock")
	}
	for _, p := range []string{homeManifest, system + "script/better_favorites_return.sh", legacyApp + "home-integration.conf"} {
		b, e := optional(root, p)
		if e != nil || b != nil {
			return nil, fmt.Errorf("legacy integration artifact needs verified recovery: %s", p)
		}
	}
	var spec Package
	b, e = read(dir, "payload/integration/legacy/rc3-copied-package.json")
	if e != nil || json.Unmarshal(b, &spec) != nil || spec.Version != "1.0.0-rc.3" || spec.Commit != "b1c9bd15eb0640133737c4aeebf3b9f180a1358e" {
		return nil, fmt.Errorf("missing audited copied-package catalogue")
	}
	out := map[string]map[string]bool{}
	for _, f := range spec.Files {
		if !strings.HasPrefix(f.Path, legacyApp) || !allowedRecoveryApp(f.Path) {
			return nil, fmt.Errorf("unsafe legacy catalogue")
		}
		if out[f.Path] == nil {
			out[f.Path] = map[string]bool{}
		}
		out[f.Path][f.SHA] = true
	}
	return out, nil
}
