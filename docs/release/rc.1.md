# v1.0.0-rc.1 review candidate

Prepared from a pinned source commit; exact identities are in packaged
`package.json`, app `release.json` and external `SHA256SUMS`. No stable tag/release,
candidate deployment or change to the accepted mounted card is part of this pass.

## Established evidence

- Accepted M6: Mini Plus MY354, firmware 202306282128, Onion v4.3.1-1; revision unknown.
  OFF stock Favorites, ON app/B Home without loops twice, Apps/X/Y and
  A/MENU/GameSwitcher/return passed. [Actual accepted hashes/log limits](../m6-acceptance.md).
- App regressions, shell syntax, ARM build, actual compiled adapter fixtures and
  exact Go-generated MainUI/runtime output checks.
- Whole-package host fixture checks exercise the actual extracted candidate:
  install/update with personal files, uninstall/stock/reinstall, refusal,
  publication/rollback and interrupted/foreign restore, bounded diagnostics.
- Actual ZIPs and their packaged self-contained installer executables passed the
  roundtrip/refusal/recovery fixture on **macOS arm64** and **Linux amd64 in the
  existing isolated Docker image**. Native Mac/Linux Go tests passed whole-package
  write/rollback, accepted-helper upgrade, staged-space simulation and preservation.
  Native SDL checks-only passed; no images were generated.
- Native Windows self-contained executable and Windows tests cross-compile.
  **No Windows execution/SD-reader acceptance follows.**

## Remaining acceptance gates before stable/public distribution

1. Execute the native Windows tests and actual extracted ZIP roundtrip on supported
   Windows/FAT SD reader. Verify reparse/link refusal, low-space/write failure,
   restore and conflict preservation. Qualify downloaded unsigned host launchers
   (macOS quarantine and Windows SmartScreen) on the intended distribution route.
2. Verify bundled prebuilt library/source correspondence and license closure,
   particularly custom OSS SDL and SwiftShader/dependency transitive components.
   Full pinned source material/notices are supplied; an existing source pin does
   not prove how each prebuilt binary was produced. Candidate remains private
   review material until this audit is complete. No ROMs/vendor MainUI are shipped.
3. Device-test the **final ZIP bytes**: app-only fresh install/update → optional
   Install → Home OFF/ON/B twice → Apps/X/Y → A/MENU/return → Uninstall → stock
   Favorites/GameSwitcher → Reinstall. Check empty icon/native fallback, latest
   wording/logging and unreadable/full-log behavior. Preserve backups first.
4. Confirm normal audio/theme/removal/settings/state on those candidate bytes.
   Previously accepted development binaries do not accept a rebuilt candidate.
5. Request a Mini/community tester; otherwise keep Mini hardware status unverified.
   Unknown versions/MainUI patches remain refused, not testing invitations to bypass.
6. Preserve candidate/source/checksums and publish those exact accepted bytes.
   Rebuild differences require a new identity and suitable checks; only then tag
   and publish stable v1.0.0. Screenshots/banner await user-supplied materials.

No additional profiling, preview batch or shortcut work is required for these gates.

### Host package procedure

`python3 tests/release_package_test.py --release <candidate-directory> --fixtures
<private-audited-fixture>` verifies archive/entry hashes and executes the actual
packaged native host tool. Tests preserve personal files, require latest update
recovery identity, exercise stock/patched mixtures and refuse foreign changes.
Go fault fixtures additionally simulate stage/no-space, publication and rollback
failures. Those simulations are not physical card/reader power-loss tests.

The ARM build passes with the existing SDL_ttf `libbz2.so.1.0` link warning and
GCC ABI notes; no new app warnings remain. The card contains bzip2, but this does
not replace the final device package dependency check. All nine packaged private
libraries match the accepted card bytes; audio has not been swapped. Runtime
script and generated MainUI outputs remain identical to the accepted hashes;
only candidate return trace gating/helper manifest and app/launcher wording/logging
change. No candidate was deployed.

### Evidence received 2026-10-04

[User-confirmed RC1 Mac/device acceptance](rc1-mac-acceptance.md) matches these exact
package hashes. Mounted-card stock restoration is independently verified; device
boot after uninstall remains pending. RC1 integrations-only uninstall retained the
app/artifacts; the authorized subsequent clean baseline archived and removed them.
[RC2](rc.2.md) corrects default complete removal and contains separate new gates.
