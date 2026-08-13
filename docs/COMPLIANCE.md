# Compliance — GW2-TrailTools

Normative allow/deny for this Nexus addon.

## Allowed

- Raidcore Nexus APIs (ImGui, InputBinds, Paths, Textures, DataLink, Log)
- MumbleLink **read-only** (map id, avatar/camera pose, UI flags)
- Writing only under `addons/GW2-TrailTools/` (settings, authoring workspace, built packs)
- Official HTTPS asset CDN for UI rail icons (`assets.gw2dat.com`) via Nexus texture APIs
- Embedded brand PNGs for QuickAccess (`assets/trailtools-icon*.png` → `Textures_GetOrCreateFromMemory`)

## Denied

- Game process memory read/write for cheating or camera matrices
- `IDXGISwapChain::Present` / `d3d11.dll` hooks
- Writing into Guild Wars 2 `bin64/cef` or other game-owned trees
- Auto-teleport / movement automation

## World GPS

Draft preview draws Blish-style upright ribbons via Nexus **SwapChain** D3D11
(`WorldGpsD3d*`) with ImGui billboard/marker fallback (`WorldGpsImgui`). Device
acquisition uses Nexus `SwapChain` only — no Present hooks.
