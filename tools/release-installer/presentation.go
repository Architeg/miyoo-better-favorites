package main

import (
	"fmt"
	"io"
	"os"
	"runtime"
	"sync"
	"time"
)

// Presentation only: the caller keeps all transaction work synchronous. The
// short-lived ticker owns no card files and is stopped before errors/completion.
var presentation *terminalPresentation

type terminalPresentation struct {
	mu              sync.Mutex
	out             io.Writer
	styled, animate bool
	active          bool
	phase           string
	started         time.Time
	quit, done      chan struct{}
}

func newPresentation(f *os.File) *terminalPresentation {
	s, e := f.Stat()
	tty := e == nil && s.Mode()&os.ModeCharDevice != 0
	return &terminalPresentation{out: f, animate: tty, styled: tty && runtime.GOOS != "windows" && os.Getenv("TERM") != "" && os.Getenv("TERM") != "dumb" && os.Getenv("NO_COLOR") == ""}
}
func (p *terminalPresentation) heading(s string) {
	p.mu.Lock()
	defer p.mu.Unlock()
	if p.styled {
		fmt.Fprintf(p.out, "\n\x1b[1m%s\x1b[0m\n\n", s)
	} else {
		fmt.Fprintf(p.out, "\n%s\n\n", s)
	}
}
func (p *terminalPresentation) start(action string) {
	p.stop()
	title := "Installing Better Favorites…"
	if action == "uninstall" {
		title = "Uninstalling Better Favorites…"
	}
	p.heading(title)
	p.mu.Lock()
	fmt.Fprint(p.out, "Keep the card connected until completion.\n\n")
	p.started = time.Now()
	p.active = true
	p.phase = "Preparing"
	if !p.animate {
		p.mu.Unlock()
		return
	}
	p.quit = make(chan struct{})
	p.done = make(chan struct{})
	quit, done := p.quit, p.done
	p.mu.Unlock()
	go func() {
		defer close(done)
		ticker := time.NewTicker(200 * time.Millisecond)
		defer ticker.Stop()
		frames := []string{"|", "/", "-", "\\"}
		i := 0
		for {
			select {
			case <-quit:
				return
			case <-ticker.C:
				p.mu.Lock()
				fmt.Fprintf(p.out, "\r  %s %-30s %ds", frames[i%len(frames)], p.phase, int(time.Since(p.started).Seconds()))
				p.mu.Unlock()
				i++
			}
		}
	}()
}
func (p *terminalPresentation) setPhase(s string) {
	p.mu.Lock()
	defer p.mu.Unlock()
	p.phase = s
	if !p.animate {
		fmt.Fprintf(p.out, "  %s\n", s)
	}
}
func (p *terminalPresentation) stop() {
	p.mu.Lock()
	q, d := p.quit, p.done
	p.active = false
	p.quit = nil
	p.done = nil
	p.mu.Unlock()
	if q != nil {
		close(q)
		<-d
		p.mu.Lock()
		fmt.Fprintln(p.out)
		p.mu.Unlock()
	}
}
func beginOperation(action string) func() {
	if presentation == nil {
		return func() {}
	}
	p := presentation
	p.mu.Lock()
	active := p.active
	p.mu.Unlock()
	if !active {
		p.start(action)
	}
	return p.stop
}
func progressPhase(s string) {
	if presentation != nil {
		presentation.setPhase(s)
	}
}
