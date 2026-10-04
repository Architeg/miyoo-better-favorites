package main

import (
	"encoding/binary"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"testing"
)

func macMetadata(t *testing.T) []byte {
	t.Helper()
	b, e := os.ReadFile("testdata/macos-appledouble.bin")
	if e != nil {
		t.Fatal(e)
	}
	return b
}
func finderMetadata(t *testing.T) []byte {
	t.Helper()
	b, e := os.ReadFile("testdata/finder-empty.bin")
	if e != nil {
		t.Fatal(e)
	}
	return b
}
func TestMetadataFormats(t *testing.T) {
	ad, ds := macMetadata(t), finderMetadata(t)
	if !appleDouble(ad) || !finderStore(ds) {
		t.Fatal("Mac format rejected")
	}
	if p := os.Getenv("BF_FINDER_METADATA"); p != "" {
		b, e := os.ReadFile(p)
		if e != nil || !finderStore(b) {
			t.Fatal("actual Finder store", e)
		}
	}
	for _, b := range [][]byte{nil, []byte("._not metadata"), ad[:25], ds[:32]} {
		if appleDouble(b) || finderStore(b) {
			t.Fatal("truncation accepted")
		}
	}
	for _, pos := range []int{0, 4, 24, 30, 34, 42, 46} {
		b := append([]byte(nil), ad...)
		binary.BigEndian.PutUint32(b[pos:], 0xffffffff)
		if appleDouble(b) {
			t.Fatal("invalid AppleDouble accepted", pos)
		}
	}
	b := append([]byte(nil), ds...)
	binary.BigEndian.PutUint32(b[8:], 0xffffff00)
	if finderStore(b) {
		t.Fatal("invalid Finder allocator accepted")
	}
	if appleDouble(make([]byte, metadataLimit+1)) || finderStore(make([]byte, metadataLimit+1)) {
		t.Fatal("unbounded metadata")
	}
}
func TestMetadataOwnedTree(t *testing.T) {
	for _, kind := range []string{"valid", "invalid-sidecar", "orphan", "hidden", "modified-companion", "symlink", "unknown-directory", "invalid-finder"} {
		t.Run(kind, func(t *testing.T) {
			r := root(t)
			mustWrite(t, r, app+"Install-Linux.desktop", []byte("package"))
			mustWrite(t, r, app+"computer/tool", []byte("package"))
			mustWrite(t, r, app+"._Install-Linux.desktop", macMetadata(t))
			mustWrite(t, r, app+"computer/.DS_Store", finderMetadata(t))
			switch kind {
			case "invalid-sidecar":
				os.WriteFile(filepath.Join(r, app+"._Install-Linux.desktop"), []byte("keep"), 0600)
			case "orphan":
				mustWrite(t, r, app+"._unrelated", macMetadata(t))
			case "hidden":
				mustWrite(t, r, app+".hidden", []byte("keep"))
			case "modified-companion":
				os.WriteFile(filepath.Join(r, app+"Install-Linux.desktop"), []byte("modified"), 0600)
			case "symlink":
				os.Remove(filepath.Join(r, app+"._Install-Linux.desktop"))
				if e := os.Symlink("Install-Linux.desktop", filepath.Join(r, app+"._Install-Linux.desktop")); e != nil {
					t.Skip(e)
				}
			case "unknown-directory":
				mustWrite(t, r, app+"unknown/.DS_Store", finderMetadata(t))
			case "invalid-finder":
				os.WriteFile(filepath.Join(r, app+"computer/.DS_Store"), []byte("keep"), 0600)
			}
			c, e := ownedTree(r, app[:len(app)-1], func(p string, b []byte) error {
				if (p == app+"Install-Linux.desktop" || p == app+"computer/tool") && string(b) == "package" {
					return nil
				}
				return fmt.Errorf("foreign %s", p)
			})
			if kind == "valid" {
				if e != nil || len(c) != 4 {
					t.Fatal(c, e)
				}
			} else if e == nil {
				t.Fatal("unknown input accepted")
			}
			if b, e := read(r, app+"computer/tool"); e != nil || string(b) != "package" {
				t.Fatal("input changed")
			}
		})
	}
}
func TestMetadataInstallUpdateUninstall(t *testing.T) {
	r, dir, _ := completeFixture(t)
	full := filepath.Join(filepath.Dir(dir), "full", filepath.FromSlash(app))
	dir = filepath.Join(full, "computer")
	for _, n := range []string{"Install-Linux.desktop", "computer/package.json"} {
		b, e := os.ReadFile(filepath.Join(full, n))
		if e != nil {
			t.Fatal(e)
		}
		mustWrite(t, r, app+n, b)
	}
	mustWrite(t, r, app+"._Install-Linux.desktop", macMetadata(t))
	mustWrite(t, r, app+"computer/.DS_Store", finderMetadata(t))
	if e := install(r, dir, filepath.Join(t.TempDir(), "update"), true, true, nil); e != nil {
		t.Fatal("update", e)
	}
	rec, e := activeRecovery(r)
	if e != nil {
		t.Fatal(e)
	}
	mustWrite(t, rec, "._recovery.json", macMetadata(t))
	mustWrite(t, rec, ".DS_Store", finderMetadata(t))
	mustWrite(t, r, system+"config/._better-favorites-installation.json", macMetadata(t))
	if e := completeUninstall(r, rec, dir, filepath.Join(t.TempDir(), "other-computer"), nil); e != nil {
		t.Fatal("portable uninstall", e)
	}
	if _, e := os.Stat(filepath.Join(r, app)); !os.IsNotExist(e) {
		t.Fatal("owned app remains", e)
	}
	if _, e := os.Stat(filepath.Join(r, system+"config/._better-favorites-installation.json")); !os.IsNotExist(e) {
		t.Fatal("owned receipt metadata remains", e)
	}
	if b, e := read(r, "Roms/favourite.json"); e != nil || string(b) != "favorite unchanged" {
		t.Fatal("favorites changed")
	}
}

func TestMetadataCrossOSUninstall(t *testing.T) {
	if os.Getenv("BF_METADATA_LINUX") != "1" {
		t.Skip("explicit Linux container execution required")
	}
	r, dir, _ := completeFixture(t)
	archive := t.TempDir()
	// Native Mac install, followed by the packaged Linux executable consuming the
	// same card journal and Mac-produced metadata. No original computer recovery.
	c := exec.Command("docker", "run", "--rm", "-v", r+":/card", "-v", dir+":/package:ro", "-v", archive+":/host", "debian:bookworm-slim", "/package/better-favorites-installer-linux-amd64", "uninstall", "--sd-root", "/card", "--powered-off", "--archive", "/host/archive")
	if out, e := c.CombinedOutput(); e != nil {
		t.Fatalf("Linux uninstall %v %s", e, out)
	}
	if _, e := os.Stat(filepath.Join(r, app)); !os.IsNotExist(e) {
		t.Fatal("owned folder remains", e)
	}
	if _, e := os.Stat(filepath.Join(archive, "archive/uninstall-result.json")); e != nil {
		t.Fatal("archive", e)
	}
}
