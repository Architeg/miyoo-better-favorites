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
[ -f "$tool" ] && [ ! -L "$tool" ] && [ -x "$tool" ] || fail "missing/unsafe packaged executable $tool"
exec "$tool" "$@"
