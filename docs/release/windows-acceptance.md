# Windows packaged-install acceptance

The user confirmed successful installation/use/uninstallation on **Windows 7 SP1
x64** and **Windows 10 x64**, with expected behavior matching the Mac test and
successful **stock Onion boot after uninstall**. Windows 10 build, shell bitness,
computer hardware and SD-reader/filesystem details were not supplied.

Device details from earlier user confirmation: Miyoo Mini Plus MY354, firmware
202306282128, Onion v4.3.1-1, hardware revision unknown. MY354 is a model code.
The diagnostic exporter itself reports device/firmware/revision unknown; those
fields must not be claimed as established from offline logs.

## Verified diagnostic evidence

The supplied archive is CRC-valid and preserved untouched; its SHA-256 is
`732f3bd2b26b13141cad6ef244a9c08920fee2e38721c7c9a8437c7ddda1ebed`.
A second byte-verified host archive retains it and member checksums outside Git.
No raw logs or personal preferences are included in this document/package.

| Item | Verified SHA-256 / result |
| --- | --- |
| App | `44a57e3473d17c8a1fa8878c6f9f393855322df9ab9cc62033cfafe40c608019` |
| Launcher | `f103f5ce8be4872590c3bc122b4d62a19726529d44c5a8f67d0416cc52e3c494` |
| Return helper | `6d8cc4789bad0811ccea1d59e5a859981dc61e499a0e947c385d97e7e5972381` |
| Patched runtime | `4e7fdcb04dd53eefbc54a5244e35dbd746681ac77b8fe55e574c3e7227744df1` |
| MainUI | All four reported patched hashes equal the audited catalogue |
| Payload identity | RC2, base `7e8df0375c5de6e969e59bb712210b11638ba3ea`, snapshot `74b290c4766b29c7e35cf7a049c2550f134bd645b807f6c6711dd2468f9fd6ce` |

The exported release.json is byte-identical to the earlier RC2 complete package.
It differs from the later host-compatible candidate's snapshot
`c0c9de8f4e2132b602174f27d2df852d1818b2c3e692e139cc13d4e48c5d2a3d`.
The app/launcher/integration bytes are the same across those revisions, but the
report does **not** identify a tested host ZIP checksum or executable/dispatcher
hash. Therefore this does not prove execution of every later dispatch branch.

The two logs establish successful audio initialization, normal SDL/audio cleanup,
exit0 cancellation cleanup, and exit21 GameSwitcher publication with history
unchanged. They do not include an exit20 game launch or detailed Home decision.
Missing Home/return trace files are reported honestly; tracing is OFF normally.
User-confirmed behavior is separate evidence from those specific log boundaries.

## Mounted-card verification after uninstall

Read-only checks found all five files byte-equal to this card's previously verified
recovery originals. No restoration or card modification was performed during this review.

| Stock file | SHA-256 |
| --- | --- |
| MainUI-283-clean | `6b01276a6292fd7061e0b2576322a52ada65b755562f97bf7656b174d475866f` |
| MainUI-283-expert | `6948b5310dda6513b9e8fa2519d90c5287fc1fd06b4f668a7f2f205406281d28` |
| MainUI-354-clean | `98c85f6c573bdeabd3762e8d9b596f354014e666cc873d0f758cbf3752620c94` |
| MainUI-354-expert | `3bd1fef7fd9bd215bb9e335b6be1101fdff510590ba0d9ca0a9707edc5d9718a` |
| runtime.sh | `a8d77dcd316bc2a323b1e015aaf4b7682d2fed677af9cdadbc00e48881425d6e` |

The app directory, active return helper and Home installation receipt are absent;
no project-named entries remained in the inspected App/config/script/logs/Roms
locations. This is independent confirmation of current cleanup, not proof of every
host-side archive or interrupted/fault case. The successful post-uninstall stock
boot is user-confirmed, not inferred from hashes.

Windows x86, ARM64, 8/8.1, other readers, exact later dispatcher binaries and fault
matrices remain unqualified. [Host targets/evidence](host-dispatch.md) and
[remaining release gates](rc.2.md) preserve those distinctions.
