# Changelog

Full player-facing notes: [`RELEASE_NOTES.md`](RELEASE_NOTES.md).

## Unreleased

- MarkersN: left rail **Data** (toolbar + full draft list) / **Settings** (type, filters, POI XML); title becomes MarkersN Settings on the gear tab
- MarkersN list click / Select Nearest lock the UberTool on that POI; **Move to Feet** and **Delete** follow the active gizmo selection (not a stale slot index)
- Delete retargets open MarkersN `poiIndex` values when draft indices shift
- Closing a pad (X) does not click through into Start recording / New Trails; Start with no Trails window only warns (does not auto-open)
- Nested pad list scroll thumbs stay in their panel (no bleed into title min/close)
- Pad clicks discard pending WndProc world-clicks before PackEdit / UberTool (Wine often misses `WantCaptureMouse`)
- Crash handler snapshots only when the fault address is inside `GW2-TrailTools.dll` (avoids Wine freezes dumping other addons); world GPS restores full D3D pipeline state after draw
- Hub tabs: **Content → Keybinds** (Content uses the anvil icon; Live / UberTool / world click live there). Pack workshop is a collapsed section on Content
- Pack-in-world / gizmo / pop-out / ground-snap checkboxes on Nexus Options, Pack editor, and Content; **Draw pack in world** defaults off; pack overlay works with the pad closed
- Draft GPS waits for **Start recording** (or an explicit place) in an open TrailsN; UberTool + Draft preview default on (saved in settings)
- **Clear world trail** on Content and Nexus Options removes a GPS that is not in any Trails window (and turns pack overlay off)
- Nexus Options credit / Ko-fi footer names **Trail Tools**
- TrailsN: more pad padding; file tools vs trail tools on separate wrap rows; recording **Start** / **Stop** are separate buttons; samples on **time** (Spacing seconds) while moving; **New Segment** writes TacO `0,0,0` plus MapID + feet vector; Start after Stop does not auto-insert a segment or vector (empty trail still gets a first vector)
- Live **Hide trail near me** toggles the player-clear bubble (radius still in Nexus Options → Trail player clear)
- UberTool: click or Select Nearest / Move to Feet to place the gizmo; drag follows the mouse (white hub / RGB axes); recording does not hop the gizmo to new samples
- Trail preview: clickable vertex circles; default no hide-around-player; optional untextured thin GPS (Nexus Options)
- **Editor** (`src/packedit/`): open `.taco` / folder, New / Close pack, Save patches original XML (keeps comments/unknown attrs on POI/Trail tags), 2D map (pan/zoom/select/Shift+click place), clickable resources, collapsed tree + clipper, pop-outs, this-map world draw (culled), WndProc pick + gizmo, duplicate, tombstone, undo, lint
- Hub wheel scrolls the pad (nested lists use their own scrollbar); 2D map **Ctrl+wheel** zoom
- Content: trail/marker defaults, project lists, open Trails1–5 / MarkersN; **Add to project** writes the one OverlayData XML
- TrailsN compact editor: file + trail toolbars, recording rail (Start/Pause, vector spacing), raw XYZ list, TacO-basic `<Trail type trailData>` copy
- Recording does not add points while standing still (compares to last trail vertex vs spacing)
- Five TrailsN windows (not four); opening an editor no longer freezes the game (swap instead of copying point vectors)
- Pack XML matches TacO OverlayData (Lady Elyssa layout): nested MarkerCategory, 3-space indent, `<Trail type="…" trailData="Data/….trl"/>`, images under `Data/Images/`
- Seed categories: root + `example` / `circle` / `heart` / `square` / `triangle` (`pack.example`, `pack.circle`, …)
- Live **3D UberTool** + **Ground snap** (plane fit from walked Mumble feet, draft points, and open-pack Y — not a collision mesh); gizmo stays on the clicked vector (does not follow recording); drag after an 8px mouse move; larger RGB arrows
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
