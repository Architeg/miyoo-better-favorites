# Compatibility

**Designed for Mini and Mini Plus; hardware tested on Mini Plus.** Supported build targets and recorded tests are different. A successful build or installer message is not a device boot test.

## Miyoo and Onion

| Configuration | Recorded evidence | Limits |
| --- | --- | --- |
| Mini Plus MY354, firmware `202306282128`, Onion `v4.3.1-1` | Browsing, themes, audio, settings, removal, A/MENU/GameSwitcher/return and packaged installation checks | Hardware revision unknown; MY354 identifies the model/platform |
| Home Favorites redirect | OFF/ON, B return without loops, Apps/X/Y tested on 354-clean | Other exact variants have ARM fixture coverage, not hardware acceptance |
| Automatic return | OFF/ON, A resume, B/START return, game switching and restored position accepted | Storage/shutdown/restart faults mostly have host coverage |
| Mini and other firmware/theme combinations | Catalogue includes 283 variants | Community hardware testing needed |

Both integrations require exact audited MainUI/runtime hashes. Unknown or independently modified system files are refused. Availability is separate from saved ON/OFF preferences. Installation and restoration require reboot.

Flip/Flip V2, other operating systems and unaudited Onion versions/forks are not qualified for these patches. There is no separate Home tile or global shortcut. App-only operation is not proof of compatibility.

## Installation computers

| Target | Requirements | Recorded tests |
| --- | --- | --- |
| Windows 7/8/8.1 x86/x64 | Legacy build; SSE2 for x86; local writable card drive | Windows 7 SP1 x64 install/use/uninstall/stock boot reported |
| Windows 10 onward x86/x64/ARM64 | Modern build; OS-supplied PowerShell/WMI | Windows 10 x64 same flow reported; other versions/architectures need testing |
| macOS Monterey onward, Intel/Apple Silicon | OS-supplied shell/tools; Rosetta entry supported | M1/Ventura 13.7.8 package checks; later M1 install/uninstall and Intel install reported |
| Linux x64 | Kernel 3.2+, POSIX shell/uname, writable mount | Container/fixture checks; physical reader tests pending |
| Linux ARM64 | Little-endian ARMv8.0, kernel 3.7+, same tools | Simulated/cross-build checks; physical reader tests pending |

All hosts install identical Miyoo payloads. Linux installers are static; Mac/Windows use OS-supplied libraries. No compiler, Python, Docker, WSL or Go installation is needed for users. The optional Terminal download route needs Internet and the listed [standard tools](online-install.md#requirements).

## Evidence limits

The Mac Terminal download/installer launch was reported successful on Intel and M1 on 2026-10-05, without security warnings. OS versions were not restated. This does not independently verify a complete RC7 install/uninstall cycle.

Earlier Windows 7 SP1 x64 and Windows 10 x64 tests include stock device boot. A later built-in-selector trial passed but did not identify its Windows version. Diagnostic exports do not bind every later host executable hash. Native Windows x86/ARM64/8/8.1 and physical Linux remain unqualified.

Intel installation succeeded after manual receipt removal. RC7's automatic recovery-backed correction and final Settings Back/B/START changes remain device-pending. Do not turn that earlier workaround into normal instructions. [RC7 targeted checklist](release/rc.7.md).

## Further references

[Host selection/minima](release/host-dispatch.md) · [Dependency/source limits](release/dependency-audit.md) · [Historical test evidence](archive/README.md)

No measured cache speedup, full gameplay ON/OFF RAM comparison or measured gameplay process absence is claimed.
