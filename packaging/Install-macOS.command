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
 stage=$(mktemp -d "${TMPDIR:-/tmp}/better-favorites-bootstrap.XXXXXX") || exit 2
 trap 'rm -f "$stage/installer"; rmdir "$stage"' EXIT
 trap 'exit 129' HUP
 trap 'exit 130' INT
 trap 'exit 143' TERM
 cp "$tool" "$stage/installer" || exit 2
 chmod 700 "$stage/installer" || exit 2
 set +e
 "$stage/installer" --card-launcher "$source" "$@"
 result=$?
 set -e
 if [ "$#" -eq 0 ]; then printf "\nPress Enter to close. "; read -r answer || :; fi
 exit "$result"
fi
[ -f "$tool" ] && [ ! -L "$tool" ] && [ -x "$tool" ] || fail "missing/unsafe packaged executable $tool"
exec "$tool" "$@"
