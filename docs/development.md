# Development

## Host environment

Development is performed on macOS.

Editor:

- Pulsar

Tools:

- Git
- Docker
- Miyoo / Onion cross-compilation toolchain
- C++
- SDL2

## Development strategy

The native application and Onion integration layer are kept separate.

## Authoritative plan and current status

Use [roadmap.md](roadmap.md) for the complete feature inventory, remaining milestone
order, acceptance criteria and unresolved decisions. Use
[development-status.md](development-status.md) for the committed/deployed checkpoint
and hardware versus host verification status. The old high-level development order
is superseded by this consolidated roadmap; core browser settings/title/navigation
work remains before profiling and Home entry integration.

**Maintenance rule:** update roadmap and status when a milestone changes or hardware
verification is received. Record commit/device/Onion/theme and tested behavior.
Do not mark a feature hardware-confirmed from a successful build or host fixture.
Do not treat code-only support as a usable persistent Settings option.

The current Makefile/Docker workflow builds a prepared checkout. Reproducible
clean-checkout dependencies, custom SDL2/audio provenance, packaging and production
app installation remain roadmap gates. `scripts/fetch-deps.sh` is real dependency
preparation; the other build/install/uninstall/package script placeholders are not
completed tooling. See [architecture](architecture.md) and
[version-specific optional return integration](onion-return.md).

M3 host timing/SDL resource checks and its separate device checklist are documented
in [browser-title-scrolling.md](browser-title-scrolling.md).

Use [m4-audit.md](m4-audit.md) to distinguish verified existing behavior, local
resource corrections, approved shoulder paging and user hardware acceptance.

M5's closed measurement pass, startup/idle results, biases, explicit gameplay and
ON/OFF deferrals, reusable tools and verified retirement archive are documented
in [m5-profiling.md](m5-profiling.md). The per-parse cache has host checks;
device speedup/build acceptance is unverified. M6 proceeds with read-only entry
investigation; no further profiling sessions or device commands are requested.
