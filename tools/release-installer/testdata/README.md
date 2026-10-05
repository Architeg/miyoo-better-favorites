# OS metadata fixtures

`macos-appledouble.bin` was produced on macOS by `ditto -c -k --sequesterRsrc
--keepParent` from a test `Install-Linux.desktop` carrying only the benign
`com.example.betterfavorites.fixture=metadata regression` attribute. It is an
actual AppleDouble encoding, not a clean payload-ZIP roundtrip. It contains no
user download origin, quarantine data or credentials.

`finder-empty.bin` retains the allocator/header of a Finder-produced `.DS_Store`.
All non-allocator blocks and unused file space were zeroed and its DSDB record count reset to zero to
exclude personal filenames. Tests also optionally validate an unchanged local
Finder file using `BF_FINDER_METADATA`, without archiving that file.

The reported mounted-card sidecar was unavailable during this correction:
`App/BetterFavorites` was absent. Its format has not been independently verified.

## RC6 directory fixture

`macos-directory-appledouble.bin` is real directory AppleDouble output from macOS `ditto -c -k --sequesterRsrc --keepParent` on a newly created host-only directory named `computer`. It contains only the benign test attribute `com.example.betterfavorites.fixture=directory regression`. The enclosing directory was archived so `__MACOSX/.../._computer` was emitted. It does not contain the mounted card's quarantine value or personal file data.

The actual RC5 card sidecar was inspected read-only: valid AppleDouble v2, 4096 bytes. Its private attribute values are deliberately not copied here. `BF_DIRECTORY_METADATA` optionally checks that actual file read-only; ordinary tests use this redistributable Mac-produced fixture.
