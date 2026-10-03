# License and dependency provenance

Better Favorites app, integration, installer and test sources are licensed under
**GPL-3.0-or-later** (GNU GPL version 3 or, at your option, any later version).
See LICENSE. Third-party code/libraries keep their respective licenses.
No ROMs, BIOS, saves, private card evidence, theme/font artwork or vendor MainUI
binaries are distributed. The user's exact originals are patched locally.

| Component | Recorded source / licensing | Candidate status |
| --- | --- | --- |
| SDL2 Miyoo fork / mini backend | Rparadise-Team/sdl2_miyoo_new commit 3c68ed01fee7feffd4ea338b1cc5018a455e2be9; fork root LGPL-2.1, SDL source zlib, embedded components retain notices | Working custom OSS ELF preserved; configuration/source unchanged; fresh binary reproduction still pending |
| SDL2_image 2.8.1 / mixer 2.6.3 / ttf 2.20.2 | Dependency archives and examples/prebuilts in that pin; zlib notices included | Full source archives included in dependency source companion; prebuilt correspondence/transitive codec audit pending |
| json-c 0.15 | Archive at same pin; MIT-style license | Source/license companion; prebuilt correspondence pending |
| SwiftShader EGL/GLES | Same fork's full swiftshader tree; Apache-2.0 and per-component licenses retained | 21 MB GLES is a disk file, not measured RAM; exact prebuilt/source/component correspondence still pending |
| libpng16 / zlib | Prebuilt examples at same pin; libpng/zlib licenses | Exact source versions/build provenance must be established before public binary redistribution |
| Go standard library/runtime | Build host Go 1.26.2; Go BSD license included | No external Go modules; runtime notice included |
| Onion runtime / MainUI / firmware drivers / libpadsp | User-installed Onion v4.3.1-1/firmware, exact original files | No public prepatched binaries or original runtime shipped; patch/source only; user backups stay private |

Release source companions contain project Git source and the pinned dependency
source tree (including upstream license files/dependency archives). Merely supplying
that tree is **not** proof of complete corresponding source for every prebuilt.
Until the outstanding provenance/transitive license audit is complete, rc.1 is
**private review material**, not a cleared public distribution. Project GPL choice
alone cannot settle third-party binary obligations. Do not remove notices or
advertise the license audit as complete.
