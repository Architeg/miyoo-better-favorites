# M5: per-parse emulator-label cache (local review)

> **Historical engineering evidence.** This page records development decisions and tests at the time. Use the [current installation guide](install.md) and [recovery guide](recovery.md) for released packages.

## Scope and evidence

Only `FavoritesParser::loadFavoritesFromText` changes in production. A local map
stores the resolved console label for each exact `systemId`, including ID/Unknown
fallbacks. `resolveSystemLabel` and its JSON/file behavior are unchanged. The map
is destroyed when each parse returns; both startup and favorites reload/removal
call that function, so the next parse reads configs again. There is no global or
parser-object cache, stale cross-reload label, canonicalized key/path, file write,
new dependency or retained cache during gameplay.

The corrected before sample has 70 favorites / 3 config paths and 70 config reads
per parse. Emulator config phase costs: 19.948 ms on the first post-boot run,
7.442–8.445 ms on two warm runs. First-presentation totals were 505.596 ms and
265.012–277.075 ms. One planned warm archive is missing. Preserve raw evidence;
these small samples establish no speedup or complete M5 acceptance.

Read-only host comparison on that same mounted corpus confirms **70 → 3** reads,
with byte-identical labels, launch/ROM/artwork paths, console IDs/labels and exact
source records/offsets. This eliminates 67 duplicate config reads for that parse;
it does not measure target startup or RAM savings. Map nodes and copied strings
are transient allocations whose net target cost is not yet measured.

## Checks

- Focused fixtures compare all output fields with the exact pre-cache parser.
  They cover repeated systems, UTF-8 labels, unknown systems, missing configs,
  malformed JSON, non-string/empty labels, non-object config and a directory in
  place of a file. Invalid favorite lines retain existing behavior.
- Reusing one parser across `loadFavoritesFromText` and `loadFavorites` verifies
  edited labels, newly created configs, repaired malformed configs, removed configs
  and valid-to-malformed changes appear immediately on the next parse. Empty/bad
  favorite input performs no emulator config reads.
- Profile counter checks establish one resolution per config path per parse,
  including cached failure fallbacks. Source identity remains unchanged for removal.
- Full local regressions and shell syntax pass, including UI/settings/removal,
  navigation/scrolling, handoff, return ownership and profiling collectors.
- Docker `source /root/setup-env.sh && make -B all` passes. The resulting binary is
  ARM32 little-endian EABI5, 380,872 bytes, SHA-256:
  `2b31a22731711caec94e4bb000a379ab431ef4ec85ba4bb017a41b3ac6cf07ec`.
  Existing `libbz2.so.1.0` linker warning remains; GCC emits ABI-change notes.
- Host parser checks use the existing cJSON compatibility layer. They compare the
  same inputs against old/new code; they do not claim full json-c equivalence.
  The ARM executable links the unchanged json-c library. Device acceptance pending.

Repeat focused host checks (existing host libcjson, no dependency installation):

```sh
sh tests/run-emulator-label-cache-checks.sh /tmp/NEW-UNUSED-test-directory
# Optional second argument: saved pre-cache favorites_parser.cpp.
# Optional third argument: read-only mounted corpus root, e.g. /Volumes/MIYOO.
```

The separate optimization diff is relative to the already-local M5 instrumentation,
not M4 HEAD. Builds, comparison output/raw titles, logs, snapshots and binaries
remain outside Git. The initial review stayed local; reviewed deployment is recorded below. No commit or push.

## Reviewed deployment and normal device acceptance

At the user's direction the reviewed cache binary is now deployed, with startup
profiling **disabled**. No new collection session was prepared. Only the binary
and its installed-binary hash in the existing profiling ownership manifest changed;
that bookkeeping keeps hash-guarded M4 rollback usable. The Onion return manifest,
launcher, runtime/helper, data/preferences/state, libraries and prior logs remain
unchanged. The original M4 binary/launcher backups are preserved.

Verified previous binary and profiling manifest backup:
`/Users/valeriybagrintsev/IT Projects/miyoo-better-favorites-backups/20261003-033812-m5-cache-deployment/`.
ARM format, source/destination bytes, executable permissions, launcher syntax,
protected-file hashes, profiling-disabled status and sync pass. New SHA-256 is
recorded above. The code remains uncommitted; normal hardware acceptance pending.

The build's behavioral acceptance remains unverified. The user closed this pass
without further device tests, Terminal commands, observers or profiling runs.
Use existing collected startup logs as **pre-cache evidence only**. Host tests
confirm 70 → 3 config reads, but no measured post-cache startup gain or memory
change exists. The previous proposal for another before/after logging session is
superseded by the user's instruction to keep profiling disabled for this deployment.

No audio, font ownership, rendering, navigation, return protocol or broad cleanup
changes. M5's unmeasured gameplay/OFF-ON comparison remains an evidence limitation.

## M5 pass closure

User requested closure without additional device tests or profiling sessions.
The 70 → 3 config-read reduction and equivalent output/failure/reload behavior are
**host-verified**. Device startup improvement and this binary's behavioral acceptance
remain **unverified**. No normal game check is requested to close this pass.
Production launcher was restored; profiling-owned card files/evidence/rollback
copies were verified and archived on host. Cache binary remains deployed with
profiling inactive. See [archive and explicit evidence deferrals](m5-profiling.md).
The cache is published in a separate commit from reusable measurement work.
