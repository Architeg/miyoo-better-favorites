#!/bin/sh
# Native SDL checks only; no screenshots, mounted-card writes or new dependencies.
# Reuse the same preserved official SDL_ttf source as build-menu-preview.sh.
set -eu
cd "$(dirname "$0")/.."
ttf_source=${1:?Path to existing SDL_ttf.c required}
output=${2:?Temporary build directory required}
font=${3:?Existing theme font required}
mkdir -p "$output"
cc -c "$ttf_source" -Ithird_party/sdl2_ttf/include $(sdl2-config --cflags) $(pkg-config --cflags freetype2) -o "$output/SDL_ttf.o"
c++ -std=c++17 -Wall -Wextra -DBETTER_FAVORITES_TITLE_TESTING -Iinclude -Ithird_party/sdl2_ttf/include $(sdl2-config --cflags) tests/browser_titles_test.cpp src/browser_titles.cpp src/title_scroll.cpp "$output/SDL_ttf.o" $(sdl2-config --libs) $(pkg-config --libs freetype2) -o "$output/browser-titles-test"
"$output/browser-titles-test" "$font"
