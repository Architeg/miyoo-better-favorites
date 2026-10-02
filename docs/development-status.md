# Development status

Updated: 2026-10-03. The [authoritative roadmap](roadmap.md) contains the complete
feature inventory, accepted defaults, unresolved choices and ordered milestones.
Update roadmap/status whenever a milestone changes or hardware verification arrives.

## Current implementation checkpoint

M4 publication checkpoint: shoulder paging and resilient theme resources.
User hardware acceptance received 2026-10-03; tested binary is identified below.

Previous committed/pushed M3 on `main`:
`1768c7a902ef33513adbd54c290188451652e1be` —
`Scroll selected favorite titles with cached UTF-8 rendering`. Remote main matched.
The user confirms M3 passed hardware testing on 2026-10-03. Accepted pre-heading
binary SHA-256: `12545265ff8508bf6767b1ef3057854d54d513f6610f602490e3c0dcb7f1ce2d`.

The M3 heading-only binary was deployed and verified, then backed up before the
combined M4 test deployment. Its SHA-256 is
`12ab1c2ce572c3462a8ff183f285f177cfa588325162996f0341f204f570dafc`.
The combined M4 acceptance includes the deployed bold-white About headings.

Combined M4 test binary: 347932 bytes, SHA-256
`aa486abad4e3272f86605945109bf0a0f76159ab373f87c1e57c38abd034b328`.
Backup: `../miyoo-better-favorites-backups/20261003-003910-m4-paging-resources-binary/`.
Only the binary was replaced; bytes, ARM format, executable permissions, unchanged
launcher syntax, 30 protected paths and sync passed. Preferences/state, launcher,
Onion integration, favorites and history were preserved. The user accepted this
combined M4 binary on 2026-10-03: “everything works.” M4 is complete; exhaustive
fault combinations remain host fixtures, not implied device measurements. No previews or historical temporary cleanup occurred.

## Verification status

- User-reported hardware results: navigation, OSS/libpadsp sound, A launch and recent
  registration/GameSwitcher visibility, browser MENU, OFF/ON automatic return,
  retained session after switching games, A resume, restored browser position,
  ordinary B exit, SELECT/Y menus and guarded removal.
- The final heavier-font/filled-control readability presentation is now confirmed
  by the supplied 2026-10-02 device photos of SELECT, removal, Settings, About and
  Help. Photo references (personal images are not bundled):
  `AEF91807-3C60-40CC-B474-16F8CAD898A9_1_201_a.heic`,
  `B573A6BF-8591-45DC-879A-70BB32D9359C_1_201_a.heic`,
  `4A3441DD-CB26-4592-A13A-112D653831A2_1_201_a.heic`,
  `59451D38-CC09-47DD-BA96-D69EB7C64CC1_1_201_a.heic`,
  `D5E3B0A7-FBB5-4EFE-88F2-5CA1FA441DC3_1_201_a.heic`. **Browser Settings functionality is now user-reported working on hardware.**
  Presentation feedback (photo `5E91BA02-BF85-4838-BDA8-E4A3A39E626E.heic`) is
  recorded separately. The new inline descriptions/chevrons/About and horizontal
  modal controls have now passed user-reported hardware testing. The final centering,
  text-size and panel-padding adjustments are also accepted with M3. Only the two
  new bold-white About headings await device verification; no exhaustive fault-case
  claim follows.
  The supplied consolidated report records B/START return as
  confirmed; older status classified START as fixture-only. Record START explicitly
  in the versioned device matrix; no exhaustive edge-case claim follows.
- Focused host tests passed: menu/text/navigation, removal/failure/conflict/backups,
  settings/state, launch/recent/rollback, MENU/launcher and runtime ownership,
  installer version/hash/rollback fixtures and shell syntax. Native SDL checks cover
  measured bounds, owned-font preservation, static reading and explicit paging.
- Docker ARM build passed. Existing SDL_ttf `libbz2.so.1.0` linker warning remains.
  Compilation/previews/fixtures are distinct from device verification.
- No measured device startup or RAM baseline. Full Mini/Plus, Onion-version, theme,
  shutdown/restart/interruption/stock-isolation and fault coverage is incomplete.

## Next unfinished milestone

**M5: establish startup and memory baselines before optimizing.** Inspect existing
loader/parser/SDL boundaries and the [runtime memory procedure](onion-return.md).
Use opt-in instrumentation, repeatable cold/warm runs and RSS/PSS/process evidence.
Compare Automatic return OFF/ON; never infer RAM from file size. No speculative
refactoring, Home integration or persistent profiling process.

M4 is complete, including approved L1/R1 pixel paging, non-repeated presses,
selectable/no-wrap clamps, theme decode fallbacks and bounded artwork. See
[acceptance, evidence and retained compatibility checklist](m4-audit.md).

M3 is hardware-accepted; its local timing policy remains explicit rather than a
claim of stock MainUI equivalence. See [implementation and regression checklist](browser-title-scrolling.md).
Home Favorites replacement remains the primary delivery goal after the ordered
core milestones. Optional artwork settings remain unresolved.

## Version reference and detail documents

Version-specific authority: inspected Onion **v4.3.1-1**. Original runtime Git blob
`4e2194f1b47c6c13605846002b6be0b42c1384f9`, SHA-256
`a8d77dcd316bc2a323b1e015aaf4b7682d2fed677af9cdadbc00e48881425d6e`.
Optional session-return integration is separate, default OFF and hash-gated; it
is not Home tile integration. Installation does not enable the app preference.

- [Return lifecycle, installation/rollback, source references and memory procedure](onion-return.md)
- [Removal transaction, Onion semantics and concurrency limits](menu-removal.md)
- [Theme/resource resolution, presentation and preview limitations](menu-presentation.md)
- [Browser Settings, persistence and device acceptance](browser-settings.md)
- [M3 title scrolling, source evidence, policy and device checks](browser-title-scrolling.md)
- [Development environment and roadmap maintenance](development.md)

Builds, previews, logs, personal preferences/state, backups and temporary audit/
patch files remain outside Git. Historical cleanup and deployment are separate tasks.

### Settings presentation details

Settings descriptions now occupy a footer-anchored, theme-derived two-line panel;
rows scroll above it. About uses browser-style “When enabled” / “When disabled”
headings. Supporting descriptions use the resolved section color where readable
on the composited surface, otherwise normal theme text. Font priority, modal
width/actions and all functional protocols are unchanged. Browser Settings
functionality, preceding presentation and M3 are hardware-confirmed. The new
bold-white About headings were included in combined M4 device acceptance.
