package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"sort"
	"strings"
	"time"
)

// Exclusive, powered-off card access is required. Hash checks detect conflicts;
// they are not filesystem compare-and-swap and do not serialize another writer.
func outsideCard(root, host string) error {
	a, e := filepath.Abs(root)
	if e != nil {
		return e
	}
	b, e := filepath.Abs(host)
	if e != nil {
		return e
	}
	if runtime.GOOS == "windows" {
		for _, p := range []string{a, b} {
			if strings.HasPrefix(p, `\\`) || len(filepath.VolumeName(p)) != 2 {
				return fmt.Errorf("recovery/archive requires local drive paths")
			}
		}
	}
	// Resolve existing ancestors as well: a host symlink/junction must not
	// disguise a recovery directory on the SD card.
	if real, e := filepath.EvalSymlinks(a); e == nil {
		a = real
	} else if !errors.Is(e, os.ErrNotExist) {
		return e
	}
	if real, e := filepath.EvalSymlinks(filepath.Dir(b)); e == nil {
		b = filepath.Join(real, filepath.Base(b))
	} else if !errors.Is(e, os.ErrNotExist) {
		return e
	}
	if !strings.EqualFold(filepath.VolumeName(a), filepath.VolumeName(b)) {
		return nil
	}
	rel, e := filepath.Rel(a, b)
	if e != nil {
		return e
	}
	if rel == "." || (rel != ".." && !strings.HasPrefix(rel, ".."+string(filepath.Separator))) {
		return fmt.Errorf("verified host archive/recovery must be outside the card: %s", host)
	}
	return nil
}
func loadRecovery(path string) (Recovery, error) {
	var r Recovery
	b, e := read(path, "recovery.json")
	if e != nil {
		return r, e
	}
	if json.Unmarshal(b, &r) != nil || r.Format != 1 || len(r.Files) == 0 {
		return r, fmt.Errorf("invalid recovery: %s", path)
	}
	seen := map[string]bool{}
	for _, s := range r.Files {
		if !safeRel(s.Path) || seen[s.Path] || s.Mode > 0777 || (!s.Integration && !allowedRecoveryApp(s.Path) && !(appPrefix(s.Path) != "" && metadataName(s.Path))) || (s.Integration && !allowedRestorePath(s.Path)) {
			return r, fmt.Errorf("unsafe recovery entry: %s", s.Path)
		}
		seen[s.Path] = true
		for _, part := range []struct{ name, h string }{{"before/", s.Before}, {"after/", s.After}, {"files/", s.Stock}} {
			if part.h == "absent" {
				continue
			}
			b, e := read(path, part.name+s.Path)
			if e != nil || digest(b) != part.h {
				return r, fmt.Errorf("recovery checksum mismatch: %s%s", part.name, s.Path)
			}
		}
	}
	// Metadata archived by a migration remains tied to authenticated sibling
	// records; accepting a metadata pathname alone would weaken recovery checks.
	dirs := map[string]bool{}
	for _, v := range r.Files {
		if metadataName(v.Path) {
			continue
		}
		for d := filepath.ToSlash(filepath.Dir(v.Path)); d != "."; d = filepath.ToSlash(filepath.Dir(d)) {
			dirs[d] = true
		}
	}
	for _, v := range r.Files {
		if !metadataName(v.Path) {
			continue
		}
		for _, part := range []struct{ name, hash string }{{"before/", v.Before}, {"after/", v.After}, {"files/", v.Stock}} {
			if part.hash == "absent" {
				continue
			}
			b, err := read(path, part.name+v.Path)
			if err != nil {
				return r, err
			}
			if err = validateMetadata(v.Path, b, dirs, func(target string) error {
				for _, peer := range r.Files {
					if peer.Path == target && !metadataName(peer.Path) {
						return nil
					}
				}
				return fmt.Errorf("metadata has no recorded companion")
			}); err != nil {
				return r, err
			}
		}
	}

	return r, nil
}
func ownJournal(current, installed []byte) bool {
	if same(current, installed) {
		return true
	}
	var a, b map[string]any
	if json.Unmarshal(current, &a) != nil || json.Unmarshal(installed, &b) != nil || a["status"] != "uninstalled" {
		return false
	}
	a["status"] = "installed"
	return bytes.Equal(encode(a), encode(b))
}
func personalFile(name string, b []byte) bool {
	switch name {
	case "settings.conf":
		return bytes.HasPrefix(b, []byte("BetterFavoritesSettings1\n"))
	case "browser-preferences.conf":
		return bytes.HasPrefix(b, []byte("BetterFavoritesBrowserPreferences1\n"))
	case "browser-state":
		return bytes.HasPrefix(b, []byte("BetterFavoritesBrowserState1\n"))
	case "home-entry.conf":
		return bytes.HasPrefix(b, []byte("BetterFavoritesHome1\n"))
	case "home-diagnostics.conf":
		return string(b) == "BetterFavoritesHomeDiagnostics1\n1\n"
	case "welcome-pending":
		return string(b) == "BetterFavoritesWelcome1\n"
	case "better-favorites.log", "better-favorites.previous.log", "home-diagnostics.log":
		return true // explicit app-owned log names, archived before removal
	}
	return false
}

