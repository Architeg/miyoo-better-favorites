#!/bin/sh
# Reuse existing native SDL backend objects; no previews or card writes.
set -eu
cd "$(dirname "$0")/.."
backend=${1:?Existing native backend directory required}
output=${2:?Temporary output directory required}
font=${3:?Existing usable theme font required}
mkdir -p "$output"
c++ -std=c++17 -Wall -Wextra -Iinclude -Itools/preview_support -Ithird_party/sdl2_ttf/include -Ithird_party/sdl2_image/include $(sdl2-config --cflags) $(pkg-config --cflags libcjson) tests/browser_resources_test.cpp src/browser_resources.cpp src/theme_loader.cpp src/theme_fonts.cpp "$backend/SDL_ttf.o" "$backend/IMG_png.o" "$backend/png_only.o" $(sdl2-config --libs) $(pkg-config --libs freetype2 libpng libcjson) -o "$output/browser-resources-test"
"$output/browser-resources-test" "$font"
