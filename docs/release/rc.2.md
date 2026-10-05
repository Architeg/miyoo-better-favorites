# v1.0.0-rc.2 — historical build

> Historical engineering record. Use [current installation instructions](../install.md) and [release notes](notes-rc.7.md).

Prepared 2026-10-04. RC1 artifacts/evidence/recovery originals are retained.
[RC1 Mac/device acceptance](rc1-mac-acceptance.md) is distinct from RC2 acceptance.

## Changes

- Only the three How to Open headings use the existing bold-white heading treatment.
  Automatic return uses explicit Better Favorites/main menu destinations with inline
  B/START badges. No other app presentation or behavior is changed.
- Exact supplied root icon.png is included in both packages; config uses Onion's
  existing absolute SD-path icon mechanism. Original artwork remains in the repository root.
- Action-only install/uninstall/export commands prompt for missing card/confirmation,
  optional patches and recovery. Valid recovery is selected only when unambiguous.
- Default uninstall restores and verifies stock files, archives recovery and removed
  files outside the card, then removes verified owned app/preferences/logs/artifacts.
  Foreign/unknown files fail preflight; interrupted cleanup can be retried. The
  explicit remove-integrations/restore actions retain app/data. No shared resource
  directories or game data are removed. Wrappers retain the executable exit status.
- Unified Windows dispatch selects isolated Go1.20.14 x86/x64 for Windows7/8/8.1
  and native modern x86/x64/ARM64 for newer Windows. Mac supports native Intel and
  Apple Silicon with Monterey minimum, including Rosetta dispatch. Linux x64/ARM64
  linkage is verified static. See [host targets and evidence](host-dispatch.md).
  The normal module remains go 1.24 and normal executables use Go 1.26.2. An equivalent
  zeroing loop replaces the newer clear builtin; ELF/output equivalence is tested.

## Identity and checks

RC2 is committed at `a31f9a2fff42d12b2a9f2cc16118f3b8a43eb94a` and pushed to main. Annotated `v1.0.0-rc.2` remains at that commit. Its prerelease is an unpublished, asset-free draft. Follow-up copy-and-click work has a separate RC3 identity and does not replace these bytes.

## Acceptance received

The user passed packaged installation/use/uninstallation on Windows7 SP1 x64 and
Windows10 x64 and confirmed stock Onion boot. Mounted-card stock hashes and cleanup
are independently verified. [Diagnostic analysis](windows-acceptance.md) identifies
the earlier RC2 payload; it does not establish an exact later dispatcher/ZIP hash.
Mac RC1 acceptance remains separate. Ordinary success does not qualify fault tests.

## Historical RC2 qualification notes

- RC2 device install → Home OFF/ON/B → Apps/X/Y → A/MENU/return → complete uninstall
  → actual stock boot/behavior → reinstall, using these final package bytes.
- Exact later host-compatible ZIP/dispatcher identification, other native Windows
  versions/architectures/readers, reparse/device path refusal and write/space/rollback
  fault checks. Normal Windows7 SP1 x64 / Windows10 x64 passes are recorded above;
  cross-compilation and host fault fixtures are not those device/reader outcomes.
- Unsigned distribution/quarantine/SmartScreen qualification.
- Prebuilt library/source correspondence and transitive license closure before public
  redistribution; companions/notices do not alone close that audit.
- Mini/community device testing; hardware evidence currently covers Mini Plus only.

The current focused acceptance procedure is [RC3](rc.3.md); community targets are not a requirement to repeat a broad matrix. This historical list does not imply the existing RC2 tag/draft are still future work. No stable release or new profiling/shortcut work is authorized.

## Separate bootstrap preparation

[Download-and-run bootstrap](../../tools/bootstrap/README.md) delegates every card
operation to the existing offline installer. Local download/archive/tag fixtures
and cross-builds are separate evidence. Public URLs are not offered; an actual
published-asset roundtrip and native Windows7 TLS execution remain pending.
The offline one-command entry scripts remain the supported route.

