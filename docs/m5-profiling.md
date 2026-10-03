# M5: startup and memory measurement before optimization

Status: **M5 measurement pass closed with explicit deferrals on 2026-10-03**.
No further device tests, profiling sessions or Terminal commands are requested.
M6 may proceed. The procedures and activation history below are retained for
reproducibility; they are **inactive historical instructions**, not the current plan.

- Original pilot: post-boot 505.361 ms; three warm runs median 267.590 ms,
  range 262.359–277.706 ms. Prelaunch hashing warmed executable/corpus files.
- Corrected session: post-boot 505.596 ms; two warm runs median 271.043 ms,
  range 265.012–277.075 ms. Planned third warm archive is absent; all seven
  available startup runs succeeded. Post-boot and warm populations stay separate.
- Pre-cache idle ON: three valid samples per role; browser smaps RSS 19,308 kB,
  PSS median 17,449 kB (17,449–17,450); runtime RSS 1,808 kB, PSS 357 kB.
- Metadata/corpus reads, instrumentation, scheduling, cache/SD state and timing
  proxy limits remain. No unbiased cold baseline or measured cache startup gain.
- **Deferred:** gameplay memory, OFF/ON comparisons, post-scroll/menu memory and
  process-absence measurements. Deferrals do not become claims of verification
  and do not block M6 read-only entry-point investigation.

M4 accepted binary remains archived (`aa486abad4e3272f86605945109bf0a0f76159ab373f87c1e57c38abd034b328`).
Raw evidence remains outside Git; original logs and rollback files were verified
and archived on host before temporary card copies were removed. Profiling is
inactive and the exact production launcher restored. See closure/archive below.

## Audit findings before changes

| Finding / source evidence | Expected benefit if device measurements justify a change | Regression risk / decision |
| --- | --- | --- |
| `FavoritesParser::loadFavoritesFromText` calls `resolveSystemLabel` for every parsed record; that opens/parses Emu/config.json each time | A parse-local label cache could reduce 70 reads to 3 on this corpus | Stale labels across reloads, missing/malformed-config semantics. Implemented separately as a per-parse cache; host 70 → 3 reduction verified, target gain unmeasured |
| `resolveThemeFonts` opens/closes all four styles for validation, then main opens usable instances; menu `readableFont` / `regularFont` probe more faces/sizes | Avoid validated-face reopening or repeated probes | Profile/theme priority, mutable styles, different sizes, honest regular-face behavior and font lifetime. Path repetition does not mean redundant instances. **Not implemented** |
| Main and MenuRenderer both decode the horizontal divider; menu owns its surfaces | Sharing an immutable surface could save decoding and one retained surface | Ownership/double-free, menu release before SDL teardown; savings unmeasured. **Not implemented** |
| Image resolution tests existence, then decodes; M4 retry may attempt missing candidates | Fewer repeated probes if significant on the card | Active-profile/theme/system fallback and corruption handling. No shortcut without measured evidence |
| `FavoritesSnapshot.bytes`, original favorites/sourceRecord strings and grouped copies coexist (`groupBrowserFavorites` copies records) | Potential memory reduction for large corpora | Snapshot conflict checks, exact removal identity, regrouping, row pointer lifetimes. Not proven unnecessary; **retain** |
| BrowserTitles caches only used text surfaces per frame; artwork caches only the selected path; audio device/WAV open once; all released before handoff | Existing bounded caching already prevents repeated title/font/image I/O while scrolling | Preserve working behavior; do not replace with speculative caches |
| Main heading/footer text helpers allocate/free rasterized text each frame; MenuRenderer samples/temporarily converts surfaces during construction | Potential CPU/allocation savings, distinguish startup from browsing | Theme/font/text changes and resource invalidation; measure before changing |
| `Theme::selectedItemPath` / `normalItemPath` have assignments but no rendering consumer; some initial asset path assignments are overwritten | Cosmetic/dead-lookup cleanup candidate only; no measured worthwhile saving | Separate cosmetic review; no removal here |
| SDL2, SDL_image, SDL_ttf, SDL_mixer, json-c all have live calls; EGL/GLES are part of the deployed SDL backend | No confirmed unused production dependency | Indirect dependencies require target link/runtime evidence. No library/asset/dependency deletion |

