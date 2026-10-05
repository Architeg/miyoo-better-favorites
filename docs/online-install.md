# Install from Terminal on Mac or Linux

[Install](install.md) · [Uninstall](uninstall.md) · [Opening help](security-opening.md)

This is the recommended Mac route and an optional Linux route. It downloads the same install ZIP, verifies its checksum and opens the packaged installer. No manual copying is needed. The [offline copy-and-click route](install.md#windows--linux-or-offline-mac-copy-then-click) remains available.

## Open the menu

1. Power off the Miyoo and connect its SD card.
2. Open Terminal on Mac (**⌘ Space → Terminal**) or your Linux terminal.
3. Paste:

```bash
curl -fsSL https://raw.githubusercontent.com/Architeg/miyoo-better-favorites/main/scripts/install-online.sh | bash
```

4. Confirm the detected card, or enter its location if detection is ambiguous.
5. Choose **[1] Install / Update**, **[2] Uninstall completely**, **[3] Export diagnostics**, or **[0] Cancel**.
6. Keep the card connected until verified success, then safely eject and boot.

The command currently selects **v1.0.0-rc.7**. ZIP and checksums come from the same release. It delegates all card changes to the existing installer; it does not bypass ownership, recovery or compatibility checks.

## Use it again or offline

Run the same command for update, uninstall or diagnostic export. Downloaded packages are retained in fresh folders under `~/BetterFavorites-Downloads/` and can be reused offline. The installed app's computer launcher also opens the same menu.

To request complete uninstall directly:

```bash
curl -fsSL https://raw.githubusercontent.com/Architeg/miyoo-better-favorites/main/scripts/install-online.sh | bash -s -- uninstall
```

Replace `uninstall` with `export-diagnostics` for export. Old development installations may require their matching restoration procedure; a newer installer is not a universal cleanup tool.

## Mac opening behavior

This route may avoid the repeated approvals associated with browser downloads and Finder extraction. It does not sign/notarize tools, remove existing quarantine or disable protection. Managed policies and other checks can still block execution. Follow [file-specific opening help](security-opening.md), and stop for malware or damaged-file warnings.

Checksums detect corruption; they are not an independent publisher signature or safety audit. Ordinary ownership or installation failures are not security-approval problems.

## Requirements

- macOS Monterey (12)+, Intel or Apple Silicon; Rosetta entry supported.
- Linux x64/ARM64 meeting the [packaged host minima](release/host-dispatch.md).
- Bash, `curl`, `unzip`, `zipinfo`, `awk`, `mktemp`, and `shasum` or `sha256sum`. Mac supplies these; Linux may need its distribution's curl/unzip packages.
- An interactive terminal, mounted card and Internet for downloading. No sudo, Python or developer tools are required.

Host probes and ZIP layout/checksums are validated before execution. The selected native backend then repeats transport/payload and card validation. [Contributor tests](../CONTRIBUTING.md#run-ordinary-checks-first).
