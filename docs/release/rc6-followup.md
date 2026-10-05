# RC6 focused follow-up — evidence and entry comparison

Local review; no tag change, publication, card deployment or stable qualification. The preparation report accompanies the new ZIP, checksums and matching source companions. The source inventory identifies its uncommitted snapshot and base commit separately.

## Actual outcomes

| Trial | Evidence / result |
| --- | --- |
| RC1 MacBook Air M1, Ventura 13.7.8 | Earlier packaged installation/device acceptance retained |
| Earlier RC2 Windows 7 SP1 x64 and Windows 10 x64 | User-confirmed install/use/uninstall and stock device boot; diagnostic export establishes Miyoo payload, not exact host executable |
| RC6 M1 / Ventura | User confirms install, uninstall and app operation passed |
| RC6 Intel / Monterey | Failed architecture selection; the fallback incorrectly required an optional CPU-type value |
| RC6 Windows | Interrupted during dispatcher staging, with a Defender detection; not accepted |
| Linux | Docker/native fixture checks; no physical reader/security/device qualification |

The latest Defender transcription is `Troian:Win32/Bearfoos.B!mi` (spelling/suffix unconfirmed). User supplied security intelligence **1.459.553.0**, created **2026-10-04 15:01**, last updated **2026-10-05 00:56**; timezone not established. This update timestamp is not the detection time. Affected paths were a temporary bootstrap `dispatch.exe`, `F:\App\BetterFavorites\computer\.bf-stage-2708509933`, and the final on-card x86 dispatcher. Detection time, engine/platform version, failed-trial OS version and actual quarantined-file hash remain unknown. The public Microsoft encyclopedia entry has suffix `!ml`; it does not establish the exact suffix of this report.

Original RC6 distributed dispatcher: 1,643,008 bytes, SHA-256 `9f638b29e2021c2836f2d84f642a9acb934f16289671cd12d5110f393cd37148`. It remains preserved unchanged as historical evidence. No further Microsoft submission preparation or submission is authorized. It is not a recovered quarantined file, and no upload or false-positive conclusion has been made. Do not disable Defender, add exclusions, or blindly restore detected files. A new backend build does not establish antivirus acceptance.

## Earlier card observation (before the latest trial)

Read-only inspection after the user confirmed exclusive access found all four MainUI variants and runtime matching audited stock hashes. The app directory is absent. A valid indexed recovery journal, five verified original system backups, older project backup folders and a root AppleDouble sidecar remain. They were copied to an ignored host archive, byte-verified against the card, and retained on the card. No restoration, metadata removal or other card write was performed. The archive contains private originals and must not be shipped. This is historical file-state evidence, not proof of rollback actor or subsequent device boot.

## Preserved-package comparison

`tools/compare-installer-entry.py` reads selected ZIP entries without executing them and reports full wrapper/tool hashes. Prior archives remain intact. The accompanying private preparation report records all comparison hashes.

| Package | Mac entry | Windows entry / operations |
| --- | --- | --- |
| RC1 final | 158-byte direct architecture wrapper; no cache | Direct amd64 helper; old uninstall retained app, unsuitable to revert |
| RC2 complete | Same direct Mac wrapper | Direct amd64 entry in this preserved archive |
| RC2 host-compatible / dispatch-final | Direct native helper; Intel fallback already defective | x86 dispatcher already present, selecting legacy/modern and native architecture; not a new RC6 feature |
| RC6 review-03 | Stable per-hash cached helper for file-specific approval; copied-card context | Batch and dispatcher run off-card; dispatcher stages native helper; backend stages full package as data and redundantly republishes identical computer tools |
| Follow-up | Correct native/Rosetta detection; same byte-verified approval cache | Same dispatcher source/bytes; identical owned transport entries retain identity and recovery records without restaging/republication; disappearance still fails and rolls back owned publications |

Preserved archive SHA-256:

- RC1 final: `baed0c887a41046546f5cbe021552eebd1fd78c744e01f680c4fc7821e7a1bdf`.
- RC2 complete: `3a3efbd78b7a7b55e96cda7c31217484e00ac19d3c8fc167afa433c93ba5f2c0`.
- RC6 review-03: `e599d606f35ebbce4a3138763962be5223480d320c677281666fd83894a61fb4`.

