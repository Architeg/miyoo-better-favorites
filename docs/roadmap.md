# Development roadmap

Authoritative product and delivery roadmap, updated 2026-10-02. Reconstructed from
the user-supplied consolidated report and checked against implementation
`cf8c840173b45e4d117f77e644f948346c212811` (`Add themed menus and safe favorite removal`).
The report's intended behavior is requirements evidence; source inspection is
implementation evidence. Neither is a substitute for recorded device tests.
See [current checkpoint/status](development-status.md) for operational progress.

**Maintenance:** update this roadmap and development status whenever a milestone
changes or hardware verification is received. Record the tested commit, device,
Onion version, theme and observed cases; do not promote host coverage to hardware
verification. Keep detailed milestone specifications here and concise status there.

## 1. Product goal and wrapper architecture

The original primary goal is **Onion Home → Favorites → Better Favorites → Onion
game launch**, using a stock-like native browser grouped by console. The current
Apps/BetterFavoritesTest entry is a development entry, not completion of that goal.
A direct Home shortcut is an alternative to investigate, not an agreed replacement
for the normal Favorites tile requirement.

Better Favorites is a C++/SDL2 presentation and favorites wrapper. Onion's
`/mnt/SDCARD/Roms/favourite.json` remains the source of truth and stock Add to
Favorites remains the adding workflow. The wrapper resolves system labels from
Onion emulator configuration, builds browser rows, displays stored labels/artwork,
and hands game execution to Onion. Systems are derived from launch paths/configs;
there is no promised hardcoded console list.

Layers:

1. **Data/model:** `favorites_parser.cpp`, `ui_rows.cpp`, `navigation.cpp`; exact
   source records are retained for guarded removal, separate from display/sort keys.
2. **Browser and menus:** SDL2 rendering/input in `main.cpp`, `menu_state.cpp`,
   `menu_renderer.cpp`, `menu_text.cpp`; theme resolution in `theme_loader.cpp`.
3. **App preferences/state:** `app_settings.cpp`, `browser_state.cpp`; these are
   app state, not a second favorites/history database.
4. **Onion handoff:** `launch_request.cpp` and the app launcher. Private requests
   leave SDL/audio first; exit 20 requests a game and exit 21 GameSwitcher.
5. **Optional return integration:** separately installed, hash-gated runtime hook
   and session helper in `integration/onion-return/`. Installation is distinct from
   enabling the default-OFF preference. No app/helper process stays alive in play.
6. **Future Home entry:** separate reversible integration, still unimplemented.
   Return integration does not supply a Home tile entry hook.

Onion retains emulator/core choices and overrides, save/resume, activity tracking,
recent lists and GameSwitcher. Directly running emulator `launch.sh` from the SDL
loop is not an equivalent lifecycle. See [architecture](architecture.md),
[Onion return/handoff details](onion-return.md) and [removal safeguards](menu-removal.md).

## 2. Non-negotiable rules

- Keep the original favorites file and stock Add to Favorites workflow. Do not
  generate a duplicate database, collections or rewritten labels to simulate grouping.
- The original read-only rule was deliberately amended for required v1 removal:
  remove only the selected exact record after Cancel-first confirmation, verified
  backup, conflict checks and same-filesystem atomic publication. Preserve every
  unrelated record/field/byte. Never remove ROMs, artwork, saves or recent history.
- Preserve stored launch/ROM paths, including `/../../`. Validate resolved files
  without normalizing the strings handed to Onion. Keep shell quoting and Onion's
  actual command parser safe; broaden filename support only after end-to-end proof.
- A normal exit, menu visit or browser MENU press must not register the selected
  game in history. A launch registers the compact type-5 record in Onion's visible
  or hidden recent list according to `.showRecents`, preserving existing records.
- Private staging, invocation ownership, failure checks and rollback must preserve
  unrelated commands/history changes. Existence checks alone are not race protection.
  Finish SDL/audio cleanup before publishing/handoff. Do not revive MainUI PID
  stop/kill assumptions that were disproved by the verified Apps launch context.
- Keep verified browser geometry/navigation and working OSS/libpadsp audio unless
  an explicitly scoped change is authorized. Do not replace the audio backend as
  a cleanup or performance shortcut.
