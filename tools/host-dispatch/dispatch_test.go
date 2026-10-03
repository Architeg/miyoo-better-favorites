package main

import (
	"os"
	"path/filepath"
	"testing"
)

func TestWindowsMatrix(t *testing.T) {
	// Probe inputs are native OS values: shell bitness has no role in dispatch.
	for _, version := range []host{{6, 1, 7601, 0}, {6, 2, 9200, 0}, {6, 3, 9600, 0}, {10, 0, 10240, 0}, {10, 0, 19045, 0}, {10, 0, 26100, 0}} {
		for _, arch := range []uint16{0x14c, 0x8664, 0xaa64, 0xffff} {
			h := version
			h.native = arch
			n, e := selectWindows(h)
			supported := arch == 0x14c || arch == 0x8664 || (arch == 0xaa64 && h.major == 10)
			if (e == nil) != supported {
				t.Fatalf("%+v %s %v", h, n, e)
			}
			if supported {
				f := "windows"
				if h.major == 6 {
					f = "windows7"
				}
				a := map[uint16]string{0x14c: "386", 0x8664: "amd64", 0xaa64: "arm64"}[arch]
				if n != "better-favorites-installer-"+f+"-"+a+".exe" {
					t.Fatal(n)
				}
			}
		}
	}
	for _, h := range []host{{0, 0, 0, 0x8664}, {6, 0, 6002, 0x8664}, {6, 4, 9000, 0x8664}, {10, 0, 0, 0x8664}, {10, 1, 26100, 0x8664}, {11, 0, 1, 0x8664}} {
		if _, e := selectWindows(h); e == nil {
			t.Fatal(h)
		}
	}
}
func TestChildStatusAndArguments(t *testing.T) {
	if os.Getenv("BF_DISPATCH_CHILD") == "1" {
		if len(os.Args) < 3 || os.Args[len(os.Args)-1] != "SD path with spaces" {
			os.Exit(19)
		}
		os.Exit(7)
	}
	own, _ := os.Executable()
	t.Setenv("BF_DISPATCH_CHILD", "1")
	if n := runTool(own, []string{"-test.run=TestChildStatusAndArguments", "--", "SD path with spaces"}); n != 7 {
		t.Fatal(n)
	}
	if n := runTool(filepath.Join(t.TempDir(), "missing"), nil); n == 0 {
		t.Fatal(n)
	}
	if n := runTool(t.TempDir(), nil); n == 0 {
		t.Fatal(n)
	}
}
