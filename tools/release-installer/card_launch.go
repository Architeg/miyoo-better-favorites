package main

import (
	"bufio"
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"runtime"
	"strings"
	"time"
)

const installationIndex = system + "config/better-favorites-installation.json"

type Installation struct {
	Format   int    `json:"format"`
	Recovery string `json:"recovery"`
	SHA      string `json:"sha256"`
	Pending  bool   `json:"pending,omitempty"`
}
type Transport struct {
	Format int           `json:"format"`
	Files  []PayloadFile `json:"files"`
}

func validateCard(root string) error {
	v, e := read(root, system+"onionVersion/version.txt")
	if e != nil || !regexp.MustCompile(`^v[0-9]+\.[0-9]+\.[0-9]+(-[A-Za-z0-9.-]+)?$`).MatchString(strings.TrimSpace(string(v))) {
		return fmt.Errorf("not an identifiable Onion card: version file unavailable")
	}
	b, e := read(root, system+"runtime.sh")
	if e != nil || !strings.HasPrefix(string(b), "#!/") {
		return fmt.Errorf("not an identifiable Onion card: runtime unavailable")
	}
	for _, p := range []string{"App", "Roms", system + "bin"} {
		n, e := join(root, p)
		if e != nil {
			return e
		}
		s, e := os.Stat(n)
		if e != nil || !s.IsDir() {
			return fmt.Errorf("missing Onion card directory: %s", p)
		}
	}
	found := false
	for _, n := range names {
		if b, e := optional(root, system+"bin/"+n); e != nil {
			return e
		} else if len(b) >= 52 && bytes.Equal(b[:7], []byte{0x7f, 'E', 'L', 'F', 1, 1, 1}) && b[18] == 40 && b[19] == 0 {
			found = true
		}
	}
	if !found {
		return fmt.Errorf("not an identifiable Miyoo Onion card: ARM MainUI unavailable")
	}
	return nil
}
func deriveCard(source string) (string, string, error) {
	p, e := filepath.Abs(source)
	if e != nil {
		return "", "", e
	}
	if filepath.Base(p) != "BetterFavorites" || filepath.Base(filepath.Dir(p)) != "App" {
		return "", "", fmt.Errorf("open this installer inside the card's App/BetterFavorites folder")
	}
	root := filepath.Dir(filepath.Dir(p))
	if e = safeCardAncestors(root); e != nil {
		return "", "", e
	}
	safe, e := join(root, strings.TrimSuffix(app, "/"))
	if e != nil {
		return "", "", e
	}
	if safe != p {
		return "", "", fmt.Errorf("ambiguous launcher path")
	}
	if e = validateCard(root); e != nil {
		return "", "", e
	}
	return root, p, nil
}
func activeRecovery(root string) (string, error) {
	b, e := read(root, installationIndex)
	if e != nil {
		return "", e
	}
	var m Installation
	if json.Unmarshal(b, &m) != nil || m.Format != 1 || !safeRel(m.Recovery) || !strings.HasPrefix(m.Recovery, system+"config/better-favorites-recovery-") || strings.Count(m.Recovery, "/") != 2 {
		return "", fmt.Errorf("invalid portable recovery index preserved")
	}
	p, e := join(root, m.Recovery)
	if e != nil {
		return "", e
	}
	b, e = read(p, "recovery.json")
	if e != nil || digest(b) != m.SHA {
		return "", fmt.Errorf("portable recovery identity/checksum mismatch: %s", m.Recovery)
	}
	if _, e = loadRecovery(p); e != nil {
		return "", e
	}
	return p, nil
}
func allowedTransportPath(p string) bool {
	if !safeRel(p) {
		return false
	}
	switch p {
	case "Install-Windows.cmd", "Install-macOS.command", "Install-Linux.sh", "Install-Linux.desktop", "Mac-first-open.html":
		return true
	}
	return strings.HasPrefix(p, "computer/")
}
func transportData(dir, rel string) ([]byte, error) {
	if strings.HasPrefix(rel, "computer/") {
		return read(dir, strings.TrimPrefix(rel, "computer/"))
	}
	if _, e := os.Stat(filepath.Join(dir, "card-files")); e == nil {
		return read(dir, "card-files/"+rel)
	}
	return read(filepath.Dir(dir), rel)
}
func transportFiles(dir string) ([]PayloadFile, error) {
	b, e := optional(dir, "transport.json")
	if e != nil || b == nil {
		return nil, e
	}
	var m Transport
	if json.Unmarshal(b, &m) != nil || m.Format != 1 || len(m.Files) < 10 || len(m.Files) > 1000 {
		return nil, fmt.Errorf("invalid computer transport manifest")
	}
	seen := map[string]bool{}
	for _, f := range m.Files {
		if !allowedTransportPath(f.Path) || seen[f.Path] || f.Mode > 0777 {
			return nil, fmt.Errorf("unsafe transport entry: %s", f.Path)
		}
		seen[f.Path] = true
		d, e := transportData(dir, f.Path)
		if e != nil {
			return nil, fmt.Errorf("computer package read/verification failed: %s: %w", f.Path, e)
		}
		if digest(d) != f.SHA {
			return nil, fmt.Errorf("computer package checksum mismatch: %s (expected %s, observed %s)", f.Path, f.SHA, digest(d))
		}
	}
	for _, p := range []string{"Install-Windows.cmd", "Install-macOS.command", "Install-Linux.sh", "Install-Linux.desktop", "computer/package.json", "computer/HOST-BUILDS.json"} {
		if !seen[p] {
			return nil, fmt.Errorf("incomplete transport: %s", p)
		}
	}
	// The manifest is itself a recorded input; no recursive self checksum.
	m.Files = append(m.Files, PayloadFile{"computer/transport.json", digest(b), 0644})
	return m.Files, nil
}
func addTransport(root, dir string, out *[]change) error {
	fs, e := transportFiles(dir)
	if e != nil {
		return e
	}
	wanted := map[string]bool{}
	for _, f := range fs {
		wanted[app+f.Path] = true
	}
	if p, e := activeRecovery(root); e == nil {
		old, e := loadRecovery(p)
		if e != nil {
			return e
		}
		for _, v := range old.Files {
			if !v.Integration && strings.HasPrefix(v.Path, app) && allowedTransportPath(strings.TrimPrefix(v.Path, app)) && !wanted[v.Path] {
				b, e := optional(root, v.Path)
				if e != nil {
					return e
				}
				if b != nil {
					if digest(b) != v.After {
						return fmt.Errorf("obsolete modified tool preserved: %s", v.Path)
					}
					if e = add(root, out, v.Path, nil, nil, 0600, false); e != nil {
						return e
					}
				}
			}
		}
	}
	for _, f := range fs {
		b, e := transportData(dir, f.Path)
		if e != nil {
			return e
		}
		if e = add(root, out, app+f.Path, b, nil, os.FileMode(f.Mode), false); e != nil {
			return e
		}
	}
	return nil
}
func validateAppInput(root, dir string, pkg Package) error {
	accepted := map[string]map[string]bool{}
	accept := func(p, h string) {
		if accepted[p] == nil {
			accepted[p] = map[string]bool{}
		}
		accepted[p][h] = true
	}
	for _, f := range pkg.Files {
		if strings.HasPrefix(f.Path, app) {
			accept(f.Path, f.SHA)
		}
	}
	fs, e := transportFiles(dir)
	if e != nil {
		return e
	}
	for _, f := range fs {
		accept(app+f.Path, f.SHA)
	}
	if p, e := activeRecovery(root); e == nil {
		r, e := loadRecovery(p)
		if e != nil {
			return e
		}
		for _, s := range r.Files {
			accept(s.Path, s.After)
		}
	} else if !errors.Is(e, os.ErrNotExist) {
		return e
	}
	// Old packages have no index. Only unambiguously matching, verified journals
	// can authenticate old payload bytes; no pathname or prefix proves ownership.
	if _, e := activeRecovery(root); errors.Is(e, os.ErrNotExist) {
		if p, e := discoverRecovery(root, dir); e == nil {
			r, e := loadRecovery(p)
			if e != nil {
				return e
			}
			for _, s := range r.Files {
				accept(s.Path, s.After)
			}
		}
	}
	own, e := migrationOwnership(root)
	if e != nil {
		return e
	}
	for p, hashes := range own {
		for h := range hashes {
			accept(p, h)
		}
	}

	expectedDirs := map[string]bool{strings.TrimSuffix(app, "/"): true}
	for p := range accepted {
		if strings.HasPrefix(p, app) {
			for d := filepath.ToSlash(filepath.Dir(p)); strings.HasPrefix(d, strings.TrimSuffix(app, "/")); d = filepath.ToSlash(filepath.Dir(d)) {
				expectedDirs[d] = true
			}
		}
	}
	location, _ := join(root, strings.TrimSuffix(app, "/"))
	if err := filepath.Walk(location, func(p string, s os.FileInfo, e error) error {
		if errors.Is(e, os.ErrNotExist) {
			return nil
		}
		if e != nil {
			return e
		}
		if s.IsDir() {
			rel, _ := filepath.Rel(root, p)
			if !expectedDirs[filepath.ToSlash(rel)] {
				return fmt.Errorf("unknown app directory preserved: %s", rel)
			}
		}
		return nil
	}); err != nil {
		return err
	}
	_, e = ownedTree(root, strings.TrimSuffix(app, "/"), func(p string, b []byte) error {
		n := strings.TrimPrefix(p, app)
		switch n {
		case "settings.conf", "browser-preferences.conf", "browser-state", "home-entry.conf":
			return nil
		} // preserve even malformed saved preferences on update
		if personalFile(n, b) || accepted[p][digest(b)] {
			return nil
		}
		return fmt.Errorf("unknown/modified app input preserved: %s", p)
	}, recordedMissing(own), expectedDirs)
	return e
}
func verifyRestored(root, recovery, dir string) error {
	r, e := loadRecovery(recovery)
	if e != nil {
		return e
	}
	// A completed restoration cannot retain either app's installed receipt,
	// including legacy journals whose receipt incorrectly lacked Integration.
	for _, marker := range []string{app + "home-integration.conf", legacyApp + "home-integration.conf"} {
		b, err := optional(root, marker)
		if err != nil || b != nil {
			return fmt.Errorf("stock restoration not verified; installed receipt preserved: %s", marker)
		}
	}
	for _, s := range r.Files {
		if !s.Integration || (s.Path != system+"runtime.sh" && !strings.HasPrefix(s.Path, system+"bin/") && s.Path != system+"script/better_favorites_return.sh" && s.Path != app+"home-integration.conf") {
			continue
		}
		b, e := optional(root, s.Path)
		if e != nil || hashOrAbsent(b) != s.Stock {
			return fmt.Errorf("stock restoration not verified: %s", s.Path)
		}
	}
	return nil
}

