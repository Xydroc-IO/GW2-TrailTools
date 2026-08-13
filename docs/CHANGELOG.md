# Changelog

Full player-facing notes: [`RELEASE_NOTES.md`](RELEASE_NOTES.md).

## Unreleased

- Hub tabs: **Pack → Content → Live → Keybinds** (Trails and Markers combined on Content)
- Content: trail/marker defaults, project lists, open Trails1–5 / MarkersN; **Add to project** writes the one OverlayData XML
- TrailsN compact editor: file + trail toolbars, recording rail (Start/Stop, Pause/Resume, vector spacing), raw point list
- Five TrailsN windows (not four); opening an editor no longer freezes the game (swap instead of copying point vectors)
- Pack XML is always **one OverlayData file** (categories + trails + markers); leftover `_Menu.xml` / `_Data.xml` are removed on save/build
- TrailsN: per-trail OverlayData attrs, reverse/densify/smooth, multi-select, Ctrl+Z undo
- MarkersN: behavior combo, mapDisplaySize/minSize/maxSize, achievement/festival/profession/race/mount/toggleCategory filters, Ctrl+Z undo
- Live: world click place marker / add trail point / select nearest (feet-plane pick)

- TrailsN: per-trail OverlayData attrs, reverse/densify/smooth, multi-select, Ctrl+Z undo
- MarkersN: behavior combo, mapDisplaySize/minSize/maxSize, achievement/festival/profession/race/mount/toggleCategory filters, Ctrl+Z undo
- Live: world click place marker / add trail point / select nearest (feet-plane pick)

## 1.0.0

First standalone release of **GW2-TrailTools** (extracted from GW2-InGame-Helper Trail Tools).

- Raidcore Nexus addon (`TRLS`) with Pack / Trails / Markers / Live / Keybinds hub
- OverlayData authoring, TrailsN / MarkersN editors, import/build `.taco`
- Draft world GPS preview (SwapChain D3D + ImGui fallback)
- Custom QuickAccess brand icon (`assets/trailtools-icon.png`, transparent PNG, baked into DLL)
- Domain layout with ≤500 lines per `src/**/*.cpp` (`make check-lines`)
- Lua: author/pack `.lua` + `script-*` attrs only — **no in-addon Lua runtime**
