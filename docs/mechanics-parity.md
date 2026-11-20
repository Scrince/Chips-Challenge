# Mechanics parity review

This guide describes how to record behavior differences between the native game and the original. It is a working review process, not a claim that the full game has been verified.

## Evidence labels

- **Match** — a specific scenario was observed in both versions and the outcomes agree.
- **Adjust** — paired evidence shows a reproducible difference. Record the original and native results, the input sequence, and the regression test needed.
- **Needs capture** — evidence is incomplete or ambiguous. Describe what must be recorded before drawing a conclusion.
- **Not present** — the mechanic does not occur in that level. This is not a parity pass.

A level being winnable, a source-code comparison, or agreement with another simulator does not by itself prove that a mechanic matches.

## Comparison procedure

1. Use `data/CHIPS.DAT` as the authority for level maps and metadata. Compare both 32×32 layers and linked-device records; do not reconstruct maps from screenshots.
2. Record the level, build, settings, initial state, and random seed or capture conditions.
3. For timing-sensitive behavior, record each key-down/key-up event and its timestamp. Include held keys, simultaneous inputs, tick phase, and pause/restart transitions when relevant.
4. Replay the same sequence in the native engine and compare player and creature positions, directions, map layers, inventory, timer, sound events, and outcome.
5. For each confirmed result, add a focused regression and note its evidence. If the original cannot be captured, leave the item as **Needs capture**.

Use short cases at timing boundaries as well as complete routes. Cover movement, hazards, blocks, ice and force floors, buttons, traps, clone machines, teleporters, creatures, layer overlaps, and terminal states. Random behavior requires recorded original evidence; a matching native seed alone is insufficient.

## Current evidence

The generated [campaign action checklist](campaign-parity-actions.md) lists level-specific scenarios and capture gaps. Focused source comparisons and regression history are in [implementation history](implementation-history.md). The current open review areas include unusual combinations of mechanics and any level-specific scenario still marked **Needs capture**.