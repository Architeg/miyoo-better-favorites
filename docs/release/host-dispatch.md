# Host compatibility and executable dispatch

This is the required target matrix, **not a claim of native acceptance on every OS**.
Every host installs the identical package.json/payload tree into the same Miyoo SD
paths. Only host selection and platform filesystem handling differ. All installers
share the same restoration/hash/ownership/full-uninstall implementation.

| Target | Native packaged executables | Minimum runtime / prerequisites |
| --- | --- | --- |
| Windows 7 / 8 / 8.1 | legacy x86, x64 — official Go 1.20.14 | NT 6.1–6.3, SSE2 for x86; local writable SD drive; native Windows system DLLs |
| Windows 10 through current releases | modern x86, x64, ARM64 — Go 1.26.2 | Windows 10 build 10240 or later; ARM64 requires native Windows ARM64 and x86 bootstrap emulation |
| macOS Monterey onward | native Intel x64 and Apple Silicon ARM64 — Go 1.26.2 | macOS 12+, /bin/sh, system uname/sw_vers/sysctl; locally mounted writable SD filesystem |
| Linux x64 | amd64 baseline v1 — Go 1.26.2 | kernel 3.2+, POSIX sh and uname, futex/epoll, normal device randomness, executable extraction directory |
| Linux ARM64 | little-endian ARMv8.0 — Go 1.26.2 | kernel 3.7+ (AArch64 kernel availability), otherwise same requirements |

Linux builds use CGO_ENABLED=0, default executable mode and internal linking.
Packaging inspects ELF program headers: PT_INTERP and PT_DYNAMIC must both be
absent. No libc/glibc/musl or other dynamic-library dependency is required.
The wrapper needs a normal userspace shell/uname; the card needs mounted FAT-capable
OS support and sufficient write permissions. Restricted containers/seccomp, noexec
extraction volumes and WSL are not qualified substitutes for native Linux.
Mac tools load OS-provided system libraries; no Homebrew/Xcode/runtime installation
is needed. Packaging inspects Mach-O deployment targets and refuses a minimum above
12.0. Pin Go1.26.2: upgrading to Go1.27 would drop Monterey and needs a new decision.
Windows uses OS DLLs, not a separately installed Go/C runtime. No PowerShell upgrade,
WSL, compiler, Python, Docker or Internet is required by users.

## One entry script per platform

From the extracted normal installer folder:

| Platform | Install | Complete uninstall |
| --- | --- | --- |
| Windows 7 onward | `.\Install-Windows.cmd install` | `.\Install-Windows.cmd uninstall` |
| macOS | `./Install-macOS.command install` | `./Install-macOS.command uninstall` |
| Linux | `./Install-Linux.sh install` | `./Install-Linux.sh uninstall` |

Action-only prompts, no-argument menu, explicit flags, export-diagnostics,
remove-integrations and emergency restore all share this interface. Uninstall
restores and verifies stock before full owned-file removal; computer-side verified
recovery remains outside SD. Foreign changes fail without claiming completion.
Drag/drop app-only installation remains available; it cannot install/remove patches.

Windows cmd invokes a read-only Go1.20.14 x86 bootstrap. RtlGetVersion obtains the
actual NT version, unaffected by compatibility-limited GetVersionEx. On systems
providing IsWow64Process2 it reads the native machine; older x86/x64 Windows uses
GetNativeSystemInfo. Caller bitness and PROCESSOR_ARCHITECTURE environment values
are not used, so a 32-bit shell on x64 chooses the x64 executable. Windows7/8/8.1
select legacy; Windows10/11 select modern. Unknown versions/native machines/probe
failures and missing backends fail before launching the card-writing installer.
The bootstrap forwards argument boundaries, streams and the actual child exit code;
there is no daemon, polling or diagnostic writer. Future unrecognized NT major/minor
versions fail clearly and require qualification instead of a speculative selection.

Mac checks sw_vers >=12, uname and native/translated sysctl results. A translated
x86_64 process on Apple Silicon selects arm64, including when its emulated CPU
view reports hw.optional.arm64=0 or lacks that key. Intel lacking hw.optional.arm64 must
identify its native hw.cputype explicitly. Contradictory probes fail. Linux checks
uname OS/native architecture and kernel release before invoking the native tool.
Unix wrappers exec the backend and retain its exit code. No host test overrides are
accepted in production; tests substitute probes only in temporary copies.

## Evidence and remaining qualification

- RC1: actual MacBook Air M1 / Ventura 13.7.8 packaged install; user confirmed six
  device checks. RC1 evidence and recovery remain preserved. Stock device boot after
  its final removal remains pending.
- RC2: Mac-host installer safety/actual ZIP roundtrips and Linux Docker fixture
  execution. A Docker or QEMU run is not native-machine acceptance of both Linux
  architectures. Actual Rosetta x86_64 entry on this M1/Ventura13.7.8 selects/executes the native
  ARM64 backend (read-only status fixture); this is not a separate Intel Mac or
  Monterey execution test. Native Monterey, Intel Mac, both Linux hardware
  architectures are pending. User-confirmed Windows7 SP1 x64 / Windows10 x64
  install/uninstall and stock device boot are recorded separately in
  [Windows evidence](windows-acceptance.md); exact later dispatcher hashes are not
  present in the export. Other Windows versions/architectures remain unqualified.
- Dispatch tests: simulated Windows version/native-machine matrix; Unix wrappers
  executed with simulated OS/kernel/Rosetta probes, including rejection, argument
  forwarding and failure propagation. Windows native API probing and .cmd execution
  have been cross-compiled, not executed on this Mac.
- HOST-BUILDS.json records actual executable hashes, toolchains, Mach-O minima and
  Linux linkage. These metadata checks do not establish OS execution acceptance.

## Portable Windows 7 test archive

Keep BetterFavorites-Windows7-Test.zip and extract via Explorer to
C:\BetterFavorites-Test. Its top-level folder is BetterFavorites-Test, with the
complete identical installer payload/manifests/checksums. The promised short alias
Install-Windows7.cmd calls the same Windows entry script/dispatcher; no separate
safety implementation. From bundled PowerShell in that folder:

```
.\Install-Windows7.cmd install
.\Install-Windows7.cmd export-diagnostics
.\Install-Windows7.cmd uninstall
```

No Get-FileHash/Expand-Archive dependency; optionally use certutil for hashes. The
normal Windows10+ backends remain separate in the same package, never relabeled as
Windows7-compatible. Go1.20 is end-of-life; its legacy runtime is an explicit support
tradeoff and native qualification/security review remains required before release. The reported normal
flow passes do not qualify every version/architecture or later dispatcher binary.

## Official platform references

- [Go minimum runtime/CPU requirements](https://go.dev/wiki/MinimumRequirements).
- [Go1.20 last Windows7/8 release](https://go.dev/doc/go1.20#windows).
- [Go1.26 last Monterey release](https://go.dev/doc/go1.26#darwin).
- [Microsoft native architecture detection](https://learn.microsoft.com/en-us/windows/win32/api/wow64apiset/nf-wow64apiset-iswow64process2).
- [GetNativeSystemInfo and ARM emulation caveat](https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getnativesysteminfo).

These requirements and toolchain pins must be reviewed on each release update.
