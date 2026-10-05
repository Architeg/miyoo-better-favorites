> **Historical engineering record — not current installation instructions.**
> Use the [installation guide](../../install.md) and [compatibility matrix](../../compatibility.md) for the released app.

# Better Favorites v1.0.0-rc.2

> Historical engineering record. Use [current installation instructions](../../install.md) and [release notes](../../release/notes-rc.7.md).

An easier way to browse your existing Onion favorites on Miyoo Mini and Mini Plus.

This is a release candidate. The source checkpoint and draft release can be prepared now; binary publication remains pending the documented dependency/source-license audit. This draft does not describe a published stable v1.0.0.

## What is included

- Console grouping or a flat list, configurable numeric prefixes and sorting.
- Selected long-title scrolling, console jumps and L1/R1 page navigation.
- Theme-aware menus, Help, Settings and artwork/resource fallbacks.
- Saved selection and viewport, restored by favorite identity.
- Favorite removal with confirmation, verified backup and exact-record preservation. Games, saves, artwork and recent history remain.
- Onion game launching and recent registration through the verified handoff.
- MENU opens Onion GameSwitcher from the browser.
- Optional Home Favorites replacement; the existing tile resources and X/Y assignments remain.
- Optional session return from GameSwitcher, including after changing games.
- Offline installers with automatic host selection, complete uninstall, per-card recovery and bounded diagnostic export.
- The supplied app icon and revised opening/return help text.

## Install and uninstall

Once cleared packages are attached to this release, extract the installer ZIP on your computer, power off the Miyoo and connect its card. Run from the extracted folder:

| Computer | Install | Complete uninstall |
| --- | --- | --- |
| Windows | `.\Install-Windows.cmd install` | `.\Install-Windows.cmd uninstall` |
| macOS | `./Install-macOS.command install` | `./Install-macOS.command uninstall` |
| Linux | `./Install-Linux.sh install` | `./Install-Linux.sh uninstall` |

Follow the prompts and retain the computer-side recovery bundle. SD-card root means the card's drive or mount, not the installer folder. Optional integration switches default OFF; enable them in app Settings after installation.

The app-only ZIP provides drag/drop Apps access without patches. Full uninstall restores and verifies the original patched system files before removing owned app/data files. Unrelated files and legitimate game/history changes are preserved. Conflicts fail clearly instead of reporting a complete uninstall.

## Compatibility and evidence

- **Miyoo tested:** Mini Plus MY354, firmware 202306282128, Onion v4.3.1-1. Hardware revision unknown.
- **Mac:** RC1 packaged installation/device acceptance on M1 / Ventura 13.7.8. RC2 host tests include actual Rosetta-to-native selection.
- **Windows:** Windows 7 SP1 x64 and Windows 10 x64 normal-flow acceptance are recorded in the supplied report. Diagnostics establish the RC2 Miyoo payload; they do not identify the exact host executable used.
- **Targets:** Windows 7 onward, macOS Monterey onward on Intel/Apple Silicon, and Linux x64/ARM64.
- **Community qualification:** Mini, other Onion/firmware combinations, physical Intel/Monterey, remaining Windows architectures/versions and Linux SD readers are not all verified. Unknown patch inputs are refused.

The final manifest must tie downloadable packages to the tagged source and list their actual hashes. Earlier private archives remain preserved and must not be silently relabeled as the final release.

## Known limits

- Optional patches are restricted to audited Onion v4.3.1-1 files; Mini Flip/other systems are not qualified.
- Stock GameSwitcher cannot distinguish B/START exit-to-menu from every signal-driven exit in the runtime-only return integration.
- The legacy Windows toolchain is isolated; it is not the modern Windows build renamed.
- Unsigned host executables can trigger platform prompts. Download-route qualification remains separate from offline package testing.
- The existing SDL_ttf bzip2 linker warning remains documented; device behavior passed the reported tests.
- Exact dependency/prebuilt/source correspondence and transitive license checks are still open. Functional acceptance does not close those checks.

## Feedback

Report bugs, theme/device results and feature suggestions through the repository issue templates. Include the package version/hash, computer and Miyoo/Onion details, steps and exact error. Export diagnostics with the installer and review the ZIP before attaching it. No Miyoo Terminal commands are needed.

Project sources are GPL-3.0-or-later. Third-party components retain their own licenses; see [third-party notices](../../../THIRD_PARTY_NOTICES.md) and the [dependency audit](../../release/dependency-audit.md).

Online bootstrap preparation is separate and is not an advertised installation route. Final tagged binaries require exact-package qualification; new source/version literals do not retroactively receive earlier device acceptance. No binary assets are published while the dependency gates remain open.
