package main

import (
	"fmt"
	"path/filepath"
	"sort"
	"strings"
)

// All inputs come from cleanupPlan, whose actual payloads remain immutable.
// Only companions of these authenticated paths and their project-owned
// directories can be refreshed. Shared directories never gain Finder ownership.
func cleanupWithMetadata(root string, changes []change, host, phase string, hook func(string, string) error) error {
	var data []change
	candidates, targets, dirs := map[string]bool{}, map[string]bool{}, map[string]bool{}
	for _, c := range changes {
		if metadataName(c.Path) {
			candidates[c.Path] = true
			if t := metadataTarget(c.Path); t != "" {
				targets[t] = true
			}
			continue
		}
		data = append(data, c)
		targets[c.Path] = true
		candidates[filepath.ToSlash(filepath.Join(filepath.Dir(c.Path), "._"+filepath.Base(c.Path)))] = true
		for d := filepath.ToSlash(filepath.Dir(c.Path)); projectMetadataDirectory(d); d = filepath.ToSlash(filepath.Dir(d)) {
			dirs[d] = true
			targets[d] = true
		}
	}
	for d := range dirs {
		candidates[d+"/.DS_Store"] = true
		candidates[filepath.ToSlash(filepath.Join(filepath.Dir(d), "._"+filepath.Base(d)))] = true
	}
	// A metadata-only directory can have an authenticated Finder record.
	for _, c := range changes {
		if filepath.Base(c.Path) == ".DS_Store" && projectMetadataDirectory(filepath.ToSlash(filepath.Dir(c.Path))) {
			dirs[filepath.ToSlash(filepath.Dir(c.Path))] = true
		}
	}
	collect := func(label string) ([]change, error) {
		var list []change
		inventory := map[string]string{}
		order := []string{}
		for p := range candidates {
			order = append(order, p)
		}
		sort.Strings(order)
		for _, p := range order {
			b, e := optional(root, p)
			if e != nil {
				return nil, e
			}
			if b == nil {
				continue
			}
			if e = validateMetadata(p, b, dirs, func(t string) error {
				if targets[t] {
					return nil
				}
				return fmt.Errorf("unowned companion: %s", t)
			}); e != nil {
				return nil, e
			}
			inventory[p] = digest(b)
			// writeNew verifies regular-file, non-link paths and readback; archive first.
			if e = writeNew(host, "metadata-"+phase+"-"+label+"/"+p, b, 0600); e != nil {
				return nil, fmt.Errorf("metadata archive failed: %s: %w", p, e)
			}
			check, e := read(host, "metadata-"+phase+"-"+label+"/"+p)
			if e != nil || !same(check, b) {
				return nil, fmt.Errorf("metadata archive verification failed: %s", p)
			}
			list = append(list, change{Path: p, Before: b, Mode: 0600})
		}
		if e := writeNew(host, "metadata-"+phase+"-"+label+".json", encode(inventory), 0600); e != nil {
			return nil, e
		}
		return list, nil
	}
	// Preserve both post-restore metadata and any state generated as data is removed.
	if _, e := collect("before"); e != nil {
		return e
	}
	if e := transact(root, data, hook); e != nil {
		return e
	}
	refreshed, e := collect("after")
	if e != nil {
		return e
	}
	// No data publication follows this metadata transaction. A further race still
	// fails strict transaction checks; it is never silently discarded.
	return transact(root, refreshed, hook)
}
func projectMetadataDirectory(p string) bool {
	return p == strings.TrimSuffix(app, "/") || strings.HasPrefix(p, app) || p == strings.TrimSuffix(legacyApp, "/") || strings.HasPrefix(p, legacyApp) || strings.HasPrefix(p, system+"config/better-favorites-")
}
