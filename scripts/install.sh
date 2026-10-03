#!/bin/sh
# Developer convenience. End users use the packaged self-contained host tool.
set -eu
cd "$(dirname "$0")/.."
exec build/release-installer install "$@"
