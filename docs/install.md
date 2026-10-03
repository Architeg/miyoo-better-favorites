# Install and update

Use the supplied installer or app-only ZIP. Current distribution is a review
candidate; no stable release is published. **Designed for Mini and Mini Plus;
hardware tested on Mini Plus.** Read [compatibility](compatibility.md) before
installing optional integrations. No firmware update or development tools are needed.

## Recommended: offline packaged installer

1. Power the Miyoo off. Remove its card and connect it to the computer.
2. Extract the installer ZIP into a computer folder **outside the SD card**.
   Keep its scripts, executables, manifests and payload together.
3. Close other card-writing programs. In the extracted folder, run:

| Platform | Install | Complete uninstall |
| --- | --- | --- |
| Windows | `.\Install-Windows.cmd install` | `.\Install-Windows.cmd uninstall` |
| macOS | `./Install-macOS.command install` | `./Install-macOS.command uninstall` |
| Linux | `./Install-Linux.sh install` | `./Install-Linux.sh uninstall` |

4. Enter the mounted card root, confirm the device is OFF and choose optional patches.
   The installer verifies the files and backs up this card's originals before replacing them.
5. Keep the printed recovery folder on the computer. Wait for success, safely
   eject/unmount, return the card to the Miyoo and boot.

The card root is the drive/folder containing `App`, `Roms` and `.tmp_update`, not
its App subfolder. No-argument scripts open the menu; action-only calls prompt.
Explicit flags remain available for automated/support use. The installer runs
offline and needs no compiler, Python, Go, Docker, WSL or PowerShell upgrade.

The wrapper selects the packaged executable for OS/version/native architecture.
[Host requirements and dispatch](release/host-dispatch.md) include legacy Windows,
Rosetta, Linux kernel minima and verified versus untested combinations. The tools
are unsigned; distribution/quarantine prompts are a separate qualification item.

### Optional integrations and first launch

Open **Apps → Better Favorites**, then press Y for Settings.

- **Home Favorites replacement:** optionally patch the existing tile's activation.
  Turn **Replace stock Favorites** ON only when available. OFF follows stock behavior.
- **Automatic return:** optionally install runtime hooks. When its switch is ON,
  B/START in GameSwitcher reopens a session started in Better Favorites, including
  after game switches. A resumes; direct game exit ends the return session.

Both preferences default OFF and are independent. Installing a patch does not
turn its switch on. About shows availability separately. Apps access, existing
themes, stock X/Y and Onion game settings remain unchanged. System patch installation
or restoration takes effect after reboot.

### Retain recovery

Keep the entire printed recovery folder, including `recovery.json`, `SHA256SUMS`,
`RESTORE.txt` and hidden `.tmp_update` content under `files/`. It is this card's
verified originals. Retain earlier successful update bundles too. Automatic
uninstall uses only a validated matching identity; ambiguous/missing recovery asks
for explicit selection rather than choosing the first/newest folder.

Recovery works with a powered-off card even when MainUI cannot open. Deleting the
app cannot restore patches. [Uninstall and recovery](uninstall.md).

## App-only drag and drop

1. With the Miyoo off, extract the app-only ZIP on the computer.
2. Copy `App/BetterFavoritesTest` into the card's `App` folder.
3. Safely eject/unmount, boot and open Apps → Better Favorites.

The exact supplied icon uses Onion's app-icon mechanism. `BetterFavoritesTest` is
the retained installation folder name. Themes, games and their artwork come from
your card. Drag/drop installs no optional system patches; unavailable settings do
not mean a patch is installed. Installer-backed installation is recommended for
verified recovery and automated complete removal.

## Updating

Extract the new installer into a **fresh computer folder** and run `install`.
It updates package-listed files and preserves app preferences/browser state.
Select optional integrations when adding them; keep each successful recovery.
Declining new optional patches does not uninstall an existing integration.

For app-only updates, merge payload files into the existing app directory. Preserve
`settings.conf`, `home-entry.conf`, `browser-preferences.conf`, `browser-state`
and an existing `home-integration.conf` receipt. Do not replace/delete the whole
folder. If your file manager cannot merge safely, use the installer.

Unknown/modified files are refused. Keep the error and backups; do not use generic
MainUI/runtime files or bypass the checks. [Troubleshooting](uninstall.md).

## Windows 7 portable test folder

Extract `BetterFavorites-Windows7-Test.zip` with Explorer to
`C:\BetterFavorites-Test`. Its retained alias supports:

```powershell
.\Install-Windows7.cmd install
.\Install-Windows7.cmd export-diagnostics
.\Install-Windows7.cmd uninstall
```

The current test alias invokes the same Windows entry/dispatch mechanism as the
normal installer. It needs no PowerShell upgrade, Get-FileHash or Expand-Archive.
[Windows acceptance and exact evidence limits](release/windows-acceptance.md).

## Export diagnostics

With the powered-off card connected, use your same entry script with
`export-diagnostics`. It writes a new ZIP beside the tool and does not enable
tracing or change preferences. [Contents/privacy](diagnostics.md).
