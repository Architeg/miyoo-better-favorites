> Release route: the packaged native Go tool now provides Install, Uninstall,
> Restore and Export diagnostics. See [quick start](install.md) and [recovery](uninstall.md).
> Historical development route: current release installation uses the native host
> tools. [Windows normal-flow acceptance](release/windows-acceptance.md) does not
> qualify every later dispatcher/version. The Python implementation
> below remains POSIX-only developer tooling. WSL is not a supported v1 user route.

# Optional Home Favorites integration

The exact deployed M6Home1 integration passed the user's four hardware checks on
Miyoo Mini Plus / Onion v4.3.1-1. See [actual hashes, logs and limits](m6-acceptance.md).
The checkpoint remains local/uncommitted. Later wording/status/recovery changes
are host-checked and undeployed. Other variants/devices, uninstall and hardware
fault recovery are not covered by that acceptance.

## Installation is separate from activation

Settings adds **Use Better Favorites on Home: ON/OFF** (or **Home Favorites** when the measured row width requires it), default OFF. Its independent
`App/BetterFavoritesTest/home-entry.conf` is exactly `BetterFavoritesHome1\n0\n`
or `BetterFavoritesHome1\n1\n`. Missing/malformed/unreadable/nonregular files use
OFF. Atomic save failures retain the previous in-memory setting. This file does
not change `settings.conf`, Automatic return's generation, browser preferences,
browser state, favorites or recent history. Repeated activation keys are ignored.

Home's existing Favorites tile, translated label, theme art and layout are kept.
Apps access and X/Y shortcuts are unchanged; no separate tile. OFF, unsupported
configuration, unavailable app, shutdown, incompatible native command and command
publication failure follow stock Favorites. Existing permanent Automatic return
integration and audio configuration are untouched. B exits Better Favorites through ordinary Onion app-return processing. It does
not invoke Automatic return: that setting owns game/GameSwitcher sessions only.
B restoring Home after a Home launch is now user hardware-confirmed. This uses
MainUI's native state save/restore and ordinary runtime app return; no custom Home
return hook or persistent app process is introduced. Apps-origin launches follow their ordinary Apps
return context. Restoring selection alone does not run the Home activation hook.

The ON/OFF value is a **saved preference**, never an installation claim. Settings
has separate status in **How to open Better Favorites**: Available / Not installed /
Unavailable on this system. The fixed selected description explains ON/OFF or the
missing/unsupported integration. Technical setup details remain in this document.
`home-integration.conf` must match the reviewed receipt; all four installed binaries
and the runtime are SHA-256 checked once each time Settings is entered, not each
frame or normal startup. That disk-read cost is unmeasured on device. It verifies
on-card files, not which image an already-running MainUI has mapped. **Binary
installation/removal requires reboot**; saved preference changes apply on the next
Home activation without repatching/reboot. Actual dispatch still checks app files.

## Developer POSIX installer / uninstaller (historical user route)

`integration/mainui-home/manage.py` accepts only four full original MainUI hashes
listed in `package.json`, the pinned compiled adapter hash, and Onion v4.3.1-1 with:

- Exact original runtime SHA-256, or
- Exact permanent-return patched runtime, helper, installed manifest and verified
  original runtime backup matching `integration/onion-return/hashes.json`.

No other MainUI patches or runtime revisions are supported. Nothing is written to
NAND. All four SD MainUI variants are patched so device/clean/expert selection stays
consistent. Runtime, return helper/manifest/backup and all app preferences are read
only. The card must be powered off, mounted through a qualified POSIX host, with no other card writer.
Hashes before rename are conflict checks, **not compare-and-swap** against writers.

Installer verifies **every original backup before any binary replacement**, stages
all outputs on their destination filesystem, fsyncs staged bytes, then uses atomic
rename and byte readback. An original backup and prepared recovery manifest are
kept in a fresh `.tmp_update/config/better-favorites-home-backup-<UTC>/` directory.
Active metadata is `.tmp_update/config/better-favorites-home.json`. The availability
receipt appears only after binaries; final installed status is last.

Handled failures restore only files still matching this invocation's published
bytes. Foreign changes are retained/reported; a recovery manifest remains when
rollback conflicts occur. Uninstall first checks **all** current installed hashes,
all verified originals, marker and manifest, then restores the originals. A
`prepared` journal permits interrupted-install recovery; new uninstall operations
write `uninstalling` before any restore. Explicit `recover` also handles an older
`installed` journal with mixed exact original/reviewed-patched files. It verifies
all originals, current bytes and receipt before writing; unfamiliar bytes are
refused and preserved. No manual journal editing is required. Host fixtures cover
interrupted uninstall with and without its new journal and full six-file recovery
without a working device UI. These are not physical power-loss tests. Backups/manifests
are retained. Multi-file publication is not one filesystem transaction; physical
power loss and FAT directory durability are not emulated. Run `sync` after actual
installation/removal and reboot before testing the new binary set.

