# Sourced only by a generated diagnostic launcher; Apps command remains unchanged.
# No wrapper, sampling loop or extra process retained during gameplay.
bf_profile_begin() {
    [ ! -L "$APP_DIR/profile.enabled" ] && [ -f "$APP_DIR/profile.enabled" ] || return 0
    bf_profile_marker=$(sed -n '1p' "$APP_DIR/profile.enabled")
    bf_profile_session=pilot
    bf_profile_root="$APP_DIR/.profiling-results"
    bf_profile_prefix=pilot
    case "$bf_profile_marker" in
        BetterFavoritesProfilePilot1) ;;
        BetterFavoritesProfileSession2)
            bf_profile_session=$(sed -n '2p' "$APP_DIR/profile.enabled")
            case "$bf_profile_session" in ''|*[!a-zA-Z0-9_-]*) return 1;; esac
            bf_profile_root="$APP_DIR/.profiling-results/session-$bf_profile_session"
            bf_profile_prefix=run
            ;;
        *) return 0;;
    esac
    bf_profile_proc=${BETTER_FAVORITES_PROC_ROOT:-/proc}
    bf_profile_card=${BETTER_FAVORITES_SD_ROOT:-/mnt/SDCARD}
    bf_profile_output=""
    mkdir -p "$bf_profile_root" || return 1
    for bf_profile_run in 1 2 3 4; do
        bf_profile_candidate="$bf_profile_root/$bf_profile_prefix-000$bf_profile_run"
        if mkdir "$bf_profile_candidate" 2>/dev/null; then
            bf_profile_output="$bf_profile_candidate";break
        fi
    done
    [ -n "$bf_profile_output" ] || return 0 # Four-run pilot only; no new full run set.
    {
        printf 'session_id=%s\n' "$bf_profile_session"
        printf 'pilot_run=%s\n' "$bf_profile_run"
        printf 'intended_condition='; [ "$bf_profile_run" -ne 1 ] && printf 'warm\n' || printf 'first-post-boot\n'
        printf 'condition_verified=no; user must confirm boot and launch conditions\n'
        printf 'kernel='; uname -a
        printf 'theme='; cat "$bf_profile_card/.tmp_update/config/active_theme" 2>/dev/null || true
        printf '\nfavorites_bytes='; wc -c < "$bf_profile_card/Roms/favourite.json"
        printf 'automatic_return='; sed -n '2p' "$APP_DIR/settings.conf" 2>/dev/null || true
        printf 'launcher_pid=%s\n' "$$"
    } > "$bf_profile_output/metadata.txt" || return 1
    cat "$bf_profile_proc/$$/stat" > "$bf_profile_output/launcher-stat.txt" 2>/dev/null || true
    if read bf_profile_uptime bf_profile_idle < "$bf_profile_proc/uptime"; then
        printf 'launch_uptime_seconds=%s\n' "$bf_profile_uptime" >> "$bf_profile_output/metadata.txt"
    fi
    export BETTER_FAVORITES_PROFILE=1
    printf 'Profiling pilot run %s: %s\n' "$bf_profile_run" "$bf_profile_output" >> "$LOG"
}
bf_profile_finish() {
    [ -n "${bf_profile_output:-}" ] || return 0
    printf 'launcher_exit=%s\n' "$1" >> "$bf_profile_output/metadata.txt" || true
    # Hash only after the binary and launcher cleanup return. Pre-launch hashing
    # warms the executable/corpus and biases even a first post-boot measurement.
    # These identify post-run files, not an immutable pre-run snapshot.
    {
        printf 'hash_timing=after-binary-exit-and-launcher-cleanup\n'
        for bf_profile_file in "$APP_DIR/better-favorites" "$bf_profile_card/Roms/favourite.json" "$APP_DIR/browser-preferences.conf"; do
            if command -v sha256sum >/dev/null 2>&1; then sha256sum "$bf_profile_file" 2>/dev/null || true
            else printf 'sha256 unavailable: %s\n' "$bf_profile_file"; fi
        done
    } >> "$bf_profile_output/metadata.txt" || true
    cp "$LOG" "$bf_profile_output/startup.log" || {
        printf 'Profile log archive failed: %s\n' "$bf_profile_output" >&2;return 0;
    }
}
