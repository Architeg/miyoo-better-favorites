> **Historical engineering record — not current installation instructions.**
> Use the [installation guide](../install.md) and [compatibility matrix](../compatibility.md) for the released app.

# M6: exact-binary Home Favorites redirect evidence

> **Historical engineering evidence.** This page records development decisions and tests at the time. Use the [current installation guide](../install.md) and [recovery guide](../recovery.md) for released packages.

2026-10-03, based on `main` HEAD `6f846ce32f6ee8f773b59dfae8cc4d2c0cf56907`.
**Local/uncommitted; deployed implementation user hardware-accepted on Mini Plus
v4.3.1-1.** See [tested hashes and log coverage](m6-acceptance.md). Diagnostic logs
are archived, marker disabled and synced. New wording/status/recovery changes after
acceptance remain host-only/undeployed. The adapter bytes have not changed.
See [installation, diagnostics and offline recovery](m6-home-integration.md).

## Accepted scope and provenance

Only activation of the existing Home Favorites row is redirected. Its translation
ID, icons, layout, theme resources, Apps entry and X/Y assignments remain stock.
There is no separate tile. Replacement is independent of Automatic return and
fails closed to stock activation.

Mounted authority: Onion `v4.3.1-1`, `.tmp_update/bin/MainUI-*`. The earlier audit is
retained at `/tmp/better-favorites-m6-mainui-audit/`, including disassembly,
`verified-mappings.json` and the pinned community reference; it was read, never run.
Reference [patcher source](https://github.com/robcodedev/onionos-mainui-patcher/blob/e445c038cfb6f7d5aaf45fb6377fa2365eb8dd0c/onionos_mainui_patcher.py),
SHA-256 `0d84158d22240583e26c793d22825dea57b13b356bf4941b095ce6ce0bef3eec`.
The allowlist below is enforced on the **complete input file**, not just the hook
bytes. Other builds, including previously patched ones, are rejected.

| Exact input | Original SHA-256 |
| --- | --- |
| MainUI-283-clean | `6b01276a6292fd7061e0b2576322a52ada65b755562f97bf7656b174d475866f` |
| MainUI-283-expert | `6948b5310dda6513b9e8fa2519d90c5287fc1fd06b4f668a7f2f205406281d28` |
| MainUI-354-clean | `98c85f6c573bdeabd3762e8d9b596f354014e666cc873d0f758cbf3752620c94` |
| MainUI-354-expert | `3bd1fef7fd9bd215bb9e335b6be1101fdff510590ba0d9ca0a9707edc5d9718a` |

## Adapter and verified native path

Sources: `integration/mainui-home/adapter.cpp`, `hooks.S`, `adapter.ld` and the
**offline generator** `prototype.py`. The generator writes a fresh host directory
under repository `build/` or host `/tmp`; it does not install anything.

`bf_home_activate` requires row numeric title ID 1 and destination 2, navigation
stack top type 0, and that parent's child pointer equal to the activating grid.
The stack global is `0x17ff38`; row offsets are +64/+76 and Window offsets +8/+12.
A non-Home grid or different row returns immediately. Boot/state restoration does
not itself hit this activation hook.

Prototype activation requires the exact file contents `BetterFavoritesHome1\n1\n`
in `App/BetterFavoritesTest/home-entry.conf`. Missing, Off, oversized, unreadable or
malformed preference means stock. This file has **not** been created on the card.
The current four-key ASCII App config schema is checked (label/icon/launch/
description, strings, no duplicates/unknown keys, launch exactly `launch.sh`).
Unsupported escaped non-ASCII metadata also falls back. This conservative reader
is a prototype constraint, not a general Onion JSON parser. Required files must be
regular, readable and not symlinks; binary must have an ARM ELF header and execute
permission. These checks do not guarantee library availability or validate every
ELF segment. App availability after dispatch remains a device risk.

Native constructor `0x3984c` initializes a 16-byte stack `AppAction`, borrowing the
128-byte config; execute `0x18c54` calls native formatter `0x18560`. Only accessed
name/launch strings are constructed, at config offsets 0 and 72; type is 3, flags
120/124 zero. Regular destructor `0x398b4`, followed by both string destructors,
releases adapter-owned allocations. The deleting destructor is never used on the
stack object. Native formatting and its local-string cleanup remain original.

Fallback replays `mov r3,#0` and resumes `0x27864`, where stock Favorites window
construction remains unchanged. Committed dispatch sets r3=3 and reaches the
existing activation epilogue `0x27970`. Outer native result-3 handling at
`0x37294`–`0x373a0` preserves the nonnegative Home window and invokes state serializer
`0x1c544`; Home serializer `0x2edac` retains current position/page bounds. The
compiled tests execute these serializers and preserve Home's child/stack pointers.
Full native event-loop execution, stock Favorites construction on fallback and
state restoration on reopening are not emulated; the outer-loop analysis is static.

## Publication transaction (three handoff hooks)

1. Activation opens the scoped transaction, capturing the Linux TID.
2. Native formatter calls its existing writer. Only caller LR `0x188d0` **and** the
   owning TID redirect to `bf_stage_native_command`. Other writer calls replay the
   original prologue, including calls during this transaction.
3. The adapter accepts only the canonical existing App command, allowing trailing
   spaces/tabs/newlines. It creates `/tmp/.better-favorites-home.<pid>.<serial>`
   exclusively, without following symlinks, checks chmod 0700, complete writes,
   EINTR/short writes, fsync and close. No pending command exists yet.
4. The hook immediately after the native writer calls `bf_publish_gate` **before**
   native App recent registration. It checks shutdown and atomically hard-links
   the completed private inode to `/tmp/cmd_to_run.sh`. This is same-filesystem
   publication in device `/tmp`, not an SD hard link or a cross-filesystem rename.
   `link` refuses an existing target; no pending file is deleted or replaced.
5. Link failure skips native recent registration, returns native failure through
   its string-cleanup path, removes only private staging, then takes stock Favorites.
   Success records commitment and unlinks the private name. Native App recent
   service `0x1204c4` then runs (type 3 App, not game registration). If it throws
   after publication, the adapter retains the command and returns 3; falling back
   at that point could create two conflicting actions.
6. Stock runtime waits for MainUI to exit, then moves pending to its active SD
   command and executes it. See mounted `launch_main_ui` / `launch_game` and
   [matching runtime](https://github.com/OnionUI/Onion/blob/v4.3.1-1/static/build/.tmp_update/runtime.sh).

Existence checks are **not** the race guarantee; the atomic no-replace link is.
An existing command, a late competing link winner and an unrelated stock writer
that publishes during staging are preserved by the tested transaction. The pending
pathname is never unlinked by the adapter, on either success or failure.

Remaining concurrency limits: unmodified keymon or another writer can subsequently
truncate/replace the pending command; stock runtime's `mv -f` can replace its active
SD path. There is no shared lock honored by these writers. This prototype guarantees
that **its publication** does not overwrite a competing pending command, not that
it owns all later Onion writes. Shutdown can also arrive after the gate's check;
stock runtime still owns shutdown precedence. No speculative global-writer patch
or ownership relaxation is included. Abrupt termination before publication can
leave a private staging orphan, but cannot queue it; cleanup failure retains its
name in process state and blocks another redirect in that MainUI invocation.

## Existing launch and return compatibility

With stock audio-fix shared value 13 enabled and normal MainUI cwd, native output is:

```sh
cd /mnt/SDCARD/App/BetterFavoritesTest; chmod a+x ./launch.sh; LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so   ./launch.sh
```

Native output includes trailing whitespace. The emitted command from each ARM test
passes unmodified `validAppCommand` via both `publishOnionLaunchCommand` and
`publishOnionSwitcherRequest`, and shell `bf_return_is_app`. OFF/ON checks also
verify existing generation/ticket rules and MENU's unchanged history behavior.
Audio-fix Off, getcwd failure or another cwd produces a different native command;
the prototype falls back, **without changing shared audio settings**.

The existing launcher captures the runtime active App command, runs the app, waits
for SDL/audio cleanup and only then publishes exit-20/21 handoff. No MainUI PID
capture, stop/kill or new loop is added. Runtime return ownership starts only from
existing A/MENU tickets; entering from Home creates no separate return marker.
B follows ordinary App return. The redirect runs only on activation, so restoring
Home selection does not itself reopen Better Favorites. Home/B/A/MENU/GameSwitcher
passed user testing on the tested Mini Plus/card. Uninstall, repeated reboot and
fault cases remain separate qualification items.

## ELF placement, byte map and sizes

Original file: 1,443,156 bytes. Reviewed output: **1,592,728 bytes**
(+149,572). Generated vendor binaries stay in ignored host build folders.
The fourth, diagnostic-only hook is at ELF entry `0x15a30`; it preserves incoming
r0–r12/LR and the entry stack, calls raw bounded diagnostics, replays `mov fp,#0`,
then resumes `0x15a34`. It performs no allocation, VFP operation or constructor work.
The harness executes this hook with a test-only return continuation; real boot is
not hardware verified. No application action is changed by this entry hook.

- New RX at VA `0x190000`, file `0x180000`: **19,864 bytes**,
  including relocated 10-entry PHDR table and 1,637 merged exidx entries.
  Code starts `0x190200`. The original first-load bias plus `e_phoff` still equals
  `PT_PHDR`; original imports, PT_NOTE/INTERP/DYNAMIC/RELRO/STACK are retained.
- BSS **128 bytes**, RW memsz growth 136 including alignment.
  Extra RW mapped pages: zero at 4K and 64K. New RX maps 20,480 bytes
  at 4K / 65,536 at 64K. No mapped W+X or overlapping load ranges.
  These are ELF layout sizes, **not measured RAM**.
- Appended zero alignment padding remains 129,708 unmapped bytes. PREL31 entries
  retain original function/extab targets and the native range-ending sentinel.
- Native exceptions use the existing libstdc++/libgcc_s EHABI; allocation-failure
  checks balance new/delete. Activation fixtures preserve r4–r11 and VFP d8–d15.

Exact hook words (little-endian bytes):

| File offset / VA | Original | Replacement | Destination |
| --- | --- | --- | --- |
| `0x17860` / `0x27860` | `0030a0e3` | `f0a605ea` | `bf_activation_hook` `0x191428` |
| `0x7fc0` / `0x17fc0` | `00482de9` | `23e505ea` | `bf_writer_hook` `0x191454` |
| `0x88d0` / `0x188d0` | `88301be5` | `ede205ea` | `bf_publication_hook` `0x19148c` |
| `0x5a30` / `0x15a30` | `00b0a0e3` | `9eee05ea` | `bf_startup_hook` `0x1914b0` |

Every changed original byte range (including ELF metadata):

| Offset | Before | After |
| --- | --- | --- |
| `0x1c` | `34` | `00` |
| `0x1e` | `00` | `18` |
| `0x2c` | `09` | `0a` |
| `0x5a30` | `00b0a0e3` | `9eee05ea` |
| `0x7fc0` | `00482de9` | `23e505ea` |
| `0x88d0` | `88301be5` | `ede205ea` |
| `0x17860` | `0030a0e3` | `f0a605ea` |
| `0x160380` | `949f16` | `701a19` |
| `0x160384` | `949f15` | `701a18` |
| `0x160388` | `8832` | `2833` |

**27 original bytes changed**; old inactive PHDR table is retained.
Payload ELF SHA-256: `1623d14320fccb07d1bb02a9a5ee2156de1da92ecbfe1f14292edeb07d125b16`.
Full maps: `build/m6-home-review-final/all-variants/<variant>/byte-map.json`.
The variant tag makes appended payload hashes differ; each map records its exact
append size/hash and full output hash. Reviewed package catalogue is
`integration/mainui-home/package.json`; C++ availability constants are generated
with it by `catalogue.py`.

| Reviewed output | SHA-256 |
| --- | --- |
| MainUI-283-clean | `a8a9771d02fbc7d13d7be2edf6cebe0d7b806c5b2a25256753bd8ed16f02b08a` |
| MainUI-283-expert | `e05e855c94a9e7bea702e552672297624b42b7e338df57c0b8bdd240a7978d12` |
| MainUI-354-clean | `4a96ca03acd4f9a4b1ffd23bcb1586d0ec509b71b3006e5322bd717922e830f4` |
| MainUI-354-expert | `fb094eb2300562642b7f43436e441b9233694f5263f870b974086e88e3db8c75` |

## Checks and repeatability

- Existing Docker GCC 8.3.0 toolchain compiles adapter with `-Wall -Wextra -Werror`;
  final adapter/harness compile has no warnings. No app dependency was changed.
- `layout_test.py` checks all four original hashes, supported/displaced instructions,
  branch targets, untouched stock builder/icons/input/state code, program headers,
  PREL31 targets, page bounds/permissions, modified-input refusal and byte coverage.
- QEMU ARM harness maps **actual patched MainUI** and compiled adapter, binds
  libstdc++/libc imports, and executes native AppAction constructor/execute/destructor,
  native formatting, original result epilogues and native Home state serialization.
  All four pass: default/disabled/malformed/symlink/FIFO preference, malformed/duplicate
  config, FIFO config/launcher/binary, unavailable launcher/binary/corrupt header, Home isolation, shutdown,
  audio/cwd fallback, existing/late/other-writer conflicts, create/chmod/write/fsync/
  close/link errors, EINTR/short writes, three allocation-failure sites, post-commit
  exception, no private orphan in handled paths and ordinary writer/AppAction use.
- Target dynamic-loader trace recognizes every patched ELF and resolves original
  imports without starting MainUI. Emulated final load permissions are applied
  before test execution; setup uses writable mappings solely for test substitution.
- Actual emitted commands pass unchanged host C++ A/MENU guards OFF/ON and shell
  return command validation. Existing project regressions and shell syntax pass.
  Existing host locale warning (`C.UTF-8` unavailable) is unrelated to this adapter.

Build inside the existing Docker toolchain (normal repository volume; no card write):

```sh
source /root/setup-env.sh
integration/mainui-home/build.sh /root/workspace/miyoo-better-favorites/build/m6-recheck
```

On the host, choose a **fresh** output directory for each run:

```sh
python3 tests/mainui-home/layout_test.py /Volumes/MIYOO/.tmp_update/bin \
  build/m6-recheck/adapter.elf build/m6-recheck/all-variants
```

`tests/mainui-home/run-arm-checks.sh` requires `qemu-arm` in an isolated disposable
Docker toolchain container, **without any mounted card**, and the generated
all-variants directory copied inside it. It creates device-path fixtures only in
that container; never run it on the device. QEMU was added only to the temporary
host test container, not project dependencies or the card. Logs and loader traces
are in `build/m6-home-review-final/emulated-results/`; command compatibility fixture is
`tests/mainui-home/command_compatibility.cpp` (link with existing launch/settings
sources) and takes a generated-command file.

### Remaining device limits

The harness replaces unavailable shared-memory/device APIs and the native App
recent service with fixtures; native stock Favorites construction is replaced by
a test continuation to observe displaced replay. Unwinding discovery is interposed
for the manually mapped ELF; the target loader was separately inspected, not
exercised through a full real MainUI session. No emulated framebuffer, keymon,
complete startup/shutdown or power-loss test is claimed. Verify real Home selection
restoration, B no-loop behavior, launch/MENU/return, stock isolation, other-writer
and interruption behavior on hardware after a reviewed reversible installer exists.
The user now confirms boot, OFF/ON Home dispatch, B no-loop restoration, Apps/X/Y
and A/MENU/Automatic return on the tested Mini Plus/card; see [actual deployed
hashes and missing log evidence](m6-acceptance.md). Other binary/device/version
compatibility, uninstall and hardware fault cases remain unverified. New wording
and recovery changes after collection are host-only. Diagnostics are opt-in and do not prove device acceptance. No performance or RAM gain is claimed.
