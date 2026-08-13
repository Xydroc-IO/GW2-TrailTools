#!/usr/bin/env python3
"""Bake assets/trailtools-icon*.png into src/ui/TrailToolsIcon.h for QuickAccess."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ICON = ROOT / "assets" / "trailtools-icon.png"
HOVER = ROOT / "assets" / "trailtools-icon-hover.png"
OUT = ROOT / "src" / "ui" / "TrailToolsIcon.h"


def emit(path: Path, arr: str, len_name: str) -> str:
	data = path.read_bytes()
	lines = [f"static const unsigned char {arr}[] = {{"]
	for i in range(0, len(data), 12):
		chunk = data[i : i + 12]
		lines.append("\t" + ", ".join(f"0x{b:02x}" for b in chunk) + ",")
	lines.append("};")
	lines.append(f"static const unsigned {len_name} = {len(data)};")
	return "\n".join(lines)


def main() -> None:
	if not ICON.is_file() or not HOVER.is_file():
		raise SystemExit(f"Missing {ICON} and/or {HOVER}")
	body = [
		"#pragma once",
		"/* Baked QuickAccess PNGs (assets/trailtools-icon*.png). Do not edit by hand —",
		"   regenerate via tools/bake_icons.py after changing assets. */",
		"",
		emit(ICON, "kTrailToolsIconPng", "kTrailToolsIconPng_len"),
		"",
		emit(HOVER, "kTrailToolsIconHoverPng", "kTrailToolsIconHoverPng_len"),
		"",
	]
	OUT.write_text("\n".join(body), encoding="utf-8")
	print(f"Wrote {OUT.relative_to(ROOT)} ({OUT.stat().st_size} bytes)")


if __name__ == "__main__":
	main()
