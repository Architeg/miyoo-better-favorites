# Architecture

Better Favorites is a native C++17/SDL2 wrapper around Onion's existing
`/mnt/SDCARD/Roms/favourite.json`. It does not create another favorites database,
emulator lifecycle or history system.

## Ownership boundaries

| Layer | Responsibility |
| --- | --- |
| Browser | Parse favorites, group/sort/display, navigate, render theme/artwork, remember position |
| App-owned files | Persist browser options and independent Home/return preferences |
| Guarded removal | Remove exactly one captured source record, preserve other fields and game data |
| Binary → outer launcher | Privately stage a request, save position and finish SDL/audio cleanup |
| Onion runtime | Execute game commands, choose cores, manage saves/resume/activity/recent history/GameSwitcher |
| Optional return integration | Reopen an owned Better Favorites session after GameSwitcher; no resident app during gameplay |
| Optional Home integration | Redirect only the existing Home Favorites activation via native AppAction |
| Host installer | Verify, back up, install/restore and completely remove positively owned files |

## Browser and data flow

1. Parse existing favorite records, preserving stored paths and exact source identity.
2. Resolve console labels with a per-parse cache and artwork with bounded fallbacks.
3. Build grouped or flat rows, applying independent display/sort preferences.
4. Restore selection by launch/ROM identity and adjust the viewport.
5. Render using current profile/theme resources before Onion/Miyoo fallbacks.
6. For A or MENU, stage privately, exit the loop and clean SDL/audio before handoff.

Stock Onion menus still add favorites. Removal backs up original bytes and refuses
conflicting changes; ROM/artwork/saves/recent history are never deletion targets.
Favorite/title display settings never write favorite/history data.

## Independent entry and return integrations

The app works from Apps without either patch. Home replacement keeps the existing
tile, label/theme resources and X/Y shortcuts; disabled/unavailable paths follow
stock behavior. Automatic return owns the session originating here, retains it
through game switching, and consumes ownership before reopening. B app exit does
not create a loop. Both switches default OFF and report installation separately.

These are implemented reversible layers, restricted to audited Onion/MainUI bytes,
not generic shell hooks or a new frontend. No separate tile/global shortcut is
included. [Detailed lifecycle and source map](development.md).

## Release boundaries

Every host installer variant shares the same Miyoo payload and safety logic. Host
selection/path handling differs by OS; a platform target is not hardware acceptance.
No proprietary originals or user data belong in Git/packages. Source/notice
correspondence and exact-package qualification remain explicit release work.

[Roadmap](roadmap.md) · [Status](development-status.md) · [Compatibility](compatibility.md).
