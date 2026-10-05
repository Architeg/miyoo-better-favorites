> **Historical engineering record — not current installation instructions.**
> Use the [installation guide](../install.md) and [compatibility matrix](../compatibility.md) for the released app.

# Persistent browser Settings

> **Historical engineering evidence.** This page records development decisions and tests at the time. Use the [current installation guide](../install.md) and [recovery guide](../recovery.md) for released packages.

Status: the user confirms browser Settings functionality/presentation and M3
passed hardware testing. The last centered-modal/larger-description/reduced-padding
adjustments were present in that M3 build. The two new bold-white About headings
still await device verification. Host tests/builds remain separate evidence.

## Behavior and controls

| Row | Values/default | Effect |
| --- | --- | --- |
| Automatic return | OFF / ON; OFF | Existing availability and generation rules unchanged |
| Group by console | ON / OFF; ON | Console headings and within-console sort; OFF globally sorts a genuinely flat list |
| Numeric prefixes | Show / Hide; Show | Display only; hides leading digits followed by a dot and optional whitespace |
| Sorting | Original label / Alphabetical title; Original label | Literal stored-label sort, or ignore numeric prefix and fold ASCII case |
| About automatic return | Action | Opens the explanation; never cycles a value |

Up/Down selects rows; Left/Right or A toggles/cycles adjustable values. Repeated
A/Left/Right events are ignored, including Automatic return. B backs out one level;
MENU closes the menu without opening GameSwitcher. SELECT/Y/browser mappings are
unchanged. Help states that console jumps are disabled in flat mode.

Changing display prefixes does not independently change sorting, source labels,
launch/history paths or records. Alternate keys do not promise Unicode collation
or natural numeric ordering. Ties use original label and source offset. Rebuilding
retains original launch/ROM identity, with source offset disambiguating duplicates;
the previous top favorite is anchored where possible and existing pixel visibility
logic adjusts the viewport. A missing identity falls back to a nearby ordinal.
Copies retain exact raw source record/offset for guarded removal.

## Persistence and compatibility

`browser-preferences.conf` is saved beside `settings.conf` in the app directory.
It is excluded from Git. The four-line canonical format (including final newline):

```text
BetterFavoritesBrowserPreferences1
1
1
0
```

Lines are group ON=1/OFF=0, prefixes Show=1/Hide=0, sort Original=0/Alphabetical=1.
All eight combinations are accepted. Missing data defaults to ON/Show/Original.
Malformed, oversized, nonregular or unreadable files default safely and log the
problem. No automatic rewrite is performed on startup. An older installation with
only `settings.conf` migrates simply by using browser defaults until the first
changed browser value. The return setting/generation format and helper remain
byte-for-byte unchanged by browser preference changes.

Save uses an exclusive temporary regular file in the destination directory, checks
write/fsync/close and rereads its encoding, then atomically renames it. The in-memory
value changes only after successful publication. Failure leaves the saved setting
and browser model in use and shows an error; cleanup attempts to remove only this
save's temporary. A filesystem becoming unwritable may also prevent that cleanup;
the leftover is never an active preference file.
This is atomic replacement with flushed file data, not a claim of guaranteed
power-loss durability on FAT or protection from a separate concurrent preference
writer. Settings never write favorites, recent history or Onion configuration.

## Presentation

Adjustable values share a fixed right edge and reserved chevron space. Selected
values use the existing theme chevron assets when decorative icons are allowed;
otherwise two simple strokes use the selected theme text color. Shafted browser
arrows and other controls are unchanged.

The description panel is anchored directly above the footer, independent of the
number of rows. Its opaque surface is derived from theme background/text colors,
without the selection asset. Rows scroll above reserved space for two lines; a
shorter description is vertically centered. Text is left-aligned and measured,
at its existing size. Automatic return ON shows inline badges in
`[B] / [START]: return here from GameSwitcher.` OFF shows no return note. Integration
availability is only in About. About uses “When enabled” / “When disabled” headings with the existing section-heading
font/size, bold white on this explanation page only, regular-face text
where installed, inline B/START/A badges, and a short final direct-exit paragraph.
No dividers or control columns; the short explanation fits on one page in the tested
light/dark themes. Measured paging remains available if another theme needs it.

