# Development status

Updated 2026-10-03. [Authoritative roadmap](roadmap.md); update both documents when
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

M7/M8 now prepare **v1.0.0-rc.1**, not stable. Post-acceptance Home wording/status/
recovery and new release logging/package code have host/ARM checks only.
The native Go offline installer replaces Python/WSL as the proposed user route;
Windows cross-compilation is implemented but native Windows execution/SD-reader
qualification remains pending. The actual extracted ZIP installer roundtrip passes on native macOS arm64 and
Linux amd64 in the isolated existing Docker image; final ZIP hardware install/uninstall/reinstall is pending.

Basic current/previous logs are bounded to128KiB total; Export diagnostics is a
host action, detailed tracing remains OFF. Short-lived logging modes leave no
resident gameplay helper. No new RAM or timing measurement is claimed.

GPL-3.0-or-later is user-approved. Dependency/source companions and hash inventory
exist, but exact prebuilt correspondence/transitive license audit remains a public
redistribution gate. Candidate is private review material until cleared.

## Exact next step

Review the pinned candidate/checksums, then qualify native Windows and the **actual
ZIP** on device using [release gates](release/rc.1.md). Complete dependency audit
before public distribution. No shortcut, profiling session, broad cleanup or new
feature is authorized by this release pass. All global shortcuts are deferred
beyond v1.0; existing Apps/Home and MainUI X/Y remain.

[Quick start](install.md) · [Recovery](uninstall.md) · [Compatibility](compatibility.md)
· [Technical guide](development.md) · [Historical evidence](developer-index.md)
