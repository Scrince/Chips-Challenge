#!/usr/bin/env python3
"""Check all reference map SVG tile crops against the campaign level pack."""

from __future__ import annotations

import argparse
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


CELL_TITLE = re.compile(r"^\((\d+),\s*(\d+)\)")
VIEW_BOX = re.compile(r"^([\d.]+)\s+([\d.]+)\s+32\s+32$")


def source_tile(view_box: str, svg_path: Path, cell: int) -> int:
    match = VIEW_BOX.match(view_box)
    if not match:
        raise ValueError(f"{svg_path}: cell {cell} has unexpected viewBox {view_box!r}")
    x, y = map(float, match.groups())
    if x % 32 or y % 32:
        raise ValueError(f"{svg_path}: cell {cell} has non-tile source crop {view_box!r}")
    return (int(x // 32) << 4) | int(y // 32)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("levelpack", type=Path)
    parser.add_argument("gallery", type=Path)
    args = parser.parse_args()

    sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "cc-speedrun"))
    from ccspeedrun.levelpack import parse_levelpack  # type: ignore[import-not-found]

    levels = parse_levelpack(args.levelpack)
    failures: list[str] = []
    pngs = list(args.gallery.glob("*.png"))
    if len(pngs) != len(levels):
        failures.append(f"gallery has {len(pngs)} PNGs for {len(levels)} campaign levels")

    for level in levels:
        svg = args.gallery.parent / "map-images" / f"{level.number:03d}-*.svg"
        candidates = list(svg.parent.glob(svg.name))
        if len(candidates) != 1:
            failures.append(f"level {level.number}: expected one SVG reference, found {len(candidates)}")
            continue
        svg_path = candidates[0]
        root = ET.parse(svg_path).getroot()
        seen: set[int] = set()
        for group in root.iter():
            title = next((child.text for child in group if child.tag.endswith("title")), None)
            match = CELL_TITLE.match(title or "")
            if not match:
                continue
            x, y = map(int, match.groups())
            if not (0 <= x < 32 and 0 <= y < 32):
                continue
            index = y * 32 + x
            if index in seen:
                continue
            seen.add(index)
            crops = [child.attrib["viewBox"] for child in group
                     if child.tag.endswith("svg") and "viewBox" in child.attrib]
            expected = [level.bottom[index] if level.bottom[index] != 0 else 0]
            if level.top[index] != 0:
                expected.append(level.top[index])
            try:
                actual = [source_tile(view_box, svg_path, index) for view_box in crops]
            except (KeyError, ValueError) as error:
                failures.append(str(error))
                continue
            if actual != expected:
                failures.append(
                    f"level {level.number} ({x},{y}): gallery crops {actual}, pack layers {expected}")
        if len(seen) != 1024:
            failures.append(f"level {level.number}: found {len(seen)} of 1024 map cells in SVG")

    if failures:
        print("Gallery map validation FAILED:")
        print("\n".join(failures[:100]))
        if len(failures) > 100:
            print(f"... and {len(failures) - 100} more")
        return 1
    print(f"Validated {len(levels)} maps: every SVG cell uses the CHIPS.DAT background/foreground tile IDs.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
