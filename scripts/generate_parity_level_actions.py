#!/usr/bin/env python3
"""Generate a campaign-specific original/native parity action checklist."""

from __future__ import annotations

import argparse
import json
import subprocess
from collections import defaultdict
from pathlib import Path


# Tile IDs from CHIPS.EXE-master/CHIPS.EXE-master/constants.asm. These are
# behavior families for planning captures, not claims that every listed rule
# has already been proven in the native engine.
FAMILIES: dict[str, tuple[set[int], str]] = {
    "chips, sockets, exit": ({0x02, 0x15, 0x22, 0x39, 0x3A, 0x3B},
        "Compare pickup, requirement decrement, socket/exit gating, completion and terminal input."),
    "colored keys and doors": ({*range(0x16, 0x1A), *range(0x64, 0x68)},
        "Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input."),
    "water, fire and boots": ({0x03, 0x04, 0x33, 0x34, 0x68, 0x69, 0x6A, 0x6B},
        "Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile."),
    "blocks and block terrain": ({0x0A, *range(0x0E, 0x12)},
        "Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through."),
    "dirt and gravel": ({0x0B, 0x2D},
        "Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry."),
    "ice and ice corners": ({0x0C, *range(0x1A, 0x1E)},
        "Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps."),
    "force floors": ({0x0D, *range(0x12, 0x15), 0x32},
        "Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision."),
    "walls and panels": ({0x01, 0x05, *range(0x06, 0x0A), 0x1E, 0x1F, 0x2C, 0x2E, 0x30},
        "Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound."),
    "toggle controls": ({0x23, 0x25, 0x26},
        "Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks."),
    "traps and tank controls": ({0x27, 0x28, 0x2B},
        "Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation."),
    "clone machines": ({0x24, 0x31},
        "Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation."),
    "teleporters": ({0x29},
        "Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound."),
    "bombs and thieves": ({0x2A, 0x21},
        "Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound."),
    "hints": ({0x2F},
        "Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input."),
    "bugs": (set(range(0x40, 0x44)), "Compare turn order, preferred turn, collision/death and movement cadence."),
    "fireballs": (set(range(0x44, 0x48)), "Compare turn order, preferred turn, hazards/boots and movement cadence."),
    "balls": (set(range(0x48, 0x4C)), "Compare turn order, reversal rules, collision and movement cadence."),
    "tanks": (set(range(0x4C, 0x50)), "Compare orientation, control-button response, collision and movement cadence."),
    "gliders": (set(range(0x50, 0x54)), "Compare turn order, water/fire terrain, collision and movement cadence."),
    "teeth": (set(range(0x54, 0x58)), "Compare player tracking, tick gating, tie-breaking, collision and movement cadence."),
    "walkers": (set(range(0x58, 0x5C)), "Compare random-choice sequence/seed, blocked moves, collision and movement cadence."),
    "blobs": (set(range(0x5C, 0x60)), "Compare random movement/seed, speed schedule, blocked moves and collision."),
    "paramecia": (set(range(0x60, 0x64)), "Compare turn order, preferred turn, collision and movement cadence."),
}


