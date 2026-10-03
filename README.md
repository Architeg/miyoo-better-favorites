<div align="center">
  <img src="App/BetterFavoritesTest/icon.png" alt="Better Favorites icon" width="74" height="74">
  <h1>Better Favorites</h1>
  <p><em>Your Onion favorites, easier to browse.</em></p>
</div>

<p align="center">
  <a href="docs/release/rc.2.md"><img alt="Release candidate" src="https://img.shields.io/badge/status-RC2%20candidate-f59e0b"></a>
  <a href="docs/compatibility.md"><img alt="Tested on Onion v4.3.1-1" src="https://img.shields.io/badge/tested%20Onion-v4.3.1--1-7c5cbf"></a>
  <a href="LICENSE"><img alt="GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-2563eb"></a>
</p>

<p align="center">
  <a href="#install">Install</a> · <a href="#controls">Controls</a> ·
  <a href="#settings">Settings</a> · <a href="#uninstall">Uninstall</a> ·
  <a href="CONTRIBUTING.md">Contribute</a>
</p>

<!-- Insert the supplied banner and real before/after GIF when ready. No broken placeholders. -->

<a id="why-better-favorites"></a>
## ⭐ Why Better Favorites?

A growing favorites list is easier to browse with console groups, readable titles and saved preferences. Better Favorites brings those features to your existing Onion favorites on **Miyoo Mini and Mini Plus**.

The native C++/SDL2 app reads Onion's `Roms/favourite.json`. Onion still handles game launching, cores, saves, recent history and GameSwitcher.

> **Release status:** RC2 is prepared for review; no public release is available yet. Mini Plus hardware tests have passed, but dependency/source-license checks remain open before binary publication. [Current status →](docs/release/rc.2.md)

<a id="features"></a>
## ✨ Features

| Feature | What you get |
| --- | --- |
| 🗂️ Console groups | Browse by system or use a flat list |
| 📝 Readable titles | Hide numeric prefixes, choose sorting and scroll long selected titles |
| 🎨 Theme-aware UI | Active-theme fonts, colors and resources, with missing-resource fallbacks |
| 🎮 Quick navigation | Console jumps, shoulder paging and remembered selection/scroll |
| 🔀 GameSwitcher | Open Onion's GameSwitcher directly with MENU |
| 🏠 Optional Home access | Open the app through the existing Favorites tile |
| ↩️ Optional return | B/START in GameSwitcher returns an originating app session here |
| 🗑️ Favorite removal | Remove the entry while keeping the game, artwork and saves |

Installation, recovery and diagnostic export work offline with the supplied package. No Miyoo Terminal commands are needed.

<a id="compatibility"></a>
## 💻 Compatibility

**Designed for Mini and Mini Plus; hardware tested on Mini Plus.**

**Device tested:** Miyoo Mini Plus (MY354), firmware `202306282128`, Onion **v4.3.1-1**. Hardware revision is unknown; MY354 identifies the model/platform. Mini testing is welcome; Flip and other systems are not qualified for these integrations.

| Installation computer | Target |
| --- | --- |
| 🪟 Windows | Windows 7 onward; legacy x86/x64 and modern x86/x64/ARM64 builds |
| 🍎 macOS | Monterey onward; Intel and Apple Silicon |
| 🐧 Linux | x64 and ARM64; kernel requirements apply |

Scripts select the computer-side executable; every computer installs the **same Miyoo files**. System patches accept only audited Onion files.

<details>
<summary><strong>Targets versus tested combinations</strong></summary>

Mac installation/device checks passed on M1 / Ventura 13.7.8 with RC1. Windows 7 SP1 x64 and Windows 10 x64 normal-flow acceptance are recorded in the supplied report; its diagnostic export identifies the RC2 Miyoo payload, not the host executable. Linux roundtrips passed in Docker.

Physical Intel/Monterey, other Windows versions/architectures, Linux SD readers and Mini hardware remain unverified. [Requirements and evidence →](docs/compatibility.md)

</details>

<a id="install"></a>
## 📦 Install

Use the supplied **installer ZIP** for the app and optional integrations. Extract it on your computer, outside the SD card.

1. **Power off** the Miyoo and connect its SD card to the computer.
2. Open a terminal in the extracted installer folder and run one command:

| Computer | Install |
| --- | --- |
| Windows PowerShell | `.\Install-Windows.cmd install` |
| macOS Terminal | `./Install-macOS.command install` |
| Linux terminal | `./Install-Linux.sh install` |

