# Quick start — v1.0.0-rc.1

This is a review candidate. Read [remaining gates](release/rc.1.md) before testing.
Designed for Mini and Mini Plus; hardware tested on Mini Plus. Optional patches
accept only the audited Onion v4.3.1-1 files. No firmware update is required.

## App only: Windows, macOS or Linux

1. Power the Miyoo off, remove its card and connect it to the computer.
2. Extract `better-favorites-1.0.0-rc.1-app-only.zip` on the computer.
3. Copy `App/BetterFavoritesTest` into the card's `App` folder.
4. Safely remove the card, boot and open **Apps → Better Favorites**.

No compiler, Python, Docker or WSL is needed. An empty icon configuration requests
Onion's ordinary missing-icon behavior; that candidate detail still needs a device
check. All game/theme files come from your existing card.

**Updating an existing app:** preserve `settings.conf`, `home-entry.conf`,
`browser-preferences.conf`, `browser-state` and `home-integration.conf`. Merge
payload files, never replace/delete the whole existing directory. If your file
manager cannot merge safely, use the installer **Install** action without selecting
new optional patches. It updates only package-listed files and retains settings.

## App plus optional integrations: one offline operation

Extract `better-favorites-1.0.0-rc.1-installer.zip` on the computer, outside the card.
Keep its files together. Leave the Miyoo OFF and close all other card writers.

| Computer | Open |
| --- | --- |
| Windows x64 | `Install-Windows.cmd` |
| macOS Apple Silicon or Intel | `Install-macOS.command` |
| Linux x64 or ARM64 | `Install-Linux.sh` |

Choose **Install**, enter the mounted card root (for example `E:\`,
`/Volumes/MIYOO`, or `/media/you/MIYOO`) and confirm **OFF**. Choose Home and/or return
integration independently. The tool installs the app in the same operation,
validates all requested integration inputs, creates a fresh verified host recovery
folder beside the tool, mirrors it on-card, stages replacements and checks outputs.
Unknown versions/hashes or third-party patch conflicts are refused before replacement.
Both app preferences remain unchanged; absent preferences default OFF.

Retain every printed recovery folder; use the **latest successful Install/Update recovery folder** for automated uninstall/restore. Keep the complete bundle, including hidden `.tmp_update` content,
`recovery.json`, `SHA256SUMS` and `RESTORE.txt`. Copy it somewhere safe. It contains
this card's originals, not generic files. Reboot after binary installation/removal.
Enable **Replace stock Favorites** or **Automatic return** in app Settings only
when the corresponding integration is available. Installation and activation are
separate. Apps access and stock X/Y stay available.

The host executables are self-contained and unsigned. Native Windows implementation
exists, but Windows execution/SD-reader qualification is pending; cross-compilation
is not that qualification. macOS/Linux filesystems and actual ZIP fixtures are
listed in [candidate checks](release/rc.1.md). Download/quarantine prompts also need
qualification on the release distribution route. No WSL route is advertised.

## Diagnostics

Choose **Export diagnostics** in the same host tool with the powered-off card
mounted. It writes a fresh ZIP beside the tool, leaving settings and tracing
unchanged. Inspect it before attaching it to an issue. Logs may name a game in an
error; favorite/history contents, ROMs, credentials and serials are not exported.
Model/firmware/revision are reported unknown when they cannot be established from
the mounted files; add those details yourself.

Detailed Home tracing stays OFF. Advanced support-only commands are documented in
[diagnostics](diagnostics.md). Recovery never requires a working Miyoo menu.
