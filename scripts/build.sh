#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
# Pin the existing verified toolchain image; do not replace working audio deps.
image=aemiii91/miyoomini-toolchain@sha256:a864876472a489f63d6223d2c8ad61e12ced679c0b177ae9429e51f3673ef4e7
commit=$(git rev-parse HEAD)
version=${BETTER_FAVORITES_RELEASE_VERSION:-1.0.0-rc.5}
docker run --rm -v "$PWD":/root/workspace/miyoo-better-favorites \
 -w /root/workspace/miyoo-better-favorites "$image" /bin/bash -c \
 "source /root/setup-env.sh && make -B all RELEASE_DEFINES='-DBETTER_FAVORITES_VERSION=\\\"$version\\\" -DBETTER_FAVORITES_COMMIT=\\\"$commit\\\"' && sh integration/mainui-home/build.sh build/rc-adapter"
