"""Read Chip's Challenge v1 level packs (CHIPS.DAT)."""

from __future__ import annotations

import argparse
import json
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any


BOARD_WIDTH = 32
BOARD_HEIGHT = 32
BOARD_CELLS = BOARD_WIDTH * BOARD_HEIGHT
SIGNATURE = 0xAAAC
FORMAT_VERSION = 2


@dataclass(frozen=True)
class Level:
    number: int
    time_limit: int
    chips_required: int
    map_type: int
    top: bytes
    bottom: bytes
    fields: dict[str, Any]
    raw_fields: tuple[tuple[int, bytes], ...] = ()

    def to_dict(self) -> dict[str, Any]:
        result = asdict(self)
        result["top"] = list(self.top)
        result["bottom"] = list(self.bottom)
        result["raw_fields"] = [
            [field_id, list(value)] for field_id, value in self.raw_fields
        ]
        return result


class LevelPackError(ValueError):
    """Raised when a level pack is truncated or malformed."""


def _u16(data: bytes, offset: int, context: str) -> int:
    if offset < 0 or offset + 2 > len(data):
        raise LevelPackError(f"Truncated {context} at byte {offset}")
    return int.from_bytes(data[offset : offset + 2], "little")


def _read_sized(data: bytes, offset: int, context: str) -> tuple[bytes, int]:
    size = _u16(data, offset, f"{context} length")
    start = offset + 2
    end = start + size
    if end > len(data):
        raise LevelPackError(f"Truncated {context}: expected {size} bytes")
    return data[start:end], end


def _expand_rle(encoded: bytes, context: str) -> bytes:
    """Expand the game's byte RLE: literals, or FF count value triples."""
    output = bytearray()
    cursor = 0
    while cursor < len(encoded):
        value = encoded[cursor]
        cursor += 1
        if value == 0xFF:
            if cursor + 2 > len(encoded):
                raise LevelPackError(f"Incomplete RLE run in {context}")
            count, value = encoded[cursor], encoded[cursor + 1]
            cursor += 2
            # The original decoder's 8-bit remaining counter wraps: count 0
            # means the first tile plus 255 repeats, or 256 tiles total.
            if count == 0:
                count = 256
            output.extend([value] * count)
        else:
            output.append(value)
        if len(output) > BOARD_CELLS:
            raise LevelPackError(f"{context} expands beyond {BOARD_CELLS} tiles")
    if len(output) != BOARD_CELLS:
        raise LevelPackError(
            f"{context} expands to {len(output)} tiles; expected {BOARD_CELLS}"
        )
    return bytes(output)


def _parse_fields(
    raw: bytes, raw_fields: list[tuple[int, bytes]] | None = None
) -> dict[str, Any]:
    fields: dict[str, Any] = {}
    names = {
        1: "time_limit",
        2: "chips_required",
        3: "title",
        4: "traps",
        5: "clones",
        6: "password",
        7: "hint",
        8: "password",
        10: "monsters",
    }
    offset = 0
    while offset < len(raw):
        if offset + 2 > len(raw):
            raise LevelPackError("Truncated optional field header")
        field_id, size = raw[offset], raw[offset + 1]
        offset += 2
        end = offset + size
        if end > len(raw):
            raise LevelPackError(f"Truncated optional field {field_id}")
        value = raw[offset:end]
        offset = end
        if raw_fields is not None:
            raw_fields.append((field_id, value))
        key = names.get(field_id)
        if key is None:
            continue
        if field_id in (1, 2):
            if size < 2:
                raise LevelPackError(f"Optional metadata field {field_id} is shorter than a word")
            fields[key] = int.from_bytes(value[:2], "little")
        elif field_id in (3, 6, 7, 8):
            text = value.split(b"\0", 1)[0]
            if field_id == 6:
                text = bytes(byte ^ 0x99 for byte in text)
            fields[key] = text.decode("cp1252", errors="replace")
        elif field_id in (4, 5, 10):
            fields[key] = list(value)
        else:
            fields[key] = list(value)
    return fields


def parse_levelpack(path: str | Path) -> list[Level]:
    data = Path(path).read_bytes()
    if len(data) < 6 or _u16(data, 0, "signature") != SIGNATURE:
        raise LevelPackError("Not a recognized Chip's Challenge level pack")
    version = _u16(data, 2, "format version")
    if version != FORMAT_VERSION:
        raise LevelPackError(f"Unsupported level pack version {version}")
    count = _u16(data, 4, "level count")
    offset = 6
    levels: list[Level] = []
    for index in range(count):
        chunk, next_offset = _read_sized(data, offset, f"level {index + 1}")
        if len(chunk) < 12:
            raise LevelPackError(f"Level {index + 1} record is too short")
        cursor = 0
        number = _u16(chunk, cursor, "level number")
        cursor += 2
        time_limit = _u16(chunk, cursor, "time limit")
        cursor += 2
        chips_required = _u16(chunk, cursor, "chip count")
        cursor += 2
        map_type = _u16(chunk, cursor, "map type")
        cursor += 2
        if map_type not in (0, 1):
            raise LevelPackError(f"Unsupported map type {map_type} in level {number}")
        top_encoded, cursor = _read_sized(chunk, cursor, f"level {number} top layer")
        bottom_encoded, cursor = _read_sized(
            chunk, cursor, f"level {number} bottom layer"
        )
        optional, cursor = _read_sized(chunk, cursor, f"level {number} fields")
        if cursor != len(chunk):
            raise LevelPackError(f"Unexpected trailing bytes in level {number}")
        raw_fields: list[tuple[int, bytes]] = []
        fields = _parse_fields(optional, raw_fields)
        # DecodeLevelFields processes optional words after the header and lets
        # them override the initial time/chip metadata.
        time_limit = fields.pop("time_limit", time_limit)
        chips_required = fields.pop("chips_required", chips_required)
        levels.append(
            Level(
                number=number,
                time_limit=time_limit,
                chips_required=chips_required,
                map_type=map_type,
                top=_expand_rle(top_encoded, f"level {number} top layer"),
                bottom=_expand_rle(bottom_encoded, f"level {number} bottom layer"),
                fields=fields,
                raw_fields=tuple(raw_fields),
            )
        )
        offset = next_offset
    if offset != len(data):
        raise LevelPackError(f"{len(data) - offset} unexpected bytes after final level")
    return levels


def main() -> None:
    parser = argparse.ArgumentParser(description="Inspect a Chip's Challenge level pack")
    parser.add_argument("levelpack", type=Path, help="path to CHIPS.DAT")
    parser.add_argument("--level", type=int, help="print this level as JSON")
    args = parser.parse_args()
    levels = parse_levelpack(args.levelpack)
    if args.level is None:
        for level in levels:
            title = level.fields.get("title", "")
            print(f"{level.number:3}  {title}")
        return
    match = next((item for item in levels if item.number == args.level), None)
    if match is None:
        parser.error(f"level {args.level} is not present in {args.levelpack}")
    print(json.dumps(match.to_dict(), indent=2))


if __name__ == "__main__":
    main()
