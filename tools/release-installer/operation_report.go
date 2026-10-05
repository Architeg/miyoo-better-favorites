package main

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"
)

// Computer-side diagnostics only. No change to card receipts or ownership rules.
// nil during ordinary unit calls; main creates one report for the helper process.
var operationLog *operationReport

type operationReport struct {
	card, action, path, unavailable  string
	log                              *os.File
	size                             int
	preparing, rolledBack, uncertain bool
	committed                        int
}

func (r *operationReport) detail(format string, args ...any) {
	if r == nil || r.log == nil {
		return
	}
	b := []byte(fmt.Sprintf(format, args...) + "\n")
	if r.size+len(b) > 1024*1024 {
		return
	}
	n, e := r.log.Write(b)
	r.size += n
	if e != nil {
		r.unavailable = e.Error()
		return
	}
	_ = r.log.Sync()
}
func reportCard(root string) {
	r := operationLog
	if r == nil {
		return
	}
	abs, e := filepath.Abs(root)
	if e != nil {
		return
	}
	r.card = filepath.Clean(abs)
	if r.log != nil {
		return
	}
	home, e := os.UserHomeDir()
	if e != nil {
		r.unavailable = e.Error()
		return
	}
	dir := filepath.Join(home, "BetterFavorites-Logs")
	if e = outsideCard(r.card, dir); e != nil {
		r.unavailable = e.Error()
		return
	}
	if _, e = join(home, "BetterFavorites-Logs"); e != nil {
		r.unavailable = e.Error()
		return
	}
	if e = os.MkdirAll(dir, 0700); e != nil {
		r.unavailable = e.Error()
		return
	}
	path := filepath.Join(dir, "installer-"+time.Now().UTC().Format("20060102T150405.000000000Z")+".log")
	r.log, e = os.OpenFile(path, os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0600)
	if e != nil {
		r.unavailable = e.Error()
		return
	}
	r.path = path
	r.detail("Better Favorites installer\nCard: %s\nState: preflight; no card writes by this attempt", r.card)
}
func reportPreparation(root string) {
	r := operationLog
	if r == nil || filepath.Clean(root) != r.card {
		return
	}
	if !r.preparing {
		r.preparing = true
		r.detail("State: card preparation/recovery writes started")
	}
}
func reportTransaction(root string, count int, failed bool, conflicts []string) {
	r := operationLog
	if r == nil || filepath.Clean(root) != r.card {
		return
	}
	if !failed {
		r.committed += count
		r.detail("Transaction verified: %d publications", count)
		return
	}
	if len(conflicts) > 0 {
		r.uncertain = true
		r.detail("Rollback unresolved: %v", conflicts)
	} else if count > 0 {
		r.rolledBack = true
		r.detail("Rollback verified: %d publications", count)
	}
}
func reportDetail(format string, args ...any) {
	if operationLog != nil {
		operationLog.detail(format, args...)
	} else {
		fmt.Printf(format+"\n", args...)
	}
}
func (r *operationReport) state() string {
	if r.uncertain {
		return "Card state is unresolved: some rollback checks failed."
	}
	if r.committed > 0 {
		return "Card files changed. This operation did not complete."
	}
	if r.rolledBack {
		return "Published changes were rolled back and verified. Recovery/staging files may remain."
	}
	if r.preparing {
		return "No installed-file publication was completed. Recovery/staging files may remain."
	}
	return "No card changes were made by this attempt."
}
func (r *operationReport) failure(w io.Writer, e error) {
	if presentation != nil {
		presentation.stop()
	}
	operation := "Operation"
	if r.action == "install" {
		operation = "Installation"
	}
	if r.action == "uninstall" {
		operation = "Uninstall"
	}
	reason, next := "The operation could not finish.", "Keep the card powered off. Share the detailed log before retrying."
	s := strings.ToLower(e.Error())
	if strings.Contains(s, "metadata") || strings.Contains(s, "unknown") || strings.Contains(s, "modified") || strings.Contains(s, "conflict") || strings.Contains(s, "checksum") {
		reason = "A file or folder could not be verified and was preserved."
		next = "Keep the reported files and recovery backups. Share the log; do not delete files to bypass the check."
	} else if strings.Contains(s, "recovery") {
		reason = "Verified recovery could not be used."
		next = "Keep the app and all recovery backups. Follow the offline recovery guide or share the log."
	}
	if r.committed > 0 || r.uncertain {
		next = "Keep the card powered off. Use the offline recovery guide and verified recovery; share this log if restoration fails."
	}
	r.detail("Action: %s\nERROR: %v\n%s\nNext: %s", r.action, e, r.state(), next)
	fmt.Fprintf(w, "\n%s failed\n\n%s\n%s\n\nNext: %s\n", operation, reason+"\nReason: "+e.Error(), r.state(), next)
	if r.path != "" && r.unavailable == "" {
		fmt.Fprintf(w, "Detailed log: %s\n", r.path)
	} else {
		fmt.Fprintf(w, "Detailed log could not be saved. Reason: %v\nLog error: %s\n", e, r.unavailable)
	}
}
func installerMenu() {
	if presentation != nil {
		presentation.stop()
		presentation.heading("Better Favorites")
	} else {
		fmt.Print("\nBetter Favorites\n\n")
	}
	fmt.Print("[1] Install / Update\n[2] Uninstall completely\n[3] Export diagnostics\n[0] Cancel\n\n")
}

// Lifecycle-boundary evidence only; diagnostics never affect publication.
func traceIntegrationMetadata(root, p, phase string) {
	if operationLog == nil || (p != homeManifest && p != returnBackup+"manifest.json") {
		return
	}
	side := filepath.ToSlash(filepath.Join(filepath.Dir(p), "._"+filepath.Base(p)))
	b, e := optional(root, side)
	operationLog.detail("Metadata %s: %s sha256=%s read_error=%v", phase, side, hashOrAbsent(b), e)
}
