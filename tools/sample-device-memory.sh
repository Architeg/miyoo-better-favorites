#!/bin/sh
# One short-lived snapshot. Preserve evidence; identity failures invalidate comparisons.
set -u
target=${1:?Verified target PID required}
output=${2:?New absolute sample directory required}
case "$target" in ''|*[!0-9]*) echo 'Numeric target PID required.' >&2; exit 1;; esac
case "$output" in /*) ;; *) echo 'Absolute output path required.' >&2; exit 1;; esac
proc=${BETTER_FAVORITES_PROC_ROOT:-/proc}
umask 077
mkdir "$output" || exit 1
failed=0
capture() { cat "$1" > "$2" 2>> "$output/errors.txt" || failed=1; }
starttime() {
    # comm may contain spaces or parentheses; field 22 is field 20 after final ') '.
    awk '{sub(/^.*\) /, ""); if (NF>=20 && $20 ~ /^[0-9]+$/) print $20}' "$1"
}
state() { awk '{sub(/^.*\) /, ""); if (NF>=20) print $1}' "$1"; }
finish() {
    trap - EXIT INT TERM
    capture "$proc/$target/stat" "$output/process-stat-after.txt"
    before=$(starttime "$output/process-stat-before.txt")
    after=$(starttime "$output/process-stat-after.txt")
    reason=valid
    if [ -z "$before" ]; then reason=missing-or-malformed-before
    elif [ -z "$after" ]; then reason=process-exited-or-unreadable-after
    elif [ "$before" != "$after" ]; then reason=pid-identity-changed
    elif [ "$(state "$output/process-stat-before.txt")" = Z ] || [ "$(state "$output/process-stat-before.txt")" = X ] || [ "$(state "$output/process-stat-after.txt")" = Z ] || [ "$(state "$output/process-stat-after.txt")" = X ]; then reason=process-exited
    elif [ "$failed" -ne 0 ]; then reason=collection-incomplete
    fi
    valid=0; [ "$reason" != valid ] || valid=1
    printf 'valid=%s\nreason=%s\npid=%s\nstarttime_before=%s\nstarttime_after=%s\n' "$valid" "$reason" "$target" "$before" "$after" > "$output/validity.txt"
    printf 'Sample saved: %s (%s)\n' "$output" "$reason"
    [ "$valid" -eq 1 ]
}
# Capture the identity before any metric collection; finalization runs on signals too.
capture "$proc/$target/stat" "$output/process-stat-before.txt"
trap 'failed=1; finish; exit 1' INT TERM
trap 'finish; exit $?' EXIT
capture "$proc/uptime" "$output/uptime.txt"
capture "$proc/meminfo" "$output/meminfo.txt"
capture "$proc/$target/status" "$output/process-status.txt"
if [ -r "$proc/$target/smaps_rollup" ]; then
    capture "$proc/$target/smaps_rollup" "$output/process-smaps.txt"
    printf 'smaps_rollup\n' > "$output/memory-source.txt"
elif [ -r "$proc/$target/smaps" ]; then
    capture "$proc/$target/smaps" "$output/process-smaps.txt"
    printf 'smaps\n' > "$output/memory-source.txt"
else
    printf 'PSS/private/shared unavailable; status RSS only\n' > "$output/memory-source.txt"
fi
if [ -f "$output/process-smaps.txt" ]; then
    awk '/^(Rss|Pss|Private_Clean|Private_Dirty|Shared_Clean|Shared_Dirty):/ {sum[$1]+=$2} END {for (key in sum) print key, sum[key], "kB"}' "$output/process-smaps.txt" > "$output/memory-totals.txt" || failed=1
fi
printf 'PID PPID COMM EXE\n' > "$output/processes.txt"
for entry in "$proc"/[0-9]*; do
    [ -r "$entry/comm" ] && [ -r "$entry/status" ] || continue
    name=$(cat "$entry/comm" 2>/dev/null) || continue
    parent=$(awk '/^PPid:/ {print $2}' "$entry/status" 2>/dev/null) || continue
    executable=$(readlink "$entry/exe" 2>/dev/null || true)
    printf '%s %s %s %s\n' "${entry##*/}" "$parent" "$name" "$executable" >> "$output/processes.txt"
done
