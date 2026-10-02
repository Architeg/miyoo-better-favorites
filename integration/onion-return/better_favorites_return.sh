# Sourced by the separately patched Onion v4.3.1-1 runtime.
# No game execution, history handling, or input interception lives here.
bf_return_diag() {
    # Boundary-only, independent of Onion's global logging switch. Best effort:
    # never make logging failure an execution/ownership decision.
    printf '%s\n' "BF_RETURN pid=$$ event=${bf_event:-init} origin=[${bf_origin:-}] owned_generation=[${bf_origin_epoch:-}] settings_generation=[${bf_setting_epoch:-}] $*" 2>/dev/null >> "$sysdir/logs/better-favorites-return.log" || :
}

bf_return_path_state() {
    if [ -L "$1" ]; then printf symlink
    elif [ -f "$1" ]; then printf file
    elif [ -e "$1" ]; then printf other
    else printf absent; fi
}

bf_return_clear_origin() {
    bf_return_diag "action=clear reason=$1"
    bf_origin=""
    bf_origin_epoch=""
}

bf_return_read_settings() {
    bf_enabled=0
    bf_setting_reason=missing-or-nonregular
    bf_setting_epoch=""
    bf_setting_file=/mnt/SDCARD/App/BetterFavoritesTest/settings.conf
    if [ -L "$bf_setting_file" ] || [ ! -f "$bf_setting_file" ]; then return 0; fi
    bf_setting_reason=oversized-or-unreadable
    [ "$(wc -c < "$bf_setting_file")" -le 128 ] || return 0
    # One snapshot: atomic app settings replacement cannot mix old/new fields.
    bf_setting_record=$(cat "$bf_setting_file") || return 0
    bf_setting_reason=invalid-generation
    bf_setting_epoch=$(printf '%s\n' "$bf_setting_record" | sed -n '3p')
    case "$bf_setting_epoch" in *[!0-9a-f]*) bf_setting_epoch=""; return 0 ;; esac
    [ "${#bf_setting_epoch}" -eq 32 ] || return 0
    bf_setting_reason=malformed-record
    [ "$bf_setting_record" = "$(printf 'BetterFavoritesSettings1\n1\n%s' "$bf_setting_epoch")" ] || {
        if [ "$bf_setting_record" = "$(printf 'BetterFavoritesSettings1\n0\n%s' "$bf_setting_epoch")" ]; then
            bf_setting_reason=disabled
        fi
        return 0
    }
    bf_enabled=1
    bf_setting_reason=enabled
}

bf_return_finish() {
    # Only origin, its generation, and the app invocation context persist.
    unset bf_owned bf_owned_epoch bf_candidate bf_rom bf_temporary bf_inode bf_current_inode
    unset bf_enabled bf_setting_epoch bf_setting_file bf_setting_record bf_setting_reason bf_event bf_ticket_generation bf_decision_reason bf_command_match
}

bf_return_clear_context() {
    unset BETTER_FAVORITES_RETURN_DIR
    if [ -n "$bf_context" ]; then
        rm -f "$bf_context/request.sh" "$bf_context/generation"
        rmdir "$bf_context" 2>/dev/null || true
    fi
    bf_context=""
}

bf_return_app_command() {
    printf '%s' 'cd /mnt/SDCARD/App/BetterFavoritesTest; chmod a+x ./launch.sh; LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so   ./launch.sh'
}

bf_return_is_app() {
    bf_candidate=$(printf '%s' "$1" | sed 's/[[:space:]]*$//')
    [ "$bf_candidate" = "$(bf_return_app_command)" ]
}

