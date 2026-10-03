#!/usr/bin/env python3
"""Exercise unmodified helper logic against isolated paths, never the card."""
from pathlib import Path
import subprocess
import tempfile
import re
import argparse

repo = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--helper', type=Path, default=repo / 'integration/onion-return/better_favorites_return.sh')
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix='better-favorites-runtime-test-') as directory:
    root = Path(directory).resolve()
    card = root / 'card'
    temp = root / 'tmp'
    for folder in (card / '.tmp_update/config', card / '.tmp_update/logs', card / 'App/BetterFavoritesTest', card / 'Roms/GB', temp):
        folder.mkdir(parents=True, exist_ok=True)
    launcher = card / 'App/BetterFavoritesTest/launch.sh'
    launcher.write_text('#!/bin/sh\nexit 0\n')
    launcher.chmod(0o700)
    (card / 'Roms/GB/one.gb').write_text('fixture')
    (card / 'Roms/GB/two.gb').write_text('fixture')
    helper = args.helper.read_text()
    # Substitute only original tokens, never /tmp within an inserted fixture path.
    helper = re.sub(r'/mnt/SDCARD|/tmp/',
                    lambda match: str(card) if match.group() == '/mnt/SDCARD'
                    else str(temp) + '/', helper)
    (root / 'helper.sh').write_text(helper)
    script = r'''
set -eu
sysdir="$CARD/.tmp_update"
log() { printf '%s\n' "$*" >> "$TEST_LOG"; }
. "$HELPER"
# Default tracing writes nothing. Opt-in lifecycle evidence is separate.
bf_return_diag test-default-off
[ ! -e "$sysdir/logs/better-favorites-return.log" ]
printf 'BetterFavoritesHomeDiagnostics1\n1\n' > "$CARD/App/BetterFavoritesTest/home-diagnostics.conf"
# Disabled by default, even when callers provide forged origin state.
bf_origin=forged
bf_context=''
bf_return init
[ -z "$bf_origin" ] && [ -z "${BETTER_FAVORITES_RETURN_DIR:-}" ]
setting="$CARD/App/BetterFavoritesTest/settings.conf"
epoch=0123456789abcdef0123456789abcdef
printf 'BetterFavoritesSettings1\n1\n%s\n' "$epoch" > "$setting"
app=$(bf_return_app_command)
game="LD_PRELOAD=fixture \"$CARD/Emu/GB/launch.sh\" \"$CARD/Roms/GB/one.gb\""
other="$CARD/Roms/GB/two.gb"
rotate_trace_fixture() {
    trace="$sysdir/logs/better-favorites-return.log"
    if [ -f "$trace" ] && [ "$(wc -c < "$trace")" -ge 60000 ]; then
        cat "$trace" >> "$trace.fixture-archive"
        : > "$trace"
    fi
}
adopt() {
    rotate_trace_fixture
    bf_return before-launch "$app"
    [ -n "$BETTER_FAVORITES_RETURN_DIR" ]
    printf '%s' "$game" > "$BETTER_FAVORITES_RETURN_DIR/request.sh"
    printf '%s\n' "$epoch" > "$BETTER_FAVORITES_RETURN_DIR/generation"
    printf '%s' "$game" > "$sysdir/cmd_to_run.sh"
    touch "$TMP/quick_switch"
    bf_return after-app "$app" 0
    [ "$bf_origin" = "$CARD/Roms/GB/one.gb" ]
    [ "$bf_origin_epoch" = "$epoch" ] && [ -z "$bf_context" ]
    [ -z "${bf_setting_record+x}" ] && [ -z "${bf_rom+x}" ]
    [ -z "${BETTER_FAVORITES_RETURN_DIR:-}" ]
    rm "$TMP/quick_switch"
    bf_return resolved-game "$CARD/Roms/GB/one.gb" 1
}
adopt
# A (and MENU resume) leaves a command; keep ownership, queue no app.
bf_return after-switcher 0
[ "$bf_origin" = "$CARD/Roms/GB/one.gb" ]
[ "$(cat "$sysdir/cmd_to_run.sh")" = "$game" ]
# Verify both menu-exit buttons after switching to another game.
for switched_exit in B START; do
    adopt
    # Same-game restart and switching games keep the originating session.
    bf_return resolved-game "$CARD/Roms/GB/one.gb" 1
    [ -n "$bf_origin" ]
    bf_return resolved-game "$other" 1
    [ "$bf_origin" = "$CARD/Roms/GB/one.gb" ]
    other_command="LD_PRELOAD=fixture \"$CARD/Emu/GB/launch.sh\" \"$other\""
    printf '%s' "$other_command" > "$sysdir/cmd_to_run.sh"
    bf_return after-switcher 0
    [ "$bf_origin" = "$CARD/Roms/GB/one.gb" ]
    [ "$(cat "$sysdir/cmd_to_run.sh")" = "$other_command" ]
    rm "$sysdir/cmd_to_run.sh"
    bf_return after-switcher 0
    [ -z "$bf_origin" ] && [ "$(cat "$sysdir/cmd_to_run.sh")" = "$app" ]
done
# A stock-menu game must never acquire a Better Favorites session.
bf_return menu
printf '%s' "$other_command" > "$sysdir/cmd_to_run.sh"
bf_return before-launch "$other_command"
bf_return resolved-game "$other" 1
[ -z "$bf_origin" ]
bf_return after-switcher 0
[ -z "$bf_origin" ] && [ "$(cat "$sysdir/cmd_to_run.sh")" = "$other_command" ]
rm "$sysdir/cmd_to_run.sh"
bf_return after-switcher 0
[ ! -e "$sysdir/cmd_to_run.sh" ]
# Both B and START have stock success + missing-command semantics.
for button in B START; do
    adopt
    rm "$sysdir/cmd_to_run.sh"
    bf_return after-switcher 0
    [ -z "$bf_origin" ]
    [ "$(cat "$sysdir/cmd_to_run.sh")" = "$app" ]
    # Reopened app B: no ticket and no quick switch -> no reopening loop.
    bf_return before-launch "$app"
    bf_return resolved-game '' 0
    bf_return after-app "$app" 0
    rm "$sysdir/cmd_to_run.sh"
    bf_return after-switcher 0
    [ ! -e "$sysdir/cmd_to_run.sh" ]
done
# Disable invalidates an active session at the next runtime boundary.
adopt
printf 'BetterFavoritesSettings1\n0\nffffffffffffffffffffffffffffffff\n' > "$setting"
rm "$sysdir/cmd_to_run.sh"
bf_return after-switcher 0
[ -z "$bf_origin" ] && [ ! -e "$sysdir/cmd_to_run.sh" ]
# Off -> on between boundaries must not resurrect old ownership.
printf 'BetterFavoritesSettings1\n1\n%s\n' "$epoch" > "$setting"
adopt
printf 'BetterFavoritesSettings1\n0\nffffffffffffffffffffffffffffffff\n' > "$setting"
printf 'BetterFavoritesSettings1\n1\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n' > "$setting"
rm "$sysdir/cmd_to_run.sh"
bf_return after-switcher 0
[ -z "$bf_origin" ] && [ ! -e "$sysdir/cmd_to_run.sh" ]
printf 'BetterFavoritesSettings1\n1\n%s\n' "$epoch" > "$setting"
# Missing/invalid settings and stale generation tickets cannot acquire ownership.
for invalid in missing malformed generation; do
    bf_return before-launch "$app"
    context="$BETTER_FAVORITES_RETURN_DIR"
    printf '%s' "$game" > "$context/request.sh"
    printf '%s\n' "$epoch" > "$context/generation"
    printf '%s' "$game" > "$sysdir/cmd_to_run.sh"
    touch "$TMP/quick_switch"
    case "$invalid" in
        missing) rm "$setting" ;;
        malformed) printf bad > "$setting" ;;
        generation) printf 'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n' > "$context/generation" ;;
    esac
    bf_return after-app "$app" 0
    [ -z "$bf_origin" ] && [ ! -e "$context" ]
    rm "$TMP/quick_switch"
    printf 'BetterFavoritesSettings1\n1\n%s\n' "$epoch" > "$setting"
done
# Direct exit, abnormal game exit, MainUI return, and runtime restart clear origin.
for event in direct crash menu restart; do
    adopt
    bf_return resolved-game "$other" 1
    case "$event" in
        direct) bf_return game-ended 0 ;;
        crash) touch "$sysdir/.runGameSwitcher"; bf_return game-ended 139; rm "$sysdir/.runGameSwitcher" ;;
        menu) bf_return menu ;;
        restart) bf_return init ;;
    esac
    [ -z "$bf_origin" ]
done
# Normal keymon termination preserves origin while opening GameSwitcher.
adopt
touch "$sysdir/.runGameSwitcher"
bf_return game-ended 143
[ -n "$bf_origin" ]
rm "$sysdir/.runGameSwitcher"
# Shutdown, unrelated pending request, and failed switcher never reopen.
for event in shutdown pending flag failed; do
    adopt
    rm "$sysdir/cmd_to_run.sh"
    case "$event" in
        shutdown) touch "$TMP/.offOrder" ;;
        pending) printf foreign > "$TMP/cmd_to_run.sh" ;;
        flag) printf foreign > "$TMP/quick_switch" ;;
    esac
    status=0
    [ "$event" != failed ] || status=1
    bf_return after-switcher "$status"
    [ ! -e "$sysdir/cmd_to_run.sh" ] && [ -z "$bf_origin" ]
    case "$event" in
        shutdown) rm "$TMP/.offOrder" ;;
        pending) [ "$(cat "$TMP/cmd_to_run.sh")" = foreign ]; rm "$TMP/cmd_to_run.sh" ;;
        flag) [ "$(cat "$TMP/quick_switch")" = foreign ]; rm "$TMP/quick_switch" ;;
    esac
done
# Failed same-filesystem move consumes origin; no own command/temp remains.
adopt
rm "$sysdir/cmd_to_run.sh"
mv() { return 1; }
bf_return after-switcher 0
unset -f mv
[ -z "$bf_origin" ] && [ ! -e "$sysdir/cmd_to_run.sh" ]
[ "$(find "$sysdir" -name 'cmd_to_run.sh.better-favorites-return.*' | wc -l)" -eq 0 ]
# An active file arriving during publication is preserved by the no-clobber move.
adopt
rm "$sysdir/cmd_to_run.sh"
mv() { printf foreign > "$sysdir/cmd_to_run.sh"; command mv "$@"; }
bf_return after-switcher 0
unset -f mv
[ "$(cat "$sysdir/cmd_to_run.sh")" = foreign ] && [ -z "$bf_origin" ]
# Shutdown arriving during publication cancels only our published command.
adopt
rm "$sysdir/cmd_to_run.sh"
mv() { touch "$TMP/.offOrder"; command mv "$@"; }
bf_return after-switcher 0
unset -f mv
[ ! -e "$sysdir/cmd_to_run.sh" ] && [ -z "$bf_origin" ]
rm "$TMP/.offOrder"
# An invocation ticket must match the actual published command.
bf_return before-launch "$app"
printf stale > "$BETTER_FAVORITES_RETURN_DIR/request.sh"
printf '%s' "$game" > "$sysdir/cmd_to_run.sh"
touch "$TMP/quick_switch"
bf_return after-app "$app" 0
[ -z "$bf_origin" ]
rm "$TMP/quick_switch"
# Restart cannot adopt an old live directory (a new invocation gets a new one).
bf_return before-launch "$app"
stale="$BETTER_FAVORITES_RETURN_DIR"
printf '%s' "$game" > "$stale/request.sh"
bf_return init
bf_return before-launch "$app"
[ "$BETTER_FAVORITES_RETURN_DIR" != "$stale" ]
bf_return after-app "$app" 0
[ -z "$bf_origin" ]
# MENU-origin sessions do not require any recent record or selected ROM.
menu_adopt() {
    rotate_trace_fixture
    bf_return before-launch "$app"
    [ "$BETTER_FAVORITES_SWITCHER_HANDOFF" = 1 ]
    context="$BETTER_FAVORITES_RETURN_DIR"
    printf 'BetterFavoritesSwitcher1\nprivate-request-nonce\n' > "$context/switcher.request"
    cp "$context/switcher.request" "$sysdir/.runGameSwitcher"
    printf '%s\n' "$epoch" > "$context/generation"
    rm -f "$sysdir/cmd_to_run.sh"
    bf_return after-app "$app" 0
    [ "$bf_origin" = BetterFavorites:GameSwitcher ]
    [ -z "${BETTER_FAVORITES_SWITCHER_HANDOFF:-}" ] && [ ! -e "$context" ]
}
for menu_exit in B START EMPTY_A; do
    menu_adopt
    # B/START delete the active command. Empty-history A publishes no command.
    rm "$sysdir/.runGameSwitcher"
    bf_return after-switcher 0
    [ -z "$bf_origin" ] && [ "$(cat "$sysdir/cmd_to_run.sh")" = "$app" ]
    bf_return before-launch "$app"
    bf_return resolved-game '' 0
    bf_return after-app "$app" 0
    rm "$sysdir/cmd_to_run.sh"
    bf_return after-switcher 0
    [ ! -e "$sysdir/cmd_to_run.sh" ]
done
for menu_exit in B START; do
    menu_adopt
    printf '%s' "$game" > "$sysdir/cmd_to_run.sh"
    rm "$sysdir/.runGameSwitcher"
    bf_return after-switcher 0
    bf_return before-launch "$game"
    bf_return resolved-game "$CARD/Roms/GB/one.gb" 1
    [ "$bf_origin" = BetterFavorites:GameSwitcher ]
    printf '%s' "$other_command" > "$sysdir/cmd_to_run.sh"
    bf_return after-switcher 0
    bf_return before-launch "$other_command"
    bf_return resolved-game "$other" 1
    [ "$bf_origin" = BetterFavorites:GameSwitcher ]
    rm "$sysdir/cmd_to_run.sh"
    bf_return after-switcher 0
    [ "$(cat "$sysdir/cmd_to_run.sh")" = "$app" ] && [ -z "$bf_origin" ]
done
for menu_failure in OFF GENERATION FLAG APPFAIL SHUTDOWN; do
    bf_return before-launch "$app"
    context="$BETTER_FAVORITES_RETURN_DIR"
    printf 'BetterFavoritesSwitcher1\nprivate-request-nonce\n' > "$context/switcher.request"
    cp "$context/switcher.request" "$sysdir/.runGameSwitcher"
    printf '%s\n' "$epoch" > "$context/generation"
    rm -f "$sysdir/cmd_to_run.sh"
    status=0
    case "$menu_failure" in
        OFF) printf 'BetterFavoritesSettings1\n0\n%s\n' "$epoch" > "$setting" ;;
        GENERATION) printf foreign > "$context/generation" ;;
        FLAG) printf foreign > "$sysdir/.runGameSwitcher" ;;
        APPFAIL) status=1 ;;
        SHUTDOWN) touch "$TMP/.offOrder" ;;
    esac
    bf_return after-app "$app" "$status"
    [ -z "$bf_origin" ] && [ ! -e "$context" ]
    rm "$sysdir/.runGameSwitcher"
    bf_return after-switcher 0
    [ ! -e "$sysdir/cmd_to_run.sh" ]
    rm -f "$TMP/.offOrder"
    printf 'BetterFavoritesSettings1\n1\n%s\n' "$epoch" > "$setting"
done

# Sourced profiling does not change the exact Apps command or exported context.
rm -f "$sysdir/cmd_to_run.sh"
APP_DIR="$CARD/App/BetterFavoritesTest"
LOG="$APP_DIR/profile-fixture.log"
printf fixture > "$LOG"
printf 'BetterFavoritesProfilePilot1\n' > "$APP_DIR/profile.enabled"
printf '{}\n' > "$CARD/Roms/favourite.json"
bf_return before-launch "$app"
profile_context="$BETTER_FAVORITES_RETURN_DIR"
. "$PROFILE_HOOK"
BETTER_FAVORITES_SD_ROOT="$CARD"
bf_profile_begin
[ "$BETTER_FAVORITES_RETURN_DIR" = "$profile_context" ]
[ "$BETTER_FAVORITES_PROFILE" = 1 ] && bf_return_is_app "$app"
printf '%s' "$game" > "$profile_context/request.sh"
printf '%s\n' "$epoch" > "$profile_context/generation"
printf '%s' "$game" > "$sysdir/cmd_to_run.sh"
touch "$TMP/quick_switch"
bf_profile_finish 0
bf_return after-app "$app" 0
[ "$bf_origin" = "$CARD/Roms/GB/one.gb" ] && [ "$bf_origin_epoch" = "$epoch" ]
rm "$TMP/quick_switch" "$sysdir/cmd_to_run.sh"
bf_return after-switcher 0
[ -z "$bf_origin" ] && [ "$(cat "$sysdir/cmd_to_run.sh")" = "$app" ]

# A log write failure cannot prevent ownership adoption or reopening.
if command -v bf_return_diag >/dev/null; then
    diagnostic="$sysdir/logs/better-favorites-return.log"
    mv "$diagnostic" "$diagnostic.saved"
    mkdir "$diagnostic"
    adopt
    rm "$sysdir/cmd_to_run.sh"
    bf_return after-switcher 0
    [ -z "$bf_origin" ] && [ "$(cat "$sysdir/cmd_to_run.sh")" = "$app" ]
    rmdir "$diagnostic"
    mv "$diagnostic.saved" "$diagnostic"
    cp "$diagnostic" "$diagnostic.evidence"
    dd if=/dev/zero of="$diagnostic" bs=1024 count=128 2>/dev/null
    bf_return_diag cap-test
    [ "$(wc -c < "$diagnostic")" -eq 131072 ]
    mv "$diagnostic.evidence" "$diagnostic"
    rm "$CARD/App/BetterFavoritesTest/home-diagnostics.conf"
    bf_return_diag disabled-test
    ! grep -q disabled-test "$diagnostic"
fi
'''
    env = dict(__import__('os').environ, CARD=str(card), TMP=str(temp), HELPER=str(root / 'helper.sh'), TEST_LOG=str(root / 'log'), PROFILE_HOOK=str(repo/'tools/profile-device-launch.sh'))
    result = subprocess.run(['sh', '-xc', script], env=env, capture_output=True, text=True)
    if result.returncode:
        print(result.stderr[-7000:])
        raise SystemExit(result.returncode)
    if 'bf_return_diag()' in helper:
        records = (card / '.tmp_update/logs/better-favorites-return.log').read_text()
        archive=card/'.tmp_update/logs/better-favorites-return.log.fixture-archive'
        if archive.exists(): records+=archive.read_text()
        for token in ('action=adopt reason=verified-ticket', 'action=adopt reason=verified-menu-ticket', 'reason=generation-mismatch',
                      'reason=settings-disabled', 'reason=game-within-session',
                      'reason=game-exit-status exit_status=139', 'reason=ordinary-menu-return',
                      'reason=runtime-init', 'reason=switcher-exit-status', 'reason=shutdown-request',
                      'reason=pending-command-or-quick-switch', 'reason=active-command-after-switcher',
                      'action=reopen', 'ticket_generation=', 'consumed_generation=',
                      'resolved_rom=[' + str(card / 'Roms/GB/two.gb') + ']'):
            assert token in records, token
print('Runtime ownership, B/START return, resume/switch/direct-exit/restart, shutdown, stale tickets and failures: PASS')
