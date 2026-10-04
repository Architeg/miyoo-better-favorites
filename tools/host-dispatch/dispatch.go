// Read-only Windows bootstrap. The Go1.20 x86 binary can start on Windows7;
// it never trusts the calling shell's architecture or compatibility version.
package main

import (
	"bytes"
	"crypto/sha256"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
)

type host struct {
	major, minor, build uint32
	native              uint16
}

func selectWindows(h host) (string, error) {
	family := ""
	if h.major == 6 && h.minor >= 1 && h.minor <= 3 && h.build > 0 {
		family = "windows7"
	}
	if h.major == 10 && h.minor == 0 && h.build >= 10240 {
		family = "windows"
	}
	if family == "" {
		return "", fmt.Errorf("unsupported/ambiguous Windows version %d.%d.%d; Windows7 or later required", h.major, h.minor, h.build)
	}
	arch := ""
	switch h.native {
	case 0x14c:
		arch = "386"
	case 0x8664:
		arch = "amd64"
	case 0xaa64:
		if family == "windows" {
			arch = "arm64"
		}
	}
	if arch == "" {
		return "", fmt.Errorf("unsupported/ambiguous native Windows architecture %#x", h.native)
	}
	return "better-favorites-installer-" + family + "-" + arch + ".exe", nil
}
func runTool(path string, args []string) int {
	s, e := os.Lstat(path)
	if e != nil || !s.Mode().IsRegular() {
		fmt.Fprintln(os.Stderr, "Missing/unsafe packaged installer:", path)
		return 2
	}
	c := exec.Command(path, args...)
	c.Stdin = os.Stdin
	c.Stdout = os.Stdout
	c.Stderr = os.Stderr
	e = c.Run()
	if e == nil {
		return 0
	}
	// Windows exit codes may appear signed in this 32-bit bootstrap. Preserve
	// their bit pattern, including native exception statuses.
	if x, ok := e.(*exec.ExitError); ok {
		return x.ExitCode()
	}
	fmt.Fprintln(os.Stderr, "Could not run packaged installer:", e)
	return 2
}
func main() {
	h, e := detectWindows()
	if e != nil {
		fmt.Fprintln(os.Stderr, "Cannot identify Windows host:", e)
		os.Exit(2)
	}
	name, e := selectWindows(h)
	if e != nil {
		fmt.Fprintln(os.Stderr, e)
		os.Exit(2)
	}
	own, e := os.Executable()
	if e != nil {
		fmt.Fprintln(os.Stderr, e)
		os.Exit(2)
	}
	args := os.Args[1:]
	if len(args) >= 2 && args[0] == "--card-launcher" {
		// Execute the selected installer off-card. The backend then validates and
		// stages the full package before writes; this dispatcher never patches cards.
		path := filepath.Join(args[1], "computer", name)
		code := stageTool(path, args)
		os.Exit(code)
	}
	os.Exit(runTool(filepath.Join(filepath.Dir(own), name), args))
}

func stageTool(path string, args []string) int {
	s, e := os.Lstat(path)
	if e != nil || !s.Mode().IsRegular() {
		fmt.Fprintln(os.Stderr, "Missing/unsafe installer:", path)
		return 2
	}
	input, e := os.Open(path)
	if e != nil {
		return 2
	}
	defer input.Close()
	before, e := input.Stat()
	if e != nil || !os.SameFile(s, before) || !before.Mode().IsRegular() || before.Size() > 64*1024*1024 {
		return 2
	}
	dir, e := os.MkdirTemp("", "better-favorites-native-")
	if e != nil {
		return 2
	}
	defer os.RemoveAll(dir)
	dest := filepath.Join(dir, filepath.Base(path))
	f, e := os.OpenFile(dest, os.O_CREATE|os.O_EXCL|os.O_WRONLY, 0700)
	if e != nil {
		return 2
	}
	hash := sha256.New()
	_, e = io.Copy(io.MultiWriter(f, hash), io.LimitReader(input, 64*1024*1024+1))
	if e == nil {
		e = f.Sync()
	}
	f.Close()
	if e != nil {
		return 2
	}
	b, e := os.ReadFile(dest)
	if e != nil || !bytes.Equal(func() []byte { v := sha256.Sum256(b); return v[:] }(), hash.Sum(nil)) {
		return 2
	}
	return runTool(dest, args)
}
