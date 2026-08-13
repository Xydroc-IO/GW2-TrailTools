# Release Notes — GW2-TrailTools 1.0.0

**First standalone release** of Trail Tools as a Raidcore Nexus addon (`GW2-TrailTools`, signature `TRLS`), extracted from [GW2-InGame-Helper](https://github.com/Xydroc-IO/GW2-InGame-Helper).

Toggle: Nexus **QuickAccess** icon or **Alt+Shift+T**.

---

## Highlights

### Authoring hub
- Side-rail tabs: **Pack → Trails → Markers → Live → Keybinds**
- Pack tab owns the OverlayData project (New / Load / Save, Combined vs Split layout)
- Trails / Markers stay in-hub; optional **Pop out** desks and TrailsN / MarkersN editors
- Cartographer UI theme (slate + teal), custom brand QuickAccess icon

### Pack workflow
- Pack name / display name, draft session save/load, reseed categories
- Import installed `.taco` into the draft
- **Build .taco** → `addons/GW2-TrailTools/pathing/`, enable root category, reload Pathing index
- Looks presets, asset browser, category tree editor
- Optional XML preview (combined or menu+data)

### Trails
- Category picker, trail list, record / segment / edit points
- `.trl` New / Load / Save / Save As
- Insert into project XML; live draft preview (world GPS + compass)

### Markers
- Drop / delete at feet, this-map filter, marker attribute editors
- Blish-style `script-*` attributes on POIs (stored in OverlayData)
- Copy markers from currently loaded Pathing packs

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

**Lady Elyssa** — UI feedback and Windows testing.

## Build from source

```bash
make -j"$(nproc)"
make check-lines
make install   # optional
```

Also: `CHANGELOG.md` for a shorter bullet summary.
