# Compatibility

**Designed for Mini and Mini Plus; hardware tested on Mini Plus.**

| Layer | Accepted evidence | Limits |
| --- | --- | --- |
| Native app | Mini Plus MY354, firmware 202306282128, Onion v4.3.1-1; navigation/theme/audio/launch/removal/return accepted | Latest release wording/logging/package await device test; Mini needs community testing |
| Home redirect | Same Mini Plus; actually executed 354-clean; OFF stock, ON app, B Home/no-loop twice, Apps and X/Y passed | Four exact binary variants pass ARM fixtures; this is not four-device hardware acceptance |
| Session return | Onion v4.3.1-1 exact runtime/helper hashes; user OFF/ON, switching, resume/restored position accepted | Restart/shutdown/storage fault matrix mostly host fixtures |
| Native host installer | macOS filesystem fixtures and exact generated outputs; Linux fixture checks | Native Windows binary exists but execution/FAT reader qualification pending |

MY354 identifies the Mini Plus platform/model; **hardware revision is unknown**.
Firmware is user supplied, separate from Onion. See [accepted deployed hashes and
log coverage](m6-acceptance.md). The official [latest Onion release](https://github.com/OnionUI/Onion/releases/latest)
resolved to v4.3.1-1 on 2026-10-03; this does not qualify other forks or prereleases.

Optional patch compatibility is an exact allowlist in
`integration/mainui-home/package.json` and `integration/onion-return/hashes.json`.
Unknown MainUI/runtime bytes, symlinks/reparse points and foreign changes are
refused. Prior exact Better Favorites installations can be adopted only with
verified manifests, receipts/helpers and original backups. Existing permanent
return backups and preferences are preserved during installation/update.

Mini 283, other firmware/theme combinations need testing. Mini Flip/Flip V2,
other OSes/Onion versions and independently patched MainUI/runtime are unsupported
for these optional integrations. Copying the app does not establish compatibility.
Availability shown in Settings is independent of an ON preference. Installation
and removal take effect after reboot; settings activation does not repatch binaries.

DLL/ELF sizes and mapped payload size do not establish RAM consumption. Startup
pilot/cache-read evidence is in [M5](m5-profiling.md); no measured cache speedup is
claimed. Gameplay/ON-OFF/process absence measurements remain deferred.