Secondary descriptions (including the keep-file sentence) prefer the resolved
console-heading color. A sampled contrast check against the composited background
or dialog material requires 4.5:1; otherwise normal theme text is used. Badges,
actions, titles and errors retain their prominent colors. About has no fixed
description panel.

Removal is a centered 520px-wide modal with 24px padding. The keep-file note uses
the existing readable body point size, not the smaller description size. The
loader first validates fonts in current-profile → active-theme → existing Onion/Miyoo
fallback order. Relative paths are resolved against the configuration that supplied
them; unusable font data falls through just like a missing file. For explanations,
it inspects native style/family and searches only current-profile and active-theme
font directories for a suitable regular face. A heavy valid face never triggers a
system fallback or an unrelated-theme search. Explanation fonts receive no
synthetic bold; readable point sizes are preserved. CleanOnionGB resolves Inter Regular
(23pt body, now 21pt description). mini.os resolves Nunito Bold (19pt body, now 19pt
description): no same-family regular face was found on the card, so native weight
is retained and logged. Clearing synthetic bold would not make that font regular.
No font is bundled or substituted from an unrelated theme.

Modal controls: Left/Right selects Cancel/Remove, initially Cancel; A activates,
B cancels, MENU closes menus. Up/Down pages long titles without moving the action
selection. Held activation/Left/Right presses are ignored. Footer: A Choose, B Back.
Paging preserves any displayed error. Exact-record removal safeguards are unchanged.

## Host checks

`sh tests/run-local-checks.sh` covers all preference combinations, missing/malformed/
oversized/symlink files, old return-file migration, staging/publication failures,
unchanged generation/favorite/history sentinels, independent display/sort modes,
flat navigation, selection/duplicate source identity and real guarded removal after
rebuilding. Existing menu, removal, state, launch/recent, launcher, switcher and
runtime ownership regressions also run. Publication permission failure is exercised
as a nonroot host user; that case is explicitly skipped under root. Native SDL
previews check measured bounds, all Settings rows, Help pages and the measured About page, modal
paging/errors, hidden hints, missing assets and untouched borrowed browser fonts.
ARM compilation is separate from hardware validation.

## Device acceptance checklist

- Start with no browser file: ON/Show/Original; existing Automatic return preference,
  availability and generation remain intact. Malformed browser data uses defaults.
- Cycle all eight browser combinations via A and Left/Right; hold each key to check
  repeats do not cycle values. Restart and confirm saved choices. Up/Down scrolls
  five rows while selected descriptions stay above the footer, without overlap.
- Flat mode has no console headings; Left/Right does nothing in the browser and Help
  says so. Grouped console jumps still work. Literal sort is not source-file order;
  Alphabetical title ignores numeric prefixes; Show/Hide never changes the chosen sort.
- Change every option from a nonfirst favorite and a scrolled viewport. Verify the
  same original launch/ROM stays selected and visible, including duplicate records.
  Remove one selected record afterward, confirming Cancel-first behavior, exact
  removal, backup and preservation of the ROM/art/history/unrelated records.
- With the browser preference destination unwritable, a change reports failure and
  retains the previous value/model/file. Confirm Settings alone leaves favorite and
  recent files unchanged. Restore access; do not manufacture faults on valuable data.
- Check light/dark text, selected chevrons, all descriptions, About, ON-only
  B/START note, availability in About, side-by-side modal actions, Up/Down title
  paging and hideHints. The new modal mapping/presentation still needs device signoff.
- Smoke A launch, MENU GameSwitcher without recent registration, B normal exit,
  OFF ordinary return and ON session return/switching, saved browser position and
  navigation audio. Record device/theme/version and tested binary hash.

The user has confirmed normal browser Settings functionality; fault cases above
remain a host/device verification matrix, not implied hardware observations. Update
roadmap/status after the new presentation/modal controls receive device feedback.
M3 horizontal browser titles is separate.

## Final checkpoint device follow-up

- Check centered heading, short/long title and keep-file message in the wider modal.
  Cancel/Remove stay side by side; Left/Right chooses, Up/Down pages a long title,
  A activates once, B cancels. Confirm Cancel starts selected.
- Check slightly larger Settings/About text with light and dark active themes.
  ON-only inline B/START note must fit within two lines. The smaller reserved panel
  must remain above the footer with separation from scrolling Settings rows.
- Reopen the app to confirm saved preferences/position; smoke-test A launch, MENU
  and automatic return. No exhaustive device failure-path coverage is implied.
