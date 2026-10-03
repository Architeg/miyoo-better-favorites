#!/bin/sh
set -eu
fail() { echo "Unsupported or ambiguous Linux host: $*" >&2; exit 2; }
cd "$(dirname "$0")" || exit 2
[ "$(uname -s)" = Linux ] || fail "requires Linux"
case "$(uname -m)" in x86_64) arch=amd64;; aarch64|arm64) arch=arm64;; *) fail "requires x64 or ARM64";; esac
# Validate numeric release prefix, then the Go runtime kernel minimum.
release=$(uname -r)
core=${release%%-*}
core=${core%%+*}
case "$core" in ''|*[!0-9.]*|.*|*..*|*.) fail "invalid kernel $release";; esac
major=${core%%.*}; tail=${core#*.}
[ "$tail" != "$core" ] || fail "incomplete kernel $release"
minor=${tail%%.*}
if [ "$major" -lt 3 ] || { [ "$major" -eq 3 ] && [ "$minor" -lt 2 ]; }; then fail "kernel 3.2 or later required"; fi
# AArch64 Linux was introduced after 3.2; do not qualify an impossible pair.
if [ "$arch" = arm64 ] && [ "$major" -eq 3 ] && [ "$minor" -lt 7 ]; then fail "ARM64 kernel 3.7 or later required"; fi
tool="./better-favorites-installer-linux-$arch"
[ -f "$tool" ] && [ ! -L "$tool" ] && [ -x "$tool" ] || fail "missing/unsafe packaged executable $tool"
exec "$tool" "$@"
