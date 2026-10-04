# First opening downloaded computer tools

Use the expected package and verify its checksums. These executables are not signed with a trusted publisher identity. File-specific approval is a user decision, not an installer action.

## macOS Monterey and newer

1. Open `Install-macOS.command` from the copied app folder. For an unidentified developer warning, use the file-specific approval offered by macOS.
2. Monterey: **System Preferences → Security & Privacy → General → Open Anyway**. Newer macOS: **System Settings → Privacy & Security → Open Anyway**. Recent versions may require this Settings route instead of Control-click Open.
3. The script can open Terminal while its separate executable is still blocked. Follow the terminal's exact **BetterFavorites-Installer** path and approve that file separately. You can locate it in Finder with Go → Go to Folder. Keep the terminal open, then type **r** to retry after approval. Every retry rechecks bytes.
4. The helper is retained in `~/Library/Caches/BetterFavorites/<executable SHA-256>/BetterFavorites-Installer`. Subsequent install/uninstall uses the same bytes/path. Approval persistence on a fresh downloaded candidate still needs user verification; a new hash is a different file and may require approval again. No automatic quarantine stripping occurs.

A rejected command is reported as a failure, not proof of Gatekeeper: the same retry prompt can follow an ordinary installer error. Read the actual error. Do not approve malware, damaged-file or managed-policy blocks as ordinary unsigned prompts. The retained cache is host-only and contains no recovery originals; keep all separate recovery archives.

[Apple's current instructions](https://support.apple.com/en-us/102445) and [version-specific Mac guide](https://support.apple.com/guide/mac-help/open-a-mac-app-from-an-unknown-developer-mh40616/mac) explain file-specific exceptions. Do not disable Gatekeeper globally. Ad-hoc signing does not establish a trusted developer identity.

## Windows

For **Windows protected your PC / Microsoft Defender SmartScreen prevented an unrecognized app**, **More info → Run anyway** is applicable only when that button is offered and you trust the verified file. A downloaded file may instead offer **Properties → General → Unblock** for that file. The initial batch entry and selected executable are separate files; approval of one does not prove approval of all children.

Smart App Control, antivirus detections and organization policies are different blocks. They may have no per-file override. Stop and report the exact message; do not disable protection or upgrade PowerShell to bypass it. Microsoft describes [App & browser control](https://support.microsoft.com/en-au/windows/security/windows-security/app-browser-control-in-the-windows-security-app) and [Smart App Control limitations](https://support.microsoft.com/en-us/windows/security/threat-malware-protection/smart-app-control-frequently-asked-questions).

## Linux

Use the desktop's **Allow launching/Trust** for `Install-Linux.desktop`. If your desktop has no such support, run `sh ./Install-Linux.sh install` in the copied app folder. Do not change global security policy. The SD executable bit is not required by the staged native tool route.

## Observed Mac failure (2026-10-04)

The supplied Keka/Finder screenshots show the script's approval followed by a blocked generic `installer` executable. The installation menu was not reached. Read-only inspection found quarantine on both card files; the ARM64 Mac tool was linker ad-hoc signed with no TeamIdentifier. No valid signing identities were available. RC4 fixes retention/naming and eliminates the second temporary executable dispatch; it does not claim notarization or fresh-download GUI acceptance.