// Metadata is validated in a second pass: names alone never confer ownership.
func ownedTree(root, rel string, check func(string, []byte) error, missingRecorded ...map[string]bool) ([]change, error) {
	p, e := join(root, rel)
	if e != nil {
		return nil, e
	}
	if _, e = os.Lstat(p); errors.Is(e, os.ErrNotExist) {
		return nil, nil
	}
	var out, metadata []change
	knownDirs := map[string]bool{}
	missing := map[string]bool{}
	if len(missingRecorded) > 0 {
		missing = missingRecorded[0]
		for target := range missing {
			for d := filepath.ToSlash(filepath.Dir(target)); d == rel || strings.HasPrefix(d, rel+"/"); d = filepath.ToSlash(filepath.Dir(d)) {
				knownDirs[d] = true
			}
		}
	}
	e = filepath.Walk(p, func(path string, s os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		relative, err := filepath.Rel(root, path)
		if err != nil {
			return err
		}
		relative = filepath.ToSlash(relative)
		if _, err = join(root, relative); err != nil {
			return err
		}
		if s.IsDir() {
			if relative != rel && strings.Contains(rel, "-backup") {
				return fmt.Errorf("unknown directory preserved: %s", relative)
			}
			return nil
		}
		if metadataName(relative) && (s.Size() < 0 || s.Size() > metadataLimit) {
			return fmt.Errorf("oversized metadata preserved: %s", relative)
		}
		b, err := read(root, relative)
		if err != nil {
			return err
		}
		c := change{relative, b, nil, s.Mode().Perm(), nil, false}
		if metadataName(relative) {
			metadata = append(metadata, c)
			return nil
		}
		if err = check(relative, b); err != nil {
			return err
		}
		for d := filepath.ToSlash(filepath.Dir(relative)); d == rel || strings.HasPrefix(d, rel+"/"); d = filepath.ToSlash(filepath.Dir(d)) {
			knownDirs[d] = true
		}
		out = append(out, c)
		return nil
	})
	if e != nil {
		return nil, e
	}
	for _, c := range metadata {
		if e = validateMetadata(c.Path, c.Before, knownDirs, func(target string) error {
			b, err := read(root, target)
			if errors.Is(err, os.ErrNotExist) && missing[target] {
				return nil
			}
			if err != nil {
				return err
			}
			return check(target, b)
		}); e != nil {
			return nil, e
		}
		out = append(out, c)
	}
	return out, nil
}
func cleanupPlan(root, recovery, dir string) ([]change, error) {
	r, e := loadRecovery(recovery)
	if e != nil {
		return nil, e
	}
	pkg, home, ret, e := loadPackage(dir)
	if e != nil {
		return nil, e
	}
	saved := map[string]Saved{}
	for _, s := range r.Files {
		saved[s.Path] = s
	}
	var out []change
	seen := map[string]bool{}
	add := func(c []change) {
		for _, x := range c {
			if !seen[x.Path] {
				out = append(out, x)
				seen[x.Path] = true
			}
		}
	}
	current := map[string]string{}
	for _, f := range pkg.Files {
		if strings.HasPrefix(f.Path, app) {
			current[f.Path] = f.SHA
		}
	}
	transport, e := transportFiles(dir)
	if e != nil {
		return nil, e
	}
	for _, f := range transport {
		current[app+f.Path] = f.SHA
	}
	expectedDirs := map[string]bool{strings.TrimSuffix(app, "/"): true}
	for p := range current {
		for d := filepath.ToSlash(filepath.Dir(p)); strings.HasPrefix(d, strings.TrimSuffix(app, "/")); d = filepath.ToSlash(filepath.Dir(d)) {
			expectedDirs[d] = true
		}
	}
	for p := range saved {
		if strings.HasPrefix(p, app) {
			for d := filepath.ToSlash(filepath.Dir(p)); strings.HasPrefix(d, strings.TrimSuffix(app, "/")); d = filepath.ToSlash(filepath.Dir(d)) {
				expectedDirs[d] = true
			}
		}
	}
	appPath, _ := join(root, strings.TrimSuffix(app, "/"))
	if e = filepath.Walk(appPath, func(p string, s os.FileInfo, e error) error {
		if errors.Is(e, os.ErrNotExist) {
			return nil
		}
		if e != nil {
			return e
		}
		if s.IsDir() {
			d, _ := filepath.Rel(root, p)
			if !expectedDirs[filepath.ToSlash(d)] {
				return fmt.Errorf("unknown app directory preserved: %s", d)
			}
		}
		return nil
	}); e != nil {
		return nil, e
	}
	// Only a verified journal may authenticate metadata whose payload was already
	// removed by partial cleanup. Present payloads still undergo normal hash checks.
	recorded := map[string]bool{}
	for p, v := range saved {
		if strings.HasPrefix(p, app) && !metadataName(p) && v.After != "absent" {
			recorded[p] = true
		}
	}
	appFiles, e := ownedTree(root, strings.TrimSuffix(app, "/"), func(p string, b []byte) error {
		if s, ok := saved[p]; ok && digest(b) == s.After {
			return nil
		}
		if current[p] != "" && digest(b) == current[p] {
			return nil
		}
		n := strings.TrimPrefix(p, app)
		if personalFile(n, b) {
			return nil
		}
		return fmt.Errorf("unknown/modified app file preserved: %s", p)
	}, recorded)
	if e != nil {
		return nil, e
	}
	add(appFiles)
	legacy, err := legacyCleanup(root, recovery)
	if err != nil {
		return nil, err
	}
	add(legacy)

	// Only recorded integration originals/journals are eligible, not arbitrary files
	// sharing a prefix. Validate every file in each owned directory.
	dirs := map[string]bool{}
	for _, s := range r.Files {
		if !s.Integration || s.Path == system+"runtime.sh" || strings.HasPrefix(s.Path, system+"bin/") {
			continue
		}
		p := s.Path
		if strings.Contains(p, "-backup") {
			dirs[filepath.ToSlash(filepath.Dir(p))] = true
		}
		b, e := optional(root, p)
		if e != nil {
			return nil, e
		}
		if b == nil {
			continue
		}
		if !cleanupOwned(root, recovery, s, b) {
			return nil, fmt.Errorf("modified integration file preserved: %s", p)
		}
		path, _ := join(root, p)
		sinfo, e := os.Stat(path)
		if e != nil {
			return nil, e
		}
		add([]change{{p, b, nil, sinfo.Mode().Perm(), nil, false}})
	}
	// Updates/reinstalls may reuse verified originals without listing those
	// unchanged files in the newest change journal. Derive ONLY their exact paths
	// from this journal's authenticated manifest and compare to its stock copies.
	originals := map[string]string{}
	if _, ok := saved[returnBackup+"manifest.json"]; ok {
		b, e := read(recovery, "files/"+system+"runtime.sh")
		if e != nil || digest(b) != ret.Original {
			return nil, fmt.Errorf("missing verified runtime original")
		}
		originals[returnBackup+"runtime.sh"] = ret.Original
		dirs[strings.TrimSuffix(returnBackup, "/")] = true
	}
	if entry, ok := saved[homeManifest]; ok {
		b, e := read(recovery, "after/"+homeManifest)
		if e != nil || digest(b) != entry.After {
			return nil, fmt.Errorf("missing authenticated Home journal")
		}
		var m struct {
			Version  string            `json:"version"`
			Backup   string            `json:"backup"`
			Original map[string]string `json:"original"`
			Patched  map[string]string `json:"patched"`
		}
		legacyHome, legacyErr := legacyHomeSpec(dir)
		if json.Unmarshal(b, &m) != nil || (m.Version != home.Version && (legacyErr != nil || m.Version != legacyHome.Version)) || !safeRel(m.Backup) || len(strings.Split(m.Backup, "/")) != 3 || !strings.HasPrefix(m.Backup, system+"config/better-favorites-home-backup-") || !bytes.Equal(encode(m.Original), encode(home.Original)) || (!bytes.Equal(encode(m.Patched), encode(home.Patched)) && (legacyErr != nil || !bytes.Equal(encode(m.Patched), encode(legacyHome.Patched)))) {
			return nil, fmt.Errorf("invalid original-backup ownership")
		}
		dirs[m.Backup] = true
		for _, n := range names {
			b, e := read(recovery, "files/"+system+"bin/"+n)
			if e != nil || digest(b) != home.Original[n] {
				return nil, fmt.Errorf("missing verified MainUI original: %s", n)
			}
			originals[m.Backup+"/"+n] = home.Original[n]
		}
	}
	for p, h := range originals {
		b, e := optional(root, p)
		if e != nil {
			return nil, e
		}
		if b == nil {
			continue
		}
		if digest(b) != h {
			return nil, fmt.Errorf("modified retained original preserved: %s", p)
		}
		add([]change{{p, b, nil, 0700, nil, false}})
	}
	for rel := range dirs {
		c, e := ownedTree(root, rel, func(p string, b []byte) error {
			s, ok := saved[p]
			if h, known := originals[p]; known {
				if digest(b) != h {
					return fmt.Errorf("modified retained original preserved: %s", p)
				}
				return nil
			}
			if !ok {
				return fmt.Errorf("unknown backup file preserved: %s", p)
			}
			if !cleanupOwned(root, recovery, s, b) {
				return fmt.Errorf("modified backup preserved: %s", p)
			}
			return nil
		})
		if e != nil {
			return nil, e
		}
		add(c)
	}
	// Only the indexed journal and its verified update lineage are ours.
	ownMirrors, e := ownedRecoveryMirrors(root, recovery)
	if e != nil {
		return nil, e
	}
	// Independently verify every owned recovery mirror before archiving/removing it.
	config, e := join(root, system+"config")
	if e != nil {
		return nil, e
	}
	items, e := os.ReadDir(config)
	if e != nil {
		return nil, e
	}
	for _, entry := range items {
		n := entry.Name()
		if !strings.HasPrefix(n, "better-favorites-") {
			continue
		}
		rel := system + "config/" + n
		if strings.HasPrefix(n, "better-favorites-recovery-") {
			if !ownMirrors[rel] {
				return nil, fmt.Errorf("unrelated recovery directory preserved: %s", rel)
			}
			mirror, e := join(root, rel)
			if e != nil {
				return nil, e
			}
			m, e := loadRecovery(mirror)
			if e != nil {
				return nil, e
			}
			checks := map[string]string{"recovery.json": digest(encode(m))}
			for _, n := range []string{"SHA256SUMS", "RESTORE.txt"} {
				if b, e := optional(mirror, n); e != nil {
					return nil, e
				} else if b != nil {
					checks[n] = digest(b)
				}
			}
			manifest, e := read(mirror, "recovery.json")
			if e != nil {
				return nil, e
			}
			checks["recovery.json"] = digest(manifest)
			for _, s := range m.Files {
				for _, part := range []struct{ n, h string }{{"before/", s.Before}, {"after/", s.After}, {"files/", s.Stock}} {
					if part.h != "absent" {
						checks[part.n+s.Path] = part.h
					}
				}
			}
			c, e := ownedTree(root, rel, func(p string, b []byte) error {
				n := strings.TrimPrefix(p, rel+"/")
				h, ok := checks[n]
				if !ok || digest(b) != h {
					return fmt.Errorf("foreign recovery mirror file preserved: %s", p)
				}
				return nil
			})
			if e != nil {
				return nil, e
			}
			add(c)
		} else if n == "better-favorites-installation.json" {
			if _, e := activeRecovery(root); e != nil {
				return nil, e
			}
			b, e := read(root, rel)
			if e != nil {
				return nil, e
			}
			add([]change{{rel, b, nil, 0600, nil, true}})
		} else if n != "better-favorites-home.json" && !dirs[rel] {
			return nil, fmt.Errorf("unrecorded integration artifact preserved: %s", rel)
		}
	}
	// Recognized lifecycle log outside the app; no shared directories are removed.
	p := system + "logs/better-favorites-return.log"
	b, e := optional(root, p)
	if e != nil {
		return nil, e
	}
	if b != nil {
		add([]change{{p, b, nil, 0600, nil, false}})
	}
	// App-generated exact-record removal backups: preserve the live favorite/history.
	roms, e := join(root, "Roms")
	if e != nil {
		return nil, e
	}
	items, e = os.ReadDir(roms)
	if e != nil && !errors.Is(e, os.ErrNotExist) {
		return nil, e
	}
	for _, item := range items {
		if !strings.HasPrefix(item.Name(), ".favourite.json.better-favorites-backup.") {
			continue
		}
		p := "Roms/" + item.Name()
		b, e := read(root, p)
		if e != nil {
			return nil, e
		}
		for _, line := range bytes.Split(b, []byte{'\n'}) {
			if len(bytes.TrimSpace(line)) == 0 {
				continue
			}
			var record map[string]any
			if json.Unmarshal(line, &record) != nil || record["type"] != float64(5) {
				return nil, fmt.Errorf("unrecognized removal backup preserved: %s", p)
			}
		}
		add([]change{{p, b, nil, 0600, nil, false}})
	}
	// Shared directories remain intact. Consider only exact sidecars of files
	// already authenticated by this cleanup plan, never unrelated directory data.
	ownedFiles := append([]change(nil), out...)
	for _, c := range ownedFiles {
		if metadataName(c.Path) {
			continue
		}
		sidecar := filepath.ToSlash(filepath.Join(filepath.Dir(c.Path), "._"+filepath.Base(c.Path)))
		if seen[sidecar] {
			continue
		}
		b, err := optional(root, sidecar)
		if err != nil {
			return nil, err
		}
		if b == nil {
			continue
		}
		if !appleDouble(b) {
			return nil, fmt.Errorf("ambiguous metadata preserved: %s", sidecar)
		}
		add([]change{{sidecar, b, nil, 0600, nil, false}})
	}

	for _, parent := range []string{system + "config", system + "script", system + "logs"} {
		path, e := join(root, parent)
		if e != nil {
			return nil, e
		}
		items, e := os.ReadDir(path)
		if errors.Is(e, os.ErrNotExist) {
			continue
		}
		if e != nil {
			return nil, e
		}
		for _, item := range items {
			n := strings.ToLower(item.Name())
			p := parent + "/" + item.Name()
			if (strings.Contains(n, "better-favorites") || strings.Contains(n, "better_favorites")) && !seen[p] && !dirs[p] && !ownMirrors[p] {
				return nil, fmt.Errorf("unrecorded project file preserved: %s", p)
			}
		}
	}
	// Guard against project integration files absent from the selected journal.
	for _, p := range []string{homeManifest, system + "script/better_favorites_return.sh", returnBackup + "runtime.sh", returnBackup + "manifest.json", app + "home-integration.conf"} {
		b, e := optional(root, p)
		if e != nil {
			return nil, e
		}
		if b != nil && !seen[p] {
			return nil, fmt.Errorf("unrecorded installed file preserved: %s", p)
		}
	}
	sort.Slice(out, func(i, j int) bool { return out[i].Path < out[j].Path })
	return out, nil
}
func archiveUninstall(root, recovery, host string, changes []change) error {
	if e := outsideCard(root, host); e != nil {
		return e
	}
	if e := os.Mkdir(host, 0700); e != nil {
		return e
	}
	// Copy and verify the supplied recovery even if it was found only on the card.
	r, e := loadRecovery(recovery)
	if e != nil {
		return e
	}
	paths := []string{"recovery.json"}
	for _, s := range r.Files {
		for _, part := range []struct{ n, h string }{{"before/", s.Before}, {"after/", s.After}, {"files/", s.Stock}} {
			if part.h != "absent" {
				paths = append(paths, part.n+s.Path)
			}
		}
	}
	for _, n := range []string{"SHA256SUMS", "RESTORE.txt"} {
		b, e := optional(recovery, n)
		if e != nil {
			return e
		}
		if b != nil {
			paths = append(paths, n)
		}
	}
	checks := map[string]string{}
	for _, p := range paths {
		b, e := read(recovery, p)
		if e != nil {
			return e
		}
		rel := "recovery/" + p
		if e = writeNew(host, rel, b, 0600); e != nil {
			return e
		}
		checks[rel] = digest(b)
	}
	for _, c := range changes {
		if c.Before == nil {
			continue
		}
		rel := "card-before/" + c.Path
		if _, ok := checks[rel]; ok {
			continue
		}
		if e = writeNew(host, rel, c.Before, c.Mode); e != nil {
			return e
		}
		checks[rel] = digest(c.Before)
	}
	if e = writeNew(host, "archive.json", encode(checks), 0600); e != nil {
		return e
	}
	for p, h := range checks {
		b, e := read(host, p)
		if e != nil || digest(b) != h {
			return fmt.Errorf("archive verification failed: %s", p)
		}
	}
	fmt.Println("Verified computer recovery/archive:", host)
	return nil
}
func verifyStock(root, dir string) error {
	_, h, r, e := loadPackage(dir)
	if e != nil {
		return e
	}
	for p, want := range map[string]string{system + "runtime.sh": r.Original, system + "bin/" + names[0]: h.Original[names[0]], system + "bin/" + names[1]: h.Original[names[1]], system + "bin/" + names[2]: h.Original[names[2]], system + "bin/" + names[3]: h.Original[names[3]]} {
		b, e := read(root, p)
		if e != nil || digest(b) != want {
			return fmt.Errorf("stock restoration not verified: %s", p)
		}
	}
	for _, p := range []string{system + "script/better_favorites_return.sh", app + "home-integration.conf"} {
		b, e := optional(root, p)
		if e != nil || b != nil {
			return fmt.Errorf("integration remains: %s", p)
		}
	}
	return nil
}
func completeUninstall(root, recovery, dir, host string, hook func(string, string) error) error {
	// Preflight all restoration and cleanup before any card mutation.
	restoreChanges, e := prepareRestore(root, recovery, dir, true)
	if e != nil {
		return e
	}
	cleanup, e := cleanupPlan(root, recovery, dir)
	if e != nil {
		return e
	}
	all := append(append([]change{}, restoreChanges...), cleanup...)
	if e = archiveUninstall(root, recovery, host, all); e != nil {
		return e
	}
	if e = transact(root, restoreChanges, hook); e != nil {
		return fmt.Errorf("uninstall incomplete; restoration: %w", e)
	}
	if e = verifyRestored(root, recovery, dir); e != nil {
		return fmt.Errorf("uninstall incomplete: %w", e)
	}
	// Regenerate against our verified restored journals, refusing any intervening change.
	after, e := cleanupPlan(root, recovery, dir)
	if e != nil {
		return fmt.Errorf("uninstall incomplete; cleanup: %w", e)
	}
	before := map[string][]byte{}
	for _, c := range cleanup {
		before[c.Path] = c.Before
	}
	changedByRestore := map[string][]byte{}
	for _, c := range restoreChanges {
		changedByRestore[c.Path] = c.After
	}
	for _, c := range after {
		expected, ok := before[c.Path]
		if output, own := changedByRestore[c.Path]; own {
			expected = output
			ok = true
		}
		if !ok || !same(c.Before, expected) {
			return fmt.Errorf("uninstall incomplete; late modification preserved: %s", c.Path)
		}
	}
	var artifacts, mirrors []change
	for _, c := range after {
		if strings.Contains(c.Path, "/better-favorites-recovery-") || c.Path == installationIndex {
			mirrors = append(mirrors, c)
		} else {
			artifacts = append(artifacts, c)
		}
	}
	if e = transact(root, artifacts, hook); e != nil {
		return fmt.Errorf("uninstall incomplete; cleanup files retained/rolled back: %w", e)
	}
	// Remove empty owned directories only. No RemoveAll; a newly inserted file is preserved.
	dirs := map[string]bool{}
	dirs[strings.TrimSuffix(app, "/")] = true
	dirs[strings.TrimSuffix(legacyApp, "/")] = true
	for _, c := range artifacts {
		p := filepath.ToSlash(filepath.Dir(c.Path))
		for strings.HasPrefix(p, strings.TrimSuffix(app, "/")) || strings.HasPrefix(p, strings.TrimSuffix(legacyApp, "/")) || strings.HasPrefix(p, system+"config/better-favorites-") {
			dirs[p] = true
			p = filepath.ToSlash(filepath.Dir(p))
		}
	}
	order := []string{}
	for p := range dirs {
		order = append(order, p)
	}
	sort.Slice(order, func(i, j int) bool { return len(order[i]) > len(order[j]) })
	for _, rel := range order {
		p, e := join(root, rel)
		if e != nil {
			return e
		}
		if e = os.Remove(p); e != nil && !errors.Is(e, os.ErrNotExist) {
			return fmt.Errorf("uninstall incomplete; nonempty/modified directory preserved: %s: %w", rel, e)
		}
	}
	// Verify restoration again before the final recovery cleanup. Host archive already verifies every recovery file.
	if e = verifyRestored(root, recovery, dir); e != nil {
		return e
	}
	if e = transact(root, mirrors, hook); e != nil {
		return fmt.Errorf("uninstall incomplete; portable recovery retained/rolled back: %w", e)
	}

	mirrorDirs := map[string]bool{}
	for _, c := range mirrors {
		if c.Path == installationIndex {
			continue
		}
		for p := filepath.ToSlash(filepath.Dir(c.Path)); strings.HasPrefix(p, system+"config/better-favorites-recovery-"); p = filepath.ToSlash(filepath.Dir(p)) {
			mirrorDirs[p] = true
		}
	}
	order = nil
	for p := range mirrorDirs {
		order = append(order, p)
	}
	sort.Slice(order, func(i, j int) bool { return len(order[i]) > len(order[j]) })
	for _, rel := range order {
		p, e := join(root, rel)
		if e != nil {
			return e
		}
		if e = os.Remove(p); e != nil && !errors.Is(e, os.ErrNotExist) {
			return fmt.Errorf("uninstall incomplete; recovery directory preserved: %s", rel)
		}
	}
	if e = verifyRestored(root, filepath.Join(host, "recovery"), dir); e != nil {
		return e
	}
	paths := []string{}
	for _, c := range after {
		paths = append(paths, c.Path)
	}
	if e = writeNew(host, "uninstall-result.json", encode(map[string]any{"complete": true, "removed": paths, "stock_verified": true}), 0600); e != nil {
		return fmt.Errorf("cleanup verified but result record failed: %w", e)
	}
	fmt.Println("Complete uninstall verified. App, preferences, app logs and owned installation artifacts removed. Stock integrations restored. Computer recovery/archive retained. Game data and shared resources unchanged.")
	return nil
}
func discoverRecovery(root, dir string) (string, error) {
	if p, e := activeRecovery(root); e == nil {
		if _, e = prepareRestore(root, p, dir, true); e != nil {
			return "", e
		}
		return p, nil
	} else if !errors.Is(e, os.ErrNotExist) {
		return "", e
	}
	candidates := []string{}
	seen := map[string]bool{}
	for _, parent := range []string{dir, filepath.Dir(dir), filepath.Join(root, system+"config")} {
		entries, e := os.ReadDir(parent)
		if e != nil {
			continue
		}
		for _, item := range entries {
			if !item.IsDir() || !strings.HasPrefix(item.Name(), "recovery-") && !strings.HasPrefix(item.Name(), "better-favorites-recovery-") && !strings.HasPrefix(item.Name(), "uninstall-archive-") {
				continue
			}
			p := filepath.Join(parent, item.Name())
			if strings.HasPrefix(item.Name(), "uninstall-archive-") {
				p = filepath.Join(p, "recovery")
			}
			if _, e := loadRecovery(p); e != nil {
				continue
			}
			if _, e := prepareRestore(root, p, dir, true); e != nil {
				continue
			}
			r, e := loadRecovery(p)
			if e != nil {
				continue
			}
			matches := true
			for _, s := range r.Files {
				if !s.Integration {
					b, e := optional(root, s.Path)
					if e != nil || (b != nil && digest(b) != s.After) {
						matches = false
						break
					}
				}
			}
			if !matches {
				continue
			}
			data, _ := read(p, "recovery.json")
			identity := digest(data)
			if !seen[identity] {
				candidates = append(candidates, p)
				seen[identity] = true
			}
		}
	}
	if len(candidates) == 1 {
		return candidates[0], nil
	}
	if len(candidates) > 1 {
		return "", fmt.Errorf("multiple valid recovery identities; specify --recovery (none selected)")
	}
	return "", fmt.Errorf("no valid matching recovery found; supply this card's retained recovery folder")
}
func freshUninstallArchive(dir string) string {
	return filepath.Join(dir, "uninstall-archive-"+time.Now().UTC().Format("20060102T150405.000000000Z"))
}
