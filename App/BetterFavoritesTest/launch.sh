#!/bin/sh

APP_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$APP_DIR" || exit 1
LOG="$APP_DIR/better-favorites.log"
ACTIVE=/mnt/SDCARD/.tmp_update/cmd_to_run.sh

{
    echo "=== Better Favorites runtime log ==="
    date
} > "$LOG"

umask 077
REQUEST_DIR="$(mktemp -d /tmp/better-favorites.XXXXXX 2>> "$LOG")"
APP_PID=""
HANDOFF_COMMITTED=0
unset BETTER_FAVORITES_REQUEST_DIR
if [ -n "$REQUEST_DIR" ]; then
    printf 'Private request directory created: %s\n' "$REQUEST_DIR" >> "$LOG"
    if cp "$ACTIVE" "$REQUEST_DIR/app-command.sh" 2>> "$LOG"; then
        echo "Captured active app command in private request directory." >> "$LOG"
    else
        echo "Could not capture active app command." >> "$LOG"
    fi
    export BETTER_FAVORITES_REQUEST_DIR="$REQUEST_DIR"
else
    echo "Could not create private request directory." >> "$LOG"
fi

cleanup() {
    trap - EXIT INT TERM
    printf 'cleanup: committed=%s app_pid=%s\n' \
        "$HANDOFF_COMMITTED" "$APP_PID" >> "$LOG"
    if [ -n "$APP_PID" ]; then
        kill -TERM "$APP_PID" 2>/dev/null || true
        wait "$APP_PID" 2>/dev/null || true
        echo "cleanup: child binary terminated or already exited." >> "$LOG"
    fi
    if [ -n "$REQUEST_DIR" ]; then
        if "$APP_DIR/better-favorites" \
            --cancel-handoff "$REQUEST_DIR" >> "$LOG" 2>&1; then
            echo "cleanup: private request removed." >> "$LOG"
        else
            echo "cleanup: private request removal failed." >> "$LOG"
        fi
        if rmdir "$REQUEST_DIR" 2>> "$LOG"; then
            echo "cleanup: private directory removed." >> "$LOG"
        else
            echo "cleanup: private directory retained." >> "$LOG"
        fi
    fi
}

trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

export LD_LIBRARY_PATH="$APP_DIR:/config/lib:/customer/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
echo "Starting Better Favorites binary." >> "$LOG"
SDL_AUDIODRIVER=dsp \
LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so \
"$APP_DIR/better-favorites" >> "$LOG" 2>&1 &
APP_PID=$!
last_app_pid="$APP_PID"
wait "$APP_PID"
app_exit=$?
APP_PID=""
printf 'binary pid=%s exit status=%s\n' "$last_app_pid" "$app_exit" >> "$LOG"

if [ "$app_exit" -eq 20 ]; then
    if [ -z "$REQUEST_DIR" ]; then
        echo "No private request directory for handoff." >> "$LOG"
        exit 1
    fi
    # The helper publishes only after the binary has finished SDL/audio
    # cleanup. It registers the recent record and sets quick_switch last;
    # ownership checks and rollback handle publication errors.
    trap '' INT TERM
    "$APP_DIR/better-favorites" \
        --publish-handoff "$REQUEST_DIR" >> "$LOG" 2>&1
    publish_status=$?
    if [ "$publish_status" -eq 0 ]; then
        HANDOFF_COMMITTED=1
    fi
    trap 'exit 130' INT
    trap 'exit 143' TERM
    if [ "$HANDOFF_COMMITTED" -eq 1 ]; then
        echo "Published game command, recent record, and quick-switch flag." >> "$LOG"
        exit 0
    fi
    echo "Could not publish game handoff." >> "$LOG"
    exit 1
fi

exit "$app_exit"
