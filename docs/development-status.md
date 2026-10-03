# Development status

Updated 2026-10-04. [Authoritative roadmap](roadmap.md); update both documents when
milestones or hardware acceptance change.

## Accepted checkpoint

M6 committed as `60b34b620f97d1819ae48736db8896ffe767ec57` — **Add hardware-accepted
optional Home Favorites redirect**. Mini Plus **MY354**, firmware **202306282128**,
Onion **v4.3.1-1**; hardware revision **unknown**. MY354 is the model/platform.
Designed for Mini and Mini Plus; hardware tested on Mini Plus. Mini/community
combinations remain unverified. [Accepted hashes and log evidence](m6-acceptance.md).

User passed OFF stock Favorites, ON Better Favorites/B Home without loop twice,
Apps/X/Y and A/MENU/GameSwitcher/Automatic return. The actually executed MainUI
variant is 354-clean. Other variant ARM fixtures are not hardware acceptance.
Accepted card binary `d637643b0be987ab68a7bb9ba6869f09b6e1e58e2e7d0919088bbb1cd54f19dc`,
launcher `4173bca55adaab4f491da2c4a07856886b783f6a408d897135a6858dc4c26419`.
Collected logs/backups remain; tracing is OFF. This release preparation does not
write the card, redeploy or retrospectively accept later candidate bytes.

M1–M4 browser/menu/settings/scrolling/paging/resource work is device accepted.
M5 is a closed pass: pilot post-boot 505.361 ms / warm median267.590(n=3);
corrected post-boot505.596 / warm271.043(n=2; fourth absent). Hash/preparation and
instrumentation bias prevent unbiased cold/gain claims. Idle ON browser smaps
RSS19,308kB/PSS17,449median; runtime RSS1,808/PSS357. Gameplay, ON/OFF, post-scroll/
menu and gameplay-process absence are **deferred**. Host cache reads70→3; no measured
device speedup. Later M6 acceptance covers the actual cache-containing deployed bytes.
[Measurements](m5-profiling.md), [cache](m5-emulator-label-cache.md),
[historical detailed status](archive/development-status-pre-release.md).

## Release preparation

M7/M8 prepare **v1.0.0-rc.2**, not stable. Complete default uninstall now restores
and verifies originals, archives recovery on the computer, and removes owned app/data.
Optional remove-integrations retains app/data. The native offline installer is
implemented; end users need no Python/WSL/developer tools.

Windows7 SP1 x64 and Windows10 x64 installation/use/uninstallation and stock Onion
boot passed per user. The diagnostics match the earlier RC2 complete payload;
current card originals and integration removal are byte-verified. Exact later
host ZIP/dispatcher hashes are not present in that export. Other native versions,
architectures/readers and fault outcomes remain separate gates.
[Windows analysis](release/windows-acceptance.md).

RC1 Mac packaged device checks passed on MacBook Air M1 / Ventura13.7.8; actual
Mac ZIP fixture roundtrips and real Rosetta selection/status also passed. Linux
has Docker fixtures, not physical reader acceptance.
[Mac evidence](release/rc1-mac-acceptance.md), [targets/minima](release/host-dispatch.md).
Go1.26.2 remains normal; Go1.20.14 is isolated for legacy Windows/dispatch.

Basic current/previous logs are bounded to128KiB total; Export diagnostics is a
host action, detailed tracing remains OFF. Short-lived logging modes leave no
resident gameplay helper. No new RAM or timing measurement is claimed.

GPL-3.0-or-later is user-approved. Dependency/source companions and hash inventory
exist, but exact prebuilt correspondence/transitive license audit remains a public
redistribution gate. Candidate is private review material until cleared.

## Exact next step

Review documentation and coordinate the next exact package/checksums, then qualify
its identified dispatcher/ZIP and remaining native targets using [release gates](release/rc.2.md). Complete dependency audit
before public distribution. No shortcut, profiling session, broad cleanup or new
feature is authorized by this release pass. All global shortcuts are deferred
beyond v1.0; existing Apps/Home and MainUI X/Y remain.

[Quick start](install.md) · [Recovery](uninstall.md) · [Compatibility](compatibility.md)
· [Technical guide](development.md) · [Historical evidence](developer-index.md)

Host compatibility update: Windows7 through current releases, macOS Monterey+
Intel/Apple Silicon, Linux x64/ARM64; one automatic-dispatch entry per platform.
See [targets, minima, dispatch and pending native qualification](release/host-dispatch.md).
Legacy builds remain isolated; every host uses the same payload/safety implementation.

## RC2 source/tag preparation — 2026-10-04

The supplied focused documentation presentation patch is applied over the current
rewrite. Newer Windows/device evidence is preserved. Three static badges and the
exact supplied icon remain; dynamic badges reporting “repo not found” were removed.
Relative links/anchors and GitHub GFM output are checked; isolated headless layout
inspection supplements the unavailable app browser tool.

Offline wrappers/full-uninstall remain the implemented route. A separate
[bootstrap preparation](../tools/bootstrap/README.md) has isolated host fixtures
and modern/legacy cross-builds, not Windows online acceptance or working public
URLs. Uninstall requires an explicit matching release/recovery.

libpng/zlib embedded versions and original upstream notices/sources are recovered.
Custom SDL fresh linking fails on MI system/graphics symbols; original link inputs
and extension/SwiftShader prebuilt correspondence remain binary publication gates.
Prepare the annotated RC2 tag and **draft prerelease without assets**. Local final
packages must identify the exact committed tree and keep new-byte qualification
separate from previous accepted candidate bytes. No stable release or SD deployment.
