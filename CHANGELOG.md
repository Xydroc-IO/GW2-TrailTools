# Changelog

Full player-facing notes: [`RELEASE_NOTES.md`](RELEASE_NOTES.md).

## 1.0.0

First standalone release of **GW2-TrailTools** (extracted from GW2-InGame-Helper Trail Tools).

- Raidcore Nexus addon (`TRLS`) with Pack / Trails / Markers / Live / Keybinds hub
- OverlayData authoring, TrailsN / MarkersN editors, import/build `.taco`
- Draft world GPS preview (SwapChain D3D + ImGui fallback)
- Custom QuickAccess brand icon (`assets/trailtools-icon.png`, transparent PNG, baked into DLL)
- Domain layout with ≤500 lines per `src/**/*.cpp` (`make check-lines`)
- Lua: author/pack `.lua` + `script-*` attrs only — **no in-addon Lua runtime**