RC2 dispatcher source is present in commit `a31f9a2fff42d12b2a9f2cc16118f3b8a43eb94a`. Compared with RC6, its dispatch rules already handled legacy Windows and shell-independent architecture. RC6 added off-card native-helper staging for a copied-card launch. Current dispatcher inspection finds OS probes, bounded copying/checksum verification and child execution, without a network client or system patch writer. This limited source review is not a security audit or a verdict on Defender.


### Executable and entry identity

Full SHA-256 values below come from the preserved archives. Package association is not proof that the exact executable was used in the historical hardware trial.

| Preserved package | Entry/tool | SHA-256 |
| --- | --- | --- |
| RC1 final | Install-Windows.cmd | `4ec9e9bd001e28c50bfcb3f5b14b7e9ac748cce8d17ed8d195de763018c4cc64` |
| RC1 final | Install-macOS.command | `1a6ba37fdb30718315b95dc6b83dff54eaffc7e7c0be0e70d559b8dfd71c2126` |
| RC1 final | better-favorites-installer-darwin-arm64 | `5917b8ee2d6b6ed846e1c259003e301d4d0ac2d40dd661ee163be87dd39d9618` |
| RC2 complete | Install-Windows.cmd | `6b73128b421324438f4e7edaac2006bd20e0a4c098e569f4211747e9aae53213` |
| RC2 complete | Install-macOS.command | `1a6ba37fdb30718315b95dc6b83dff54eaffc7e7c0be0e70d559b8dfd71c2126` |
| RC2 complete | better-favorites-installer-darwin-arm64 | `7344c479e6353e21b7dbae7a63b368649eb40805e36134812db1af6843009ff9` |
| RC2 host-compatible | Install-Windows.cmd | `de5d00a8a00e4633274f80b7750cf65ed5b935a985f651bbbb5ee44e2db28aa5` |
| RC2 host-compatible | Install-macOS.command | `f5bc6004721adf65f95b6899ce4422d89d213e90ac0514922756ff732a72977b` |
| RC2 host-compatible | better-favorites-dispatch-windows-386.exe | `ee0b97b6da1a3b5a5569d63c8f77fc1c2444c199491083f98b6e9699bcfd3a77` |
| RC2 host-compatible | better-favorites-installer-darwin-arm64 | `7344c479e6353e21b7dbae7a63b368649eb40805e36134812db1af6843009ff9` |
| RC6 review-03 | Install-Windows.cmd | `34f5ab8301610e82043d09fca34e2d2fcd127d5422daa252a9daecd1100d6d63` |
| RC6 review-03 | Install-macOS.command | `114bd931bc16ade3de7fe74c7adfa86a92a0c5b42b8778f78b4d4cf8f785a7d0` |
| RC6 review-03 | better-favorites-dispatch-windows-386.exe | `9f638b29e2021c2836f2d84f642a9acb934f16289671cd12d5110f393cd37148` |
| RC6 review-03 | better-favorites-installer-darwin-arm64 | `05a68ee75ed1696ce7ff7c384f4c51251a1765fc46e8dc6a8c81a6b4eb35c4ef` |

## Controlled comparison: one corrected installer

Host tests execute the current extracted host wrapper with explicit card/package/recovery arguments, restore stock with current complete uninstall, then execute the current copied-card wrapper/menu against the same stock fixture and current payload. Both use one transaction, metadata, recovery and uninstall implementation. They verify stock/patched outputs, protected data and recovery. Existing entry tests cover identical helper arguments/status and cache identity. Windows routing/staging tests run as simulations; no new native Windows acceptance is claimed.

The simpler direct-entry workflow is available for a **physical controlled comparison after security review**, not a bypass for the detection. Run advanced commands in the **extracted computer-tools folder**, not a different backend or old RC2 binary. Specify the same powered-off card, current package and fresh recovery/archive destinations. The standard copy-to-card click workflow remains the default. No evidence yet demonstrates fewer warnings with current binaries under direct entry, so the approval cache has not been removed or a second installer introduced. Do not retry a detected Windows file merely via another path.

## Focused corrections