Commands from the matching checkout (install is for a new eligible card only):

```sh
cd "$HOME/IT Projects/miyoo-better-favorites"
python3 integration/mainui-home/manage.py status --sd-root /Volumes/MIYOO
python3 integration/mainui-home/manage.py install --sd-root /Volumes/MIYOO \
  --payload build/m6-home-review-final/adapter.elf --powered-off
sync
```

The reviewed app binary/launcher must separately be backed up/deployed/verified;
this installer never replaces app files or saves a preference. Leave replacement
OFF initially, reboot, then activate it in Settings. Ordinary uninstall:

```sh
python3 integration/mainui-home/manage.py uninstall --sd-root /Volumes/MIYOO --powered-off
sync
```

If a conflict is reported, retain the card and backups; review the changed files
instead of deleting them or overriding the guard. Disable the saved preference in
the app before removal if desired; uninstall preserves it. ON with integration
absent is explicitly unavailable and cannot enable stock binaries.

## Temporary diagnostics, independently optional

The exact `home-diagnostics.conf` marker enables diagnostics independently of Home
replacement and Automatic return. Off/absent is the default. No polling process,
daemon, per-frame log or device Terminal command is needed.

MainUI writes `App/BetterFavoritesTest/home-diagnostics.log`, bounded to 128 KiB:

- `M6Home1`, exact MainUI variant, PID and attempt ID (`pid.seconds.microseconds`, hex).
- Actual ELF entry (`event=mainui-startup`) when that process starts.
- Actual Home Favorites activation and ON / OFF-or-malformed preference decision.
- Stock fallback reason, stage result, atomic publication result, native dispatch
  result/exception, and final redirect/fallback.

A fourth narrow ELF-entry hook supplies startup evidence without changing native
application actions. Diagnostics use nonblocking, no-follow opens, descriptor
regular-file checks, bounded buffers and single best-effort writes without fsync
or retry. Full/unwritable/FIFO logs suppress evidence and never alter dispatch.
Negative native syscall results are recorded in hexadecimal. Read/write latency
and instrumentation can affect timing; these logs are not a startup benchmark.
Even while disabled, marker checks add filesystem lookups at these boundaries.

The existing launcher log `better-favorites.log` records entry, PID/parent PID,
binary exit, cleanup and handoff outcome. With diagnostics ON it adds lifecycle
lines and the last committed Home attempt as **candidate_attempt**, which may be
stale on a later Apps launch. Runtime is MainUI's parent, so it cannot inherit a
new environment variable from MainUI. Candidate correlation never authenticates
ownership or changes the canonical App command. Native hook log records before
launcher entry explain stock fallback even when the launcher never runs. The next
observed MainUI startup has its own PID, not a retained app/launcher process.

Host-only enable, then after normal device interaction power off/remount and collect:

```sh
python3 integration/mainui-home/diagnostics.py enable --sd-root /Volumes/MIYOO --powered-off
# Unmount through your normal procedure, boot and perform the checklist below.
# Power off and remount; archive uses a NEW directory and refuses to reuse it.
archive="$HOME/IT Projects/miyoo-better-favorites-backups/m6-diagnostics-$(date +%Y%m%d-%H%M%S)"
python3 integration/mainui-home/diagnostics.py collect --sd-root /Volumes/MIYOO --output "$archive"
python3 integration/mainui-home/diagnostics.py disable --sd-root /Volumes/MIYOO --powered-off
sync
```

`remove` is an alias for removing only the recognized activation marker; it does
not remove collected/raw logs or binary instrumentation. Disable/removal needs no
binary reinstall or runtime edit; marker absence suppresses all diagnostic writes.
Unknown marker content is preserved/reported. Host archives retain both logs and
Home preference/receipt snapshots with verified hashes. The ordinary app log still
rotates on each app start; collect promptly. Home hook history stays capped and is
not silently truncated. To remove instrumentation itself use the verified MainUI
uninstaller; do not patch it out in-place.

## Focused evidence and retained device checklist

