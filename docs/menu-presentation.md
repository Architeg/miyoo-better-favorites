# Menu presentation review

Local presentation refactor, 2026-10-02. The user reports hardware-confirmed
navigation, removal, launch/return, persistence and audio in the deployed binary.
This refactor is not deployed or committed.

## Mounted asset trace

The current `/.tmp_update/config/active_theme` contains
`/mnt/SDCARD/Themes/mini.os by architeg/`, which **exists** on this card. The earlier
`.../mini.os by architeg/2021 Stock by Miyoo` path remains missing but is no longer
configured. No theme configuration was modified by this step.

Resolution remains profile override -> active theme -> Miyoo fallback for the
working browser. Menu selection now aliases that exact browser result. Popup
art searches compatible profile/active-theme `bg-pop-menu-4.png`, then
`menu-sub-bg.png`; it does not pull an unrelated stock popup into a missing theme.

| Resource | Exact current mounted file |
| --- | --- |
| Popup candidate | `/Volumes/MIYOO/Themes/mini.os by architeg/skin/bg-pop-menu-4.png` |
| Old popup highlight | `/Volumes/MIYOO/miyoo/app/skin/list-item-select-bg-short.png` |
| Browser and new menu highlight | `/Volumes/MIYOO/Themes/mini.os by architeg/skin/bg-list-s.png` |
| Background | `/Volumes/MIYOO/Themes/mini.os by architeg/skin/background.png` |
| Header | `/Volumes/MIYOO/Themes/mini.os by architeg/skin/bg-title.png` |
| Footer | `/Volumes/MIYOO/Themes/mini.os by architeg/skin/tips-bar-bg.png` |
| A icon candidate | `/Volumes/MIYOO/Themes/mini.os by architeg/skin/icon-A-54.png` |
| B icon candidate | `/Volumes/MIYOO/Themes/mini.os by architeg/skin/icon-B-54.png` |
| All fonts | `/Volumes/MIYOO/Themes/mini.os by architeg/Nunito-Bold.ttf` |

Loaded font sizes: title 24, list 20 (existing browser bold style), body 18,
hint 19. List/hint/title text is white; selected text is `#d0d0d0`, as in the
browser. These values come from the existing loader, including profile overrides.

**Red cause:** active-theme `list-item-select-bg-short.png` is absent, so the old
resolver selected the stock 160x30 reddish asset. Its mean RGB is approximately
(178,56,35); it was stretched into popup rows. The active 640x56 browser highlight
is opaque RGB (39,39,39). New menu rendering uses this browser asset at native
height, centered within each measured row.

The active popup candidate is 320x250 with transparent rounded edges. The current
and Analogue candidates have surfaces close to their browser backgrounds, so
these cases use an opaque contrasting plate derived from theme background/text
colors, with a subtle derived boundary. Compatible artwork with sufficient
surface/text contrast is composited over that plate. Transparency never exposes
browser text through the popup. No fixed red/gray palette is used.

Inspected `icon-A-54.png` and `icon-B-54.png`: both depict button-position dots,
with a different dot lit, not A/B lettering. X/Y 54px assets are absent; the Miyoo
skin also has a stock `icon-START.png` and keyboard/arrow artwork. There are no
compatible active-theme letter controls/capsules for all needed buttons. Menus
therefore share theme-colored letter circles and named capsules. The existing
`hideIcons` suppresses decorative theme images, while cached drawn circles,
capsules and arrows are readable text-control labels and remain visible.
`hideHints` hides menu footer labels and counters. Help controls and explicit
title-paging instructions remain explanatory content. The current profile uses
hideIcons=true and hideHints=false. Browser rendering preferences are unchanged.

## Presentation changes only

`MenuRenderer` shares row measurement, text layout, button hints, spacing and
palette derivation. `menu_text.cpp` wraps using the loaded SDL_ttf font width and
splits long words only at UTF-8 boundaries. No menu text uses clipping.

- Settings: Automatic return label left and ON/OFF right; short contextual
  description. Technical installation/availability details live in explanation.
  An unavailable integration is stated explicitly in the contextual description.
- Help: aligned control/action columns with short labels. Browser and menu
  contexts are separate reading pages; Up/Down changes only the displayed page.
- Explanation: measured paragraphs, with Up/Down reading pages if needed.
- Removal: question, full game title, “The game file will be kept.”, Cancel/Remove.
  The verified confirmation cursor still starts on Cancel. Extremely long titles
  use LEFT/RIGHT in a dedicated paging strip. Text stays still until explicitly
  paged; Up/Down continues to select Cancel/Remove.
- Errors show concise complete measured summaries without timed pagination.
  Original technical details remain in logs; the clear-on-action policy is unchanged.
- Footer: shared centerline, symbol size and gaps; concise context actions.
  Oversized theme hint fonts are reopened at a smaller measured size inside the
  existing 60px footer, preserving font family/color and browser font resources.

`MenuState`, favorite parsing/removal, handoff, settings persistence, browser
state, launcher and runtime helper are unchanged by this refactor. Main only
calls the new renderer and adds read-only page movement/reset. Browser row/
viewport/artwork geometry and OSS/libpadsp/Mix initialization remain unchanged.
Menu-owned resources are freed before the existing SDL/audio teardown.

## Real SDL previews and limits

Previews execute **the production `MenuRenderer` and `ThemeLoader`** on native
SDL2 2.32.8 surfaces at the same 640x480 RGB565 geometry, with real card assets and
fonts. A host-only cJSON bridge covers the simple json-c getters used by the
loader; the production ARM json-c dependency remains unchanged. The temporary
font/PNG backends use official sources:

