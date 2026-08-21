# Release Notes — GW2-TrailTools 1.0.0.1

Patch on the first standalone Trail Tools Nexus addon (`GW2-TrailTools`, signature `TRLS`), from [GW2-InGame-Helper](https://github.com/Xydroc-IO/GW2-InGame-Helper).

Toggle: Nexus **QuickAccess** icon or **Alt+Shift+T**.

---

## 1.0.0.1

- Content: authoring hub without pack-world toggles / project lists; Pose **COPY** inserts at OverlayData caret; **Write into OverlayData**
- Pack identity + Import/Build on Content; Looks / default textures under Nexus Options
- TrailsN: recording controls stay on the recording window; focus switches trail GPS; Live **All TrailsN** preview; per-trail texture in Attrs
- MarkersN: sticky per-window POI; Data **Copy XML**; Settings uses hub Options gear (selected rail icons brighter)
- XML editor Tab inserts 3 spaces (TacO indent; VK_TAB under Nexus)

---

## Highlights

### Authoring hub
- Side-rail tabs: **Content → Keybinds** (Content uses the former Editor anvil icon; Live pose / UberTool / world click). Collapsed Pack on Content is identity + Import/Build (Lua list parked)
- Pack-in-world / gizmo / pop-out checkboxes on Nexus Options and Pack editor (not Content). **Draw pack in world** starts off; overlay still works with the pad closed
- Default trail/marker looks and default texture / marker-icon browser under Nexus Options
- In-place `.taco` tools (open zip/folder, 2D map, tree, details, resources) live under Nexus Options → **Pack editor**. Save patches original XML. Wheel scrolls lists; **Ctrl+wheel** zooms the map
- Nexus Options footer: CREATED BY XYDROC + Ko-fi (Trail Tools)
- OverlayData New / Load / Save is on **Content** — **one XML file**; Pose **COPY** inserts at the buffer caret
- **Content** trail/marker defaults + TrailsN / MarkersN chips; **Write into OverlayData** updates the draft (no duplicate project lists)
- TrailsN / MarkersN pop-outs own their own `.trl` / marker data until **Write into OverlayData** / desk Add to project
- Cartographer UI theme (slate + teal), custom brand QuickAccess icon

### Pack workflow
- Pack name / display name, draft session save/load, reseed categories
- Import installed `.taco` into the draft
- **Build .taco** → `addons/GW2-TrailTools/pathing/`, enable root category, reload Pathing index
- Looks presets, asset browser (Nexus Options); category tree via Reseed / OverlayData XML
- Single OverlayData XML in TacO form (3-space indent, nested MarkerCategory, `<Trail type trailData="Data/….trl"/>`, assets in `Data/Images/`)
- Pack → Reseed for Lady Elyssa–style leaves (`example` / `circle` / `heart` / `square` / `triangle`)
- OverlayData **XML editor** pop-out: edit raw text (custom attrs/layout); Apply fills TrailsN/MarkersN; Save writes the buffer as-is

### Markers
- Default marker category on Content; MarkersN pop-outs for focused POIs
- MarkersN left rail: **Data** (Insert / Select Nearest / Delete / Move to Feet / Undo / **Copy XML** + full draft list) and **Settings** (type, GUID, XYZ, behavior, Display, Filters, Script / Blish, Copy POI XML)
- List click / Select Nearest / Insert rebind that MarkersN only; Delete / Move to Feet / Settings edit the window’s sticky POI
- Drop / delete at feet, this-map filter on the Markers desk
- Named behavior combo + Display (mapDisplaySize / minSize / maxSize) + Filters (achievement, festival, profession/race/mount, tip/copy/schedule, toggleCategory, iconFile)
- Blish-style `script-*` attributes on POIs (stored in OverlayData)
- Snapshot undo (Ctrl+Z) on MarkersN
- Copy markers from currently loaded Pathing packs

### Live
- **3D UberTool** + **Draft preview** default **on** (persisted). **Ground snap**: plane fit from walked Mumble feet, draft points, and the open pack (same Mumble pose TacO uses — not game process memory, not a live mesh). Click a draft marker or trail vertex (stays selected while recording), drag the white hub to slide / RGB arrows for XYZ, Ctrl+click a trail to insert a point, right-click while dragging to cancel. Gizmo clicks are swallowed so camera look does not steal the drag; movement follows the mouse. Pad list selection locks the gizmo until a world pick.
- **All TrailsN** (with Draft preview) shows every open Trails window GPS, or only the active / recording trail
- **Hide trail near me** (Content → Live, and Nexus Options) toggles the player-clear bubble; **Trail player clear** is the radius
- World click / map Shift+click place against that plane when Ground snap is on. Disabled for that click when UberTool consumes it, or when the pointer is over Trail Tools chrome.
- Empty-world `LBUTTONDOWN` is seen through Nexus `WndProc` (`WorldClick`) because ImGui does not get clicks on empty game view.

### Trails
- Default trail category on Content; up to **five** TrailsN windows
- Compact TrailsN: New / Load / Save / Save As · New Segment / Insert Vector / Select Nearest / Move to Feet / Delete Nearest / Undo
- Recording rail: **Start** and **Stop** are separate buttons (Stop cannot immediately Start); Pause/Resume; **Spacing in seconds** (0.3 = 1/3 s); samples only while the character moves; controls apply to the recording window (focus can retarget)
- World GPS for a draft trail does not appear until **Start** (or Insert Vector / world-click add point) in an open TrailsN. Start with no Trails window only warns
- Closing a pad (X) does not click through into Start / New Trails
- **Clear world trail** (Content Live and Nexus Options) deletes a GPS that is not in any Trails window and turns pack overlay off
- After Stop, walking does not add points. Start on a trail that already has vectors does not insert a section or extra vertex (use New Segment / Insert Vector). Empty trails still get a first vector on Start
- New Segment: TacO `0,0,0` break + current MapID + vector at feet
- Draft vertices drawn as clickable circles; Select Nearest / Move to Feet move the UberTool onto that vector
- `.trl` stays with the pop-out until **Write into OverlayData** / desk Add to project inserts it into OverlayData
- Per-`<Trail>` attrs (texture, trailScale, fade, alpha, animSpeed)
- Geometry: reverse / densify / smooth / multi-select (Ctrl/Shift)
- Snapshot undo (Ctrl+Z) on TrailsN
- Live draft preview follows the focused TrailsN trail (or all open TrailsN when that option is on)

### Stability (Wine / multi-addon)
- Crash-Logs snapshots only when the exception address is inside `GW2-TrailTools.dll`
- World GPS draw saves/restores blend, depth, raster, IA, shaders, and samplers after ribbons
- Nested pad scroll thumbs stay clipped to their child panels

### Keybinds
- Trail record / pause / section / delete-segment chords
- Marker delete + place-slot chords (Ctrl+Numpad-style)

---

## Branding & version

| | |
|--|--|
| Version | **1.0.0.1** (`AddonVersion.h`) |
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
- Collapsed **Pack** on Content lists `.lua` under the authoring folder (`Scripts/` recommended).
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
