# Development status

Updated 2026-10-06. [Roadmap](roadmap.md) · [Release notes](release/notes-rc.7.md) · [Contributing](../CONTRIBUTING.md)

## Current delivery

The published **v1.0.0-rc.7** package is built from `462ebd0309ad329b8852e1791e1835327d117301`.
It contains one ready-to-install ZIP, matching project/dependency source, notices
and checksums. The Mac-recommended/Linux-optional Terminal downloader reuses that package. The bundled documentation snapshot is `e576c3e`; later repository branding and guide changes are not yet bundled.
Both supported system integrations install together and default OFF; updates
preserve preferences. Complete uninstall restores verified stock files and keeps
a computer-side recovery archive. Portable card recovery supports changing computers.

## Milestones

| Milestone | Status |
| --- | --- |
| M1–M2: menus, readability and browser settings | Implemented; recorded device acceptance |
| M3: selected long-title scrolling | Implemented; recorded device acceptance |
| M4: paging and resource fallbacks | Implemented; recorded device acceptance |
| M5: startup/RAM profiling and label cache | Closed; host config reads reduced 70 → 3; no measured device speedup claim |
| M6: Home Favorites integration | Implemented; OFF/ON and return behavior accepted on the tested Mini Plus |
| M7: compatibility and targeted cleanup | Documented targets, tests and bounded diagnostics; broader community testing continues |
| M8: packaging, install/uninstall and docs | Implemented for the shared host backend |
| M9: versioned distribution | Package and source companions published; existing release published; bundled docs lag current main |

## Recorded hardware and host evidence

- Miyoo Mini Plus MY354, firmware **202306282128**, Onion **v4.3.1-1**;
  hardware revision unknown. Browser/audio/themes/settings/removal, A launch,
  MENU/GameSwitcher, B/START return and Home OFF/ON have recorded user acceptance.
- Mac M1/Ventura installation and complete uninstall passed on the preceding package.
  Earlier tests identify Ventura 13.7.8. Intel installation succeeded after a
  reported receipt conflict; RC7’s automatic correction has host regression coverage.
- Windows installation and complete uninstall passed with the built-in selector
  and no reported security warning. Earlier tests include Windows 7 SP1 x64 and
  Windows 10 x64; the latest run did not identify its Windows version.
- Linux package/container and architecture fixtures pass; physical reader/device
  cycles are not yet recorded.

[Compatibility matrix](compatibility.md) · [Exact RC7 evidence](release/rc.7.md).

## Focused follow-up

Repeat the Intel cross-computer receipt sequence and check the final Settings
Back/B/START presentation. Community tests are welcome on Linux and Mini devices.
Keep new-byte acceptance separate from historical tests. Global shortcuts remain
deferred beyond v1.0; they are not required to use Home or Apps access.

Dependency notices/source are supplied. Exact reproduction of every permissive
prebuilt remains a documented engineering goal, with no new specific missing
SwiftShader notice identified. [Dependency audit](release/dependency-audit.md).

Historical startup measurements, earlier bugs and preparation decisions remain in
[the engineering index](developer-index.md); they are not current installation instructions.

## Documentation and packaging follow-up

The live release description is synchronized into committed release notes without changing its ZIP checksum. Root artwork references now use `assets/icon.png`; the installed icon destination is unchanged. README is intentionally unchanged: its first-launch SELECT shortcut wording should read SELECT → Settings, since SELECT opens Actions. Its Big Sur approval reference is not a supported target: the installer requires Monterey or newer.

The published ZIP still needs a separate documentation/branding refresh. No executable, release asset, tag or SD-card file was changed by this documentation cleanup.