def dump_levels(executable: Path, pack: Path) -> list[dict[str, object]]:
    result = subprocess.run([str(executable), str(pack)], check=True, capture_output=True, text=True)
    return json.loads(result.stdout)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dump-tool", type=Path, required=True, help="chips-levelpack-dump executable")
    parser.add_argument("--pack", type=Path, required=True, help="canonical CHIPS.DAT")
    parser.add_argument("--output", type=Path, required=True, help="Markdown report destination")
    args = parser.parse_args()

    levels = dump_levels(args.dump_tool, args.pack)
    if len(levels) != 149:
        raise SystemExit(f"Expected 149 levels, found {len(levels)}")

    lines = [
        "# Campaign parity action checklist",
        "",
        "> Generated from the decoded campaign in `data/CHIPS.DAT`. `NEEDS CAPTURE` means a paired original/native observation is still required; it does not assert a mismatch. Only mark an item `ADJUST` after a reproducible comparison establishes the differing behavior.",
        "",
        "## How to use this checklist",
        "",
        "For every level, run the same initial state and deterministic input trace in the original executable and native build. Capture key-down and key-up timestamps, simulation tick/phase, player and creature positions, both map layers, inventory, remaining chips, timer, outcome, and sound events. Use the exact campaign level data; do not reconstruct a map from its thumbnail. For random behavior, record and replay the native seed and repeat original captures to establish whether the original sequence is deterministic.",
        "",
        "Regenerate after changing the canonical pack with `python scripts/generate_parity_level_actions.py --dump-tool <path-to-chips-levelpack-dump> --pack data/CHIPS.DAT --output docs/campaign-parity-actions.md`.",
        "",
        "Status vocabulary: `NEEDS CAPTURE` = not yet proven; `MATCH` = paired evidence agrees for the named scenario; `ADJUST` = a reproducible mismatch with evidence; `N/A` = mechanic absent from the decoded map. This generated initial report contains no inferred `ADJUST` statuses.",
        "",
        "## Level-by-level actions",
        "",
        "Each level always gets the input/timing protocol below. The feature actions are added from tile IDs actually present in either layer. A tile’s presence identifies a scenario to capture, not full link semantics; inspect link/metadata fields and surrounding tiles before finalizing a trace.",
        "",
    ]

    for level in levels:
        top = bytes.fromhex(str(level["top_hex"]))
        bottom = bytes.fromhex(str(level["bottom_hex"]))
        present = set(top) | set(bottom)
        title = str(level.get("title") or f"Level {level['number']}").replace("|", "\\|")
        lines += [
            f"### Level {int(level['number']):03d} — {title}",
            "",
            f"- Map-backed baseline: **32×32**, time limit **{level['time_limit']}**, chips required **{level['chips_required']}**, password `{level.get('password') or 'none'}`.",
            "- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.",
            "- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.",
        ]
        found = 0
        for family, (tile_ids, action) in FAMILIES.items():
            matched = present & tile_ids
            if matched:
                found += 1
                locations: list[str] = []
                for layer_name, layer in (("top", top), ("bottom", bottom)):
                    cells = [(i % 32 + 1, i // 32 + 1, tile) for i, tile in enumerate(layer)
                             if tile in tile_ids]
                    if cells:
                        preview = ", ".join(f"({x},{y})=0x{tile:02X}" for x, y, tile in cells[:8])
                        suffix = f", … (+{len(cells) - 8})" if len(cells) > 8 else ""
                        locations.append(f"{layer_name}: {preview}{suffix}")
                lines.append(f"- {family} at {'; '.join(locations)} — `NEEDS CAPTURE`: {action}")
        if not found:
            lines.append("- No special family detected by the tile census — `NEEDS CAPTURE`: verify ordinary floor/wall movement, blocked input, tick cadence, timer and restart/load phase against the original.")
        lines.append("")

    lines += [
        "## Existing evidence to reuse",
        "",
        "- `tests/fixtures/README.md` documents eight native-winning routes; Level 2 has a 53-move original-played route with captured key-down timestamps and a recorded 18-tick initial phase. That phase is a fixture parameter, not a universal original startup value.",
        "- `tests/fixtures/level2-key-capture.json` and `scripts/replay_recorded_key_events.py` provide one reproducible physical-key timing case. Extend this method to all levels and input edge cases; do not treat a route win as proof of complete rules parity.",
        "- `tests/fixtures/native-known-routes.json` supplies routes for selected levels. They are useful scenario seeds, but only the Level 2 route is documented as played in the original executable; native route success alone is not an original/native comparison.",
        "- Consult `T:/ChipsChallenge/CHIPS.EXE-master/CHIPS.EXE-master/{movement.asm,logic.asm,constants.asm,seg2.asm}` for authoritative scheduling, movement eligibility, key routing and tile IDs. Use the original application for observations where the disassembly is ambiguous.",
        "",
        "## Completion rule",
        "",
        "Do not report the campaign as parity-complete while any campaign mechanic has only a code-reading argument or native-only replay. Every observed mismatch must point to an original capture, native replay, exact input/tick trace, differing state/event, a focused regression scenario, and the implementation location to adjust. Preserve uncertain and untested behavior as `NEEDS CAPTURE`.",
        "",
    ]

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(lines), encoding="utf-8")
    print(f"Wrote checklist for {len(levels)} levels to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