Host: bounded ARM FIFO tests for preference/config/binary/launcher; native ABI,
constructor/destructor/exceptions/state serialization; no-replace conflict cases;
optional startup/Home logging, enabled-while-OFF and log-full/FIFO/failure isolation;
exact ELF/byte maps for four variants; install/uninstall, pre-replacement backup
failure, late-publication rollback, interrupted-journal recovery, foreign-file
preservation, independent preferences/availability and permanent-return preservation.
App regressions, SDL offscreen layout checks (no preview files), shell syntax and
Docker ARM build pass. Existing GCC ABI notes and SDL_ttf libbz2 linker warning
remain. No new dependency or measured RAM claim.

Items 1–3 passed on the tested card as summarized in the acceptance record.
Remaining negative/fault combinations and uninstall still require qualification:

1. **OFF:** Home Favorites stays stock. Apps still opens Better Favorites. Verify
   Home setting persists and availability is separate. X/Y and theme tile unchanged.
2. **ON:** Home Favorites opens Better Favorites; B returns to Home at Favorites
   without reopening. Reopen/exit repeatedly; missing/malformed config stays stock.
3. **Game lifecycle:** A launches, recent/GameSwitcher stays populated; MENU opens
   GameSwitcher. Automatic return OFF/ON and switching games retain existing policy.
   B/START return and saved browser position work. No new resident helper is introduced
   by this design; live gameplay process-absence measurement remains deferred in M5.
4. **Diagnostics:** enable while replacement OFF and ON; collect native fallback /
   publication / launcher / subsequent startup evidence. Disable, verify no new
   Home log writes. Do not infer ownership from candidate correlation alone.
5. **Uninstall:** power off/remount, verified-original restore, sync/reboot. Stock
   Favorites restored, Apps and permanent Automatic return remain available; all
   preferences/favorites/history/saves/themes/audio untouched. Host conflict fault
   fixtures are not device power-loss tests.

## First device deployment record — 2026-10-03

Four reviewed MainUI variants and the matching app/launcher were backed up,
installed and verified byte-for-byte with executable permissions. ARM format,
launcher syntax, availability receipt and sync passed. At deployment: replacement **OFF**, diagnostics **ON**. After successful device
testing the user left replacement **ON**; collection disabled diagnostics without
changing that preference. No app/runtime files were replaced during collection. 28 protected paths, including libraries, saved settings/state,
favorites/history and permanent return files, matched their pre-deployment hashes.

App SHA-256: `d637643b0be987ab68a7bb9ba6869f09b6e1e58e2e7d0919088bbb1cd54f19dc`. This is the actual hardware-tested app, rather than the earlier review build.
A/MENU/return logic and the reviewed MainUI payload are unchanged.

Host six-file backup: `/Users/valeriybagrintsev/IT Projects/miyoo-better-favorites-backups/20261003-201736-m6-first-device`.
Additional verified MainUI originals: `.tmp_update/config/better-favorites-home-backup-20261003T171736.792512Z` on the card.

### Recovery even if MainUI does not open

Power off the Miyoo and mount its card on the Mac. This does not need a working
device menu or Terminal. The rollback tool verifies all six originals/current
hashes, archives available logs, disables diagnostics, uninstalls the redirect
and restores the former app/launcher, including known mixed-file recovery after
interrupted uninstall. It never requires MainUI or Miyoo Terminal. It refuses foreign changes, preserves
preferences/data/permanent return integration and retains all backups.

```sh
cd "$HOME/IT Projects/miyoo-better-favorites"
python3 tools/rollback-m6-device.py --sd-root /Volumes/MIYOO \
  --backup "$HOME/IT Projects/miyoo-better-favorites-backups/20261003-201736-m6-first-device" --powered-off
sync
```

Then reboot. Do not manually overwrite conflicting files or discard backup
manifests. Log collection, byte verification, diagnostic disable and sync are complete; see
the acceptance archive. No additional device tests or Terminal commands were requested.

## Host routes and Windows validation limits

Use Python 3.8+ on macOS or Linux with a POSIX filesystem interface. The Home tools
also pass fixtures on the existing Linux toolchain's Python 3.7.3; that old version
is not a recommendation to install an obsolete Python. The separate return manager
also requires POSIX file permissions plus `sh`/`patch`; its installation is optional
and does not provide Home dispatch. No native PowerShell/Python binary installer
is supported: Home tools refuse before filesystem access if POSIX safeguards are
missing. Do not bypass them.

