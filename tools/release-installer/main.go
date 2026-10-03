package main

import (
	"archive/zip"
	"bufio"
	"flag"
	"fmt"
	"io"
	"os"
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
	for _, p := range []string{app + "better-favorites.log", app + "better-favorites.previous.log", app + "home-diagnostics.log"} {
		limit := int64(65536)
		if strings.HasSuffix(p, "home-diagnostics.log") {
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
	if len(args) == 0 {
		in := bufio.NewReader(os.Stdin)
		fmt.Println("Better Favorites — offline card tool\n1 Install app + optional Home/return patches\n2 Uninstall patches (retain app/data)\n3 Restore interrupted installation\n4 Export diagnostics\nPower the Miyoo OFF and close other card writers.")
		fmt.Print("Action (1–4): ")
		a, _ := in.ReadString('\n')
		actions := map[string]string{"1": "install", "2": "uninstall", "3": "restore", "4": "export-diagnostics"}
		action := actions[strings.TrimSpace(a)]
		if action == "" {
			return fmt.Errorf("invalid action")
		}
		fmt.Print("SD-card root: ")
		sd, _ := in.ReadString('\n')
		sd = strings.Trim(strings.TrimSpace(sd), "\"")
		args = []string{action, "--sd-root", sd}
		if action != "export-diagnostics" {
			fmt.Print("Powered off, exclusive card access? Type OFF: ")
			answer, _ := in.ReadString('\n')
			if strings.TrimSpace(answer) != "OFF" {
				return fmt.Errorf("cancelled")
			}
			args = append(args, "--powered-off")
		}
		if action == "install" {
			for _, n := range []string{"home", "return"} {
				fmt.Printf("Install optional %s integration? (y/N): ", n)
				a, _ = in.ReadString('\n')
				if strings.EqualFold(strings.TrimSpace(a), "y") {
					args = append(args, "--"+n)
				}
			}
		}
		if action == "uninstall" || action == "restore" {
			fmt.Print("Verified recovery folder (host or card mirror): ")
			p, _ := in.ReadString('\n')
			args = append(args, "--recovery", strings.Trim(strings.TrimSpace(p), "\""))
		}
	}
	action := args[0]
	f := flag.NewFlagSet(action, flag.ContinueOnError)
	root := f.String("sd-root", "", "mounted card root")
	dir := f.String("package", "", "extracted release directory")
	recovery := f.String("recovery", "", "new host recovery directory for Install; retained recovery directory for Uninstall/Restore")
	output := f.String("output", "", "new diagnostics ZIP")
	off := f.Bool("powered-off", false, "confirm Miyoo is powered off and no other writer is using the card")
	home := f.Bool("home", false, "install optional Home Favorites redirect")
	ret := f.Bool("return", false, "install optional session return")
	if e := f.Parse(args[1:]); e != nil {
		return e
	}
	if f.NArg() != 0 || *root == "" {
		return fmt.Errorf("specify --sd-root; use --help for options")
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
	case "uninstall", "restore":
		if *recovery == "" {
			return fmt.Errorf("specify --recovery; preserve originals before deleting the app")
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
		os.Exit(1)
	}
}
