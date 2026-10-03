# Build and package provenance

Project license: GPL-3.0-or-later. Go host tooling has no external modules; the Go
runtime has its own BSD notice. Native user installation needs no Go/Python/WSL.

## Prepared checkout

1. Prepare pinned headers/source via `sh scripts/fetch-deps.sh` (networked developer
   step), plus the SDL_mixer extraction in [CONTRIBUTING](../../CONTRIBUTING.md), pin SDL fork `3c68ed01fee7feffd4ea338b1cc5018a455e2be9`.
2. Preserve the qualified OSS SDL library at
   `third_party/sdl2_miyoo/custom/libSDL2-2.0.so.0`. It must match the recorded
   working dependency hash; do not silently substitute the upstream mini-audio build.
3. Commit reviewed source; run `sh scripts/build.sh` with the existing Docker image
   pinned by digest `sha256:a864876472a489f63d6223d2c8ad61e12ced679c0b177ae9429e51f3673ef4e7`.
   This builds the ARM executable with version/source literals and audited adapter.
4. `sh scripts/package.sh --output dist/<fresh-directory> --legacy-toolchain <Go1.20.14-root>` requires a clean pinned
   checkout. It pins Go1.26.2 for modern Windows x86/x64/ARM64, macOS ARM64/x64 and Linux
   ARM64/x64, plus explicit isolated Go1.20.14 for legacy Windows x86/x64 and the
   read-only x86 dispatcher. It verifies Linux static linkage and Monterey Mach-O minima. It writes app-only/installer ZIPs, source companions, notices,
   dependency inventory and SHA256SUMS. Reusing an output directory is refused.
5. Extract and test these ZIPs with `BF_RELEASE_PACKAGE` set to extracted installer
   root and `BF_FIXTURE_REPO` to the prepared audited private fixture checkout.

ZIP timestamps/order and source compression timestamps are pinned to source commit.
Go uses trimpath/buildvcs=false. App/compiler binaries can differ across toolchain
versions; byte-reproducibility must be checked explicitly, not assumed from this
procedure. The ARM build still reports GCC ABI notes and the existing SDL_ttf
libbz2 linker warning; runtime dependency presence is separate from linking.

## Working custom SDL source/configuration

The tracked pinned fork source was unchanged. The existing build configuration is:
`--host=arm-linux-gnueabihf --enable-oss --disable-alsa --disable-audio-mini
--enable-video-mini --disable-jack --disable-pulseaudio --disable-pipewire
--disable-sndio --disable-diskaudio --disable-dummyaudio --disable-video-opengl
--disable-video-opengles --disable-video-opengles2 --disable-video-x11
--disable-video-kmsdrm --disable-video-vulkan --disable-video-wayland
--disable-video-dummy --disable-hidapi --disable-libudev --disable-dbus
--disable-fcitx --disable-ime --disable-joystick-virtual --disable-power`.
Use the pinned compiler and mini/SwiftShader libraries in an isolated dependency
build. A fresh custom SDL reproduction and exact prebuilt correspondence audit
remain release gates; prepared-checkout build success is not clean-source proof.
Do not alter audio/global system configuration to silence warnings.

The package includes SDL/image/mixer/ttf/json-c/png/z and SwiftShader EGL/GLES
private libraries. Firmware/Onion supplies libc/libstdc++/libgcc, MI driver libraries,
freetype, bzip2 and libpadsp. Inherited runtime search paths are retained. Packaged normal flows have user evidence; separate app-only fresh closure and
all new package variants still need exact-package qualification.

### Canonical adapter ELF metadata

The accepted raw adapter ELF included its link input pathname in an unmapped
STT_FILE string. `hooks.S` now supplies an explicit filename directive so raw
ELF output is independent of the output directory. Canonical payload SHA-256:
`c8110095a3da0637ffd21788968e9c2f7b3a6637d57d7f333c1acacefd908146`.
The accepted raw ELF (`1623d143…`) remains preserved. All loadable code/data/unwind
sections and all four generated MainUI SHA-256 values are **byte-identical** to
the accepted deployment. This is metadata reproducibility, not a new redirect
behavior or a replacement of accepted card files.

## RC2 private review snapshots / Windows 7

Normal release packaging still requires a clean pinned checkout. An explicit
`--review-snapshot` prepares uncommitted private review artifacts with base commit
and exact source SHA inventory; its source companion contains those actual files.
This does not pretend to be a new committed checkpoint. Default candidate revision
is rc.2; BETTER_FAVORITES_RELEASE_VERSION can explicitly select another build ID.

`tools/build-windows7-test.py --toolchain <isolated official Go1.20.14> --output
<fresh-directory>` copies installer Go sources into a temporary test-only module
(go 1.20), executes host tests and cross-builds x64/x86. The official archive checksum
is verified before use. Normal go.mod stays go 1.24 and normal tools use Go1.26.2.
Supply BF_FIXTURE_REPO/BF_RELEASE_PACKAGE to include exact-package host tests.
Native Windows7 SP1 x64 and Windows10 x64 normal-flow acceptance is user-confirmed;
this Mac cannot execute Windows binaries itself. The export does not identify the
exact later host dispatcher or ZIP. [Windows evidence](windows-acceptance.md).

## Host-dispatch review extension

See [target matrix/dispatch/evidence](host-dispatch.md). Production packaging requires
an explicit legacy toolchain path; it never silently downgrades normal go.mod or Go.
Build a fresh review directory; preserve prior RC1/RC2 archives and inventories.
Run `python3 tests/host_dispatch_test.py`, Go1.20.14 `go test ./...` in
`tools/host-dispatch`, installer regressions and actual ZIP roundtrips. Native Windows
probes/wrappers are not executed by simulated tests. No device writes are needed.

`python3 tools/package-windows7-test.py --release <fresh-release-dir> --output
<fresh-path>/BetterFavorites-Windows7-Test.zip` creates the offline test archive and
sidecar checksum; existing output is refused. Preserve the previous Desktop archive
before replacing its path. Both packages have the exact same payload/package.json.

The detailed [dependency/source/notice audit](dependency-audit.md) separates known
source versions from unidentified prebuilt correspondence. Contributor compilation
and ordinary tests do not require a private card/firmware fixture.
