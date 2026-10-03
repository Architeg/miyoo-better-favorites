# Complete uninstall and failed-menu recovery

Power the Miyoo OFF and mount its card. From the extracted installer folder:

- macOS: `./Install-macOS.command uninstall`
- Linux: `./Install-Linux.sh uninstall`
- Windows 7 onward: `.\Install-Windows.cmd uninstall`
- Windows 7 test bundle: `.\Install-Windows7.cmd uninstall`

The tool prompts for the card and OFF confirmation. It locates and validates recovery
only when one matching identity exists; otherwise it asks for this card's retained
recovery folder. Multiple identities are never resolved by choosing the first/newest.
The no-argument menu and fully specified commands still work.

**Default uninstall is complete removal in one operation:** preflight every file;
keep a verified recovery/removal archive on the computer; restore stock MainUI/runtime;
automatically verify hashes; remove the owned app (including preferences/state/logs),
helpers, receipts, journals, on-card backups/mirrors and app-generated removal backups.
Favorites/history, ROMs, saves, artwork, themes, shared system libraries, shortcuts
and unrelated files/directories are preserved. App-private libraries are removed
with the app. Unknown/modified files cause failure and identify unresolved paths;
partial cleanup is never reported as complete. Keep the printed computer archive.

## Retain app/preferences: optional operation

Use the same wrapper with `remove-integrations` instead of `uninstall`. This restores
stock integrations and retains the app/data/backups. Installation and removal of
system binaries take effect after reboot. Deleting the app alone cannot undo patches.

## Missing MainUI or interrupted operation

No Miyoo menu, Terminal, compiler, Python, Docker, WSL or network is needed. Power off,
mount the card and use the same tool with `restore` to restore known original/patched
mixtures while retaining app/data. Use `uninstall` to finish complete removal.
A failed cleanup can be retried with its retained computer recovery bundle. Do not
use an older bundle if a later update changed the expected outputs; conflict checks
will refuse it. If recovery is missing/corrupt, nothing is guessed or blindly deleted.

## Manual recovery if the host tool cannot run

Use this card's retained recovery bundle. Show hidden files. Read RESTORE.txt;
`files/` is the stock integration copy-back tree. `before/` and `after/` must not be
copied blindly. Copy only verified `files/.tmp_update/runtime.sh` and existing
`files/.tmp_update/bin/MainUI-*` to identical SD paths. Never overwrite a foreign
patch without preserving it and seeking support. Never delete shared .tmp_update.
Mac `shasum -a 256` / Linux `sha256sum` / Windows `certutil -hashfile FILE SHA256`
can verify individual files. Windows 7's bundled PowerShell lacks Get-FileHash and
Expand-Archive; use Explorer's Extract All for the portable test ZIP.
Keep the entire recovery bundle. Reboot and confirm stock behavior afterward.
The updated automatic uninstall does all routine verification/removal; manual copying
is an emergency fallback, not the normal installation procedure.
