# Dependency provenance and reproduction audit

This page records release work, not extra user installation requirements.
[Third-party notices](../../THIRD_PARTY_NOTICES.md) are the readable attribution
summary. [Build instructions](build.md) record the actual compiler/configuration.

## What packaging currently distributes

`tools/package-release.py` copies nine app-private libraries, recorded in
[dependency-hashes.json](dependency-hashes.json), plus the ARM app and host tools.
It verifies those expected hashes; it does not manufacture evidence of their source
correspondence. No original MainUI/runtime, ROM, save or user theme is copied.

The SDL fork pin is `3c68ed01fee7feffd4ea338b1cc5018a455e2be9`.
Its SDL header declares 2.0.20; that alone does not identify a compiled ELF.
The preserved OSS SDL library is separate from the upstream mini-audio prebuilt.
A fresh checkout can compile the app against pinned headers/prebuilts, but release
packaging requires the exact qualified custom OSS file. No fresh recreation claim
is made until source/configuration/toolchain output is reproduced and checked.

## Notice locations in generated packages

| Recorded source | Generated notice |
| --- | --- |
| Fork root LICENSE | `licenses/LICENSE` (LGPL-2.1 text; distinct from project root LICENSE) |
| SDL core | `licenses/sdl2-LICENSE.txt` |
| SDL2_image 2.8.1 | `licenses/SDL2_image-2.8.1.tar-SDL2_image-2.8.1-LICENSE.txt` |
| SDL2_mixer 2.6.3 | `licenses/SDL2_mixer-2.6.3.tar-SDL2_mixer-2.6.3-LICENSE.txt` and extracted codec notices |
| SDL2_ttf 2.20.2 | `licenses/SDL2_ttf-2.20.2.tar-SDL2_ttf-2.20.2-LICENSE.txt` and extracted shaping notices |
| json-c 0.15 | `licenses/json-c-0.15.tar-json-c-0.15-COPYING` |
| SwiftShader | `licenses/swiftshader-LICENSE.txt`, `licenses/swiftshader-AUTHORS.txt` |
| Modern and legacy Go | `licenses/Go-LICENSE`, copied from preserved repository BSD text |

The packager extracts files named LICENSE, LICENSE.txt or COPYING from the four
extension/json archives. That filename filter is not a complete transitive audit;
other notice names/components must be reviewed. Required originals remain intact.

## Open correspondence and licensing items

| Item | Evidence available | Missing work / release condition |
| --- | --- | --- |
| Custom OSS SDL | Accepted library hash, pinned fork, recorded configure switches/toolchain | Reproduce from clean source and establish exact binary correspondence |
| SDL extensions/json-c | Versioned source archives and expected prebuilt hashes | Trace build configuration, linked/static codec components and matching source/notices |
| SwiftShader EGL/GLES | Full fork tree and root Apache/AUTHORS texts, expected ELF hashes | Establish prebuilt revision/configuration and component-specific license/source closure |
| libpng16/zlib | Library hashes, embedded 1.6.37/1.2.11 strings, pinned source and original notices | Establish exact build/source correspondence and remaining artifact obligations |
| Go host runtime | Go1.26.2 modern / official Go1.20.14 legacy, no external modules, BSD text | Keep exact toolchain/build identities and corresponding notice; legacy runtime security review |
| Firmware-provided libraries | Existing Onion/Miyoo runtime dependency paths | Document runtime closure; do not bundle unidentified firmware libraries |

The companion `sdl2-miyoo-<pin>.tar.gz` contains the fork LICENSE/Makefiles, SDL,
mini and SwiftShader trees, including extension source archives. Supplying it does
not automatically close any row above. Project source companions correspond to
package source inventories; verified device behavior is a different question.

## Tools and verification boundaries

Docker's pinned Miyoo compiler image is a developer build dependency, not a user
installation dependency or a bundled image. Keep its identity, source/runtime terms
and compiler/linker notes in [build provenance](build.md). Existing GCC ABI notes
and the bzip2 link warning do not establish runtime availability; inspect ELF needs
and verify card-supplied dependencies separately.

Go1.26.2 builds modern Windows/macOS/Linux installers. Go1.20.14 builds only legacy
Windows installers and the read-only Windows dispatcher, in an isolated module;
normal go.mod remains go1.24. [Host dispatch](host-dispatch.md) records minimum OSes,
static Linux headers and Mach-O load minima. The full toolchain is not shipped;
compiled executables include licensed runtime/standard-library code.

Do not publish binaries as license-cleared until the exact source/prebuilt and
transitive notice issues are closed. Preserve all license/source companions and
original copyright texts during package/documentation updates.

## RC2 follow-up evidence (2026-10-04)

Binary strings from the hash-qualified libpng report 1.6.37; zlib reports 1.2.11.
SwiftShader EGL/GLES reports 4.1.0.7. These identify embedded version strings, not
a reconstructed build. [Pinned upstream source metadata](upstream-sources.json)
records full commit/archive/notice hashes for libpng and zlib. The packager checks
and supplies both archives and their original LICENSE/README notices. Working
shared libraries are unchanged. Earlier unidentified-version and missing dedicated
notice findings are narrowed by this evidence; exact build correspondence remains open.

The clean SDL source archive needs autogen before configure; the pinned compiler
image lacks autoconf. Host autogen succeeds. The isolated pinned-toolchain build, with verified revision
and SDL_image headers and recorded CFLAGS, fails at the link step on MI_GFX/MI_SYS
symbols. The recorded configure flags therefore do not reproduce the qualified
ELF on their own; the original link inputs/overrides must be recovered. No fresh
audit build replaces the working library, and no byte-reproduction claim is made.

Binary assets remain unpublished until custom SDL, extension/codec and SwiftShader
source/build correspondence and transitive license obligations are established.
The source checkpoint/tag and draft release can be prepared independently.
