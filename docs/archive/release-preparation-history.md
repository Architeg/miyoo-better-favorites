# Historical release preparation

These dated notes describe earlier development states. Current installation,
compatibility and delivery status are in the [user guide](../install.md) and
[current status](../development-status.md).

## v1 release preparation update — 2026-10-03

Accepted M6 checkpoint is committed. Public wording: **Designed for Mini and Mini
Plus; hardware tested on Mini Plus.** Mini Plus MY354, firmware202306282128,
Onionv4.3.1-1; revision unknown. [M6 evidence](m6-acceptance.md).

All global shortcuts are deferred entirely for v1.0; independent shortcut research
is historical, not the next implementation. Existing Home tile/theme, Apps and
X/Y remain. M7/M8 prioritize app-only ZIP, self-contained offline native host
installer, verified uninstall/manual recovery, bounded diagnostics, contributor/
release docs and license/source audit. Candidate version is **1.0.0-rc.1**.

Host/emulated checks and accepted earlier deployment do not accept rebuilt rc.1.
[Exact remaining gates](release/rc.1.md): native Windows/reader execution, final
ZIP device install→OFF/ON→game/return→uninstall/stock→reinstall, prebuilt/source/
license correspondence. Until these pass the candidate is engineering review material;
M9 stable publication is pending. Screenshots/banner remain user-supplied future
materials, not a packaging blocker. [Current status](../development-status.md).

## RC2 preparation update — 2026-10-04

RC1 six packaged-install device checks passed per user on MacBook Air M1 / Ventura
13.7.8, with hashes matching the actual RC1 artifact. Post-uninstall device boot is
pending at that RC1 review; later Windows testing confirms stock boot and
stock mounted-card hashes/integration removal are independently verified.
The retained RC1 app/installation artifacts were archived and fully cleaned after
stock verification. [Evidence](release/rc1-mac-acceptance.md).

M7/M8 prepare a fresh RC2 review snapshot: exact supplied icon, limited wording/three
headings, action-only host commands, complete verified default uninstall and separate
Windows 7 x64/x86 Go 1.20.14 test bundle. No release toolchain downgrade. Native
Windows/reader execution was pending at this preparation point. The later Windows
acceptance update below records normal-flow passes; exact later dispatcher/ZIP and
license closure remain gates. [RC2 gates](release/rc.2.md). No M9 publication or shortcut work yet.

Host compatibility update: Windows7 through current releases, macOS Monterey+
Intel/Apple Silicon, Linux x64/ARM64; one automatic-dispatch entry per platform.
See [targets, minima, dispatch and pending native qualification](../release/host-dispatch.md).
Legacy builds remain isolated; every host uses the same payload/safety implementation.

Windows acceptance update: user passed Windows7 SP1 x64 and Windows10 x64
packaged installation/use/uninstallation and confirmed stock Onion boot. All five
mounted system files equal verified stock originals; app/active integrations are
absent. [Sanitized evidence and payload/dispatcher limits](release/windows-acceptance.md).
Other targets, exact later dispatcher hashes and fault matrices remain separate gates.

### RC2 documentation and bootstrap preparation — 2026-10-04

Focused documentation presentation, current Windows evidence and the unchanged
offline one-command/full-uninstall interface are consolidated. The online bootstrap
is separate and has no advertised download URL or native Windows online acceptance.
libpng/zlib source/notices narrow prior gaps; custom SDL link reproduction and
extension/SwiftShader correspondence still block binary publication. RC2 source
checkpoint/tag and a draft prerelease do not constitute stable v1.0.0 or final-byte
hardware acceptance. [Details](release/rc.2.md), [audit](../release/dependency-audit.md).


### Copy-and-click release preparation — RC3

RC2 remains immutable at its existing annotated tag. Follow-up RC3 implements the full-package copy → click workflow, both supported integrations with fresh switches OFF, off-card staging, portable recovery, complete uninstall and once-only guidance. Updates preserve browser/settings data. The app-only package remains separate; source archives are developer material. Host fixtures and cross-builds do not establish new device or graphical launcher acceptance. See [RC3's one fresh-user cycle](release/rc.3.md) and [component-specific dependency audit](../release/dependency-audit.md). No shortcuts, profiling, new core feature or mounted-card deployment is part of this pass.

### RC4 release preparation

One user ZIP and `App/BetterFavorites` replace the live test identifier. Verified legacy migration and portable recovery are host-tested; renamed-package device acceptance remains pending. Shortcuts remain deferred. See [RC4](release/rc.4.md).

### RC7 checkpoint — 2026-10-05

M7/M8's preceding portable copy-and-click candidate passed user-confirmed M1 and Windows install/complete uninstall and app behavior. RC7 closes focused receipt bookkeeping, menu-session verification and Settings badge gaps. Intel cross-computer correction and final Settings changes await targeted hardware verification; physical Linux remains community-pending. Source/notices accompany the development build; stable publication is not claimed. [Evidence and remaining checklist](../release/rc.7.md). Shortcuts remain deferred.
