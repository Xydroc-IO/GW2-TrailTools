# Architecture — GW2-TrailTools

Standalone Raidcore Nexus addon extracted from GW2-InGame-Helper **Trail Tools**.

| Field | Value |
|-------|-------|
| Addon name | `GW2-TrailTools` |
| Signature | `0x54524C53` (`TRLS`) |
| Version | **1.0.0** (`AddonVersion.h`) |
| Module size rule | **≤500 lines** per `src/**/*.cpp` |

## One-line summary

Nexus loads `GW2-TrailTools.dll`; ImGui Trail Tools pad authors TacO/Blish packs under `addons/GW2-TrailTools/pathing/authoring/`, builds `.taco` into `pathing/`. MumbleLink supplies map/pose for Live + draft preview. No CEF, no Present hooks.

## Process model

```text
Guild Wars 2.exe
 └─ Nexus loads GW2-TrailTools.dll
      ├─ QuickAccess (baked trailtools-icon.png) → KB_TRLS_TOGGLE
      ├─ RT_Render → TrailToolsPad + PackEdit + UberTool + optional world preview
      ├─ WndProc → WorldClick (empty-world LBUTTONDOWN; swallow when UberTool grabs)
      └─ RT_OptionsRender → settings
```

## Source domains

```text
src/
  entry*.cpp          Nexus GetAddonDef / load / unload
  app/                Globals, Settings, AddonPaths, AspectLayout, UiScale, EiRuntime, AddonVersion
  ui/                 Gw2Ui, PadNav, PadDock, HelperTheme, UI_Render, QuickAccess, WorldClick
  pathing/            PathingTrails runtime index
    packs/            PathingParse (.taco / XML / .trl)
    world/            WorldGpsMath + D3D/ImGui preview
  packedit/           In-place .taco document (tree / details / zip save / world gizmo)
  trailtools/         Authoring pad (Pack / Content / Keybinds + TrailsN / MarkersN)
assets/               Brand PNGs (icon / hover / logo) — bake with tools/bake_icons.py
```

## Module size

Prefer **≤500 lines** per `.cpp`. Split pad vs state vs binds vs parse. Generated / blob headers (`TrailToolsIcon.h`) exempt. Enforce with `make check-lines`.

## Authoring model

Hub tabs: **Content** → Keybinds. **Content** is project lists plus Live pose / UberTool / world click. Pack name / import / **Build .taco** sit in a collapsed Pack section on Content. In-place `.taco` file tools are under Nexus Options → Pack editor.

**Pack editor** (`src/packedit/`) is a separate document from the Pack/Content draft. Open a `.taco` or folder from Nexus Options → Pack editor, then Save. World-draw / gizmo / pop-out / ground-snap checkboxes appear in Nexus Options, the Pack editor header, and Content (same flags, distinct ImGui IDs). **Draw pack in world** starts **off** on open/load so a pack is not painted until that box is checked; overlay still runs if the hub pad is closed. Save **patches POI/Trail tags in the original XML files**. New packs still emit one OverlayData.xml.

Editor lists sit inside the scrolling hub: the mouse wheel moves the **pad**; drag a list scrollbar to move inside it; **Ctrl+wheel** zooms the 2D map. Resource rows are selectable (copy path, jump to a user).

**Ground snap** (`TrailToolsGround.cpp`) fits a local height plane from walked Mumble feet, draft points, and the **open pack**. TacO also uses Mumble (not GW2 process memory); neither has a live collision mesh. Empty ground falls back to feet Y.

TrailsN/MarkersN remain the authoring workbench for new OverlayData.

XML shape follows TacO: nested `<MarkerCategory>`, `<POIs>` with `<Trail trailData="Data/{stem}.trl"/>` and `<POI>`, textures/icons under `Data/Images/`. Default seed is root + leaves `example` / `circle` / `heart` / `square` / `triangle`. TrailsN can copy a **basic** `<Trail type trailData>` tag (TacO TrailsN style).

Live **UberTool** (`TrailToolsUberTool.cpp` + `TrailToolsUberToolDraw.cpp`): click a draft vert/marker (or Select Nearest / Move to Feet); the gizmo stays on that index while recording samples. Drag the white hub to slide on the vertex Y; RGB arrows lock X/Y/Z. Clicks on a handle are swallowed in `WorldClick` so camera look does not steal the drag. Pick rays unproject the same view-projection as world overlay (drag follows the mouse). World click / Shift+click place use the same ground plane when Ground snap is on.

Recording samples on a **time** interval (`Spacing` seconds) and skips duplicate poses while standing still. **Stop** and **Start** are separate ImGui widgets so Stop cannot immediately Start. Start after a stop does not insert a section or extra vector (empty trails still get a first vector). **New Segment** is the only TrailsN control that writes a TacO `0,0,0` break plus MapID + feet vector. Those breaks split world ribbons.

Draft GPS / vertex dots draw only for an **open TrailsN** after **Start** (or Insert Vector / world-click add point). UberTool and Draft preview default **on** (`UberTool` / `DraftPreview` in `settings.ini`). Start with no Trails window opens one and can absorb a leftover hub trail. **Clear world trail** (`ClearWorldDraftTrails` + `PackEdit::HideWorldOverlay`) wipes hub + TrailsN drafts, stops recording, and turns pack overlay off. Closing the last TrailsN stops recording.

**Hide trail near me** (`G::WorldTrailPlayerClearOn`) gates the player-clear bubble; radius remains **Trail player clear** in Nexus Options.

Nexus Options ends with a centered credit block (CREATED BY XYDROC + Ko-fi for Trail Tools).

**XML editor** (`TrailToolsPadXmlEdit.cpp`) is a pop-out OverlayData text buffer. Save writes that text as-is so pack-specific attributes survive; **Apply to editors** parses known TacO/Blish fields into the draft.

## Branding

- QuickAccess: `Textures_GetOrCreateFromMemory` from baked `assets/trailtools-icon*.png` (transparent alpha)
- README logo: `assets/trailtools-logo.png`

## On-disk layout

```text
<GW2>/addons/GW2-TrailTools.dll
<GW2>/addons/GW2-TrailTools/
  settings.ini
  pathing/
    authoring/<PackName>/
    <PackName>.taco
```

## Compliance

Allowed: Nexus APIs, MumbleLink read-only, SwapChain D3D world GPS ribbons.  
Denied: GW2 **process** memory R/W, Present hooks, writes into `bin64/cef`.

## Pack runtime

`PathingTrails::Update` indexes `addons/GW2-TrailTools/pathing/*.taco` and loads
current-map markers for **Copy from loaded Pathing**. Category enables persist in
settings.