Existing host tools were menu/resource/title checks, not full startup or device
memory profilers. [onion-return.md](onion-return.md#state-disk-sizes-and-device-memory-measurement)
already defines target PID ancestry, OFF/ON comparisons, RSS/PSS and observer limits.
The new tools reuse it; no runtime, launcher or GameSwitcher change is required in
this review diff.

## Opt-in instrumentation

Set `BETTER_FAVORITES_PROFILE=1` for the GUI process. All other values disable it.
Normal runs perform no profiling clock reads, `/proc` reads, metric allocations or
profile output. They retain small inactive scope/branch overhead; that overhead
is not claimed measured or zero. Handoff CLI modes never initialize profiling.

`startup_profile::Session` starts after GUI argument validation. Scopes measure:

- Theme loading, each theme/config open+read+parse attempt.
- App/browser preferences, favorite snapshot read, parsing, grouping/row creation,
  browser position restoration.
- Each Emu config read+parse: calls, distinct paths, total duration (including failures).
- SDL video/audio/events initialization, image/TTF initialization, display setup.
- Each font open/probe, font validation, each PNG attempt, menu resource construction.
- Mixer opening, WAV decode, volume config and total audio setup.
- First frame including event polling, artwork/text rendering, upload and present.

At the first SDL present, after the existing input poll and successful upload/
clear/copy, emit `BF_PROFILE` JSON lines to existing stderr logging, once, then
release all metric/path containers. Failure is `presentation_error` or
`incomplete_startup`, not a successful frame. No periodic logs, new thread or
resident helper. A returned SDL present is the **first usable frame proxy**; actual
visible/input usability still requires device observation. SDL_RenderPresent has
no result code; this is not proof of physical scanout or input response latency.

Times are monotonic microseconds. Totals are **inclusive**, so do not add parsing
and Emu-config time, or menu and font/image time. `unique` counts resource paths,
not font face/size/style instances. Missing candidates and failures count too.
PNG file decode is separate from text glyph rasterization, which is included in
first-frame work. Later browsing/menus and cleanup are not timed by this startup
session. ON/OFF retention needs separate `/proc` samples.

Device `/proc/uptime` timestamps at GUI entry and first-frame completion correlate
with the launcher's opt-in hook timestamp. They are coarser than phase clocks and include
launcher setup and dynamic linking. **Dynamic linker time alone is unavailable**;
no supported target linker trace has been established. Never subtract inclusive
phase sums to label the remainder as linking. Avoid suspend between timestamps.

## Tools and conditions

- `tools/profile-host-startup.sh BACKEND OUTPUT CARD [RUNS]`: partial, read-only host
  loader observation using existing SDL backend objects and the host cJSON bridge.
  Not the ARM json-c library, OSS backend, renderer, or first usable frame. It does
  not decode the selected game's artwork or honor the saved selection; record
  these exclusions when using the result.
- `tools/manage-profiling.py prepare --output NEW_HOST_DIR` generates a diagnostic
  launcher from the accepted repository launcher. `activate --powered-off` requires
  the exact accepted M4 binary and launcher, verified backups, ARM diagnostic binary
  and no pre-existing diagnostic files/evidence. The marker is published last.
  `disable` removes only the owned marker; `rollback` restores verified M4 binary/
  launcher and removes only this pilot's hook/marker. Backups/logs remain intact.
- `tools/profile-device-launch.sh` is now a **sourced hook**, not an Apps wrapper.
  This correction matters: `bf_return_is_app` recognizes an exact command string.
  Changing the Apps command to invoke a wrapper breaks origin adoption. Keep
  config.json, runtime/helper/manifest and the normal `./launch.sh` command unchanged.
  Generated launcher sources the hook internally; export/archiving do not modify
  `BETTER_FAVORITES_RETURN_DIR`, generation, pending/active commands or exit 20/21.
  Host fixtures verify cleanup-before-publication and real helper adoption/reopen.
  The same existing launcher exits before gameplay; no extra resident process.
  Four directories `App/BetterFavoritesTest/.profiling-results/pilot-0001..0004`
  capture one intended cold and three warm runs, then automatic profiling stops.
  A label is not verified cache/boot evidence; user must confirm conditions.
- `tools/sample-device-memory.sh VERIFIED_PID NEW_SAMPLE_DIR`: single snapshot of
  status/stat, smaps_rollup or summed smaps, meminfo and PID/PPid/comm/executable
  inventory. Missing PSS is explicitly unavailable, never zero. `/proc/PID/stat` field 22 is
  parsed after the final comm parenthesis before and after collection. PID reuse,
  exit/zombie, malformed stat or failed collection creates `validity.txt` with
  `valid=0`, retains partial evidence and returns failure. It does not read
  arbitrary command lines/environments. Use once for app PID and separately for
  the verified runtime/game PID. Naming/ancestry, not the first pidof result,
  identifies the runtime. Existing sample directories are refused.
- `tools/summarize-memory-profile.py SAMPLE_DIR...` accepts only `valid=1` samples
  with equal numeric before/after starttimes. Invalid/unverified samples are printed
  as EXCLUDED and never enter any aggregate. Missing fields are not zero-filled.
- `tools/summarize-startup-profile.py LOG...`: outcome-separated per-metric count,
  median, min/max. Successful, failed, incomplete, host-partial and unattributed
  populations remain separate for all phases and launch correlations.
  Adjacent `metadata.txt` permits coarse launch-to-main/first-frame correlation.
  Keep conditions separate; never pool OFF/ON or cold/warm results.

### Repeatable device procedure (not executed)

1. Review/deploy the opt-in build separately with verified backups. Keep accepted
   M4 binary for rollback; record both hashes. Record Mini/Plus, RAM, kernel, Onion
   version/runtime/helper hashes, SD make/filesystem, CPU governor/frequency,
   services/network, theme/config/font hashes, favorite corpus hash/bytes/count,
   image dimensions and saved selection/viewport/browser settings. Do not store
   personal favorite records in Git. Record any unsupported CPU/PSS fields.
2. Use the same theme, corpus, selected game, settings and services throughout.
   Define operational **cold** as first app launch after a clean boot and fixed
   30-second settling interval, 5–10 separate boots per OFF/ON condition. Onion
   may already warm theme files; do not call this a proven empty-cache run. Do not
   force drop_caches or alter libraries/audio configuration.
3. Define **warm** as 10 fresh app processes reopened with normal B exit between
   runs on the same boot. Preserve the same selection and wait interval. The
   diagnostic hook runs inside the unchanged Apps launcher; each run has its own directory.
   Normal startup tests do not launch/remove games or modify favorite/history.
4. Retain raw profile logs and filled-in metadata. Example after a reviewed
   diagnostic launcher is installed: open BetterFavoritesTest normally from Apps;
   do not run the sourced hook directly. Do not overwrite runs. Correlate
   first-present marker with device/video observation and one navigation response.
   Check sound, responsiveness and the profile-disabled path. Compare instrumented
   versus disabled runs externally to estimate measurement overhead.
5. In a separate run, sample the app after 5 seconds stationary, after the same
   scrolling/menu sequence, then after 10 seconds stationary. Use known app PID
   from the profile/launcher log, and record process starttime to detect PID reuse.
   Example: `sh /path/to/sample-device-memory.sh "$verified_app_pid" /mnt/SDCARD/profiles/off-browser-01`.
   A one-shot observer may briefly perturb responsiveness; startup timing runs
   should not have a concurrent memory observer.
6. For gameplay compare installed integration OFF and ON, toggled using the working
   Settings UI (never hand-edit generation/state). Same game/core/save/scene and
   30-second settle after launch. Trace the game → exe/launcher → actual runtime
   ancestry; record known launcher/app PIDs and starttimes. Sample the
   runtime and game individually 5 times each, with identical intervals. RSS is
   not additive for shared mappings; report PSS/private/shared where supported.
   Whole-device meminfo includes caches/observers and is not app RAM.
7. During gameplay, verify the logged app and app launcher PIDs
   are gone (or demonstrably reused), executable inventory has no Better Favorites
   process, and no separate return-helper process persists. Function definitions
   in Onion's existing runtime shell are expected. **Sourced helper absence cannot
   be inferred merely from `pidof`**; inspect captured ancestry/known process IDs.
   Repeat after A resume, switching games, B/START return then relaunch, and an OFF
   launch. Unexpected retained process is a failed acceptance case. Sampling tools
   and any remote shell are observers, not claimed app overhead.
8. Save every repeated result with condition, run number and failure outcome.
   Report median/range (and percentile only with adequate count), unavailable
   metrics and between-run variability. An original-runtime baseline is optional
   under the existing separate rollback procedure; do not uninstall integration
   just to collect OFF/ON data.

### Before/after gate

No performance or cosmetic cleanup is implemented here. Establish device baseline
first, rank measured costs and propose only a small isolated change with expected
benefit and regression risk. Compare it against accepted baseline on identical
device/theme/corpus/state/game conditions with the same instrumentation. Reject
noise-sized or regressive changes. Run regressions/ARM build and require device
acceptance before publishing. Cosmetic cleanup and performance changes must have
separate diffs/commits; M5 profiling remains uncommitted for review. If no worthwhile
bottleneck appears, keep the working implementation.

## Available baseline / unavailable measurements

Only the partial host baseline below is available. Device startup/first usable
frame, audio, dynamic linking isolation, app/runtime RSS/PSS, ON/OFF costs and
absence during gameplay are **not measured**. No before/after optimization claim.

2026-10-03 host: macOS 13.7.8 arm64, native `-O2` driver; preserved native SDL_ttf/
PNG objects and cJSON bridge, mounted card read-only. Theme: `mini.os by architeg`;
70 valid favorite records, 18,935 bytes, 3 distinct emulator-config paths.
Corpus SHA-256: `6b527b64d4e1a620919bc059ccd58747310a0e744872c0860ea7ad9c7dc4bcf6`.
Theme config SHA-256: `d90c337811aba66e485a50475b16dec661d1da571f9e3d09478406939134168a`.
Resolved font SHA-256: `34f790c2b4a9bd25ae90f0647924a84a19fbc86d2c885562d0d10caaf6028c43`.
No cache flush; first observed host run is **not** a device cold run. Next six runs
are in-process warm repeats, not fresh device processes. Native library/JSON/SD
cache behavior differs from the Miyoo. Raw local log: `/tmp/better-favorites-m5-host-baseline.log`.

| Partial host phase (inclusive) | First observation ms | Next 6 median ms | Next 6 min–max ms |
| --- | ---: | ---: | ---: |
| partial_total | 63.540 | 7.467 | 7.353–7.732 |
| theme.total | 9.845 | 0.260 | 0.255–0.277 |
| theme.config_read_parse | 7.683 | 0.059 | 0.054–0.063 |
| favorites.read | 1.420 | 0.070 | 0.066–0.074 |
| favorites.parse_inclusive | 3.735 | 1.107 | 1.080–1.123 |
| emu.config_read_parse | 3.531 | 0.915 | 0.889–0.931 |
| favorites.group_rows | 0.062 | 0.050 | 0.048–0.053 |
| font.validation | 13.076 | 0.181 | 0.176–0.192 |
| font.open | 13.745 | 0.726 | 0.712–0.758 |
| image.decode | 24.067 | 4.301 | 4.260–4.529 |
| menu.resources | 15.505 | 2.683 | 2.628–2.789 |

Each run: 70 Emu config read attempts / 3 paths; 17 font opens / 1 path
(different sizes/styles/probes); 27 PNG attempts / 25 paths (including failures).
The read counts support investigating a per-parse label cache; host timing does
not justify changing the device implementation yet. Repeated total host ms: `63.540, 7.732, 7.387, 7.507, 7.353, 7.467, 7.467`.

## Final local verification

- Existing host behavior regressions plus profile-off/counter/finish-once fixtures passed.
- Observer fixtures passed smaps/rollup/unsupported aggregation, PID validation,
  existing-output refusal, sourced-hook limit/exit and failure-aware summary correlation.
- Native SDL font ownership/layout and corrupt/missing-resource checks passed in
  checks-only mode; no previews generated.
- Docker `source /root/setup-env.sh && make -B all` passed; ARM32 LE EABI5 confirmed.
  Existing SDL_ttf `libbz2.so.1.0` linker warning remains. GCC 8.3 emitted standard
  GCC-7.1 parameter-passing ABI notes for the new metric map (notes, not build errors).
- `git diff --check` and shell syntax passed. Repository production launcher and Onion integration are unchanged. A generated
  diagnostic launcher and opt-in binary/hook/marker are installed for the pilot;
  accepted M4 is verified in rollback backups. M5 is unstaged.

## Initial collection-method pilot — before the full matrix

Only request **one cold plus three warm launches** initially. No full repeated
matrix is requested until these logs have been inspected. Failed launcher exits
after a first frame have their own population too; partial/missing metrics are
not counted as successes or zero-filled.

1. With power off/card mounted, Codex can verify current M4, back it up on host and
   card, activate the diagnostic binary/launcher/hook/marker, verify bytes, modes,
   syntax and untouched data/integration hashes, and sync. No config/runtime edits.
2. User: insert card, boot normally, wait 30 seconds, open Apps → BetterFavoritesTest
   (first launch after that boot). Wait five seconds, open Settings with Y and close it with B to check
   input/sound without changing preferences or position, then B to exit. Repeat opening/waiting/B three times without reboot. Record
   device model, boot/settle conditions and whether first frame/input/sound were usable.
   Keep theme/settings/selection consistent; do not change Automatic return for
   these first method-validation runs. Avoid launching/removing games during them.
3. User: power off and mount the card again. Codex can collect four metadata/log
   directories, inspect startup outcomes/timestamp correlation, then summarize only
   successful runs. No physical device action or live target `/proc` observation
   is possible from a powered-off mounted card. Failures remain separate evidence.
4. Once the method is valid, separately plan a small A/MENU/automatic-return smoke
   and live `/proc` sampling, then request the full cold/warm OFF/ON matrix. Host
   ownership tests are not device verification. Do not assume game process absence
   from logs alone. Rollback to accepted M4 is available at any point while offline.

Activation commands (operator-confirmed powered-off card only):

```sh
python3 tools/manage-profiling.py activate --card /Volumes/MIYOO --powered-off
python3 tools/manage-profiling.py status --card /Volumes/MIYOO
# After collection, stop instrumentation without deleting evidence:
python3 tools/manage-profiling.py disable --card /Volumes/MIYOO --powered-off
# Or restore the exact accepted binary and launcher:
python3 tools/manage-profiling.py rollback --card /Volumes/MIYOO --powered-off
```

Mixed outcome fixtures prove failed runs cannot shift success phase medians.
FIFO-synchronized PID fixtures cover mid-collection reuse/exit; malformed/zombie
identities and missing PSS remain distinct. Activation fixtures cover verified
backup guards, protected data, foreign-file refusal and failed publication/rollback.
The corrected approach has no optimization or cosmetic changes.

### Verified pilot activation

Host M4 backup:
`../miyoo-better-favorites-backups/20261003-013958-m5-pilot-m4-rollback/`.
Card rollback backup: `App/BetterFavoritesTest/.profiling-backup/`.
Both original binary/launcher were byte-verified before replacement. Installed:

| Pilot file | SHA-256 |
| --- | --- |
| binary | `a4e3ff614524ffbd1cc3086ab31ef87d205ecdd994013cc7dff03cc484326474` |
| generated launcher | `8b02cb4feac2e0aff25ae2f816efbd001deb0091f37c44ccce750b41052559a7` |
| sourced hook | `54446304f69c90a7cdeea405f4d2bc34f11cfb00c60fb64972c570675df540a2` |
| four-run marker | `8ed3b5bf8c5145901a58e16ed8c038517b9a307911a67aec6dc6694500c03171` |

Installed bytes/hashes, executable bits, shell syntax and sync passed. Manager
verified known app data/libraries, favorites/history, runtime/helper, installed
return manifest and original runtime backup unchanged. No credential/config
inventory outside known app/integration files is read. Offline installation assumes
one host writer; separate checks do not make multi-file publication transactional
against concurrent external edits or sudden card removal. Marker-last publication,
owned-file failure rollback and verified originals protect the normal offline path.
Activation alone implies no measured startup/RAM result or device ownership acceptance;
the separately collected startup pilot is reported below.

### Four-run device collection pilot — collected 2026-10-03

The user reports all four pilot launches worked on hardware. All four archives
were collected read-only from the card into
`/tmp/better-favorites-m5-pilot-collected-20261003/`, with byte comparison
and a SHA-256 manifest for all twelve source files. Raw logs and personal metadata
remain outside Git. Each run contains one schema-1 `first_presented_frame` summary,
21 distinct phase records, matching launcher/binary PIDs, monotonic timestamp
ordering, binary/launcher exit 0, and completed private-request/directory cleanup.
No failed, incomplete, malformed or missing pilot runs were found. Success and
failure populations remain separate in the summarizer and regression fixtures.

Conditions: mounted Onion v4.3.1-1, ARMv7 Linux 4.9.84, active theme
`mini.os by architeg`, Automatic return ON, 70 favorites / 18,935 bytes.
The binary, favorites and browser-preference hashes match across all four runs;
the binary matches the activated pilot hash above. Exact device model and
independently recorded boot/settle conditions were not supplied. The first
post-boot run is separated from three consecutive warm runs using pilot order,
user confirmation and kernel uptime, not the device's incorrect wall clock.

**Bias:** the installed pilot hook hashed executable, favorites and preferences
before its launch timestamp. This reads/warm-caches those files and excludes that
hashing time from the reported launch interval. Run 1 is a post-boot, hash-warmed
pilot, **not an unbiased cold-start baseline**. Logging, other preparation reads,
SD/cache conditions and `/proc/uptime` quantization also affect observations.
First presentation is an SDL-call proxy, not measured physical display latency.

| Run | Condition | Main entry → first presentation (ms) | Recorded launch → main entry (ms) | Recorded launch → presentation (ms) | Outcome |
| --- | --- | ---: | ---: | ---: | --- |
| 1 | post-boot (hash-warmed) | 505.361 | 830 | 1330 | Successful; exits 0 |
| 2 | warm | 277.706 | 40 | 320 | Successful; exits 0 |
| 3 | warm | 262.359 | 50 | 310 | Successful; exits 0 |
| 4 | warm | 267.590 | 50 | 310 | Successful; exits 0 |

Warm main-entry-to-presentation median: **267.590 ms**, range 262.359–277.706 ms
(n=3). Recorded launch-to-presentation median: 310 ms, range 310–320 ms.
Do not attribute the launch-to-main gap exclusively to dynamic linking: it includes
launcher preparation, process creation/loading and scheduling, without subphase probes.

| Inclusive startup phase | Post-boot pilot (ms) | Warm median (ms) | Warm range (ms; n=3) |
| --- | ---: | ---: | --- |
| `audio.mixer_open` | 0.811 | 0.637 | 0.599–0.648 |
| `audio.total` | 75.711 | 74.092 | 74.002–75.022 |
| `audio.volume_config` | 0.268 | 0.268 | 0.265–0.269 |
| `audio.wav_decode` | 73.633 | 72.937 | 72.784–73.769 |
| `browser.restore` | 1.516 | 0.222 | 0.206–0.256 |
| `display.setup` | 1.767 | 1.061 | 0.979–1.071 |
| `emu.config_read_parse` | 19.445 | 7.450 | 7.267–7.571 |
| `favorites.group_rows` | 3.536 | 1.357 | 1.264–1.387 |
| `favorites.parse_inclusive` | 28.600 | 10.849 | 10.707–11.191 |
| `favorites.read` | 0.457 | 0.218 | 0.214–0.224 |
| `font.open` | 11.062 | 8.953 | 8.886–9.366 |
| `font.validation` | 4.614 | 2.547 | 2.410–2.651 |
| `frame.first_render_and_events` | 118.522 | 92.257 | 88.317–102.319 |
| `image.decode` | 77.057 | 46.921 | 44.640–48.388 |
| `menu.resources` | 47.997 | 33.724 | 33.633–33.737 |
| `sdl.image_ttf_init` | 90.636 | 0.543 | 0.505–0.551 |
| `sdl.init_video_audio_events` | 73.508 | 22.029 | 21.780–23.299 |
| `settings.browser` | 0.178 | 0.083 | 0.077–0.083 |
| `settings.return` | 0.281 | 0.130 | 0.125–0.155 |
| `theme.config_read_parse` | 6.948 | 0.989 | 0.981–1.051 |
| `theme.total` | 18.975 | 2.756 | 2.675–2.857 |

Phase timings overlap; do not add them. Per run: 3 theme config reads / 3 paths,
70 emulator config reads / 3 paths, 17 font opens / 1 path, 28 image decode attempts /
26 paths. Attempts include missing candidates and probes, not just successful loads.
These are observations for later investigation, not an optimization decision.

**Warnings, separate from failed runs:** every log contains 18 libstdc++
“no version information available” warnings and 14 missing profile-theme image
candidate messages. These did not prevent first presentation or audio initialization;
the user confirmed usable behavior. Missing override candidates exercise existing
resource fallbacks. No change to libraries or theme configuration was made.

**Collection worked; scope remains limited:** these four normal app exits validate
startup archive collection and user-reported usability. They establish no RSS/PSS,
ON/OFF comparison, gameplay process absence, A/MENU handoff or return-ownership
acceptance for this profiling build. M5 remains incomplete. Accepted M4 backups
are retained unchanged.

**Local correction for future measurements:** the source hook now hashes only in
`bf_profile_finish`, after binary exit and launcher cleanup. Metadata explicitly
labels hashes as post-run observations; they are not immutable pre-run snapshots.
A PATH-stub fixture rejects any hash before binary execution and checks all three
hashes after finish, plus zero hashing once the four-run limit is reached.
Other prelaunch theme/settings/size reads remain disclosed preparation costs.
At pilot collection the corrected hook was not deployed. The follow-up fresh
session activation below replaces it; the old four archives remain immutable.
Do not delete/reuse those evidence directories.

### Corrected session and no-Wi-Fi live collection — prepared 2026-10-03

Operator confirmed power off/card mounted. Fresh session
`20261003-corrected-01` is activated at
`App/BetterFavoritesTest/.profiling-results/session-20261003-corrected-01/`;
its four initially empty slots are allocated atomically as `run-0001`…`run-0004`.
No old slot is reused. All twelve pilot files and original accepted M4
binary/launcher remain byte-identical; the old installed-hash manifest is also
archived in the fresh session backup. The active manifest now records the
corrected hook/marker hashes, retaining its accepted M4 original hashes.

Host verified previous-activation backup:
`/Users/valeriybagrintsev/IT Projects/miyoo-better-favorites-backups/20261003-022649-m5-corrected-session/`.
Card verified previous-activation backup:
`App/BetterFavoritesTest/.profiling-sessions/20261003-corrected-01/backup/`.
The original `App/BetterFavoritesTest/.profiling-backup/` M4 binary/launcher
remain the accepted rollback source. No runtime/helper/return-manifest edit.

| File | Session SHA-256 |
| --- | --- |
| `better-favorites` | `a4e3ff614524ffbd1cc3086ab31ef87d205ecdd994013cc7dff03cc484326474` |
| `launch.sh` | `8b02cb4feac2e0aff25ae2f816efbd001deb0091f37c44ccce750b41052559a7` |
| `profile-device-launch.sh` | `d2e89a4f272f124d37f61db34df99013385a51db3a0b4108a41b672cf19d564f` |
| `profile.enabled` | `39ff1621df3c1cae6b7f146c5fe2034cb9ccba61e0e14adf342499421cd33a6c` |
| `profile-memory-session.sh` | `bf30cfb2fbdcae56b4dad7cfd88534aa4cab5ac425819616af29b7b94f67a149` |
| `sample-device-memory.sh` | `9f4b7ec3160bb497e4cb3451b267beaad1a870547852b9873fa232ce569e1d71` |

Changed card files: corrected sourced hook, profiling marker, profiling activation
manifest; added finite observer and identity-checked snapshot scripts. Binary and
diagnostic launcher are unchanged. Copies/modes, syntax, protected data/integration
hashes, pilot/rollback preservation and sync passed. Apps config remains byte-identical:
`launch.sh` continues to receive the exact command accepted by `bf_return_is_app`:

```text
cd /mnt/SDCARD/App/BetterFavoritesTest; chmod a+x ./launch.sh; LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so   ./launch.sh
```

Hashing is now after binary exit/launcher cleanup, outside timed startup preparation.
Hashes describe post-run files; metadata/marker/theme/settings/size/stat reads,
process creation, profile scopes, log output and SD/cache state still perturb the
measurement. Reading favorites for its size can warm corpus data, so even this
first post-boot sample is not a fully unbiased cold-I/O experiment. The same
binary, theme, corpus, settings and restored selection are required. No cache
dropping, service changes or memory observer during timing runs.

#### Operator steps: corrected timing pilot first

1. Record device model, battery/power, theme, grouping/sorting/prefix settings and
   boot/settle interval. Keep the current Automatic return ON and the same
   favorites/selection. Insert card, boot normally; wait 30 seconds.
2. Apps → BetterFavoritesTest, wait five seconds, Y → B without changing settings,
   then B to exit. This is the first post-boot launch. Repeat three times without
   reboot, keeping the selection and procedure identical. Confirm input/audio.
   No A/MENU gameplay or memory collector during these four runs.
3. Power off and mount the card. Codex copies/hashes archives read-only, validates
   outcomes/hash timing/PIDs/cleanup, separates first run from three warm runs,
   and reports failures independently. Do not delete evidence or reset slots.
   At activation no corrected results existed; collected results are reported below.
   Booting/controls are operator actions.

#### Verified available tools and remaining access limits

This device has no usable Wi-Fi per the operator. Mounted source confirms
`App/Terminal/launch.sh` starts `.tmp_update/bin/st`; the installed networking
script implements Dropbear and Telnet, but those are not currently usable access.
Runtime kills network services during gameplay unless `.keepServicesAlive` exists
(`runtime.sh` game-launch branch). Do not change that flag/network policy.
No remote `/proc` access can be claimed from a mounted, powered-off card.

A finite `profile-memory-session.sh` observer, started from Terminal using
`nohup`, waits 40 seconds and takes three snapshots two seconds apart, then exits.
It samples exactly one `runtime.sh` and one exact browser executable or
`/mnt/SDCARD/RetroArch/retroarch` (use one known RetroArch game for the comparison).
Ancestry must connect target to runtime. Missing/ambiguous identities are recorded
and refused; no first-PID guess. Both endpoints' starttimes invalidate exiting/reused
PID snapshots. Raw inventories survive refusal. Non-RetroArch games or a different
runtime comm need an evidence-based target adjustment, not arbitrary selection.

**Device capability pilot:** verify `command -v nohup` in Terminal first. Terminal
command entry, nohup availability, survival after Terminal exit and actual process
names need device validation; host fixtures cannot establish these. If unavailable
or ambiguous, retain logs and stop comparisons. No permanent observer is installed
or automatically started. The two copied scripts are inert until explicitly run.

Run each condition separately, with a new session label; never reuse output paths:

```sh
a=/mnt/SDCARD/App/BetterFavoritesTest
command -v nohup
nohup sh "$a/profile-memory-session.sh" live01 idle > "$a/live01-idle.log" 2>&1 < /dev/null &
exit
```

Exit Terminal normally, open BetterFavoritesTest within 40 seconds and stay idle
for approximately a minute. After the first idle collector is validated, repeat
with **new** log names and these condition arguments:

| Condition | Device action during delay / sampling |
| --- | --- |
| `idle` | Browser idle, same position, no further input during samples |
| `exercised` | Scroll both directions; open/close Settings and Help, then hold a specified menu open. Record which screen and approximate duration |
| `game-off` | Set Automatic return OFF before starting collector; launch the agreed game from Better Favorites, hold a fixed save/scene |
| `game-on` | Set Automatic return ON before starting collector; launch exactly the same game/core/save/scene, with the same wait interval |

Restore the operator's original preference after comparisons. Timing runs are
already capped at four; later launches create no startup phase records. Do not
clear the cap for live-memory runs. These are short measurement observers, not
application/helper processes retained by the return feature. Observer shell,
`sleep`, `cat`, `awk` and SD writes perturb system memory/CPU/cache; their PIDs are
recorded, so report this overhead and avoid summing shared RSS. RSS/PSS are
per-target, with missing PSS explicitly unavailable. Service conditions must match.
Three within-session snapshots are a collection pilot, not independent repeated
boots or a sufficient causal ON/OFF baseline.

Inspect each gameplay inventory for app executable/cwd, known launcher ancestry
and return-helper processes. The observer moves cwd to `/`, so it does not count
as an app-context launcher. No app binary or shell in the app cwd should remain
during settled gameplay. Reconcile exact PIDs/starttimes and parent chains with
launch logs; an inventory's absence is evidence only at that sampling instant.
The sourced return helper lives in the existing runtime shell, not a separate
resident process. Do not mistake the explicitly launched measurement observer
for a return-feature helper, or claim whole-session absence from three snapshots.

#### Separate A/MENU/return smoke

After timing, without observers: OFF → A launch → MENU opens GameSwitcher → A
resumes → B/START gives ordinary Onion return. ON → A launch → MENU → A resume
then B/START → Better Favorites with restored browser position. Also browser
MENU → existing GameSwitcher (no selected-favorite registration); ON B/START
reopens, OFF ordinary return. Test switching to an existing other history game
under ON retains session ownership. Empty history must exit without a loop.
B from Better Favorites ends normally. Record the tested path/results separately
from timing/memory, and restore the previous Automatic return value. These
smokes use normal Onion history/launch behavior; collectors never edit history.

Codex can install/verify offline, collect mounted result directories and compare
valid samples. The operator must boot, enter Terminal commands, select scenes,
operate controls and power off/remount. First validate the idle observer before
requesting the remaining live matrix. Corrected startup samples are reported below; live RAM samples remain pending.

#### Reversible controls (offline, powered off only)

```sh
# Restore this session's previous hook/marker/hash manifest; retain all evidence.
python3 tools/manage-profiling.py restore-session --session-id 20261003-corrected-01 --card /Volumes/MIYOO --powered-off
# Or restore accepted M4 binary/launcher (original verified backup retained).
python3 tools/manage-profiling.py rollback --card /Volumes/MIYOO --powered-off
# Disable instrumentation without deleting archives:
python3 tools/manage-profiling.py disable --card /Volumes/MIYOO --powered-off
```

Session restore and M4 rollback leave the inert memory scripts and all collected
evidence present; no cleanup requested. Fresh activation refuses existing session
IDs and validates current hashes, originals, backups and unchanged Apps launcher.
Owned-file rollback is covered by injected publication failures. Multi-file
publication assumes a powered-off card and one host writer; it is not transactional
against arbitrary concurrent external edits or sudden card removal.

#### Optimization decision gate

Do not optimize from the four hash-warmed pilot logs. After the corrected sample
and live collector are validated, rank candidates with phase median/range,
call/unique-path evidence, a conservative achievable saving, retained-memory
evidence and behavior risk. Repeated Emu reads (70 / 3 paths), font probes/reopens
and duplicate resource decoding are candidates for measurement; required resources
and exact-record state must not be removed merely because repeated. Audio's
measured inclusive cost is not permission to alter the verified OSS/libpadsp path.
Report correlations as correlations; ON/OFF runtime differences need repeated
matched runs. No benefit ranking or implementation is justified before those
results, and no optimization/cosmetic cleanup has been performed.

### Corrected session collection — 2026-10-03

Collected 2026-10-03 (Europe/Moscow report date), read-only from mounted card.
Three archives present and byte-verified: one first post-boot and two warm.
**run-0004 absent:** the planned third warm run was not collected; it is missing
evidence, not a demonstrated startup failure. User reports no issues.
All three present runs validate schema, summary/phase completeness, PID/PPID,
uptime ordering, post-exit hashes, binary/launcher exit 0 and private cleanup.
No failed/incomplete/malformed present runs. Raw logs remain unchanged on card
and in this host directory with SHA-256 manifest. Earlier pilot files are also
verified unchanged on both card and host.

Binary/corpus/preferences hashes, theme, kernel, corpus size and Automatic return
ON match the earlier pilot. Exact device model and independent settle-condition
record remain unavailable. Device wall-clock dates are wrong; use kernel uptime.

| Metric | Earlier post-boot (n=1) | Corrected post-boot (n=1) | Earlier warm median [range] (n=3) | Corrected warm median [range] (n=2) |
| --- | ---: | ---: | --- | --- |
| App entry → first presentation (ms) | 505.361 | 505.596 | 267.590 [262.359–277.706] | 271.043 [265.012–277.075] |
| Recorded launch → main entry (ms) | 830.000 | 860.000 | 50.000 [40.000–50.000] | 50.000 [50.000–50.000] |
| Recorded launch → presentation (ms) | 1330.000 | 1370.000 | 310.000 [310.000–320.000] | 325.000 [320.000–330.000] |

## Inclusive phases (ms; overlapping, do not sum)

| Phase | Earlier post-boot | Corrected post-boot | Earlier warm median [range], n=3 | Corrected warm median [range], n=2 |
| --- | ---: | ---: | --- | --- |
| audio.mixer_open | 0.811 | 0.809 | 0.637 [0.599–0.648] | 0.643 [0.641–0.646] |
| audio.total | 75.711 | 85.774 | 74.092 [74.002–75.022] | 78.552 [76.907–80.197] |
| audio.volume_config | 0.268 | 0.264 | 0.268 [0.265–0.269] | 0.269 [0.267–0.271] |
| audio.wav_decode | 73.633 | 83.638 | 72.937 [72.784–73.769] | 77.344 [75.696–78.991] |
| browser.restore | 1.516 | 1.568 | 0.222 [0.206–0.256] | 0.249 [0.248–0.251] |
| display.setup | 1.767 | 1.168 | 1.061 [0.979–1.071] | 1.090 [1.087–1.092] |
| emu.config_read_parse | 19.445 | 19.948 | 7.450 [7.267–7.571] | 7.944 [7.442–8.445] |
| favorites.group_rows | 3.536 | 3.620 | 1.357 [1.264–1.387] | 1.591 [1.577–1.605] |
| favorites.parse_inclusive | 28.600 | 29.026 | 10.849 [10.707–11.191] | 11.566 [10.900–12.231] |
| favorites.read | 0.457 | 0.480 | 0.218 [0.214–0.224] | 0.221 [0.216–0.226] |
| font.open | 11.062 | 9.243 | 8.953 [8.886–9.366] | 9.156 [9.022–9.290] |
| font.validation | 4.614 | 2.919 | 2.547 [2.410–2.651] | 2.641 [2.561–2.720] |
| frame.first_render_and_events | 118.522 | 136.727 | 92.257 [88.317–102.319] | 90.346 [87.828–92.863] |
| image.decode | 77.057 | 69.999 | 46.921 [44.640–48.388] | 44.952 [44.637–45.267] |
| menu.resources | 47.997 | 50.757 | 33.724 [33.633–33.737] | 33.619 [33.615–33.623] |
| sdl.image_ttf_init | 90.636 | 71.460 | 0.543 [0.505–0.551] | 0.557 [0.539–0.576] |
| sdl.init_video_audio_events | 73.508 | 72.553 | 22.029 [21.780–23.299] | 22.477 [21.799–23.156] |
| settings.browser | 0.178 | 0.941 | 0.083 [0.077–0.083] | 0.082 [0.080–0.084] |
| settings.return | 0.281 | 0.275 | 0.130 [0.125–0.155] | 0.127 [0.121–0.133] |
| theme.config_read_parse | 6.948 | 7.650 | 0.989 [0.981–1.051] | 0.975 [0.944–1.006] |
| theme.total | 18.975 | 20.006 | 2.756 [2.675–2.857] | 2.758 [2.662–2.853] |

All present runs have 3 theme config reads / 3 paths; 70 emulator config reads /
3 paths; 17 font opens / 1 path; 28 image attempts / 26 paths. Attempts include
missing candidates and probes, not only successful decoding. All three have
18 libstdc++ version warnings and 14 missing profile-theme image candidates,
separate from failures; audio initialization succeeded.

The hook no longer hashes the executable/corpus/preferences before launch.
Post-exit hashes are observations, not immutable pre-launch snapshots, and can
warm caches for the subsequent deliberately warm runs. Remaining preparation
reads (including favorites size), metadata/log writes, scopes/instrumentation,
process creation, linking, SD/cache/scheduling variability and uptime quantization
remain. Main-to-frame is the SDL first-presentation proxy, not physical display
latency or externally measured input readiness. Pre-timestamp preparation costs
are excluded from recorded-launch timing; linking/loading are not isolated.

The near-identical post-boot totals and overlapping warm ranges establish no
performance improvement/regression or causal effect of moving hashes. Small
samples and one missing warm run prohibit strong comparative claims. Collection
worked for all available runs, but the intended four-run corrected set is incomplete.
Do not append a post-reboot launch and label it another warm run from this boot.
For a complete four-run set use a fresh session and preserve this one.

No live memory samples, OFF/ON memory comparison, gameplay process absence or
profiling-build A/MENU/return smoke acceptance are established by these logs.
No optimization or publication.

Host preserved raw collection, SHA manifest, validation and separate summaries:
`/tmp/better-favorites-m5-corrected-01-collected-20261003-01dsfhps/`. Startup profiling was subsequently disabled with operator
confirmation of power off/card mounted. Only the owned marker was removed;
all archives, accepted M4 originals, binary/launcher and collector scripts verified
unchanged. Syntax and sync passed. Installed scripts match reviewed source.
No additional timing launch should be silently treated as a warm run after reboot.

#### Exact idle-browser Terminal pilot

No Wi-Fi required. Use a fresh log/sample name. In device Terminal:

```sh
a=/mnt/SDCARD/App/BetterFavoritesTest
if command -v nohup >/dev/null 2>&1 &&
   [ ! -e "$a/idle-20261003-01.log" ] &&
   [ ! -e "$a/.profiling-memory/idle-20261003-01-idle" ]; then
    nohup sh "$a/profile-memory-session.sh" idle-20261003-01 idle > "$a/idle-20261003-01.log" 2>&1 < /dev/null &
    echo "Started. Exit Terminal and open Better Favorites."
else
    echo "Not started: nohup unavailable or trial name already used."
fi
```

Only if `Started` appears, enter `exit`, then immediately open BetterFavoritesTest
via Apps. Within 40 seconds it must be at the idle browser; leave selection,
settings and menus untouched. Stay idle 90 seconds as a practical allowance.
The collector waits 40 seconds, then takes three paired runtime/browser snapshots
with two intervening 2-second waits: minimum 44 seconds **plus** inventory/read/write
time, not a guaranteed 44-second deadline. It is finite and does not auto-repeat.

Return with B and reopen Terminal to check:

```sh
a=/mnt/SDCARD/App/BetterFavoritesTest
tail -n 1 "$a/idle-20261003-01.log"
grep -H '^valid=' "$a/.profiling-memory/idle-20261003-01-idle/"*/validity.txt
```

Completion log must say:
`Collector complete: /mnt/SDCARD/App/BetterFavoritesTest/.profiling-memory/idle-20261003-01-idle (failed=0)`.
Six validity files (three runtime and three browser) must each report `valid=1`.
A missing completion line, `failed=1`, missing/invalid samples, or refused PID
identity is method failure/partial evidence; preserve everything for inspection.
Do not retry with the same name or truncate its log. If `nohup` is unavailable,
stop and report that instead of assuming background survival.

The log is `App/BetterFavoritesTest/idle-20261003-01.log`; raw samples and inventories
are under `App/BetterFavoritesTest/.profiling-memory/idle-20261003-01-idle/`.
Then power off/remount for Codex to copy, verify validity/starttimes and report
RSS/PSS or unsupported metrics. Opening Terminal before completion can invalidate
this trial. Sampling overhead and whole-device/shared-RSS limitations remain.
This idle pilot alone will not prove gameplay process absence or OFF/ON memory cost.

### Collected idle-memory evidence and reviewed cache deployment — 2026-10-03

The finite idle observer completed with `failed=0`. All six snapshot directories
validate unchanged endpoint PID starttimes (runtime PID 780, browser PID 3514).
Three same-boot idle samples per role, Automatic return ON, **pre-cache profiling
binary** `a4e3ff614524ffbd1cc3086ab31ef87d205ecdd994013cc7dff03cc484326474`:

| Role | smaps RSS median/range (kB) | smaps PSS median/range (kB) | Private clean (kB) | Private dirty (kB) |
| --- | --- | --- | ---: | ---: |
| Browser | 19,308 / 19,308–19,308 | 17,449 / 17,449–17,450 | 10,532 | 6,464 |
| Runtime | 1,808 / 1,808–1,808 | 357 / 357–357 | 0 | 212 |

`smaps_rollup` was unavailable; the sampler aggregated `smaps`. Kernel status
VmRSS is 19,240 kB for browser and 1,808 kB for runtime; VmSize 58,660 and 3,144 kB.
Status and smaps readings are separate observations, not an atomic snapshot; do
not silently substitute VmRSS for summed smaps RSS. Shared mappings make RSS
non-additive. These short, correlated idle samples include observer/SD-write
perturbation. They do not isolate return-integration cost, show gameplay process
absence, cover post-scroll/menus, or establish an OFF/ON comparison. No cache
memory improvement is measured. Exact device model was not recorded.

Preserved byte-verified idle raw logs, SHA manifest and per-role summaries:
`/tmp/better-favorites-m5-idle-collected-20261003-0jrtdj_m/`. Originals remain on the card. All prior startup
archives and accepted M4 rollback files remain unchanged. Startup evidence stays
separate: original pilot n=1 post-boot + n=3 warm; corrected n=1 post-boot + n=2
warm, with run-0004 absent. Post-boot totals 505.361/505.596 ms; warm medians
267.590/271.043 ms with overlapping ranges. Both sets use the pre-cache binary;
there is no after-optimization timing sample.

The reviewed per-parse cache binary was deployed with verified backup:
`/Users/valeriybagrintsev/IT Projects/miyoo-better-favorites-backups/20261003-033812-m5-cache-deployment/`.
The cache implementation is published separately from profiling.
Profiling stays disabled; no new session, Terminal commands or observers are
requested. Only binary plus its profiling ownership-manifest hash were published.
Hardware cache acceptance awaits one ordinary browsing/game launch-return check.
No optimization beyond this isolated cache, cosmetic cleanup, commit or push.

## Final retirement and retained evidence

User requested closure without further device tests. Reusable opt-in tools,
tests and docs remain in Git. `tools/retire-profiling.py` archives/verifies owned
files first, refuses foreign files/hash mismatches, atomically restores the normal
launcher, then removes only individually reverified owned files. It keeps the
binary and protects saved data and permanent Onion return integration.

Verified host archive: `/Users/valeriybagrintsev/IT Projects/miyoo-better-favorites-backups/20261003-034734-m5-retired-card-archive/`.
Its `archive-manifest.json` records 106 archived source files and production
launcher bytes. `retirement-verification.json` records the protected hashes and
104 removed temporary files (1,109,348 logical bytes). Removed:

- `profile-device-launch.sh`, `profile-memory-session.sh`, `sample-device-memory.sh`;
  the activation marker was already absent.
- `.profiling-results/` (four original pilot archives, three corrected archives),
  `.profiling-memory/` (six idle samples and inventories), and the idle collector log.
- `.profiling-backup/` and `.profiling-sessions/` temporary card rollback copies.
  Their exact binary/launcher/manifest bytes remain in the verified host archive
  and previous host backups. Permanent Onion return backup/manifest is untouched.

Restored `launch.sh` SHA-256:
`b301be6c0ba37f3159774a306d16b6dd45fce976a4a30d86f0b85a09d3f66a5f`.
Retained cache binary SHA-256:
`2b31a22731711caec94e4bb000a379ab431ef4ec85ba4bb017a41b3ac6cf07ec`.
Copies/hashes, executable permissions, ARM format, shell syntax and sync verified.
Preferences/browser state, libraries, favorites/history, theme configuration,
ordinary app logs and permanent runtime/helper/return manifest/backup are retained.
No card ejection. User may review the normal deployed build without profiling;
this pass requests no more device checks and claims no cache-build hardware acceptance.

M5 is closed as this documented measurement/isolated-change pass, not as complete
performance certification. Unknown gameplay/OFF-ON/process-absence behavior stays
explicitly deferred. M6 is unblocked; no Home integration has been implemented.
