#!/bin/sh
# No installation or production dependency changes. Pass a preserved directory
# containing official SDL_ttf 2.20.2 SDL_ttf.c and SDL_image 2.8.1 src/IMG_png.c.
set -eu
cd "$(dirname "$0")/.."
source_dir=${1:?SDL upstream source directory required}
output=${2:?Preview build output directory required}
mkdir -p "$output"
cc -c "$source_dir/SDL_ttf.c" -Ithird_party/sdl2_ttf/include $(sdl2-config --cflags) $(pkg-config --cflags freetype2) -o "$output/SDL_ttf.o"
cc -c "$source_dir/IMG_png.c" -DLOAD_PNG -Ithird_party/sdl2_image/include $(sdl2-config --cflags) $(pkg-config --cflags libpng) -o "$output/IMG_png.o"
cc -c tools/preview_support/png_only.c -Ithird_party/sdl2_image/include $(sdl2-config --cflags) -o "$output/png_only.o"
c++ -std=c++17 -Wall -Wextra -DBETTER_FAVORITES_MENU_RENDER_TESTING -Iinclude -Itools/preview_support -Ithird_party/sdl2_ttf/include -Ithird_party/sdl2_image/include $(sdl2-config --cflags) $(pkg-config --cflags libcjson) tools/menu_preview.cpp src/menu_renderer.cpp src/menu_text.cpp src/theme_loader.cpp "$output/SDL_ttf.o" "$output/IMG_png.o" "$output/png_only.o" $(sdl2-config --libs) $(pkg-config --libs freetype2 libpng libcjson) -o "$output/menu-preview"
