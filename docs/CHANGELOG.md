# Changelog

Full player-facing notes: [`RELEASE_NOTES.md`](RELEASE_NOTES.md).

## Unreleased

- Hub tabs: **Editor → Pack → Content → Live → Keybinds**
- **Editor** (`src/packedit/`): open `.taco` / folder, New / Close pack, Save patches original XML (keeps comments/unknown attrs on POI/Trail tags), 2D map (pan/zoom/select/Shift+click place), clickable resources, collapsed tree + clipper, pop-outs, this-map world draw (culled), WndProc pick + gizmo, duplicate, tombstone, undo, lint
- Hub wheel scrolls the pad (nested lists use their own scrollbar); 2D map **Ctrl+wheel** zoom
- Content: trail/marker defaults, project lists, open Trails1–5 / MarkersN; **Add to project** writes the one OverlayData XML
- TrailsN compact editor: file + trail toolbars, recording rail (Start/Pause, vector spacing), raw XYZ list, TacO-basic `<Trail type trailData>` copy
- Recording does not add points while standing still (compares to last trail vertex vs spacing)
- Five TrailsN windows (not four); opening an editor no longer freezes the game (swap instead of copying point vectors)
- Pack XML matches TacO OverlayData (Lady Elyssa layout): nested MarkerCategory, 3-space indent, `<Trail type="…" trailData="Data/….trl"/>`, images under `Data/Images/`
- Seed categories: root + `example` / `circle` / `heart` / `square` / `triangle` (`pack.example`, `pack.circle`, …)
- Live **3D UberTool** + **Ground snap** (plane fit from walked Mumble feet, draft points, and open-pack Y — not a collision mesh)
- Empty-world clicks via Nexus `WndProc` (`WorldClick`) so ImGui can pick when the cursor is not over a window
- Editable OverlayData window (Pack → XML editor): keep custom TacO/Blish attrs; Apply maps known fields into the editors; Save writes the text as-is
- Draft world preview follows the focused TrailsN recording trail
- TrailsN: per-trail OverlayData attrs, reverse/densify/smooth, multi-select, Ctrl+Z undo
- MarkersN: behavior combo, mapDisplaySize/minSize/maxSize, achievement/festival/profession/race/mount/toggleCategory filters, Ctrl+Z undo
- Live: world click place marker / add trail point / select nearest (feet-plane pick; skipped while UberTool consumes the click)

## 1.0.0

First standalone release of **GW2-TrailTools** (extracted from GW2-InGame-Helper Trail Tools).

- Raidcore Nexus addon (`TRLS`) with Pack / Trails / Markers / Live / Keybinds hub
- OverlayData authoring, TrailsN / MarkersN editors, import/build `.taco`
- Draft world GPS preview (SwapChain D3D + ImGui fallback)
- Custom QuickAccess brand icon (`assets/trailtools-icon.png`, transparent PNG, baked into DLL)
- Domain layout with ≤500 lines per `src/**/*.cpp` (`make check-lines`)
- Lua: author/pack `.lua` + `script-*` attrs only — **no in-addon Lua runtime**
