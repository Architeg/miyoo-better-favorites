# Contributing

Bug reports, feature ideas and device/theme test reports are welcome. Use the issue
templates, include exact app/Onion/model/firmware where known, and attach reviewed
host-exported diagnostics if helpful. No Terminal access on the Miyoo is required.
MY354 is a model/platform code; do not infer hardware revision.

## Work on the code

Read the [technical guide](docs/development.md), [roadmap](docs/roadmap.md) and
[verification status](docs/development-status.md). Preserve the wrapper boundary:
Onion owns game execution, cores, save/resume, history and GameSwitcher. Do not
replace working audio or global theme settings to fix an unrelated issue.

Prepare the pinned dependencies with `sh scripts/fetch-deps.sh`, then follow
[build/package provenance](docs/release/build.md). Docker builds the ARM app;
Go 1.24+ builds offline host tools with the standard library only. Development
Python is for fixtures/package generation, not a user installation dependency.

Run `sh tests/run-local-checks.sh`; use `tests/mainui-home/run-arm-checks.sh` only
inside the isolated Docker fixture container. Run `go test ./...` in
`tools/release-installer`. Exact binary tests require **private read-only** audited
fixtures supplied through `BF_FIXTURE_REPO`; do not commit vendor binaries, card
backups, game data, logs, settings, credentials or generated outputs.

Qualify the **actual extracted ZIP** with `BF_RELEASE_PACKAGE` and the native host
installer tests, then the release checklist. Native Windows execution and SD
reader testing are separate from cross-compilation/Linux emulation. New devices,
versions or hashes need audited source/ABI/layout and appropriate device evidence;
never bypass an allowlist to claim compatibility.

Keep PRs scoped. Separate cosmetic cleanup from measured performance work. Include
changes, risks, relevant checks and missing hardware evidence. Update roadmap and
status on milestone or hardware acceptance changes. GPL-3.0-or-later applies to
project sources; preserve separate dependency notices/source obligations.
