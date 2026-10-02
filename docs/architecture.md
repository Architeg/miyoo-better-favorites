# Architecture

## Principle

Better Favorites does not replace Onion's favorites database.

The existing Onion favorites file remains the source of truth:

`/mnt/SDCARD/Roms/favourite.json`

Stock Add to Favorites remains the adding workflow. Stock removal remains
available; Better Favorites also supports guarded removal of exactly the selected
original record with a verified backup and atomic replacement. ROMs, artwork,
saves, recent history and unrelated records/fields are preserved.

The [authoritative roadmap](roadmap.md) records accepted defaults, unresolved
settings and the complete milestone order; [status](development-status.md) records
implementation and verification evidence.

## Application flow

1. Read `favourite.json`
2. Resolve each favorite to its console/system
3. Resolve the ROM path
4. Resolve matching box art
5. Group games by console
6. Sort within groups by original label by default; alternate prefix-ignoring title sorting has parser support but no persistent Settings control
7. Render a stock-like Favorites interface using SDL2
8. Privately stage the selected game/history request; finish SDL/audio cleanup
9. Hand off through the outer launcher to Onion's existing runtime mechanisms

Onion owns cores, saves/resume, activity tracking and GameSwitcher. Optional
session-return integration restores the app/browser position; it is distinct from
the unfinished Home Favorites entry integration. Current grouping is unconditional;
a real persistent grouped/flat choice remains a roadmap milestone.

## UI

The application will be a native C/C++ SDL2 application.

Shell scripts are reserved for:

- installation
- uninstallation
- backups
- integration with Onion
- Favorites Home tile handoff

## Favorites tile integration

The native application must work independently before modifying the Onion Home screen.

Favorites tile integration will therefore be a separate, reversible layer.

Target flow:

Onion Home
→ Favorites
→ Better Favorites
→ selected game
→ Onion/RetroArch launch mechanism
