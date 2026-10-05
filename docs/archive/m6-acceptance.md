> **Historical engineering record — not current installation instructions.**
> Use the [installation guide](../install.md) and [compatibility matrix](../compatibility.md) for the released app.

# M6 hardware acceptance and compatibility

Recorded 2026-10-03. User reports all four device checks passed on **Miyoo Mini
Plus**, Onion **v4.3.1-1**. Model code **MY354** identifies the model/platform, not the hardware revision.
Miyoo firmware **202306282128** is user supplied; hardware revision remains
**unknown**. Onion version is a separate value.

Designed for Mini and Mini Plus; hardware tested on Mini Plus. Mini and other
combinations need community testing. No firmware update is requested.

1. OFF: Home Favorites opens stock Favorites.
2. ON: Home Favorites opens Better Favorites; B restores Home without a loop,
   including two repeated entries/exits.
3. Apps access and ordinary X/Y still work.
4. A launch, MENU/GameSwitcher and Automatic return still work.

These are user hardware observations. The app feeling fast is subjective, not a
new benchmark. Uninstall, crash recovery, missing-app/fault combinations and other
devices have not been hardware-qualified by these checks. New wording/status and
host recovery changes made **after** collection await device acceptance and have
not been deployed. This acceptance also covers ordinary browsing/game behavior of
the cache-containing deployed build, without measuring cache speedup.

## Tested bytes

Current card bytes matched the first deployment manifest and all originals remain
backed up. MainUI logging identifies the executed variant as `MainUI-354-clean`;
installing all four variants does not mean all four were executed on hardware.

| Deployed file | SHA-256 |
| --- | --- |
| `.tmp_update/bin/MainUI-283-clean` | `a8a9771d02fbc7d13d7be2edf6cebe0d7b806c5b2a25256753bd8ed16f02b08a` |
| `.tmp_update/bin/MainUI-283-expert` | `e05e855c94a9e7bea702e552672297624b42b7e338df57c0b8bdd240a7978d12` |
| `.tmp_update/bin/MainUI-354-clean` | `4a96ca03acd4f9a4b1ffd23bcb1586d0ec509b71b3006e5322bd717922e830f4` |
| `.tmp_update/bin/MainUI-354-expert` | `fb094eb2300562642b7f43436e441b9233694f5263f870b974086e88e3db8c75` |
| `App/BetterFavoritesTest/better-favorites` | `d637643b0be987ab68a7bb9ba6869f09b6e1e58e2e7d0919088bbb1cd54f19dc` |
| `App/BetterFavoritesTest/launch.sh` | `4173bca55adaab4f491da2c4a07856886b783f6a408d897135a6858dc4c26419` |

Original runtime SHA-256: `a8d77dcd316bc2a323b1e015aaf4b7682d2fed677af9cdadbc00e48881425d6e`.
Installed return runtime SHA-256: `4e7fdcb04dd53eefbc54a5244e35dbd746681ac77b8fe55e574c3e7227744df1`.
Return helper SHA-256: `95cf66de2fffcb4311a572a76b2a62929abfe0b76fe598be459231dc76b02586`.
Exact original MainUI allowlist is in [prototype evidence](m6-mainui-prototype.md)
and `integration/mainui-home/package.json`. Availability checks and managers must
use the matching package, not offsets from another version.

## Collected evidence and limits

Fresh host archive: `../miyoo-better-favorites-backups/m6-accepted-20261003-204554/`.
Both log copies were byte-verified against the card **before** disabling diagnostics.
`collection.json` records bytes/hashes; `acceptance.json` links the user checks to
current deployed files. Original deployment manifest/rollback instructions were
copied and verified too. Raw card logs and every backup were preserved.

| Raw log | Bytes | SHA-256 |
| --- | ---: | --- |
| `home-diagnostics.log` | 2665 | `054e0cdc32f203ae02f178fa4b9049893c665b8ca98a09e34cda8c96bf53d368` |
| `better-favorites.log` | 5569 | `8977ca013cf5be79d63d21774d53675fb212cee9744a6bffdc4dbb52ea350764` |

- Native log: six MainUI startups, one OFF-or-malformed Home fallback (the user's
  OFF check establishes OFF), three ON attempts with staging ready, publication
  committed, native result 3 and redirect published. Subsequent startups observed.
- Rotating launcher log retains only the last attempt
  `00000cb1.687c67a4.00092e8f`: entry, binary exit 0, no game handoff, own private
  request/directory cleanup. `candidate_attempt` is correlation, not ownership.
- Earlier launcher exits, the A/MENU/return session, Apps and X/Y checks are not
  individually evidenced in retained logs. Their acceptance comes from the user.
  Native logs do not identify which physical button exited the browser.
- The device clock dates these entries in July 2025; do not infer trial dates or
  elapsed performance from that wall clock. Collection occurred on 2026-10-03.
- Diagnostic marker is absent after disable and sync. No further run was requested
  to measure absence of future writes. Profiling remains disabled; no RAM or
  gameplay process-absence measurement was made during this trial.

## Compatibility matrix

| Target | Base app | Session return | Home redirect | L1+Y |
| --- | --- | --- | --- | --- |
| Tested Mini Plus/card, v4.3.1-1 | User hardware accepted | User hardware accepted | User hardware accepted, 354-clean | Unimplemented |
| Other Mini/Mini Plus, exact audited files | Candidate; device qualification needed | Eligible only at exact runtime/helper hashes | Eligible only at all four exact binary hashes; not hardware accepted | Unverified |
| Other Onion versions/forks | Unestablished | Hash/version refusal | Hash/version refusal | Unverified |
| Miyoo Mini Flip / modified distributions | Unverified | Separate audit required | Do not apply this package | Unverified |
| Miyoo Flip / Flip V2 | Not supported by this package | Separate port required | Mini offsets must not be applied | Unverified |
| Stock OS / other frontend | Unestablished | Onion-dependent | Onion-dependent | Unverified |

Mini Flip and Flip/Flip V2 are distinct targets, not aliases. Official Onion 4.3
installation documentation covers Mini and Mini Plus; follow its device-specific
firmware requirements rather than recommending a change just for this app.
[Onion installation requirements](https://onionui.github.io/docs/installation).

Base app, optional runtime return, optional Home binary integration and any future
shortcut have separate compatibility and activation. Apps remains usable without
Home replacement. Home OFF is independent of Automatic return. Multiple entry
routes do not repair an incompatible binary or shared launch protocol.

See [installation, uninstall and offline recovery](m6-home-integration.md), and
[separate L1+Y investigation](m6-shortcut-investigation.md).

Official [latest Onion release](https://github.com/OnionUI/Onion/releases/latest) resolved to v4.3.1-1 on 2026-10-03; forks/prereleases are separate.
