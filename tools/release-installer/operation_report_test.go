package main

import (
	"bytes"
	"errors"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestOperationReportStates(t *testing.T) {
	for _, kind := range []string{"preflight", "preparation", "rollback", "partial", "conflict"} {
		t.Run(kind, func(t *testing.T) {
			r := root(t)
			path := filepath.Join(t.TempDir(), "details.log")
			f, e := os.Create(path)
			if e != nil {
				t.Fatal(e)
			}
			defer f.Close()
			old := operationLog
			operationLog = &operationReport{card: r, action: "install", path: path, log: f}
			defer func() { operationLog = old }()
			want := "No card changes"
			switch kind {
			case "preparation":
				reportPreparation(r)
				want = "No installed-file publication"
			case "rollback":
				mustWrite(t, r, "owned", []byte("old"))
				e = transact(r, []change{{"owned", []byte("old"), []byte("new"), 0600, nil, false}}, func(phase, p string) error {
					if phase == "after" {
						return errors.New("injected")
					}
					return nil
				})
				if e == nil {
					t.Fatal("missing failure")
				}
				want = "rolled back and verified"
			case "partial":
				reportTransaction(r, 1, false, nil)
				want = "Card files changed"
			case "conflict":
				reportTransaction(r, 1, true, []string{"owned"})
				want = "unresolved"
			}
			var out bytes.Buffer
			operationLog.failure(&out, errors.New("ambiguous/invalid OS metadata preserved: App/BetterFavorites/._computer"))
			if !strings.Contains(out.String(), "Reason: ambiguous/invalid OS metadata preserved: App/BetterFavorites/._computer") || !strings.Contains(out.String(), want) || !strings.Contains(out.String(), "Next:") || !strings.Contains(out.String(), "Detailed log:") || strings.Contains(out.String(), "\x1b") {
				t.Fatal(out.String())
			}
			b, _ := os.ReadFile(path)
			if !bytes.Contains(b, []byte("._computer")) {
				t.Fatal("reason lost")
			}
		})
	}
}

func TestOperationReportBoundedBestEffort(t *testing.T) {
	p := filepath.Join(t.TempDir(), "log")
	f, e := os.Create(p)
	if e != nil {
		t.Fatal(e)
	}
	defer f.Close()
	r := &operationReport{path: p, log: f}
	r.detail("%s", strings.Repeat("x", 1024*1024))
	r.detail("small evidence")
	b, _ := os.ReadFile(p)
	if string(b) != "small evidence\n" {
		t.Fatal("log not bounded")
	}
	f.Close()
	r.detail("write failure")
	var out bytes.Buffer
	r.failure(&out, errors.New("actual failure"))
	if !strings.Contains(out.String(), "actual failure") || !strings.Contains(out.String(), "could not be saved") {
		t.Fatal(out.String())
	}
}
