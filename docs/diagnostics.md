# Export diagnostics

1. Power off the Miyoo and connect its SD card to your computer.
2. Open the computer launcher in `App/BetterFavorites`, or use the [Mac/Linux Terminal command](online-install.md).
3. Choose **[3] Export diagnostics**.
4. The tool prints the new ZIP's location on your computer. Review it before sharing.

No Miyoo Terminal, network connection on the device or development tools are needed. Export does not change preferences or enable detailed tracing. Existing exports are kept.

## What is included

The report contains bounded app/installer log tails, selected settings/status, version metadata, system/helper hashes and the active theme identifier. At most two installer log tails are included, each up to 128 KiB.

**Review before uploading.** Game filenames, theme paths and preferences may be personal. ROMs, BIOS, saves, favorite/history contents, credentials and private recovery originals are not collected. Never upload a complete card image.

Add known device/firmware details and the computer OS/version to your report. Include the tested ZIP checksum when possible. An offline report cannot prove a device boot, measure RAM or identify every host executable used.

## If a report is missing information

Share the exact error, what action you chose and the last completed step. Missing detailed logs are expected when tracing is OFF; they do not establish whether an interaction happened.

Basic logs rotate current/previous sessions and are bounded. Detailed tracing stays OFF normally. Enable it only for a specific support request, then disable it. [Developer logging and commands](integrations.md#diagnostics).
