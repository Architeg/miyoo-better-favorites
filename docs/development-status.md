# Development status

Updated: 2026-10-02. The [authoritative roadmap](roadmap.md) contains the complete
feature inventory, accepted defaults, unresolved choices and ordered milestones.
Update roadmap/status whenever a milestone changes or hardware verification arrives.

## Current implementation checkpoint

Committed and pushed on `main`:
`cf8c840173b45e4d117f77e644f948346c212811` —
`Add themed menus and safe favorite removal`.

Includes SELECT Launch/Remove/Settings/Help, Y full-screen Settings, nested return
explanation, measured theme-aware menus/Help, centered Cancel-first removal modal,
exact-record backup/conflict/atomic safeguards, persistent Automatic return,
selection/viewport restoration and verified A/MENU Onion handoffs.
The final readability pass uses app-owned heavier fonts, filled theme-colored
controls and stronger arrows; shared browser fonts/geometry and audio are preserved.

The checkpoint was rebuilt and only its binary deployed to BetterFavoritesTest.
Deployed binary SHA-256:
`1374fa41d06496df016d7976e044aae9ea9a822a70216ed31ab7463f33ac2527`.
Verified backup: `../miyoo-better-favorites-backups/20261002-212808-ui-checkpoint-binary/`.
Bytes/permissions, unchanged launcher syntax and sync were checked. Runtime,
helper, themes, libraries, favorites, history and settings were not replaced.
This roadmap update changes documentation only; no deployment or implementation.

## Verification status

- User-reported hardware results: navigation, OSS/libpadsp sound, A launch and recent
  registration/GameSwitcher visibility, browser MENU, OFF/ON automatic return,
  retained session after switching games, A resume, restored browser position,
  ordinary B exit, SELECT/Y menus and guarded removal.
- The preceding presentation was confirmed across light/dark themes with photos.
  **Final heavier-font/filled-control micro-adjustments remain device-unverified**
  despite deployment. The supplied consolidated report records B/START return as
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

**M1: verify final readability on device and close the UI checkpoint.**
The next implementation milestone is **M2: complete persistent browser Settings
for grouping, numeric-prefix display and sorting**. Their accepted defaults are
ON, shown and OriginalLabel sorting. Fields/parser support do not constitute
usable Settings controls; grouping is still unconditional.

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
- [Development environment and roadmap maintenance](development.md)

Builds, previews, logs, personal preferences/state, backups and temporary audit/
patch files remain outside Git. Historical cleanup and deployment are separate tasks.
