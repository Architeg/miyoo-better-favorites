# Better Favorites

**[Download the install ZIP](https://github.com/Architeg/miyoo-better-favorites/releases/download/v1.0.0-rc.7/better-favorites-1.0.0-rc.7.zip)** — one package for Windows, Mac and Linux. You do not need GitHub’s “Source code” downloads to install the app.

## ✨ Features

- Console groups or a flat list, saved sorting/display preferences, long-title scrolling and shoulder-button paging.
- Theme-aware menus, remembered selection and safe favorite removal that keeps games and saves.
- Onion game launching and MENU → GameSwitcher.
- Optional Home Favorites replacement and automatic GameSwitcher return; both start **OFF**.

## 🐛 Fixes

- Corrected recovery-backed Home receipt handling across reinstall and complete uninstall.
- Cached Settings verification so returning from explanation pages avoids repeated system-file checks.
- Separate B/START badges in Automatic return descriptions.

## 📦 Install and remove

Power off the Miyoo, connect its card and copy **App/BetterFavorites** from the ZIP into the card’s **App** folder. Open the computer launcher inside that folder:

- **1:** Install / Update
- **2:** Uninstall completely
- **3:** Export diagnostics

Enable the optional integrations in **Y → Settings**. Deleting the app folder alone does not undo patches.

[Installation guide](https://github.com/Architeg/miyoo-better-favorites/blob/main/docs/install.md) · [Mac/Linux Terminal alternative](https://github.com/Architeg/miyoo-better-favorites/blob/main/docs/online-install.md)

## 💻 Compatibility

Hardware tested on **Miyoo Mini Plus / Onion v4.3.1-1**. Designed for Mini and Mini Plus; system integrations require audited file hashes. Windows 7+, macOS Monterey+ Intel/Apple Silicon and Linux x64/ARM64 are targets. Physical Linux and additional device/version testing are welcome.

[Compatibility and known limits](https://github.com/Architeg/miyoo-better-favorites/blob/main/docs/compatibility.md) · [Report a bug](https://github.com/Architeg/miyoo-better-favorites/issues/new/choose)

<details>
<summary>Checksums and developer downloads</summary>

Install ZIP SHA-256:

```text
bea79fa6bc98d3ed9ab26503502342ed4619976a063e31a6ca93b1a78c78a389
```

Only **better-favorites-1.0.0-rc.7.zip** is an installer. `SHA256SUMS` verifies downloads; `SOURCE-INVENTORY.json` and the project/dependency source archives support licensing and development. They are not extra installation steps.

Binary source commit: `462ebd0309ad329b8852e1791e1835327d117301`. Bundled guides refreshed from documentation commit `e576c3e2956cb8b1e57f4385df1b50ebcab665ae`. Executables, launchers, libraries and integration payloads are unchanged. Matching binary-source companions are retained; the documentation inventory records the newer guides separately. The Terminal downloader reuses this package.

</details>