- Intel selection uses native ABI and positive Rosetta evidence. Missing optional keys are accepted; contradictions fail with concise probe evidence. Both wrapper copies come from one source.
- Host transport hashes are verified before patch preparation. Read/stage errors retain the underlying OS error and separate byte mismatch evidence in the detailed log. Identical copied tools remain recorded, checked before/through/after publication, and are not rewritten. Files removed by antivirus are not silently reconstructed by rollback.
- Shared progress uses normal terminal foreground, optional non-Windows bold, ASCII spinner/elapsed time on a terminal and plain phase lines otherwise. Card operations remain synchronous; animation stops before prompts, errors or success.
- Menu **[3] Export diagnostics** uses the existing allowlisted exporter and adds at most two 128 KiB installer log tails. Missing/unsafe logs are reported; no Defender store, secrets or private recovery originals are exported.
- Requested two feature headings use existing bold white styling; descriptions keep existing fonts/sizes and inline B/START badges. Redundant Apps line and positive Available messages are removed; unavailable checks remain.

## Acceptance gates

1. Intel Monterey: current copied wrapper selects the Intel helper, install/uninstall succeeds, then verify stock device boot. Simulated probes are insufficient.
2. Mac M1/Ventura: current Finder copy/approval/menu/install, option 3 export, app wording and complete uninstall. Separate conditional security termination from ordinary metadata/recovery errors.
3. Windows: latest metadata preflight failure is separate from the historical Defender detection. Verify the corrected package install/use/export/uninstall and stock boot without overriding security warnings. No Microsoft submission is part of this work.
4. Physical Linux reader/desktop checks remain pending. Host fault checks do not qualify devices.

No extra profiling, previews, audio/global-settings changes or integration-policy changes are part of this follow-up.

## Latest lifecycle correction (2026-10-05)

Follow-up installation passed on M1/Ventura and Intel/Monterey. Uninstall failed on both Macs due an older backup omitted from the latest journal. The latest Windows attempt failed verification on `._welcome-pending` before card writes; this is not evidence of a new Defender detection.

Read-only card inspection found a consumed welcome marker and a regular 4096-byte AppleDouble sidecar, SHA-256 `e6da019da34bf58d85311aaaa41b04ac222214846a474e00c0d7c20089906aed`. The shared bounded validator accepts its actual bytes. Its two entries are nonoverlapping and within the file. It was not removed to bypass preflight.

The oldest backup is authenticated by the indexed recovery chain, including the pending interrupted-install journal and four audited original MainUI hashes. The current old directory is empty; earlier originals remain in the preserved host evidence and verified journals. Cleanup now includes verified older backup ownership and checks every present byte; foreign contents remain protected. Update uses the same cleanup preflight before writes.

Only the exact app-consumed welcome lifecycle receives carried-forward evidence (`lifecycle` in format-1 recovery). It requires verified prior marker bytes; no arbitrary absent file or orphan sidecar is accepted. Present companion files still undergo ordinary validation. The app's marker creation/dismissal behavior is unchanged. Rotated logs remain explicit personal files; arbitrary transient names are not allowlisted.

Mac approval actions use the requested vertical R/S/0 choices, uppercase/lowercase, with separation from Details. Ordinary errors never enter this flow.

### Replacement Windows entry (current review)

The compiled x86 dispatcher was already present in RC2. It is now omitted from
new host builds, the user ZIP and transport manifests. Preserved older archives
are unchanged. A quarantined old dispatcher is never reconstructed: verified
recovery ownership permits its absent companion metadata to be archived/removed.
Present modified tools still fail verification.

`Install-Windows.cmd` uses one transparent literal built-in PowerShell command,
generated from `packaging/windows-select.ps1`. It queries Windows version and
native processor architecture through WMI; a 32-bit command shell uses native
PowerShell through Sysnative where available. Windows 7/8/8.1 select the existing
Go 1.20.14 native helper; newer supported Windows selects the modern helper.
Unknown/contradictory identities fail before the installer runs. No downloaded
code, execution-policy override, encoded command, compiler or compiled dispatcher
is used. The small batch entry and selected byte-verified native helper execute
from an invocation-private computer directory so complete uninstall can remove
the card app. One existing backend still handles all transactions and recovery.

The selector and staging/hash failure cases execute under Linux PowerShell in a
container; CMD flow is source-checked. This is **not native Windows 7/10 execution
or Defender acceptance**. No security-warning improvement is claimed, and the
native installer itself may still need investigation if detected. Do not override
malware warnings. No Microsoft submission or exclusions are part of this work.

### Short acceptance checklist

