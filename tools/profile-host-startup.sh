#!/bin/sh
# Existing native SDL backend only; read-only corpus/theme access, no screenshots.
set -eu
cd "$(dirname "$0")/.."
backend=${1:?Existing native backend directory required}
output=${2:?Temporary output directory required}
card=${3:?Read-only card/corpus root required}
runs=${4:-7}
mkdir -p "$output"
c++ -std=c++17 -O2 -Wall -Wextra -Iinclude -Itools/preview_support -Ithird_party/sdl2_ttf/include -Ithird_party/sdl2_image/include $(sdl2-config --cflags) $(pkg-config --cflags libcjson) tools/profile-host-startup.cpp src/favorites_parser.cpp src/browser_model.cpp src/ui_rows.cpp src/navigation.cpp src/favorite_removal.cpp src/menu_renderer.cpp src/menu_text.cpp src/theme_loader.cpp src/theme_fonts.cpp src/browser_resources.cpp "$backend/SDL_ttf.o" "$backend/IMG_png.o" "$backend/png_only.o" $(sdl2-config --libs) $(pkg-config --libs freetype2 libpng libcjson) -o "$output/profile-host-startup"
"$output/profile-host-startup" "$card" "$runs"
