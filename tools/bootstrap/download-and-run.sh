#!/bin/sh
# Preparation-only entry: no public bootstrap URL is advertised until release assets exist.
set -eu
fail() { echo "Bootstrap: $*" >&2; exit 1; }
action=${1:-}; tag=${2:-}
case "$action" in install|uninstall) ;; *) fail 'usage: download-and-run.sh install|uninstall vX.Y.Z [installer arguments]';; esac
printf '%s\n' "$tag" | LC_ALL=C grep -Eq '^v[0-9]+\.[0-9]+\.[0-9]+(-[A-Za-z0-9.-]+)?$' || fail 'explicit published tag required'
shift 2
case "$(uname -s):$(uname -m)" in
 Darwin:*)
  version=$(sw_vers -productVersion); [ "${version%%.*}" -ge 12 ] || fail 'macOS Monterey or later required'
  machine=$(uname -m); translated=$(/usr/sbin/sysctl -n sysctl.proc_translated 2>/dev/null || echo 0)
  case "$machine:$translated" in arm64:0|x86_64:1) arch=arm64;; x86_64:0) arch=amd64;; *) fail 'ambiguous Mac architecture';; esac
  host=darwin;;
 Linux:x86_64) host=linux;arch=amd64;;
 Linux:aarch64|Linux:arm64) host=linux;arch=arm64;;
 *) fail 'unsupported OS or architecture';;
esac
command -v curl >/dev/null || fail 'curl unavailable; use the offline package'
repo=https://github.com/Architeg/miyoo-better-favorites
api=https://api.github.com/repos/Architeg/miyoo-better-favorites
name=better-favorites-bootstrap-$host-$arch
# Durable computer-side location; downloads and installer recovery are retained.
store=$HOME/BetterFavorites-Downloads
mkdir -p "$store"
work=$(mktemp -d "$store/bootstrap-$tag.XXXXXX")
fetch() { curl -q --fail --location --proto '=https' --proto-redir '=https' --silent --show-error "$1" -o "$2"; }
fetch "$api/releases/tags/$tag" "$work/release.json"
grep -Eq '"draft"[[:space:]]*:[[:space:]]*false' "$work/release.json" || fail 'draft/private release refused'
grep -Eq '"published_at"[[:space:]]*:[[:space:]]*"[0-9]' "$work/release.json" || fail 'release is unpublished'
grep -Eq '"immutable"[[:space:]]*:[[:space:]]*true' "$work/release.json" || fail 'immutable published release required'
fetch "$repo/releases/download/$tag/BOOTSTRAP-SHA256SUMS" "$work/BOOTSTRAP-SHA256SUMS"
expected=$(awk -v n="$name" '$2==n && length($1)==64 {print $1}' "$work/BOOTSTRAP-SHA256SUMS")
printf '%s\n' "$expected" | LC_ALL=C grep -Eq '^[a-f0-9]{64}$' || fail 'missing/ambiguous bootstrap checksum'
fetch "$repo/releases/download/$tag/$name" "$work/$name"
if command -v shasum >/dev/null; then actual=$(shasum -a 256 "$work/$name" | awk '{print $1}');
elif command -v sha256sum >/dev/null; then actual=$(sha256sum "$work/$name" | awk '{print $1}');
else fail 'SHA-256 tool unavailable; use the offline package'; fi
[ "$actual" = "$expected" ] || fail 'bootstrap checksum mismatch'
chmod 700 "$work/$name"
exec "$work/$name" "$action" --tag "$tag" --store "$store" -- "$@"
