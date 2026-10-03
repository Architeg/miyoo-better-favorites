# Uninstall and failed-menu recovery

**Deleting BetterFavoritesTest does not undo MainUI/runtime patches. Restore the
integrations first, then optionally delete the app.** Keep all originals/backups.
No recovery step requires MainUI, Miyoo Terminal, network access or firmware writes.

## Normal uninstall

Power off and mount the card on Windows/macOS/Linux. Open the packaged host tool,
choose **Uninstall**, give the card root and this card's **latest successful Install/Update recovery folder**. Older bundles retain stock originals for manual copy-back, but automatic restore refuses a stale expected installed manifest.
It verifies every applicable backup and destination before changing anything,
restores stock runtime/MainUI first, then removes its own receipts/helper and makes
journals inert. Reboot and confirm stock Home Favorites and GameSwitcher behavior.
The app, preferences, browser state and backups remain. You may now remove the app
folder. Favorites, history, themes, libraries, saves and firmware are untouched.

## If MainUI will not open

1. Power the Miyoo off. Remove/mount the card on a computer.
2. Open the same packaged host tool. Choose **Restore**, select this card and its
   recovery folder. This accepts exact known original/installed mixtures left by
   interruption. Foreign files are preserved and reported.
3. Reboot after successful verification. Use Apps or stock Favorites.

Host recovery is `recovery-<timestamp>` beside the extracted tool by default.
A mirror is `.tmp_update/config/better-favorites-recovery-<timestamp>` on-card.
Neither is automatically deleted. `before/` is the pre-operation snapshot;
`after/` records expected installed outputs; **`files/` is the stock integration
copy-back tree**. Earlier permanent return/Home backups remain in place.

## Manual copy-back without the tool

Use this **specific card's** verified bundle. Show hidden files (macOS Cmd+Shift+.,
Windows Explorer View → Show → Hidden items, Linux Ctrl+H). Read `RESTORE.txt` and
verify `SHA256SUMS` before copying. Hash tools are optional for navigation but
required for claiming verified restoration:

- Windows PowerShell: `Get-FileHash -Algorithm SHA256 "<backup file>"`.
- macOS Terminal: `shasum -a 256 "<backup file>"`.
- Linux: `sha256sum "<backup file>"`.

Copy **only files actually present under `files/`**, preserving exact paths:

- `files/.tmp_update/runtime.sh` → card `.tmp_update/runtime.sh`.
- `files/.tmp_update/bin/MainUI-283-clean`, `MainUI-283-expert`, `MainUI-354-clean`,
  `MainUI-354-expert` → matching card `.tmp_update/bin/` files.

Check copied hashes against the bundle. Once originals are verified restored,
remove only the known owned `App/BetterFavoritesTest/home-integration.conf` and
`.tmp_update/script/better_favorites_return.sh` if their expected hashes match the
manifest. Keep journals, backups and personal data. An inert journal can be handled
by the tool before reinstalling; do not manually edit it. Do not copy `after/`,
delete shared `.tmp_update`, or restore another card's files. If current files
contain a later unrelated patch, archive them and seek support before replacement.

## Limits

The card must be offline with exclusive writer access. Check-then-rename detects
observed conflicts; it is not filesystem compare-and-swap. Staged files are flushed
and replaced on the same filesystem. Multi-file installation is not one atomic
transaction. Verified journals/rollback handle known failures and interruptions;
physical power loss, a failing card/reader, Windows FAT behavior and manual copying
are separate risks requiring qualification. Preserve both host/card originals.
