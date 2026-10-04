#!/bin/sh
# Read-only host checks precede installer execution and all SD writes.
set -eu
fail() { echo "Unsupported or ambiguous macOS host: $*" >&2; exit 2; }
cd "$(dirname "$0")" || exit 2
[ "$(/usr/bin/uname -s)" = Darwin ] || fail "requires macOS"
version=$(/usr/bin/sw_vers -productVersion) || fail "version unavailable"
case "$version" in ''|*[!0-9.]*|.*|*..*|*.) fail "invalid version $version";; esac
major=${version%%.*}
[ "$major" -ge 12 ] 2>/dev/null || fail "Monterey 12 or later required"
machine=$(/usr/bin/uname -m) || fail "architecture unavailable"
arm=$(/usr/sbin/sysctl -n hw.optional.arm64 2>/dev/null) || arm=absent
translated=$(/usr/sbin/sysctl -n sysctl.proc_translated 2>/dev/null) || translated=0
case "$arm:$machine:$translated" in
 1:arm64:0|1:x86_64:1|0:x86_64:1|absent:x86_64:1) arch=arm64;;
 0:x86_64:0) arch=amd64;;
 absent:x86_64:0)
  [ "$(/usr/sbin/sysctl -n hw.cputype 2>/dev/null)" = 16777223 ] || fail "native architecture unavailable"
  arch=amd64;;
 *) fail "inconsistent architecture/translation ($arm/$machine/$translated)";;
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
 while :; do
  verify
  set +e
  "$approved" --card-launcher "$source" "$@"
  result=$?
  set -e
  [ "$result" -ne 0 ] || break
  case "$result" in
   137)
    printf '\nInstaller terminated with status 137 (possible SIGKILL).\n'
    printf 'This alone does not establish that macOS security blocked it.\n'
    printf 'If macOS displayed an approval warning, approve only this verified file:\n%s\n' "$approved"
    printf 'Monterey: System Preferences > Security & Privacy > General.\n'
    printf 'Newer macOS: System Settings > Privacy & Security.\n'
    printf 'Do not override malware/damaged-file or managed-policy warnings.\n'
    printf 'The verified file is retained. After approval, type r to retry; anything else closes: '
    ;;
   *)
    case "${1:-menu}" in install) operation=Installation;; uninstall) operation=Uninstall;; *) operation=Operation;; esac
    printf '\n%s failed (status %s).\n' "$operation" "$result"
    printf 'The installer reason is shown above. Resolve that reported problem before trying again.\n'
    printf 'Files reported as unknown or modified are preserved; use verified recovery if restoration is required.\n'
    break
    ;;
  esac
  read -r answer || answer=
  [ "$answer" = r ] || break
 done
 if [ "$#" -eq 0 ]; then printf "\nPress Enter to close. "; read -r answer || :; fi
 exit "$result"
fi
[ -f "$tool" ] && [ ! -L "$tool" ] && [ -x "$tool" ] || fail "missing/unsafe packaged executable $tool"
exec "$tool" "$@"
