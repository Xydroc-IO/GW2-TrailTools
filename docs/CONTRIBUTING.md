# Contributing — GW2-TrailTools

Raidcore **Nexus** ImGui DLL. Current shipping version: **1.0.0**. Normative allow/deny: [`COMPLIANCE.md`](COMPLIANCE.md).

## Prerequisites

- `x86_64-w64-mingw32-g++`, `make`, `python3`, `git`
- Arch/Manjaro: `sudo pacman -S --needed mingw-w64-gcc make git`

## Build

```bash
make -j"$(nproc)"
make check-lines
make install   # optional
```

## Rules (strict)

1. **≤500 lines** per `src/**/*.cpp`. Split by concern — never grow past the limit.
2. **Domain folders** only: `app/`, `ui/`, `pathing/`, `trailtools/` (+ Nexus `entry*`).
3. Flat includes (`#include "Foo.h"`) via Makefile `-Isrc/...` paths.
4. Domains use public headers + `*Shared.h` / `*Internal.h` with **one** defining TU for Shared globals.
5. Stay inside compliance: Nexus APIs, MumbleLink read-only. **No** game memory R/W or Present hooks.
6. Focused diffs; no secrets / large `.taco` packs in git.

## Branding / QuickAccess icon

- Source art: `assets/trailtools-icon.png` (+ hover / logo) — keep **transparent** backgrounds.
- Bake into `src/ui/TrailToolsIcon.h`: `python3 tools/bake_icons.py`
- Bump `TEX_GW2_TRAILTOOLS_QA_v*` in `TrailToolsQuickAccess.cpp` when the PNG changes (Nexus caches by id).
- Version bumps: edit `src/app/AddonVersion.h` and note in `CHANGELOG.md`.

## Code map

| Area | Path |
|------|------|
| Nexus load/unload | `src/entry*.cpp` |
| Settings / paths / version | `src/app/` |
| Pad chrome / QuickAccess | `src/ui/` |
| `.taco` / `.trl` parse | `src/pathing/packs/` |
| Authoring pad | `src/trailtools/` |
| OverlayData XML | `TrailToolsXml.cpp` (TacO indent / `trailData`) |
| 3D UberTool | `TrailToolsUberTool.cpp` + `TrailToolsWorldPick.cpp` |

## License

MIT — see [LICENSE](../LICENSE).
