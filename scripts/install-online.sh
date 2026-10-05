#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Download the single user ZIP; all card writes remain in its existing installer.
# Bash 3.2 compatible (macOS Monterey); no Python, Go, sudo or new executable.
set -euo pipefail

# Parse the complete entry before attaching /dev/tty when invoked through a pipe.
main() {

readonly REPOSITORY=Architeg/miyoo-better-favorites
readonly RELEASE_ROOT=https://github.com/Architeg/miyoo-better-favorites/releases/download
readonly DEFAULT_TAG=v1.0.0-rc.7
readonly MAX_ZIP_BYTES=104857600
tag=$DEFAULT_TAG
card=""
action=""
authenticated=0
work=""
bold=""; reset=""
if [[ -t 1 && ${TERM:-} != dumb && -z ${NO_COLOR+x} ]]; then
  bold=$'\033[1m'; reset=$'\033[0m'
fi
heading() { printf '\n%b%s%b\n\n' "$bold" "$1" "$reset"; }
fail() {
  printf '\nBetter Favorites: %s\n' "$*" >&2
  [[ -z $work ]] || printf 'Downloaded files retained: %s\n' "$work" >&2
  exit 1
}
usage() {
  printf '%s\n' 'Usage: install-online.sh [install|uninstall|export-diagnostics]' \
    '       [--tag vX.Y.Z[-rc.N]] [--sd-root /mounted/card] [--github-auth]' \
    'No action opens the existing Install / Uninstall / Export diagnostics menu.'
}
while [[ $# -gt 0 ]]; do
  case $1 in
    --tag|--sd-root)
      [[ $# -ge 2 ]] || fail "$1 needs a value"
      if [[ $1 == --tag ]]; then tag=$2; else card=$2; fi
      shift 2;;
    --github-auth) authenticated=1; shift;;
    install|uninstall|export-diagnostics)
      [[ -z $action ]] || fail 'choose only one action'
      action=$1; shift;;
    --help|-h) usage; exit 0;;
    *) fail "unknown argument: $1";;
  esac
done
[[ $tag =~ ^v[0-9]+\.[0-9]+\.[0-9]+(-[A-Za-z0-9]+([.-][A-Za-z0-9]+)*)?$ ]] || fail 'invalid release tag'
for tool in curl unzip zipinfo awk mktemp; do
  command -v "$tool" >/dev/null || fail "$tool is required; use the offline ZIP instead"
done
if command -v shasum >/dev/null; then
  digest() { shasum -a 256 "$1" | awk '{print $1}'; }
elif command -v sha256sum >/dev/null; then
  digest() { sha256sum "$1" | awk '{print $1}'; }
else
  fail 'a SHA-256 tool is required; use the offline ZIP instead'
fi

# Identify a supported ABI now; the packaged wrapper repeats native/version
# checks before selecting the executable, including Rosetta on Apple Silicon.
os=$(uname -s)
machine=$(uname -m)
case $os in
  Darwin)
    version=$(/usr/bin/sw_vers -productVersion) || fail 'macOS version unavailable'
    [[ $version =~ ^[0-9]+(\.[0-9]+)*$ ]] || fail 'invalid macOS version'
    [[ ${version%%.*} -ge 12 ]] || fail 'macOS Monterey or later required'
    translated=$(/usr/sbin/sysctl -n sysctl.proc_translated 2>/dev/null) || translated=0
    case $machine:$translated in
      arm64:0|x86_64:1) platform='macOS Apple Silicon';;
      x86_64:0) platform='macOS Intel';;
      *) fail 'unsupported or ambiguous Mac architecture';;
    esac
    entry=Install-macOS.command;;
  Linux)
    case $machine in
      x86_64) platform='Linux x64';;
      aarch64|arm64) platform='Linux ARM64';;
      *) fail 'Linux x64 or ARM64 required';;
    esac
    entry=Install-Linux.sh;;
  *) fail 'this download route supports macOS and Linux only';;
esac

# Do not consume a piped script as menu input (curl ... | bash).
if [[ ! -t 0 ]]; then
  if ! { exec </dev/tty; } 2>/dev/null; then fail 'open an interactive Terminal to use this installer'; fi
fi
heading 'Better Favorites — download and install'
printf 'Computer: %s\nRelease: %s\n' "$platform" "$tag"
printf '\nPower off the Miyoo and connect its SD card.\n'

