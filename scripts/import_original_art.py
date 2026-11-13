#!/usr/bin/env python3
"""Convert the supplied Windows game artwork to crisp 32-bit PNG assets."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT.parent / "CHIPS.EXE-master" / "CHIPS.EXE-master"
RESOURCE = SOURCE / "res"
OUT = ROOT / "assets" / "art"
MANIFEST = ROOT / "assets" / "ASSET_MANIFEST.json"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def save_scaled(source: Path, destination: Path, scale: int,
                transparent_palette_index: int | None = None) -> None:
    with Image.open(source) as original:
        image = original.convert("RGBA")
        if transparent_palette_index is not None:
            pixels = original.tobytes()
            alpha = Image.new("L", original.size)
            alpha.putdata([0 if pixel == transparent_palette_index else 255 for pixel in pixels])
            image.putalpha(alpha)
    if scale != 1:
        image = image.resize((image.width * scale, image.height * scale), Image.Resampling.NEAREST)
    image.save(destination, optimize=False)


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    tile_source = RESOURCE / "OBJ32_4.bmp"
    background_source = RESOURCE / "BACKGROUND.bmp"
    victory_source = RESOURCE / "CHIPEND.bmp"
    digits_source = RESOURCE / "200.bmp"
    scoreboard_source = RESOURCE / "INFOWND.bmp"
    icon_source = SOURCE / "chips.ico"

    save_scaled(tile_source, OUT / "tile-atlas.png", 2)
    # Preserve the original circuit-board title backdrop at a crisp 2x scale.
    save_scaled(background_source, OUT / "menu-background.png", 2)
    save_scaled(victory_source, OUT / "victory-illustration.png", 2)
    # Keep the original metal information-window frame, labels, readout wells,
    # and corner bolts crisp for the native scoreboard.
    save_scaled(scoreboard_source, OUT / "scoreboard-panel.png", 2)
    # Palette index 0 is the original strip's black field; make that field
    # transparent so the green glyphs sit cleanly on the native HUD's black.
    save_scaled(digits_source, OUT / "digital-digits.png", 2,
                transparent_palette_index=0)
    save_scaled(icon_source, OUT / "app-icon.png", 8)
    # Reconstruct every original transparent DrawTile sprite (0x40..0x6F)
    # from its source image cell (tile + 0x30) and binary mask cell
    # (tile + 0x60). This preserves the exact 16-color art and transparency.
    original_sprites = Image.new("RGBA", (1536, 2048), (0, 0, 0, 0))
    with Image.open(tile_source) as original:
        original = original.convert("RGBA")
        for frame, tile in enumerate(range(0x40, 0x70)):
            image_tile = tile + 0x30
            left = (image_tile >> 4) * 32
            top = (image_tile & 0x0F) * 32
            sprite = original.crop((left, top, left + 32, top + 32))
            mask_tile = tile + 0x60
            mask_left = (mask_tile >> 4) * 32
            mask_top = (mask_tile & 0x0F) * 32
            mask = original.crop((mask_left, mask_top, mask_left + 32, mask_top + 32))
            pixels = sprite.load()
            mask_pixels = mask.load()
            for y in range(32):
                for x in range(32):
                    mask_color = mask_pixels[x, y]
                    if mask_color[:3] not in (
                        (0, 0, 0), (255, 255, 255), (255, 255, 0)
                    ):
                        raise ValueError(
                            f"Original transparency mask 0x{mask_tile:02X} has an unknown color")
                    red, green, blue, _ = pixels[x, y]
                    pixels[x, y] = (red, green, blue, 0 if mask_color[:3] == (0, 0, 0) else 255)
            sprite = sprite.resize((256, 256), Image.Resampling.NEAREST)
            original_sprites.paste(sprite, ((frame % 6) * 256, (frame // 6) * 256))
    original_sprites.save(OUT / "original-transparent-sprites.png", optimize=False)

    # Re-read the saved sheet and compare every frame pixel with its original
    # DrawTile image/mask pair. This catches frame-order, crop, scale, and PNG
    # serialization mistakes before the manifest is refreshed.
    with Image.open(tile_source) as original, Image.open(OUT / "original-transparent-sprites.png") as generated:
        original = original.convert("RGBA")
        generated = generated.convert("RGBA")
        if generated.size != (1536, 2048):
            raise ValueError(
                "Original transparent sprite sheet must be a 6x8 grid of 256px frames "
                f"(got {generated.width}x{generated.height})")
        for frame, tile in enumerate(range(0x40, 0x70)):
            image_tile = tile + 0x30
            image_left = (image_tile >> 4) * 32
            image_top = (image_tile & 0x0F) * 32
            expected = original.crop((image_left, image_top, image_left + 32, image_top + 32))
            mask_tile = tile + 0x60
            mask_left = (mask_tile >> 4) * 32
            mask_top = (mask_tile & 0x0F) * 32
            mask = original.crop((mask_left, mask_top, mask_left + 32, mask_top + 32))
            expected_pixels = expected.load()
            mask_pixels = mask.load()
            for y in range(32):
                for x in range(32):
                    mask_color = mask_pixels[x, y][:3]
                    if mask_color not in ((0, 0, 0), (255, 255, 255), (255, 255, 0)):
                        raise ValueError(
                            f"Original transparency mask 0x{mask_tile:02X} has an unknown color")
                    red, green, blue, _ = expected_pixels[x, y]
                    expected_pixels[x, y] = (
                        red, green, blue, 0 if mask_color == (0, 0, 0) else 255)
            expected = expected.resize((256, 256), Image.Resampling.NEAREST)
            frame_left = (frame % 6) * 256
            frame_top = (frame // 6) * 256
            actual = generated.crop((frame_left, frame_top, frame_left + 256, frame_top + 256))
            if actual.tobytes() != expected.tobytes():
                raise ValueError(
                    f"Sprite frame for tile 0x{tile:02X} does not match its original DrawTile cells")

    records = []
    for asset_path, source_path, description in (
        (OUT / "tile-atlas.png", tile_source, "Original 16-color game tile sheet; converted to RGBA and enlarged 2x with nearest-neighbor sampling."),
        (OUT / "menu-background.png", background_source, "Original green circuit-board title background; converted to RGBA and enlarged 2x with nearest-neighbor sampling."),
        (OUT / "victory-illustration.png", victory_source, "Original CHIPEND victory illustration; converted to RGBA and enlarged 2x with nearest-neighbor sampling."),
        (OUT / "scoreboard-panel.png", scoreboard_source, "Original INFOWND scoreboard panel with its steel frame, red labels, recessed readout wells, and four corner bolts; converted to RGBA and enlarged 2x with nearest-neighbor sampling."),
        (OUT / "digital-digits.png", digits_source, "Original scoreboard digit strip; black palette-index 0 converted to transparent RGBA, then enlarged 2x with nearest-neighbor sampling."),
        (OUT / "app-icon.png", icon_source, "Original Windows game icon; converted to RGBA and enlarged 8x with nearest-neighbor sampling."),
    ):
        with Image.open(asset_path) as image:
            dimensions = f"{image.width}x{image.height}"
        records.append({
            "path": asset_path.relative_to(ROOT / "assets").as_posix(),
            "format": f"PNG, RGBA, {dimensions}",
            "provenance": description,
            "source": source_path.relative_to(ROOT.parent).as_posix(),
            "source_sha256": sha256(source_path),
            "sha256": sha256(asset_path),
        })
    game_background_path = OUT / "menu-background-hd.png"
    if game_background_path.is_file():
        with Image.open(game_background_path) as image:
            dimensions = f"{image.width}x{image.height}"
            background_mode = image.mode
        original_background = OUT / "menu-background.png"
        records.append({
            "path": game_background_path.relative_to(ROOT / "assets").as_posix(),
            "format": f"PNG, {background_mode}, {dimensions}",
            "provenance": "High-resolution generated recreation of the original green circuit-board title background, with smooth antialiased traces for full-window scaling behind gameplay.",
            "source": "assets/art/menu-background.png",
            "source_sha256": sha256(original_background),
            "sha256": sha256(game_background_path),
        })
    original_icon = OUT / "app-icon.png"
    with Image.open(original_icon) as image:
        image.save(OUT / "app-icon.ico", format="ICO",
                   sizes=[(16, 16), (24, 24), (32, 32), (48, 48),
                          (64, 64), (128, 128), (256, 256)])
    records.append({
        "path": "art/app-icon.ico",
        "format": "ICO, 16/24/32/48/64/128/256 pixel variants",
        "provenance": "Windows multi-resolution icon generated from the supplied original game icon PNG with nearest-neighbor resizing; embedded as a Windows resource by CMake.",
        "source": "assets/art/app-icon.png",
        "source_sha256": sha256(original_icon),
        "sha256": sha256(OUT / "app-icon.ico"),
    })

    original_sprite_path = OUT / "original-transparent-sprites.png"
    with Image.open(original_sprite_path) as image:
        dimensions = f"{image.width}x{image.height}"
    records.append({
        "path": original_sprite_path.relative_to(ROOT / "assets").as_posix(),
        "format": f"PNG, RGBA, {dimensions}, 6x8 grid of 256x256 frames for tile IDs 0x40-0x6F",
        "provenance": "Exact original DrawTile image/mask composition for every transparent sprite ID 0x40-0x6F; image tile ID+0x30, mask tile ID+0x60; black mask transparent; enlarged 8x using nearest-neighbor sampling.",
        "source": tile_source.relative_to(ROOT.parent).as_posix(),
        "source_sha256": sha256(tile_source),
        "reference": "assets/art/tile-atlas.png image/mask cell pairs",
        "sha256": sha256(original_sprite_path),
    })

    # These checked-in cleanup variants are aligned to the source tile IDs:
    # 36 creature poses in a 6x6 grid (0x40-0x63), then eight key/boot icons
    # in a 4x2 grid (0x64-0x6B). Keep their hashes in the generated manifest
    # so packaged builds cannot silently drop or replace them.
    cleanup_records = (
        (OUT / "app-icon-polished.png", "PNG, RGBA, 256x256",
         "Checked-in cleaned pixel-art variant of the supplied game's icon; used for the native window and high-resolution Windows executable resource. The exact converted original icon remains packaged as a reference.",
         "art/app-icon.png"),
        (OUT / "app-icon-polished.ico", "ICO, multi-resolution polished Windows icon",
         "Checked-in multi-resolution ICO built from the cleaned original-game icon artwork for the Windows executable resource.",
         "art/app-icon-polished.png"),
        (OUT / "player-poses-polished.png", "PNG, RGBA, 2172x724, 4 horizontal 543x724 directional frames for tile IDs 0x6C-0x6F",
         "Checked-in polished land poses derived from the supplied original Chip sprites, ordered north, west, south, east; used by the native renderer with aspect-preserving pixel-crisp scaling.",
         "art/original-transparent-sprites.png"),
        (OUT / "player-swim-poses-polished.png", "PNG, RGBA, 2172x724, 4 horizontal 543x724 directional frames for tile IDs 0x3C-0x3F",
         "Checked-in polished swimming poses derived from the supplied original Chip sprites, ordered north, west, south, east; used by the native renderer with aspect-preserving pixel-crisp scaling.",
         "art/original-transparent-sprites.png"),
        (OUT / "creature-sprites-polished.png", "PNG, RGBA, 1536x1536, 6x6 grid for tile IDs 0x40-0x63",
         "Checked-in cleaned creature artwork, ordered by original creature tile ID; pixel-crisp 256px frames with transparent backgrounds. The exact source sprite sheet remains the runtime fallback.",
         "art/original-transparent-sprites.png"),
        (OUT / "key-boot-icons-polished.png", "PNG, RGBA, 1774x887, 4x2 grid for tile IDs 0x64-0x6B",
         "Checked-in cleaned key and boot artwork, ordered by original item tile ID; transparent square frames. The exact source sprite sheet remains the runtime fallback.",
         "art/original-transparent-sprites.png"),
    )
    for asset_path, asset_format, description, reference in cleanup_records:
        if not asset_path.is_file():
            continue
        records.append({
            "path": asset_path.relative_to(ROOT / "assets").as_posix(),
            "format": asset_format,
            "provenance": description,
            "source": ("assets/" + reference if reference.startswith("art/app-icon")
                else "CHIPS.EXE-master/CHIPS.EXE-master/res/OBJ32_4.bmp"),
            "source_sha256": sha256(ROOT / "assets" / reference) if reference.startswith("art/app-icon") else sha256(tile_source),
            "reference": f"assets/{reference}",
            "sha256": sha256(asset_path),
        })

    font_records = (
        (ROOT / "assets/fonts/NotoSans.ttf", "TrueType variable font",
         "Noto Sans bundled as the runtime interface font under the SIL Open Font License.",
         "https://github.com/google/fonts/blob/main/ofl/notosans/NotoSans%5Bwdth%2Cwght%5D.ttf"),
        (ROOT / "assets/fonts/OFL.txt", "Text license",
         "SIL Open Font License for the bundled Noto Sans font.",
         "https://github.com/google/fonts/blob/main/ofl/notosans/OFL.txt"),
    )
    for asset_path, asset_format, description, source in font_records:
        if not asset_path.is_file():
            continue
        records.append({
            "path": asset_path.relative_to(ROOT / "assets").as_posix(),
            "format": asset_format,
            "provenance": description,
            "source": source,
            "source_sha256": sha256(asset_path),
            "sha256": sha256(asset_path),
        })

    manifest = {
        "schema_version": 5,
        "asset_scope": "runtime artwork, bundled UI font, and original user-supplied Windows game audio",
        "conversion": "python scripts/import_original_art.py (Pillow); original artwork is converted to RGBA and enlarged with nearest-neighbor sampling; every transparent sprite frame reproduces the source DrawTile image/mask pair with image tile+0x30 and mask tile+0x60.",
        "legacy_media_policy": "The supplied original artwork is intentionally used at runtime. The legacy executable, assembly, and original audio files remain outside runtime dependencies.",
        "assets": records,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    for record in records:
        print(f"{record['path']}: {record['format']} (source SHA-256 {record['source_sha256']})")


if __name__ == "__main__":
    main()
