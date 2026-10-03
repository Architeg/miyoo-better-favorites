package main

import (
	"bytes"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"time"
)

const app = "App/BetterFavoritesTest/"
const system = ".tmp_update/"
const homeManifest = system + "config/better-favorites-home.json"
const returnBackup = system + "config/better-favorites-return-backup/"

var names = []string{"MainUI-283-clean", "MainUI-283-expert", "MainUI-354-clean", "MainUI-354-expert"}

type HomeSpec struct {
	Version  string            `json:"version"`
	Payload  string            `json:"payload_sha256"`
	Original map[string]string `json:"originals"`
	Patched  map[string]string `json:"patched"`
}
type ReturnSpec struct {
	Version  string `json:"version"`
	Blob     string `json:"original_git_blob"`
	Original string `json:"original_sha256"`
	Patched  string `json:"patched_sha256"`
	Helper   string `json:"helper_sha256"`
}
type PayloadFile struct {
	Path string `json:"path"`
	SHA  string `json:"sha256"`
	Mode uint32 `json:"mode"`
}
type Package struct {
	Format  int           `json:"format"`
	Version string        `json:"version"`
	Commit  string        `json:"commit"`
	Files   []PayloadFile `json:"files"`
}
type change struct {
	Path          string
	Before, After []byte
	Mode          os.FileMode
	Stock         []byte
	Integration   bool
}
type Saved struct {
	Path        string `json:"path"`
	Before      string `json:"before_sha256"`
	After       string `json:"after_sha256"`
	Stock       string `json:"stock_sha256"`
	Mode        uint32 `json:"mode"`
	Integration bool   `json:"integration"`
}
type Recovery struct {
	Format  int     `json:"format"`
	Version string  `json:"version"`
	Commit  string  `json:"commit"`
	Files   []Saved `json:"files"`
}

