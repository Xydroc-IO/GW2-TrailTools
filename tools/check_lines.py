#!/usr/bin/env python3
"""Fail if any src/**/*.cpp exceeds 500 lines (strict domain rule)."""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LIMIT = 500
bad: list[tuple[Path, int]] = []

for path in sorted((ROOT / "src").rglob("*.cpp")):
    lines = sum(1 for _ in path.open("r", encoding="utf-8", errors="replace"))
    if lines > LIMIT:
        bad.append((path.relative_to(ROOT), lines))

if bad:
    print(f"STRICT: .cpp files must be ≤{LIMIT} lines (split by domain concern):")
    for p, n in bad:
        print(f"  {n:4d}  {p}")
    sys.exit(1)

print(f"OK: all src/**/*.cpp ≤ {LIMIT} lines")
