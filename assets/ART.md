# Original game artwork

The runtime uses the user-supplied Windows game's original indexed-color art. `scripts/import_original_art.py` converts the original `OBJ32_4.bmp`, `200.bmp`, `BACKGROUND.bmp`, `CHIPEND.bmp`, and `chips.ico` into RGBA PNG/ICO assets. The original inputs remain read-only under `../CHIPS.EXE-master/CHIPS.EXE-master/`.

Run `python scripts/import_original_art.py` from the project root to recreate the runtime files and `ASSET_MANIFEST.json`. The import uses Pillow, retains source colors and pixel shapes, and enlarges pixel art with nearest-neighbor sampling. Manifest hashes and source paths make the conversion reproducible.

## Gameplay sprite rendering

The original tile sheet has 13 columns by 16 rows of 32×32 cells. For tile ID `t`, its cell is column `t >> 4`, row `t & 0x0f`. The runtime atlas is an exact 2× nearest-neighbor conversion (64×64 cells), used for terrain and opaque artwork.

Original transparent sprites use the game's `DrawTile` composition: image cell `t + 0x30`, mask cell `t + 0x60`, for sprite IDs `0x40`–`0x6F`. The importer applies the original black/white/yellow mask exactly, then enlarges each 32×32 sprite 8× into `original-transparent-sprites.png`. This exact 6×8 conversion remains the fallback for every sprite ID and supplies Chip's four land poses.

The runtime also uses the checked-in cleaned creature sheet for IDs `0x40`–`0x63` and cleaned key/boot sheet for IDs `0x64`–`0x6B`. Their row-major IDs match the original source sheet; they replace only the pixels drawn in a tile or inventory slot and never edit map data. The item atlas is 4×2; the creature atlas is 6×6. All sheets use nearest-neighbor sampling. The importer records their hashes and original sheet provenance while leaving the cleanup images untouched.

Swimming Chip uses original opaque atlas tiles `0x3C`–`0x3F`, which include the matching water and flipper pose. The board renderer keeps the game's upper/lower layer rules; Level 3's boot pickup positions are validated against the original pack.

## Interface art

The title background source is the original `BACKGROUND.bmp` circuit board. Win cards use original `CHIPEND.bmp`; the scoreboard uses original `200.bmp` with its black field made transparent. `app-icon.png` and `app-icon.ico` retain the exact original icon. The window and Windows executable use the checked-in cleaned variants `app-icon-polished.png` and `app-icon-polished.ico`, mapped from the same original image and included in the manifest with hashes.

The menu and gameplay use `menu-background-hd.png` as a smooth, full-window circuit-board backdrop behind the menu card, board, and scoreboard. It is a high-resolution generated recreation of the original background; `menu-background.png` remains unchanged as the source/reference image. The generated background is linearly filtered when scaled to the window and has its own manifest hash.

Interface text uses the bundled Noto Sans variable font (`fonts/NotoSans.ttf`) rendered with SDL3_ttf and light hinting. This replaces the custom block bitmap face for menus, prompts, dialogs, help, and result cards. The original digital scoreboard numerals and pixel-art sprites remain unchanged. Noto Sans is distributed under the SIL Open Font License; the license is included in `fonts/OFL.txt`.

The renderer uses nearest-neighbor scaling for original pixel art. It does not ship the prior image-generated player, key, boot, or creature replacements. Source files, executable, disassembly, and original audio are not runtime dependencies. Audio remains synthesized by the project and documented in `AUDIO.md`.
