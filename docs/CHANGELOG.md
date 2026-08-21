# Changelog

Full player-facing notes: [`RELEASE_NOTES.md`](RELEASE_NOTES.md).

## Unreleased

## 1.0.0.1

- Content hub: pack-world toggles / project lists removed; Pose **COPY** inserts at OverlayData caret; **Write into OverlayData**; Fill from draft only on the XML editor; `PadNav` action-button contrast
- Pack on Content slimmed to identity + Import/Build (Lua parked); Looks + default textures under Nexus Options; Categories tree removed
- TrailsN: Start/Stop/Pause stay on the recording window; focusing another TrailsN switches preview/recorder without waiting to walk; **All TrailsN** Live toggle (persisted); per-trail texture picker in Attrs
- MarkersN: sticky per-window POI; Data **Copy XML**; Settings uses the same Options gear as hub Keybinds; selected rail icons render brighter
- OverlayData XML: Tab inserts 3 spaces (TacO indent; works under Nexus via VK_TAB)

## 1.0.0

First standalone release of **GW2-TrailTools** (extracted from GW2-InGame-Helper Trail Tools).

- Raidcore Nexus addon (`TRLS`) with Pack / Trails / Markers / Live / Keybinds hub
- OverlayData authoring, TrailsN / MarkersN editors, import/build `.taco`
- Draft world GPS preview (SwapChain D3D + ImGui fallback)
- Addon-polled keybinds, Nexus QuickAccess brand icon
