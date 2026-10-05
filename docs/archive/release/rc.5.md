> **Historical engineering record — not current installation instructions.**
> Use the [installation guide](../../install.md) and [compatibility matrix](../../compatibility.md) for the released app.

# RC5 installation correction — historical build

> Historical engineering record. Use [current installation instructions](../../install.md) and [release notes](../../release/notes-rc.7.md).

Identity: **1.0.0-rc.5**, one `better-favorites-1.0.0-rc.5.zip`. It keeps the
copy-to-card → click-to-install workflow. No UI, audio, runtime or MainUI adapter
behavior changes; existing tags and earlier candidates remain intact.

## Focused fixes

- Validated AppleDouble sidecars beside authenticated package/app files no longer
  stop install, update or legacy migration. Finder `.DS_Store` is accepted only
  in directories containing authenticated project files. The same platform-neutral
  checks apply to complete uninstall and owned recovery/backup cleanup.
- Metadata is never executed or merged into payload files. Payload hashes are
  still mandatory. Links/reparse points, invalid metadata, orphan sidecars,
  unrelated hidden files and unknown directories are preserved with a failure.
  After partial cleanup, a missing companion is allowed only when that exact
  installed path is authenticated by the verified recovery journal.
- Metadata containers are bounded to 4 MiB. AppleDouble v2 entry tables are checked
  for sizes, bounds, duplicate IDs and overlap; Finder `Bud1` allocator/block
  tables, DSDB root and free-list bounds are checked. Attribute values and Finder
  records are opaque, never applied. Unrecognized formats fail closed.
- Complete uninstall archives validated metadata alongside owned files before
  removal. Shared directories and unrelated Finder metadata remain untouched.
- Ordinary helper failures retain their actual reason and exit status, with no
  security retry. Status 137 is accurately reported as a possible SIGKILL; only
  that context offers existing file-specific approval/retry, with byte checks.

## Evidence and limits

[RC4 outcome](rc.4.md#confirmed-rc4-installation-outcome): helper approval/menu/card
identification succeeded; installation failed on reported metadata. No RC4 device
installation acceptance is claimed. The reported sidecar was absent on the mounted
card during inspection, so its actual format remains unverified.

Regression fixtures include AppleDouble produced by macOS `ditto` from a benign
attribute and a Finder-produced allocator with personal filename records removed.
An unchanged local Finder file was also validated read-only. Clean ZIP tests are
augmented with metadata inserted after copying, including nested project folders.
Shell status simulations do not establish Gatekeeper GUI behavior. Cross-platform
source/build tests do not establish new Windows or Miyoo execution acceptance.

Final preparation report and `SHA256SUMS` identify the exact source commit, app,
all host helpers, ZIP and matching source/license companions. No mounted-card
writes, deployment, tag changes or release publication are part of this fix.

## Remaining acceptance

1. Extract/copy this exact ZIP on Mac, approve only verified files if required,
   install without manual metadata cleanup; verify both switches default OFF.
2. Exercise the existing Home/Apps/A/MENU/Automatic return device checks.
3. Update and complete uninstall, including a Mac-copied card on Windows/Linux;
   verify stock boot, then reinstall. Keep recovery archives.
4. For a real validation failure, confirm its reason remains visible and no
   Gatekeeper retry is offered. Never override malware/damaged/policy warnings.

The source regression suite covers these filesystem/error cases in temporary
fixtures. Device acceptance remains pending for the corrected candidate.

## Host verification

Focused format/ownership tests passed against the `ditto` sidecar fixture and an
unchanged local Finder file. Actual install/update/migration/uninstall fixtures
passed, including nested metadata, verified recovery mirrors, invalid/orphan
sidecars, unrelated hidden files, modified payloads, links, and partial/interrupted
cleanup. A native Mac-installed fixture was completely uninstalled by the packaged
Linux executable in a container, consuming the same journal and Mac metadata;
this is not physical Linux reader or new Windows execution acceptance.

Wrapper simulations cover status 1 with the actual reported reason, other ordinary
failure statuses, possible status-137 termination, controlled retry at the same
verified cache path and tamper refusal. Existing application regressions and shell
syntax checks passed. ARM build passed with the existing libbz2 linker warning and
GCC ABI notes. No new device, Gatekeeper GUI, memory or performance claim is made.