func digest(b []byte) string { h := sha256.Sum256(b); return hex.EncodeToString(h[:]) }
func same(a, b []byte) bool  { return (a == nil) == (b == nil) && bytes.Equal(a, b) }
func hashOrAbsent(b []byte) string {
	if b == nil {
		return "absent"
	}
	return digest(b)
}
func encode(v any) []byte { d, _ := json.MarshalIndent(v, "", "  "); return append(d, '\n') }
func regular(p string) ([]byte, error) {
	f, e := openRegular(p)
	if e != nil {
		return nil, e
	}
	defer f.Close()
	s, e := f.Stat()
	if e != nil {
		return nil, e
	}
	if !s.Mode().IsRegular() || s.Size() > 64*1024*1024 {
		return nil, fmt.Errorf("unsafe or oversized file: %s", p)
	}
	data, e := io.ReadAll(io.LimitReader(f, 64*1024*1024+1))
	if e != nil {
		return nil, e
	}
	if len(data) > 64*1024*1024 {
		return nil, fmt.Errorf("file grew past bound: %s", p)
	}
	return data, nil
}
func snapshot(p string) ([]byte, error) {
	d, e := regular(p)
	if errors.Is(e, os.ErrNotExist) {
		return nil, nil
	}
	return d, e
}
func safeRel(rel string) bool {
	return rel != "" && !strings.Contains(rel, "\\") && !strings.Contains(rel, ":") && filepath.ToSlash(filepath.Clean(rel)) == rel && !strings.HasPrefix(rel, "/") && rel != ".." && !strings.HasPrefix(rel, "../")
}
func join(root, rel string) (string, error) {
	if !safeRel(rel) {
		return "", fmt.Errorf("unsafe relative path: %s", rel)
	}
	p := root
	parts := strings.Split(rel, "/")
	for i, part := range append([]string{""}, parts...) {
		if part != "" {
			p = filepath.Join(p, part)
		}
		bad, e := unsafeAttributes(p)
		if errors.Is(e, os.ErrNotExist) {
			continue
		}
		if e != nil {
			return "", e
		}
		if bad {
			return "", fmt.Errorf("link/reparse point refused: %s", p)
		}
		s, e := os.Lstat(p)
		if e != nil {
			return "", e
		}
		if i < len(parts) && !s.IsDir() {
			return "", fmt.Errorf("not a directory: %s", p)
		}
	}
	return p, nil
}
func pathFor(root, rel string) (string, error) { return join(root, rel) }
func read(root, rel string) ([]byte, error) {
	p, e := join(root, rel)
	if e != nil {
		return nil, e
	}
	return regular(p)
}
func optional(root, rel string) ([]byte, error) {
	p, e := join(root, rel)
	if e != nil {
		return nil, e
	}
	return snapshot(p)
}
func mkdirParents(root, rel string) error {
	parent := filepath.ToSlash(filepath.Dir(rel))
	if parent == "." {
		return nil
	}
	if _, e := join(root, parent); e != nil {
		return e
	}
	return os.MkdirAll(filepath.Join(root, filepath.FromSlash(parent)), 0700)
}
func writeNew(root, rel string, data []byte, mode os.FileMode) error {
	if e := mkdirParents(root, rel); e != nil {
		return e
	}
	p, e := join(root, rel)
	if e != nil {
		return e
	}
	f, e := os.OpenFile(p, os.O_WRONLY|os.O_CREATE|os.O_EXCL, mode)
	if e != nil {
		return e
	}
	_, e = f.Write(data)
	if e == nil {
		e = f.Sync()
	}
	ce := f.Close()
	if e == nil {
		e = ce
	}
	if e != nil {
		return e
	}
	d, e := regular(p)
	if e != nil {
		return e
	}
	if !bytes.Equal(d, data) {
		return fmt.Errorf("backup verification failed: %s", rel)
	}
	return flushDirectory(filepath.Dir(p))
}
func stage(p string, data []byte, mode os.FileMode) (string, error) {
	f, e := os.CreateTemp(filepath.Dir(p), ".bf-stage-")
	if e != nil {
		return "", e
	}
	name := f.Name()
	ok := false
	defer func() {
		f.Close()
		if !ok {
			os.Remove(name)
		}
	}()
	if e = f.Chmod(mode); e != nil {
		return "", e
	}
	if _, e = f.Write(data); e != nil {
		return "", e
	}
	if e = f.Sync(); e != nil {
		return "", e
	}
	if e = f.Close(); e != nil {
		return "", e
	}
	d, e := regular(name)
	if e != nil || !bytes.Equal(d, data) {
		return "", fmt.Errorf("stage verification failed: %s", p)
	}
	ok = true
	return name, nil
}

