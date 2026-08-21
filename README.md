# GW2-TrailTools

<p align="center">
  <img src="assets/trailtools-logo.png" alt="Trail Tools" width="420"/>
</p>

<p align="center"><strong>v1.0.0.1</strong></p>

Standalone **Raidcore Nexus** addon for Guild Wars 2 — TacO/Blish **trail & marker pack authoring**, split out from [GW2-InGame-Helper](https://github.com/Xydroc-IO/GW2-InGame-Helper) Trail Tools.

Official Nexus APIs + MumbleLink (read-only). **No** game memory R/W, **no** Present hooks.

## Features

- **Content** / Keybinds authoring hub (anvil icon on Content); Pose COPY inserts at OverlayData caret; **Write into OverlayData**
- Collapsed Pack on Content: pack name, Import / Build .taco (Lua list parked); Looks + default textures in Nexus Options
- In-place `.taco` pack editor under Nexus Options (tree, 2D map); pack-in-world / gizmo / pop-out toggles on Options and Pack editor
- Ground snap from walked Mumble + pack/draft points (plane fit, not a collision mesh)
- One OverlayData XML (TacO layout: nested categories, `trailData="Data/….trl"`, `Data/Images/`) + up to five TrailsN / four MarkersN editors
- Editable OverlayData window for custom TacO/Blish attributes and layouts (Tab = 3 spaces; Fill from draft on the XML editor)
- Live 3D UberTool + draft preview **on by default** (saved in `settings.ini`): click-select + RGB move gizmo on draft markers and trail vertices
- MarkersN: **Data** / **Settings** side rail; list shows every draft marker; Data **Copy XML**; Move to Feet follows the selected gizmo POI
- TrailsN recording: no world GPS until **Start**; Start requires an open Trails window; Stop stays stopped; New Segment for TacO `0,0,0` breaks
- Closing a pad (X) does not click through into Start / New Trails
- **Clear world trail** (Content and Nexus Options) drops a GPS that is not in any Trails window
- Optional **Hide trail near me** player-clear bubble (Nexus Options radius)
- Crash-Logs only for faults inside this DLL; world GPS restores D3D pipeline after draw
- Nexus Options credit / Ko-fi footer (Trail Tools)
- Import existing `.taco`, build new packs under `addons/GW2-TrailTools/pathing/`
- Addon-polled trail/marker chords (works while pad closed)
- Nexus QuickAccess icon (custom Trail Tools brand art)

## Requirements

- Guild Wars 2 (64-bit)
- [Raidcore Nexus](https://raidcore.gg/gw2/nexus)
- MinGW-w64 (`x86_64-w64-mingw32-g++`) to build from source

## Build

```bash
git clone https://github.com/Xydroc-IO/GW2-TrailTools.git
cd GW2-TrailTools
make -j"$(nproc)"
make check-lines   # enforce ≤500 lines per src/**/*.cpp
make install       # optional — copies into GW2 addons/
```

Dependencies under `deps/` (Nexus API headers, Raidcore imgui, miniz) are vendored in-tree.

Or CMake:

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake
cmake --build build -j"$(nproc)"
```

## Install (players)

1. Close Guild Wars 2.
2. Copy `GW2-TrailTools.dll` into `<GW2>/addons/`.
3. Start the game → `Ctrl+O` → enable **GW2-TrailTools**.
4. Toggle pad: Nexus **QuickAccess** icon (top bar) or `Alt+Shift+T`.

Runtime data: `<GW2>/addons/GW2-TrailTools/` (`settings.ini`, `pathing/authoring/`, built `.taco`).

Crash dumps: `<GW2>/addons/GW2-TrailTools/Crash-Logs/` (`crash-trail.txt`, `crash.log`, timestamped snapshot folders). Snapshots are written only when the fault address is inside `GW2-TrailTools.dll` (other addons' AVs are ignored so Wine is not frozen by dump I/O).

## Branding

| Asset | Path | Use |
|-------|------|-----|
| QuickAccess icon | [`assets/trailtools-icon.png`](assets/trailtools-icon.png) | Nexus top-bar shortcut (baked into DLL) |
| Hover icon | [`assets/trailtools-icon-hover.png`](assets/trailtools-icon-hover.png) | QuickAccess hover |
| Logo | [`assets/trailtools-logo.png`](assets/trailtools-logo.png) | README / docs |

Icons use a **transparent** PNG alpha (no white square). After changing art:

```bash
python3 tools/bake_icons.py
# bump TEX_GW2_TRAILTOOLS_QA_v* ids in TrailToolsQuickAccess.cpp if Nexus caches the old texture
make -j"$(nproc)"
```

## Version

Shipping revision is **1.0.0.1** (`src/app/AddonVersion.h` / CMake `VERSION 1.0.0.1`). See [`docs/RELEASE_NOTES.md`](docs/RELEASE_NOTES.md) and [`docs/CHANGELOG.md`](docs/CHANGELOG.md).

## Source layout (domain-based, ≤500 lines / `.cpp`)

| Domain | Path |
|--------|------|
| Nexus entry | `src/entry*.cpp` |
| App shell | `src/app/` |
| UI chrome | `src/ui/` |
| Pack parse types | `src/pathing/` |
| **Trail Tools** | `src/trailtools/` |
| Pack Editor | `src/packedit/` |

See [`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md) and [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## Acknowledgments

Huge thanks to **Lady Elyssa** for helping sort the UI, feature selection, and extensive Windows testing.

## License

MIT — see [LICENSE](LICENSE). Guild Wars 2 and related trademarks belong to ArenaNet / NCSoft.
