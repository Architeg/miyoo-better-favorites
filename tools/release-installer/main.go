package main

import (
	"archive/zip"
	"bufio"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

func bounded(root, rel string, limit int64) ([]byte, error) {
	p, e := join(root, rel)
	if e != nil {
		return nil, e
	}
	f, e := openRegular(p)
	if e != nil {
		return nil, e
	}
	defer f.Close()
	s, e := f.Stat()
	if e != nil || !s.Mode().IsRegular() {
		return nil, fmt.Errorf("unsafe diagnostic input")
	}
	if s.Size() > limit {
		if _, e = f.Seek(s.Size()-limit, io.SeekStart); e != nil {
			return nil, e
		}
	}
	return io.ReadAll(io.LimitReader(f, limit))
}
func export(root, dir, out string) error {
	pkg, h, r, e := loadPackage(dir)
	if e != nil {
		return e
	}
	report := map[string]any{"app_version": pkg.Version, "package_source_commit": pkg.Commit, "device_model": "unknown", "firmware": "unknown", "hardware_revision": "unknown", "collection": "offline card; no live RAM evidence", "clock": "device clock may be inaccurate"}
	values := map[string]any{}
	for _, p := range []string{system + "onionVersion/version.txt", system + "config/active_theme", app + "release.json", app + "settings.conf", app + "browser-preferences.conf", app + "home-entry.conf", app + "home-integration.conf"} {
		d, e := bounded(root, p, 4096)
		if e != nil {
			values[p] = "missing/unreadable"
		} else {
			values[p] = string(d)
		}
	}
	report["configuration"] = values
	hashes := map[string]any{}
	for _, n := range names {
		p := system + "bin/" + n
		d, e := read(root, p)
		if e != nil {
			hashes[p] = "unreadable"
		} else {
			hashes[p] = map[string]any{"sha256": digest(d), "recognized": digest(d) == h.Original[n] || digest(d) == h.Patched[n]}
		}
	}
	for _, p := range []string{app + "better-favorites", app + "launch.sh", system + "runtime.sh", system + "script/better_favorites_return.sh"} {
		d, e := read(root, p)
		if e != nil {
			hashes[p] = "missing/unreadable"
		} else {
			hashes[p] = digest(d)
		}
	}
	report["hashes"] = hashes
	report["runtime_original_sha256"] = r.Original
	report["runtime_patched_sha256"] = r.Patched
	f, e := os.OpenFile(out, os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0600)
	if e != nil {
		return e
	}
	z := zip.NewWriter(f)
	ok := false
	defer func() {
		z.Close()
		f.Close()
		if !ok {
			os.Remove(out)
		}
	}()
	write := func(name string, data []byte) error {
		w, e := z.Create(name)
		if e != nil {
			return e
		}
		_, e = w.Write(data)
		return e
	}
	if e = write("report.json", encode(report)); e != nil {
		return e
	}
	missing := map[string]string{}
	for _, p := range []string{app + "better-favorites.log", app + "better-favorites.previous.log", app + "home-diagnostics.log", system + "logs/better-favorites-return.log"} {
		limit := int64(65536)
		if strings.HasSuffix(p, "home-diagnostics.log") || strings.HasSuffix(p, "better-favorites-return.log") {
			limit = 131072
		}
		d, e := bounded(root, p, limit)
		if e != nil {
			missing[p] = "missing/unreadable"
			continue
		}
		if e = write(filepath.Base(p), d); e != nil {
			return e
		}
	}
	if e = write("missing.json", encode(missing)); e != nil {
		return e
	}
	if e = write("PRIVACY.txt", []byte("Review before sharing. Only allowlisted logs/configuration and hashes are collected. No ROM/history contents, credentials, serials or live process samples. Error logs may contain game filenames. Unknown model/firmware are not inferred.\n")); e != nil {
		return e
	}
	if e = z.Close(); e != nil {
		return e
	}
	if e = f.Sync(); e != nil {
		return e
	}
	if e = f.Close(); e != nil {
		return e
	}
	ok = true
	fmt.Println("Diagnostics exported:", out)
	return nil
}
func trace(root string, on bool) error {
	if e := cardVersion(root); e != nil {
		return e
	}
	p := app + "home-diagnostics.conf"
	old, e := optional(root, p)
	if e != nil {
		return e
	}
	expected := []byte("BetterFavoritesHomeDiagnostics1\n1\n")
	if old != nil && string(old) != string(expected) {
		return fmt.Errorf("foreign diagnostic marker preserved")
	}
	var after []byte
	if on {
		after = expected
	}
	return transact(root, []change{{p, old, after, 0600, nil, false}}, nil)
}
func run(args []string) error {
	if len(args) >= 2 && args[0] == "--card-launcher" {
		return launchFromCard(args[1], args[2:])
	}
	if len(args) > 0 && args[0] == "--staged-card" {
		return runStagedCard(args[1:])
	}
	in := bufio.NewReader(os.Stdin)
	ask := func(prompt string) (string, error) {
		fmt.Print(prompt)
		answer, e := in.ReadString('\n')
		if e != nil && len(answer) == 0 {
			return "", fmt.Errorf("input required: %s", prompt)
		}
		return strings.Trim(strings.TrimSpace(answer), "\""), nil
	}
	if len(args) == 0 {
		fmt.Println("Better Favorites — offline card tool\n1 Install / Update\n2 Uninstall completely\n0 Cancel")
		a, e := ask("Action: ")
		if e != nil {
			return e
		}
		if a == "0" || a == "" {
			return nil
		}
		action := map[string]string{"1": "install", "2": "uninstall"}[a]
		if action == "" {
			return fmt.Errorf("invalid action")
		}

		args = []string{action}
	}
	interactive := len(args) == 1
	action := args[0]
	switch action {
	case "install", "uninstall", "restore", "remove-integrations", "export-diagnostics", "status", "trace-on", "trace-off":
	default:
		return fmt.Errorf("unknown action: %s", action)
	}
	f := flag.NewFlagSet(action, flag.ContinueOnError)
	root := f.String("sd-root", "", "mounted card root")
	dir := f.String("package", "", "extracted release directory")
	recovery := f.String("recovery", "", "new host recovery directory for Install; retained recovery directory for Uninstall/Restore")
	output := f.String("output", "", "new diagnostics ZIP")
	archive := f.String("archive", "", "new computer archive directory for complete uninstall")
	off := f.Bool("powered-off", false, "confirm Miyoo is powered off and no other writer is using the card")
	home := f.Bool("home", true, "install optional Home Favorites redirect")
	ret := f.Bool("return", true, "install optional session return")
	if e := f.Parse(args[1:]); e != nil {
		return e
	}
	if f.NArg() != 0 {
		return fmt.Errorf("unexpected arguments")
	}
	if *root == "" {
		var e error
		*root, e = ask("SD-card root: ")
		if e != nil {
			return e
		}
		interactive = true
	}
	if *root == "" {
		return fmt.Errorf("empty SD-card root")
	}
	if interactive && action != "export-diagnostics" && action != "status" && !*off {
		answer, e := ask("Powered off, exclusive card access? Type OFF: ")
		if e != nil {
			return e
		}
		if answer != "OFF" {
			return fmt.Errorf("cancelled")
		}
		*off = true
	}

	var e error
	*root, e = filepath.Abs(*root)
	if e != nil {
		return e
	}
	if runtime.GOOS == "windows" && (strings.HasPrefix(*root, `\\`) || len(filepath.VolumeName(*root)) != 2) {
		return fmt.Errorf("use a local SD-card drive; UNC/device paths unsupported")
	}
	if *dir == "" {
		exe, e := os.Executable()
		if e != nil {
			return e
		}
		*dir = filepath.Dir(exe)
	}
	*dir, e = filepath.Abs(*dir)
	if e != nil {
		return e
	}
	stamp := time.Now().UTC().Format("20060102T150405.000000000Z")
	if action == "export-diagnostics" {
		if *output == "" {
			*output = filepath.Join(*dir, "diagnostics-"+stamp+".zip")
		}
		return export(*root, *dir, *output)
	}
	if action == "status" {
		pkg, h, r, e := loadPackage(*dir)
		if e != nil {
			return e
		}
		v, e := read(*root, system+"onionVersion/version.txt")
		if e != nil {
			return e
		}
		data := map[string]any{"package": pkg.Version, "onion": strings.TrimSpace(string(v)), "home": h.Version, "return": r.Version}
		for _, p := range []string{system + "runtime.sh", app + "home-entry.conf", app + "settings.conf"} {
			d, e := optional(*root, p)
			if e != nil {
				return e
			}
			data[p] = hashOrAbsent(d)
		}
		fmt.Print(string(encode(data)))
		return nil
	}
	if !*off {
		return fmt.Errorf("mutations require --powered-off; close all other card writers")
	}
	switch action {
	case "install":
		if *recovery == "" {
			*recovery = filepath.Join(*dir, "recovery-"+stamp)
		}
		return install(*root, *dir, *recovery, *home, *ret, nil)
	case "uninstall", "restore", "remove-integrations":
		if *recovery == "" {
			selected, err := discoverRecovery(*root, *dir)
			if err != nil {
				if !interactive {
					return err
				}
				fmt.Println(err)
				selected, err = ask("This card's verified recovery folder: ")
				if err != nil {
					return err
				}
			}
			*recovery = selected
		}
		if action == "uninstall" {
			if *archive == "" {
				*archive = freshUninstallArchive(*dir)
			}
			return completeUninstall(*root, *recovery, *dir, *archive, nil)
		}
		return restore(*root, *recovery, *dir, action == "restore", nil)
	case "trace-on", "trace-off":
		return trace(*root, action == "trace-on")
	default:
		return fmt.Errorf("unknown action: %s", action)
	}
}
func main() {
	if e := run(os.Args[1:]); e != nil {
		fmt.Fprintln(os.Stderr, e)
		var child *exec.ExitError
		if errors.As(e, &child) && (child.ExitCode() > 0 || runtime.GOOS == "windows") {
			os.Exit(child.ExitCode())
		}
		os.Exit(1)
	}
}
