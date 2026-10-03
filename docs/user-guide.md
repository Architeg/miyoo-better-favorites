# Controls and settings

## Favorites list

| Control | Action |
| --- | --- |
| Up / Down | Move selection |
| Left / Right | Jump console; disabled in flat mode |
| L1 / R1 | Page up / down; clamp, no wrap |
| A | Launch selected game through Onion |
| B | Ordinary app exit |
| SELECT | Launch / Remove from Favorites / Settings / Help |
| Y | Full-screen Settings |
| MENU | Onion GameSwitcher; does not add the selected favorite to history |

Only the selected overflowing browser title scrolls after a reading delay; menus
pause/reset it. Long dialog titles use explicit pages rather than automatic changes.

## Inside menus

Up/Down selects rows. Left/Right changes adjustable values; A toggles/cycles or
opens an action. B backs out one level; MENU closes the menu without opening
GameSwitcher. Repeated activation presses are ignored. Filled theme-colored button
labels are text controls; hideIcons still controls decorative browser assets and
hideHints hides footers.

Remove confirmation starts on **Cancel**. Left/Right chooses Cancel/Remove; A
chooses, B cancels. Up/Down pages a long title. Only the selected exact favorite
record is removed; ROM/artwork/saves/recent history stay. A verified backup and
conflict checks precede atomic publication. External changes refuse removal.

## Persistent options

| Option | Default | Meaning |
| --- | --- | --- |
| Group by console | ON | Headings and console jumps; OFF is a flat list |
| Numeric prefixes | Show | Display only; stored labels and sort stay unchanged |
| Sorting | Original label | Literal-label sort; Alphabetical title ignores leading numeric prefix |
| Automatic return | OFF | With integration available, B/START from GameSwitcher reopens this browser session, even after switching games |
| Replace stock Favorites | OFF | With Home integration available, existing Home Favorites opens Better Favorites |

Write failures keep the saved value. Selection follows launch/ROM identity after
regrouping/sorting; removal retains exact record identity. About shows installation
availability separately. Automatic return and Home replacement are independent.

A in GameSwitcher resumes. Direct game exit or ordinary menu return ends the owned
return session; reboot/restart, preference disable or generation change clears it.
Ownership is consumed before reopening to prevent a loop. B from Better Favorites
uses Onion's ordinary app return: Home when opened from Home (verified), Apps when
opened from Apps. It does not create automatic return ownership.

Global shortcuts are deferred entirely for v1.0. Ordinary MainUI X/Y assignments
are unchanged. Adding favorites still uses Onion's own menus.
