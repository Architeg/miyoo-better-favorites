# Distributed dependency audit

[Readable notices](../../THIRD_PARTY_NOTICES.md) · [Build procedure](build.md) · [Exact binary evidence](dependency-components.json)

This audit concerns the actual nine unchanged private app libraries in [dependency-hashes.json](dependency-hashes.json). MainUI, firmware drivers, user themes, ROMs and saves are not in the app payload. Build success, runtime availability, attribution/source supply and byte-identical reproduction are different questions.

## Component decisions

| Actual distributed file | License/source evidence and supplied material | Concrete missing item / smallest remedy |
| --- | --- | --- |
| libSDL2-2.0.so.0 | SDL 2.0.20 fork; core zlib, Miyoo backend LGPL-2.1. Pinned SDL source, root LGPL and SDL license supplied. Rebuilt entire ELF equals accepted SHA f103f543… | Custom SDL source/build correspondence is resolved. External firmware SDK is a build prerequisite, not redistributed. |
| libSDL2_image-2.0.so.0 | IMG_Linked_Version disassembly sets 2.0.5. Official pinned 2.0.5 source/COPYING and embedded miniz/NanoSVG source notices supplied. Dynamic-loader strings identify external jpeg/png/tiff/webp libraries. | Corrects previous 2.8.1 archive-versus-binary confusion. Exact byte rebuilding/configuration is unproven, not itself a zlib-license publication condition. No required root notice remains missing. |
| libSDL2_mixer-2.0.so.0 | Mix_Linked_Version sets 2.0.4. Official source/COPYING and its codec/TiMidity notice families supplied. libvorbisidec.so.1 is loaded externally; no exported codec-library definitions were identified. | Corrects 2.6.3 header/source-versus-binary confusion. Original configure/static-codec selection is unavailable; version-matched source and all available embedded-code notices are provided. This is not an exhaustive proof of every static component. |
| libSDL2_ttf-2.0.so.0 | TTF_Linked_Version sets 2.0.15. Official source/COPYING supplied. DT_NEEDED names freetype, bzip2, png, zlib and SDL. | Corrects 2.20.2 header-versus-binary confusion. Required binary root notice supplied; installed freetype/bzip2 are not redistributed by this app. |
| libjson-c.so.5 | MIT-style upstream author notices; pinned 0.15 archive/COPYING supplied | Binary build identity is not proven by archive name. No additional source-rebuild condition is imposed by this permissive notice. |
| libpng16.so.16 | Embedded 1.6.37 string; pinned 1.6.37 source/original LICENSE supplied | Build configuration remains unknown; exact byte reproduction is separate from preserved libpng notice requirements. |
| libz.so.1 | Embedded 1.2.11 string; pinned 1.2.11 source/original README license supplied | No missing dedicated license remains. Reproduction is a separate goal. |
| libEGL.so / libGLESv2.so | SwiftShader 4.1.0.7 strings match src/Common/Version.h; custom eglUpdateBufferSettings appears in the supplied fork. Apache-2.0/AUTHORS and all available third-party notice families supplied with source. | **Remaining binary-publication question:** upstream did not provide the component/NOTICE inventory for these particular prebuilts. Smallest remedy is its prebuilt attribution/build inventory, sufficient to map notices to actual embedded components; an entire byte-identical rebuild is not inherently required by Apache-2.0. A compatible rebuild with known inputs is an alternative, but this pass does not replace working graphics/audio libraries. |
| Modern/legacy Go host code | Go1.26.2 and isolated official Go1.20.14, standard library only; preserved BSD notice | Source/toolchain identities recorded. No user toolchain requirement; legacy security/OS execution limitations remain disclosed. |

The extension version evidence is the compiled ARM version function, not the newer development headers. Official upstream archives/notices are pinned in [upstream-sources.json](upstream-sources.json). Source companions preserve all notice families, including COPYING.txt, COPYRIGHT and NOTICE. Possible third-party notices are supplied conservatively; this is not an assertion that every optional codec was linked.

## Custom OSS SDL: resolved exact reproduction

The previously missing link inputs are **libmi_common, libmi_sys and libmi_gfx**, plus the correct EGL/GLES library search directory. The accepted ELF's DT_NEEDED independently names these inputs. Adding them in the isolated pinned-toolchain build produced **the entire original ELF byte for byte**, SHA256 `f103f5439d62a570b8977dcd7352779404cfb5112fc79008899c91c718a85d8a`.

The exact configure switches are in [build provenance](build.md). SDL source/revision, SDL_image header and recorded compiler flags were retained. Final link addition:

```text
EXTRA_LDFLAGS=-L<SDK>/mini/lib -lmi_common -lmi_sys -lmi_gfx
 -L<fork>/prebuilt/mini -lm -ldl -lEGL -lGLESv2 -lpthread -lrt
```

No rebuilt audit library replaces the accepted working file. OSS remains enabled; mini audio stays disabled. MI SDK headers explicitly restrict disclosure/redistribution. New source companions exclude `mini/inc` and `mini/lib`; firmware driver binaries are not in app ZIPs. SDK access for this advanced reproduction must be separately authorized. Ordinary contributor app/tests do not require publishing private SDK/firmware fixtures.

## Runtime bzip2 and compiler notes

SDL_ttf DT_NEEDED includes libbz2.so.1.0. Read-only inspection finds `miyoo/lib/libbz2.so.1.0` on the tested card, SHA256 `027db0ed00700063f4af51afafd7598672e3536d21f4af0fef4c419a8ed1440c`; Onion's inherited library search environment includes this path. This supports the recorded working runtime, not a fresh device test or guarantee for every card. The build's existing link warning reports the build-time search environment. GCC ABI notes alone are not release blockers. No shared library/audio setting is changed or bundled merely to silence a warning.

## Publication boundary

Custom SDL correspondence, actual extension versions and missing root notices are resolved/narrowed as above. Do not retain a general requirement to byte-reproduce all permissively licensed prebuilts. The specific outstanding attribution mapping is SwiftShader's actual prebuilt component inventory, affecting both full and app-only ZIPs because both include EGL/GLES. Project source and new SDK-excluding source companions are separate artifacts. No binary release is published in this preparation pass; historical archives remain intact.

Apache's [redistribution conditions](https://www.apache.org/licenses/LICENSE-2.0) distinguish license/NOTICE preservation from exact build reproducibility. LGPL terms remain supplied in licenses/LICENSE and apply to the modified Miyoo backend.
