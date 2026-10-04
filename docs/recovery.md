# Recovery when the Miyoo menu will not open

No MainUI or Miyoo Terminal is needed. Power the device off, remove its card and connect it to a computer.

## Preferred recovery

Open the computer launcher in `App/BetterFavorites` and choose **Uninstall completely**. It validates the card-side installation index and recovery hashes. If the app folder/launcher was removed during interruption, merge the matching ready-to-install package files back into it and retry. Do not erase the hidden recovery directory.

Portable recovery directories are `.tmp_update/config/better-favorites-recovery-*`; `.tmp_update/config/better-favorites-installation.json` identifies the active verified journal. Paths inside are relative to the card, so moving computers/mounts works. Original stock files are not replaced by patched files on update. The pending index remains valid through an interrupted attempt; verified previous-journal hashes identify this installation’s update chain. Unrelated recovery directories are preserved, not adopted for cleanup.

Missing, conflicting or altered recovery requires support. Keep the card and any computer recovery copies unchanged. The installer preserves unknown changes rather than falsely reporting complete removal.

## Manual emergency fallback

Show hidden files. In the matching recovery directory, read `RESTORE.txt` and verify `SHA256SUMS` (a support person can help). Copy only verified originals under `files/.tmp_update/runtime.sh` and `files/.tmp_update/bin/MainUI-*`, when present, back to the identical card paths. Do not blindly copy `before/` or `after/`: they can contain patched files or old app files.

Preserve any unexpectedly modified destination before replacement. Do not erase recovery or delete the app before original system restoration is verified. After originals are restored, use the normal complete-uninstall tool for owned-file cleanup, then safely eject and reboot. Existing development-era backup formats are not interchangeable with release journals.

An optional computer archive under `BetterFavorites-Recovery` contains verified recovery and removed app files. Losing the original installation computer does not invalidate the card-side journal.
