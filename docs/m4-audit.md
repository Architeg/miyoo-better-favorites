# M4 audit and confirmed gaps

> **Historical engineering evidence.** This page records development decisions and tests at the time. Use the [current installation guide](install.md) and [recovery guide](recovery.md) for released packages.

Status: **M4 complete.** The user accepted the deployed combined M4 on
2026-10-03: “everything works.” Tested ARM binary: 347932 bytes, SHA-256
`aa486abad4e3272f86605945109bf0a0f76159ab373f87c1e57c38abd034b328`. This acceptance includes approved L1/R1 paging
and the deployed resource behavior. No new binary or redeployment is needed for
publication. Preceding checkpoint: `1768c7a902ef33513adbd54c290188451652e1be`.

Device acceptance is user-reported normal-use evidence, not a claim that every
fault fixture was executed on hardware. Corrupt/missing resource combinations,
malformed configs, firmware-font fallback, allocation failure and exhaustive
boundary combinations retain their separately stated host/source evidence.
No device timing or RSS/PSS result has been measured. No previews generated.

## Exact roadmap scope

Page-at-a-time movement alongside existing row movement and console jumps;
selectable destinations at list ends/headings/empty or partial pages; consistent
viewport; missing/corrupt artwork without stale images or crashes; usable empty/
incomplete theme resources, active-theme resources before system fallback;
proportional artwork and unchanged working geometry. M4 excludes redesign,
profiling, new artwork/preferences/history and Home integration.

## Audit against implementation and tests

| Requirement | Existing implementation/evidence | Confirmed gap / local correction |
| --- | --- | --- |
| Row/end navigation | `navigation.cpp` next/previousSelectableRow; hardware normal-use acceptance | Added grouped/flat/empty/single/end/header-skipping fixtures; no rewrite |
| Console jumps | next/previousConsoleRow plus main viewport correction; Left/Right, flat-mode no-op | Tests now include empty groups and ends; bindings unchanged |
| Sticky headings/vertical viewport | main's pixel visibility loop and header overlay; accepted browser geometry | Preserved; dedicated full-device combinations remain evidence gaps |
| Page-at-a-time browser movement | Previously absent | Approved L1/R1 now uses measured pixel budgets, selectable targets and no-wrap clamps; user-reported device acceptance; exhaustive combinations remain host fixtures |
| Clear previous art on failed/new selection | Existing main frees the old surface before IMG_Load; path cache | Extracted unchanged into updateFavoriteArtwork to test valid → corrupt/missing/empty transitions and cache reuse |
| Missing/corrupt art presentation | Existing preview background with no artwork; no previous game's image | Verified host fixtures; blank behavior retained; no new placeholder |
| Proportional artwork | Width-only scaling, centered at y=240 | Very tall art could cross header/footer. Fit now limits width and height to existing preview/content bounds; normal-size positions unchanged |
| Image priority | ThemeLoader chooses by file existence: profile → active → Miyoo | Existing but corrupt overrides blocked valid lower assets. Decode fallback now follows the same candidate order |
| Completely missing background/selection | Main required background and could exit; absent highlight lost selection surface | Generate opaque theme-derived cached surfaces only after all existing candidates fail, using original 640x480/640x56 geometry |
| Optional title/footer/divider/icons/preview | Existing omissions/text/line fallbacks; fonts and hints preserved | Decode lower candidates before existing omissions; missing preview material continues to suppress art rather than inventing layout |
| Fonts/malformed config | M2 actual font validation, profile → active → SD fallback, inherited config | Native fixtures verify missing/corrupt fonts and malformed config. Add Onion's `/customer/app/Exo-2-Bold-Italic.ttf` last; never switch family merely for weight |
| Existing M3/menu/input/audio/protocols | Cached UTF-8 scrolling and menu/launch/settings/removal tests, hardware acceptance | Unchanged; regressions and offscreen layout checks run without images |

