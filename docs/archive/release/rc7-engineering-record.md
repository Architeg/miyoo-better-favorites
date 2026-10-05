> **Historical RC7 preparation evidence.** Current limits are in [compatibility](../../compatibility.md).

# Release engineering notes — v1.0.0-rc.7

[User-facing release notes](../../release/notes-rc.7.md) · [Install](../../install.md)

This engineering record preserves the source changes, test evidence and known gaps.
The one install ZIP contains `App/BetterFavorites`; source/license companions are
for developers and attribution, not additional installation steps.

## Recorded hardware evidence

User-confirmed Mac M1 installation and complete uninstall passed. Windows installation and complete uninstall passed without security warnings using the built-in selector. App behavior otherwise passed. The tested preceding user ZIP is `better-favorites-1.0.0-rc.6.zip`, SHA-256 `73b9418114ab3955119d818f5fef5ea7bfbab79111d858da874ed3bd8477ca41`; ARM app SHA-256 `59e29da899280daf5361b1ce5ea09ff688176f8d9619b26c3cdc9039b959fb7b`. User confirmation is distinct from a full hash-bound log for each host run.

Designed for Mini and Mini Plus; hardware tested on Mini Plus: MY354, firmware 202306282128, Onion v4.3.1-1, hardware revision unknown. Earlier host evidence covers M1/Ventura13.7.8, Windows7 SP1 x64 and Windows10 x64. The latest Windows selector acceptance does not identify its exact OS version. Intel Mac installation succeeded after manual receipt removal. Physical Linux qualification remains open.

## Intel failure: established facts and limits

The supplied installer error says `unknown/modified app input preserved: App/BetterFavorites/home-integration.conf`, preflight, no card writes. The actual detailed Intel log is not present on this Mac. Neither the original receipt bytes nor the Windows uninstall recovery/result is available here. Therefore we cannot establish whether that operation retained/restored the receipt or another operation recreated it.

Read-only mounted-card inspection after the successful reinstall found receipt SHA-256 `643750c6c4c577dd55022b368e885ee187fff617e0f920c0162cf3995c51c2cd`, matching the four installed MainUI hashes. Its new indexed journal records `before: absent`, `stock: absent`, confirming this successful install started without a receipt. The preceding ZIP contains no receipt. These facts do not establish the earlier receipt's ownership.

## Focused corrections

- Recovery creation normalizes both current/legacy receipt records: generated integration state with stock absence, never an original to restore. Restore handles authenticated receipt records even if an older caller omitted the integration flag. Full uninstall verifies neither app location retains a receipt, as well as stock system hashes, before reporting success.
- Full reinstall reconciles an obsolete receipt only when a verified indexed or unambiguously discovered recovery records its exact bytes, the audited originals and patched identities, and runtime plus all four MainUI files are currently stock. Foreign/missing/ambiguous evidence remains a failure; the installer never accepts a receipt merely by name. The normal transaction preserves original bytes for rollback. Updates still preserve preferences and recovery.
- Home availability verification is cached for one menu session. Back from About/How to open reuses it. Closing/reopening Settings refreshes it, and changing the Home preference forces fresh verification before writing. No delay, daemon or altered game handoff is added.
- The actual Settings Automatic return panel now uses separate `[B] / [START]` tokens in ON and OFF descriptions, preserving wording, fonts, colors and geometry.

The accepted RC6 work includes inventory-bound Mac directory metadata, portable recovery lineage/lifecycle cleanup, accurate normal-versus-security errors, stable verified Mac helper paths and built-in Windows selection. Previous x86 dispatch executables remain historical tools, not a new RC7 feature.

## Install / uninstall

1. Download the single ready-to-install ZIP from this version's release; read `Mac-first-open.html` on Mac.
2. Power off, mount the card and copy/merge `App/BetterFavorites` into its `App` folder.
3. Open the platform launcher in that copied folder. Choose **1 Install / Update**, confirm power OFF, then safely eject and boot.
4. Both supported patches are installed; **Replace stock Favorites** and **Automatic return** start OFF. Enable independently in Settings. OFF is not uninstall.
5. To fully remove, power off/mount again, open the same launcher and choose **2 Uninstall completely**. It verifies stock restoration, archives recovery on the computer and removes owned app/data. **3 Export diagnostics** collects a reviewed report without Miyoo Terminal.

Deleting the app cannot undo patches. Stop on unknown modifications; use verified recovery. Unsigned Mac script/helper may require separate file-specific approval. No global security/quarantine change is used. [Install](../../install.md), [recovery](../../recovery.md), [security guide](../../security-opening.md).

## Checks and remaining acceptance

Host checks cover shared-backend updated installation → complete uninstall → package copy → reinstall, exact receipt stock absence, authenticated obsolete receipt and foreign/no-recovery/modified-system rejection. Existing metadata and interrupted cleanup checks remain. Real SDL Settings ON/OFF checks assert both badges and text bounds; a real MenuState regression counts verification across submenu Back, closure and preference changes. ARM compilation is not hardware acceptance.

Remaining targeted checks:

- Intel: Windows-style full uninstall → copy RC7 → install without deleting a receipt manually; complete uninstall and stock boot. Preserve logs and recovery if any conflict is reported.
- Device: Back from both explanation pages returns promptly; ON and OFF Settings descriptions show B and START correctly; Home switch still revalidates.
- Community: physical Linux x64/ARM64 install/uninstall and unverified host/device combinations.

Dependency source/license companions and notices accompany the binary package. Exact reconstruction of permissive prebuilts remains separate from supplied attribution; see [component audit](../../release/dependency-audit.md).
