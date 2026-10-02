# Development status

Updated: 2026-10-02. Native SDL2 frontend for Onion's existing
`/mnt/SDCARD/Roms/favourite.json`.

## Hardware-confirmed behavior

The user confirmed the preceding UI across dark and light themes, with device
photos: SELECT actions, full-screen Settings/Help, automatic-return explanation,
readable control labels and centered Cancel-first removal. Navigation, exact-record
removal, A launch, MENU GameSwitcher, persistence and OSS/libpadsp audio are reported
working. These hardware results precede the final readability adjustments below.

Earlier verified behavior includes Automatic return OFF using Onion's normal
menus, ON reopening Better Favorites after GameSwitcher B, switching games
retaining the origin session, A resuming, and selection/viewport restoration.
START, failure, shutdown, restart, removed-entry fallback and stock-menu isolation have
focused host coverage; not every edge case has independent device confirmation.
No device RAM measurement is recorded.

## Current UI checkpoint

Base: `2f7571d03055c0c11de69625c98ff9d05788f031` on `main`,
`Open Onion GameSwitcher from browser MENU`. The pending checkpoint adds SELECT
Launch/Remove/Settings/Help, Y Settings, nested explanation, measured themed menus,
modal confirmation, guarded exact-record removal and focused tests.

Final micro-adjustments use app-owned slightly larger/heavier body/hint fonts,
same-family heavier faces where available, otherwise SDL_ttf bold. Shared browser
fonts are not altered. Theme-colored filled letter circles/named capsules and
slightly stronger arrows improve readability. **These final adjustments await
device verification**, despite passing host SDL previews and the Docker ARM build.
Only two new previews were generated: light Help and dark removal; other render
checks run offscreen. See [menu presentation](menu-presentation.md) and
[removal semantics/safeguards](menu-removal.md).

Checks: menu text/navigation, removal backup/conflict/failure fixtures, settings,
state, A/history/rollback, MENU handoff, launcher cleanup, runtime session ownership,
installer fixtures, actual SDL bounds/owned-font/paging checks and shell syntax.
ARM compilation remains distinct from hardware verification; the existing
SDL2_ttf `libbz2.so.1.0` linker warning persists.

Builds, previews, logs, personal settings/state, backups and temporary artifacts
are excluded from Git. This checkpoint does not deploy or change card files.
The last deployed binary SHA-256 is
`57c104b6835fc3c9eea4bd5ba462db3ff82c9a191f46cfef4ec4e33534864f98`.
Last app backup: `../miyoo-better-favorites-backups/20261002-205610-device-menu-presentation-binary`.

## Preserved Onion lifecycle and ownership

Non-repeated A saves browser position, privately stages command/recent data and
exits 20 after SDL/audio cleanup. Stored paths, including `/../../`, are preserved
in Onion's double-quoted command. The launcher checks its active app command,
publishes history/command on destination filesystems, rolls back owned changes and
sets quick_switch last. Browser MENU stages the existing GameSwitcher request,
exits 21 after cleanup and registers no selected favorite. Menu MENU only closes
the menu; B backs one level, or exits from the browser normally.

Onion owns cores, execution, save/resume, activity tracking and GameSwitcher.
MainUI is absent in the verified Apps context (`launch.sh → exe → runtime.sh`);
no MainUI stop/kill handoff remains. No app/helper stays alive during gameplay.

Automatic return defaults OFF. Y opens Settings; SELECT also offers Settings.
Availability is independent of the preference and requires the active runtime
context. Installation does not enable it; disabling rotates generation and
invalidates ownership. Optional runtime integration retains session ownership
through GameSwitcher switches, consumes it before reopening, and clears it on
direct exit, MainUI return, runtime restart, non-game launch, disable/generation
change and shutdown. Stock launches do not adopt ownership; normal app B cannot
create a reopening loop.

Browser state records launch/ROM identity, nearby fallback ordinal and viewport.
Removing a favorite rebuilds rows and selects the nearest remaining entry. Removal
backs up/verifies and atomically publishes one exact source span; ROM/art/saves/
histories and unrelated fields/records stay intact. The final snapshot-check/rename
race with a noncooperating writer is documented, not claimed to be eliminated.

## Version reference

Mounted Onion **v4.3.1-1** is the version-specific authority. Original runtime
SHA-256: `a8d77dcd316bc2a323b1e015aaf4b7682d2fed677af9cdadbc00e48881425d6e`;
Git blob: `4e2194f1b47c6c13605846002b6be0b42c1384f9`.
The optional runtime patch is separate, disabled by default and hash-gated;
[installation/rollback/lifecycle/memory procedure](onion-return.md).
Original backup/manifest remain in `.tmp_update/config/better-favorites-return-backup/`.
This UI checkpoint changes no launcher/runtime/helper protocol.

## Source references

- [Runtime: main loop, launch_game, check_switcher, launch_switcher](https://github.com/OnionUI/Onion/blob/v4.3.1-1/static/build/.tmp_update/runtime.sh)
- [AdvanceMENU return launcher](https://github.com/OnionUI/Onion/blob/v4.3.1-1/static/build/.tmp_update/bin/adv/run_advmenu.sh)
- [GameSwitcher: buttons, signals, exit branch](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/gameSwitcher/gameSwitcher.c)
- [resumeGame and MainUI state](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/system/state.h)
- [MENU termination/resume actions](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/keymon/menuButtonAction.h)
- [Mounted RandomGamePicker reference](/Volumes/MIYOO/App/RandomGamePicker/random.sh)

## Agreed roadmap and next unfinished feature

SELECT actions, full-screen Settings/Help and guarded single-favorite removal are
complete in this checkpoint. The remaining explicitly planned feature in
[README — Planned behavior](../README.md#planned-behavior) is launching Better
Favorites from the normal Favorites tile on Onion's Home screen.

That entry-point integration is a separate next step. Reuse this browser, Onion's
existing favorites file/Add to Favorites workflow and verified launch/return
handoff. Investigate the stock Favorites tile dispatch first: MainUI internals
and a supported tile-level replacement hook are not yet verified. Do not assume
that an app launcher or per-game Favorites GLO hook replaces the tile.
