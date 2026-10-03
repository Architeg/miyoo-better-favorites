//go:build !windows

package main

import (
	"os"
	"path/filepath"
	"syscall"
	"testing"
	"time"
)

func TestNonblockingFIFO(t *testing.T) {
	p := filepath.Join(t.TempDir(), "fifo")
	if e := syscall.Mkfifo(p, 0600); e != nil {
		t.Fatal(e)
	}
	done := make(chan error, 1)
	go func() { _, e := regular(p); done <- e }()
	select {
	case e := <-done:
		if e == nil {
			t.Fatal("FIFO accepted")
		}
	case <-time.After(time.Second):
		t.Fatal("FIFO open blocked")
	}
}

func TestArchiveAliasCannotEnterCard(t *testing.T) {
	card := root(t)
	host := root(t)
	alias := filepath.Join(host, "alias")
	if e := os.Symlink(card, alias); e != nil {
		t.Fatal(e)
	}
	if e := outsideCard(card, filepath.Join(alias, "recovery")); e == nil {
		t.Fatal("card alias accepted as external archive")
	}
	if e := outsideCard(card, filepath.Join(host, "recovery")); e != nil {
		t.Fatal(e)
	}
}
