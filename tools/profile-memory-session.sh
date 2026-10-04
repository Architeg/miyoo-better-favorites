#!/bin/sh
# Finite observer for a non-Wi-Fi device. Run separately from startup timing.
# Start from Terminal with nohup; exits after three snapshots, no service/daemon.
set -u
app=${BETTER_FAVORITES_APP_DIR:-/mnt/SDCARD/App/BetterFavorites}
proc=${BETTER_FAVORITES_PROC_ROOT:-/proc}
session=${1:?Fresh session ID required}
condition=${2:?idle, exercised, game-off or game-on required}
case "$session" in ''|*[!a-zA-Z0-9_-]*) exit 1;; esac
case "$condition" in idle|exercised) target_kind=browser;; game-off|game-on) target_kind=game;; *) exit 1;; esac
wait_seconds=${BETTER_FAVORITES_SAMPLE_DELAY:-40}
case "$wait_seconds" in ''|*[!0-9]*) exit 1;; esac
[ "$wait_seconds" -le 300 ] || exit 1
# Move observer outside the app cwd so app-context checks don't count this shell.
cd / || exit 1
umask 077
root="$app/.profiling-memory/$session-$condition"
mkdir -p "$app/.profiling-memory" || exit 1
mkdir "$root" || exit 1 # Never reuse or remove an earlier sample.
printf 'condition=%s\ndelay_seconds=%s\nobserver_pid=%s\n' "$condition" "$wait_seconds" "$$" > "$root/conditions.txt"
printf 'Delayed collector started. Open the requested screen/game within %s seconds.\n' "$wait_seconds"
printf 'automatic_return=' >> "$root/conditions.txt"
sed -n '2p' "$app/settings.conf" >> "$root/conditions.txt" 2>/dev/null || true
uname -a > "$root/kernel.txt"
sleep "$wait_seconds"
failed=0
for sample in 1 2 3; do
    # Snapshot identities, app cwd and executable, without reading environments.
    inventory="$root/inventory-$sample.txt"
    printf 'PID PPID COMM EXE CWD\n' > "$inventory"
    runtime_count=0;target_count=0;runtime_pid=;target_pid=
    for entry in "$proc"/[0-9]*; do
        [ -r "$entry/comm" ] && [ -r "$entry/status" ] || continue
        comm=$(cat "$entry/comm" 2>/dev/null) || continue
        exe=$(readlink "$entry/exe" 2>/dev/null || true)
        cwd=$(readlink "$entry/cwd" 2>/dev/null || true)
        parent=$(awk '/^PPid:/ {print $2}' "$entry/status" 2>/dev/null)
        printf '%s %s %s %s %s\n' "${entry##*/}" "$parent" "$comm" "$exe" "$cwd" >> "$inventory"
        if [ "$comm" = runtime.sh ]; then runtime_count=$((runtime_count+1));runtime_pid=${entry##*/};fi
        if { [ "$target_kind" = browser ] && [ "$exe" = "$app/better-favorites" ]; } ||
           { [ "$target_kind" = game ] && [ "$exe" = /mnt/SDCARD/RetroArch/retroarch ]; }; then
            target_count=$((target_count+1));target_pid=${entry##*/}
        fi
    done
    # All candidates recorded. Refuse ambiguous/missing targets; never pick first.
    printf 'sample=%s\nruntime_candidates=%s\ntarget_candidates=%s\n' "$sample" "$runtime_count" "$target_count" >> "$root/conditions.txt"
    if [ "$runtime_count" -eq 1 ] && [ "$target_count" -eq 1 ]; then
        ancestor=$target_pid;depth=0;matched=0
        while [ "$depth" -lt 32 ] && [ -r "$proc/$ancestor/status" ]; do
            [ "$ancestor" != "$runtime_pid" ] || { matched=1;break; }
            ancestor=$(awk '/^PPid:/ {print $2}' "$proc/$ancestor/status")
            case "$ancestor" in ''|*[!0-9]*|0) break;; esac
            depth=$((depth+1))
        done
        if [ "$matched" -ne 1 ]; then
            printf 'Target is not a child/descendant of identified runtime; samples refused.\n' >> "$root/errors.txt"
            failed=1;runtime_count=0;target_count=0
        fi
    fi
    if [ "$runtime_count" -eq 1 ]; then
        sh "$app/sample-device-memory.sh" "$runtime_pid" "$root/runtime-$sample" || failed=1
    else
        printf 'Runtime identity unresolved; inspect inventory.\n' >> "$root/errors.txt";failed=1
    fi
    if [ "$target_count" -eq 1 ]; then
        sh "$app/sample-device-memory.sh" "$target_pid" "$root/$target_kind-$sample" || failed=1
    else
        printf 'Target identity unresolved; inspect inventory.\n' >> "$root/errors.txt";failed=1
    fi
    [ "$sample" -eq 3 ] || sleep 2
done
printf 'Collector complete: %s (failed=%s)\n' "$root" "$failed"
exit "$failed"
