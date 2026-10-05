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
| Windows 10 through current releases, x86/x64 and modern ARM64 | Modern executables; OS-provided Windows PowerShell/WMI selection; ARM64 uses its native installer | Windows 10 x64 user passed the same flow; exact build/tool hash not supplied |
| macOS Monterey onward, Intel/Apple Silicon | OS-provided shell/tools/libraries; native executable selection including Rosetta | RC1 MacBook Air M1 / Ventura13.7.8 checks; RC6 M1/Ventura install/uninstall/app use user-confirmed; later Intel installation succeeded after manual stale-receipt removal; RC7 correction pending |
| Linux x64 | Kernel3.2+, POSIX sh/uname, writable mount; static executable | Docker package/installer fixtures only |
| Linux ARM64 | Little-endian ARMv8.0, kernel3.7+, otherwise same | Docker/simulated architecture checks; physical reader qualification pending |

Native Windows x86/ARM64/8/8.1, additional Intel Mac versions and both physical Linux
architecture/reader combinations still need testing. The diagnostic export verifies
the earlier RC2 Miyoo payload, **not an exact later host dispatcher hash**. Do not
extend that evidence to every compiled branch. [Windows report analysis](release/windows-acceptance.md),
[Mac RC1 evidence](release/rc1-mac-acceptance.md), [host dispatch/minima](release/host-dispatch.md).

All hosts install the same payload at the same SD paths. Unsupported/ambiguous
host probes fail before installer writes. Linux binaries have no dynamic-library
requirements; Mac/Windows use OS-supplied libraries. Native target qualification
is distinct from code support, cross-compilation and Docker/QEMU execution.

## Current release evidence and limits

The latest user report confirms M1 installation and complete uninstall, Windows installation and complete uninstall without a security warning using the built-in selector, and working app behavior. Earlier Windows 7 SP1 x64 / Windows 10 x64 tests include stock device boot; the latest selector trial did not specify its Windows version.

Intel Mac installation failed on `home-integration.conf` after Windows uninstall and succeeded after manual removal. The supplied error reports preflight failure and no writes. Its original detailed log and pre-removal receipt/recovery state are not available on this Mac. Current read-only inspection sees a successful reinstall with a matching receipt and patched system files; it cannot reconstruct the earlier cause.

RC7 tests shared-backend full uninstall → package copy → install and recovery-backed obsolete-receipt reconciliation. The Intel sequence and final Settings Back/B/START correction remain hardware-pending. Physical Linux readers remain community-test targets. Historical RC4–RC6 failures are retained in release records, not the current normal-flow status. See [RC7](release/rc.7.md).

[Dependency audit](release/dependency-audit.md) distinguishes supplied notices/source from exact prebuilt reproduction. No measured cache speedup or gameplay ON/OFF memory result is claimed. [Profiling evidence](m5-profiling.md).

## Mac Terminal download route

The recommended Mac command was user-tested on Intel and M1 Macs on 2026-10-05:
download and installer launch succeeded with no reported security warnings.
The OS versions were not restated for these runs. This evidence is separate from
recorded package install/uninstall and device checks above. Physical Linux online
installation and a Windows online download route are not qualified by this test.
