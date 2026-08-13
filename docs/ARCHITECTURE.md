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
      ├─ RT_Render → TrailToolsPad + optional draft preview
      └─ RT_OptionsRender → settings
```

## Source domains

```text
src/
  entry*.cpp          Nexus GetAddonDef / load / unload
  app/                Globals, Settings, AddonPaths, AspectLayout, UiScale, EiRuntime, AddonVersion
  ui/                 Gw2Ui, PadNav, PadDock, HelperTheme, UI_Render, QuickAccess + TrailToolsIcon.h
  pathing/            PathingTrails runtime index
    packs/            PathingParse (.taco / XML / .trl)
    world/            WorldGpsMath + D3D/ImGui preview
  trailtools/         Authoring pad
assets/               Brand PNGs (icon / hover / logo) — bake with tools/bake_icons.py
```

## Module size

Prefer **≤500 lines** per `.cpp`. Split pad vs state vs binds vs parse. Generated / blob headers (`TrailToolsIcon.h`) exempt. Enforce with `make check-lines`.

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
Denied: game memory R/W, Present hooks, writes into `bin64/cef`.

## Pack runtime

`PathingTrails::Update` indexes `addons/GW2-TrailTools/pathing/*.taco` and loads
current-map markers for **Copy from loaded Pathing**. Category enables persist in
settings.
