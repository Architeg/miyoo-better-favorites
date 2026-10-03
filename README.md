# Better Favorites

Better Favorites makes Onion's Favorites list easier to browse: games can be
grouped by console, numeric prefixes can be hidden, and long selected titles
scroll without moving the rest of the list. It uses your existing favorites and
Onion's game launcher, saves, recent history and GameSwitcher.

**Designed for Mini and Mini Plus; hardware tested on Mini Plus.** The tested
system is Onion v4.3.1-1. Mini and other combinations need community testing.

## Get started

The release candidate offers an **app-only ZIP**: copy its `App/BetterFavoritesTest`
folder into the powered-off card's `App` folder, then open **Apps → Better
Favorites**. The internal folder name is retained for compatibility.

Home Favorites replacement and automatic return are optional, independent
integrations. The packaged offline installer makes verified backups first.
Both preferences default OFF. See the [quick start](docs/install.md) and
[compatibility guide](docs/compatibility.md). Candidate installation still has
[acceptance gates](docs/release/rc.1.md); it is not stable v1.0.0.

## Use it

A launches, B goes back, SELECT opens actions, Y opens Settings, and MENU opens
Onion's GameSwitcher. L1/R1 moves a page; Left/Right jumps consoles when grouped.
Settings remembers grouping, prefixes and sorting. Removing a favorite keeps its
game file, artwork, saves and recent history. [Controls and settings](docs/user-guide.md).

**Restore optional integrations before deleting the app.** Deleting its folder
cannot undo MainUI/runtime changes. Keep your installer recovery folder; it works
without the Miyoo menu or Terminal. [Uninstall and recovery](docs/uninstall.md).

## Help improve it

Report bugs, suggest features or share device/theme test results through the
[issue templates](https://github.com/Architeg/miyoo-better-favorites/issues/new/choose).
The host tool's **Export diagnostics** action collects a bounded report you can
inspect before sharing. No Miyoo Terminal commands are required.

[Contributing](CONTRIBUTING.md) · [Technical guide](docs/development.md) ·
[Roadmap](docs/roadmap.md) · [Changelog](CHANGELOG.md)

Project sources are **GPL-3.0-or-later**. Bundled dependencies retain their own
licenses; see [notices and provenance limits](THIRD_PARTY_NOTICES.md).