bf_return() {
    bf_event=$1
    # Runtime restart never adopts an old context, even when /tmp survives.
    if [ "$1" = init ]; then
        bf_return_clear_origin runtime-init
        bf_context=""
        unset BETTER_FAVORITES_RETURN_DIR
    fi
    bf_return_read_settings
    bf_return_diag "action=boundary enabled=$bf_enabled settings_reason=$bf_setting_reason arg2=[${2:-}] arg3=[${3:-}] active=$(bf_return_path_state "$sysdir/cmd_to_run.sh") pending=$(bf_return_path_state /tmp/cmd_to_run.sh) quick_switch=$(bf_return_path_state /tmp/quick_switch) switcher_flag=$(bf_return_path_state "$sysdir/.runGameSwitcher") shutdown=$(bf_return_path_state /tmp/.offOrder)"
    if [ "$bf_enabled" != 1 ] || [ "${bf_origin_epoch:-}" != "$bf_setting_epoch" ]; then
        if [ "$bf_enabled" != 1 ]; then
            bf_return_clear_origin "settings-$bf_setting_reason"
        else
            bf_return_clear_origin generation-mismatch
        fi
    fi
    case "$1" in
        init) ;;
        before-launch)
            bf_return_clear_context
            if bf_return_is_app "$2"; then
                bf_return_clear_origin new-app-invocation
                bf_context=$(mktemp -d /tmp/better-favorites-return.XXXXXX) || bf_context=""
                if [ -n "$bf_context" ]; then
                    export BETTER_FAVORITES_RETURN_DIR="$bf_context"
                    bf_return_diag "action=context-created context=[$bf_context]"
                else
                    bf_return_diag 'action=context-failed reason=private-directory-creation-failed'
                    log 'Better Favorites return: cannot create invocation context'
                fi
            fi
            ;;
        resolved-game)
            # Origin names the Better Favorites session, not the current game.
            # GameSwitcher may replace the game without changing the origin.
            if [ "$3" != 1 ]; then
                bf_return_clear_origin "resolved-nongame is_game=$3 resolved_rom=[$2]"
            else
                bf_return_diag "action=retain reason=game-within-session resolved_rom=[$2]"
            fi
            ;;
        after-app)
            bf_return_clear_origin after-app-reset
            bf_ticket_generation=""
            if [ -n "$bf_context" ] && [ ! -L "$bf_context/generation" ] &&
               [ -f "$bf_context/generation" ]; then
                bf_ticket_generation=$(cat "$bf_context/generation" 2>/dev/null) || bf_ticket_generation=""
            fi
            # Diagnostic reads must not block on a FIFO or follow a foreign link.
            bf_command_match=uncheckable
            if [ ! -L "$bf_context/request.sh" ] && [ -f "$bf_context/request.sh" ] &&
               [ ! -L "$sysdir/cmd_to_run.sh" ] && [ -f "$sysdir/cmd_to_run.sh" ]; then
                bf_command_match=$(cmp -s "$bf_context/request.sh" "$sysdir/cmd_to_run.sh" && printf yes || printf no)
            fi
            bf_return_diag "action=adoption-check exit_status=$3 context=[$bf_context] ticket_generation=[$bf_ticket_generation] request=$(bf_return_path_state "$bf_context/request.sh") generation_file=$(bf_return_path_state "$bf_context/generation") app_match=$(bf_return_is_app "$2" && printf yes || printf no) command_match=$bf_command_match"
            if [ "$bf_enabled" = 1 ] && [ "$3" = 0 ] && bf_return_is_app "$2" &&
               [ -n "$bf_context" ] && [ -f /tmp/quick_switch ] &&
               [ ! -L "$bf_context/request.sh" ] && [ ! -L "$bf_context/generation" ] &&
               [ "$(cat "$bf_context/generation" 2>/dev/null)" = "$bf_setting_epoch" ] &&
               cmp -s "$bf_context/request.sh" "$sysdir/cmd_to_run.sh"; then
                bf_rom=$(awk '{ st=index($0,"\" \""); print substr($0,st+3,length($0)-st-3)}' "$bf_context/request.sh")
                bf_rom=$(realpath "$bf_rom" 2>/dev/null) || bf_rom=""
                case "$bf_rom" in
                    /mnt/SDCARD/Roms/*)
                        if [ -f "$bf_rom" ]; then
                            bf_origin="$bf_rom"
                            bf_origin_epoch="$bf_setting_epoch"
                            bf_return_diag "action=adopt reason=verified-ticket exit_status=$3 resolved_rom=[$bf_rom]"
                        else
                            bf_return_diag "action=reject reason=resolved-rom-not-file resolved_rom=[$bf_rom]"
                        fi
                        ;;
                    *) bf_return_diag "action=reject reason=resolved-rom-outside-root-or-realpath-failed resolved_rom=[$bf_rom]" ;;
                esac
            else
                bf_return_diag "action=reject reason=adoption-preconditions-failed exit_status=$3 enabled=$bf_enabled ticket_generation=[$bf_ticket_generation]"
            fi
            bf_return_clear_context
            ;;
        game-ended)
            # Direct exit/failure returns normally to MainUI. A normal TERM
            # from keymon (143) is used for save-and-open-GameSwitcher.
            if { [ "$2" != 0 ] && [ "$2" != 143 ]; } ||
               { [ ! -f "$sysdir/.runGameSwitcher" ] && [ ! -f /tmp/quick_switch ]; }; then
                if [ "$2" != 0 ] && [ "$2" != 143 ]; then
                    bf_return_clear_origin "game-exit-status exit_status=$2"
                else
                    bf_return_clear_origin "game-ended-without-switcher-or-quick-switch exit_status=$2"
                fi
            else
                bf_return_diag "action=retain reason=game-ended-with-switcher-or-quick-switch exit_status=$2"
            fi
            ;;
        menu)
            bf_return_clear_origin ordinary-menu-return
            bf_return_clear_context
            ;;
        after-switcher)
            bf_owned="$bf_origin"
            bf_owned_epoch="$bf_origin_epoch"
            # Consume before publication, including every failure path.
            bf_return_clear_origin "consume-before-switcher-decision exit_status=$2"
            if [ -z "$bf_owned" ] || [ "$2" != 0 ] ||
               [ -f /tmp/.offOrder ]; then
                if [ -z "$bf_owned" ]; then bf_decision_reason=no-owner
                elif [ "$2" != 0 ]; then bf_decision_reason=switcher-exit-status
                else bf_decision_reason=shutdown-request; fi
                bf_return_diag "action=skip reason=$bf_decision_reason exit_status=$2 consumed_rom=[$bf_owned] consumed_generation=[$bf_owned_epoch] shutdown_file=$([ -f /tmp/.offOrder ] && printf yes || printf no)"
                bf_return_finish; return 0
            fi
            if [ -e "$sysdir/cmd_to_run.sh" ] || [ -L "$sysdir/cmd_to_run.sh" ]; then
                # A/MENU resumes or switches games within the same session.
                bf_origin="$bf_owned"
                bf_origin_epoch="$bf_owned_epoch"
                bf_return_diag "action=retain reason=active-command-after-switcher exit_status=$2"
                bf_return_finish; return 0
            fi
            if [ -e /tmp/cmd_to_run.sh ] || [ -L /tmp/cmd_to_run.sh ] ||
               [ -e /tmp/quick_switch ] || [ -L /tmp/quick_switch ]; then
                bf_return_diag "action=skip reason=pending-command-or-quick-switch pending=$(bf_return_path_state /tmp/cmd_to_run.sh) quick_switch=$(bf_return_path_state /tmp/quick_switch)"
                log 'Better Favorites return: unrelated pending command/flag; return skipped'
                bf_return_finish; return 0
            fi
            if [ ! -x /mnt/SDCARD/App/BetterFavoritesTest/launch.sh ]; then
                bf_return_diag 'action=skip reason=launcher-unavailable'
                log 'Better Favorites return: launcher unavailable'
                bf_return_finish; return 0
            fi
            bf_temporary=$(mktemp "$sysdir/cmd_to_run.sh.better-favorites-return.XXXXXX") || { bf_return_diag 'action=skip reason=temporary-file-creation-failed'; bf_return_finish; return 0; }
            bf_inode=$(ls -i "$bf_temporary" | awk '{print $1}')
            if bf_return_app_command > "$bf_temporary" && chmod 700 "$bf_temporary" &&
               [ ! -f /tmp/.offOrder ]; then
                # Same-filesystem rename. Runtime is the serialized command
                # owner here. -i refuses an already-present foreign command;
                # it is not an atomic compare-and-rename against other writers.
                printf 'n\n' | mv -i "$bf_temporary" "$sysdir/cmd_to_run.sh" || true
            fi
            if [ -e "$bf_temporary" ]; then
                rm -f "$bf_temporary"
                bf_return_diag "action=skip reason=publication-failed-or-shutdown-before-move active=$(bf_return_path_state "$sysdir/cmd_to_run.sh") shutdown=$(bf_return_path_state /tmp/.offOrder)"
                log 'Better Favorites return: publication failed; ordinary menu return'
            elif [ -f /tmp/.offOrder ]; then
                # A shutdown arriving during the move still wins. Remove only
                # our inode with our exact command, never a foreign replacement.
                bf_current_inode=$(ls -i "$sysdir/cmd_to_run.sh" 2>/dev/null | awk '{print $1}')
                if [ -n "$bf_inode" ] && [ "$bf_inode" = "$bf_current_inode" ] &&
                   [ "$(cat "$sysdir/cmd_to_run.sh")" = "$(bf_return_app_command)" ]; then
                    rm -f "$sysdir/cmd_to_run.sh"
                fi
                bf_return_diag 'action=skip reason=shutdown-after-move'
                log 'Better Favorites return: shutdown cancelled reopening'
            else
                bf_return_diag "action=reopen reason=owned-session-no-active-command consumed_rom=[$bf_owned] consumed_generation=[$bf_owned_epoch]"
                log 'Better Favorites return: reopening app'
            fi
            ;;
    esac
    bf_return_finish; return 0
}
