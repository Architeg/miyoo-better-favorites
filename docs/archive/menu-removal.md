> **Historical engineering record — not current installation instructions.**
> Use the [installation guide](../install.md) and [compatibility matrix](../compatibility.md) for the released app.

# SELECT actions, full-screen Settings and favorite removal

> **Historical engineering evidence.** This page records development decisions and tests at the time. Use the [current installation guide](../install.md) and [recovery guide](../recovery.md) for released packages.

Based on hardware-verified MENU checkpoint
`2f7571d03055c0c11de69625c98ff9d05788f031`. Navigation/removal and preceding
presentation are user-confirmed on device across themes. Browser Settings are also
reported working. The latest targeted modal presentation/control changes await
device verification.

## Controls and UI

- Browser Up/Down selects games; Left/Right selects the previous/next console
  in grouped mode, and is disabled in flat mode.
- A launches the selected game through the existing exit-20 Onion handoff.
- SELECT opens the top-left, four-row action menu: Launch, Remove from Favorites,
  Settings, Help. Launch/removal are dimmed and inert without a selected game.
- Y opens full-screen Settings directly. Automatic return ON/OFF is the first
  selectable row; A or Left/Right changes the real preference and generation.
  Grouping, numeric prefixes and sorting follow; the final About action explains
  automatic return and reports integration availability.
- A activates a menu row; B backs one level. Browser B exits normally.
- MENU closes all open menu pages, consuming that press; only browser MENU opens
  Onion GameSwitcher through the verified exit-21 flow.
- Help lists these working controls. No stock-only core/reset/scraper options.
- Removal confirmation names the game and initially selects Cancel. Left/Right
  chooses Cancel/Remove in one horizontal row; A activates. A on Cancel or B
  returns to actions. MENU cancels back to the browser. Up/Down only pages long
  titles; it cannot choose Remove. Repeated activation presses are ignored.

Settings/Help retain the existing header/footer geometry and theme fonts/assets.
SELECT uses a compact top-left popup; menu selection reuses the browser highlight.
Removal uses a centered 520px-wide modal with 24px padding over a subdued browser.
Bounded titles page explicitly with Up/Down, with matching title-page hints; errors
remain complete and survive paging. Footer badges say A Choose, B Back. The keep-file
note retains the full readable body size and uses a verified regular face where
the theme supplies one; mini.os has only Nunito Bold, a documented limitation. Browser
geometry, artwork/navigation and OSS/libpadsp initialization remain unchanged.
The current active theme path exists; no card configuration was changed.
See [presentation/resource resolution](menu-presentation.md).

## Verified Onion reference and limits

Mounted v4.3.1-1 `Roms/favourite.json` has 70 valid, independent JSON lines.
Stored records contain label/launch/type/imgpath/rompath, including `/../../`.
Matching [JsonGameEntry_fromJson](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/components/JsonGameEntry.h)
uses this file/record format. The matching
[stock removal prompt, entries 81/82](https://github.com/OnionUI/Onion/blob/v4.3.1-1/static/build/miyoo/app/lang/en.lang)
says removing from favorites does not delete SD-card content.
Mounted `App/romscripts/favorites/favfix.sh` invokes `tools favfix`; matching
[keymon main](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/keymon/keymon.c)
triggers that repair after favorites change in MainUI. We do not run repair/sort
or copy their rewrite behavior: that would unnecessarily rewrite other entries.
MainUI and the mounted tools implementation are binaries; MainUI's exact removal
algorithm and conflict handling are not source-verified. This patch follows the
verified record-only semantics and adds its own conservative publication checks.

## Removal transaction

1. Read a bounded regular file without following a symlink; capture bytes, inode,
   device, size, mode, mtime and ctime. Before/after read and pathname identity must
   agree. Feed these same bytes to `FavoritesParser::loadFavoritesFromText`.
2. Parse one complete JSON object per line, retain each accepted record's exact
   byte offset/span through grouping/sorting. Invalid/non-object/trailing JSON
   lines are omitted from browsing but preserved on disk. Duplicate labels and
   identical entries remain distinct source spans.
3. On confirmed removal, compare the current file with the original snapshot.
   A conflict refuses removal and asks the user to reopen the app.
4. Create an exclusive random backup beside the file:
   `Roms/.favourite.json.better-favorites-backup.XXXXXX`. Write/fsync/close, reread
   and verify it byte-for-byte, then sync the directory. A failed backup stops
   before replacing the source. Backups are retained, including an incomplete
   backup if its verification failed. No automatic backup deletion.
5. Erase exactly that source span in memory. Write/verify/fsync a same-directory
   temporary file; compare the source snapshot again; rename atomically and sync
   the directory. Failures clean only this transaction's temporary file.
6. Re-read/group/rebuild rows; select the successor at the old selectable ordinal,
   clamping to the previous game when the final entry was removed. Empty lists
   remain usable (Settings/Help/GameSwitcher/B); launch/removal are disabled.
   Apply the existing viewport correction and save the new nonempty position.

Every unrelated record, unknown field, raw path, newline and malformed unselected
line is preserved byte-for-byte. ROMs, artwork, saves and both recent lists are
never opened for writing by removal. After a successful rename, directory-sync
failure is reported as a committed removal with a retained backup, not falsely
reported as an unchanged source or rolled back over a later writer.

The snapshot checks detect observed external changes, including same-byte inode
replacement. POSIX rename is not compare-and-swap: a noncooperating writer in the
last comparison/rename gap remains a concurrency assumption. Onion serializes
MainUI and the Apps launcher on the verified runtime; do not externally edit the
card while the app is removing a favorite. Separate checks do not eliminate this
race. Backups permit manual recovery; inspect later changes before restoring one.

## Checks and remaining device work

Run `sh tests/run-local-checks.sh` for menu nesting/repeat/disabled-action tests,
Cancel-first confirmation, nearest-row fallback and removal fixtures: unknown
fields/CRLF/final unterminated line, duplicate labels, last favorite, mismatch,
external append/replace before publication, backup creation/verification failure,
replacement staging/publication failure, symlink refusal and unchanged ROM/art/
save/history sentinels. Tests run in isolated host paths.
Existing A/recent registration/rollback, MENU, launcher, preference/state and
session ownership regressions are included. Runtime installer fixtures remain
available with their pinned original runtime reference. Native json-c is not
available on the host; parser changes are ARM-compiled, and the real card's JSON
lines were validated read-only. No additional dependency was installed.

Build with the existing Docker ARM command. Compilation is not hardware proof:
the preceding UI/removal was reported working on hardware across themes. Final
font/filled-control readability changes still require device verification.
The runtime helper, patched runtime and installed manifest require no update.
Future deployment should replace only the rebuilt app binary with verified backup;
launcher remains byte-identical to the hardware-verified checkpoint.

Host `/private/tmp` inventory: `python3 tools/inventory-host-temp.py`. Read-only,
project-prefix-filtered, sizes are logical bytes and symlinks are not followed.
No device `/tmp` is accessed. Reusable checks live in the repository; old temp
edit/deploy scripts are checkpoint-specific artifacts, not supported reusable tools.
