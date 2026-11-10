#!/usr/bin/env python3
"""Export a labeled reference sheet for the converted original game tiles."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_ATLAS = PROJECT_ROOT / "assets" / "art" / "tile-atlas.png"
DEFAULT_OUTPUT = PROJECT_ROOT / "docs" / "art" / "tile-index.png"
TILE_SIZE = 64  # Runtime atlas cell size after the source sheet's 2x conversion.
TILE_COUNT = 0x70
COLUMNS = 8
CELL_WIDTH = 92
CELL_HEIGHT = 92
MARGIN = 14
HEADER_HEIGHT = 42


def export_reference(atlas_path: Path, output_path: Path) -> None:
    atlas = Image.open(atlas_path).convert("RGBA")
    if atlas.size != (832, 1024):
        raise ValueError(f"Expected the converted 832x1024 tile atlas, got {atlas.size}")

    rows = (TILE_COUNT + COLUMNS - 1) // COLUMNS
    width = MARGIN * 2 + COLUMNS * CELL_WIDTH
    height = HEADER_HEIGHT + MARGIN + rows * CELL_HEIGHT + MARGIN
    sheet = Image.new("RGB", (width, height), (35, 39, 47))
    draw = ImageDraw.Draw(sheet)
    font = ImageFont.load_default()
    draw.text((MARGIN, 12), "Original game tiles - IDs 0x00-0x6F", fill=(245, 245, 245), font=font)

    for tile_id in range(TILE_COUNT):
        column = tile_id >> 4
        row = tile_id & 0x0F
        sprite = atlas.crop((column * TILE_SIZE, row * TILE_SIZE,
                             (column + 1) * TILE_SIZE, (row + 1) * TILE_SIZE))

        sheet_column = tile_id % COLUMNS
        sheet_row = tile_id // COLUMNS
        left = MARGIN + sheet_column * CELL_WIDTH
        top = HEADER_HEIGHT + MARGIN + sheet_row * CELL_HEIGHT
        draw.rectangle((left, top, left + CELL_WIDTH - 1, top + CELL_HEIGHT - 1),
                       fill=(192, 192, 192), outline=(95, 102, 115))
        sheet.paste(sprite, (left + (CELL_WIDTH - TILE_SIZE) // 2, top + 2), sprite)
        label = f"0x{tile_id:02X}"
        box = draw.textbbox((0, 0), label, font=font)
        text_x = left + (CELL_WIDTH - (box[2] - box[0])) // 2
        draw.text((text_x, top + TILE_SIZE + 7), label, fill=(15, 18, 22), font=font)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(output_path, format="PNG", optimize=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--atlas", type=Path, default=DEFAULT_ATLAS,
                        help="Converted original tile atlas (default: assets/art/tile-atlas.png)")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT,
                        help="Reference-sheet output path (default: docs/art/tile-index.png)")
    args = parser.parse_args()
    export_reference(args.atlas, args.output)
    print(f"Wrote {TILE_COUNT} labeled tile cells to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
