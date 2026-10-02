# Development status

Updated: 2026-10-02. The [authoritative roadmap](roadmap.md) contains the complete
feature inventory, accepted defaults, unresolved choices and ordered milestones.
Update roadmap/status whenever a milestone changes or hardware verification arrives.

## Current implementation checkpoint

Browser Settings checkpoint: persistent grouping, numeric prefixes and sorting;
independent display/sort behavior; unchanged return protocol and exact-record
removal. This checkpoint is published on `main` after the prior documentation
HEAD `83828b63b8be20b225f792c6c159556f6f4f5534`.

The user confirms functionality and presentation of deployed binary
`4f857eb6b9d7797cee875302450dbb33251cea32ec66ed5b260dc96047e3a62e`
passed hardware testing. Its deployment backup is
`../miyoo-better-favorites-backups/20261002-232707-fixed-settings-panel-binary/`.
Final adjustments in this checkpoint center the modal heading/title/note, increase
explanation/description text by two points, and reduce the reserved panel padding
from 24 to 16 pixels. These final adjustments are host-checked and ARM-built;
they await device verification. They do not change functional protocols.

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
  text-size and panel-padding adjustments remain hardware-pending; no exhaustive
  fault-case claim follows.
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

**M3: selected long browser-title horizontal scrolling.** M2 functionality and
preceding presentation are user-confirmed on hardware. Final scoped readability
adjustments are included in the Settings checkpoint with a short device follow-up;
they do not block beginning M3 as explicitly requested. M3 must remain a separate
local implementation until reviewed. See [Settings verification](browser-settings.md).

Browser horizontal title scrolling and page-navigation/artwork completion also
remain unfinished. Home Favorites replacement is the original primary goal, but
it is not the only remaining feature or the immediate next implementation.
See [ordered milestones and acceptance criteria](roadmap.md#5-ordered-remaining-milestones).
No page-button assignment or optional setting default is assumed.

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
- [Development environment and roadmap maintenance](development.md)

Builds, previews, logs, personal preferences/state, backups and temporary audit/
patch files remain outside Git. Historical cleanup and deployment are separate tasks.

### Settings presentation details

Settings descriptions now occupy a footer-anchored, theme-derived two-line panel;
rows scroll above it. About uses browser-style “When enabled” / “When disabled”
headings. Supporting descriptions use the resolved section color where readable
on the composited surface, otherwise normal theme text. Font priority, modal
width/actions and all functional protocols are unchanged. Browser Settings
functionality and preceding presentation are hardware-confirmed; only final
centering, two-point text increase and reduced padding remain hardware-pending.
