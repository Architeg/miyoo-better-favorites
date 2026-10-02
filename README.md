# Better Favorites

Better Favorites is an open-source replacement favorites browser for Onion OS on the Miyoo Mini and Miyoo Mini Plus.

## Goal

Keep Onion's normal **Add to Favorites** workflow while replacing the Favorites browsing experience with a segmented interface grouped by console.

Example:

Game Boy
- Kirby's Dream Land
- Tetris

Game Boy Color
- Pokémon Crystal
- Zelda: Oracle of Ages

Game Boy Advance
- Advance Wars
- Metroid Fusion

Only consoles containing favorites are displayed.

## Planned behavior

- Reads Onion's existing `Roms/favourite.json`
- Does not maintain a separate favorites database
- Groups favorites automatically by console
- Displays box art
- Mimics the stock Onion Favorites interface
- Adds visible console dividers
- Launches games through Onion's existing emulator configuration
- Eventually launches from the normal Favorites tile on the Onion Home screen

## Platform

- Miyoo Mini
- Miyoo Mini Plus
- Onion OS

## Status

Native SDL2 browser with hardware-confirmed Onion launch/GameSwitcher return,
SELECT actions, Settings/Help and safe favorite removal. Final menu readability
adjustments await device verification. See [development status](docs/development-status.md).
The normal Home-screen Favorites tile integration remains planned.