// Verify before staging. Execution is off-card so Windows file locks and Linux
// noexec mounts cannot prevent complete removal. No recovery lives in this stage.
func launchFromCard(source string, args []string) error {
	root, source, e := deriveCard(source)
	if e != nil {
		return e
	}
	reportCard(root)
	dir := filepath.Join(source, "computer")
	// Top launchers are in the app. Their manifest entries use card-files/ in
	// the private staging layout; read them via a dedicated source verifier.
	b, e := read(dir, "transport.json")
	if e != nil {
		return e
	}
	var m Transport
	if json.Unmarshal(b, &m) != nil || m.Format != 1 || len(m.Files) < 10 || len(m.Files) > 1000 {
		return fmt.Errorf("invalid transport")
	}
	temp, e := os.MkdirTemp("", "better-favorites-execution-")
	if e != nil {
		return e
	}
	defer os.RemoveAll(temp)
	for _, f := range m.Files {
		if !allowedTransportPath(f.Path) || f.Mode > 0777 {
			return fmt.Errorf("unsafe transport input")
		}
		data, e := read(source, f.Path)
		if e != nil || digest(data) != f.SHA {
			return fmt.Errorf("transport checksum mismatch: %s", f.Path)
		}
		dest := strings.TrimPrefix(f.Path, "computer/")
		if !strings.HasPrefix(f.Path, "computer/") {
			dest = "card-files/" + f.Path
		}
		if e = writeNew(temp, dest, data, os.FileMode(f.Mode)); e != nil {
			return e
		}
	}
	if e = writeNew(temp, "transport.json", b, 0644); e != nil {
		return e
	}
	if _, e = transportFiles(temp); e != nil {
		return e
	}
	if _, _, _, e = loadPackage(temp); e != nil {
		return e
	}
	own, e := os.Executable()
	if e != nil {
		return e
	}
	name := filepath.Base(own)
	// First-stage wrapper names the bootstrap copy 'installer'. Locate the exact
	// native executable by bytes, not shell architecture or user-provided name.
	current, e := regular(own)
	if e != nil {
		return e
	}
	found := ""
	for _, f := range m.Files {
		if strings.HasPrefix(f.Path, "computer/better-favorites-installer-") && digest(current) == f.SHA {
			if found != "" {
				return fmt.Errorf("ambiguous executable identity")
			}
			found = filepath.Join(temp, strings.TrimPrefix(f.Path, "computer/"))
		}
	}
	if found == "" {
		return fmt.Errorf("running %s is not the package-verified installer", name)
	}
	return runStagedCard(append([]string{root, temp}, args...))
}
func runStagedCard(args []string) error {
	if len(args) < 2 {
		return fmt.Errorf("invalid staged invocation")
	}
	root, dir := args[0], args[1]
	reportCard(root)
	if e := validateCard(root); e != nil {
		return e
	}
	if e := outsideCard(root, dir); e != nil {
		return e
	}
	if _, e := transportFiles(dir); e != nil {
		return e
	}
	in := bufio.NewReader(os.Stdin)
	ask := func(prompt string) (string, error) {
		fmt.Print(prompt)
		s, e := in.ReadString('\n')
		if e != nil && len(s) == 0 {
			return "", fmt.Errorf("cancelled: no confirmation")
		}
		return strings.TrimSpace(s), nil
	}
	action := ""
	more := []string{}
	if len(args) > 2 {
		action = args[2]
		more = args[3:]
	} else {
		installerMenu()
		s, e := ask("Choose: ")
		if e != nil {
			return e
		}
		switch s {
		case "", "0":
			return nil
		case "1":
			action = "install"
		case "2":
			action = "uninstall"
		case "3":
			action = "export-diagnostics"
		default:
			return fmt.Errorf("invalid choice")
		}
	}
	if operationLog != nil {
		operationLog.action = action
		operationLog.detail("Action: %s", action)
	}
	// Explicit support arguments remain available; their existing safety checks
	// and powered-off confirmation apply. The simple menu never asks for a path.
	if len(more) > 0 {
		if e := capturedTarget(root, dir, more); e != nil {
			return e
		}
		return run(append([]string{action, "--sd-root", root, "--package", dir}, more...))
	}
	if action == "export-diagnostics" {
		home, e := os.UserHomeDir()
		if e != nil {
			return e
		}
		return export(root, dir, filepath.Join(home, "better-favorites-diagnostics-"+time.Now().UTC().Format("20060102T150405.000000000Z")+".zip"))
	}
	if action != "install" && action != "uninstall" {
		return fmt.Errorf("advanced action requires explicit arguments")
	}
	version, _ := read(root, system+"onionVersion/version.txt")
	fmt.Printf("Card: %s (Onion %s)\n", root, strings.TrimSpace(string(version)))
	s, e := ask("Miyoo powered OFF, no other card writer; proceed? (y/N): ")
	if e != nil {
		return e
	}
	if !strings.EqualFold(s, "y") {
		return nil
	}
	if action == "install" {
		both := true
		if cardVersion(root) != nil {
			s, e = ask("This Onion version is unsupported for both integrations. Install the app only? (y/N): ")
			if e != nil {
				return e
			}
			if !strings.EqualFold(s, "y") {
				return nil
			}
			both = false
		}
		return install(root, dir, filepath.Join(dir, "transaction-recovery"), both, both, nil)
	}
	finish := beginOperation("uninstall")
	defer finish()
	progressPhase("Verifying recovery")
	recovery, e := discoverRecovery(root, dir)
	if e != nil {
		return fmt.Errorf("cannot uninstall: %w; keep card recovery and see docs/recovery.md", e)
	}
	home, e := os.UserHomeDir()
	if e != nil {
		return e
	}
	archive := filepath.Join(home, "BetterFavorites-Recovery", "uninstall-"+time.Now().UTC().Format("20060102T150405.000000000Z"))
	if e = os.MkdirAll(filepath.Dir(archive), 0700); e != nil {
		return e
	}
	return completeUninstall(root, recovery, dir, archive, nil)
}

