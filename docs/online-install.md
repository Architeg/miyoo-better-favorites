# Alternative: install from Terminal on Mac or Linux

[Install guide](install.md) · [Uninstall](uninstall.md) · [README](../README.md)

The normal copy-to-card installer remains available. This alternative downloads
the **same ready-to-install ZIP**, checks its SHA-256, and opens the existing
installer menu. You do not need to extract or copy the folder yourself.

## Public download command

**While this repository is private, the following public command cannot download
it.** Use the authenticated route below for private testing. This command becomes
usable when the repository and release are public; it does not change visibility.

1. Power off the Miyoo and connect its SD card to your computer.
2. Open Terminal and paste:

```bash
curl -fsSL https://raw.githubusercontent.com/Architeg/miyoo-better-favorites/main/scripts/install-online.sh | bash
```

3. Confirm the detected card. If none or several are found, enter its location.
4. The usual menu opens: **1 Install / Update**, **2 Uninstall completely**,
   **3 Export diagnostics**, or **0 Cancel**. Choose an option and follow its prompts.
5. After verified success, safely eject the card. Installation/restoration takes
   effect after booting the Miyoo.

The current downloader selects **v1.0.0-rc.7** explicitly, because it is a prerelease.
It does not use GitHub's latest-stable link or silently select a different build.

## If Mac security approvals were getting in the way

Browser downloads and Finder extraction can apply download quarantine. Apple's
[trusted-execution explanation](https://developer.apple.com/forums/thread/706442)
documents that `curl` and command-line `unzip` do not apply/propagate it in that
way. This Terminal route may avoid the repeated first-open approvals you see
with the browser ZIP.

It does **not** sign or notarize the installer, remove existing quarantine, or
disable Mac protection. Other checks and managed policies can still block it.
An ownership/checksum error is an installation error, not a security approval
problem. Keep its log and recovery files. [Opening guide](security-opening.md).

## Downloads, updates and complete removal

Verified packages are retained in a fresh folder under
`~/BetterFavorites-Downloads/`. They can be reused offline. The app and both
supported integrations are installed through the existing backend; fresh switches
remain **OFF** and updates preserve saved preferences. Portable recovery stays on
the card, so it remains usable from a different computer.

To open the menu again, run the same command, or use the computer launcher in
the installed `App/BetterFavorites` folder. To select an action directly:

```bash
curl -fsSL https://raw.githubusercontent.com/Architeg/miyoo-better-favorites/main/scripts/install-online.sh | bash -s -- uninstall
```

Use `export-diagnostics` instead of `uninstall` to select diagnostic export.
For an older installation, keep its matching offline tools or specify the
matching full-package release with `--tag vX.Y.Z`. Do not assume a new installer
can remove every historical development build. No files are deleted by the
downloader; complete removal and restoration belong to the verified backend.

## Private prerelease testing

The optional [GitHub CLI](https://cli.github.com/) must already be authenticated
with access to this repository. The downloader never asks for a token or prints
credentials. From an updated checkout:

```bash
bash scripts/install-online.sh --github-auth
```

Or download and run its readable script through that authenticated connection:

```bash
gh api repos/Architeg/miyoo-better-favorites/contents/scripts/install-online.sh -H 'Accept: application/vnd.github.raw+json' | bash -s -- --github-auth
```

The user ZIP and `SHA256SUMS` come from the same explicitly selected release.
Private access is an optional development route, not a new prerequisite for the
public installer or the offline package.

## Host requirements and checks

- macOS Monterey (12) onward: Intel, native Apple Silicon and Rosetta entry.
- Linux x64 or ARM64; the packaged wrapper checks its existing kernel minima.
- Bash, `curl`, `unzip`, `zipinfo`, `awk`, `mktemp`, and `shasum` or `sha256sum`.
  Mac includes these; Linux users may need their distribution's unzip/curl packages.
- An interactive Terminal and mounted Onion card; no sudo, Python, compiler or
  separately downloaded bootstrap executable is required.

The downloader selects the platform wrapper. That unchanged wrapper selects the
native executable and repeats OS/architecture checks. It refuses unsafe ZIP
members, duplicate/missing checksums, links and unsupported layouts before running
the package. The backend verifies transport/payload checksums and card identities
before publishing changes. Checksums downloaded beside a ZIP detect corruption;
they are not an independent publisher signature.

Developer fixtures run without network or a real SD card:

```bash
bash -n scripts/install-online.sh
python3 tests/online_install_test.py -v
# Optional: native Linux menu/Cancel check using an existing full package.
python3 tests/online_install_test.py -v --package /path/to/better-favorites.zip
```

Host probes are simulated in isolated test copies, including Intel with a missing
optional translation key, native ARM and Rosetta. Production accepts no host or
URL overrides. Fixture tests and Linux-container execution are separate from
physical Mac/Linux installation and public-download acceptance.
