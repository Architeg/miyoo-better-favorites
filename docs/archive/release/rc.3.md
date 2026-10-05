> **Historical engineering record — not current installation instructions.**
> Use the [installation guide](../../install.md) and [compatibility matrix](../../compatibility.md) for the released app.

# RC3 copy-and-click preparation

> Historical engineering record. Use [current installation instructions](../../install.md) and [release notes](../../release/notes-rc.7.md).

RC2 remains committed/tagged at `a31f9a2fff42d12b2a9f2cc16118f3b8a43eb94a`, with an unpublished asset-free draft. New RC3 packages have their own source commit and checksums; earlier device acceptance does not identify these bytes.

## Implementation

- Full ZIP: App/BetterFavoritesTest contains the app, three computer entry routes and verified computer transport. App-only ZIP contains no installer/integration payload.
- Card layout and Onion identity are validated before writes. No drive scan/path entry in the normal menu.
- Normal menu has Install / Update, Uninstall completely and Cancel. Advanced support actions stay explicit.
- Both supported integrations are installed without enabling their fresh OFF switches. Unknown modified audited files fail; genuine unsupported Onion versions may explicitly choose app-only.
- Native execution is staged privately off-card; parent waits and cleans its owned stage after the child exits. Hard termination may leave a private host stage; it is not recovery and no global cleanup is run.
- Active card recovery is identified by relative journal path/hash. A pending pointer is durable before system publication; interrupted attempts remain in the verified update lineage. Only that lineage is eligible for cleanup; unrelated valid recovery directories cause refusal and are preserved. Updates retain stock originals. Computer archives are extra protection.
- Complete uninstall verifies restoration, archives evidence, removes owned app/tools, then removes portable recovery. Conflicts and interruptions retain retry evidence and never report complete success.
- Fresh installation alone adds a once-only guidance marker. Update preserves preferences and does not recreate a dismissed notice.

## Focused host evidence versus pending execution

Fixtures cover layout/Unicode/overlap, copied inputs, package integrity, updated ownership, portable recovery after relocation, failures/interruption/retry and full folder removal. Native macOS shell-entry full-ZIP and Linux-container executions are recorded separately. GLib desktop command parsing uses a real parser with a non-terminal fixture; it does not establish desktop terminal opening. Simulated OS dispatch and cross-builds are not Explorer/Finder/Linux-desktop or Miyoo hardware acceptance. No broad OS/device matrix is required for this pass.

## One final-package fresh-user cycle

1. Record package SHA256SUMS and source commit; begin from a verified stock card.
2. Copy full app folder and click Install / Update. Confirm both integrations available and both switches OFF, with one-time guidance.
3. Apps opens the browser; Home OFF opens stock Favorites. Enable Home replacement and verify Home opens Better Favorites and B returns without a loop.
4. Launch one game, MENU/GameSwitcher and enable Automatic return; verify B/START returns, A resumes and saved position remains.
5. Power off, remount, click Uninstall completely. Verify success and stock Onion boot. Games/data must remain.
6. Reinstall from the same full ZIP. Verify normal Apps access and fresh OFF switches.

No Miyoo Terminal, extra profiling or preview batch. [Concrete dependency questions](../../release/dependency-audit.md) affect binary publication; exact-byte reproduction alone is not a license requirement. Prepare stable identity/assets only after this cycle and genuine publication requirements are closed. No release publication or card write during preparation.
