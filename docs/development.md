# Technical development guide

This native SDL2 app wraps Onion's existing favorites and launch lifecycle. The
[roadmap](roadmap.md) is authoritative for scope; [status](development-status.md)
separates device acceptance from host/ARM fixtures. Update both on milestone or
hardware evidence changes. Historical investigations remain in the
[developer index](developer-index.md); user installation belongs in
[quick start](install.md), not a source checkout.

## Data and browser

`favorites_parser.cpp` loads `Roms/favourite.json` into `Favorite` records with
original labels, paths and exact source record identity. A per-parse cache maps
emulator config paths to labels, including missing/malformed results, and refreshes
on every reload. It reduces observed host reads70→3; no measured device gain.
`browser_model.cpp`, `ui_rows.cpp`, `navigation.cpp` build grouped or flat rows,
literal-label or prefix-ignoring title sorting, selectable console/page movement
and viewport clamping. Prefix display and sorting are independent.

`browser_preferences.cpp` owns atomic browser options in a separate file;
`app_settings.cpp` retains the compact generation-based return protocol;
`home_entry_settings.cpp` owns independent Home preference/availability. Defaults:
groupON, prefixShow, OriginalLabel, returnOFF, HomeOFF. Writes never update
favorites/history. On failure keep the saved setting. `browser_state.cpp` saves
selected original launch/ROM identity, nearby index and viewport; removed favorites
select a nearby remaining record. Code support is not a persistent option unless
it is actually exposed and saved.

`favorite_removal.cpp` compares the captured source snapshot, backs up exact bytes,
preserves unrelated fields/records, stages on the same filesystem and replaces
only the selected exact record. It never removes ROM/artwork/save/history. Conflict
checks are not atomic compare-and-swap; offline/cooperative writers are assumed.

## Rendering/resources/audio

`theme_loader.cpp` resolves profile override → active theme → Onion/Miyoo fallback.
`theme_fonts.cpp` validates faces; regular explanation fonts are sought only in
current profile/theme, never unrelated themes. Heavier built-in faces remain
heavier if no suitable regular face exists. Browser fonts remain separate from
app-owned styled menu fonts. `browser_resources.cpp` validates bounded artwork and
falls back on missing/corrupt images. Geometry remains640×480 with existing row,
preview, console-heading and footer regions.

`menu_renderer.cpp` shares measured text/rows/button hints. Text badges remain
legible independent of hideIcons; hideHints hides footers. SELECT actions, Y
Settings, nested About/Help and centered Cancel-first removal retain isolated
controls. Settings descriptions are fixed above the footer; dialog title paging
is explicit. `browser_titles.cpp`/`title_scroll.cpp` cache selected UTF-8 surfaces,
clip to the real title region and scroll only overflow after ~1s. Menu entry pauses
scroll; returning restarts delay. No per-frame font/image loading.

Audio uses qualified OSS SDL plus per-app `SDL_AUDIODRIVER=dsp` and libpadsp preload.
Do not alter Onion global audio configuration, replace the working SDL binary or
change font ownership in a release tooling fix. Required libs and unresolved
source correspondence are documented in [build provenance](release/build.md).

## Onion launch and GameSwitcher

A validates the selected stored paths without rewriting `..` segments; resolved
regular-file checks and shell/parser-safe quoting guard Onion's double-quoted
command. It privately stages command and compact type5 recent record, saves browser
state, exits the event loop and completes SDL/audio cleanup. Exit20 tells the outer
launcher to register the recent list and publish the active command atomically on
its filesystem. `launch_request.cpp` checks exact invocation ownership, uses Onion's
RandomGamePicker/quick_switch flow, and rolls back only owned outputs on failure.
Normal B exit cancels private requests; no game is executed inside SDL.

MENU privately requests GameSwitcher, saves state and cleans up, then exit21 asks
the launcher to publish the compatible switcher command. It never registers the
highlighted favorite merely for MENU. Open-menu MENU only closes the menu. The
captured active command must match the exact accepted native Apps format (including
spacing/libpadsp), so incompatible launch routes are refused rather than weakening
ownership.

