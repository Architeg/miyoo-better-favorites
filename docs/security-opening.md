# First opening downloaded computer tools

Use the expected package and verify its checksums. These executables are not signed with a trusted publisher identity. File-specific approval is a user decision, not an installer action.

## macOS Monterey and newer

**Before opening the script:** read the [offline Mac first-open guide](../packaging/Mac-first-open.html), included at ZIP root and in the app folder. Two files can require separate approval. macOS may ask you to approve this unsigned installer. Download it only from the official Better Favorites release. Checksums detect byte changes; they are not a security audit.

1. Open `Install-macOS.command` from the copied app folder. For an unidentified developer warning, use the file-specific approval offered by macOS.
2. Monterey: **System Preferences → Security & Privacy → General → Open Anyway**. Newer macOS: **System Settings → Privacy & Security → Open Anyway**. Recent versions may require this Settings route instead of Control-click Open.
3. The script can open Terminal while its separate executable is still blocked. Follow the terminal's exact **BetterFavorites-Installer** path and approve that file separately. You can locate it in Finder with Go → Go to Folder. Keep the terminal open, then type **r** to retry after approval. Every retry rechecks bytes.
4. The helper is retained in `~/Library/Caches/BetterFavorites/<executable SHA-256>/BetterFavorites-Installer`. Subsequent install/uninstall uses the same bytes/path.

[Apple's current instructions](https://support.apple.com/en-us/102445) and [version-specific Mac guide](https://support.apple.com/guide/mac-help/open-a-mac-app-from-an-unknown-developer-mh40616/mac) explain file-specific exceptions. Do not disable Gatekeeper globally. Ad-hoc signing does not establish a trusted developer identity.

## Windows

For **Windows protected your PC / Microsoft Defender SmartScreen prevented an unrecognized app**, **More info → Run anyway** is applicable only when that button is offered and you trust the verified file. A downloaded file may instead offer **Properties → General → Unblock** for that file. The initial batch entry and selected executable are separate files; approval of one does not prove approval of all children.

Smart App Control, antivirus detections and organization policies are different blocks. They may have no per-file override. Stop and report the exact message; do not disable protection or upgrade PowerShell to bypass it. Microsoft describes [App & browser control](https://support.microsoft.com/en-au/windows/security/windows-security/app-browser-control-in-the-windows-security-app) and [Smart App Control limitations](https://support.microsoft.com/en-us/windows/security/threat-malware-protection/smart-app-control-frequently-asked-questions).

## Linux

Use the desktop's **Allow launching/Trust** for `Install-Linux.desktop`. If your desktop has no such support, run `sh ./Install-Linux.sh install` in the copied app folder. Do not change global security policy. The SD executable bit is not required by the staged native tool route.
