# Diagnostics and privacy

## Export a report

Power off the Miyoo, connect its card and open the copied App/BetterFavorites folder.
Use the same entry script as installation:

| Platform | Export |
| --- | --- |
| Windows | `.\Install-Windows.cmd export-diagnostics` |
| macOS | `./Install-macOS.command export-diagnostics` |
| Linux | `./Install-Linux.sh export-diagnostics` |

The Windows7 test folder also supports `.\Install-Windows7.cmd export-diagnostics`.
The copied launcher derives the card automatically. A fresh ZIP is written in your computer home folder;
existing exports are not replaced. Export does not alter preferences or enable tracing.
No Miyoo Terminal, Wi-Fi, SSH or developer tools are needed.

## Contents and privacy

Exports include bounded app log tails, allowlisted app settings/status, version/source
metadata, MainUI/runtime/helper hashes, active-theme identifier and missing evidence.
**Inspect the ZIP before sharing.** Error logs may include game filenames; preferences
and theme paths can be personal. Favorites/history contents, ROMs, saves, credentials
and serials are not collected. Do not upload private recovery originals or card images.

Model/firmware/revision remain unknown when offline files cannot establish them.
Supply known device details separately. Current exports do not capture the computer
OS/build/architecture, host installer executable hash or tested ZIP checksum; include
those in your test report. Offline inspection does not measure RAM or prove a reboot.
[Windows evidence example](release/windows-acceptance.md).

## Basic logs

`App/BetterFavorites/better-favorites.log` and `better-favorites.previous.log`
each cap at 65,536 bytes (128KiB total). They rotate at launcher entry and record
startup, errors, SDL/audio cleanup and handoff boundaries. No normal navigation or
per-frame logger, watcher or resident gameplay helper is added.

Logging is best-effort: full/unwritable/nonregular files do not change launch or
fallback behavior. Links/FIFOs are refused. App-owned streams are bounded; loader
failures before main may appear only as launcher exit status. Concurrent app
invocations are unsupported. Preserve existing evidence before repeated tests.

## Detailed tracing — support only

Detailed Home/return tracing is OFF normally and independent of both feature
switches. Only enable it when troubleshooting a lifecycle/redirect issue. With
exclusive access to the powered-off card, use your same entry script with `trace-on`,
then later `trace-off`. For example:

```powershell
.\Install-Windows.cmd trace-on
.\Install-Windows.cmd export-diagnostics
.\Install-Windows.cmd trace-off
```

macOS/Linux use their same scripts and actions. Disabling removes only the owned
activation marker; collected logs/exports remain. Missing detailed logs when tracing
is OFF are expected, not evidence that every interaction did or did not occur.

New Home and return trace writes each cap at128KiB; existing oversized legacy logs
are retained without growth and export reads bounded tails. Trace writes and the
short-lived executable logging modes may affect startup timing; their cost is
unmeasured. Do not infer performance improvements from earlier differently
instrumented sessions. [Developer guide](development.md).
