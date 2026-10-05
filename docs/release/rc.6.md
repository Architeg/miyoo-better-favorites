# RC6 — directory metadata and Mac first-open correction

Local follow-up review candidate; no stable release or follow-up device acceptance. Initial RC6 M1/Ventura acceptance is recorded below. Existing tags, release archives, rollback backups and mounted-card files are preserved.

## Actual RC5 result

The user reported installation stopped at `App/BetterFavorites/._computer`. Read-only inspection on 2026-10-05 confirmed a regular 4096-byte AppleDouble v2 file, SHA-256 `f94e1302769591070cabb893bdb00bd29680dbb49f04fe731541d2965ebfad49`. Its two entries are bounded within the file. The companion `computer` is an installer directory. RC5 incorrectly required a regular-file companion. RC5 installation is **not accepted**. Earlier successful device checks do not qualify RC6.

## Focused changes

- Directory ownership comes from package inventories, verified recovery records or verified descendant files. AppleDouble validation remains bounded; paths are checked without following symlinks. Unrelated directories and malformed metadata remain protected.
- The rule applies to copied input, update, legacy migration, recovery loading, interrupted cleanup and complete uninstall on another computer. Metadata is archived as data, never executed or merged into payload files.
- Installer messages distinguish preflight, preparation, verified publications and rollback outcomes. Bounded, best-effort host logs retain detailed errors; logging failure cannot change the transaction result. Terminal output uses normal foreground and no ANSI requirement.
- The simple menu offers Install / Update, Uninstall completely, Export diagnostics and Cancel. Status 1 remains an ordinary failure. Possible SIGKILL remains conditional guidance, with stable byte-verified helper retry and an optional Open Settings action.
- `Mac-first-open.html` is available at ZIP root and inside the app before the blocked script is launched. It explains separate script/helper approvals and Monterey versus Ventura-and-later settings.

The initial RC6 candidate retained the exact RC5 ARM app. The follow-up rebuilds only the requested onboarding wording and removes redundant positive availability messages; app behavior, launcher, libraries and integration payloads remain unchanged. Separate app/package provenance and source-snapshot hashes remain recorded. See [RC6 follow-up](rc6-followup.md) for the latest outcome and entry-workflow comparison.

## Signing feasibility

Read-only `security find-identity -v -p codesigning` returned **0 valid identities** on this Mac. `codesign` and `notarytool` are available; no repository notarization workflow was found. No credentials were read or exported, and keychain credential profiles were not enumerated. Tool availability does not establish a usable notarization account. Signing cannot be completed with the currently available identity.

With an authorized Developer ID identity and notarization setup, sign both native Mac helper architectures and the complete chosen entry distribution, notarize that distribution, then test a quarantined download through Finder. Adding an `.app` wrapper alone would not remove trust warnings. This candidate remains unsigned by a trusted publisher.

## Verification and remaining gate

The accompanying preparation report records exact executed host checks and artifact hashes. Real Mac-generated directory metadata is used in regression fixtures; the private card attribute values are not distributed. Simulated dispatch and host fixture roundtrips do not verify Finder approval or device operation.

Short Mac acceptance checklist:

1. Read `Mac-first-open.html` before launching. Test the two file-specific approvals if macOS requests them; stop for malware/damaged/policy warnings.
2. Copy the complete folder normally with Finder, retaining generated metadata. Install and verify `._computer` no longer blocks; confirm an ordinary failure does not trigger security retry.
3. Check Apps, Home OFF/ON, game launch and return on the Miyoo. Power off and remount.
4. Uninstall completely; confirm stock boot and retained computer recovery archive. Report the detailed log if any step fails.

## User-confirmed RC6 outcome

Mac M1/Ventura installation, complete uninstall and app use passed per the user. Intel Monterey failed host detection. Windows installation was interrupted with a Defender alert. These results are separate from earlier RC1/RC2 acceptance and from Linux-container fixtures. The follow-up has not yet passed those device/host acceptance gates.

## Latest follow-up evidence

Installation passed on M1/Ventura and Intel/Monterey per the user. Complete uninstall failed on both due an older unrecorded backup. The latest Windows attempt failed welcome-marker metadata preflight before any card writes, separately from earlier Defender evidence. The lifecycle/recovery correction is host-tested; its final packaged uninstall and stock boot remain pending. See [focused follow-up](rc6-followup.md).

## Superseding acceptance — 2026-10-05

The user now confirms M1 install/complete uninstall and Windows install/complete uninstall without security warnings using the built-in selector; app behavior passed. Intel install failed on a surviving receipt and succeeded after manual deletion. Historical failures above remain evidence, not the current normal-flow status. Final focused receipt and Settings corrections are tracked separately in [RC7](rc.7.md), with hardware confirmation pending.