- [SDL_ttf 2.20.2 SDL_ttf.c](https://github.com/libsdl-org/SDL_ttf/blob/release-2.20.2/SDL_ttf.c), Git blob `16428b154395db6d1a6781da3530190d8c37eb2b`.
- [SDL_image 2.8.1 PNG loader](https://github.com/libsdl-org/SDL_image/blob/release-2.8.1/src/IMG_png.c), Git blob `fed971b6861b98802d5d544ea3268740a770bbc4`.

These match the checked-in API header versions. Sources/builds remain in host
`/tmp/better-favorites-sdl-preview`; no production dependencies were installed or
replaced. PNG loading/saving uses upstream SDL_image's backend, with a host-only
PNG dispatch wrapper. The preview underlay is representative browser content,
not an execution of the full Miyoo main loop. Native FreeType/SDL versions can
rasterize differently from the deployed ARM libraries; this is not hardware
verification of presentation or audio.

The earlier preview matrix is retained as historical output. This revision
produces only three images, as described below. Native tests assert that rendered
text fits screen bounds and exercise hidden hints offscreen. Missing-background
captures exercise the standalone menu renderer; app startup still requires its
browser background/fonts. No full-app missing-background fallback is claimed.

Reusable preview build/driver sources are under `tools/`. Native source/runtime
requirements remain unchanged. See the current three-image invocation below.

Checks: menu text, actual SDL bounds/previews, existing menu navigation/removal
failures, A/MENU handoff, launcher cleanup, settings/state and session-return
regressions; shell syntax; Docker ARM build. Final build has no compiler errors
or warnings, with the existing SDL2_ttf/libbz2 linker warning. Deployment, commits,
pushes and historical temporary-file cleanup were not performed.

Final pass: popup surface mixes 12% theme text into the sampled background; the
active selection asset remains unchanged. All footer groups use the same baseline,
10px symbol/text gap and 28px group gap, including hidden-label/icon variants.

## Device-photo refinement (local, not deployed)

Menu text-control labels are separate from decorative theme images. A/B/X/Y
use small circles, SELECT/START/MENU use capsules, even with
`hideIcons=1`. `hideHints=1` hides entire menu footers. Help content and title
paging remain explanatory content. Browser preferences/rendering are unchanged.
Directional labels reuse resolved left/right assets when decorative icons are
allowed, then test the loaded font's arrow glyphs, then use small SDL line arrows.
Controls, arrows, dimming surface, dialog material and theme colors are cached;
control surfaces are released before existing SDL/audio cleanup.

Inspected active `skin/pop-bg.png` (640x300), `div-line-h.png`, left/right 24px
arrows, A/B 54px assets and existing browser divider/outline helpers. A/B assets
are position dots, not letters; no suitable X/Y or text capsule assets were found.
The dialog resolver uses profile then active theme, without pulling unrelated
stock dialog art. Only the asset's plain central material is reused: Analogue's
full dialog has baked CANCEL/OK artwork. Missing dialog material falls back to
an opaque theme-derived panel/boundary. Browser dimming uses its sampled color.

Removal is a measured centered modal with 20px horizontal and at least 24px
vertical safety margins, bounded title wrapping, LEFT/RIGHT title pages, and
complete concise errors inside. Cancel/Remove remain vertical rows with the
existing Up/Down selection and A/B actions. Settings values remain right aligned.
Explanation separates integration status and short On/Off sections.

Generate exactly three review images with the reusable driver:

```sh
sh tools/build-menu-preview.sh /tmp/better-favorites-sdl-preview /tmp/better-favorites-sdl-preview/build
mkdir -p /tmp/better-favorites-device-menu-previews
/tmp/better-favorites-sdl-preview/build/menu-preview \
  /tmp/better-favorites-menu-previews/current-card \
  /tmp/better-favorites-menu-previews/alternate-card \
  /tmp/better-favorites-menu-previews/missing-card \
  /tmp/better-favorites-device-menu-previews
```

Outputs: Help, removal over representative browser, alternate/missing contact
sheet. Contact-sheet sample symbols include X/START for visual validation; they
are not extra Help actions. Native SDL renders the production menu code with
real mounted theme resources; this is not device rasterization verification.
Tests also render preferences, both Settings rows and explanation offscreen.
Historical preview matrices/files are retained without regeneration or cleanup.

## Final readability checkpoint

The preceding presentation was hardware-confirmed by the user across dark and
light themes. These final micro-adjustments are host-tested/ARM-built and await
device verification. Menu Help, Settings-description, About and footer text use
separate app-owned fonts, one point larger where it fits. Same-family heavier
faces are preferred (for example CleanOnionGB's `Inter-SemiBold.otf`); otherwise
SDL_ttf bold is applied only to those new instances. Browser fonts/styles remain
unchanged. Filled A/B/X/Y circles and SELECT/START/MENU capsules use theme ink and
contrasting theme background letters; arrows are modestly heavier. Controls stay
cached and are released before SDL cleanup. Wrapping/paging are measured again.

The current reusable preview driver writes at most two images:
`menu-preview LIGHT_FIXTURE DARK_FIXTURE MISSING_FIXTURE OUTPUT_DIR` produces
`light-help.png` and `dark-removal.png`; all long-title/error/missing-resource,
hidden-footer, Settings/About and borrowed-font preservation checks run offscreen.
Earlier three-image/contact-sheet and matrix captures are historical and retained.
The sample row remains preview-only, guarded by `BETTER_FAVORITES_MENU_RENDER_TESTING`.
