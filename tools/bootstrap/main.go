// Download/extract only. All SD-card changes remain owned by the existing installer.
package main

import (
	"archive/zip"
	"bytes"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"runtime"
	"strings"
	"time"
)

const repository = "Architeg/miyoo-better-favorites"

var tagPattern = regexp.MustCompile(`^v[0-9]+\.[0-9]+\.[0-9]+(?:-[A-Za-z0-9.-]+)?$`)
var filePattern = regexp.MustCompile(`^[A-Za-z0-9._/-]+$`)

type release struct {
	Immutable  bool    `json:"immutable"`
	Tag        string  `json:"tag_name"`
	Draft      bool    `json:"draft"`
	Prerelease bool    `json:"prerelease"`
	Published  string  `json:"published_at"`
	Assets     []asset `json:"assets"`
}
type asset struct {
	Name   string `json:"name"`
	URL    string `json:"browser_download_url"`
	Digest string `json:"digest"`
}
type options struct {
	action, tag, store, api string
	args                    []string
	testing                 bool
	client                  *http.Client
	invoke                  func(string, []string) error
}

func get(o options, url string, limit int64) ([]byte, error) {
	u := url
	if !strings.HasPrefix(u, "https://") && !o.testing {
		return nil, errors.New("HTTPS required")
	}
	response, err := o.client.Get(u)
	if err != nil {
		return nil, err
	}
	defer response.Body.Close()
	if response.StatusCode != 200 {
		return nil, fmt.Errorf("download HTTP %d", response.StatusCode)
	}
	if !strings.HasPrefix(response.Request.URL.String(), "https://") && !o.testing {
		return nil, errors.New("non-HTTPS redirect refused")
	}
	b, err := io.ReadAll(io.LimitReader(response.Body, limit+1))
	if err != nil {
		return nil, err
	}
	if int64(len(b)) > limit {
		return nil, errors.New("download exceeds limit")
	}
	return b, nil
}
func jsonGet(o options, url string, v interface{}) error {
	b, e := get(o, url, 4<<20)
	if e != nil {
		return e
	}
	return json.Unmarshal(b, v)
}
func tagCommit(o options, tag string) (string, error) {
	var ref struct {
		Object struct {
			Type string `json:"type"`
			SHA  string `json:"sha"`
			URL  string `json:"url"`
		} `json:"object"`
	}
	if e := jsonGet(o, o.api+"/git/ref/tags/"+tag, &ref); e != nil {
		return "", e
	}
	if !regexp.MustCompile(`^[a-f0-9]{40}$`).MatchString(ref.Object.SHA) || ref.Object.Type != "tag" {
		return "", errors.New("annotated release tag required")
	}
	var t struct {
		Object struct {
			Type string `json:"type"`
			SHA  string `json:"sha"`
		} `json:"object"`
	}
	// Construct URLs ourselves; never follow a metadata-supplied API host.
	if e := jsonGet(o, o.api+"/git/tags/"+ref.Object.SHA, &t); e != nil {
		return "", e
	}
	if t.Object.Type != "commit" || !regexp.MustCompile(`^[a-f0-9]{40}$`).MatchString(t.Object.SHA) {
		return "", errors.New("invalid release commit")
	}
	return ref.Object.SHA + ":" + t.Object.SHA, nil
}
func archiveAsset(r release, name string) (asset, error) {
	var out asset
	n := 0
	for _, a := range r.Assets {
		if a.Name == name {
			out = a
			n++
		}
	}
	if n != 1 {
		return out, fmt.Errorf("expected exactly one release asset %s", name)
	}
	return out, nil
}
func sums(data []byte) (map[string]string, error) {
	out := map[string]string{}
	for _, line := range strings.Split(strings.TrimSpace(string(data)), "\n") {
		if len(line) < 67 || line[64:66] != "  " {
			return nil, errors.New("malformed checksums")
		}
		digest, name := line[:64], line[66:]
		if _, e := hex.DecodeString(digest); e != nil || len(digest) != 64 || !safePath(name) {
			return nil, errors.New("unsafe checksum entry")
		}
		if _, ok := out[name]; ok {
			return nil, errors.New("duplicate checksum entry")
		}
		out[name] = strings.ToLower(digest)
	}
	return out, nil
}
func hash(b []byte) string { h := sha256.Sum256(b); return hex.EncodeToString(h[:]) }
func safePath(name string) bool {
	return name != "" && filePattern.MatchString(name) && !strings.HasPrefix(name, "/") && !strings.Contains(name, "\\") && filepath.ToSlash(filepath.Clean(name)) == name && name != "." && name != ".." && !strings.HasPrefix(name, "../")
}
func extract(data []byte, dest string) error {
	z, e := zip.NewReader(bytes.NewReader(data), int64(len(data)))
	if e != nil {
		return e
	}
	if len(z.File) > 10000 {
		return errors.New("too many ZIP entries")
	}
	seen := map[string]bool{}
	var total uint64
	for _, f := range z.File {
		name := strings.TrimSuffix(f.Name, "/")
		key := strings.ToLower(name)
		if !safePath(name) || seen[key] || f.Mode()&os.ModeSymlink != 0 || (!f.Mode().IsRegular() && !f.FileInfo().IsDir()) {
			return errors.New("unsafe or duplicate ZIP member")
		}
		seen[key] = true
		total += f.UncompressedSize64
		if f.UncompressedSize64 > 512<<20 || total > 1<<30 {
			return errors.New("ZIP expansion exceeds limit")
		}
	}
	for _, f := range z.File {
		path := filepath.Join(dest, filepath.FromSlash(f.Name))
		if f.FileInfo().IsDir() {
			if e = os.MkdirAll(path, 0700); e != nil {
				return e
			}
			continue
		}
		if e = os.MkdirAll(filepath.Dir(path), 0700); e != nil {
			return e
		}
		in, e := f.Open()
		if e != nil {
			return e
		}
		mode := os.FileMode(0600)
		if f.Mode()&0111 != 0 {
			mode = 0700
		}
		out, e := os.OpenFile(path, os.O_CREATE|os.O_EXCL|os.O_WRONLY, mode)
		if e != nil {
			in.Close()
			return e
		}
		_, copyErr := io.Copy(out, in)
		in.Close()
		closeErr := out.Close()
		if copyErr != nil {
			return copyErr
		}
		if closeErr != nil {
			return closeErr
		}
	}
	return nil
}
func verifyPackage(dir, commit string) error {
	data, e := os.ReadFile(filepath.Join(dir, "SHA256SUMS"))
	if e != nil {
		return e
	}
	checks, e := sums(data)
	if e != nil {
		return e
	}
	e = filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if info.IsDir() {
			return nil
		}
		rel, _ := filepath.Rel(dir, path)
		name := filepath.ToSlash(rel)
		if name == "SHA256SUMS" {
			return nil
		}
		b, e := os.ReadFile(path)
		if e != nil {
			return e
		}
		if checks[name] != hash(b) {
			return fmt.Errorf("package checksum mismatch: %s", name)
		}
		delete(checks, name)
		return nil
	})
	if e != nil {
		return e
	}
	if len(checks) != 0 {
		return errors.New("missing package files")
	}
	var p struct {
		Commit string `json:"commit"`
	}
	b, e := os.ReadFile(filepath.Join(dir, "package.json"))
	if e != nil {
		return e
	}
	if e = json.Unmarshal(b, &p); e != nil {
		return e
	}
	if p.Commit != commit {
		return errors.New("package source does not match annotated tag")
	}
	return nil
}
func run(o options) (string, error) {
	if o.action != "install" && o.action != "uninstall" {
		return "", errors.New("use install or uninstall")
	}
	if o.action == "uninstall" && o.tag == "" {
		return "", errors.New("uninstall requires the installed release tag and matching recovery; latest is not assumed compatible")
	}
	if runtime.GOOS != "windows" && runtime.GOOS != "darwin" && runtime.GOOS != "linux" {
		return "", errors.New("unsupported OS")
	}
	endpoint := "/releases/latest"
	if o.tag != "" {
		if !tagPattern.MatchString(o.tag) {
			return "", errors.New("invalid release tag")
		}
		endpoint = "/releases/tags/" + o.tag
	}
	var r release
	if e := jsonGet(o, o.api+endpoint, &r); e != nil {
		return "", e
	}
	if !r.Immutable || r.Draft || r.Published == "" || !tagPattern.MatchString(r.Tag) || (o.tag == "" && r.Prerelease) || (o.tag != "" && r.Tag != o.tag) {
		return "", errors.New("release is not eligible/published")
	}
	pin, e := tagCommit(o, r.Tag)
	if e != nil {
		return "", e
	}
	name := "better-favorites-" + strings.TrimPrefix(r.Tag, "v") + ".zip"
	a, e := archiveAsset(r, name)
	if e != nil {
		return "", e
	}
	s, e := archiveAsset(r, "SHA256SUMS")
	if e != nil {
		return "", e
	}
	if !o.testing {
		prefix := "https://github.com/" + repository + "/releases/download/" + r.Tag + "/"
		if a.URL != prefix+name || s.URL != prefix+"SHA256SUMS" {
			return "", errors.New("unexpected release download location")
		}
	}
	b, e := get(o, s.URL, 1<<20)
	if e != nil {
		return "", e
	}
	checks, e := sums(b)
	if e != nil {
		return "", e
	}
	expected := checks[name]
	if len(expected) != 64 {
		return "", errors.New("installer checksum absent")
	}
	archive, e := get(o, a.URL, 256<<20)
	if e != nil {
		return "", e
	}
	if hash(archive) != expected {
		return "", errors.New("archive checksum mismatch")
	}
	if a.Digest != "" && a.Digest != "sha256:"+expected {
		return "", errors.New("asset digest mismatch")
	}
	if e = os.MkdirAll(o.store, 0700); e != nil {
		return "", e
	}
	// Retain the package/recovery location. Never remove it as a temporary download.
	dir, e := os.MkdirTemp(o.store, r.Tag+"-")
	if e != nil {
		return "", e
	}
	if e = os.WriteFile(filepath.Join(dir, "download.zip"), archive, 0600); e != nil {
		return dir, e
	}
	pkg := filepath.Join(dir, "installer")
	if e = os.Mkdir(pkg, 0700); e != nil {
		return dir, e
	}
	if e = extract(archive, pkg); e != nil {
		return dir, e
	}
	if e = verifyPackage(pkg, strings.Split(pin, ":")[1]); e != nil {
		return dir, e
	}
	again, e := tagCommit(o, r.Tag)
	if e != nil {
		return dir, e
	}
	if again != pin {
		return dir, errors.New("release tag changed during download")
	}
	fmt.Println("Verified package retained at:", dir, "(keep its recovery files)")
	if o.invoke != nil {
		return dir, o.invoke(pkg, append([]string{o.action}, o.args...))
	}
	hostTools := filepath.Join(pkg, "App", "BetterFavorites", "computer")
	var cmd *exec.Cmd
	if runtime.GOOS == "windows" {
		cmd = exec.Command(filepath.Join(hostTools, "better-favorites-dispatch-windows-386.exe"), append([]string{o.action}, o.args...)...)
	} else {
		entry := "Install-Linux.sh"
		if runtime.GOOS == "darwin" {
			entry = "Install-macOS.command"
		}
		cmd = exec.Command("/bin/sh", filepath.Join(hostTools, entry))
		cmd.Args = append(cmd.Args, append([]string{o.action}, o.args...)...)
	}
	cmd.Dir = hostTools
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return dir, cmd.Run()
}
func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "Usage: bootstrap install|uninstall [--tag vX.Y.Z] [--store DIRECTORY] [-- installer arguments]")
		os.Exit(2)
	}
	home, e := os.UserHomeDir()
	if e != nil {
		fmt.Fprintln(os.Stderr, e)
		os.Exit(2)
	}
	f := flag.NewFlagSet("bootstrap", flag.ContinueOnError)
	tag := f.String("tag", "", "published tag; required for uninstall")
	store := f.String("store", filepath.Join(home, "BetterFavorites-Downloads"), "persistent computer folder")
	if e = f.Parse(os.Args[2:]); e != nil {
		os.Exit(2)
	}
	client := &http.Client{Timeout: 3 * time.Minute, CheckRedirect: func(req *http.Request, via []*http.Request) error {
		if req.URL.Scheme != "https" {
			return errors.New("HTTPS redirect required")
		}
		if len(via) > 5 {
			return errors.New("too many redirects")
		}
		return nil
	}}
	o := options{action: os.Args[1], tag: *tag, store: *store, args: f.Args(), api: "https://api.github.com/repos/" + repository, client: client}
	_, e = run(o)
	if e != nil {
		fmt.Fprintln(os.Stderr, "Bootstrap:", e)
		var exit *exec.ExitError
		if errors.As(e, &exit) {
			os.Exit(exit.ExitCode())
		}
		os.Exit(1)
	}
}