// Offline, exclusive card access is required. Hash rechecks detect conflicts;
// they are not atomic compare-and-swap and do not eliminate concurrent writers.
// Fault hooks are test arguments, never production environment switches.
func transact(root string, changes []change, hook func(string, string) error) (err error) {
	stages := map[string]string{}
	published := []change{}
	defer func() {
		for _, p := range stages {
			os.Remove(p)
		}
		if err == nil {
			return
		}
		var conflicts []string
		for i := len(published) - 1; i >= 0; i-- {
			c := published[i]
			p, e := join(root, c.Path)
			if e != nil {
				conflicts = append(conflicts, c.Path)
				continue
			}
			cur, e := snapshot(p)
			if e != nil || !same(cur, c.After) {
				conflicts = append(conflicts, c.Path)
				continue
			}
			if c.Before == nil {
				e = os.Remove(p)
			} else {
				var temp string
				temp, e = stage(p, c.Before, c.Mode)
				if e == nil {
					cur, e = snapshot(p)
					if e == nil && !same(cur, c.After) {
						e = fmt.Errorf("late rollback conflict")
					}
					if e == nil {
						e = replaceFile(temp, p)
					}
					os.Remove(temp)
				}
			}
			if e == nil {
				cur, e = snapshot(p)
				if e == nil && !same(cur, c.Before) {
					e = fmt.Errorf("rollback readback mismatch")
				}
			}
			if e != nil {
				conflicts = append(conflicts, c.Path)
			}
		}
		err = fmt.Errorf("%w; preserved rollback conflicts: %v", err, conflicts)
	}()
	for _, c := range changes {
		if hook != nil {
			if e := hook("stage", c.Path); e != nil {
				return e
			}
		}
		if e := mkdirParents(root, c.Path); e != nil {
			return e
		}
		p, e := join(root, c.Path)
		if e != nil {
			return e
		}
		if c.After != nil {
			temp, e := stage(p, c.After, c.Mode)
			if e != nil {
				return e
			}
			stages[c.Path] = temp
		}
	}
	for _, c := range changes {
		p, e := join(root, c.Path)
		if e != nil {
			return e
		}
		cur, e := snapshot(p)
		if e != nil {
			return e
		}
		if !same(cur, c.Before) {
			return fmt.Errorf("changed file preserved: %s", c.Path)
		}
		if hook != nil {
			if e = hook("before", c.Path); e != nil {
				return e
			}
		}
		cur, e = snapshot(p)
		if e != nil || !same(cur, c.Before) {
			return fmt.Errorf("late conflict preserved: %s", c.Path)
		}
		// Register before mutation: even an OS error with partially applied publication
		// is inspected and rolled back ONLY if our exact output is present.
		published = append(published, c)
		if c.After == nil {
			if cur != nil {
				e = os.Remove(p)
			}
		} else {
			e = replaceFile(stages[c.Path], p)
		}
		if e != nil {
			return e
		}
		if e = flushDirectory(filepath.Dir(p)); e != nil {
			return e
		}
		if hook != nil {
			if e = hook("after", c.Path); e != nil {
				return e
			}
		}
		cur, e = snapshot(p)
		if e != nil || !same(cur, c.After) {
			return fmt.Errorf("publication verification failed: %s", c.Path)
		}
	}
	return nil
}
func loadPackage(dir string) (Package, HomeSpec, ReturnSpec, error) {
	var pkg Package
	var home HomeSpec
	var ret ReturnSpec
	d, e := read(dir, "package.json")
	if e != nil {
		return pkg, home, ret, e
	}
	if e = json.Unmarshal(d, &pkg); e != nil {
		return pkg, home, ret, e
	}
	if pkg.Format != 1 || pkg.Version != "1.0.0-rc.1" || len(pkg.Commit) != 40 {
		return pkg, home, ret, fmt.Errorf("unsupported package")
	}
	seen := map[string]bool{}
	for _, f := range pkg.Files {
		if seen[f.Path] || !safeRel(f.Path) || f.Mode > 0777 {
			return pkg, home, ret, fmt.Errorf("bad package entry")
		}
		if strings.HasPrefix(f.Path, app) && !allowedAppFile(strings.TrimPrefix(f.Path, app)) {
			return pkg, home, ret, fmt.Errorf("personal/unexpected app file excluded: %s", f.Path)
		}
		seen[f.Path] = true
		d, e = read(dir, "payload/"+f.Path)
		if e != nil || digest(d) != f.SHA {
			return pkg, home, ret, fmt.Errorf("package checksum mismatch: %s", f.Path)
		}
	}
	d, e = read(dir, "payload/integration/mainui-home/package.json")
	if e == nil {
		e = json.Unmarshal(d, &home)
	}
	if e != nil {
		return pkg, home, ret, e
	}
	d, e = read(dir, "payload/integration/onion-return/hashes.json")
	if e == nil {
		e = json.Unmarshal(d, &ret)
	}
	if e != nil {
		return pkg, home, ret, e
	}
	if home.Version != "M6Home1" || ret.Version != "v4.3.1-1" || len(home.Original) != 4 || len(home.Patched) != 4 {
		return pkg, home, ret, fmt.Errorf("bad integration catalogue")
	}
	return pkg, home, ret, nil
}
func receipt(h HomeSpec) []byte {
	s := "BetterFavoritesHomeInstalled1\n" + h.Version + "\n"
	for _, n := range names {
		s += h.Patched[n] + "\n"
	}
	return []byte(s)
}
func patchRuntime(original, patch []byte) ([]byte, error) {
	lines := bytes.SplitAfter(original, []byte{'\n'})
	var result []byte
	position := 0
	pattern := regexp.MustCompile(`^@@ -(\d+),0 \+\d+(?:,\d+)? @@$`)
	p := strings.Split(string(patch), "\n")
	for i := 0; i < len(p); i++ {
		m := pattern.FindStringSubmatch(p[i])
		if m == nil {
			if strings.HasPrefix(p[i], "@@") {
				return nil, fmt.Errorf("unsupported patch hunk")
			}
			continue
		}
		n, _ := strconv.Atoi(m[1])
		if n < position || n > len(lines) {
			return nil, fmt.Errorf("bad patch position")
		}
		for position < n {
			result = append(result, lines[position]...)
			position++
		}
		for i+1 < len(p) && strings.HasPrefix(p[i+1], "+") {
			i++
			result = append(result, []byte(p[i][1:]+"\n")...)
		}
	}
	for position < len(lines) {
		result = append(result, lines[position]...)
		position++
	}
	return result, nil
}
func cardVersion(root string) error {
	v, e := read(root, system+"onionVersion/version.txt")
	if e != nil {
		return e
	}
	if strings.TrimSpace(string(v)) != "v4.3.1-1" {
		return fmt.Errorf("unsupported Onion version; use app-only/support guide")
	}
	return nil
}
func add(root string, out *[]change, rel string, after, stock []byte, mode os.FileMode, integration bool) error {
	before, e := optional(root, rel)
	if e != nil {
		return e
	}
	*out = append(*out, change{rel, before, after, mode, stock, integration})
	return nil
}
func prepareInstall(root, dir string, homeOn, returnOn bool) ([]change, Package, error) {
	pkg, h, r, e := loadPackage(dir)
	if e != nil {
		return nil, pkg, e
	}
	if e = cardVersion(root); e != nil {
		return nil, pkg, e
	}
	var changes []change
	runtime, e := read(root, system+"runtime.sh")
	if e != nil {
		return nil, pkg, e
	}
	rh := digest(runtime)
	if rh != r.Original && rh != r.Patched {
		return nil, pkg, fmt.Errorf("unknown runtime preserved")
	}
	if rh == r.Patched {
		helper, e := read(root, system+"script/better_favorites_return.sh")
		if e != nil || digest(helper) != r.Helper {
			return nil, pkg, fmt.Errorf("changed return helper preserved")
		}
		orig, e := read(root, returnBackup+"runtime.sh")
		if e != nil || digest(orig) != r.Original {
			return nil, pkg, fmt.Errorf("return original backup mismatch")
		}
		m, e := read(root, returnBackup+"manifest.json")
		var manifest map[string]any
		if e != nil || json.Unmarshal(m, &manifest) != nil || manifest["status"] != "installed" || manifest["helper_sha256"] != r.Helper {
			return nil, pkg, fmt.Errorf("return manifest mismatch")
		}
	}
	// App-only package entries preserve personal files by an explicit payload list.
	for _, f := range pkg.Files {
		if !strings.HasPrefix(f.Path, app) {
			continue
		}
		data, e := read(dir, "payload/"+f.Path)
		if e != nil {
			return nil, pkg, e
		}
		if e = add(root, &changes, f.Path, data, nil, os.FileMode(f.Mode), false); e != nil {
			return nil, pkg, e
		}
	}
	if returnOn && rh == r.Original {
		helper, e := read(dir, "payload/integration/onion-return/better_favorites_return.sh")
		if e != nil || digest(helper) != r.Helper {
			return nil, pkg, fmt.Errorf("helper package mismatch")
		}
		old, e := optional(root, system+"script/better_favorites_return.sh")
		if e != nil || old != nil {
			return nil, pkg, fmt.Errorf("existing return helper preserved")
		}
		oldOriginal, e := optional(root, returnBackup+"runtime.sh")
		if e != nil {
			return nil, pkg, e
		}
		if oldOriginal != nil && digest(oldOriginal) != r.Original {
			return nil, pkg, fmt.Errorf("return backup conflict")
		}
		patch, e := read(dir, "payload/integration/onion-return/runtime.patch")
		if e != nil {
			return nil, pkg, e
		}
		patched, e := patchRuntime(runtime, patch)
		if e != nil || digest(patched) != r.Patched {
			return nil, pkg, fmt.Errorf("generated runtime mismatch")
		}
		oldManifest, e := optional(root, returnBackup+"manifest.json")
		if e != nil {
			return nil, pkg, e
		}
		if oldManifest != nil {
			var m map[string]any
			if json.Unmarshal(oldManifest, &m) != nil || m["status"] != "uninstalled" || m["original_sha256"] != r.Original || m["patched_sha256"] != r.Patched {
				return nil, pkg, fmt.Errorf("return manifest conflict")
			}
		}
		// Backup/journal files must exist and verify BEFORE executable replacements.
		if oldOriginal == nil {
			if e = add(root, &changes, returnBackup+"runtime.sh", runtime, nil, 0700, true); e != nil {
				return nil, pkg, e
			}
		}
		m := map[string]any{"version": r.Version, "original_git_blob": r.Blob, "original_sha256": r.Original, "patched_sha256": r.Patched, "helper_sha256": r.Helper, "mode": 448, "status": "installed"}
		if e = add(root, &changes, system+"script/better_favorites_return.sh", helper, nil, 0700, true); e != nil {
			return nil, pkg, e
		}
		if e = add(root, &changes, system+"runtime.sh", patched, runtime, 0700, true); e != nil {
			return nil, pkg, e
		}
		if e = add(root, &changes, returnBackup+"manifest.json", encode(m), oldManifest, 0600, true); e != nil {
			return nil, pkg, e
		}
	}
	if returnOn && rh == r.Patched {
		original, e := read(root, returnBackup+"runtime.sh")
		if e != nil {
			return nil, pkg, e
		}
		helper, e := read(root, system+"script/better_favorites_return.sh")
		if e != nil {
			return nil, pkg, e
		}
		manifest, e := read(root, returnBackup+"manifest.json")
		if e != nil {
			return nil, pkg, e
		}
		for _, item := range []struct {
			p            string
			after, stock []byte
			mode         os.FileMode
		}{
			{system + "runtime.sh", runtime, original, 0700},
			{system + "script/better_favorites_return.sh", helper, nil, 0700},
			{returnBackup + "manifest.json", manifest, manifest, 0600},
		} {
			if e = add(root, &changes, item.p, item.after, item.stock, item.mode, true); e != nil {
				return nil, pkg, e
			}
		}
	}
	if homeOn {
		payload, e := read(dir, "payload/integration/mainui-home/adapter.elf")
		if e != nil || digest(payload) != h.Payload {
			return nil, pkg, fmt.Errorf("adapter mismatch")
		}
		prior, e := optional(root, homeManifest)
		if e != nil {
			return nil, pkg, e
		}
		var manifest map[string]json.RawMessage
		backupRel := ""
		installed := false
		if prior != nil {
			if json.Unmarshal(prior, &manifest) != nil {
				return nil, pkg, fmt.Errorf("home manifest invalid")
			}
			var status, version string
			json.Unmarshal(manifest["status"], &status)
			json.Unmarshal(manifest["version"], &version)
			json.Unmarshal(manifest["backup"], &backupRel)
			if version != h.Version || (status != "installed" && status != "uninstalled") || !strings.HasPrefix(backupRel, system+"config/better-favorites-home-backup-") || !safeRel(backupRel) {
				return nil, pkg, fmt.Errorf("home manifest conflict")
			}
			installed = status == "installed"
		}
		if backupRel == "" {
			backupRel = system + "config/better-favorites-home-backup-" + time.Now().UTC().Format("20060102T150405.000000000Z")
		}
		modes := map[string]int{}
		for _, n := range names {
			current, e := read(root, system+"bin/"+n)
			if e != nil {
				return nil, pkg, e
			}
			expected := h.Original[n]
			if installed {
				expected = h.Patched[n]
			}
			if digest(current) != expected {
				return nil, pkg, fmt.Errorf("unknown or conflicting MainUI preserved: %s", n)
			}
			original := current
			if prior != nil {
				original, e = read(root, backupRel+"/"+n)
				if e != nil || digest(original) != h.Original[n] {
					return nil, pkg, fmt.Errorf("home original backup mismatch: %s", n)
				}
			}
			patched, e := patchHome(original, payload, n)
			if e != nil || digest(patched) != h.Patched[n] {
				return nil, pkg, fmt.Errorf("generated MainUI mismatch: %s (%v)", n, e)
			}
			modes[n] = 448
			if prior == nil {
				if e = add(root, &changes, backupRel+"/"+n, original, nil, 0700, true); e != nil {
					return nil, pkg, e
				}
			}
			if e = add(root, &changes, system+"bin/"+n, patched, original, 0700, true); e != nil {
				return nil, pkg, e
			}
		}
		marker, e := optional(root, app+"home-integration.conf")
		if e != nil || installed && !bytes.Equal(marker, receipt(h)) || !installed && marker != nil {
			return nil, pkg, fmt.Errorf("home receipt conflict")
		}
		if e = add(root, &changes, app+"home-integration.conf", receipt(h), nil, 0600, true); e != nil {
			return nil, pkg, e
		}
		m := map[string]any{"version": h.Version, "status": "installed", "backup": backupRel, "original": h.Original, "patched": h.Patched, "modes": modes, "runtime_sha256": rh}
		if e = add(root, &changes, homeManifest, encode(m), nil, 0600, true); e != nil {
			return nil, pkg, e
		}
	}
	// Retain backup changes first, then app, helper, runtime, MainUI and metadata.
	sort.SliceStable(changes, func(i, j int) bool { return installRank(changes[i].Path) < installRank(changes[j].Path) })
	return changes, pkg, nil
}
func installRank(p string) int {
	if strings.Contains(p, "-backup") && strings.HasSuffix(p, ".json") == false {
		return 0
	}
	if strings.HasPrefix(p, app) && !strings.HasSuffix(p, "home-integration.conf") {
		return 1
	}
	if strings.HasPrefix(p, system+"script/") {
		return 2
	}
	if p == system+"runtime.sh" {
		return 3
	}
	if strings.HasPrefix(p, system+"bin/") {
		return 4
	}
	return 5
}
func saveRecovery(root, host string, changes []change, pkg Package) (string, error) {
	if _, e := os.Lstat(host); !errors.Is(e, os.ErrNotExist) {
		return "", fmt.Errorf("recovery directory must be NEW: %s", host)
	}
	if e := os.Mkdir(host, 0700); e != nil {
		return "", e
	}
	recovery := Recovery{1, pkg.Version, pkg.Commit, nil}
	var sums strings.Builder
	for _, c := range changes {
		stock := c.Stock
		if c.Integration && c.Path == system+"runtime.sh" && stock == nil {
			stock = c.Before
		}
		saved := Saved{c.Path, hashOrAbsent(c.Before), hashOrAbsent(c.After), hashOrAbsent(stock), uint32(c.Mode), c.Integration}
		recovery.Files = append(recovery.Files, saved)
		for _, part := range []struct {
			name string
			data []byte
		}{{"before/", c.Before}, {"files/", stock}, {"after/", c.After}} {
			if part.data == nil {
				continue
			}
			if e := writeNew(host, part.name+c.Path, part.data, c.Mode); e != nil {
				return "", e
			}
			sums.WriteString(digest(part.data) + "  " + part.name + c.Path + "\n")
		}
	}
	if e := writeNew(host, "recovery.json", encode(recovery), 0600); e != nil {
		return "", e
	}
	if e := writeNew(host, "SHA256SUMS", []byte(sums.String()), 0600); e != nil {
		return "", e
	}
	guide := []byte("POWER OFF, remove the card and mount it on a computer. MainUI/Terminal are not needed.\nRestore uses this recovery.json and verifies every backup and destination.\nManual fallback: copy files/.tmp_update/runtime.sh (if present) and files/.tmp_update/bin/MainUI-* to the identical SD paths. Show hidden files. Verify SHA256SUMS first.\nDo not copy before/ or after/ blindly. Do not delete the app before restoring the integrations.\nAfter originals are verified, remove only the owned App/BetterFavoritesTest/home-integration.conf and .tmp_update/script/better_favorites_return.sh. Preserve all backups, preferences and data.\nIf destinations contain unrelated changes, preserve them and seek support before manual replacement. Reboot after restoration.\n")
	if e := writeNew(host, "RESTORE.txt", guide, 0600); e != nil {
		return "", e
	}
	// Mirror the same verified originals/journal on-card before the first executable
	// publication. It survives an interrupted multi-file install; keep the host copy.
	cardRel := system + "config/better-favorites-recovery-" + time.Now().UTC().Format("20060102T150405.000000000Z")
	for _, c := range changes {
		stock := c.Stock
		if stock != nil {
			if e := writeNew(root, cardRel+"/files/"+c.Path, stock, c.Mode); e != nil {
				return "", e
			}
		}
		if c.Before != nil {
			if e := writeNew(root, cardRel+"/before/"+c.Path, c.Before, c.Mode); e != nil {
				return "", e
			}
		}
		if c.After != nil {
			if e := writeNew(root, cardRel+"/after/"+c.Path, c.After, c.Mode); e != nil {
				return "", e
			}
		}
	}
	if e := writeNew(root, cardRel+"/recovery.json", encode(recovery), 0600); e != nil {
		return "", e
	}
	return cardRel, nil
}
func install(root, dir, recovery string, homeOn, returnOn bool, hook func(string, string) error) error {
	changes, pkg, e := prepareInstall(root, dir, homeOn, returnOn)
	if e != nil {
		return e
	}
	// Backup verification must complete for EVERY affected file before mutation.
	mirror, e := saveRecovery(root, recovery, changes, pkg)
	if e != nil {
		return e
	}
	fmt.Println("Verified recovery:", recovery, "; card mirror:", mirror)
	e = transact(root, changes, hook)
	if e != nil {
		return e
	}
	fmt.Println("Install verified. Saved preferences unchanged. Optional integrations take effect after reboot.")
	return nil
}
func restore(root, recovery, dir string, interrupted bool, hook func(string, string) error) error {
	_, home, ret, err := loadPackage(dir)
	if err != nil {
		return err
	}
	if e := cardVersion(root); e != nil {
		return e
	}
	data, e := read(recovery, "recovery.json")
	if e != nil {
		return e
	}
	var r Recovery
	if e = json.Unmarshal(data, &r); e != nil || r.Format != 1 {
		return fmt.Errorf("invalid recovery manifest")
	}
	var changes []change
	seen := map[string]bool{}
	for _, s := range r.Files {
		if !s.Integration {
			continue
		}
		if seen[s.Path] || !allowedRestorePath(s.Path) || s.Mode > 0777 {
			return fmt.Errorf("unsafe restore entry")
		}
		seen[s.Path] = true
		if s.Path == system+"runtime.sh" && (s.Stock != ret.Original || s.After != ret.Patched) {
			return fmt.Errorf("unknown recovery runtime")
		}
		if s.Path == system+"script/better_favorites_return.sh" && (s.Stock != "absent" || s.After != ret.Helper) {
			return fmt.Errorf("unknown recovery helper")
		}
		if s.Path == app+"home-integration.conf" && (s.Stock != "absent" || s.After != digest(receipt(home))) {
			return fmt.Errorf("unknown recovery receipt")
		}
		for _, name := range names {
			if s.Path == system+"bin/"+name && (s.Stock != home.Original[name] || s.After != home.Patched[name]) {
				return fmt.Errorf("unknown recovery MainUI")
			}
			if strings.Contains(s.Path, "better-favorites-home-backup-") && strings.HasSuffix(s.Path, "/"+name) && s.After != home.Original[name] {
				return fmt.Errorf("unknown original backup")
			}
		}
		if s.Path == returnBackup+"runtime.sh" && s.After != ret.Original {
			return fmt.Errorf("unknown runtime original backup")
		}
		current, e := optional(root, s.Path)
		if e != nil {
			return e
		}
		h := hashOrAbsent(current)
		ownUninstalled := false
		if (s.Path == homeManifest || s.Path == returnBackup+"manifest.json") && current != nil {
			expected, readErr := read(recovery, "after/"+s.Path)
			if readErr == nil && digest(expected) == s.After {
				var actual, installed map[string]any
				if json.Unmarshal(current, &actual) == nil && json.Unmarshal(expected, &installed) == nil && actual["status"] == "uninstalled" {
					actual["status"] = "installed"
					ownUninstalled = bytes.Equal(encode(actual), encode(installed))
				}
			}
		}
		if !ownUninstalled && h != s.After && h != s.Stock && !(interrupted && h == s.Before) {
			return fmt.Errorf("changed installed file preserved: %s", s.Path)
		}
		var target []byte
		if s.Stock != "absent" {
			target, e = read(recovery, "files/"+s.Path)
			if e != nil || digest(target) != s.Stock {
				return fmt.Errorf("recovery backup checksum mismatch: %s", s.Path)
			}
		}
		// Integration backup originals are retained; metadata is made inert rather
		// than erasing old journals, so later reinstallation can validate them.
		if strings.Contains(s.Path, "-backup") {
			if strings.HasSuffix(s.Path, "manifest.json") && current != nil {
				var m map[string]any
				if json.Unmarshal(current, &m) != nil {
					return fmt.Errorf("bad return journal")
				}
				m["status"] = "uninstalled"
				target = encode(m)
			} else {
				continue
			}
		}
		if s.Path == homeManifest && current != nil {
			var m map[string]any
			if json.Unmarshal(current, &m) != nil {
				return fmt.Errorf("bad home journal")
			}
			m["status"] = "uninstalled"
			target = encode(m)
		}
		changes = append(changes, change{s.Path, current, target, os.FileMode(s.Mode), nil, true})
	}
	sort.SliceStable(changes, func(i, j int) bool { return restoreRank(changes[i].Path) < restoreRank(changes[j].Path) })
	if e = transact(root, changes, hook); e != nil {
		return e
	}
	fmt.Println("Verified stock integrations restored. App, preferences, all backups and data retained. Reboot before removing app.")
	return nil
}
func restoreRank(p string) int {
	if p == system+"runtime.sh" {
		return 0
	}
	if strings.HasPrefix(p, system+"bin/") {
		return 1
	}
	return 2
}
func allowedRestorePath(p string) bool {
	if !safeRel(p) {
		return false
	}
	if p == system+"runtime.sh" || p == system+"script/better_favorites_return.sh" || p == homeManifest || p == app+"home-integration.conf" || p == returnBackup+"manifest.json" || p == returnBackup+"runtime.sh" {
		return true
	}
	for _, n := range names {
		if p == system+"bin/"+n {
			return true
		}
	}
	for _, n := range names {
		if strings.HasPrefix(p, system+"config/better-favorites-home-backup-") && strings.HasSuffix(p, "/"+n) {
			return true
		}
	}
	return false
}

func allowedAppFile(name string) bool {
	switch name {
	case "better-favorites", "launch.sh", "config.json", "release.json", "libSDL2-2.0.so.0", "libSDL2_image-2.0.so.0", "libSDL2_mixer-2.0.so.0", "libSDL2_ttf-2.0.so.0", "libjson-c.so.5", "libpng16.so.16", "libz.so.1", "libEGL.so", "libGLESv2.so":
		return true
	}
	return false
}
