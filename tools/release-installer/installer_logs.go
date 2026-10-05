package main

import (
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"time"
)

func exportInstallerLogs(card string, write func(string, []byte) error, missing map[string]string) error {
	home, e := os.UserHomeDir()
	if e != nil {
		missing["installer-logs"] = "home unavailable"
		return nil
	}
	dir := filepath.Join(home, "BetterFavorites-Logs")
	if e = outsideCard(card, dir); e != nil {
		missing["installer-logs"] = "not outside card"
		return nil
	}
	if _, e = join(home, "BetterFavorites-Logs"); e != nil {
		missing["installer-logs"] = "unsafe path"
		return nil
	}
	entries, e := os.ReadDir(dir)
	if e != nil {
		missing["installer-logs"] = "missing/unreadable"
		return nil
	}
	names := []string{}
	for _, s := range entries {
		n := s.Name()
		if !strings.HasPrefix(n, "installer-") || !strings.HasSuffix(n, ".log") {
			continue
		}
		stamp := strings.TrimSuffix(strings.TrimPrefix(n, "installer-"), ".log")
		if _, e = time.Parse("20060102T150405.000000000Z", stamp); e != nil {
			continue
		}
		names = append(names, n)
	}
	sort.Sort(sort.Reverse(sort.StringSlice(names)))
	if len(names) > 2 {
		names = names[:2]
	}
	for i, n := range names {
		b, e := bounded(dir, n, 128*1024)
		key := fmt.Sprintf("installer/session-%d.log", i+1)
		if e != nil {
			missing[key] = "missing/unreadable/unsafe"
			continue
		}
		if e = write(key, b); e != nil {
			return e
		}
	}
	return nil
}
