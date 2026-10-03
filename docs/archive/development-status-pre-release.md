# Historical pre-release status

Preserved 2026-10-03. Current release status supersedes next-step/install claims below.

# Development status

Updated: 2026-10-03. The [authoritative roadmap](roadmap.md) contains the complete
feature inventory, accepted defaults, unresolved choices and ordered milestones.
Update roadmap/status whenever a milestone changes or hardware verification arrives.

## Current implementation checkpoint

Published/pushed M4 on `main`: `be00372ce53b4b2e8f4d48d1974335ffa932034c` —
`Add shoulder paging and resilient theme resources`. Remote main matched.
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
- Limited pre-cache startup and idle RSS/PSS evidence is recorded below. Full Mini/Plus, Onion-version, theme,
  shutdown/restart/interruption/stock-isolation and fault coverage is incomplete.

## Next unfinished milestone

**M6 Home Favorites redirect: user hardware-accepted, local/uncommitted.**
Miyoo Mini Plus, Onion v4.3.1-1; device revision/firmware unknown. OFF stock Favorites,
ON Better Favorites with B Home/no-loop twice, Apps/X/Y and A/MENU/GameSwitcher/
Automatic return passed. Actual tested app SHA-256:
`d637643b0be987ab68a7bb9ba6869f09b6e1e58e2e7d0919088bbb1cd54f19dc`.
The native log identifies 354-clean. All six deployed hashes, raw archive and
missing log coverage are in [M6 acceptance](m6-acceptance.md). Diagnostics were
archived/byte-verified, disabled and synced; logs/backups/preferences remain.
No new speed/RAM claim follows. The cache-containing build's normal browsing/game
behavior is now user accepted, without a measured cache improvement.

Later local changes provide concise Home wording, explicit availability and safe
mixed-file recovery; their new UI/recovery has only host verification. See
[installation, compatibility and offline recovery](m6-home-integration.md).
[Independent L1+Y investigation](m6-shortcut-investigation.md) is separate: input
conflicts and incompatible stock keymon command remain blockers; no shortcut is
installed, available or hardware-accepted. No shortcut instructions are exposed
in production Help. M7 remains the next ordered roadmap milestone; shortcut work
is a separately requested investigation, not a replacement of Apps/Home.

### M5 pass closed with explicit deferrals — 2026-10-03

Seven available pre-cache startup archives validate success and cleanup. Original
pilot post-boot 505.361 ms, warm median 267.590 ms (n=3); corrected post-boot
505.596 ms, warm median 271.043 ms (n=2, fourth archive absent). Original hashing
and remaining preparation/instrumentation/cache biases prevent unbiased cold or
causal gain claims. Idle ON sampling validates three samples per role: browser
smaps RSS 19,308 kB / median PSS 17,449 kB; runtime RSS 1,808 / PSS 357 kB.

Gameplay memory, ON/OFF comparisons, post-scroll/menu memory and gameplay process
absence measurements are **deferred**, not verified and not prerequisites for
M6. No more profiling sessions, Terminal commands or device checks are requested.
See [measurements, limits and verified retirement archive](m5-profiling.md).

The isolated per-parse emulator-label cache is host-verified: output/failure/reload
checks pass and config reads fall 70 → 3 on the current corpus. ARM build and
regressions pass. The reviewed cache binary is deployed, with device speedup unverified. Normal browsing/game behavior of the later
M6 cache-containing build is now user accepted. Audio, font ownership,
rendering and broader cleanup are unchanged. See [cache review](m5-emulator-label-cache.md).

Exact production launcher restored; profiling hooks/collectors/evidence/card
rollback copies were archived and verified before removal. Cache binary retained,
profiling disabled, saved data and permanent Onion return integration preserved.
Host archive: `/Users/valeriybagrintsev/IT Projects/miyoo-better-favorites-backups/20261003-034734-m5-retired-card-archive/`.
M5 closure did not retrospectively accept its cache build. Later M6 acceptance
now covers the actual deployed cache-containing bytes, without a speedup claim.

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
