# Compatibility and test evidence

**Designed for Mini and Mini Plus; hardware tested on Mini Plus.**
Target support and recorded tests are separate. Builds, simulation and an installer
message do not substitute for a device boot or native filesystem test.

## Miyoo and Onion

| Configuration | Evidence | Limits |
| --- | --- | --- |
| Mini Plus MY354, firmware 202306282128, Onion v4.3.1-1 | User-confirmed browsing/themes/audio/settings/removal/launch/GameSwitcher/return and packaged installation checks | Hardware revision unknown; MY354 is the model/platform code |
| Home Favorites redirect | OFF stock/ON app, B return without loops, Apps/X/Y accepted; 354-clean actually exercised | Four exact binary variants have ARM fixtures, not four hardware combinations |
| Automatic session return | OFF/ON, A resume, B/START return, game switching and restored browser position accepted | Storage/shutdown/restart fault coverage is mostly host fixtures |
| Mini / other firmware and theme combinations | Source/binary catalogue includes 283 variants | Community hardware testing required |

Optional integrations accept exact hashes from the Home and return catalogues.
Unknown or independently patched MainUI/runtime files are refused. Original files
are backed up per card; existing verified preferences/original backups are retained
on updates. Availability is separate from saved ON/OFF switches; installation and
restoration of system files require reboot.

Mini Flip/Flip V2, other operating systems and unaudited Onion versions/forks are
not qualified for these optional integrations. App-only use does not establish
compatibility. There is no separate Home tile or global shortcut in this release.

## Installation computers

| Target | Requirements | Recorded execution/acceptance |
| --- | --- | --- |
| Windows 7/8/8.1 x86/x64 | Legacy Go1.20.14 binaries; SSE2 for x86; local writable card drive | Windows 7 SP1 x64 user passed install/use/uninstall/stock device boot |
| Windows 10 through current releases, x86/x64 and modern ARM64 | Modern executables; native version/architecture dispatch; ARM64 needs x86 bootstrap emulation | Windows 10 x64 user passed the same flow; exact build/tool hash not supplied |
| macOS Monterey onward, Intel/Apple Silicon | OS-provided shell/tools/libraries; native executable selection including Rosetta | MacBook Air M1 / Ventura13.7.8 packaged device checks; native fixture roundtrips and actual Rosetta selection/status |
| Linux x64 | Kernel3.2+, POSIX sh/uname, writable mount; static executable | Docker package/installer fixtures only |
| Linux ARM64 | Little-endian ARMv8.0, kernel3.7+, otherwise same | Docker/simulated architecture checks; physical reader qualification pending |

Native Windows x86/ARM64/8/8.1, physical Intel Mac/Monterey and both physical Linux
architecture/reader combinations still need testing. The diagnostic export verifies
the earlier RC2 Miyoo payload, **not an exact later host dispatcher hash**. Do not
extend that evidence to every compiled branch. [Windows report analysis](release/windows-acceptance.md),
[Mac RC1 evidence](release/rc1-mac-acceptance.md), [host dispatch/minima](release/host-dispatch.md).

All hosts install the same payload at the same SD paths. Unsupported/ambiguous
host probes fail before installer writes. Linux binaries have no dynamic-library
requirements; Mac/Windows use OS-supplied libraries. Native target qualification
is distinct from code support, cross-compilation and Docker/QEMU execution.

## Current release limits

User-confirmed Windows uninstall/stock boot is independently supported by mounted-card
verification: all five system files match this card's verified stock originals,
with the app and active integration files absent. Fault tests do not become hardware
acceptance merely because the normal flow passed.

The new copy-and-click candidate still needs its short final-package device cycle.
Unsigned Finder/Explorer/desktop opening has not been established by shell fixtures.
Component-specific attribution evidence is documented in the [dependency audit](release/dependency-audit.md); exact byte reproduction is a separate goal.
[Current acceptance steps](release/rc.3.md). Earlier RC2 evidence is historical. Disk sizes are not RAM measurements; no measured
cache speedup or gameplay/ON-OFF process-memory result is claimed. [Profiling evidence](m5-profiling.md).
