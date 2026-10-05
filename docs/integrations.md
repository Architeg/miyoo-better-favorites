# Integration and installer reference

For contributors and support. Users should start with [install](install.md), [uninstall](uninstall.md) or [recovery](recovery.md).

## Independent installation and activation

`tools/release-installer/` prepares the Home and return patches by default on audited cards. They share one backend on every computer. App preferences start OFF and independently enable behavior; availability is verified separately. Updates preserve preferences. Installation/restoration takes effect after reboot.

The app always remains in Apps. There is no separate Home tile or global shortcut. Ordinary X/Y assignments, theme configuration and global audio settings are unchanged.

## Home activation

`integration/mainui-home/adapter.cpp` and `hooks.S` redirect only Home's Favorites activation through native AppAction. Disabled, malformed or unavailable configuration follows stock behavior. Native state saving and result propagation are preserved.

Scoped command-writer hooks privately stage and publish without replacing another pending command. AppAction result 3 or readback alone is not ownership proof. The installer accepts only catalogue hashes and verified backups; foreign changes stop restoration.

Detailed ELF/ABI/byte-map evidence is in the [historical prototype record](archive/m6-mainui-prototype.md). Use [current compatibility](compatibility.md), not old deployment commands, for support limits.

## Onion launch and session return

`src/launch_request.cpp` privately stages game/GameSwitcher requests. The binary saves browser state and finishes SDL/audio cleanup before exit 20 (game) or 21 (GameSwitcher). `App/BetterFavorites/launch.sh` then verifies its active command, registers recent history only for game launch, and publishes using Onion's RandomGamePicker/quick_switch mechanism.

The launch command retains original paths and Onion's quoting. No emulator is launched from the SDL event loop. Onion retains core/save/resume/activity/GameSwitcher ownership. B exits normally; MENU inside a menu only closes it.

`integration/onion-return/runtime.patch` and `better_favorites_return.sh` own a session originating in the app, not one ROM. With Automatic return ON, B/START after GameSwitcher consumes ownership before reopening; game switching retains it. A resumes. Direct game exit, ordinary MainUI return, runtime restart/reboot, preference disable/generation change invalidate ownership. Shutdown takes precedence. No app or helper stays alive during gameplay.

`settings.conf` retains the return generation protocol. Home preference and browser preferences use separate files. [Architecture and source map](development.md) · [Historical lifecycle and memory procedure](archive/onion-return.md).

## Computer installer interface

The no-argument entry opens the Install / Uninstall / Export diagnostics menu. Action-only calls prompt for the card and required confirmation:

| Platform | Install | Complete uninstall |
| --- | --- | --- |
| Windows | `.\Install-Windows.cmd install` | `.\Install-Windows.cmd uninstall` |
| macOS | `./Install-macOS.command install` | `./Install-macOS.command uninstall` |
| Linux | `sh ./Install-Linux.sh install` | `sh ./Install-Linux.sh uninstall` |

Run these from the copied app folder. All wrappers use the same backend and retain failure status. `export-diagnostics`, `status`, `trace-on`, `trace-off` and `remove-integrations` are support actions. `restore` additionally restores recorded app snapshots; use it only with matching recovery, not as a replacement for complete uninstall.

Fully specified automation uses `--sd-root`, `--package`, `--powered-off`, `--recovery`, `--archive` or `--output` as appropriate. `--home=false` / `--return=false` are support installation choices, not a second download. Explicit removal/restoration requires verified recovery; ambiguity is an error.

## Ownership and recovery

The backend verifies transport/payload bytes, exact system hashes and all backups before publication. Staging uses the destination filesystem. Multi-file replacement is not one atomic filesystem transaction; rollback restores only this invocation's unchanged published bytes. Offline exclusive card access is required.

Complete uninstall preflights stock restoration and cleanup, verifies a computer archive outside the card, restores/checks stock files, removes owned app/data and artifacts, and removes portable recovery last. A partial failure never reports complete uninstall. Foreign files, links/reparse points and ambiguous recovery are preserved.

Structurally validated AppleDouble companions are accepted only for inventory-owned files/directories; bounded Finder metadata is accepted only in owned directories. This applies across host OSes and interrupted retries. Actual payload validation is unchanged. Unknown/orphan/malformed metadata remains protected.

Home receipts are generated integration state with stock absence, never stock originals to restore. Stale receipt reconciliation requires authenticated recovery and matching stock system identities; names alone are insufficient. See `receipt_repair.go`, `metadata.go`, `metadata_cleanup.go`, `recovery_sequence_test.go` and `uninstall.go`.

## Legacy development installations

An authenticated `BetterFavoritesTest` installation can migrate when portable recovery proves ownership of old files and original system bytes. An exact never-installed RC3 copy is recognized only with stock systems and no active integration. Conflicting preferences or unknown files are refused.

An older development installation without portable journals needs its matching development uninstall procedure first. Do not rename patched directories or assume release recovery formats match development backups. [Historical Home installation evidence](archive/m6-home-integration.md).

## Diagnostics

App current/previous logs each cap at 65,536 bytes. Home/return trace writes each cap at 128 KiB; legacy oversized logs are retained without further growth. Export reads bounded tails. Logging is best-effort and never changes handoff authority or fallback behavior. No per-frame logger or watcher is used.

From the copied app folder, use your platform entry with `trace-on`, reproduce the specific issue, run `export-diagnostics`, then `trace-off`. Disabling removes the owned marker and retains evidence. Detailed tracing is independent of both feature preferences and normally OFF.

Loader failures before main may appear only as launcher status. Trace writes can bias timing. Mounted-card inspection cannot measure live RSS/PSS or prove gameplay process absence. [Historical measurement evidence](archive/m5-profiling.md).
