<div align="center">
  <img src="App/BetterFavorites/icon.png" alt="Better Favorites icon" width="74" height="74">
  <h1>Better Favorites</h1>
  <p><em>Your Onion favorites, easier to browse.</em></p>
</div>

<p align="center">
  <a href="docs/release/rc.5.md"><img alt="Release candidate" src="https://img.shields.io/badge/status-RC5%20candidate-f59e0b"></a>
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

> **Release status:** RC5 is a local installation-fix candidate. No public binary asset is available yet. Earlier hardware acceptance remains recorded; RC4 approval succeeded but installation failed on metadata; the corrected package needs device acceptance. [Current status →](docs/release/rc.5.md)

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
| ↩️ Optional return | Return to Better Favorites with B/START after using GameSwitcher |
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

Use **`better-favorites-<version>.zip`**, the one ready-to-install package containing the app and computer tools. No public download link is advertised before its asset exists.

1. Download and extract the ready-to-install ZIP on your computer.
2. **Power off** the Miyoo and connect its SD card.
3. Copy the supplied **App/BetterFavorites** folder into the card's **App** folder.
4. Inside that copied folder, open **Install-Windows.cmd**, **Install-macOS.command**, or **Install-Linux.desktop** for your computer.
5. Choose **Install / Update**, confirm the Miyoo is off, and wait for success.
6. Safely eject, insert the card and boot.

The tool identifies the card automatically and prepares both supported integrations. Their app switches start **OFF**. No terminal commands, card-path entry, developer tools or Miyoo Terminal are needed. On Linux, your desktop may require **Allow launching**; unsigned tools may need a file-specific Open confirmation. [Simple platform steps →](docs/install.md)

**Updating?** Merge the new folder's contents into the existing folder. Replace supplied files, but keep preferences/state; do not delete or replace the entire existing app folder.

**GitHub “Source code” archives are for developers**, not ready-to-install apps. Dependency/source companions are attribution and development material, not alternative installers.

### First opening on Mac or Windows

These tools are unsigned/not notarized. On Mac, approve the launcher file if requested; the separate **BetterFavorites-Installer** may also need file-specific **Open Anyway**. The terminal retains its verified path and offers retry after approval. Windows reputation warnings may offer **More info → Run anyway**; this option is not present for every security policy. Stop for malware/damaged-file warnings. [Platform-specific opening guide →](docs/security-opening.md)

<a id="first-launch"></a>
## 🚀 First launch

Open **Apps → Better Favorites**, then press **Y** for Settings.

- **Replace stock Favorites:** ON opens this app from the Home Favorites tile; OFF opens stock Favorites.
- **Automatic return:** ON brings you back to Better Favorites with B/START in GameSwitcher, including after switching games. OFF uses Onion's normal main-menu return.

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
## 🧹 Complete uninstall

Power off and connect the card. Open the same computer launcher in **App/BetterFavorites** and choose **Uninstall completely**.

The tool restores and verifies the original system files, then removes Better Favorites, its preferences, logs and owned installation files. Games, saves, artwork, favorites, recent history, themes and unrelated files remain. A verified copy of recovery is kept on that computer.

Portable recovery is stored separately on the card, so uninstall can run on a different computer. **Deleting the app alone cannot undo the system patches.** Conflicts or missing recovery stop removal with an explanation. [Uninstall and recovery →](docs/uninstall.md)

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