Optional `integration/onion-return/runtime.patch` adds only runtime boundary hooks;
`better_favorites_return.sh` tracks an originating **session**, not one ROM. With
returnON, GameSwitcher B/START consumes ownership before reopening the app;
switching games retains it. A resumes. Ordinary MainUI return/direct game exit,
restart/reboot/disable/generation change invalidate it; shutdown wins. No browser
or helper stays alive during gameplay. Stock sessions remain unchanged. See
[exact runtime source and lifecycle](onion-return.md).

## Native Home integration

Home-only Favorites activation dispatches MainUI's native AppAction. Scoped writer
and publication hooks stage and no-replace publish the canonical app command;
native result3/readback alone were insufficient conflict protection. Disabled,
malformed or unavailable app falls back to stock. Existing tile/theme/Apps and X/Y
are unchanged. All shortcuts are deferred beyondv1.0.

`integration/mainui-home/adapter.cpp` and `hooks.S` preserve calling convention,
displaced instructions, allocation cleanup/results and native state saving. The
exact-hash generator relocates PHDR/unwind metadata, uses separate RX permissions
and existing RW BSS page capacity. [M6 prototype](m6-mainui-prototype.md) records
byte maps; `tools/release-installer/elf.go` reproduces the same audited output hashes.
Payload/mapped sizes are not measured RSS. Native Home diagnostic writes are
optional/bounded and never authority for ownership.

## Offline installation and recovery

`tools/release-installer` uses Go's standard library. No interpreter/WSL is needed
on the user computer. Install verifies package files and exact Onion inputs before
creating verified host/card recovery trees. Stock originals and pre-operation
snapshots are distinct. All replacements are staged/flushed first. Order: backup,
app, helper, runtime, MainUI, availability/installed metadata. Rollback only restores
an invocation's exact published bytes; foreign modifications are retained.
Uninstall/Restore preflight all originals/destinations, restore runtime/MainUI first,
then remove owned helper/receipt and mark journals inert. Preferences and backups
remain. Multi-file publication is not one atomic filesystem transaction.

Unix opens O_NOFOLLOW|O_NONBLOCK then validates the descriptor as a regular file.
Windows uses CreateFileW OPEN_REPARSE_POINT, rejects directory/reparse handles and
special/network paths, and publishes same-directory stages with MoveFileExW
REPLACE_EXISTING|WRITE_THROUGH, without COPY_ALLOWED. This is not a guarantee of
FAT physical power-loss immunity. [Microsoft API behavior](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw).
Native Windows execution/reader qualification remains pending. Exclusive offline
card access is required; separate rechecks are conflict detection, not CAS.

Per-card recovery retains exact SD paths and checksums. Host/card copies work
without MainUI, Terminal, Wi-Fi or SSH. Never delete the app before restoration or
copy another card's originals. [Recovery](uninstall.md).

## Diagnostics and release

`diagnostics.cpp` bounds app-owned current/previous logs; launcher modes rotate and
append boundary events best-effort. Normal navigation/per-frame messages are
absent. No logging daemon is added; log helper invocations finish before handoff.
Host Export diagnostics reads a bounded allowlist and does not silently enable
tracing. Filesystem/proc RAM evidence cannot be inferred from the mounted card.
[Caps/privacy/timing limits](diagnostics.md).

`scripts/build.sh` pins Docker toolchain digest and version/source literals;
`tools/package-release.py` requires a clean commit, builds native host tools and
creates app-only/installer ZIPs plus source/notices/checksums. Generated binaries,
previews, logs, personal settings, backups and vendor fixtures stay out of Git.
[Build/provenance](release/build.md), [rc.1 gates](release/rc.1.md),
[contributing/tests](../CONTRIBUTING.md). Candidate acceptance requires the exact
ZIP, native host filesystem tests and hardware installation roundtrip; compilation
or emulation alone is insufficient.
