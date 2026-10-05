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

## Mac metadata and first opening

Read [Mac first open](../packaging/Mac-first-open.html) before launching the unsigned script. Script and helper approvals are separate. Valid bounded AppleDouble metadata beside inventory-owned files **or directories**, and valid Finder metadata within owned directories, are supported across computers. Unknown or modified content and symlinks remain protected. Do not manually remove metadata to bypass a failed check. An incomplete operation reports its recorded change state and a computer-side detailed log; keep recovery backups.

## Interrupted RC6 Windows attempt

An antivirus interruption is not a reason to disable checks or restore a detected executable blindly. Keep the card powered off and preserve its verified recovery. The observed card now has all four audited stock MainUI files and stock runtime, no app folder, and retained project recovery records; host read-only archival preserved those records. No system restoration was needed. This observation does not prove who restored them or that a subsequent device boot passed. Do not retry the Windows package until the detection has been investigated. [RC6 follow-up evidence](release/rc6-followup.md).

## Interrupted app replacement or metadata cleanup

Deleting/re-copying the app leaves system integrations installed. Keep the portable
recovery index and journals. Re-copy the current package, then use Install / Update
to reconstruct a missing app receipt only when verified integration state agrees,
or Uninstall completely to restore and clean up. This repair does not recreate
dismissed Getting started guidance. Do not manufacture receipts or edit system files.

Valid owned AppleDouble metadata can change as macOS replaces its companion.
Uninstall archives it on the computer, restores verified originals, revalidates
metadata and removes only authenticated project content. A failed cleanup is not
a complete uninstall even if restoration succeeded. Keep the card powered off and
retry with its retained recovery; unknown changes remain protected.

## Receipt ownership

An installed Home receipt is generated state, not a stock file. Complete uninstall verifies its absence in both current and legacy app locations. A full reinstall can reconcile a stale receipt only with verified recovery and matching stock runtime/MainUI identities. Missing ownership evidence or foreign bytes are preserved and reported; there is no routine manual-deletion step. [RC7 evidence and limits](release/rc.7.md).
