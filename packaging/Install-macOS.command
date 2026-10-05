#!/bin/sh
# Read-only host checks precede installer execution and all SD writes.
set -eu
fail() {
 printf 'Cannot identify this Mac: %s\n' "$*" >&2
 printf 'Detection: macOS=%s; machine=%s; arm64=%s; translated=%s\n' "${version:-unknown}" "${machine:-unknown}" "${arm:-unknown}" "${translated:-unknown}" >&2
 exit 2
}
cd "$(dirname "$0")" || exit 2
[ "$(/usr/bin/uname -s)" = Darwin ] || fail "requires macOS"
version=$(/usr/bin/sw_vers -productVersion) || fail "version unavailable"
case "$version" in ''|*[!0-9.]*|.*|*..*|*.) fail "invalid version $version";; esac
major=${version%%.*}
[ "$major" -ge 12 ] 2>/dev/null || fail "Monterey 12 or later required"
machine=$(/usr/bin/uname -m) || fail "architecture unavailable"
arm=$(/usr/sbin/sysctl -n hw.optional.arm64 2>/dev/null) || arm=absent
translated=$(/usr/sbin/sysctl -n sysctl.proc_translated 2>/dev/null) || translated=0
# uname identifies the running ABI; a positive translation flag establishes
# Apple Silicon under Rosetta. Missing optional Intel sysctl keys are normal.
case "$arm:$translated" in
 0:0|1:0|absent:0|0:1|1:1|absent:1) :;;
 *) fail "invalid optional architecture evidence";;
esac
case "$machine:$translated" in
 arm64:0) [ "$arm" != 0 ] || fail "conflicting native ARM evidence"; arch=arm64;;
 x86_64:1) arch=arm64;;
 x86_64:0) [ "$arm" != 1 ] || fail "Apple Silicon reported without a translation flag"; arch=amd64;;
 *) fail "unsupported or conflicting architecture evidence";;
esac
tool="./better-favorites-installer-darwin-$arch"
if [ -d computer ]; then
 source=$(pwd -L)
 tool="computer/${tool#./}"
 [ -f "$tool" ] && [ ! -L "$tool" ] || fail "missing/unsafe packaged executable $tool"
 # A stable per-byte-identity path retains file-specific approval across actions.
 # Never erase quarantine or replace an existing cached file with different bytes.
 hash=$(/usr/bin/shasum -a 256 "$tool") || exit 2
 hash=${hash%% *}
 case "$hash" in *[!0-9a-f]*|'') fail "invalid executable checksum";; esac
 cache="$HOME/Library/Caches/BetterFavorites"
 for parent in "$HOME/Library" "$HOME/Library/Caches" "$cache" "$cache/$hash"; do
  [ ! -L "$parent" ] || fail "unsafe approval cache $parent"
  if [ ! -d "$parent" ]; then mkdir -m 700 "$parent" || exit 2; fi
  [ -O "$parent" ] || fail "approval cache is not owned by this user"
 done
 stage="$cache/$hash"
 approved="$stage/BetterFavorites-Installer"
 if [ ! -e "$approved" ]; then
  temporary=$(mktemp "$stage/.copy.XXXXXX") || exit 2
  trap 'rm -f "$temporary"' EXIT
  cp -p "$tool" "$temporary" && chmod 700 "$temporary" && mv -n "$temporary" "$approved" || exit 2
  rm -f "$temporary"
 fi
 verify() {
  [ -f "$approved" ] && [ ! -L "$approved" ] && [ -O "$approved" ] || fail "unsafe cached installer"
  actual=$(/usr/bin/shasum -a 256 "$approved") || exit 2
  [ "${actual%% *}" = "$hash" ] || fail "cached installer changed; preserved for inspection"
 }
 approval_actions() {
  printf '\nThe verified installer is retained.\n\n[R] Retry\n[S] Open Settings\n[0] Close\n\nChoose: '
 }
 while :; do
  verify
  set +e
  "$approved" --card-launcher "$source" "$@"
  result=$?
  set -e
  [ "$result" -ne 0 ] || break
  case "$result" in
   137)
    printf '\nInstaller could not start\n\n'
    printf 'If macOS showed an unsigned-developer approval warning:\n'
    if [ "$major" -eq 12 ]; then
     printf '  1. Open System Preferences > Security & Privacy > General.\n'
    else
     printf '  1. Open System Settings > Privacy & Security.\n'
    fi
    printf '  2. Choose Open Anyway for BetterFavorites-Installer.\n'
    printf '  3. Return here and choose Retry.\n\n'
    printf 'Stop for malware, damaged-file or managed-policy warnings.\n'
    printf '\nDetails\nStatus: 137 (possible SIGKILL; does not establish a security block).\n'
    printf 'The helper may have started before termination. Card state is unknown.\n'
    printf 'Verified helper: %s\n' "$approved"
    printf 'Installer logs, if started: ~/BetterFavorites-Logs/\n'
    approval_actions
    ;;
   *)
    case "${1:-menu}" in install) operation=Installation;; uninstall) operation=Uninstall;; *) operation=Operation;; esac
    printf '\n%s failed (status %s).\n' "$operation" "$result"
    printf 'See the installer message and detailed log above. Resolve that reported problem before trying again.\n'
    printf 'Files reported as unknown or modified are preserved; use verified recovery if restoration is required.\n'
    break
    ;;
  esac
  read -r answer || answer=
  while [ "$answer" = s ] || [ "$answer" = S ]; do
   /usr/bin/open -b com.apple.systempreferences || printf 'Open System Preferences/Settings from the Apple menu.\n'
   approval_actions
   read -r answer || answer=
  done
  [ "$answer" = r ] || [ "$answer" = R ] || break
 done
 if [ "$#" -eq 0 ]; then printf "\nPress Enter to close. "; read -r answer || :; fi
 exit "$result"
fi
[ -f "$tool" ] && [ ! -L "$tool" ] && [ -x "$tool" ] || fail "missing/unsafe packaged executable $tool"
exec "$tool" "$@"
