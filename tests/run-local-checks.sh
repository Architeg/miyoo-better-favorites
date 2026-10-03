#!/bin/sh
# Host C++ checks use isolated temporary files; no card writes or new dependencies.
set -eu
cd "$(dirname "$0")/.."
output=$(mktemp -d "${TMPDIR:-/tmp}/better-favorites-checks.XXXXXX")
trap 'rm -rf "$output"' EXIT HUP INT TERM
compile_run() {
    name=$1; shift
    c++ -std=c++17 -Wall -Wextra -Iinclude "$@" -o "$output/$name"
    "$output/$name"
}
compile_run startup-profile tests/startup_profile_test.cpp
compile_run navigation-boundaries tests/navigation_boundaries_test.cpp src/navigation.cpp src/ui_rows.cpp
compile_run title-scroll tests/title_scroll_test.cpp src/title_scroll.cpp
compile_run text tests/menu_text_test.cpp src/menu_text.cpp
compile_run menu tests/menu_state_test.cpp src/menu_state.cpp src/navigation.cpp src/ui_rows.cpp
compile_run removal -DBETTER_FAVORITES_REMOVAL_TESTING tests/favorite_removal_test.cpp src/favorite_removal.cpp
compile_run browser-preferences -DBETTER_FAVORITES_PREFERENCES_TESTING tests/browser_preferences_test.cpp src/browser_preferences.cpp src/app_settings.cpp
compile_run browser-model tests/browser_model_test.cpp src/browser_model.cpp src/ui_rows.cpp src/navigation.cpp src/favorite_removal.cpp
compile_run settings tests/app_settings_test.cpp src/app_settings.cpp
compile_run state tests/browser_state_test.cpp src/browser_state.cpp src/ui_rows.cpp src/navigation.cpp
compile_run launch -DBETTER_FAVORITES_HANDOFF_TESTING tests/launch_request_test.cpp src/launch_request.cpp src/app_settings.cpp
compile_run switcher -DBETTER_FAVORITES_HANDOFF_TESTING tests/switcher_request_test.cpp src/launch_request.cpp src/app_settings.cpp
if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists libcjson; then
    sh tests/run-emulator-label-cache-checks.sh "$output/emulator-label-cache"
else
    echo 'Emulator-label cache host test: SKIP (existing libcjson host bridge unavailable)'
fi
python3 tests/profiling_tools_test.py
python3 tests/profiling_activation_test.py
python3 tests/profiling_retirement_test.py
python3 tests/launcher_handoff_test.py
python3 tests/runtime_return_test.py
sh -n App/BetterFavoritesTest/launch.sh integration/onion-return/better_favorites_return.sh tools/profile-device-launch.sh tools/profile-memory-session.sh tools/sample-device-memory.sh
