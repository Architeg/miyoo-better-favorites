#!/bin/sh

set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

SDL_DIR="$ROOT_DIR/third_party/sdl2_miyoo"
JSONC_DIR="$ROOT_DIR/third_party/json-c"
JSONC_INCLUDE="$JSONC_DIR/include"

SDL_REPO="https://github.com/Rparadise-Team/sdl2_miyoo_new.git"
SDL_COMMIT="3c68ed01fee7feffd4ea338b1cc5018a455e2be9"

SDL_TTF_DIR="$ROOT_DIR/third_party/sdl2_ttf"
SDL_TTF_INCLUDE="$SDL_TTF_DIR/include"
SDL_IMAGE_DIR="$ROOT_DIR/third_party/sdl2_image"
SDL_IMAGE_INCLUDE="$SDL_IMAGE_DIR/include"

echo "==> Preparing Better Favorites dependencies"

# ----------------------------------------------------------------------
# SDL2 Miyoo
# ----------------------------------------------------------------------

if [ -d "$SDL_DIR/.git" ]; then
    echo "==> SDL2 Miyoo repository already exists"
else
    echo "==> Cloning SDL2 Miyoo"
    git clone "$SDL_REPO" "$SDL_DIR"
fi

cd "$SDL_DIR"

git fetch --all --tags
git checkout "$SDL_COMMIT"

CURRENT_COMMIT="$(git rev-parse HEAD)"

if [ "$CURRENT_COMMIT" != "$SDL_COMMIT" ]; then
    echo "ERROR: SDL2 dependency is not at the expected commit"
    exit 1
fi

echo "==> SDL2 Miyoo ready at $CURRENT_COMMIT"

# ----------------------------------------------------------------------
# json-c headers
# ----------------------------------------------------------------------

JSONC_ARCHIVE="$SDL_DIR/sdl2/dependency/json-c-0.15.tar.gz"

if [ ! -f "$JSONC_ARCHIVE" ]; then
    echo "ERROR: json-c archive not found:"
    echo "$JSONC_ARCHIVE"
    exit 1
fi

echo "==> Preparing json-c 0.15 headers"

rm -rf "$JSONC_DIR"
mkdir -p "$JSONC_INCLUDE"

tar -xzf "$JSONC_ARCHIVE" \
    --strip-components=1 \
    -C "$JSONC_INCLUDE" \
    json-c-0.15/arraylist.h \
    json-c-0.15/debug.h \
    json-c-0.15/json.h \
    json-c-0.15/json_c_version.h \
    json-c-0.15/json_inttypes.h \
    json-c-0.15/json_object.h \
    json-c-0.15/json_object_iterator.h \
    json-c-0.15/json_pointer.h \
    json-c-0.15/json_tokener.h \
    json-c-0.15/json_types.h \
    json-c-0.15/json_util.h \
    json-c-0.15/linkhash.h \
    json-c-0.15/printbuf.h

cat > "$JSONC_INCLUDE/json_config.h" <<'EOF'
#ifndef JSON_C_CONFIG_H
#define JSON_C_CONFIG_H

#define JSON_C_HAVE_INTTYPES_H 1

#endif
EOF

echo "==> json-c headers ready"

# ----------------------------------------------------------------------
# SDL2_ttf headers
# ----------------------------------------------------------------------

SDL_TTF_ARCHIVE="$SDL_DIR/sdl2/dependency/SDL2_ttf-2.20.2.tar.gz"

if [ ! -f "$SDL_TTF_ARCHIVE" ]; then
    echo "ERROR: SDL2_ttf archive not found:"
    echo "$SDL_TTF_ARCHIVE"
    exit 1
fi

echo "==> Preparing SDL2_ttf 2.20.2 headers"

rm -rf "$SDL_TTF_DIR"
mkdir -p "$SDL_TTF_INCLUDE"

tar -xzf "$SDL_TTF_ARCHIVE" \
    --strip-components=1 \
    -C "$SDL_TTF_INCLUDE" \
    SDL2_ttf-2.20.2/SDL_ttf.h

echo "==> SDL2_ttf headers ready"

# ----------------------------------------------------------------------
# SDL2_image headers
# ----------------------------------------------------------------------

SDL_IMAGE_ARCHIVE="$SDL_DIR/sdl2/dependency/SDL2_image-2.8.1.tar.gz"

if [ ! -f "$SDL_IMAGE_ARCHIVE" ]; then
    echo "ERROR: SDL2_image archive not found:"
    echo "$SDL_IMAGE_ARCHIVE"
    exit 1
fi

echo "==> Preparing SDL2_image 2.8.1 headers"

rm -rf "$SDL_IMAGE_DIR"
mkdir -p "$SDL_IMAGE_INCLUDE"

tar -xzf "$SDL_IMAGE_ARCHIVE" \
    --strip-components=2 \
    -C "$SDL_IMAGE_INCLUDE" \
    SDL2_image-2.8.1/include/SDL_image.h

echo "==> SDL2_image headers ready"

echo
echo "Dependencies successfully prepared."