- Reuse compatible active-theme assets, fonts, colors and system resources.
  Preserve the user's theme configuration. Missing resources use measured,
  theme-derived fallbacks; no fixed unrelated palette or new bundled artwork.
  Drawn menu text-control labels are distinct from decorative images: `hideIcons`
  still governs decoration, `hideHints` hides menu footers. Browser preferences
  remain separate and unchanged.
- Preserve session return, shutdown precedence and loop prevention. Clear ownership
  on normal MainUI/direct game return, restart/reboot, disable/generation change,
  and before reopening. Switching via GameSwitcher retains the originating session.
  Stock-menu launches must remain unaffected. No permanent preload daemon is agreed.
- Prefer reversible SD-card/Onion mechanisms. No firmware/NAND modification is
  agreed. Back up/verify before installation, and refuse unknown versions/hashes
  until their compatibility is established.
- Do not equate compilation, fixtures, desktop previews or file sizes with device
  behavior, universal theme support, timing or RAM measurements.

## 3. Complete feature inventory and evidence

Status vocabulary: **hardware-confirmed** means reported device observation;
**awaiting device verification** means implemented with host/build evidence only
for the stated change; **partial** means foundations exist but usable behavior or
coverage is missing; **planned** means no complete implementation; **deferred**
and **superseded** do not add obligations to the active v1 milestone sequence.

### Implemented and hardware-confirmed

M3 selected-title scrolling passed user hardware testing on 2026-10-03. Host timing,
UTF-8 clipping and cache tests are additional evidence, not exhaustive device coverage.

Browser Settings functionality (persistent grouping, numeric-prefix display and
sorting) and presentation are user-confirmed on deployed build `4f857eb6`.
Only the final centering, description-size and panel-padding adjustments await
device follow-up;
this does not imply exhaustive fault-injection or theme coverage.