Tested here: macOS Python 3.11.6 filesystem/backup/transaction/foreign-change checks;
Linux container filesystem checks and exact-binary install/uninstall/recovery;
bounded compiled ARM adapter fixtures for all four variants. The earlier real
installation verified macOS-mounted FAT bytes/permissions. **No Windows or WSL
host/SD reader is available here. Windows remains unqualified**, rather than being
inferred from the Linux container. Native-Windows refusal is a simulated host
fixture, not a Windows execution result.

The candidate Windows route is WSL with a Windows-mounted FAT drive exposed through
DrvFS. Do not use raw-disk commands, formatting, or `wsl --mount` for this workflow:
Microsoft documents that it does not support USB/flash/SD readers. DrvFS permissions
and `chmod` differ from native Linux; NTFS metadata documentation is not proof of
FAT behavior. [Microsoft mount limits](https://learn.microsoft.com/en-us/windows/wsl/wsl2-mount-disk),
[DrvFS permissions](https://learn.microsoft.com/en-us/windows/wsl/file-permissions).

Candidate commands **to qualify, not advertised as tested Windows support**:

```sh
# Inside WSL with the matching checkout; E: is only an example SD drive.
sudo mkdir -p /mnt/miyoo-card
sudo mount -t drvfs E: /mnt/miyoo-card
BF_SD='/mnt/miyoo-card'
# Device OFF, no competing writer. Creates/removes ONLY its new private fixture.
python3 tools/check-m6-filesystem.py --scratch-parent "$BF_SD" --allow-test-write
python3 integration/mainui-home/manage.py status --sd-root "$BF_SD"
```

Before qualifying Windows, run exact-package install/uninstall/failed-publication
and mixed-restore recovery fixtures on the actual WSL/reader/FAT setup, archive
results and verify backups/readback/executable permissions. The preflight tests
required descriptor flags, regular reads, fchmod/execute, fsync, same-filesystem
replacement and own rollback/foreign preservation. It does not prove multi-file
power-loss durability, all link semantics or a complete installation. Stop on
failure. Until qualified use macOS/Linux for binary recovery.

After a qualified install/uninstall, `sync`, leave the mount directory, unmount
WSL's view (`sudo umount /mnt/miyoo-card`) and safely eject in Windows before boot.
No ejection is performed by our tools or this session.

## Recovery without MainUI, network or Miyoo Terminal

Power off using the model's documented procedure; no universal force-off hold time
is assumed. Mount the card on a qualified computer. If only the redirect fails,
move aside the regular, non-link preference: missing means OFF, and old bytes remain
available. This does **not** repair a MainUI startup failure or uninstall hooks.

```sh
BF_SD='/Volumes/MIYOO' # Linux: set the actual mount instead.
BF_PREF="$BF_SD/App/BetterFavoritesTest/home-entry.conf"
if [ -f "$BF_PREF" ] && [ ! -L "$BF_PREF" ]; then
  mv -n "$BF_PREF" "$BF_PREF.disabled-$(date -u +%Y%m%dT%H%M%SZ)"
fi
sync
```

Native PowerShell can perform this narrow preference-only step; it is not a native
binary installer. This command is documented but not executed on Windows here:

```powershell
$BfSd = 'E:\' # Verify the real SD drive.
$BfPref = Join-Path $BfSd 'App\BetterFavoritesTest\home-entry.conf'
if (Test-Path -LiteralPath $BfPref -PathType Leaf) {
    $BfItem = Get-Item -LiteralPath $BfPref
    if (($BfItem.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
        throw 'Preference is a link; preserve it for inspection.'
    }
    $BfSaved = $BfPref + '.disabled-' + (Get-Date -Format 'yyyyMMdd-HHmmssfff')
    if (Test-Path -LiteralPath $BfSaved) { throw 'Backup destination exists.' }
    Move-Item -LiteralPath $BfPref -Destination $BfSaved -ErrorAction Stop
}
```

For startup failure or interrupted install/removal, use the matching Home package's
explicit verified-original restore; it retains app files, preferences and permanent
return integration:

```sh
python3 integration/mainui-home/manage.py recover --sd-root "$BF_SD" --powered-off
sync
```

Ordinary uninstall deliberately rejects unexpected/mixed installed bytes; `recover`
permits only exact original/patched mixtures and recognized absent/present receipts.
Both refuse unfamiliar changes or corrupt backups. `tools/rollback-m6-device.py`
above restores all six first-deployment files instead if a full rollback is wanted.
Keep the host backup, on-card Home backup and original return backup. Removing both
integrations requires Home uninstall first, then the separate return uninstaller;
never delete favorites/history/ROMs/saves as part of app removal. Installation and
binary restoration take effect **after reboot**, not merely after toggling a setting.
