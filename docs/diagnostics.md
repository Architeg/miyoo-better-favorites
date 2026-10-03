# Bounded diagnostics

Basic logs: `App/BetterFavoritesTest/better-favorites.log` and
`better-favorites.previous.log`, each at most **65,536 bytes**, total **128 KiB**.
The launcher rotates once at entry; the binary captures app-owned startup/error
lines and cleanup/handoff events. No per-frame or normal navigation logging, log
watcher, polling daemon or gameplay process is added. Short-lived binary log modes
finish before handoff. A full/unwritable/nonregular log is best-effort and never
changes launch/fallback/return results. Old oversized logs are bounded at rotation;
foreign links/FIFOs are refused. Concurrent app invocations are unsupported.

The release adds a few boundary-only executable invocations for logging; their
startup/timing cost is unmeasured. SD writes can affect timing. Do not compare the
old profiling pilots as if instrumentation were unchanged. No further measurements
are requested for this release pass. Native library stdout/stderr is discarded;
app-owned C++ error streams are bounded. Loader failures before main are visible as
launcher exit results, not guaranteed complete native error paragraphs.

Use the host tool **Export diagnostics** with the card mounted. It does not enable
tracing or alter preferences. A fresh ZIP contains bounded log tails, allowlisted
app settings/status, package/deployed version/commit, recognized MainUI hashes,
runtime/helper hashes, active-theme identifier and explicit missing evidence.
Unknown model/firmware/revision stay unknown. No favorites/history contents,
credentials, serials or host personal paths are collected. Logs can contain game
filenames in errors; inspect before sharing. Offline exports do not measure RAM.

Support-only detailed Home and return traces each cap new writes at 128 KiB and share the opt-in marker; both are OFF normally in the candidate. Existing oversized legacy logs are retained without growth; export takes bounded tails. The accepted deployed helper still has its old logging until a separately qualified candidate installation.
With the self-contained executable for your host, powered-off card mounted:

```text
better-favorites-installer-<host> trace-on --sd-root <card> --powered-off
better-favorites-installer-<host> export-diagnostics --sd-root <card> --output <new.zip>
better-favorites-installer-<host> trace-off --sd-root <card> --powered-off
```

The actual filenames are in the quick-start platform table/package. Enable/disable
only owns the exact marker. Disabling preserves raw logs and exports. Tracing is
independent of Home and Automatic return preferences. Attempt IDs are evidence,
not command/session ownership. No Miyoo Terminal commands are required.
