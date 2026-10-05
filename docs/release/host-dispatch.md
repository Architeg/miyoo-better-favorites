# Host compatibility and executable dispatch

This is the required target matrix, **not a claim of native acceptance on every OS**.
Every host installs the identical package.json/payload tree into the same Miyoo SD
paths. Only host selection and platform filesystem handling differ. All installers
share the same restoration/hash/ownership/full-uninstall implementation.

| Target | Native packaged executables | Minimum runtime / prerequisites |
| --- | --- | --- |
| Windows 7 / 8 / 8.1 | legacy x86, x64 — official Go 1.20.14 | NT 6.1–6.3, SSE2 for x86; local writable SD drive; native Windows system DLLs |
| Windows 10 through current releases | modern x86, x64, ARM64 — Go 1.26.2 | Windows 10 build 10240 or later; ARM64 requires native Windows ARM64 |
| macOS Monterey onward | native Intel x64 and Apple Silicon ARM64 — Go 1.26.2 | macOS 12+, /bin/sh, system uname/sw_vers/sysctl; locally mounted writable SD filesystem |
| Linux x64 | amd64 baseline v1 — Go 1.26.2 | kernel 3.2+, POSIX sh and uname, futex/epoll, normal device randomness, executable private computer temporary directory |
| Linux ARM64 | little-endian ARMv8.0 — Go 1.26.2 | kernel 3.7+ (AArch64 kernel availability), otherwise same requirements |

Linux builds use CGO_ENABLED=0, default executable mode and internal linking.
Packaging inspects ELF program headers: PT_INTERP and PT_DYNAMIC must both be
absent. No libc/glibc/musl or other dynamic-library dependency is required.
The wrapper needs a normal userspace shell/uname; the card needs mounted FAT-capable
OS support and sufficient write permissions. Restricted containers/seccomp and WSL are not qualified substitutes for native Linux. The card itself may be noexec: the wrapper copies the selected native executable to a private computer temporary directory before running it. A noexec computer temporary directory fails clearly.
Mac tools load OS-provided system libraries; no Homebrew/Xcode/runtime installation
is needed. Packaging inspects Mach-O deployment targets and refuses a minimum above
12.0. Pin Go1.26.2: upgrading to Go1.27 would drop Monterey and needs a new decision.
Windows uses OS DLLs, not a separately installed Go/C runtime. No PowerShell upgrade,
WSL, compiler, Python, Docker or Internet is required by users.

## One entry script per platform

The recommended route is to copy the ready-to-install package app folder to the card, then open its platform launcher. These are optional support commands from that copied app folder:

| Platform | Install | Complete uninstall |
| --- | --- | --- |
| Windows 7 onward | `.\Install-Windows.cmd install` | `.\Install-Windows.cmd uninstall` |
| macOS | `./Install-macOS.command install` | `./Install-macOS.command uninstall` |
| Linux | `./Install-Linux.sh install` | `./Install-Linux.sh uninstall` |

Action-only prompts, no-argument menu, explicit flags, export-diagnostics,
remove-integrations and emergency restore all share this interface. Uninstall
restores and verifies stock before full owned-file removal; computer-side verified
recovery remains outside SD. Foreign changes fail without claiming completion.
Both integrations are installed by default on supported cards; saved switches default OFF.

Windows cmd uses built-in PowerShell/WMI to select the existing native helper.
Windows 7/8/8.1 use legacy x86/x64 builds; supported Windows 10+ uses modern
x86/x64/ARM64. Native processor identity, not shell bitness, selects architecture;
Sysnative selects native PowerShell when called from a 32-bit shell on x64.
Native PowerShell pointer size distinguishes a 32-bit Windows installation on an
x64-capable processor from a 64-bit Windows installation.
Unknown versions, ambiguous probes, links and missing/changed helpers fail before
card writes. The generated literal command has no script-policy override or
encoded/downloaded code. Its readable source is `packaging/windows-select.ps1`.

The copied batch and selected helper run off-card, with byte verification against
`HOST-SHA256SUMS`. The helper uses the same transaction backend and receives the
original app path. Ordinary child exit status is retained. No compiled dispatcher
is packaged. Selector tests executed in Linux PowerShell plus source checks are
not native Windows or antivirus acceptance; Win7's bundled PowerShell still needs
physical qualification of this new entry.

Mac checks sw_vers >=12, uname and native/translated sysctl results. A translated
x86_64 process on Apple Silicon selects arm64, including when its emulated CPU
view reports hw.optional.arm64=0 or lacks that key. Intel lacking hw.optional.arm64 must
identify its native hw.cputype explicitly. Contradictory probes fail. Linux checks
uname OS/native architecture and kernel release before invoking the native tool.
Copied-card Unix wrappers wait for staged execution, retain its exit code and remove only their own bootstrap directory. Standalone advanced wrappers exec the backend. No host test overrides are
accepted in production; tests substitute probes only in temporary copies.

## Evidence and remaining qualification

- RC1: actual MacBook Air M1 / Ventura 13.7.8 packaged install; user confirmed six
  device checks. RC1 evidence and recovery remain preserved. Stock device boot after
  that historical RC1 removal was not separately identified; later Windows uninstall/stock boot acceptance is recorded independently.
- RC2: Mac-host installer safety/actual ZIP roundtrips and Linux Docker fixture
  execution. A Docker or QEMU run is not native-machine acceptance of both Linux
  architectures. Actual Rosetta x86_64 entry on this M1/Ventura13.7.8 selects/executes the native
  ARM64 backend (read-only status fixture); this is not a separate Intel Mac or
  Monterey execution test. Later Intel/Monterey installation is user-confirmed after receipt removal; RC7’s receipt correction and both physical Linux architectures remain pending. User-confirmed Windows7 SP1 x64 / Windows10 x64
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
Install-Windows7.cmd calls the same Windows entry script; no separate
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

## Copy-to-card follow-up evidence

The RC3 entry captures the original logical app path before moving execution off-card. The backend validates App/BetterFavorites, Onion version/runtime/ARM MainUI and all path ancestors, then verifies the transport/package before staging. Windows transfers its batch context and working directory to a private computer folder so the card folder can be removed. The active portable journal index uses card-relative paths/hash, not the original host username/mount.

Native macOS shell-entry and Linux-container user-ZIP roundtrips exercise the new menu and complete removal. GLib desktop Exec parsing is tested with a non-terminal fixture; this is not a desktop terminal/Allow launching test. Finder quarantine/opening and Explorer's new staged batch execution cannot be established by these headless checks. Previous Mac/Windows install acceptance remains valid for its earlier package, not these new launcher bytes. Community target testing is welcome; this pass asks for one final-package fresh-user cycle, not a broad new OS matrix.
