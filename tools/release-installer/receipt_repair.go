package main

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
)

// Called only after catalogue, all installed MainUI bytes and original backup
// bytes have been checked. One authenticated journal must also bind the exact
// manifest, receipt, runtime and installed MainUI identities together.
func authenticateMissingReceipt(root, marker string, manifest []byte, h HomeSpec, stockRuntime string) error {
	active, err := activeRecovery(root)
	if err != nil {
		return fmt.Errorf("missing receipt %s: verified indexed recovery required: %w", marker, err)
	}
	mirrors, err := ownedRecoveryMirrors(root, active)
	if err != nil {
		return err
	}
	runtime, err := read(root, system+"runtime.sh")
	if err != nil {
		return err
	}
	for rel := range mirrors {
		journal, err := loadRecovery(filepath.Join(root, filepath.FromSlash(rel)))
		if err != nil {
			return err
		}
		records := map[string]Saved{}
		for _, s := range journal.Files {
			records[s.Path] = s
		}
		if records[marker].After != digest(receipt(h)) || records[homeManifest].After != digest(manifest) {
			continue
		}
		rr, ok := records[system+"runtime.sh"]
		if (ok && rr.After != digest(runtime)) || (!ok && digest(runtime) != stockRuntime) {
			continue
		}
		matches := true
		for _, n := range names {
			s, ok := records[system+"bin/"+n]
			if !ok || !s.Integration || s.After != h.Patched[n] || s.Stock != h.Original[n] {
				matches = false
			}
		}
		if matches {
			return nil
		}
	}
	return fmt.Errorf("missing receipt %s: journal, integration manifest and installed system identities do not agree; preserved", marker)
}

// Only verified recovery ownership can authorize an orphan companion after an
// interrupted removal or quarantine. New package names alone are insufficient.
func recordedMissing(own map[string]map[string]bool) map[string]bool {
	out := lifecycleMissing(own)
	for p, hashes := range own {
		if appPrefix(p) == "" || metadataName(p) || !allowedRecoveryApp(p) {
			continue
		}
		for h := range hashes {
			if len(h) == 64 {
				out[p] = true
			}
		}
	}
	return out
}

func homeReceiptPath(p string) bool {
	return p == app+"home-integration.conf" || p == legacyApp+"home-integration.conf"
}

// Stock binaries plus an exact, verified recovery record establish obsolescence.
// A familiar filename or catalogue receipt alone never establishes ownership.
func authenticateObsoleteReceipt(root, dir string) (bool, error) {
	marker, err := optional(root, app+"home-integration.conf")
	if err != nil || marker == nil {
		return false, err
	}
	_, home, ret, err := loadPackage(dir)
	if err != nil {
		return false, err
	}
	runtime, err := read(root, system+"runtime.sh")
	if err != nil {
		return false, err
	}
	if digest(runtime) != ret.Original {
		return false, nil
	}
	for _, n := range names {
		b, err := read(root, system+"bin/"+n)
		if err != nil {
			return false, err
		}
		if digest(b) != home.Original[n] {
			return false, nil
		}
	}
	recovery, err := activeRecovery(root)
	if errors.Is(err, os.ErrNotExist) {
		recovery, err = discoverRecovery(root, dir)
	}
	if err != nil {
		return false, fmt.Errorf("obsolete receipt preserved: verified recovery required: %w", err)
	}
	journal, err := loadRecovery(recovery)
	if err != nil {
		return false, err
	}
	records := map[string]Saved{}
	for _, s := range journal.Files {
		records[s.Path] = s
	}
	receiptRecord := records[app+"home-integration.conf"]
	if receiptRecord.Stock != "absent" || receiptRecord.After != digest(marker) || !acceptedReceipt(dir, digest(marker), app+"home-integration.conf") {
		return false, fmt.Errorf("obsolete receipt preserved: recovery ownership mismatch")
	}
	for _, n := range names {
		s := records[system+"bin/"+n]
		if !s.Integration || s.Stock != home.Original[n] || !acceptedHomeHash(dir, n, s.After) {
			return false, fmt.Errorf("obsolete receipt preserved: incomplete system recovery")
		}
	}
	if _, err = prepareRestore(root, recovery, dir, true); err != nil {
		return false, err
	}
	reportDetail("Verified stock system and recovery authenticate obsolete app receipt; full install will replace it transactionally")
	return true, nil
}
