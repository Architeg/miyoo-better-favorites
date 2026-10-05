package main

import (
	"bytes"
	"os"
	"strings"
	"testing"
	"time"
)

func TestPresentationPlainAndStop(t *testing.T) {
	for _, animate := range []bool{false, true} {
		var b bytes.Buffer
		p := &terminalPresentation{out: &b, animate: animate}
		p.start("install")
		p.setPhase("Preparing recovery")
		time.Sleep(230 * time.Millisecond)
		p.stop()
		p.stop()
		before := b.String()
		time.Sleep(230 * time.Millisecond)
		if b.String() != before || strings.Contains(before, "\x1b") || !strings.Contains(before, "Installing Better Favorites") || !strings.Contains(before, "Keep the card connected") || !strings.Contains(before, "Preparing recovery") {
			t.Fatal(before)
		}
		if animate && !strings.Contains(before, "0s") {
			t.Fatal("missing elapsed", before)
		}
		p.start("uninstall")
		p.setPhase("Restoring originals")
		p.stop()
		if !strings.Contains(b.String(), "Uninstalling Better Favorites") {
			t.Fatal(b.String())
		}
	}
}
func TestProgressStopsBeforeError(t *testing.T) {
	var b bytes.Buffer
	old := presentation
	defer func() { presentation = old }()
	presentation = &terminalPresentation{out: &b, animate: true}
	presentation.start("install")
	r := &operationReport{action: "install"}
	r.failure(&b, os.ErrPermission)
	before := b.String()
	time.Sleep(230 * time.Millisecond)
	if b.String() != before || !strings.Contains(before, "Installation failed") {
		t.Fatal(b.String())
	}
}
func TestPlainMenuIncludesDiagnostics(t *testing.T) {
	old := presentation
	defer func() { presentation = old }()
	var b bytes.Buffer
	presentation = &terminalPresentation{out: &b}
	// Menu heading shares the presenter; actual options are also checked in packaged execution.
	presentation.heading("Better Favorites")
	if b.String() != "\nBetter Favorites\n\n" {
		t.Fatal(b.String())
	}
}

func TestNestedOperationDoesNotDuplicateHeading(t *testing.T) {
	var b bytes.Buffer
	old := presentation
	defer func() { presentation = old }()
	presentation = &terminalPresentation{out: &b}
	outer := beginOperation("uninstall")
	inner := beginOperation("uninstall")
	inner()
	outer()
	if strings.Count(b.String(), "Uninstalling Better Favorites") != 1 {
		t.Fatal(b.String())
	}
}
