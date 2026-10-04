# Complete uninstall

[Install](install.md) · [Recovery](recovery.md)

1. Power off the Miyoo and connect its card to any supported computer.
2. Open the appropriate computer launcher inside `App/BetterFavorites`.
3. Choose **Uninstall completely**, confirm the card and that the Miyoo is off.
4. Wait for verified success, safely eject and boot stock Onion.

One operation validates portable recovery, restores and verifies original patched system files, archives a verified copy on this computer, and removes owned app/preferences/state/logs/computer tools/integration artifacts. Portable card recovery is removed last, after restoration and app cleanup. Shared directories and game data are retained.

The computer archive is in `BetterFavorites-Recovery` under your home folder. It can contain app preferences/logs and recovery originals: keep it private. Recovery does not depend on your original computer, username or drive letter.

Do not delete the app as a substitute for uninstall. It cannot restore patched MainUI/runtime files.

## If removal stops

The tool reports the unresolved path and returns failure. Unknown/modified files are preserved. Missing or corrupt originals are never guessed. Recovery remains available through restoration/app-cleanup failures; retry after resolving the reported problem. After an interruption that removed the launcher, copy the matching ready-to-install package contents back into the app folder and reopen it. Do not replace personal files.

[Recovery without MainUI or device Terminal](recovery.md).

## Advanced operations

The entry scripts accept `uninstall` for complete removal and `remove-integrations` for restoring only integrations, retaining app/data. Explicit support flags remain available; this latter operation requires the validated recovery and powered-off card. Never select an ambiguous recovery by guessing.

Validated Mac metadata in owned app/recovery folders is included in the verified
computer archive and removed with those folders, regardless of the uninstalling
computer's OS. Unrelated hidden files, malformed/orphan sidecars and links remain
protected: uninstall reports failure rather than complete removal. Shared folders
and their Finder metadata are not cleaned. No whole-card metadata cleanup is used.
