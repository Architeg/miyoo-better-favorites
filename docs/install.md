# Install or update Better Favorites

[README](../README.md) · [Compatibility](compatibility.md) · [Uninstall](uninstall.md)

Download the single **[install ZIP](https://github.com/Architeg/miyoo-better-favorites/releases/download/v1.0.0-rc.7/better-favorites-1.0.0-rc.7.zip)**. GitHub's Source code downloads and dependency companions are for developers, not additional installation steps. No development tools are needed.

## Mac: Terminal download (recommended)

1. Power off the Miyoo and connect its SD card to your Mac.
2. Open Terminal: press **⌘ Space**, type **Terminal**, then press Enter.
3. Paste:

```bash
curl -fsSL https://raw.githubusercontent.com/Architeg/miyoo-better-favorites/main/scripts/install-online.sh | bash
```

4. Confirm the detected card and choose **[1] Install / Update**.
5. Wait for **Install verified**, safely eject, insert the card and boot.

This downloads the same package and opens its installer without manual copying. Linux can also use this route. See [Terminal download details](online-install.md).

## Windows / Linux, or offline Mac: copy, then click

1. Power off the Miyoo and connect its SD card.
2. Download and extract the install ZIP on your computer.
3. Copy `App/BetterFavorites` into the card's `App` folder.
4. Open the copied `BetterFavorites` folder and its launcher:

| Computer | Open |
| --- | --- |
| Windows | `Install-Windows.cmd` |
| Mac | `Install-macOS.command` |
| Linux | `Install-Linux.desktop`; allow launching if asked |

5. Choose **[1] Install / Update** and confirm the card and powered-off Miyoo.
6. Keep the card connected until verified success, then safely eject and boot.

Unsigned Mac tools may need separate approval for the script and helper. Read the [opening guide](security-opening.md) or bundled [offline Mac guide](../packaging/Mac-first-open.html) first. Linux users without desktop launcher support can run `sh ./Install-Linux.sh install` from the copied app folder.

## First launch on Miyoo

Open **Apps → Better Favorites**. Press **Y** for Settings, or **SELECT → Settings**.
The installer prepares both supported integrations; **Replace stock Favorites** and **Automatic return** start **OFF** on a fresh installation. Enable them independently in Settings. OFF disables a feature; it does not uninstall its patch.

Installation/restoration takes effect after reboot. Unknown system files are not patched. [Controls and settings](user-guide.md).

## Update without losing preferences

For Terminal installation, run the same command and choose Install / Update.
For the offline ZIP, merge the new folder's **contents** into the existing app folder and replace supplied files. Keep personal files not supplied by the package. On Mac, use Merge or copy the contents; do not replace the whole folder.

Open the launcher and choose Install / Update. Saved preferences, browser position and verified original system backups are retained. Unknown or modified files are preserved and reported.

## If installation stops

Read the reason and recorded change state. Keep the detailed log and recovery backups. Do not delete a reported file, edit a receipt, or disable security checks to bypass verification.

Valid Mac metadata belonging to known package files/directories is handled automatically, including on another OS. Unrelated or malformed content remains protected. [Recovery](recovery.md) · [Export diagnostics](diagnostics.md).

Older development installations may need their matching restoration tools before migration. Do not rename a patched app folder; see the [developer migration reference](integrations.md#legacy-development-installations).
