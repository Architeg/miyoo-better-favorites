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
Windows uses OS DLLs, not a separately installed Go/C runtime. Offline use requires no PowerShell upgrade,
WSL, compiler, Python, Docker or Internet. The optional downloader needs Internet.

## One entry script per platform

Mac recommends the [Terminal download route](../online-install.md); the offline route copies the app folder to the card and opens its platform launcher. These are optional support commands from that copied app folder:

| Platform | Install | Complete uninstall |
| --- | --- | --- |
| Windows 7 onward | `.\Install-Windows.cmd install` | `.\Install-Windows.cmd uninstall` |
| macOS | `./Install-macOS.command install` | `./Install-macOS.command uninstall` |
| Linux | `sh ./Install-Linux.sh install` | `sh ./Install-Linux.sh uninstall` |

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
qualification for the specific version/architecture being claimed.

Mac checks sw_vers >=12, uname and native/translated sysctl results. A translated
x86_64 process on Apple Silicon selects arm64, including when its emulated CPU
view reports hw.optional.arm64=0 or lacks that key. Intel lacking hw.optional.arm64 must
identify its native hw.cputype explicitly. Contradictory probes fail. Linux checks
uname OS/native architecture and kernel release before invoking the native tool.
Copied-card Unix wrappers wait for staged execution, retain its exit code and remove only their own bootstrap directory. Standalone advanced wrappers exec the backend. No host test overrides are
accepted in production; tests substitute probes only in temporary copies.

## Evidence and remaining qualification

Use the [current compatibility matrix](../compatibility.md). Earlier RC1/RC2 dispatch, Rosetta and Windows test narratives are preserved in the [historical qualification record](../archive/release/host-dispatch-pre-cleanup.md), not qualification of every current executable.

The built-in Windows selector has later user acceptance, but the trial did not identify its exact Windows version. Native x86/ARM64/8/8.1 and physical Linux readers remain pending. HOST-BUILDS.json records executable hashes/toolchains/linkage; metadata inspection is not native execution evidence.

The old Windows 7 Desktop test archive is developer material, not a second current user ZIP. Its alias calls the same installer. Go1.20.14 is end-of-life; legacy support carries that disclosed tradeoff. Keep modern and legacy runtime identities separate.

## Official platform references

- [Go minimum runtime/CPU requirements](https://go.dev/wiki/MinimumRequirements).
- [Go1.20 last Windows7/8 release](https://go.dev/doc/go1.20#windows).
- [Go1.26 last Monterey release](https://go.dev/doc/go1.26#darwin).
- [Microsoft native architecture detection](https://learn.microsoft.com/en-us/windows/win32/api/wow64apiset/nf-wow64apiset-iswow64process2).
- [GetNativeSystemInfo and ARM emulation caveat](https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getnativesysteminfo).

These requirements and toolchain pins must be reviewed on each release update.
