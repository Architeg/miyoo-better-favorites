# Uninstall Better Favorites completely

[Install](install.md) · [Recovery](recovery.md)

1. Power off the Miyoo and connect its SD card to a supported computer.
2. Open the computer launcher in `App/BetterFavorites`, or use the [Mac/Linux Terminal command](online-install.md).
3. Choose **[2] Uninstall completely** and confirm the card and powered-off Miyoo.
4. Wait for verified success, safely eject and boot stock Onion.

The tool restores and verifies this card's original patched system files, then removes Better Favorites, its settings, logs and owned installation files. Games, saves, artwork, themes, favorites, recent history, shortcuts and unrelated files remain.

**Deleting the app folder alone cannot undo system patches.** Switching a feature OFF also does not uninstall it.

## Your recovery copy

A verified archive is retained under `BetterFavorites-Recovery` in your computer's home folder. Keep it private: it can contain preferences, logs and original system files.

Portable recovery on the card allows uninstall from another supported computer. The card-side recovery is removed last, after restoration and cleanup verify successfully. Shared directories are not removed.

## If uninstall stops

The message identifies what could not be verified. Unknown changes are preserved and the operation reports failure, not complete removal. Keep recovery and logs; do not guess which files to delete.

If an interruption removed the launcher, merge the matching install ZIP's files back into the app folder and retry. Preserve personal files and the hidden recovery directory. [Recovery without MainUI or Miyoo Terminal](recovery.md).

For support only, `remove-integrations` restores integrations while retaining app/data. It is not the default uninstall. [Developer operations](integrations.md#computer-installer-interface).
