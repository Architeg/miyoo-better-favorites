# Download-and-run bootstrap preparation

This is **separate from the working offline installer**. No public bootstrap URL
is advertised: no eligible release/assets exist yet, the renamed package needs acceptance,
and native Windows online execution/TLS has not been qualified.

The Go downloader selects a published explicit tag or the latest stable release,
requires GitHub release immutability plus an annotated tag, resolves its commit, and verifies the one user ZIP
against SHA256SUMS from that same release. It checks safe ZIP members, expanded-size
limits, all package checksums and package.json's commit, then checks that the tag
has not moved. Only then does it invoke the existing host dispatcher/entry script inside `App/BetterFavorites/computer`. This is the advanced installer interface, not a second package or new card writer.
It implements **no SD-card restoration, patch or removal logic**.

Downloads and matching recovery stay under `~/BetterFavorites-Downloads` by default;
failed downloads/extraction are retained for investigation. Never choose a store
on the SD card. Installer prompts/arguments, terminal input and child exit status
are preserved. No sudo, watcher, daemon or global execution-policy change is used.
Uninstall requires an explicit installed release tag and the existing installer's
matching validated recovery; latest is not assumed compatible with old installs.

The shell and Windows PowerShell 2-compatible preparation stubs require an explicit
published tag and verify a separate BOOTSTRAP-SHA256SUMS before executing a native
bootstrap. Windows uses a Go1.20.14 x86 bootstrap, then the existing native installer
dispatcher. x86 execution support is required. TLS1.2/certificate availability on
Windows7 is an open native gate; failures tell users to use the extracted offline
package. The scripts do not weaken certificate verification. macOS/Linux require
curl and shasum or sha256sum for the first download; the native tool handles ZIPs.

## Host developer checks

```sh
(cd tools/bootstrap && go test ./...)
python3 tools/bootstrap/build.py --legacy-toolchain /path/to/go1.20.14 \
  --output build/bootstrap-fresh
sh -n tools/bootstrap/download-and-run.sh
```

Fixtures cover explicit/stable selection, unpublished/draft refusal, ambiguous
assets, checksums/digests, unsafe archives, source mismatch, tag changes, child
failure and retention. They use an isolated local HTTP server, never a mounted
card. Test-only HTTP acceptance exists only in internal Go options; public CLI
requires HTTPS. Cross-builds are not Windows/macOS/Linux online acceptance.

The published release must have GitHub immutable-release protection enabled.
Separate tag readbacks detect movement; they do not make an ordinary tag immutable.

Before offering a remote one-liner, qualify the actual published immutable release
assets, shell download stub, native Windows7/10 TLS and entry behavior, plus
install/uninstall with retained recovery. Until then README documents only the
offline copy-to-card installer. Its new full ZIP layout is not yet supported/qualified by the downloader, which still expects the earlier installer ZIP layout. Do not advertise a download-and-run URL until that separate adaptation and real route are qualified.
