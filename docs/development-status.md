# Development status

Updated: 2026-10-02. Native SDL2 frontend for Onion's existing
`/mnt/SDCARD/Roms/favourite.json`.

## Hardware-confirmed checkpoint

The user confirms the following device results for the installed session-return
checkpoint (these are hardware reports, separate from the host checks):

- UI/navigation and OSS/libpadsp audio work.
- Selected games launch through Onion with GameSwitcher registration.
- Automatic return OFF preserves Onion's ordinary return to stock menus.
- Automatic return ON reopens Better Favorites from GameSwitcher.
- Switching games through GameSwitcher retains the originating session and return.
- A resumes the running game.
- Reopening restores the selected favorite and previous browser scroll position.

START, direct exit/restart/shutdown edge cases, removed-favorite fallback, and
stock-menu isolation have focused host coverage; independent device confirmation
of every edge case is still pending. No device RAM measurements are recorded.

Earlier diagnostics verified MainUI is absent during app launch:
`launch.sh → exe → runtime.sh`; the active command is
`.tmp_update/cmd_to_run.sh`, not `/tmp/cmd_to_run.sh`.

## Git checkpoint and checks

- Branch: `main`.
- Previous published base: `dd708a163739fead9095dcbf46b9b44b764a9a8b` —
  `Launch favorites through Onion with GameSwitcher registration`.
- This checkpoint: `Add optional Onion session return with browser state restoration`.
- Focused settings, browser-state, launch/history/rollback, runtime lifecycle, and
  installer/uninstaller fixtures cover the feature. Runtime fixtures run with
  both `/tmp` and macOS temporary roots. Shell syntax and Docker ARM build are
  final gates and passed for this checkpoint; compilation is separate from the
  hardware results above. Installer fixtures used the cached original runtime
  after verifying its SHA-256 and source blob; the card was not mounted during
  the final review. Deployed files were not changed.
- Existing ARM linker warning: SDL2_ttf's `libbz2.so.1.0` is not found at link time.
- Build outputs, app settings/state, logs, backups, caches and temporary audit/diff
  files stay outside Git. Deployment is a separate explicit action.

## Implementation and ownership

Non-repeated A validates the selected favorite and resolved files while preserving
stored paths, including `/../../`, and Onion's safe double-quoted command format.
The app saves browser position, privately stages command/recent data, and exits
20 after SDL/audio cleanup. The launcher publishes command/history on destination
filesystems, checks ownership, rolls back failures, and sets quick_switch last.
Normal B exit does not register history. Onion owns cores, execution, save/resume,
activity tracking and GameSwitcher. There is no MainUI stop/kill handoff.

Automatic return defaults OFF in app-owned settings. SELECT opens Settings;
A toggles, B closes. Integration availability is displayed separately from the
preference. Installation/uninstallation does not change the preference. Changes
rotate a generation: disable/re-enable cannot revive old ownership.

The optional runtime hook accepts an invocation ticket after app return and owns
the session originating in Better Favorites, rather than one ROM. GameSwitcher
resumes/switches retain ownership; B/START consume it before reopening the app.
Direct game exit, ordinary MainUI return, runtime restart/reboot, non-game launch,
and preference disable/generation mismatch invalidate it. Shutdown has priority.
Normal app B exit cannot create a reopening loop; stock launches never adopt a
session. No app, launcher or separate helper stays alive during gameplay.

Browser state uses selected launch/ROM identity, fallback ordinal, and viewport
identity. Missing favorites select a nearby clamped entry; corrupt state uses
defaults. This is app state, not a separate history system. Lifecycle diagnostics
in `.tmp_update/logs/better-favorites-return.log` report ownership and decisions;
logging failure does not alter the handoff. Browser MENU extension is hardware verified; see below.

## Version and deployment

Version-specific authority: mounted Onion **v4.3.1-1**, matching original runtime
Git blob `4e2194f1b47c6c13605846002b6be0b42c1384f9`. The runtime patch is separate
from app deployment and hash-gated; see [installation, rollback, lifecycle and
memory measurement](onion-return.md). MainUI internals remain unverified.

- App backup: `../miyoo-better-favorites-backups/20261002-030930`.
- Reviewed session helper/manifest update backup:
  `../miyoo-better-favorites-backups/20261002-033020-return-helper`.
- Verified original runtime and manifest:
  `/Volumes/MIYOO/.tmp_update/config/better-favorites-return-backup/`.
- App deployment and subsequent helper/manifest update were verified by bytes,
  hashes and shell syntax, then synced. Libraries, favorites, histories, original
  runtime backup and app setting were preserved during the helper update.
- The installed helper/manifest correspond to this checkpoint. Automatic return
  was ON at the last read-only inspection; this personal preference is not tracked.

## Source references

- [Runtime: main loop, launch_game, check_switcher, launch_switcher](https://github.com/OnionUI/Onion/blob/v4.3.1-1/static/build/.tmp_update/runtime.sh)
- [AdvanceMENU return launcher](https://github.com/OnionUI/Onion/blob/v4.3.1-1/static/build/.tmp_update/bin/adv/run_advmenu.sh)
- [GameSwitcher: buttons, signals, exit branch](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/gameSwitcher/gameSwitcher.c)
- [resumeGame and MainUI state](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/system/state.h)
- [MENU termination/resume actions](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/keymon/menuButtonAction.h)
- [Mounted RandomGamePicker reference](/Volumes/MIYOO/App/RandomGamePicker/random.sh)

## Roadmap and exact next step

Next: implement SELECT actions, full-screen Settings/Help, and guarded removal of
one favorite. Preserve browser geometry and verified Onion launch/return/audio.
Inspect Onion removal semantics first; no deployment of this next step yet.

## Hardware-verified MENU checkpoint

The previous automatic-return checkpoint is `cd55c018cea2a273eef9f1feacf0cb8d27866a97`.
MENU saves browser state, stages a private request and exits 21 after cleanup.
The launcher removes only its captured active app command and publishes Onion's
`.runGameSwitcher` flag. No selected favorite/history registration occurs. ON
adopts a GameSwitcher-origin session; OFF uses Onion's normal app/menu return.
The helper and manifest were updated alongside the binary/launcher; runtime hook
placement is unchanged. Settings MENU closes the overlay; browser B exits.

On 2026-10-02 the user confirmed the deployed MENU revision works as expected on
hardware. This complements the earlier hardware-confirmed A launch, OFF/ON return,
switching games, A resume and restored selection/viewport. Failure, shutdown and
empty-history cases remain fixture-verified unless separately observed on device.

Deployment backup: `../miyoo-better-favorites-backups/20261002-180751-menu`.
Focused handoff/lifecycle, launcher, settings/state and installer checks passed.
Shell syntax and Docker ARM build passed, with the existing libbz2 linker warning.
No device RAM measurement has been made.
