# Install or update Better Favorites

[README](../README.md) · [Uninstall](uninstall.md) · [Compatibility](compatibility.md)

## Choose the download

- **Full ZIP:** app and clickable computer tools; prepares Home replacement and Automatic return when supported.
- **App-only ZIP:** browser for Apps, without system patches or computer installer.
- **GitHub Source code:** contributor sources, not a ready-to-install package.

No public download link is advertised until the asset exists. RC3 is a new candidate, not the tagged RC2 bytes.

## Recommended: copy, then click

1. Extract the full ZIP on your computer.
2. Power off the Miyoo, remove its card and connect it to the computer.
3. Copy `App/BetterFavoritesTest` into the card's `App` folder.
4. Open that copied folder and the launcher for your computer:

| Computer | Open |
| --- | --- |
| Windows | `Install-Windows.cmd` (double-click in Explorer) |
| macOS | `Install-macOS.command` (open in Finder) |
| Linux | `Install-Linux.desktop` (Allow launching if your desktop asks) |

5. The launcher opens a terminal automatically. Choose **Install / Update**, confirm the displayed card and that the Miyoo is powered off.
6. Wait for verified success, safely eject, insert the card and boot.

You do not enter a card path or select integrations. Both supported integrations are installed; their app switches remain **OFF** on a fresh install. Updates preserve switches, preferences and browser position. Unsupported Onion versions offer app-only installation explicitly. Corruption, modified audited files and transaction failures stop with an error; they do not silently become app-only success.

Portable verified recovery lives in the card's hidden `.tmp_update/config` directory, outside the app. Keep it. Optional computer copies are additional protection, not an uninstall dependency. Installation and restoration take effect after reboot.

## Opening unsigned files

Windows may show a file-specific security confirmation for downloaded tools. macOS may require Control-click → Open for that file. Do not disable SmartScreen or Gatekeeper globally. A downloaded `.command` may need executable permission if your extractor discards it; use an extractor preserving ZIP permissions. Linux launchers vary by desktop: approve **Allow launching/Trust** for this file. It opens a terminal and stages the native executable on the computer, so the SD volume may be mounted `noexec`.

If your desktop cannot launch `.desktop` files, use the explicit support command below. No claim is made that every Linux desktop launches identically.

## First launch

Open **Apps → Better Favorites**. A one-time theme-aware notice explains actual integration availability and the two OFF switches. Press A/B or MENU to close it; Y later opens Settings.

- Enable **Replace stock Favorites** to open Better Favorites from the Home Favorites tile.
- Enable **Automatic return** to return to Better Favorites with B/START from GameSwitcher, including after switching games.

Turning a switch OFF disables its behavior. Complete uninstall restores the original system files. Apps access and stock X/Y shortcuts remain unchanged.

## Update without losing preferences

Copy the **contents** of the new `BetterFavoritesTest` folder into the existing folder and replace supplied files. Keep files already there that the package does not supply. On macOS, do not choose whole-folder Replace; open both folders and copy their contents or use Merge.

Open the copied computer launcher and choose Install / Update. The package contains no personal preferences/state. Existing verified originals remain stock originals through updates; newly copied exact package files are recognized. Unknown or modified files are preserved and reported.

## App-only drag and drop

Copy the app-only package's `App/BetterFavoritesTest` folder into `App` while powered off. Safely eject and boot; open Apps. There are no system patches to enable unless previously installed. App-only does not overwrite or uninstall existing integrations.

## Advanced/support commands

Run these in the copied app folder if a support person asks you to. Normal installation requires no manually opened terminal.

| Platform | Install | Complete uninstall |
| --- | --- | --- |
| Windows | `.\Install-Windows.cmd install` | `.\Install-Windows.cmd uninstall` |
| macOS | `./Install-macOS.command install` | `./Install-macOS.command uninstall` |
| Linux | `sh ./Install-Linux.sh install` | `sh ./Install-Linux.sh uninstall` |

Use `export-diagnostics` instead of `install` to export logs on the computer. Fully specified `--sd-root`, `--powered-off`, `--package`, `--recovery` and `--archive` backend interfaces remain for automation. `remove-integrations` is an advanced operation, not the default uninstall.

The experimental download bootstrap is separate and is not needed by this offline package.
