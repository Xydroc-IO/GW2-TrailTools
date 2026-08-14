# Release Notes — GW2-TrailTools 1.0.0

**First standalone release** of Trail Tools as a Raidcore Nexus addon (`GW2-TrailTools`, signature `TRLS`), extracted from [GW2-InGame-Helper](https://github.com/Xydroc-IO/GW2-InGame-Helper).

Toggle: Nexus **QuickAccess** icon or **Alt+Shift+T**.

---

## Highlights

### Authoring hub
- Side-rail tabs: **Editor → Pack → Content → Live → Keybinds**
- **Editor** tab: in-place `.taco` (open zip or folder, 2D map, tree, details, clickable resources). Save patches original XML. **Close pack** clears the session. Wheel scrolls the pad; **Ctrl+wheel** zooms the map.
- Pack tab owns the OverlayData project (New / Load / Save) — **one XML file**
- **Content** lists trails and markers in that project and opens TrailsN / MarkersN editors
- TrailsN / MarkersN pop-outs own their own `.trl` / marker data until **Add to project**
- Cartographer UI theme (slate + teal), custom brand QuickAccess icon

### Pack workflow
- Pack name / display name, draft session save/load, reseed categories
- Import installed `.taco` into the draft
- **Build .taco** → `addons/GW2-TrailTools/pathing/`, enable root category, reload Pathing index
- Looks presets, asset browser, category tree editor
- Single OverlayData XML in TacO form (3-space indent, nested MarkerCategory, `<Trail type trailData="Data/….trl"/>`, assets in `Data/Images/`)
- Pack → Reseed for Lady Elyssa–style leaves (`example` / `circle` / `heart` / `square` / `triangle`)
- OverlayData **XML editor** pop-out: edit raw text (custom attrs/layout); Apply fills TrailsN/MarkersN; Save writes the buffer as-is

### Trails
- Default trail category on Content; up to **five** TrailsN windows
- Compact TrailsN: New / Load / Save / Save As · New Segment / Insert Vector / Select Nearest / Move to Feet / Delete Nearest / Undo
- Recording rail: Start/Pause, vector spacing; raw XYZ list; TacO-basic `<Trail type trailData>` copy
- Recording skips samples while standing still (last vertex vs spacing)
- `.trl` stays with the pop-out until **Add to project** inserts it into OverlayData
- Per-`<Trail>` attrs (texture, trailScale, fade, alpha, animSpeed)
- Geometry: reverse / densify / smooth / multi-select (Ctrl/Shift)
- Snapshot undo (Ctrl+Z) on TrailsN
- Live draft preview (world GPS + compass) follows the focused TrailsN trail

### Markers
- Default marker category on Content; MarkersN pop-outs for focused POIs
- Insert / Select Nearest / Delete / Move to Feet / Undo
- Drop / delete at feet, this-map filter, marker attribute editors
- Named behavior combo + Display (mapDisplaySize / minSize / maxSize) + Filters (achievement, festival, profession/race/mount, toggleCategory)
- Blish-style `script-*` attributes on POIs (stored in OverlayData)
- Snapshot undo (Ctrl+Z) on MarkersN
- Copy markers from currently loaded Pathing packs

### Live
- **3D UberTool** + **Ground snap**: plane fit from walked Mumble feet, draft points, and the open pack (same Mumble pose TacO uses — not game process memory, not a live mesh). Click a draft marker or trail vertex, drag RGB axes to move, Ctrl+click a trail to insert a point, right-click while dragging to cancel.
- World click / map Shift+click place against that plane when Ground snap is on. Disabled for that click when UberTool consumes it.
- Empty-world `LBUTTONDOWN` is seen through Nexus `WndProc` (`WorldClick`) because ImGui does not get clicks on empty game view.

### Keybinds
- Trail record / pause / section / delete-segment chords
- Marker delete + place-slot chords (Ctrl+Numpad-style)

---

## Branding & version

| | |
|--|--|
| Version | **1.0.0** (`AddonVersion.h`) |
| Icon | `assets/trailtools-icon.png` (transparent PNG, baked into DLL) |
| Logo | `assets/trailtools-logo.png` |
| Update link | https://github.com/Xydroc-IO/GW2-TrailTools |

---

## Install

1. Close Guild Wars 2.
2. Copy `GW2-TrailTools.dll` into `<GW2>/addons/`.
3. `Ctrl+O` → enable **GW2-TrailTools**.
4. Open via QuickAccess or **Alt+Shift+T**.

Data: `<GW2>/addons/GW2-TrailTools/` (`settings.ini`, `pathing/authoring/`, built `.taco`).

---

## Lua (what works / what does not)

**Authoring — yes**
- Pack tab lists `.lua` under the authoring folder (`Scripts/` recommended).
- **Build .taco** packs those files into the archive with the rest of the pack.
- Marker editors expose Blish `script-once` / `script-trigger` (and related) attrs written into OverlayData.

**Runtime — no (this addon)**
- GW2-TrailTools does **not** embed a Lua VM / PathingLua host.
- Scripts are not executed inside Trail Tools.
- Built packs that contain `.lua` / script attrs are meant for a **Pathing consumer that runs Lua** (e.g. GW2-InGame-Helper Pathing with Lua enabled), not for in-addon script execution.

---

## Compliance

- Nexus APIs + MumbleLink **read-only**
- World preview via SwapChain D3D (no Present hooks)
- **No** game memory R/W

See `COMPLIANCE.md` and `ARCHITECTURE.md`.

---

## Acknowledgments

**Lady Elyssa** — UI feedback, feature selection, and Windows testing.

## Build from source

```bash
make -j"$(nproc)"
make check-lines
make install   # optional
```

Also: `CHANGELOG.md` for a shorter bullet summary.
