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
