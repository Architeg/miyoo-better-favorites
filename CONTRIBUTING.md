<div align="center">
  <h1>Contributing to Better Favorites</h1>
  <p><em>Help improve the app, the instructions, or compatibility.</em></p>
</div>

<p align="center">
  <a href="README.md">About the app</a> ·
  <a href="#report-a-useful-bug">Report a bug</a> ·
  <a href="#get-the-source">Get started</a> ·
  <a href="#run-ordinary-checks-first">Run checks</a> ·
  <a href="docs/developer-index.md">Developer guides</a>
</p>

You do not need firmware binaries or a development environment to help.

| How to help | Where to start |
| --- | --- |
| 🐛 Report a bug | Describe what happened and how to reproduce it |
| 🎨 Test a theme or device | Share readability, missing-resource and control results |
| 📖 Improve the docs | Fix unclear steps, missing information or confusing errors |
| 💡 Suggest a feature | Explain the user need and existing Onion behavior |
| 🛠️ Contribute code | Read the [architecture](docs/architecture.md) and [development guide](docs/development.md) |

Use the [issue templates](https://github.com/Architeg/miyoo-better-favorites/issues/new/choose).
Check the [roadmap](docs/roadmap.md) before starting a substantial feature.

<a id="report-a-useful-bug"></a>
## Report a useful bug

Include:

1. App version and download ZIP SHA-256, when available.
2. Miyoo model, firmware and Onion version; hardware revision only if known.
3. Active theme; whether optional integrations are installed and enabled.
4. Steps, expected behavior, actual behavior and whether it repeats.
5. For installation: computer OS/version/architecture, shell, SD reader/filesystem,
   command/action and exact error. Note install/update/uninstall and post-uninstall boot separately.
6. A relevant photo or reviewed diagnostic export if useful.

MY354 identifies a model/platform, not a hardware revision. Unknown information is
better than a guess. A successful build is not a device test.

With the Miyoo powered off and card connected, open its `App/BetterFavorites` computer launcher and choose **[3] Export diagnostics**. The resulting computer archive path is printed; no Miyoo Terminal is needed. **Inspect before uploading.** Reports can include game filenames,
theme paths and preferences. Never upload ROMs, BIOS, saves, credentials, complete
card images or private recovery originals. [Diagnostic contents/privacy](docs/diagnostics.md).

<a id="get-the-source"></a>
## Get the source

```sh
git clone https://github.com/Architeg/miyoo-better-favorites.git
cd miyoo-better-favorites
```

<a id="developer-prerequisites"></a>
### What you need

| Work | Requirements |
| --- | --- |
| Ordinary browser/core checks | Git, POSIX shell, C++17 compiler, Python (3.11 recommended) |
| Installer tests | Go 1.26.2; standard library only, no external modules |
| Windows legacy installer qualification | Isolated official Go 1.20.14; keep normal Go/module unchanged |
| ARM app build | Docker and the existing pinned Miyoo toolchain image; prepared dependency headers |
| Optional SDL resource checks | Existing SDL2/freetype/libpng development tools; see detailed guide |

macOS contributors need developer command-line tools for host C++ checks; end users
do not. Linux needs a host C++ compiler. Native Windows supports Go tests directly;
POSIX C++/shell scripts need a suitable developer environment such as Git Bash or
Linux/WSL2. That is a contributor option, **not** the user installation route.
A container or cross-build does not qualify a native reader/filesystem.

<a id="run-ordinary-checks-first"></a>
## Run ordinary checks first

These do not require a card or proprietary firmware fixtures:

```sh
sh tests/run-local-checks.sh
(cd tools/release-installer && go test ./...)
(cd tools/host-dispatch && go test ./...)
(cd tools/bootstrap && go test ./...)
python3 tests/host_dispatch_test.py
python3 tests/online_install_test.py -v
```

The C++/Python suite exercises browser navigation/settings/state, removal, handoff,
return lifecycle and profiling utilities with temporary fixtures. Its optional
emulator-label test explicitly skips if the existing host libcjson bridge is absent.
Go tests skip exact vendor/package cases unless their fixture variables are supplied.
The dispatch suite simulates host probes; it does not run every target OS.

<a id="prepare-and-build-the-arm-app"></a>
## Prepare and build the ARM app

Dependency preparation downloads the pinned SDL Miyoo fork and extracts headers;
it writes only generated `third_party` directories. It is a developer network step.
The script prepares json-c, SDL_ttf, SDL_image and SDL_mixer headers from the pinned archives.

```sh
sh scripts/fetch-deps.sh
sh scripts/build.sh
```

`build/better-favorites` is the ARM output. The build also assembles the audited Home
adapter; no MainUI binary fixture is needed for compilation. The image digest and
version/source literals are set by the script. This produces a compiled executable,
not hardware acceptance or a fully reproducible public release.

**Packaging has an additional prerequisite:** the verified custom OSS SDL library
and dependency hashes. Fresh cloning does not reproduce that library today. Do not
silently substitute a different audio build. See [build/provenance](docs/release/build.md)
and the [dependency audit](docs/release/dependency-audit.md).

<a id="source-map"></a>
## Source map

| Location | Responsibility |
| --- | --- |
| `src/`, `include/` | Favorites parsing/model, navigation, theme/rendering/audio, menus, persistence and launch requests |
| `App/BetterFavorites/` | Onion app config, icon and outer handoff launcher |
| `integration/onion-return/` | Optional runtime/session return hooks |
| `integration/mainui-home/` | Optional exact-binary Home Favorites adapter and catalogue |
| `tools/release-installer/`, `packaging/windows-select.ps1` | Installation/restoration/full removal and built-in Windows host selection |
| `scripts/install-online.sh` | Optional Mac/Linux downloader; delegates card changes to the packaged installer |
| `packaging/`, `tools/package-release.py` | Entry scripts, package inventory, native builds and source companions |
| `tests/` | Host fixtures, lifecycle/navigation/storage tests and isolated ARM harnesses |
| `docs/` | User guides, architecture, roadmap and evidence |

<a id="integration-and-hardware-qualification"></a>
## Integration and hardware qualification

Ordinary contributions do not need private firmware. Exact MainUI patch output,
displaced instructions and runtime hashes use legally obtained, read-only audited
fixtures supplied through `BF_FIXTURE_REPO`. Actual extracted ZIP tests additionally
use `BF_RELEASE_PACKAGE`. Do not commit or redistribute those originals.

The main ARM harness refuses mounted cards and runs only in an isolated Docker
container. It requires generated exact-binary prototypes and QEMU; the complete
procedure is in [development](docs/development.md) and [Home integration](docs/archive/m6-home-integration.md).
Do not run it on the device or treat emulation as acceptance.

Release qualification tests the **actual package**: install, optional Home OFF/ON,
Apps/X/Y, game/GameSwitcher/return, complete uninstall, stock boot and reinstall.
Record package/tool/deployed hashes, device/host/theme and missing evidence. New
versions need an audited compatibility decision; never bypass an allowlist.

<a id="send-a-focused-pull-request"></a>
## Send a focused pull request

1. **Fork** this repository on GitHub.
2. Clone your fork and create a branch for the change.
3. Make the change, run the relevant checks and push your branch to your fork.
4. Open a pull request from that branch to **Architeg/miyoo-better-favorites → main**.
5. Explain the change and test results. A maintainer reviews it before merging;
   opening a pull request does not change the published app automatically.

For a larger feature, open an issue first so we can agree on scope.
[GitHub’s fork-and-pull-request guide](https://docs.github.com/en/get-started/exploring-projects-on-github/contributing-to-a-project).


- Explain the user problem, scope, behavior change and regression risk.
- List commands/results and distinguish simulation, host execution and hardware tests.
- Preserve Onion's game/core/save/history/GameSwitcher ownership and working audio.
- Keep geometry, theme resolution and exact-record identity intact unless the task explicitly changes them.
- Update guides, roadmap/status and acceptance evidence when behavior or verification changes.
- Exclude binaries, logs, personal preferences, backups, ROMs and firmware fixtures.
- Keep cleanup separate from measured performance work; avoid unrequested restructuring.

Project code is GPL-3.0-or-later. Preserve upstream notices and identify any new
dependency/source obligations. Documentation and test reports are valuable PRs too.

## Current qualification gaps

Physical Linux install/uninstall and reader tests are pending; Docker tests do not qualify them. RC7's recovery-backed Intel cross-computer receipt correction and Settings submenu/badge fix need a targeted check. Keep accepted earlier package evidence separate from new bytes. [Release engineering notes and checklist](docs/release/rc.7.md).