1. Mac: copy the new package normally; install, dismiss first-run guidance, update, then completely uninstall. No manual metadata deletion. Check stock boot and retained computer archive.
2. Retry a previously failed uninstall with its authenticated older journals intact. Unknown changes must fail before mutation.
3. Windows: test the corrected metadata path with the same Mac-copied card; install/update/uninstall and stock boot. Keep any security detection separate from an installer error.
4. Mac approval, if requested: column R/S/0 works; an ordinary failure retains the actual reason and exits with its status.

These are pending physical acceptance gates. Host fixtures, including interrupted publication/retry and portable journals, do not replace them.

Updates preserve existing preference files, including malformed saved data, without rewriting them. Their exact snapshots are recorded in the new recovery journal so complete uninstall can safely archive them. Unknown payload bytes remain protected.

## Current sequence and focused recovery correction

Latest user sequence supersedes the preceding snapshot: the app folder was
manually replaced, Mac install refused a missing Home receipt, Mac uninstall
restored nine files then stopped on changed Home-manifest metadata, a later
Windows installation succeeded, and Windows uninstall was blocked before its
helper ran. The current read-only inspection confirms installed MainUI/runtime
patches and a present matching receipt, with a newer indexed Windows journal.
The app hash matches `c765f8540678afb8a82cb797b0136d69c1efc0f7df25c8a6e1583b5caae473a7`.
This does not establish successful Windows uninstall.

Missing receipt reconstruction now requires agreeing indexed recovery lineage,
exact integration manifest, audited originals and installed MainUI/runtime hashes.
Existing mismatched receipt bytes remain a conflict. This is authenticated repair,
not a fresh installation. Deleting/re-copying app files does not erase system patches.

Uninstall replaces the Home manifest with its restored status before cleanup.
The old code then compared the sidecar against its pre-restoration bytes. The log
establishes this order, but cannot attribute the old metadata change definitively
to macOS versus another writer. Tests reproduce changed/generated AppleDouble
immediately after our manifest publication. Newly validated owned metadata is
archived before removal; actual payload and recovery records remain byte-strict.
Payload deletion precedes the final metadata refresh/removal, so automatic sidecar
disappearance is harmless. Further malformed metadata, links or foreign payload
changes fail with their actual path and reason. Host logs record companion hashes
at integration-manifest publication boundaries. Restoration and complete cleanup
are reported separately on failure; retries use the retained portable journal.

### Onboarding and narrow Settings audit

A genuinely fresh install creates `welcome-pending`; the app consumes it once.
Authenticated update/repair with a surviving recovery identity does not recreate
it. That explains its absence in this mixed recovery sequence. Complete uninstall
followed by fresh installation creates it again. These lifecycle cases are fixtures,
not new hardware acceptance.

Opening Settings, including returning from About with B, synchronously checks the
runtime and four 1,592,704-byte MainUI files in `homeEntryStatus`. Navigation within
an already open page has no deliberate delay or preference write. Changing a
preference performs durable writes; grouping/sorting also rebuild in-memory rows
and save browser position. Renderer fonts/badges are cached; ordinary text surfaces
are rendered and freed as before. No device timing proves which work dominates,
so no latency optimization or changed availability policy is included.

The only new app presentation change is spacing the About Automatic return tokens
as `[B] / [START]`, allowing both existing filled controls to render. Actual SDL
About-layout checks assert two START badges, measured bounds and single-page fit.
No preview images or extra profiling sessions are generated.

During the powered-off mounted-card recovery, lifecycle logging observed the Home
manifest sidecar `e6da019da34bf58d85311aaaa41b04ac222214846a474e00c0d7c20089906aed`
immediately before our manifest replacement and `absent` immediately afterward.
The pre-operation archive retains those bytes. This establishes that the restore
operation can change incidental sidecar state on this Mac/card; it does not prove
which component regenerated the different bytes in the earlier failed attempt.

## Superseding acceptance — 2026-10-05

The user now confirms M1 install/complete uninstall and Windows install/complete uninstall without security warnings using the built-in selector; app behavior passed. Intel install failed on a surviving receipt and succeeded after manual deletion. Historical failures above remain evidence, not the current normal-flow status. Final focused receipt and Settings corrections are tracked separately in [RC7](rc.7.md), with hardware confirmation pending.