// Copied-card support calls may supply redundant explicit locations, but cannot
// redirect the validated target or package. Standalone advanced CLI is unchanged.
func capturedTarget(root, dir string, args []string) error {
	for i := 0; i < len(args); i++ {
		key, value, has := strings.Cut(args[i], "=")
		name := strings.TrimLeft(key, "-")
		if name != "sd-root" && name != "package" {
			continue
		}
		if !has {
			i++
			if i == len(args) {
				return fmt.Errorf("missing %s value", key)
			}
			value = args[i]
		}
		actual, e := filepath.Abs(value)
		if e != nil {
			return e
		}
		expected := root
		if name == "package" {
			expected = dir
		}
		if actual != filepath.Clean(expected) {
			return fmt.Errorf("%s conflicts with captured card/package location", key)
		}
	}
	return nil
}

func safeCardAncestors(root string) error {
	for p := root; ; p = filepath.Dir(p) {
		bad, e := unsafeAttributes(p)
		if e != nil {
			return e
		}
		if bad {
			// macOS's OS-owned /tmp and /var aliases are not removable-card aliases.
			real, err := filepath.EvalSymlinks(p)
			systemAlias := runtime.GOOS == "darwin" && err == nil && ((p == "/tmp" && real == "/private/tmp") || (p == "/var" && real == "/private/var"))
			if !systemAlias {
				return fmt.Errorf("card path ancestor link/reparse point refused: %s", p)
			}
		}
		if filepath.Dir(p) == p {
			break
		}
	}
	return nil
}
func recoveryIdentity(root string, m Installation) (string, error) {
	if m.Format != 1 || !safeRel(m.Recovery) || !strings.HasPrefix(m.Recovery, system+"config/better-favorites-recovery-") || strings.Count(m.Recovery, "/") != 2 {
		return "", fmt.Errorf("invalid recovery identity preserved")
	}
	p, e := join(root, m.Recovery)
	if e != nil {
		return "", e
	}
	b, e := read(p, "recovery.json")
	if e != nil || digest(b) != m.SHA {
		return "", fmt.Errorf("recovery lineage checksum mismatch: %s", m.Recovery)
	}
	if _, e = loadRecovery(p); e != nil {
		return "", e
	}
	return p, nil
}
func previousRecovery(root, dir string, index []byte) (*Installation, error) {
	if index != nil {
		var m Installation
		if json.Unmarshal(index, &m) != nil {
			return nil, fmt.Errorf("invalid installation index preserved")
		}
		if _, e := recoveryIdentity(root, m); e != nil {
			return nil, e
		}
		return &m, nil
	}
	// A failed/legacy unindexed install can be retried only when one verified
	// journal is unambiguous. Never adopt an arbitrary prefix-named directory.
	entries, e := os.ReadDir(filepath.Join(root, system+"config"))
	if errors.Is(e, os.ErrNotExist) {
		return nil, nil
	}
	if e != nil {
		return nil, e
	}
	exists := false
	for _, v := range entries {
		if strings.HasPrefix(v.Name(), "better-favorites-recovery-") {
			exists = true
		}
	}
	if !exists {
		return nil, nil
	}
	p, e := discoverRecovery(root, dir)
	if e != nil {
		return nil, e
	}
	b, e := read(p, "recovery.json")
	if e != nil {
		return nil, e
	}
	rel, e := filepath.Rel(root, p)
	if e != nil {
		return nil, e
	}
	m := Installation{Format: 1, Recovery: filepath.ToSlash(rel), SHA: digest(b)}
	if _, e = recoveryIdentity(root, m); e != nil {
		return nil, e
	}
	return &m, nil
}
func ownedRecoveryMirrors(root, recovery string) (map[string]bool, error) {
	b, e := read(recovery, "recovery.json")
	if e != nil {
		return nil, e
	}
	selected := digest(b)
	var start Installation
	if index, e := optional(root, installationIndex); e != nil {
		return nil, e
	} else if index != nil {
		if json.Unmarshal(index, &start) != nil || start.SHA != selected {
			return nil, fmt.Errorf("selected recovery differs from this installation identity")
		}
	} else {
		entries, e := os.ReadDir(filepath.Join(root, system+"config"))
		if e != nil {
			return nil, e
		}
		found := 0
		for _, v := range entries {
			if strings.HasPrefix(v.Name(), "better-favorites-recovery-") {
				rel := system + "config/" + v.Name()
				if data, e := read(filepath.Join(root, rel), "recovery.json"); e == nil && digest(data) == selected {
					start = Installation{Format: 1, Recovery: rel, SHA: selected}
					found++
				}
			}
		}
		if found != 1 {
			return nil, fmt.Errorf("portable recovery identity is missing or ambiguous")
		}
	}
	out := map[string]bool{}
	for i := 0; i < 1000; i++ {
		if out[start.Recovery] {
			return nil, fmt.Errorf("cyclic recovery lineage preserved")
		}
		p, e := recoveryIdentity(root, start)
		if e != nil {
			return nil, e
		}
		out[start.Recovery] = true
		r, e := loadRecovery(p)
		if e != nil {
			return nil, e
		}
		if r.Previous == nil {
			return out, nil
		}
		start = *r.Previous
	}
	return nil, fmt.Errorf("recovery lineage exceeds safe limit")
}
