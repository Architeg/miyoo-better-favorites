# Architecture

## Principle

Better Favorites does not replace Onion's favorites database.

The existing Onion favorites file remains the source of truth:

`/mnt/SDCARD/Roms/favourite.json`

Users continue adding and removing favorites through the normal Onion interface.

## Application flow

1. Read `favourite.json`
2. Resolve each favorite to its console/system
3. Resolve the ROM path
4. Resolve matching box art
5. Group games by console
6. Sort games alphabetically inside each console
7. Render a stock-like Favorites interface using SDL2
8. Launch the selected game using Onion's existing launch mechanisms

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
