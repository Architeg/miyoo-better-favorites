# Recover when the Miyoo menu will not open

Power off the Miyoo, remove its SD card and connect it to a computer. You do not need a working MainUI, Miyoo Terminal, Wi-Fi or the original installation computer.

## Use verified uninstall first

1. Open the computer launcher in `App/BetterFavorites`.
2. Choose **[2] Uninstall completely** and follow its prompts.
3. Wait for verified stock restoration and cleanup, then safely eject and boot.

If the launcher is missing, merge files from the matching install ZIP back into the app folder and retry. Keep personal files and hidden recovery files. See [complete uninstall](uninstall.md).

Portable recovery is stored outside the app under `.tmp_update/config/better-favorites-recovery-*`. The file `.tmp_update/config/better-favorites-installation.json` identifies the verified installation. Do not erase these files or choose an unrelated card's backup.

If recovery is missing, corrupt or conflicts with current files, stop and ask for help. Keep the card and any computer recovery archives unchanged. [Export diagnostics](diagnostics.md).

## Manual emergency restoration

Use this only with verified originals from **this card**; ask for help if unsure.

1. Show hidden files and open the matching recovery folder.
2. Read `RESTORE.txt` and verify its `SHA256SUMS`.
3. Copy the originals under `files/.tmp_update/runtime.sh` and `files/.tmp_update/bin/MainUI-*`, when present, back to their identical card paths.
4. Preserve any unexpectedly modified destination before replacing it. Keep recovery and the app until stock restoration is verified.
5. Run complete uninstall for the remaining owned-file cleanup, then safely eject and reboot.

Do not copy `before/` or `after/` blindly: they may contain patched or earlier app files. Older development backups are not interchangeable with release recovery journals. Never manufacture receipts or copy another card's originals.

## Approval or verification errors

For blocked computer tools, follow [file-specific opening help](security-opening.md). Ordinary installation or ownership errors cannot be fixed by security approval. Keep the error and log rather than removing metadata or bypassing checks.

An interrupted cleanup remains incomplete even if system restoration succeeded. Retained recovery allows a verified retry; foreign changes remain protected.
