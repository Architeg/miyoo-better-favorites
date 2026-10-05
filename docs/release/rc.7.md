# RC7 qualification and provenance

[Published wording](notes-rc.7.md) · [Compatibility](../compatibility.md) · [Build](build.md)

## Published package

RC7 uses one `better-favorites-1.0.0-rc.7.zip` for Windows, Mac and Linux. Source archives and dependency companions are for development/licensing, not extra installation steps. Binary source is `462ebd0309ad329b8852e1791e1835327d117301`.

The published ZIP's bundled documentation was refreshed from `e576c3e2956cb8b1e57f4385df1b50ebcab665ae`; it does not yet contain current main's branding, screenshots and subsequent guide edits. The published checksum remains unchanged in [release notes](notes-rc.7.md). Repository docs, the separately maintained release description and downloadable assets must not be assumed synchronized.

## Changes and host evidence

- Recovery-backed Home receipt reconciliation and complete-uninstall stock absence.
- Settings-session availability caching and B/START text-control fixes.
- Shared-backend install/update/uninstall/reinstall, portable recovery, metadata and foreign-change checks.
- Archive integrity, exact payload/library hashes and documentation-only refresh checks.

These are host/build checks, not device acceptance of every RC7 correction. The published release wording is edited independently by the owner and takes precedence over earlier draft prose. See the [archived engineering evidence](../archive/release/rc7-engineering-record.md) for the exact earlier observations.

## Remaining targeted checks

- Intel: cross-computer full uninstall → copy RC7 → install without manually deleting receipts; verify complete uninstall and stock boot.
- Device: Settings Back behavior and separate B/START badges after the final correction.
- Community: physical Linux x64/ARM64, Mini and other unaudited host/device combinations.

Keep earlier accepted package results separate from new-byte tests. No ARM build, emulation or static hash check alone establishes hardware behavior. [Current evidence matrix](../compatibility.md).
