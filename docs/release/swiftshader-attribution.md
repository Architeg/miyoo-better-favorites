# SwiftShader prebuilt attribution mapping

Scope: unchanged `libEGL.so` (55,100 bytes, SHA-256 `58e07aec9d59058b02c3ab8478038eec7e7c7964bedb8707f369998425692570`) and `libGLESv2.so` (21,775,928 bytes, `80374f563291d83042f1fe1970f9c99063bcd48f1578cfec83ea8613d36382fe`). Source: `swiftshader/` at SDL fork commit `3c68ed01fee7feffd4ea338b1cc5018a455e2be9`. No graphics/audio replacement.

## Concrete evidence and notice mapping

| Component | Evidence / limit | Material supplied in the user ZIP |
| --- | --- | --- |
| SwiftShader EGL/GLES, renderer and reactor | Both binaries report 4.1.0.7; fork Version.h agrees. Custom eglUpdateBufferSettings matches the fork. CMake libGLESv2 links libGLESCommon, gl_swiftshader_core and GLCompiler. | `licenses/swiftshader-LICENSE.txt` (Apache-2.0), `swiftshader-AUTHORS.txt`; entire pinned source companion retains source copyright headers. |
| LLVM reactor | Binary strings include LLVM intrinsic tables (`llvm.hexagon.C4.fastcorner9`) and LLVM type/calling-convention names. Reactor CMake supports LLVM and Subzero; source defaults LLVM. Exact LLVM revision is **not established** by these strings. | Root LLVM 7 and LLVM 10 licenses added explicitly, alongside pinned LLVM/subzero component notices. LLVM 10 Apache-2.0 with LLVM exception and legacy NCSA terms are retained. |
| LLVM support's regex and other embedded support code | Separate COPYRIGHT.regex and support LICENSE.TXT exist in the source tree. LLVM root license identifies per-file/per-directory third-party terms. | All available LICENSE/COPYING/NOTICE/COPYRIGHT families preserved under `licenses/swiftshader-components/`; full source retains inline terms. |
| Generated GLSL Bison parser | GLCompiler source list includes glslang_tab.cpp; its original header names GNU Bison 3.0.4 and FSF, GPLv3-or-later with explicit larger-work exception. This is source linkage evidence, not a recovered prebuilt link log. | Exact header/exemption `licenses/SwiftShader-Bison-skeleton.txt`, plus project GPL text and full unchanged generated source. |
| Alternate Subzero, ANGLE, SPIR-V, ASTC and ancillary dependencies | Source supports multiple targets/backends and contains these components. GL_ANGLE extension strings alone do **not** establish linked ANGLE code. No claim that every source dependency is embedded in these two binaries. | Conservatively supplied root/third-party notice families and pinned source, including Subzero/LLVM licenses. |

## Closed omissions versus remaining provenance

The package previously omitted the **LLVM 7/10 root license texts** (the fork contains source references to them, but not those root files). The exact Bison header was available in the source companion but is now also supplied as a readable binary-package notice. These concrete omissions have been remedied without changing any executable library.

Known source-supported component attribution is covered by the mapped material. No further **specific missing required notice** was identified in this audit. This closes the earlier blanket assertion that an unidentified notice inventory automatically forbids packaging. It does **not** prove the exact prebuilt component inventory: the original linker log/options and exact LLVM revision remain unavailable. Optional bitwise reproduction is a separate goal, not imposed as a blanket release gate. Reopen attribution if concrete additional embedded code is identified.

Official LLVM texts were retrieved verbatim from [LLVM 10.0.0](https://github.com/llvm/llvm-project/blob/llvmorg-10.0.0/llvm/LICENSE.TXT) and [LLVM 7.0.0](https://github.com/llvm/llvm-project/blob/llvmorg-7.0.0/llvm/LICENSE.TXT). Provenance/SHA-256 is recorded in `third_party/notices/SwiftShader-supplemental.json`; packaging verifies every byte. Existing notices remain unmodified.

Optional upstream question, not sent: “For the two prebuilt hashes above, which LLVM revision/backend and non-SwiftShader components were statically linked, and is the original link command or attribution list available?” No rebuild or maintainer contact is required to retain the current working library files.
