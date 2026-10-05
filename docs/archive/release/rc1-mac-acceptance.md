> **Historical engineering record — not current installation instructions.**
> Use the [installation guide](../../install.md) and [compatibility matrix](../../compatibility.md) for the released app.

# RC1 Mac/device acceptance — recorded 2026-10-04

User confirmed all six packaged-install device checks passed using a **MacBook Air
M1, macOS Ventura 13.7.8** and the existing Mini Plus MY354, firmware 202306282128,
Onion v4.3.1-1 (hardware revision unknown).

The supplied `diagnostics-01.zip` is CRC-valid. Its report matches source checkpoint
`7e8df0375c5de6e969e59bb712210b11638ba3ea` and the actual RC1 package:

- App: `307573e27cdb45c5360f2a7bfe2095a4bede879c8ce003aa5c0d985cd3c02205`.
- Launcher: `f103f5ce8be4872590c3bc122b4d62a19726529d44c5a8f67d0416cc52e3c494`.
- Return helper: `6d8cc4789bad0811ccea1d59e5a859981dc61e499a0e947c385d97e7e5972381`.
- Four patched MainUI hashes match the exact integration catalogue; runtime matches
  `4e7fdcb04dd53eefbc54a5244e35dbd746681ac77b8fe55e574c3e7227744df1`.

The recovery manifest and all before/after/stock copies pass their recorded hashes.
The diagnostics captured a patched installation; they do not prove later uninstall.
Detailed Home/return logs are missing, as expected with tracing disabled. The user
confirmation is hardware evidence; log absence is not proof of every interaction.
Keep the untouched Desktop diagnostics/recovery originals outside Git.

After the user's RC1 uninstall, mounted-card inspection independently found all
four MainUI files and runtime byte-equal to this recovery's verified stock originals,
with the active return helper and app Home receipt absent. **Post-uninstall device
boot/stock behavior verification remains pending.** RC1 uninstall retained the app,
preferences and installation artifacts; it was an integrations-only operation.
A subsequent explicitly authorized clean-baseline operation archived/verified and
removed 66 identified project files, retaining all host originals. Live favorites,
history and metadata for 18,758 unrelated files stayed unchanged. The exact path
report and hashes are in the user's private host archive; no private data is shipped.

RC2 corrects default uninstall to complete removal. RC1 acceptance does not establish
hardware/Windows acceptance of RC2's changed app, icon or uninstall behavior.
