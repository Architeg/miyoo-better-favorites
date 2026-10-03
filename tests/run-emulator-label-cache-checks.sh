#!/bin/sh
# Uses existing host-only cJSON compatibility layer, no dependency installation.
set -eu
cd "$(dirname "$0")/.."
output=${1:?New output directory required}
mkdir "$output"
c++ -std=c++17 -O2 -Wall -Wextra -Iinclude -Itools/preview_support $(pkg-config --cflags libcjson) tests/emulator_label_cache_test.cpp src/favorites_parser.cpp src/browser_model.cpp src/ui_rows.cpp src/navigation.cpp $(pkg-config --libs libcjson) -o "$output/cache-test"
if [ "$#" -gt 2 ]; then
    "$output/cache-test" "$3" > "$output/cached-output.txt" 2> "$output/cached-counts.txt"
else
    "$output/cache-test" > "$output/cached-output.txt"
fi
# Caller can supply the pre-change parser for exact before/after fixture comparison.
if [ "$#" -gt 1 ]; then
    c++ -std=c++17 -O2 -Wall -Wextra -DBETTER_FAVORITES_UNCACHED_REFERENCE -Iinclude -Itools/preview_support $(pkg-config --cflags libcjson) tests/emulator_label_cache_test.cpp "$2" src/browser_model.cpp src/ui_rows.cpp src/navigation.cpp $(pkg-config --libs libcjson) -o "$output/reference-test"
    if [ "$#" -gt 2 ]; then
        "$output/reference-test" "$3" > "$output/reference-output.txt" 2> "$output/reference-counts.txt"
    else
        "$output/reference-test" > "$output/reference-output.txt"
    fi
    cmp "$output/reference-output.txt" "$output/cached-output.txt"
fi
tail -n 1 "$output/cached-output.txt"
