package main

import (
	"archive/zip"
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"net/http/httptest"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

const testCommit = "0123456789012345678901234567890123456789"

func testZip(extra string, symlink bool) []byte {
	b := new(bytes.Buffer)
	w := zip.NewWriter(b)
	contents := map[string]string{"package.json": `{"commit":"` + testCommit + `"}`, "App/BetterFavorites/computer/Install-macOS.command": "#!/bin/sh\nexit 17\n", "App/BetterFavorites/computer/Install-Linux.sh": "#!/bin/sh\nexit 17\n"}
	sums := ""
	for _, n := range []string{"package.json", "App/BetterFavorites/computer/Install-macOS.command", "App/BetterFavorites/computer/Install-Linux.sh"} {
		sums += hash([]byte(contents[n])) + "  " + n + "\n"
	}
	contents["SHA256SUMS"] = sums
	for n, s := range contents {
		f, _ := w.Create(n)
		f.Write([]byte(s))
	}
	if extra != "" {
		h := &zip.FileHeader{Name: extra}
		if symlink {
			h.SetMode(os.ModeSymlink | 0700)
		}
		f, _ := w.CreateHeader(h)
		f.Write([]byte("bad"))
	}
	w.Close()
	return b.Bytes()
}
func fixture(t *testing.T, mutation string) (options, *int) {
	t.Helper()
	data := testZip("", false)
	if mutation == "traversal" {
		data = testZip("../foreign", false)
	}
	if mutation == "symlink" {
		data = testZip("link", true)
	}
	if mutation == "extra" {
		data = testZip("unlisted", false)
	}
	calls := 0
	refCalls := 0
	var server *httptest.Server
	server = httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		switch r.URL.Path {
		case "/releases/latest", "/releases/tags/v1.0.0-rc.4":
			tag := "v1.0.0-rc.4"
			name := "better-favorites-1.0.0-rc.4.zip"
			draft := mutation == "draft"
			pre := mutation == "prerelease"
			published := "2026-10-04T00:00:00Z"
			if mutation == "unpublished" {
				published = ""
			}
			if mutation == "wrongtag" {
				tag = "v2.0.0"
			}
			assets := []asset{{Name: name, URL: server.URL + "/download.zip"}, {Name: "SHA256SUMS", URL: server.URL + "/sums"}}
			if mutation == "ambiguous" {
				assets = append(assets, assets[0])
			}
			if mutation == "digest" {
				assets[0].Digest = "sha256:" + strings.Repeat("0", 64)
			}
			json.NewEncoder(w).Encode(release{Immutable: mutation != "mutable", Tag: tag, Draft: draft, Prerelease: pre, Published: published, Assets: assets})
		case "/git/ref/tags/v1.0.0-rc.4":
			refCalls++
			sha := strings.Repeat("a", 40)
			if mutation == "moved" && refCalls > 1 {
				sha = strings.Repeat("b", 40)
			}
			kind := "tag"
			if mutation == "lightweight" {
				kind = "commit"
			}
			fmt.Fprintf(w, `{"object":{"type":%q,"sha":%q}}`, kind, sha)
		case "/git/tags/" + strings.Repeat("a", 40), "/git/tags/" + strings.Repeat("b", 40):
			commit := testCommit
			if mutation == "source" {
				commit = strings.Repeat("0", 40)
			}
			fmt.Fprintf(w, `{"object":{"type":"commit","sha":%q}}`, commit)
		case "/download.zip":
			w.Write(data)
		case "/sums":
			digest := hash(data)
			if mutation == "hash" {
				digest = strings.Repeat("0", 64)
			}
			fmt.Fprintf(w, "%s  better-favorites-1.0.0-rc.4.zip\n", digest)
		default:
			http.Error(w, "missing", 404)
		}
	}))
	t.Cleanup(server.Close)
	o := options{action: "install", tag: "v1.0.0-rc.4", store: filepath.Join(t.TempDir(), "durable"), api: server.URL, testing: true, client: server.Client(), invoke: func(dir string, args []string) error {
		calls++
		if args[0] != "install" && args[0] != "uninstall" {
			t.Fatal(args)
		}
		return nil
	}}
	return o, &calls
}
func TestPublishedPackageDelegatesAndRetains(t *testing.T) {
	o, calls := fixture(t, "")
	o.args = []string{"--sd-root", "card path", "--powered-off"}
	o.invoke = func(dir string, args []string) error {
		*calls++
		if strings.Join(args, "|") != "install|--sd-root|card path|--powered-off" {
			t.Fatal(args)
		}
		return nil
	}
	dir, e := run(o)
	if e != nil || *calls != 1 {
		t.Fatal(dir, e, *calls)
	}
	if _, e = os.Stat(filepath.Join(dir, "download.zip")); e != nil {
		t.Fatal(e)
	}
}
func TestRefusalsNeverInvokeInstaller(t *testing.T) {
	for _, m := range []string{"draft", "mutable", "unpublished", "wrongtag", "ambiguous", "digest", "lightweight", "hash", "source", "moved", "traversal", "symlink", "extra"} {
		t.Run(m, func(t *testing.T) {
			o, c := fixture(t, m)
			_, e := run(o)
			if e == nil || *c != 0 {
				t.Fatal(e, *c)
			}
		})
	}
}
func TestStableSelectionAndUninstall(t *testing.T) {
	o, c := fixture(t, "prerelease")
	o.tag = ""
	if _, e := run(o); e == nil || *c != 0 {
		t.Fatal(e)
	}
	o, c = fixture(t, "")
	o.action = "uninstall"
	o.tag = ""
	if _, e := run(o); e == nil || *c != 0 {
		t.Fatal(e)
	}
	o.tag = "v1.0.0-rc.4"
	if _, e := run(o); e != nil || *c != 1 {
		t.Fatal(e)
	}
}
func TestChildFailurePreservesPackage(t *testing.T) {
	o, c := fixture(t, "")
	o.invoke = func(string, []string) error { return errors.New("child failure") }
	dir, e := run(o)
	if e == nil || *c != 0 {
		t.Fatal(e)
	}
	if _, e = os.Stat(filepath.Join(dir, "installer", "SHA256SUMS")); e != nil {
		t.Fatal(e)
	}
}
func TestHTTPSRequired(t *testing.T) {
	o, c := fixture(t, "")
	o.testing = false
	if _, e := run(o); e == nil || *c != 0 {
		t.Fatal(e)
	}
}
func TestUnsafePaths(t *testing.T) {
	for _, p := range []string{"../x", "/x", "a/../../x", "a\\x", "C:/x", "a/./x", "a//x", "."} {
		if safePath(p) {
			t.Fatal(p)
		}
	}
}

func TestActualEntryExitStatus(t *testing.T) {
	if runtime.GOOS == "windows" {
		t.Skip("Unix fixture; Windows native qualification separate")
	}
	o, _ := fixture(t, "")
	o.invoke = nil
	_, err := run(o)
	var child *exec.ExitError
	if !errors.As(err, &child) || child.ExitCode() != 17 {
		t.Fatal(err)
	}
}
