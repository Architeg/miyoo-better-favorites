<div align="center">
  <img src="App/BetterFavorites/icon.png" alt="Better Favorites icon" width="74" height="74">
  <h1>Better Favorites</h1>
  <p><em>Your Onion favorites, easier to browse.</em></p>
</div>

<p align="center">
  <a href="https://github.com/Architeg/miyoo-better-favorites/releases/tag/v1.0.0-rc.7"><img alt="Download Better Favorites" src="https://img.shields.io/badge/download-Better%20Favorites-2563eb"></a>
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

> **[Download Better Favorites](https://github.com/Architeg/miyoo-better-favorites/releases/download/v1.0.0-rc.7/better-favorites-1.0.0-rc.7.zip)** — one ZIP for Windows, Mac and Linux. [What’s new →](docs/release/notes-rc.7.md)

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

Mac M1/Ventura and Windows installation, complete uninstall and device use have
recorded user tests. Earlier Windows tests include Windows 7 SP1 x64 and Windows
10 x64. Intel Mac installation has been tested; a reported cross-computer receipt
conflict has a host-tested correction that still needs a repeat device check.

Linux has automated container tests; physical Linux testing is welcome. Other
Onion versions, Mini devices and additional host configurations need community
reports. [Full compatibility and evidence →](docs/compatibility.md)

</details>

<a id="install"></a>
## 📦 Install

**Power off the Miyoo and connect its SD card** before either method.

### 🍎 Mac: one Terminal command (recommended)

Press **⌘ Space**, type **Terminal**, and press Enter. Paste:

```bash
curl -fsSL https://raw.githubusercontent.com/Architeg/miyoo-better-favorites/main/scripts/install-online.sh | bash
```

Confirm the detected card and choose **[1] Install / Update**. The script downloads
and verifies the same release ZIP, selects Intel or Apple Silicon automatically,
and opens the installer. No manual extraction or copying is needed.
Wait for verified success, safely eject, insert the card and boot.

This route was reported to launch successfully without security warnings on
Intel and M1 Macs. It uses the existing installer and does not disable security
protections. [Full Terminal steps →](docs/online-install.md)

### 🪟 Windows / 🐧 Linux: download and open

Use **[better-favorites-1.0.0-rc.7.zip](https://github.com/Architeg/miyoo-better-favorites/releases/download/v1.0.0-rc.7/better-favorites-1.0.0-rc.7.zip)**, the single package containing the app and offline computer tools.

1. Download and extract the ready-to-install ZIP on your computer.
2. Copy the supplied **App/BetterFavorites** folder into the card's **App** folder.
3. Inside that copied folder, double-click **Install-Windows.cmd** on Windows,
   or open **Install-Linux.desktop** on Linux.
4. Choose **[1] Install / Update**, confirm the Miyoo is off, keep the card connected, and wait for verified success.
5. Safely eject, insert the card and boot.

Linux users can also use the Terminal command above; physical Linux testing is
welcome. On Linux, your desktop may require **Allow launching**.
Mac users who prefer an offline ZIP can follow the same copy steps and open
**Install-macOS.command**; see [Mac first open](packaging/Mac-first-open.html)
if approval is requested. [Platform steps →](docs/install.md)

### Updates, removal and help

Both methods identify the card and prepare the same supported integrations.
Their switches start **OFF**; enable them in **Y → Settings**.

Run the Terminal command again or open the installed computer launcher to choose
**[2] Uninstall completely** or **[3] Export diagnostics**. Diagnostics are saved
on your computer; review them before sharing.

**Updating?** With the Terminal route, run the same command and choose Install /
Update. With the ZIP, merge its folder contents into the existing app folder;
replace supplied files and keep preferences/state. Do not replace the entire
existing folder.

**Recovery:** deleting the app folder does not undo patches. Keep recovery files;
use Install / Update for verified repair or Uninstall completely for restoration
and cleanup. [Uninstall and recovery →](docs/uninstall.md)

**GitHub “Source code” archives are for developers**, not ready-to-install apps.
Source/license companions are not extra installation steps.

For the offline ZIP, unsigned tools may need file-specific approval.
[Mac/Windows opening help →](docs/security-opening.md)

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

Preferences save immediately; a failed save retains the previous value. Select a setting to read its description; its About page explains how it works.

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
| Card path not found | Reconnect the card; in Parallels, attach the SD reader to Windows |
| Installer reports a conflict | Preserve the message and recovery bundle; do not overwrite the unknown file |
| Need help with a bug | Export diagnostics and attach the reviewed ZIP to an issue |

Power off the Miyoo and connect the card. Open the computer launcher inside
**App/BetterFavorites** and choose **[3] Export diagnostics**. It saves a ZIP on
your computer and prints its location. No manually entered command is needed.

Review before sharing: errors may contain game filenames, theme paths and preferences. Detailed tracing is OFF by default. [Diagnostics and privacy →](docs/diagnostics.md)

<a id="contribute-and-learn-more"></a>
## 🤝 Help improve Better Favorites

[Report a bug or suggest a feature](https://github.com/Architeg/miyoo-better-favorites/issues/new/choose), share a theme/device test, or [give the project a star](https://github.com/Architeg/miyoo-better-favorites/stargazers). You can contribute without writing code.

**[Contributing](CONTRIBUTING.md)** · [Development](docs/development.md) · [Roadmap](docs/roadmap.md) · [Changelog](CHANGELOG.md)

<a id="credits-and-license"></a>
## 📜 Credits and license

Thanks to [OnionUI](https://github.com/OnionUI/Onion), the [SDL/Miyoo fork contributors](https://github.com/Rparadise-Team/sdl2_miyoo_new), and the upstream library authors.

Project sources are **[GPL-3.0-or-later](LICENSE)**. Dependencies retain their own licenses. [Third-party notices and provenance →](THIRD_PARTY_NOTICES.md)


