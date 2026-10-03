<div align="center">
  <h1>Third-party notices</h1>
  <p><em>Credits, licenses and source provenance.</em></p>
</div>

<p align="center">
  <a href="LICENSE">Project license</a> ·
  <a href="#distributed-dependencies">Included dependencies</a> ·
  <a href="#supplied-by-the-users-onion-installation">Onion resources</a> ·
  <a href="docs/release/dependency-audit.md">Detailed audit</a>
</p>

Better Favorites project sources are **GPL-3.0-or-later**; see [LICENSE](LICENSE).
That license does not replace the licenses of SDL, Go or other dependencies.
Their copyright/license texts must be retained when their code is distributed.

No ROMs, BIOS, saves, user themes/fonts or original MainUI binaries are included.
Optional patches operate on the user's verified originals and keep private recovery
copies. The app's supplied icon is packaged separately from user theme resources.

<a id="distributed-dependencies"></a>
## Distributed dependencies

The candidate includes the private app libraries below and native host installers.
Archive versions identify the recorded **source**, not proven versions of every
prebuilt binary. The common fork pin is
`3c68ed01fee7feffd4ea338b1cc5018a455e2be9`.

| Component / upstream | Purpose | Recorded version or pin | License | Notice / source supplied |
| --- | --- | --- | --- | --- |
| [SDL2 Miyoo fork](https://github.com/Rparadise-Team/sdl2_miyoo_new) | Display, input, OSS audio | Common fork pin; SDL source header says 2.0.20 | Fork root LGPL-2.1; SDL core zlib; embedded components retain notices | `licenses/LICENSE`, `licenses/sdl2-LICENSE.txt`; dependency companion `sdl2/`, `mini/` |
| [SDL2_image](https://github.com/libsdl-org/SDL_image) | Artwork decoding | Source archive 2.8.1 at fork pin | zlib; codec components have separate terms | `licenses/SDL2_image-*LICENSE.txt`; companion `sdl2/dependency/SDL2_image-2.8.1.tar.gz` |
| [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer) | Navigation sound | Source archive 2.6.3 at fork pin | zlib; codec components have separate terms | `licenses/SDL2_mixer-*`; companion `sdl2/dependency/SDL2_mixer-2.6.3.tar.gz` |
| [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf) | Theme font rendering | Source archive 2.20.2 at fork pin | zlib; embedded font/shaping components have separate terms | `licenses/SDL2_ttf-*`; companion `sdl2/dependency/SDL2_ttf-2.20.2.tar.gz` |
| [json-c](https://github.com/json-c/json-c) | JSON parsing | Source archive 0.15 at fork pin | MIT-style notices, including upstream authors | `licenses/json-c-*COPYING`; companion `sdl2/dependency/json-c-0.15.tar.gz` |
| [SwiftShader](https://github.com/google/swiftshader) EGL/GLES | Graphics implementation libraries | Fork's `swiftshader/` tree; separate prebuilt revision unverified | Apache-2.0 at tree root; bundled components retain their own terms | `licenses/swiftshader-LICENSE.txt`, `licenses/swiftshader-AUTHORS.txt`; companion `swiftshader/` |
| [libpng](http://www.libpng.org/pub/png/libpng.html) | PNG support | Binary reports 1.6.37; exact build correspondence unresolved | libpng license family; exact artifact obligations unresolved | `licenses/libpng-1.6.37-LICENSE.txt`; pinned upstream source companion; correspondence remains open |
| [zlib](https://zlib.net/) | Compression support | Binary reports 1.2.11; exact build correspondence unresolved | zlib license family; artifact correspondence unresolved | `licenses/zlib-1.2.11-README.txt`; pinned upstream source companion; correspondence remains open |
| [Go runtime / standard library](https://go.dev/) — modern | Windows/macOS/Linux host tools | Go 1.26.2; no external Go modules | BSD-3-Clause | `licenses/Go-LICENSE`; repository notice `third_party/notices/Go-BSD.txt` |
| [Go runtime / standard library](https://go.dev/) — legacy | Windows 7/8/8.1 tools and x86 dispatcher | Official Go 1.20.14, isolated from normal module/toolchain | BSD-3-Clause | Same preserved Go notice; toolchain identity in `HOST-BUILDS.json` |

`licenses/` refers to the generated package directory. Wildcards above identify
its upstream notice families; [exact locations and audit notes](docs/release/dependency-audit.md)
explain the long filenames. Complete original text is retained, not replaced by
this summary. Project source and dependency source companions are listed in the
package's `SOURCE.txt`; keep those companions with binary distributions.

<a id="supplied-by-the-users-onion-installation"></a>
## Supplied by the user's Onion installation

| Component / upstream | Purpose | Known version or source | Licensing / distribution boundary |
| --- | --- | --- | --- |
| [Onion](https://github.com/OnionUI/Onion) runtime and GameSwitcher | Game execution, return, saves/resume and tracking | Audited v4.3.1-1 | Uses the user's installation; original runtime/GameSwitcher binaries are not bundled |
| Miyoo MainUI and firmware/driver libraries | Native menu/app dispatch and device services | Audited MainUI hashes; tested firmware 202306282128 | Vendor originals are not redistributed; per-card backups remain private |
| System C/C++ runtime, freetype, bzip2 and libpadsp | Existing loader/font/audio dependencies | Exact card-supplied versions not established | Not included in the app ZIP; their existing distribution terms remain applicable |
| Theme fonts, icons, artwork and sounds | Active-theme and Onion/Miyoo presentation/audio | User-selected installation | No user theme/font pack is bundled or relicensed by this project |

<a id="what-remains-unresolved"></a>
## What remains unresolved

The source companion preserves the pinned fork, extension archives and upstream
license files. **That is not proof of complete corresponding source for every
prebuilt.** The custom OSS SDL build needs fresh reproduction; extension/codec and
SwiftShader prebuilt correspondence needs tracing. libpng/zlib version strings and
upstream notices are now recorded; exact build/source correspondence remains open.

> **Publication status:** public binary redistribution remains gated by those specific
> provenance/license items. Device acceptance and installer success do not close this audit. See the
[developer dependency audit](docs/release/dependency-audit.md) and
[build provenance](docs/release/build.md) before distributing binaries. No required
license text has been removed or modified in this documentation rewrite.

## RC2 artifact investigation

The hashed libpng and zlib binaries contain version strings **1.6.37** and
**1.2.11** respectively. Their original upstream license/README notices and pinned
source companions are now preserved. This closes the missing-notice/version-evidence
items, not proof of exact compiler/source correspondence. SwiftShader reports
4.1.0.7; its precise prebuilt source revision remains unverified.
