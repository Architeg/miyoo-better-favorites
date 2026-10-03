#!/bin/sh

APP_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$APP_DIR" || exit 1
LOG="$APP_DIR/better-favorites.log"
ACTIVE=/mnt/SDCARD/.tmp_update/cmd_to_run.sh

export BETTER_FAVORITES_LOG="$LOG"
export LD_LIBRARY_PATH="$APP_DIR:/config/lib:/customer/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
# Bound current/previous sessions through short-lived binary modes. No logger
# process survives the app or handoff. Logging failures never change decisions.
"$APP_DIR/better-favorites" --rotate-log >/dev/null 2>&1 || true
log() { "$APP_DIR/better-favorites" --log-event "$*" >/dev/null 2>&1 || true; }
log "launcher entry pid=$$ ppid=$PPID"

# Optional lifecycle evidence only; the native command and ownership checks stay exact.
# Markers are independent of both saved Home replacement and Automatic return.
HOME_DIAGNOSTICS=0
marker="$APP_DIR/home-diagnostics.conf"
if [ -f "$marker" ] && [ ! -L "$marker" ] && [ "$(wc -c < "$marker" 2>/dev/null)" -eq 34 ] 2>/dev/null &&
   [ "$(cat "$marker" 2>/dev/null)" = "$(printf 'BetterFavoritesHomeDiagnostics1\n1')" ]; then
    HOME_DIAGNOSTICS=1
fi
home_diagnostic() {
    [ "$HOME_DIAGNOSTICS" -eq 1 ] || return 0
    # Correlation is a candidate only: runtime does not inherit MainUI's environment.
    candidate=none
    hook_log="$APP_DIR/home-diagnostics.log"
    if [ -f "$hook_log" ] && [ ! -L "$hook_log" ]; then
        candidate=$(awk '/event=publication reason=committed/ {for(i=1;i<=NF;i++)if($i~/^attempt=/)last=$i} END {print last}' "$hook_log" 2>/dev/null)
    fi
    log "M6Home1 launcher pid=$$ ppid=$PPID event=$1 candidate_${candidate:-attempt=none}"
}
home_diagnostic entry

umask 077
REQUEST_DIR="$(mktemp -d /tmp/better-favorites.XXXXXX 2>/dev/null)"
APP_PID=""
HANDOFF_COMMITTED=0
unset BETTER_FAVORITES_REQUEST_DIR
if [ -n "$REQUEST_DIR" ]; then
    log "Private request directory created: $REQUEST_DIR"
    if cp "$ACTIVE" "$REQUEST_DIR/app-command.sh" 2>/dev/null; then
        log "Captured active app command in private request directory."
    else
        log "Could not capture active app command."
    fi
    export BETTER_FAVORITES_REQUEST_DIR="$REQUEST_DIR"
else
    log "Could not create private request directory."
fi

cleanup() {
    trap - EXIT INT TERM
    home_diagnostic "exit committed=$HANDOFF_COMMITTED binary_status=${app_exit:-not-returned}"
    log "cleanup: committed=$HANDOFF_COMMITTED app_pid=$APP_PID"
    if [ -n "$APP_PID" ]; then
        kill -TERM "$APP_PID" 2>/dev/null || true
        wait "$APP_PID" 2>/dev/null || true
        log "cleanup: child binary terminated or already exited."
    fi
    if [ -n "$REQUEST_DIR" ]; then
        if "$APP_DIR/better-favorites" \
            --cancel-handoff "$REQUEST_DIR" >/dev/null 2>&1; then
            log "cleanup: private request removed."
        else
            log "cleanup: private request removal failed."
        fi
        if rmdir "$REQUEST_DIR" 2>/dev/null; then
            log "cleanup: private directory removed."
        else
            log "cleanup: private directory retained."
        fi
    fi
}

trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

export BETTER_FAVORITES_SETTINGS="$APP_DIR/settings.conf"
export BETTER_FAVORITES_BROWSER_STATE="$APP_DIR/browser-state"
log "Starting Better Favorites binary."
SDL_AUDIODRIVER=dsp \
LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so \
"$APP_DIR/better-favorites" >/dev/null 2>&1 &
APP_PID=$!
last_app_pid="$APP_PID"
wait "$APP_PID"
app_exit=$?
APP_PID=""
log "binary pid=$last_app_pid exit status=$app_exit"

if [ "$app_exit" -eq 20 ] || [ "$app_exit" -eq 21 ]; then
    if [ -z "$REQUEST_DIR" ]; then
        log "No private request directory for handoff."
        exit 1
    fi
    # Both operations publish only after SDL/audio cleanup. A registers its
    # recent record and sets quick_switch; MENU only requests GameSwitcher.
    # Ownership checks and rollback handle publication errors.
    trap '' INT TERM
    operation=--publish-handoff
    [ "$app_exit" -ne 21 ] || operation=--publish-switcher-handoff
    "$APP_DIR/better-favorites" \
        "$operation" "$REQUEST_DIR" >/dev/null 2>&1
    publish_status=$?
    if [ "$publish_status" -eq 0 ]; then
        HANDOFF_COMMITTED=1
    fi
    trap 'exit 130' INT
    trap 'exit 143' TERM
    if [ "$HANDOFF_COMMITTED" -eq 1 ]; then
        log "Published Onion handoff: $operation."
        exit 0
    fi
    log "Could not publish game handoff."
    exit 1
fi

exit "$app_exit"