3. Follow the prompts. **SD-card root** means the card location, such as `E:\` or `/Volumes/MIYOO`, not the installer folder or the command again.
4. Choose either optional integration, both, or neither. Keep the recovery folder.
5. Wait for verified success, safely eject, insert the card and boot.

**No Go, Python, Docker, WSL or compiler is required.** The script without an action opens its menu. [Detailed installation →](docs/install.md)

<details>
<summary><strong>Prefer drag and drop?</strong></summary>

Extract the **app-only ZIP** and copy `App/BetterFavoritesTest` into the card's `App` folder while the Miyoo is off. Eject, boot and open **Apps → Better Favorites**.

This installs the browser without patches. The internal folder name is retained for compatibility. Updating an existing folder requires merging files to preserve preferences. [App-only guide →](docs/install.md#app-only-drag-and-drop)

</details>

<a id="first-launch"></a>
## 🚀 First launch

Open **Apps → Better Favorites**, then press **Y** for Settings.

- **Replace stock Favorites:** ON opens this app from the Home Favorites tile; OFF opens stock Favorites.
- **Automatic return:** ON returns an originating session here with B/START in GameSwitcher, including after switching games. OFF uses Onion's normal main-menu return.

Both switches default **OFF**, need their installed integrations, and work independently. Apps access and existing X/Y shortcuts stay available. Direct game exit ends the automatic-return session.

<a id="controls"></a>
## 🎮 Controls

| Button | In the favorites browser |
| --- | --- |
| ↑ / ↓ | Move selection |
| ← / → | Jump consoles when grouped |
| L1 / R1 | Page up / down; stop at list ends |
| **A** | Launch the selected game |
| **B** | Exit to Onion's main menu |
| **SELECT** | Open actions |
| **Y** | Open Settings |
| **MENU** | Open Onion GameSwitcher |

In menus, **A** activates and **B** goes back or cancels. MENU closes menus without opening GameSwitcher. Removal uses ←/→ for Cancel/Remove and ↑/↓ for long-title pages. [Full controls →](docs/user-guide.md)

<a id="settings"></a>
## ⚙️ Settings

| Setting | Default | Effect |
| --- | --- | --- |
| Group by console | ON | Console headings/jumps; OFF gives a flat list |
| Numeric prefixes | Show | Display only; stored labels remain unchanged |
| Sorting | Original label | Literal-label order or alphabetical title without leading numeric prefixes |
| Replace stock Favorites | OFF | Use the Home tile when its integration is installed |
| Automatic return | OFF | Return from GameSwitcher when its integration is installed |

Preferences save immediately; a failed save retains the previous value. About pages explain integration availability separately from its ON/OFF switch.

<a id="remove-a-favorite"></a>
## 🗑️ Remove a favorite

Press **SELECT → Remove from Favorites**. Cancel is selected first; choose with ←/→, confirm with A, or cancel with B.

**Your game stays on the card.** Only the selected favorites record is removed. ROMs, artwork, saves and recent history remain. A backup is verified first; other records are preserved and conflicting edits are refused. Add favorites through Onion's existing menus.

<a id="update-or-uninstall"></a>
<a id="uninstall"></a>
## ↩️ Update or uninstall

**Update:** extract the new installer into a fresh computer folder and run `install` again. Preferences and browser position stay. Retain the recovery bundles.

**Complete uninstall:** power off, connect the card, and run:

| Computer | Uninstall |
| --- | --- |
| Windows PowerShell | `.\Install-Windows.cmd uninstall` |
| macOS Terminal | `./Install-macOS.command uninstall` |
| Linux terminal | `./Install-Linux.sh uninstall` |

The tool restores and verifies original patched system files, retains a recovery archive on your computer, and removes owned app files, preferences, logs and integration artifacts. Games, saves, favorites/history and unrelated resources remain. Unknown modifications are preserved and reported as failure.

> Deleting the app folder alone cannot undo system patches. Recovery works even if the Miyoo menu will not open.

Use `remove-integrations` to keep the app while restoring its system integrations. [Uninstall and recovery →](docs/uninstall.md)

<a id="troubleshooting-and-diagnostics"></a>
## 🛟 Help and diagnostics

| Problem | Next step |
| --- | --- |
| Integration unavailable | Check installation and Onion compatibility in About |
| Card path not found | Enter the actual SD drive/mount; attach the reader to Windows in Parallels |
| Installer reports a conflict | Preserve the message and recovery bundle; do not overwrite the unknown file |
| Need help with a bug | Export diagnostics and attach the reviewed ZIP to an issue |

With the powered-off card connected, use the platform script with `export-diagnostics`, for example:

```powershell
.\Install-Windows.cmd export-diagnostics
```

Review before sharing: errors may contain game filenames, theme paths and preferences. Detailed tracing is OFF by default. [Diagnostics and privacy →](docs/diagnostics.md)

<a id="contribute-and-learn-more"></a>
## 🤝 Help improve Better Favorites

[Report a bug or suggest a feature](https://github.com/Architeg/miyoo-better-favorites/issues/new/choose), share a theme/device test, or [give the project a star](https://github.com/Architeg/miyoo-better-favorites/stargazers). You can contribute without writing code.

**[Contributing](CONTRIBUTING.md)** · [Development](docs/development.md) · [Roadmap](docs/roadmap.md) · [Changelog](CHANGELOG.md)

<a id="credits-and-license"></a>
## 📜 Credits and license

Thanks to [OnionUI](https://github.com/OnionUI/Onion), the [SDL/Miyoo fork contributors](https://github.com/Rparadise-Team/sdl2_miyoo_new), and the upstream library authors.

Project sources are **[GPL-3.0-or-later](LICENSE)**. Dependencies retain their own licenses. [Third-party notices and provenance →](THIRD_PARTY_NOTICES.md)