is_card() {
  [[ -d $1 && ! -L $1 && -d $1/App && -d $1/Roms &&
     -f $1/.tmp_update/onionVersion/version.txt && -f $1/.tmp_update/runtime.sh ]]
}
if [[ -z $card ]]; then
  candidates=("")
  candidate_count=0
  if [[ $os == Darwin ]]; then
    roots=(/Volumes/*)
  else
    roots=(/media/*/* /run/media/*/* /mnt/* /media/*)
  fi
  for root in "${roots[@]}"; do
    if is_card "$root"; then
      duplicate=0
      for existing in "${candidates[@]}"; do [[ $existing != "$root" ]] || duplicate=1; done
      if [[ $duplicate -eq 0 ]]; then
        candidates[$candidate_count]=$root
        candidate_count=$((candidate_count + 1))
      fi
    fi
  done
  if [[ $candidate_count -eq 1 ]]; then
    card=${candidates[0]}
  else
    if [[ $candidate_count -gt 1 ]]; then
      printf '\nMore than one Onion card is connected:\n'
      printf '  %s\n' "${candidates[@]}"
    fi
    printf '\nSD-card location (for example /Volumes/MIYOO or /media/you/MIYOO): '
    IFS= read -r card || fail 'no card selected'
  fi
fi
[[ $card == /* ]] || fail 'use the absolute SD-card location'
is_card "$card" || fail 'not an identifiable mounted Onion card'
card=$(cd "$card" && pwd -P)
printf '\nCard: %s\nUse this card? (y/N): ' "$card"
IFS= read -r answer || fail 'cancelled'
case $answer in y|Y) :;; *) printf 'Cancelled. No card writes.\n'; exit 0;; esac

# Keep the verified package on the computer for later offline reuse. The backend
# creates the same portable card recovery as the double-click route.
[[ ${HOME:-} == /* && -d $HOME && ! -L $HOME ]] || fail 'a regular computer home directory is required'
store=$HOME/BetterFavorites-Downloads
[[ ! -L $store && ( ! -e $store || -d $store ) ]] || fail 'unsafe download directory'
mkdir -p "$store"
store=$(cd "$store" && pwd -P)
case $store/ in "$card/"*) fail 'download directory must be outside the SD card';; esac
work=$(mktemp -d "$store/$tag.XXXXXX")
chmod 700 "$work"
archive=better-favorites-${tag#v}.zip
fetch() {
  curl -q --fail --location --silent --show-error --proto '=https' --proto-redir '=https' \
    --connect-timeout 15 --max-time 300 --max-filesize "$3" --output "$2" "$1"
}
heading 'Downloading the release'
if [[ $authenticated -eq 1 ]]; then
  command -v gh >/dev/null || fail '--github-auth requires the GitHub CLI with repository access'
  gh release download "$tag" --repo "$REPOSITORY" --pattern SHA256SUMS --pattern "$archive" --dir "$work" || fail 'authenticated download failed'
else
  fetch "$RELEASE_ROOT/$tag/SHA256SUMS" "$work/SHA256SUMS" 1048576 || fail 'release unavailable; private repositories require --github-auth, or use the offline ZIP'
  fetch "$RELEASE_ROOT/$tag/$archive" "$work/$archive" "$MAX_ZIP_BYTES" || fail 'package download failed; use the offline ZIP if necessary'
fi
checksum_size=$(wc -c < "$work/SHA256SUMS" | tr -d '[:space:]')
archive_size=$(wc -c < "$work/$archive" | tr -d '[:space:]')
[[ $checksum_size -gt 0 && $checksum_size -le 1048576 && $archive_size -gt 0 && $archive_size -le $MAX_ZIP_BYTES ]] || fail 'download exceeds size limits'
expected=$(awk -v name="$archive" '$2 == name { count++; if (NF != 2 || length($1) != 64 || $1 ~ /[^0-9a-fA-F]/) bad=1; value=tolower($1) } END { if (count != 1 || bad) exit 1; print value }' "$work/SHA256SUMS") || fail 'missing or ambiguous ZIP checksum'
[[ $(digest "$work/$archive") == "$expected" ]] || fail 'ZIP checksum mismatch; nothing installed'
printf 'ZIP checksum verified.\n'

# Reject dangerous entries before unzip; the packaged backend subsequently
# verifies every transport/payload checksum before any publication.
zipinfo -1 "$work/$archive" > "$work/members.txt" || fail 'cannot inspect ZIP'
awk '
  /^\// || /\\/ || /:/ || /\^/ || /[[:cntrl:]]/ || /(^|\/)\.\.?($|\/)/ || /\/\// { exit 1 }
  { if ($0 == "" || seen[$0]++) exit 1; count++ }
  END { if (count == 0 || count > 5000) exit 1 }
' "$work/members.txt" || fail 'unsafe or duplicate ZIP paths'
zipinfo -l "$work/$archive" > "$work/zip-details.txt" || fail 'cannot inspect ZIP modes'
awk '
  /^[dlbcps-][rwxstST-]/ { if (substr($1,1,1) != "-" && substr($1,1,1) != "d") exit 1; count++; size += $4 }
  END { if (count == 0 || size > 1073741824) exit 1; print count }
' "$work/zip-details.txt" > "$work/member-count" || fail 'unsupported ZIP entries or expanded size'
[[ $(wc -l < "$work/members.txt" | tr -d '[:space:]') == $(cat "$work/member-count") ]] || fail 'ambiguous ZIP member names'
unzip -tqq -P '' "$work/$archive" || fail 'damaged or encrypted ZIP'
mkdir "$work/package"
unzip -qq -P '' "$work/$archive" -d "$work/package" || fail 'ZIP extraction failed'
computer=$work/package/App/BetterFavorites/computer
[[ -f $computer/$entry && ! -L $computer/$entry && -f $computer/transport.json && -f $computer/package.json ]] || fail 'release does not contain the current full installer layout'
heading 'Opening the existing installer'
printf 'Verified package kept at:\n  %s\n\n' "$work/package"
printf 'Install, uninstall and diagnostics use the same tools and recovery as the offline ZIP.\n'
printf 'The installer will confirm that the Miyoo is powered off before changing the card.\n\n'
if [[ -n $action ]]; then
  exec /bin/sh "$computer/$entry" --staged-card "$card" "$computer" "$action"
else
  exec /bin/sh "$computer/$entry" --staged-card "$card" "$computer"
fi

}

main "$@"