Valid assets keep their existing surfaces/colors/dimensions. Image candidates are
read only at startup, except the unchanged selection-path artwork cache. Popup/
dialog material retains the agreed profile/active-only policy; absent materials
use existing theme-derived surfaces rather than an unrelated stock popup. No
installed theme configuration, saved preference/state or Onion script changes.
The small extraction of art cache logic enables tests; it does not change retry
policy. A failed same-path image is not retried every frame.

## Approved paging policy and references

[Onion v4.3.1-1 keymap_sw.h](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/system/keymap_sw.h)
maps L1=`SDLK_e`, R1=`SDLK_t`, L2=Tab, R2=Backspace, START=Return.
Current app uses Up/Down rows, Left/Right consoles, A launch, B exit, SELECT actions,
Y Settings and MENU GameSwitcher. L1 pages up and R1 pages down (approved 2026-10-03).

Page distance is 360px minus the current sticky console heading. Each traversed
heading counts its actual 48px plus divider height; each game counts 60px. Select
the furthest reachable game within this budget, or at least the next game when a
large heading gap exceeds it. Partial pages clamp to the first/last game and never
wrap. At an end both selection and viewport stay unchanged. The existing render
visibility correction adjusts the viewport after a changed selection; audio and
M3 delay reset use the existing navigation path. SDL repeat events are ignored:
one page per physical press. Menu/dialog dispatch consumes shoulders before browser
navigation. Help uses cached themed L1/R1 capsules; browser footer is unchanged.

[Onion components/list.h](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/components/list.h)
uses active/scroll position and handles disabled rows/wrap based on repeat;
it does not settle our grouped page movement or button policy.
[theme_getImagePath / theme_loadImage / theme_loadFont](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/theme/load.h)
establish profile → theme → Miyoo image lookup and firmware font fallback.
Stock image selection is existence-based; our decode retry closes the explicit
corrupt-resource requirement. Internal firmware fallback cannot be exercised on
macOS; its source path is verified, its device behavior remains to be checked.
If no usable font exists anywhere or allocation fails, a readable UI cannot be
created without new resources: main retains its logged required-resource failure.
No claim of all-resource fault immunity or measured RAM/startup cost is made.

## Host checks

- `sh tests/run-local-checks.sh`: navigation boundaries plus existing scrolling,
  settings/model/removal/launch/history/runtime/shell regressions.
- `sh tools/check-browser-resources.sh BACKEND_DIR OUTPUT_DIR THEME_FONT`: real
  SDL_image/SDL_ttf decoding fixtures for valid/corrupt/missing priority tiers,
  profile/active popup material, malformed configs, missing active marker,
  theme-derived background/selection dimensions, cleared/cached artwork and
  wide/tall/very-thin content bounds. Private fixtures only; no previews/card writes.
- Existing menu SDL bounds/font/ownership checks with `--checks-only` and ARM build.

## Device checklist for the combined M4 test binary

1. Browse grouped/flat and empty lists, first/last rows and console boundaries.
   Left/Right and saved selection/viewport must retain their accepted behavior.
2. On a test copy of theme resources, exercise missing/corrupt profile assets,
   valid active assets and missing/corrupt active assets with system fallbacks.
   Keep required readable fonts; verify generated background/highlight only when
   all matching images are unavailable. Do not alter the user's working theme.
3. Switch quickly between valid, missing, corrupt and empty artwork paths. No
   previous image may remain. The existing preview blank/material is retained.
4. Check unusually tall/wide/small art: proportions retained, no header/footer
   overlap, normal art position unchanged. Long-title scrolling stays in its region.
5. Smoke-test audio, Settings persistence, exact removal, A launch and MENU/return.
   Verify the newly deployed bold-white About headings separately.
6. L1/R1 page both directions in grouped/flat mode, across headings and partial
   pages. Check empty/single lists, first/last no-op and no wrap. Hold a shoulder:
   one page until released and pressed again. Menus/dialogs must ignore shoulders.
7. Page to a game, exit/reopen and launch/return: selection and viewport restore.
   Changed selection restarts long-title delay; ends produce no extra sound.

M4 is hardware-accepted and complete. The checklist remains available for broader
compatibility testing; M5 startup/memory measurement is next.
