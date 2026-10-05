> **Historical engineering record — not current installation instructions.**
> Use the [installation guide](../install.md) and [compatibility matrix](../compatibility.md) for the released app.

# M3: selected browser-title scrolling

> **Historical engineering evidence.** This page records development decisions and tests at the time. Use the [current installation guide](../install.md) and [recovery guide](../recovery.md) for released packages.

Status: M3 is user-confirmed on hardware (2026-10-03): “everything works
correctly.” Accepted binary SHA-256:
`12545265ff8508bf6767b1ef3057854d54d513f6610f602490e3c0dcb7f1ce2d`.
Host timing/SDL checks and ARM build also pass. This is normal-use acceptance,
not exhaustive fault/device/theme coverage. Only the two About headings changed
in this checkpoint: bold white at their existing font/size. Those changes still
await device verification. No previews were generated.

## Inspected reference and policy

The mounted Onion version is v4.3.1-1. Matching source was read through GitHub MCP:

- [theme_renderListLabel / theme_renderList](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/theme/render/list.h)
  renders the complete UTF-8 label with SDL_ttf, crops the source rectangle, and
  draws preview artwork afterward. This shared renderer has no horizontal timer.
- [components/list.h](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/components/list.h)
  provides selection/vertical scrolling, not a title marquee.
- Mounted `.tmp_update/runtime.sh`, `mount_main_ui()`, binds
  `.tmp_update/bin/MainUI-$DEVICE_ID-$mainui_mode` over `miyoo/app/MainUI`.
  The latter is an empty mount target while the card is on macOS; the actual
  `MainUI-354-clean` / `MainUI-283-clean` files are stripped ARM executables.
  The 354 binary imports `TTF_RenderUTF8_Blended`; this does not establish its
  internal scrolling delay, speed or end behavior.

Stock MainUI timing is therefore **unknown**, not claimed reproduced. For this
review, Better Favorites waits **1000ms**, scrolls left at **30 pixels/second**,
holds the fully exposed end for **1000ms**, snaps to the start, then waits the
initial 1000ms again. This explicit app policy needs device readability feedback;
it is not a new persistent Settings option. No bounce, extra duplicated title,
byte slicing or new artwork/font/dependency is used.

## Implementation and boundaries

`TitleScroll` (`include/title_scroll.h`, `src/title_scroll.cpp`) uses elapsed SDL
milliseconds with wrap-safe unsigned deltas. Selected original launch/ROM paths
and source-record offset identify the game; displayed-label or measured-width
changes reset the delay. Navigation restarts it even if multiple queued key events
move away and back in one frame. Short labels never move.

`BrowserTitles` (`include/browser_titles.h`, `src/browser_titles.cpp`) borrows the
existing browser font, caches whole UTF-8 SDL_ttf surfaces by label/color, and
blits them with a pixel offset. Visible selected/unselected variants are reused;
unused entries are freed after each frame, including when the list becomes empty.
Failed rasterizations are cached while visible to avoid repeated attempts/logs.
Caches are explicitly released before SDL/font cleanup. No font/image is loaded
per animation frame; no thread or blocking animation delay is added. Existing
input processing, vsync and OSS/libpadsp audio stay in the same loop.

The title origin remains x=20 and vertical row geometry is unchanged. Without a
loaded preview background, the right boundary is x=620. With it, the title ends
8px before the existing preview column (`640 - previewBackground->w`). This
reserves even transparent preview material and missing-art space consistently.
Rows are clipped to their own height, content y=60..419 and any preexisting SDL
clip rectangle; the previous clip is restored after drawing. Console headings,
artwork and footer use their existing render paths. Unselected titles stay at
pixel offset zero. Menus freeze the current offset; closing them resets to the
start with a full delay, including nested/removal menus and batched events.

The actual cache memory depends on visible text widths and font height. Host tests
verify reuse/eviction, not measured device RAM or frame time. Oversized/malformed
text that SDL_ttf cannot rasterize follows the existing failure behavior (no label,
logged error); no claim of universal Unicode glyph coverage is made.

## Checks

- `sh tests/run-local-checks.sh`: timing/end hold/restart, selection/source identity,
  prefix/display text, viewport-width change, menu pause/resume, short/zero-width
  regions and SDL 32-bit tick wrap; existing preferences/model/removal, settings,
  state, A/MENU launch/history and runtime-return regressions, shell syntax.
- Native SDL: pixel equality to a whole UTF-8 raster shifted by 30px; no writes
  outside the title rectangle; inherited clip restored; stationary unselected
  rows; no new rasterization across 1000 idle frames; eviction across 100 different
  rows; explicit cleanup and borrowed font unchanged. Tested with the existing
  dark and light theme fonts. No image output.
- Existing Docker `make -B all`: ARM ELF build passed. Existing SDL_ttf linker
  warning about `libbz2.so.1.0` remains; no new compiler warnings.

Repeat native checks using the existing SDL source/font resources (no installs):

```sh
sh tools/check-title-scroll.sh /path/to/SDL_ttf.c /tmp/better-favorites-m3-native /path/to/theme/font.ttf
```

## Device regression checklist

1. Focus a long title: stationary for about one second, then smooth leftward
   motion, one-second end hold, restart and delay. Short/unselected titles stay still.
2. Move rapidly up/down, switch consoles, and move away/back. Each new selection
   starts at the beginning. Navigation clicks, responsiveness and A launch remain normal.
3. Open SELECT, Settings, Help and removal while moving. The underlay stops; B/MENU
   back to the browser starts a fresh delay. Cancel removal; confirm action controls
   and explicit dialog title pages have not changed.
4. Toggle numeric prefixes, grouping and sorting. Selection/viewport stay correct;
   changed displayed text resets. Check flat and grouped lists, duplicates and empty list.
5. Check very long/accented/CJK labels, available glyphs, light/dark themes, missing
   artwork and missing preview background. Text must stop before artwork and never
   enter headings/footer; vertical row positions must stay unchanged.
6. Smoke-test A game launch, MENU/GameSwitcher and ON/OFF return with saved position.
   Report timing/readability, input/audio regressions and any animation stutter.
