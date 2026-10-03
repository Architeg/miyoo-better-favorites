#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
exec build/release-installer uninstall "$@"
