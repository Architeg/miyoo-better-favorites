# Optional Onion return integration

Installed session-return checkpoint, 2026-10-02. The user reports hardware-confirmed
OFF/ON return behavior, GameSwitcher game switching, A resume, and restoration of
selected favorite/scroll position. B/START share the source-verified return path;
START and remaining lifecycle edge cases have host coverage, with separate device
confirmation still pending. No device RAM measurement has been made.

The runtime patch is separate. **Automatic return defaults off in the app.**
Installation and removal preserve the app preference; installation does not activate
the feature. An already-enabled preference stays enabled after installation.

## Files and protocol

- `integration/onion-return/runtime.patch`: hook calls added to the exact
  [v4.3.1-1 runtime](https://github.com/OnionUI/Onion/blob/v4.3.1-1/static/build/.tmp_update/runtime.sh).
- `integration/onion-return/better_favorites_return.sh`: sourced runtime helper.
- `integration/onion-return/manage.py`: host installer and uninstaller; neither changes app settings.
- `integration/onion-return/hashes.json`: original and installed file hashes.
- `src/launch_request.cpp`: transactionally publishes a return ticket only when
  the runtime exports a fresh `BETTER_FAVORITES_RETURN_DIR` and the app setting is on.
- `src/browser_state.cpp`, `include/browser_state.h`, `src/main.cpp`: app-owned
  browser-state serialization and restoration; launcher sets its file location.

The runtime creates a mode-0700 `/tmp/better-favorites-return.XXXXXX` directory
for each recognized BetterFavoritesTest invocation. After binary exit 20 and SDL/
 audio cleanup, the existing handoff helper publishes an exact copy of the game
command as `request.sh` and the settings generation as `generation` in that
directory, before setting quick_switch. Failure
rolls the ticket back along with this invocation's command/history changes.

After the app returns successfully, runtime verifies ticket/active-command
agreement, generation agreement, enabled preference, and quick_switch; resolves
the ROM and accepts origin in runtime memory,
and removes its private ticket/context. Old directories are never adopted after
restart. A killed runtime may leave an inert directory until reboot; there is no
wildcard cleanup of other invocations.

When GameSwitcher returns successfully with no active command, runtime consumes
origin **before** creating a same-filesystem temporary app command and renaming
it into `.tmp_update/cmd_to_run.sh`. No extra quick_switch flag is needed in that
branch: the next existing runtime iteration runs the app. The hook does not run
an emulator or retain the app launcher in memory.

## App preference and ownership invalidation

Y opens full-screen Settings; SELECT opens actions, including Settings and Help.
Non-repeated A on Automatic return toggles it; B backs one level and MENU closes
all open menu pages without opening GameSwitcher. Integration availability is shown separately from
the preference and requires the live private runtime context, not just a helper
file on disk or an ON preference. Installed but not active in this invocation
shows unavailable. Browsing layout/navigation and sound remain unchanged.
`App/BetterFavoritesTest/settings.conf` is an app-owned, versioned 60-byte record
with an on/off value and a random 32-hex generation. Missing/invalid settings
mean off. Successful changes use a destination-directory temporary file, fsync,
and atomic replacement; failure logs an error and keeps the old UI preference.
Every successful change rotates the generation, including disabling.

The runtime rechecks the preference/generation at each lifecycle hook. Disabling
invalidates accepted ownership at the next hook, before any reopening decision.
A disable/re-enable between hooks also invalidates it through generation mismatch;
old tickets cannot revive the session. With no poller during gameplay, runtime
memory is cleared at that boundary rather than asynchronously. App reopening
already consumes ownership before the user can disable it. A new launch while off
still uses ordinary Onion handoff/history but does not publish an origin ticket.
Installation alone neither creates settings nor opts the user in.

## Lifecycle

| Event | Behavior |
| --- | --- |
| A/MENU resumes the game | Existing Onion command/resume path; session ownership retained. |
| B or START exits GameSwitcher | Reopen Better Favorites with saved selection/viewport. |
| GameSwitcher resumes/switches to another ROM | Session ownership retained; B/START still reopen Better Favorites. |
| Direct game exit to menu | Ownership cleared; ordinary MainUI return. |
| Same-game restart | Ownership retained within the current runtime session. |
| Runtime restart/reboot | Origin reset; Onion's existing auto-resume remains unchanged. |
| App B/normal exit, failed launch | No accepted ticket; no reopening loop. |
| Shutdown or failed GameSwitcher | Reopening skipped; ownership consumed. |
| Invalid/stale ticket or unrelated pending command/flag | No adoption/return; unrelated files preserved. |

`browser-state` in the app directory uses a versioned, bounded, length-prefixed
format: original launch/ROM strings, selectable ordinal, first row, and top-row
identity. This stores raw strings without shell evaluation. Successful A staging
is preceded by atomic state saving; a save failure keeps browsing. A rejected
launch can still update the saved browser position but never queues a game.
On reopening, identity wins over indices; a removed favorite falls back to the
clamped selectable ordinal. A missing top anchor falls back to the old first
row, clamped safely. Existing pixel-height/sticky-header logic keeps selection
visible. Corrupt/missing state uses defaults. The state is not history.

## Installation and rollback

Use macOS/Linux with Python 3, `patch`, and `sh`. Power the Miyoo off and mount its
card on the host. Never run this installer against a live device/runtime.

First separately back up and verify the app binary and launcher, then deploy the
rebuilt app files through the normal reviewed deployment procedure. This manager
only manages the runtime integration, not app deployment, libraries, or history.

From repository root:

```sh
python3 integration/onion-return/manage.py install --sd-root /Volumes/MIYOO
sync
```

Install requires the exact version string `v4.3.1-1`, original SHA-256, and Git
blob SHA **4e2194f1b47c6c13605846002b6be0b42c1384f9**. It applies the patch in a
host temporary directory, verifies the result/helper against package hashes,
and checks shell syntax before card publication. It refuses existing helper,
or backup paths and symlinks at managed locations.

The original runtime and manifest are retained under
`.tmp_update/config/better-favorites-return-backup/`. Backup bytes are verified
and the backup file fsynced before replacement. Helper publishes first, runtime
last, using destination-directory temporary files. Failed installation restores
only its own published bytes; a foreign replacement is preserved and reported.
Reboot to load installed runtime changes. With the default-off preference, first
verify ordinary launches; then use **SELECT → Settings → Automatic return → A**
to enable it. B closes Settings. No installer activation command or shared enable
flag exists.

Disable in the app through the same Settings panel. To uninstall, use an offline card:

```sh
python3 integration/onion-return/manage.py uninstall --sd-root /Volumes/MIYOO
sync
```

Uninstall verifies original backup and installed file hashes, restores/verifies
the original runtime first, then removes only the matching helper file. App settings and browser state remain untouched.
Backups remain. Cleanup is retryable if completion fails after restoration.
Changed files cause refusal instead of overwriting unrelated edits. An existing
backup also blocks reinstall: inspect and archive it manually before a new
installation. For manual recovery, restore the verified original runtime from
that backup only after checking that the destination has no unrelated edits.

## State, disk sizes, and device memory measurement

Host-built sizes for this revision (logical bytes, not filesystem allocation or RAM):

| Item | Bytes / lifetime |
| --- | --- |
| ARM app executable, including debug info | 206,868; exits before gameplay |
| Binary increase over registered-launch checkpoint | 32,012 (previous binary 174,856) |
| Binary increase over preceding return-only build | 13,944 (previous binary 192,924) |
| Original / patched Onion runtime | 24,440 / 24,939; +499 |
| Sourced runtime helper | 13,955; combined runtime/helper disk increase 14,454 |
| Separate runtime patch / host installer | 670 / 7,268; host review/install artifacts |
| App settings | 60 once written; no file required for default off |
| Browser state | Path-dependent, at most 65,536; persists on card |
| Private ticket | Exact game command plus 33-byte generation; removed after app return |

During gameplay the added persistent runtime data is the canonical originating ROM
string or GameSwitcher-origin marker (session provenance, not a current-game restriction), its 32-character
generation, and an empty invocation-context variable.
When disabled these strings are empty. Helper function definitions remain loaded
in Onion's existing runtime shell. Scratch strings are unset on hook completion;
there is no polling timer, resident app, launcher, or separate helper process.
Transient shell utilities execute only at lifecycle boundaries. The installer is
a host tool. Private directories can remain inert after runtime SIGKILL until
reboot; they are never adopted. Installation also retains a verified 24,440-byte
original runtime and manifest on card. These disk sizes do not measure shell
allocator overhead, function definitions, page mappings, or RAM savings.

**Limited pre-cache idle measurements exist:** three valid ON samples give
browser smaps RSS 19,308 kB / median PSS 17,449 kB and runtime RSS 1,808 kB /
PSS 357 kB. They do not measure return-integration overhead or gameplay memory.
M5 is closed with gameplay/OFF-ON/process-absence work explicitly deferred; see
[results and limitations](m5-profiling.md). No RAM savings or cache build's device
acceptance is claimed. The procedure below is retained reference, **inactive and
not requested** in this pass; these deferrals do not block M6 investigation.

1. Compare three clean boots: original runtime, patched runtime with app setting
   off, and patched runtime with setting on and a Better Favorites-owned game.
   Use the same game/core, save state, scene, services, and warm-up interval.
2. Identify the actual runtime shell PID from the game's `/proc/<pid>/status`
   `PPid` chain and `/proc/<parent>/comm`; inspect only the relevant `cmdline`
   if needed. Do not assume the app launcher is still alive or choose the first
   arbitrary PID. Record the ancestry and a `ps` snapshot.
3. After the app has exited and the game has settled, collect:
   `cat /proc/<runtime-pid>/status`, `/proc/<runtime-pid>/smaps_rollup` if
   supported (otherwise `smaps`), and `/proc/meminfo`. Record VmRSS/VmSize,
   Pss and Private/Shared mappings where available. `free` alone cannot isolate
   runtime cost because caches and shared pages vary.
4. Confirm `pidof better-favorites` is empty during gameplay and process ancestry
   has no Better Favorites launcher/helper left waiting. Repeat after A resume
   and after toggling the setting off before another game launch. Any unexpected
   resident process is a failure to investigate.
5. Take multiple equivalent snapshots and report median/range plus kernel,
   device, core, and build identifiers. Keep observation commands short-lived;
   save samples to a dedicated test-log directory, without touching histories.
   Account for observer processes when comparing whole-device totals. RSS is not
   additive across shared mappings; prefer Pss when available. Report unavailable
   metrics explicitly, and keep disk-byte counts separate from measured RAM.

## Checks and remaining limits

Host checks:

```sh
c++ -std=c++17 -Wall -Wextra -Iinclude tests/app_settings_test.cpp src/app_settings.cpp -o /tmp/better-favorites-settings-test
/tmp/better-favorites-settings-test
c++ -std=c++17 -Wall -Wextra -Iinclude tests/browser_state_test.cpp src/browser_state.cpp src/ui_rows.cpp -o /tmp/better-favorites-browser-state-test
/tmp/better-favorites-browser-state-test
c++ -std=c++17 -Wall -Wextra -DBETTER_FAVORITES_HANDOFF_TESTING -Iinclude tests/launch_request_test.cpp src/launch_request.cpp src/app_settings.cpp -o /tmp/better-favorites-launch-request-test
/tmp/better-favorites-launch-request-test
python3 tests/runtime_return_test.py
TMPDIR=/tmp python3 tests/runtime_return_test.py
python3 tests/runtime_installer_test.py --reference /Volumes/MIYOO/.tmp_update/config/better-favorites-return-backup/runtime.sh
sh -n App/BetterFavoritesTest/launch.sh
sh -n integration/onion-return/better_favorites_return.sh
```

The installer tests use temporary card fixtures; their runtime reference is
read-only and must be the original version (use runtime.sh before installation
or the verified runtime backup afterward). Docker build:

```sh
docker run --rm -v "$PWD":/root/workspace/miyoo-better-favorites \
  -w /root/workspace/miyoo-better-favorites aemiii91/miyoomini-toolchain:latest \
  /bin/bash -c 'source /root/setup-env.sh && make -B all'
```

GameSwitcher, emulator scripts, recent registration format, save/resume, activity
tracking, UI geometry, and OSS/libpadsp audio are unchanged. Stock launches never
acquire origin, including a later stock launch of the same ROM.

GameSwitcher INT/TERM also return success and delete the active command in this
version; they therefore behave like B/START unless shutdown is requested. This
runtime-only integration cannot distinguish those input reasons.

Publication relies on Onion's serialized runtime owning the command slot while
GameSwitcher/app returns. `mv -i`, content/inode checks, and installer hash checks
are conflict guards, **not atomic compare-and-rename/unlink** against independent
writers. Install only on an offline card. SIGKILL/power loss cannot make the
multi-file handoff atomic. The hook checks shutdown before and after publication;
Onion's existing shutdown check still takes precedence if it arrives afterward.

This is specifically for the inspected runtime, app command, and v4.3.1-1 source.
An Onion update/custom runtime requires renewed review. Device tests must verify
BusyBox/filesystem behavior, origin propagation, populated GameSwitcher history,
A/B/START, game switching, direct exit, restart, rollback, and stock-menu use.
Compilation and host fixtures are not hardware verification.

## Installed checkpoint (2026-10-02)

App backup, both files verified before replacement:
`../miyoo-better-favorites-backups/20261002-030930/`.
Original runtime and verified manifest:
`/Volumes/MIYOO/.tmp_update/config/better-favorites-return-backup/`.
The installed helper and manifest now match the current package after the verified
session-return update. App copies match their
rebuilt repository sources. Executable permissions, shell syntax, and sync
passed. App settings remain absent (default off). Libraries, favourites and
visible/hidden recent lists matched pre-installation snapshots. Hardware results
are recorded above; device RAM measurements remain pending.

## Session ownership and lifecycle diagnostics

The initial canonical ROM is retained only as provenance for a session accepted
from the app ticket. `resolved-game` retains ownership for **any** game in that
session; it clears for a non-game. Switching/resuming uses Onion's existing
commands without altering GameSwitcher, emulator execution, histories, cores,
save/resume or activity tracking. No stock-menu launch acquires ownership, even
when it launches the same ROM. GameSwitcher B/START consume ownership and reopen
the app at its saved position; changing games does not change browser-state.
Normal app B exit cannot reopen it again. Direct game exit, ordinary MainUI
return, runtime restart, disable/generation mismatch, and shutdown retain their
existing invalidation/precedence rules. The MENU extension below was confirmed working on hardware by the user on 2026-10-02.

Boundary-only diagnostics append to
`.tmp_update/logs/better-favorites-return.log`, independently of Onion's global
logging flag. Logging failure is best effort and cannot gate game execution or
return. Each record has runtime PID, hook, origin ROM, owned/settings generation,
and action/reason. Boundary records include arguments (resolved ROM or command,
exit status/is_game), active/pending/quick-switch/switcher/shutdown file states.
Adoption records add ticket generation, command/app agreement, and resolved ROM;
clear/retain/skip/reopen records explain the decision. No polling process or
per-frame logging is added. Clear reasons precede clearing, retaining the old
identity for diagnosis. State observations are not atomic filesystem snapshots.
The append-only diagnostic file should be collected/archived between device runs;
no automatic deletion/rotation is added to this diagnostic revision.

For a helper-only revision, only the helper needs code deployment to
`.tmp_update/script/better_favorites_return.sh`; `runtime.sh`, app binary and
launcher are unchanged. Also update the verified install manifest's
`helper_sha256` under `config/better-favorites-return-backup/`, without changing
the original backup or original/patched runtime hashes, so this package's safe
uninstaller remains compatible. Prepare the manifest from the verified installed record, changing only the
helper hash. Before a future reviewed update, verify current runtime/helper/manifest and back up the existing helper
and manifest, publish the helper using a destination-directory temporary file,
then update its manifest hash and verify both. On failure restore only this
update's own files. Preserve the original runtime backup, settings, libraries,
favorites and history. The reviewed update was installed and verified on
2026-10-02; its helper/manifest backup is
`../miyoo-better-favorites-backups/20261002-033020-return-helper/`.
`manage.py install`
correctly refuses an existing installation; it is not an upgrade command.

## Browser MENU extension (hardware-verified checkpoint)

Verified mounted v4.3.1-1 reference:
`App/PackageManager/data/App/GameSwitcher (Shortcut)/App/StartGameSwitcher/launch.sh`
only touches `.tmp_update/.runGameSwitcher`. Runtime `main` runs `check_switcher`
after `check_game`; `check_switcher` prioritizes that flag, and `launch_switcher`
changes to `.tmp_update`, starts Onion audio, runs `gameSwitcher` with libpadsp,
then invokes the existing after-switcher hook. App postprocessing already invokes
the after-app hook. No new runtime hook placement is needed.

[Matching GameSwitcher source](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/gameSwitcher/gameSwitcher.c)
shows B/START remove the active command. A/MENU call
[resumeGame](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/system/state.h),
which only publishes when it finds a matching valid history entry. With empty
history, A/MENU can publish nothing (`gameIndex` is zero). A leftover app command
must therefore be removed before GameSwitcher runs to prevent replaying it.
Onion itself can deduplicate history or move a resumed game to its front; this
extension adds no recent-list entry just for pressing MENU.

One non-repeated MENU/ESC press outside Settings saves selection/viewport and
privately stages `switcher.request`. An empty favorites list has no position to
save and leaves any previous state file intact. The binary returns **21** only
after all SDL/audio cleanup. A remains exit 20; B remains ordinary app exit.
Inside Settings and other menu pages, MENU only closes the menu.

The outer launcher dispatches exit 21 to `--publish-switcher-handoff`. It verifies
its captured active app command, runtime binary, and absent pending/quick-switch,
shutdown and GameSwitcher flags. It prepares an app-command recovery file on the
command filesystem, removes only the captured command after an identity/content
recheck, and creates `.runGameSwitcher` last with O_EXCL, fsync and close checks.
The flag contains an invocation-unique token; Onion only tests its presence. A
failed handoff removes only its own ticket/flag and restores the app command only
into an absent slot, preserving foreign files. Snapshot/unlink/restore checks
still rely on Onion's serialized app-return writer; they are not filesystem CAS.
Stage/validation failures keep browsing. Publication failure after cleanup exits
to Onion normally, with error logging and no accepted return session.

With Automatic return ON, the MENU-capable runtime helper exports
`BETTER_FAVORITES_SWITCHER_HANDOFF=1`. The publisher places a matching token and
settings generation in the private runtime context. After successful app return,
the helper adopts `BetterFavorites:GameSwitcher` as session provenance only when
ticket, flag, generation, app identity and absent command/queue agree and shutdown
is absent. This supports B/START return from GameSwitcher immediately or after
selecting/switching games. Empty-history A/MENU also return once while owned;
origin is consumed before reopening, so ordinary B cannot reopen in a loop.
With the preference OFF no session is adopted and Onion returns to its stock
menus. With ON and an older/no runtime helper, MENU fails closed while browsing
instead of promising an unavailable return path. A launch is unchanged.

Required future deployment: revised binary, repository launcher, return helper,
and the matching installed-manifest helper hash. **No runtime.sh replacement** is
needed: its patched hash/hook placement and original backup remain unchanged.
Do not use install over an existing installation; follow the verified helper/hash
update procedure and back up app/helper/manifest before an authorized deployment.

Additional checks:

```sh
c++ -std=c++17 -Wall -Wextra -DBETTER_FAVORITES_HANDOFF_TESTING -Iinclude tests/switcher_request_test.cpp src/launch_request.cpp src/app_settings.cpp -o /tmp/better-favorites-switcher-test
/tmp/better-favorites-switcher-test
python3 tests/launcher_handoff_test.py
```

Runtime fixtures cover MENU ownership with empty history, B/START, game switching,
OFF, generation/flag mismatch, shutdown, failure and reopened-app B. Handoff tests
cover cancellation, both history files unchanged, foreign commands/flags, blocked
publication and rollback. The actual input mapping, MENU cleanup/launch timing,
Onion audio transition and empty-history screen remain device test requirements.

Final local MENU checks passed: focused C++ handoff, existing A launch/history,
settings/persistence, lifecycle fixtures under /tmp and macOS temporary roots,
real-launcher dispatch with isolated child stubs, installer fault fixtures, shell
syntax, Python parsing and whitespace. Docker rebuilt an ARM EABI5 executable;
the existing SDL2_ttf `libbz2.so.1.0` linker warning remains. These checks do not
establish device MENU/audio timing or measured RAM use. Installed card files still
match checkpoint hashes; no deployed files were changed.

On 2026-10-02, after verified deployment of the binary, launcher, helper and
manifest, the user confirmed MENU works as expected on hardware. Failure and
shutdown edge cases remain fixture checks; no RAM measurement is claimed.