Final readability confirmation: supplied 2026-10-02 device photos show the deployed
SELECT menu, removal modal, Settings, About and Help. This is screen-specific
evidence, not an exhaustive theme/fault/lifecycle matrix. Photo identifiers are
recorded in [status](development-status.md#verification-status).

The supplied report and preceding device reports confirm the following major
behaviors. This does not imply every failure case/device/theme/version was tested.

| Feature | Implementation/evidence |
| --- | --- |
| Native SDL2 grouped browser | Existing app; stock-like layout reported working |
| Existing favorites/stock adding workflow | Reads original line records; no duplicate database or adding system |
| Console-derived groups and nonempty systems | Launch path/Emu config resolution; no fixed GB/GBC/NES list |
| Console headings/dividers/sticky headers | Nonselectable headings; existing browser render/navigation |
| Up/Down and viewport edge scrolling | Skips headings; uses the full established viewport |
| Left/Right console jumps | First game of adjacent group, heading at top, no end wrap |
| Artwork and themed browser elements | Proportional selected-game artwork; title/highlight/footer/counters |
| OSS/libpadsp navigation audio | Hardware-verified existing audio setup |
| A launch and Onion recent registration | Runtime handoff; games appear in GameSwitcher |
| Browser MENU opens GameSwitcher | Exit-21 flow; no selected-favorite recent entry |
| GameSwitcher A resume | Uses Onion's running-game behavior |
| Automatic return OFF/ON | OFF ordinary Onion menus; ON reopens the browser |
| Session retained when switching games | Session ownership replaced single-ROM ownership |
| Remembered browser selection/viewport | Restored after reopening |
| Normal browser B exit | Ordinary app return without a reopening loop |
| SELECT actions / Y Settings | Launch, Remove, Settings, Help; nested Settings explanation |
| Menu B/MENU behavior | B backs one level; MENU closes menus without opening GameSwitcher |
| Guarded one-record removal | Reported working; verified backup/conflict/atomic safeguards in source |
| Cancel-first centered removal modal | Theme-aware UI; original navigation retained |
| Help, dialog paging and control labels | Preceding presentation reported working across dark/light themes |

The supplied report explicitly describes B/START return as device-confirmed; the
older repository status had classified START as fixture-covered. Retain that
reported confirmation, but capture START explicitly in the next versioned device
matrix rather than treating the discrepancy as evidence of exhaustive coverage.
Theme/device identities and test logs are not complete enough for universal claims.

### Implemented but awaiting device verification or edge-case confirmation

| Feature/change | Current evidence and missing evidence |
| --- | --- |
| Bold-white Automatic return headings | Included in accepted combined M4 binary; exhaustive theme matrix remains pending |
| M4 shoulder paging/resource fallback | Combined deployed binary hardware-accepted 2026-10-03; exhaustive fault combinations remain host-only, see [audit](m4-audit.md) |
| Empty favorites browsing/menu availability | Implemented and fixture-covered; complete device empty-list matrix pending |
| Removed remembered entry → nearby selection | Identity/ordinal fallback and tests; dedicated device edge-case check pending |
| Corrupt/missing preference/state behavior | Host fixtures; device fault cases not exhaustively observed |
| Signal/failure/foreign-command rollback | Ownership tests; hardware interruption/publication failure matrix pending |
| Shutdown/restart/stale ownership/stock isolation | Host lifecycle tests; complete device/version matrix pending |
| Version/hash-gated integration installer/uninstaller | Verified fixture backup/rollback behavior; broader device installation coverage pending |

### Partially implemented

| Feature | Existing support versus missing work |
| --- | --- |
| Remember-position preference | Restoration works; optional ON/OFF control not implemented or finalized |
| Hint/artwork preferences | Theme hide flags and artwork rendering work; proposed app controls absent |
| Artwork failure behavior | IMG loading failures handled; full missing/corrupt presentation not verified |
| Theme fallback coverage | Menu fallbacks exist; full-browser startup/fallback coverage incomplete |
| Lightweight operation | Limited startup/idle measurements; host-verified per-parse label cache. Gameplay/OFF-ON evidence deferred; no device gain claimed |
| Maintainability | Separate menu/model/state/removal/handoff modules; substantial browser logic in `main.cpp` |
| Dependency preparation | `fetch-deps.sh` pins an SDL commit; full custom SDL/audio build provenance/reproduction incomplete |
| Installation | Return integration manager works; production app installer/upgrade/uninstaller incomplete |

### Planned

- M4 is hardware-complete; M5 measurement pass is closed with documented deferrals. M6 read-only entry-point investigation is next.
- Browser page-at-a-time navigation after button semantics are resolved.
- Device follow-up for the final centering/text-size/padding adjustments; optional controls remain unresolved.
- Measured startup/memory profiling and evidence-based optimization.
- Normal Home Favorites tile entry; direct Home shortcut investigation as alternative.
- Explicit device/Onion/theme compatibility matrix and filename compatibility audit.
- Remaining targeted cleanup, reproducible package, application install/upgrade/remove,
  production documentation, license/notices and versioned release.

Deferred ideas and superseded decisions are recorded in section 7, not mixed into
completed browser features or the ordered remaining milestones.

## 4. Settings matrix: requirements versus proposed controls

`include/settings.h` defines app configuration. `src/app_settings.cpp` continues
to persist only Automatic return/generation using the unchanged runtime protocol.
`src/browser_preferences.cpp` separately persists grouping, prefixes and sorting;
the five-row screen exposes four adjustable values plus About. A code field alone
still does **not** imply a usable persistent option. See [browser Settings](browser-settings.md).

| Area | Accepted requirement/default | Current implementation | Remaining Settings work / unresolved optional choice |
| --- | --- | --- | --- |
| Grouping | Group by console ON by default | Persistent UI; OFF is a flat list with global chosen sort, no headings or console jumps | Normal functionality user-confirmed; edge-case matrix remains |
| Numeric prefixes | Show by default; Hide removes leading digit-dot/space prefixes for display only | Persistent UI; stored labels and sort mode remain independent | Normal functionality user-confirmed; other prefix formats not agreed |
| Sorting | Original label default; alternate Alphabetical title ignores numeric prefix | Persistent UI; literal-label order, or ASCII-folded title keys; ties use literal label/source offset | Normal functionality user-confirmed; Unicode collation/natural numeric sorting not promised |
| Long titles | Selected overflowing row scrolls after about one second; other rows stationary | Selected-title scrolling hardware-accepted; dialog paging remains separate | Behavior is core; enable/disable/speed control proposed, exact choices/default not finalized |
| Remembered position | Preserve selected identity and viewport; nearby fallback if removed | Implemented independently of a user toggle | Optional toggle proposed; default and OFF behavior/storage retention unresolved |
| Hints | Follow theme preferences; readable menu text-control labels as accepted | Theme hideIcons/hideHints applied; no app override | App hint override proposed; defaults/precedence unresolved; preserve browser/menu distinction |
| Artwork | Selected artwork proportionally displayed | Rendering implemented; failure presentation partial | Optional app show/hide setting proposed; not accepted as required v1 toggle; default/layout when OFF unresolved |
| Automatic return | OFF by default; separate optional integration; availability independent of preference | Persistent setting/generation, live-context status and explanation | Retain verified behavior; not blocked on Home integration; broader compatibility verification pending |

Settings changes must not write favorite labels/order, lose selection identity,
change Onion core settings or accidentally rotate/revive return ownership. Expand
browser preferences without changing the return helper's settings format. The
separate versioned browser file has safe defaults for missing/malformed data;
checked same-directory atomic publication precedes applying a value in memory.

## 5. Ordered remaining milestones

This order is the supplied report's corrected order. Core settings, title scrolling
and remaining navigation/artwork precede profiling and Home entry integration.

### M1 — Verify final readability and close the UI checkpoint

- **Intended behavior:** existing menu UI remains readable across light/dark themes,
  including heavier small text and filled control labels; no navigation redesign.
- **Current/missing:** final adjustments are committed/deployed and the supplied
  2026-10-02 device photos confirm the presentation shown. M1 is closed for that
  checkpoint; later scoped refinements have their own pending acceptance in M2.
- **Dependencies:** the deployed `cf8c840` build, its verified backup and actual
  themes/fonts; no new implementation is needed unless a real regression is found.
- **Reuse/inspect:** current active-theme resolution, SDL_ttf owned fonts, cached
  controls, browser highlight/dialog/divider assets and theme hide preferences.
- **Acceptance:** Help/Settings/About/footer readable and measured; filled buttons
  contrast correctly; no clipping, changed controls, altered browser fonts/geometry
  or regressions in A/MENU/B, removal, return/state/audio.
- **Host/device verification:** existing font-preservation/bounds/paging/regression
  tests and ARM build already pass. On device check light/dark text, long-title
  modal/error/paging, hide preferences and a brief workflow smoke test; record cases.
- **Exclusions/decisions:** no further polishing/features absent observed usability
  problems; exact supported-theme list belongs to M7. Build/deployment is not signoff.

### M2 — Complete usable browser Settings: grouping, prefixes and sorting

- **Intended behavior:** user can persist grouped versus flat browsing, show/hide
  numeric prefixes and select OriginalLabel versus AlphabeticalTitle sorting.
  Accepted defaults: grouping ON, prefixes shown, OriginalLabel sorting.
- **Current/missing:** implemented in the Settings checkpoint: four adjustable rows plus About,
  separate atomic browser preferences, live flat/grouped rebuild, independent display/
  sort options, original launch/ROM selection anchors and exact-record removal identity.
  User confirms deployed functionality and presentation, including inline descriptions,
  chevrons, About and horizontal modal actions/Up-Down paging, passed hardware tests.
  The subsequent M3 hardware acceptance includes final modal centering, larger
  descriptions and reduced padding. Fault cases remain separately tracked.
- **Dependencies:** current menus/state/removal identity and unchanged return format.
  OFF now explicitly means no headings or console jumps, globally sorted favorites.
- **Reuse/inspect:** parser display/sort functions, row/navigation/state helpers,
  app setting transaction/generation parser; Onion Emu labels and original favorites.
  Inspect stock presentation/order without rewriting its records.
- **Acceptance:** defaults unchanged; choices work immediately and survive restart;
  display hiding never changes stored labels; alternate sort ignores supported
  prefixes; selected game identity survives changes; source favorite/history/core
  data unchanged. Bad/older settings and failed writes leave a usable previous state.
- **Host/device verification:** defaults/migration/read-write-failure tests; grouped/
  flat/empty/duplicate/Unicode/prefix/sort fixtures and state restoration tests.
  Device cycle all combinations, restart and launch/return/remove smoke checks.
- **Exclusions/decisions:** no empty categories or unrelated metadata/core options.
  Original label is literal-label sorting, not source order. Alternate ASCII-folded
  keys preserve UTF-8 bytes; tie handling is deterministic. Unicode collation remains
  unresolved. No invented defaults for optional hint/artwork/state controls. The
  [device checklist](browser-settings.md#device-acceptance-checklist) records the
  final readability follow-up. M2 is closed for functionality and preceding
  presentation; M3 begins separately at the user's request.

### M3 — Scroll selected long browser titles horizontally

- **Intended behavior:** after approximately one second, an overflowing selected
  title scrolls horizontally; unselected rows remain stationary. Selection change
  resets the presentation. Dialog title paging remains a separate feature.
- **Current/missing:** implemented and user-confirmed on hardware on 2026-10-03
  (“everything works correctly”). Selected overflow moves using cached whole UTF-8
  text, clipped before preview/content boundaries; identity/label changes reset,
  menus pause and return restarts the delay. Host timing/SDL/resource checks and
  ARM build pass. Dialog Up/Down paging remains independent. Exhaustive device/
  theme/failure coverage is not implied.
- **Dependencies:** M2 display labels and model/state changes; current artwork area,
  sticky headings and stable browser geometry.
- **Reuse/inspect:** SDL_ttf width measurement, cached selected text, SDL timing and
  clipping; inspect stock Onion/Miyoo scrolling if observable/source-accessible.
- **Acceptance:** short titles do not move; only selected overflow moves inside its
  bounds; no overlap with artwork/headings/footer, vertical viewport movement or
  input blocking; UTF-8 readable; selection/menu/reopen reset behavior is consistent.
- **Host/device verification:** timing/bounds/reset fixtures, numeric-prefix changes,
  very long/Unicode labels, missing artwork and frame behavior; on device check
  readability, input latency, audio and light/dark fonts.
- **Exclusions/decisions:** no general animation redesign or new Settings control.
  Inspected v4.3.1-1 shared renderer crops static labels; mounted MainUI is a bind
  target for stripped binaries, so exact stock timing is unknown. Explicit local
  review policy: 1000ms delay, 30px/s, 1000ms end hold, snap to start and repeat with
  delay. Readability/speed remain subject to device feedback; optional Settings
  controls/defaults remain unresolved. See [M3 evidence and checklist](browser-title-scrolling.md).

### M4 — Finish core navigation and artwork/fallback cases

- **Intended behavior:** page-at-a-time movement complements row movement and console
  jumps; missing/corrupt artwork and empty/incomplete resource cases stay usable.
- **Current/missing:** row/console/sticky/edge navigation and no-stale artwork
  behavior already exist; focused host boundaries/cache/decode checks now cover
  them. Separate local corrections retry corrupt image candidates, provide
  theme-derived missing required surfaces and fit tall art to content bounds.
  Approved L1/R1 now pages by visible pixel height with heading accounting,
  selectable targets and no-wrap clamps. Repeats/menu shoulders are ignored.
  **M4 complete:** user hardware acceptance on 2026-10-03, tested binary SHA-256
  `aa486abad4e3272f86605945109bf0a0f76159ab373f87c1e57c38abd034b328`. Acceptance covers the deployed
  build; exhaustive corrupt/missing-resource fault combinations remain host-only; see [requirement-by-requirement audit](m4-audit.md).
- **Dependencies:** approved L1/R1 page controls; M2 grouped/flat settings and M3
  title behavior. Left/Right is already reserved for console jumps.
- **Reuse/inspect:** selectable rows/viewport helpers, sticky divider treatment,
  SDL_image/error paths, existing theme preview backgrounds and stock page behavior.
- **Acceptance:** pages land on selectable entries, respect ends/headings/empty lists
  and keep viewport consistent; existing console jumps retain accepted semantics;
  missing/bad images do not crash or display a previous game's artwork. Fallbacks
  are theme-derived and do not require editing theme configuration.
- **Host/device verification:** boundary/group/flat/partial-page/empty fixtures,
  image decode/missing paths, unusual aspect ratios and incomplete-theme startup
  tests; device paging, long titles and artwork changes while rapidly navigating.
- **Exclusions/decisions:** L1/R1 mapping and no-wrap policy are now agreed. Flat mode
  disables console jumps. Left/Right remain console navigation; X/START are not
  repurposed. Artwork
  placeholder versus blank presentation and optional hide-artwork layout are unresolved.

### M5 — Profile startup and memory; optimize measured bottlenecks

**Closed pass with explicit deferrals (2026-10-03); M6 may proceed.** This is not
full performance certification or new hardware acceptance of the cache build.

- **Intended behavior:** fast first usable frame and lightweight browsing, preserving
  working UI/audio/Onion handoff and no app/helper retained by design during gameplay.
- **Collected:** seven successful pre-cache startup archives. Original post-boot
  505.361 ms; warm median 267.590 ms (n=3). Corrected post-boot 505.596 ms;
  warm median 271.043 ms (n=2; planned fourth archive absent). Original prelaunch
  hashing and remaining metadata/IO/instrumentation/cache biases are explicit.
  Three valid idle ON samples per role: browser smaps RSS 19,308 kB / median
  PSS 17,449 kB; runtime RSS 1,808 kB / PSS 357 kB. No unbiased cold-start claim.
- **Separate optimization review:** per-parse emulator-label cache is reviewed
  separately. No measured target startup gain or cache-build hardware acceptance.
- **Deferred evidence:** gameplay memory, ON/OFF comparison, post-scroll/menu memory
  and process-absence measurement. No more profiling sessions, Terminal commands
  or device checks requested for this pass. Preserve these gaps; do not infer RAM
  from disk size or gameplay absence from idle samples. Deferrals do not block M6.
- **Retained mechanisms:** default-off startup scopes, identity-validated RSS/PSS
  collectors, guarded activation/retirement tools, fixtures and raw evidence on
  host. No persistent observer, audio/font-ownership changes or speculative cleanup.
  [Detailed results and archive](m5-profiling.md); [return memory limits](onion-return.md).
- **Closure:** all temporary profiling-owned card files archived/byte-verified before
  removal; exact production launcher restored, cache binary kept, permanent return
  integration/data preserved. Existing user hardware acceptance remains distinct.
  Any future optimization requires separately authorized matched measurements.

### M6 — Implement a verified reversible Home entry point

- **Intended behavior:** original target remains normal Home Favorites → Better
  Favorites → Onion game launch. Investigate a direct Home shortcut as an alternative.
- **Current/missing:** Apps test entry and return integration work; neither replaces
  the Home tile. No supported tile dispatch/shortcut hook has been established.
- **Dependencies:** stable M2–M4 browser and closed M5 pass (deferrals do not block investigation); exact target device/runtime/source and safe
  integration design; inventory stock dispatch before choosing a mechanism.
- **Reuse/inspect:** mounted v4.3.1-1 `runtime.sh` MainUI launch/mount/command handoff,
  actual MainUI tile dispatch, state/keymon/GLO mechanisms, existing Apps launcher,
  verified A/MENU/return protocol and any documented SD-card shortcut facilities.
  Favorites GLO scripts are per-game actions, not a verified Home tile override.
- **Acceptance:** chosen agreed entry opens the existing browser; stock adding/data,
  launch/recent/GameSwitcher/return remain intact; B returns appropriately without
  loops; enabling/upgrading/removing the entry is reversible with verified backups
  and stock restoration. Stock games/apps continue working; no NAND/firmware writes.
- **Host/device verification:** ownership/version/hash/failure/rollback fixtures and
  installation dry runs, then actual Home entry/B/game/return/stock launch/reboot/
  uninstall checks on each claimed device/version.
- **Exclusions/decisions:** do not infer success from runtime-return installation.
  Supported tile hook, shortcut mechanism, preference precedence, fallback behavior,
  installed app name/path and final selection between replacement/alternative need
  investigation and agreement. MainUI reverse-engineering is not a promised feature.

### M7 — Compatibility and targeted maintainability cleanup

- **Intended behavior:** explicitly supported Miyoo Mini/Plus and Onion versions,
  robust data/themes, with understandable code and unchanged verified behavior.
- **Current/missing:** target devices are named but comprehensive coverage is absent;
  optional hook supports only the verified v4.3.1-1 runtime hash. Parser/menus/state/
  removal/handoff modules exist; `main.cpp` remains large and diagnostics need audit.
- **Dependencies:** final features and measured behavior; identified firmware/runtime
  assets, test fixtures and reproducible regression baseline.
- **Reuse/inspect:** actual versioned Onion runtime/proxy/Emu/core settings, GameSwitcher
  string parser/recent format, stock launches, theme/font/image resources and logging.
- **Acceptance:** publish device/version/theme/fault matrix covering empty lists,
  Unicode/long titles, missing ROM/artwork/resources, settings/state changes, shutdown/
  signals/restart and stock isolation. Audit valid filenames including rejected `&`
  against both shell and runtime parsing. Any widened support keeps quoting safe.
  Extract only useful responsibilities, remove abandoned code/noisy diagnostics,
  retain useful errors/tools and keep tests meaningful and behavior stable.
- **Host/device verification:** parser/path/quoting/migration/resource/transaction
  fixtures plus all lifecycle tests; device matrix and repeat baseline measurements
  when changed. Validate rollback/uninstall preserving user data on real targets.
- **Exclusions/decisions:** universal Onion/theme support is not established. Unsupported
  versions must be identified, not guessed compatible. No code redesign for appearance;
  host temporary-file cleanup is separate, selective and explicitly authorized, never
  a blanket `/tmp` or device-temporary-directory deletion.

### M8 — Reproducible packaging, app installation and documentation

- **Intended behavior:** a clean checkout builds a documented package; user can install,
  upgrade and uninstall the app with optional integrations separately controlled.
- **Current/missing:** Docker builds the prepared checkout; `fetch-deps.sh` pins SDL
  source, but custom SDL/audio binary provenance and full reproduction are incomplete.
  `scripts/build.sh`, `install.sh`, `uninstall.sh`, `package.sh`, install/uninstall docs
  and `LICENSE` are empty. The working return manager is not an app installer.
- **Dependencies:** M6 integration design, M7 supported matrix, dependency/license
  decisions, version/layout/defaults and reproducible custom SDL2/audio procedure.
- **Reuse/inspect:** existing toolchain/Makefile/fetch-deps, verified OSS/libpadsp SDL
  build, required private libraries (including SDL_ttf dependency chain), return
  manager's backup/hash/rollback design and Onion app packaging conventions.
- **Acceptance:** clean-checkout dependency preparation and ARM build documented and
  repeatable; pinned toolchain/source/binary provenance or deterministic custom build;
  package contains executable/launcher/required private libraries/defaults without
  personal data. App install/upgrade/uninstall preserves favorites/history/saves/
  settings as agreed and restores integrations safely. README/build/architecture/
  install/uninstall/version/known-limitations docs and chosen license/notices complete.
- **Host/device verification:** fresh-environment build/package inventory/ABI/dependency
  checks, hash/manifests, failure/foreign-change rollback fixtures, then device fresh
  install/upgrade/reboot/uninstall tests including optional integrations OFF/ON.
- **Exclusions/decisions:** no shipping developer logs/previews/backups/credentials.
  A successful existing-checkout build does not prove reproduction. Application name,
  package layout/version scheme, settings retention on uninstall, license and exact
  toolchain pinning still require decisions; do not invent defaults.

### M9 — Publish a versioned release; revisit deferred ideas afterward

- **Intended behavior:** downloadable verified version with clear installation,
  support boundaries and release notes, instead of requiring this development checkout.
- **Current/missing:** no release artifact is documented; this docs pass does not
  independently verify the live GitHub release list. All production gates above remain.
- **Dependencies:** M1–M8 acceptance, license/notices, package/version and supported matrix.
- **Reuse/inspect:** repository tags/releases, reproducible package/manifests and proven
  install/rollback instructions; stock Onion workflows remain authoritative.
- **Acceptance:** agreed release version/tag, downloadable package with hashes/notices,
  supported device/version list, known limitations, install/upgrade/uninstall and tested
  release notes. Deferred features remain separate unless explicitly reprioritized.
- **Host/device verification:** package provenance/content and clean-install checks;
  smoke-test the exact distributable on declared devices/versions, not a local substitute.
- **Exclusions/decisions:** release numbering/distribution mechanics and support policy
  are unfinalized. This milestone is not authorization to publish a release today.

## 6. Verification, compatibility and known gaps

Current optional runtime reference is Onion **v4.3.1-1**. Its original runtime blob
is `4e2194f1b47c6c13605846002b6be0b42c1384f9`, SHA-256
`a8d77dcd316bc2a323b1e015aaf4b7682d2fed677af9cdadbc00e48881425d6e`.
See [hash manifest](../integration/onion-return/hashes.json),
[installer/rollback](../integration/onion-return/manage.py) and
[versioned Onion references](onion-return.md). Do not generalize this hash gate to
other Onion versions. App/theme compatibility and runtime-hook compatibility are
separate dimensions; Mini/Plus are targets, not a completed support matrix.

Evidence gaps and limitations to retain until resolved:

- Final readability is confirmed by supplied device photos for the screens shown;
  browser Settings functionality and preceding presentation are hardware-confirmed.
  Only the new bold-white About headings are hardware pending. mini.os has
  no matching regular Nunito face; native bold is retained without a false weight claim.
- Grouping/prefix/sort are now persistent UI controls in a separate file. Optional
  long-title/hint/artwork/remember-position controls remain proposals, not implemented.
  Return setting's compact format/helper compatibility remains unchanged.
- Browser title scrolling is hardware-accepted for the reported test;
  dialog paging is a separate presentation feature.
  Browser page controls are not defined. Flat mode disables console jumps.
- Valid paths with `&` and other excluded characters cannot currently launch;
  broader compatibility must satisfy shell execution and Onion's parser together.
  Resolved existing traversal paths are supported without changing their strings.
- Alternate title sort folds ASCII only; Unicode collation/natural numeric sorting
  are not established requirements or implemented guarantees.
- Missing/corrupt artwork and full-browser missing-font/background behavior are not
  exhaustively proven. Menu missing-asset fallbacks do not establish full-app startup.
- Removal snapshot comparisons are not compare-and-swap rename. An uncooperative
  writer between final comparison and rename is a documented race assumption;
  observed conflicts are refused and backups retained. No full concurrent-edit claim.
- Recent registration/handoff rollback and runtime ownership have strong host fixtures,
  but interruption/storage/shutdown/restart/stock-isolation device cases are incomplete.
- Limited pre-cache startup and idle ON RSS/PSS are measured; full gameplay/OFF-ON/process-absence evidence is deferred. Resource/file sizes do not measure RAM.
- Existing SDL_ttf `libbz2.so.1.0` linker warning and private library provenance are
  packaging gaps. Do not change working audio/dependencies merely to silence it.
- Home dispatch/hook remains unknown; MainUI internals are not verified. Return helper
  availability does not imply Home integration, and preference ON does not imply installation.
- No complete application installer/reproducible release/license selection yet.

Host gates include `tests/run-local-checks.sh`, `tests/runtime_installer_test.py`
with the pinned runtime reference, shell syntax and Docker ARM compilation. SDL
preview tools validate measured layouts with actual theme resources but desktop
rasterization is not hardware proof. Device verification must record actual inputs,
expected/observed results and deployment hashes before status is promoted.

## 7. Deferred ideas and superseded decisions

### Alternatives researched before choosing the wrapper

Per-console generated collections, Collection Switcher/SearchFilter, system-prefix
label rewriting, Favorites-as-folders, another frontend and MainUI reverse-engineering
were possible approaches investigated earlier. They are **not** six additional
promised wrapper features. Revisit only if an agreed integration constraint requires it.

### Outside core v1 scope

Custom collections, manual ordering, playtime/last-played displays, extra details/
metadata, completion tracking, search/filtering and nested organization are deferred.
Search was considered unnecessary for initial scope. Scraping, separate emulator
management and independent save-state management are not part of the chosen product.
Artwork/state/hint/scroll optional Settings controls remain proposals where defaults
or choices were not accepted; do not silently promote them to required release gates.
Further visual polish is deferred after M1 absent a demonstrated usability problem.

### Superseded policies

- Y context menu → **SELECT actions; Y full-screen Settings**. MENU closes an open
  menu; browser MENU opens GameSwitcher. B remains one-level back/normal app exit.
- Read-only favorites → narrowly authorized exact-record removal required for v1;
  stock adding stays intact, and unrelated data remains protected.
- Single-ROM return ownership → **session originating in Better Favorites**, retained
  through GameSwitcher switches; invalidation/consumption/loop prevention remain mandatory.
- MainUI capture/stop/kill handoff → Onion's Apps command/quick_switch mechanism;
  MainUI is absent in the verified Apps launch chain.
- Publishing a request from the live SDL event loop → private staging, cleanup,
  then outer-launcher publication with invocation ownership and safe rollback.
- Timed rotating dialog/error text → explicit dialog paging and concise complete
  errors, with technical detail retained in logs.
- Home integration as the only remaining feature or startup profiling as the immediate
  next implementation → corrected ordered core-feature block M2–M4 first.

No deferred choice may change the accepted mapping, data rules or Onion lifecycle
without an explicit update to this authoritative roadmap and development status.
