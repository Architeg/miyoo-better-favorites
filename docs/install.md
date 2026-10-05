# Install or update Better Favorites

[README](../README.md) · [Uninstall](uninstall.md) · [Compatibility](compatibility.md)

## One ready-to-install download

`better-favorites-<version>.zip` contains the app and offline computer tools. GitHub Source code and dependency/source companions are for developers. There is no separate app-only download. RC7 retains corrected Intel detection, bounded installer diagnostics and unchanged-tool publication avoidance, with focused receipt/Settings corrections. Earlier package bytes and acceptance remain recorded separately.

## Recommended: copy, then click

1. Extract the ready-to-install ZIP on your computer.
2. Power off the Miyoo, remove its card and connect it to the computer.
3. Copy `App/BetterFavorites` into the card's `App` folder.
4. Open that copied folder and the launcher for your computer:

| Computer | Open |
| --- | --- |
| Windows | `Install-Windows.cmd` (double-click in Explorer) |
| macOS | `Install-macOS.command` (open in Finder) |
| Linux | `Install-Linux.desktop` (Allow launching if your desktop asks) |

5. The launcher opens a terminal automatically. Choose **[1] Install / Update**, confirm the displayed card and that the Miyoo is powered off. Progress reports actual phases; keep the card connected until completion.
6. Wait for verified success, safely eject, insert the card and boot.

Both supported integrations are installed by default; their app switches remain **OFF** on a fresh install. Updates preserve switches, preferences and browser position. OFF disables the installed behavior; it does not remove patches. Unsupported versions and conflicting system files are never patched. The support-only explicit app-without-patches fallback is not a second download and cannot discard an existing integration's recovery.

Portable verified recovery lives in the card's hidden `.tmp_update/config` directory, outside the app. Keep it. Optional computer copies are additional protection, not an uninstall dependency. Installation and restoration take effect after reboot.

## First opening

See [Mac/Windows file-specific security approval](security-opening.md). Approving the Mac script does not approve its child. The launcher retains an identifiable, byte-verified host helper for approval and retry. No quarantine attributes or global protection settings are changed. Linux uses its desktop's **Allow launching/Trust** step; if unavailable, use the shell command below.

## First launch

Open **Apps → Better Favorites**. A one-time theme-aware notice explains actual integration availability and the two OFF switches. Press A/B or MENU to close it; Y later opens Settings.

- Enable **Replace stock Favorites** to open Better Favorites from the Home Favorites tile.
- Enable **Automatic return** to return to Better Favorites with B/START from GameSwitcher, including after switching games.

Turning a switch OFF disables its behavior. Complete uninstall restores the original system files. Apps access and stock X/Y shortcuts remain unchanged.

## Update without losing preferences

Copy the **contents** of the new `BetterFavorites` folder into the existing folder and replace supplied files. Keep files already there that the package does not supply. On macOS, do not choose whole-folder Replace; open both folders and copy their contents or use Merge.

Open the copied computer launcher and choose Install / Update. The package contains no personal preferences/state. Existing verified originals remain stock originals through updates; newly copied exact package files are recognized. Unknown or modified files are preserved and reported.

## Migrating BetterFavoritesTest

Copy the new `BetterFavorites` folder beside the old `BetterFavoritesTest`, then open the **new** launcher. A verified portable recovery lineage authenticates old files and stock originals. A never-installed exact RC3 copy is also recognized when every system file is stock and no integration is active. Install / Update migrates preferences, browser position and dismissed guidance, replaces both integration commands, then retires the old owned folder. It refuses unknown files or conflicting saved data instead of merging them. Keep recovery and use the matching development restoration procedure first if an older development install has no portable journal. Do not manually rename a patched installation.

## Export diagnostics

Open the same copied launcher and choose **[3] Export diagnostics**. The card is identified from that folder and a fresh archive is saved on your computer. Exports include at most two bounded installer log tails; review them before sharing. They do not enable tracing. [Contents and privacy](diagnostics.md).

## Advanced/support commands

Run these in the copied app folder if a support person asks you to. Normal installation requires no manually opened terminal.

| Platform | Install | Complete uninstall |
| --- | --- | --- |
| Windows | `.\Install-Windows.cmd install` | `.\Install-Windows.cmd uninstall` |
| macOS | `./Install-macOS.command install` | `./Install-macOS.command uninstall` |
| Linux | `sh ./Install-Linux.sh install` | `sh ./Install-Linux.sh uninstall` |

Use `export-diagnostics` instead of `install` to export logs on the computer. Fully specified `--sd-root`, `--powered-off`, `--package`, `--recovery` and `--archive` backend interfaces remain for automation. `remove-integrations` is an advanced operation, not the default uninstall.

The experimental download bootstrap is separate and is not needed by this offline package.

## Metadata created by Mac copying

The installer recognizes structurally valid AppleDouble sidecars associated with
verified project files, and Finder metadata in verified project directories.
These do not require manual cleanup, including when later uninstalling on Windows
or Linux. Actual payload checks remain unchanged. Unknown/modified files, links
and ambiguous metadata are preserved and reported as ordinary installation
errors. Keep that message and recovery archive; approval troubleshooting cannot
resolve an ownership or checksum failure.

## Mac metadata and first opening

Read [Mac first open](../packaging/Mac-first-open.html) before launching the unsigned script. Script and helper approvals are separate. Valid bounded AppleDouble metadata beside inventory-owned files **or directories**, and valid Finder metadata within owned directories, are supported across computers. Unknown or modified content and symlinks remain protected. Do not manually remove metadata to bypass a failed check. An incomplete operation reports its recorded change state and a computer-side detailed log; keep recovery backups.

Valid welcome-marker metadata remains recognized after the app dismisses first-run guidance, when verified package/recovery evidence authenticates the consumed marker. Unknown orphan sidecars remain protected. Updates also check that authenticated older backup artifacts can be archived and removed by complete uninstall before publishing changes. The Mac approval retry choices are R, S and 0; ordinary install/uninstall errors remain separate.

The latest preceding candidate passed user-confirmed M1 and Windows install/complete uninstall. Intel install succeeded after manual receipt removal; RC7’s recovery-backed correction remains device-pending. [Current evidence](release/rc.7.md).

Updates preserve existing preference files, including malformed saved data, without rewriting them. Their exact snapshots are recorded in the new recovery journal so complete uninstall can safely archive them. Unknown payload bytes remain protected.

## Receipt ownership

An installed Home receipt is generated state, not a stock file. Complete uninstall verifies its absence in both current and legacy app locations. A full reinstall can reconcile a stale receipt only with verified recovery and matching stock runtime/MainUI identities. Missing ownership evidence or foreign bytes are preserved and reported; there is no routine manual-deletion step. [RC7 evidence and limits](release/rc.7.md).
