# Historical download-bootstrap prototype

The current Mac/Linux entry is [`scripts/install-online.sh`](../../scripts/install-online.sh).
It downloads the same full user ZIP and calls the existing installer without a
separate bootstrap executable. [User instructions](../../docs/online-install.md).
Windows uses the accepted offline PowerShell selector.

The Go downloader and older shell/PowerShell files in this directory are retained
as research/test code. They expect an earlier package layout, separate bootstrap
assets and immutable releases. They are **not** an installation option and must
not be advertised as working with the current user ZIP.

Developer checks:

```sh
(cd tools/bootstrap && go test ./...)
```

Its fixtures cover release selection, checksums, archive validation, source/tag
identity and child failures using a local server. This does not qualify a native
online installation route. For work on the shipping entry instead, run:

```sh
bash -n scripts/install-online.sh
python3 tests/online_install_test.py -v
```

Keep all SD writes, patch publication, portable recovery and removal in the shared
`tools/release-installer` backend. Do not introduce a second implementation.
