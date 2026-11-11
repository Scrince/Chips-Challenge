# Campaign parity action checklist

> Generated from the decoded campaign in `data/CHIPS.DAT`. `NEEDS CAPTURE` means a paired original/native observation is still required; it does not assert a mismatch. Only mark an item `ADJUST` after a reproducible comparison establishes the differing behavior.

## How to use this checklist

For every level, run the same initial state and deterministic input trace in the original executable and native build. Capture key-down and key-up timestamps, simulation tick/phase, player and creature positions, both map layers, inventory, remaining chips, timer, outcome, and sound events. Use the exact campaign level data; do not reconstruct a map from its thumbnail. For random behavior, record and replay the native seed and repeat original captures to establish whether the original sequence is deterministic.

Regenerate after changing the canonical pack with `python scripts/generate_parity_level_actions.py --dump-tool <path-to-chips-levelpack-dump> --pack data/CHIPS.DAT --output docs/campaign-parity-actions.md`.

Status vocabulary: `NEEDS CAPTURE` = not yet proven; `MATCH` = paired evidence agrees for the named scenario; `ADJUST` = a reproducible mismatch with evidence; `N/A` = mechanic absent from the decoded map. This generated initial report contains no inferred `ADJUST` statuses.

## Level-by-level actions

Each level always gets the input/timing protocol below. The feature actions are added from tile IDs actually present in either layer. A tile’s presence identifies a scenario to capture, not full link semantics; inspect link/metadata fields and surrounding tiles before finalizing a trace.

### Level 001 — LESSON 1

- Map-backed baseline: **32×32**, time limit **100**, chips required **11**, password `BDHP`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (13,11)=0x02, (16,11)=0x15, (19,11)=0x02, (16,12)=0x22, (11,14)=0x02, (21,14)=0x02, (14,15)=0x02, (18,15)=0x02, … (+5) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (14,12)=0x18, (18,12)=0x18, (11,13)=0x67, (13,13)=0x16, (19,13)=0x17, (21,13)=0x67, (14,14)=0x64, (18,14)=0x65, … (+7) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- walls and panels at top: (11,9)=0x01, (12,9)=0x01, (13,9)=0x01, (14,9)=0x01, (15,9)=0x01, (17,9)=0x01, (18,9)=0x01, (19,9)=0x01, … (+74) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- hints at top: (16,14)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.

### Level 002 — LESSON 2

- Map-backed baseline: **32×32**, time limit **100**, chips required **4**, password `JXMJ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (13,9)=0x02, (24,12)=0x02, (9,13)=0x15, (10,13)=0x22, (24,14)=0x02, (13,17)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (17,12)=0x03, (18,12)=0x03, (17,13)=0x03, (18,13)=0x03, (17,14)=0x03, (18,14)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (20,13)=0x0A, (21,13)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (10,8)=0x01, (11,8)=0x01, (12,8)=0x01, (13,8)=0x01, (14,8)=0x01, (15,8)=0x01, (16,8)=0x01, (10,9)=0x01, … (+52) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- hints at top: (23,13)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- bugs at top: (14,12)=0x40, (15,13)=0x40, (14,14)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.

### Level 003 — LESSON 3

- Map-backed baseline: **32×32**, time limit **100**, chips required **4**, password `ECBQ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (17,7)=0x15, (17,8)=0x22, (17,12)=0x02, (13,16)=0x02, (20,16)=0x02, (16,20)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (15,11)=0x03, (16,11)=0x03, (17,11)=0x03, (18,11)=0x03, (15,12)=0x03, (16,12)=0x6A, (18,12)=0x03, (15,13)=0x03, … (+16) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (12,14)=0x1A, (13,14)=0x0C, (14,14)=0x0C, (12,15)=0x0C, (12,16)=0x0C, (12,17)=0x1D, (13,17)=0x0C, (14,17)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (13,9)=0x0D, (14,9)=0x14, (15,9)=0x14, (16,9)=0x14, (17,9)=0x14, (18,9)=0x14, (19,9)=0x14, (20,9)=0x14, … (+45) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (16,6)=0x01, (17,6)=0x01, (18,6)=0x01, (16,7)=0x01, (18,7)=0x01, (12,8)=0x01, (13,8)=0x01, (14,8)=0x01, … (+90) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- hints at top: (17,16)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.

### Level 004 — LESSON 4

- Map-backed baseline: **32×32**, time limit **150**, chips required **9**, password `YMCJ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (12,6)=0x02, (15,8)=0x22, (18,8)=0x15, (25,8)=0x02, (25,11)=0x02, (25,13)=0x02, (25,16)=0x02; bottom: (11,15)=0x02, (13,15)=0x02, (11,17)=0x02, (13,17)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at bottom: (12,16)=0x04 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (11,15)=0x0A, (13,15)=0x0A, (12,16)=0x0A, (11,17)=0x0A, (13,17)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (5,5)=0x01, (6,5)=0x01, (7,5)=0x01, (8,5)=0x01, (9,5)=0x01, (10,5)=0x01, (11,5)=0x01, (12,5)=0x01, … (+130) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (23,7)=0x26, (22,9)=0x23, (20,10)=0x25, (23,11)=0x25, (19,12)=0x23, (23,13)=0x25, (20,14)=0x26, (22,15)=0x23, … (+1) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (6,7)=0x28, (12,9)=0x28, (6,11)=0x28 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- hints at top: (15,11)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- tanks at top: (9,6)=0x4C, (9,10)=0x4C — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.

### Level 005 — LESSON 5

- Map-backed baseline: **32×32**, time limit **100**, chips required **0**, password `TQKB`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (12,6)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (15,13)=0x17, (15,18)=0x65 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (14,18)=0x03, (16,18)=0x04, (14,19)=0x03, (16,19)=0x04, (14,20)=0x03, (16,20)=0x04 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- walls and panels at top: (11,5)=0x01, (12,5)=0x01, (13,5)=0x01, (14,5)=0x01, (15,5)=0x01, (16,5)=0x01, (17,5)=0x01, (18,5)=0x01, … (+71) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (17,14)=0x23, (17,16)=0x25 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (17,8)=0x27, (19,8)=0x2B, (17,11)=0x27, (19,11)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (19,16)=0x24; bottom: (19,19)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (13,6)=0x2A, (16,7)=0x2A, (17,7)=0x2A, (18,7)=0x2A, (18,8)=0x2A, (16,9)=0x2A, (17,9)=0x2A, (18,9)=0x2A, … (+7) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- hints at top: (21,19)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- fireballs at top: (19,19)=0x45 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (18,16)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- gliders at top: (19,12)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 006 — LESSON 6

- Map-backed baseline: **32×32**, time limit **100**, chips required **4**, password `WNLD`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (10,13)=0x02, (24,13)=0x02, (10,14)=0x02, (24,14)=0x02, (10,15)=0x02, (24,15)=0x02, (10,16)=0x02, (24,16)=0x02, … (+2) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (9,9)=0x1F, (10,9)=0x1F, (11,9)=0x1F, (12,9)=0x1F, (13,9)=0x1F, (14,9)=0x1F, (15,9)=0x1F, (16,9)=0x1F, … (+75) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- hints at top: (17,14)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.

### Level 007 — LESSON 7

- Map-backed baseline: **32×32**, time limit **150**, chips required **3**, password `FXQO`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (12,10)=0x02, (16,10)=0x02, (20,18)=0x22, (21,18)=0x15, (21,20)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (22,12)=0x04, (18,13)=0x68, (22,13)=0x04, (22,14)=0x04, (22,15)=0x04, (12,17)=0x03, (12,18)=0x03, (12,19)=0x03, … (+2) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- force floors at top: (14,8)=0x0D, (15,8)=0x14, (16,8)=0x14, (17,8)=0x14, (18,8)=0x14, (19,8)=0x14, (20,8)=0x14, (21,8)=0x14, … (+45) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (13,7)=0x01, (14,7)=0x01, (15,7)=0x01, (16,7)=0x01, (17,7)=0x01, (18,7)=0x01, (19,7)=0x01, (20,7)=0x01, … (+119) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (16,15)=0x29, (18,15)=0x29, (16,17)=0x29, (18,17)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (14,11)=0x21, (18,19)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- hints at top: (15,13)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.

### Level 008 — LESSON 8

- Map-backed baseline: **32×32**, time limit **100**, chips required **1**, password `NHAG`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (11,15)=0x02, (20,15)=0x22, (21,15)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (3,8)=0x2D, (4,8)=0x2D, (5,8)=0x2D, (6,8)=0x2D, (7,8)=0x2D, (8,8)=0x2D, (9,8)=0x2D, (10,8)=0x2D, … (+165) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,6)=0x01, (2,6)=0x01, (3,6)=0x01, (4,6)=0x01, (5,6)=0x01, (6,6)=0x01, (7,6)=0x01, (8,6)=0x01, … (+133) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- hints at top: (3,14)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- teeth at top: (10,15)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 009 — NUTS AND BOLTS

- Map-backed baseline: **32×32**, time limit **400**, chips required **9**, password `KCRE`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (28,2)=0x15, (6,4)=0x02, (17,5)=0x22, (13,6)=0x02, (11,10)=0x02, (13,18)=0x02, (9,19)=0x02, (4,21)=0x02, … (+2); bottom: (14,17)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (22,9)=0x64, (18,14)=0x67, (19,14)=0x16, (25,14)=0x18, (26,14)=0x65, (18,16)=0x66, (19,16)=0x19, (25,16)=0x17, … (+4) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (17,21)=0x03, (17,22)=0x03, (17,23)=0x03, (21,23)=0x04, (23,23)=0x04, (24,23)=0x04, (25,23)=0x04, (26,23)=0x04, … (+32) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (29,7)=0x0A, (8,14)=0x0A, (10,14)=0x0A, (7,15)=0x0A, (9,15)=0x0A, (14,17)=0x0A, (16,26)=0x0A, (15,27)=0x0A, … (+2) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (4,2)=0x2D, (5,2)=0x2D, (6,2)=0x2D, (7,2)=0x2D, (8,2)=0x2D, (9,2)=0x2D, (10,2)=0x2D, (11,2)=0x2D, … (+42) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (16,7)=0x0C, (17,7)=0x1B, (20,7)=0x1A, (21,7)=0x0C, (22,7)=0x0C, (23,7)=0x0C, (24,7)=0x1B, (17,8)=0x0C, … (+84) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (22,21)=0x12, (9,23)=0x0D, (9,24)=0x0D, (10,24)=0x0D, (9,25)=0x0D, (10,25)=0x0D, (11,25)=0x0D, (9,26)=0x0D, … (+12) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+510) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (20,14)=0x26, (24,14)=0x26, (20,16)=0x25, (24,16)=0x25, (22,20)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (28,4)=0x2B, (29,6)=0x27 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- bombs and thieves at top: (7,17)=0x2A, (8,17)=0x2A, (9,17)=0x2A, (10,17)=0x2A, (12,17)=0x2A, (13,17)=0x2A, (12,19)=0x2A, (13,19)=0x2A — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (7,3)=0x40, (10,3)=0x40, (13,3)=0x40, (5,4)=0x40, (7,5)=0x40, (10,5)=0x40, (13,5)=0x40, (14,6)=0x40, … (+3) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (30,5)=0x44, (22,12)=0x45, (16,15)=0x46, (28,15)=0x44, (22,18)=0x47, (3,27)=0x47, (5,31)=0x45 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.

### Level 010 — BRUSHFIRE

- Map-backed baseline: **32×32**, time limit **80**, chips required **0**, password `UVWS`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (21,19)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (2,1)=0x04, (24,1)=0x04, (23,2)=0x04, (31,2)=0x04, (17,3)=0x04, (21,3)=0x04, (27,3)=0x04, (29,3)=0x04, … (+112) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- walls and panels at top: (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, (9,1)=0x01, (10,1)=0x01, (11,1)=0x01, … (+297) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (2,28)=0x43 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.

### Level 011 — TRINITY

- Map-backed baseline: **32×32**, time limit **300**, chips required **3**, password `CNPE`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x02, (30,2)=0x22, (2,4)=0x02, (30,4)=0x02, (30,25)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (7,10)=0x65, (27,12)=0x64, (13,17)=0x16, (19,17)=0x17, (16,20)=0x19, (17,30)=0x67 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (8,4)=0x03, (9,4)=0x03, (10,4)=0x03, (11,4)=0x03, (12,4)=0x03, (13,4)=0x03, (14,4)=0x03, (18,4)=0x04, … (+33) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- dirt and gravel at top: (15,31)=0x2D, (19,31)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (8,2)=0x0C, (9,2)=0x0C, (10,2)=0x0C, (11,2)=0x0C, (12,2)=0x0C, (13,2)=0x0C, (14,2)=0x0C, (15,2)=0x0C, … (+23) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (8,8)=0x0D, (30,10)=0x12, (29,11)=0x13, (6,12)=0x0D, (23,13)=0x14, (25,13)=0x13, (8,14)=0x12, (21,15)=0x14, … (+27) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+536) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (16,30)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (6,10)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- gliders at top: (28,12)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 012 — HUNT

- Map-backed baseline: **32×32**, time limit **400**, chips required **652**, password `WVHI`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x02, (3,2)=0x02, (4,2)=0x02, (5,2)=0x02, (6,2)=0x02, (7,2)=0x02, (8,2)=0x02, (9,2)=0x02, … (+788) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+160) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teeth at top: (16,24)=0x54, (17,24)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 013 — SOUTHPOLE

- Map-backed baseline: **32×32**, time limit **0**, chips required **0**, password `OCKS`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (18,14)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- ice and ice corners at top: (2,2)=0x0C, (3,2)=0x0C, (4,2)=0x0C, (5,2)=0x0C, (6,2)=0x0C, (7,2)=0x0C, (8,2)=0x0C, (9,2)=0x0C, … (+864) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (26,16)=0x0D, (22,17)=0x14, (6,19)=0x13, (17,25)=0x12, (7,27)=0x13, (8,27)=0x13, (13,27)=0x13 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+118) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- hints at top: (20,25)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.

### Level 014 — TELEBLOCK

- Map-backed baseline: **32×32**, time limit **250**, chips required **4**, password `BTDY`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (11,6)=0x02, (5,8)=0x02, (13,12)=0x02, (7,14)=0x02, (10,22)=0x22, (10,23)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (18,10)=0x65, (28,10)=0x65, (23,13)=0x17, (21,15)=0x17, (25,15)=0x17, (23,17)=0x17, (9,20)=0x65, (18,20)=0x65 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (11,7)=0x03, (6,8)=0x03, (7,8)=0x03, (11,8)=0x03, (7,12)=0x03, (11,12)=0x03, (12,12)=0x03, (7,13)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (19,11)=0x0A, (23,11)=0x0A, (27,11)=0x0A, (19,15)=0x0A, (27,15)=0x0A, (19,19)=0x0A, (23,19)=0x0A, (27,19)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+260) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (9,10)=0x29, (23,15)=0x29, (10,20)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.

### Level 015 — ELEMENTARY

- Map-backed baseline: **32×32**, time limit **250**, chips required **12**, password `COZQ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (14,2)=0x02, (15,5)=0x02, (19,7)=0x02, (17,8)=0x02, (17,11)=0x22, (2,17)=0x02, (17,18)=0x15, (31,18)=0x02, … (+6) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (31,2)=0x65, (19,5)=0x65, (13,12)=0x16, (15,12)=0x16, (17,12)=0x16, (19,12)=0x16, (21,12)=0x16, (11,13)=0x65, … (+12) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (17,2)=0x04, (18,2)=0x04, (19,2)=0x04, (20,2)=0x04, (25,2)=0x04, (26,2)=0x04, (27,2)=0x04, (29,2)=0x04, … (+165); bottom: (7,12)=0x6B, (27,12)=0x69, (7,16)=0x68, (27,16)=0x6A — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (7,12)=0x0A, (27,12)=0x0A, (19,14)=0x0A, (7,16)=0x0A, (27,16)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (19,17)=0x0C, (19,18)=0x0C, (19,19)=0x1D, (20,19)=0x0C, (21,19)=0x0C, (22,19)=0x0C, (23,19)=0x0C, (24,19)=0x0C, … (+89) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (4,2)=0x0D, (5,2)=0x14, (6,2)=0x14, (7,2)=0x14, (10,2)=0x0D, (11,2)=0x14, (12,2)=0x14, (2,3)=0x0D, … (+85) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+526) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (17,10)=0x27, (17,17)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- bombs and thieves at top: (2,2)=0x21, (9,13)=0x2A, (25,13)=0x2A, (9,15)=0x2A, (25,15)=0x2A, (26,29)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.

### Level 016 — CELLBLOCKED

- Map-backed baseline: **32×32**, time limit **0**, chips required **0**, password `SKKK`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (31,31)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+1013) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- hints at top: (15,11)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.

### Level 017 — NICE DAY

- Map-backed baseline: **32×32**, time limit **100**, chips required **46**, password `AJMG`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x15, (3,2)=0x22, (30,2)=0x22, (31,2)=0x15, (2,3)=0x22, (3,3)=0x22, (30,3)=0x22, (31,3)=0x22, … (+54) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+116) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (10,2)=0x25, (11,2)=0x25, (12,2)=0x25, (13,2)=0x25, (14,2)=0x25, (15,2)=0x25, (16,2)=0x25, (17,2)=0x25, … (+84) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- balls at top: (15,12)=0x4B, (23,13)=0x4B, (9,14)=0x4B, (20,15)=0x4B, (18,16)=0x4B, (12,17)=0x4B, (16,18)=0x4B, (20,19)=0x4B, … (+1) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- walkers at top: (4,2)=0x58, (29,2)=0x58, (2,4)=0x58, (31,4)=0x58, (2,29)=0x58, (31,29)=0x58, (4,31)=0x58, (29,31)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 018 — CASTLE MOAT

- Map-backed baseline: **32×32**, time limit **600**, chips required **0**, password `HMJL`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (24,25)=0x15, (23,26)=0x22, (24,26)=0x22, (25,26)=0x22, (23,27)=0x22, (24,27)=0x22, (25,27)=0x22, (23,28)=0x22, … (+5) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (23,12)=0x03, (24,12)=0x03, (25,12)=0x03, (26,12)=0x03, (27,12)=0x03, (28,12)=0x03, (20,13)=0x03, (21,13)=0x03, … (+130); bottom: (29,2)=0x68 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (29,2)=0x0A, (23,3)=0x0A, (5,4)=0x0A, (15,5)=0x0A, (27,6)=0x0A, (24,7)=0x0A, (4,8)=0x0A, (9,9)=0x0A, … (+7) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+400) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bombs and thieves at top: (20,10)=0x2A, (15,12)=0x2A, (15,13)=0x2A, (11,17)=0x2A, (11,18)=0x2A — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.

### Level 019 — DIGGER

- Map-backed baseline: **32×32**, time limit **210**, chips required **146**, password `MRHR`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (4,4)=0x02, (4,5)=0x02, (6,5)=0x02, (7,5)=0x02, (8,5)=0x02, (9,5)=0x02, (10,5)=0x02, (11,5)=0x02, … (+147) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (2,2)=0x0B, (3,2)=0x0B, (4,2)=0x0B, (5,2)=0x0B, (6,2)=0x0B, (7,2)=0x0B, (8,2)=0x0B, (9,2)=0x0B, … (+624) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+116) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teeth at top: (6,9)=0x54, (17,12)=0x54, (25,17)=0x54, (17,18)=0x54, (30,19)=0x54, (2,20)=0x54, (5,23)=0x54, (14,25)=0x54, … (+2) — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 020 — TOSSED SALAD

- Map-backed baseline: **32×32**, time limit **400**, chips required **15**, password `KGFP`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (17,4)=0x02, (18,4)=0x02, (17,5)=0x02, (18,5)=0x02, (22,18)=0x02, (31,18)=0x02, (17,19)=0x15, (29,20)=0x02, … (+8) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (5,4)=0x17, (4,9)=0x65, (10,16)=0x19, (6,31)=0x67 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (16,3)=0x04, (17,3)=0x04, (18,3)=0x04, (19,3)=0x04, (16,4)=0x04, (19,4)=0x04, (16,5)=0x04, (19,5)=0x04, … (+6) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (27,30)=0x10 — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (26,31)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (23,6)=0x0C, (24,6)=0x0C, (25,6)=0x0C, (26,6)=0x0C, (27,6)=0x0C, (28,6)=0x0C, (29,6)=0x0C, (30,6)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (8,2)=0x13, (9,2)=0x13, (10,2)=0x13, (11,2)=0x13, (26,8)=0x14, (27,8)=0x14, (28,8)=0x14, (29,8)=0x14, … (+10) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+531) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (3,6)=0x25, (9,7)=0x23, (2,10)=0x26, (4,14)=0x25 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (9,11)=0x28, (30,24)=0x2B, (30,25)=0x2B, (30,26)=0x2B, (30,27)=0x2B, (30,28)=0x2B, (30,29)=0x2B; bottom: (31,23)=0x27, (31,24)=0x27, (31,25)=0x27, (31,26)=0x27, (31,27)=0x27, (31,28)=0x27 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at bottom: (31,29)=0x24, (27,30)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- teleporters at top: (6,5)=0x29, (31,31)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bugs at top: (19,31)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (9,10)=0x4A, (15,14)=0x4B, (9,18)=0x49, (8,19)=0x49, (7,20)=0x49, (6,21)=0x49, (5,22)=0x49, (4,23)=0x49, … (+7) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (7,7)=0x4D, (2,8)=0x4F, (6,9)=0x4F, (7,11)=0x4D, (4,12)=0x4F, (6,13)=0x4F, (9,25)=0x4D, (4,26)=0x4F, … (+3) — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.

### Level 021 — ICEBERG

- Map-backed baseline: **32×32**, time limit **150**, chips required **9**, password `UGRW`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (12,6)=0x02, (11,7)=0x02, (13,7)=0x02, (2,8)=0x02, (4,8)=0x02, (17,8)=0x02, (19,8)=0x02, (3,9)=0x02, … (+4) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (2,2)=0x03, (3,2)=0x03, (4,2)=0x03, (5,2)=0x03, (6,2)=0x03, (7,2)=0x03, (8,2)=0x03, (9,2)=0x03, … (+604) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (3,26)=0x0E — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (8,10)=0x1D, (9,10)=0x0C, (10,10)=0x0C, (11,10)=0x0C, (12,10)=0x0C, (13,10)=0x0C, (14,10)=0x0C, (15,10)=0x0C, … (+152) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+124) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (2,26)=0x24; bottom: (3,26)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.

### Level 022 — FORCED ENTRY

- Map-backed baseline: **32×32**, time limit **300**, chips required **8**, password `WZIN`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (22,3)=0x02, (22,10)=0x02, (14,11)=0x02, (11,13)=0x02, (15,22)=0x02, (18,24)=0x02, (31,24)=0x02, (29,27)=0x02, … (+2) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- ice and ice corners at top: (5,2)=0x0C, (6,2)=0x0C, (15,2)=0x0C, (16,2)=0x0C, (17,2)=0x0C, (18,2)=0x0C, (19,2)=0x0C, (4,4)=0x0C, … (+98) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (3,2)=0x13, (4,2)=0x13, (7,2)=0x13, (8,2)=0x0D, (9,2)=0x13, (10,2)=0x0D, (11,2)=0x13, (12,2)=0x13, … (+763) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+125) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (3,3)=0x29, (29,30)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.

### Level 023 — BLOBNET

- Map-backed baseline: **32×32**, time limit **500**, chips required **88**, password `HUVE`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (10,2)=0x02, (23,2)=0x02, (5,3)=0x02, (8,3)=0x02, (15,3)=0x02, (18,3)=0x02, (25,3)=0x02, (28,3)=0x02, … (+84) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (2,2)=0x2D, (3,2)=0x2D, (4,2)=0x2D, (5,2)=0x2D, (6,2)=0x2D, (7,2)=0x2D, (8,2)=0x2D, (9,2)=0x2D, … (+391); bottom: (17,2)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+126) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- blobs at top: (3,3)=0x5C, (6,3)=0x5C, (9,3)=0x5C, (13,3)=0x5C, (16,3)=0x5C, (19,3)=0x5C, (23,3)=0x5C, (26,3)=0x5C, … (+72) — `NEEDS CAPTURE`: Compare random movement/seed, speed schedule, blocked moves and collision.

### Level 024 — OORTO GELD

- Map-backed baseline: **32×32**, time limit **550**, chips required **48**, password `UNIZ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (4,2)=0x02, (6,2)=0x02, (8,2)=0x02, (10,2)=0x02, (12,2)=0x02, (14,2)=0x02, (18,2)=0x02, (20,2)=0x02, … (+46) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- blocks and block terrain at top: (14,13)=0x0A, (15,13)=0x0A, (16,13)=0x0A, (17,13)=0x0A, (18,13)=0x0A, (19,13)=0x0A, (13,14)=0x0A, (20,14)=0x0A, … (+16) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+434) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (4,3)=0x25, (6,3)=0x25, (8,3)=0x25, (10,3)=0x26, (12,3)=0x26, (14,3)=0x26, (18,3)=0x26, (20,3)=0x26, … (+60) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- gliders at top: (16,6)=0x50, (23,6)=0x50, (6,12)=0x50, (29,13)=0x50, (23,18)=0x50, (23,20)=0x50, (29,20)=0x50, (6,21)=0x50, … (+2) — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 025 — BLINK

- Map-backed baseline: **32×32**, time limit **600**, chips required **28**, password `PQGV`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (27,1)=0x15, (25,2)=0x02, (27,2)=0x22, (29,2)=0x02, (10,6)=0x02, (14,6)=0x02, (25,6)=0x02, (29,6)=0x02, … (+22) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+543) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (27,4)=0x29, (12,8)=0x29, (24,14)=0x29, (12,17)=0x29, (28,18)=0x29, (6,21)=0x29, (18,23)=0x29, (26,27)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.

### Level 026 — CHCHCHIPS

- Map-backed baseline: **32×32**, time limit **300**, chips required **170**, password `YVYJ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (1,2)=0x15, (2,2)=0x22, (3,2)=0x02, (4,2)=0x02, (5,2)=0x02, (6,2)=0x02, (7,2)=0x02, (8,2)=0x02, … (+164) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+508) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (29,16)=0x26, (25,19)=0x23, (23,21)=0x26 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- clone machines at top: (6,20)=0x24; bottom: (19,21)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bugs at top: (26,18)=0x43 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (14,2)=0x4B, (19,2)=0x4A, (20,2)=0x4A, (21,3)=0x4A, (22,3)=0x4A, (23,4)=0x4A, (24,4)=0x4A, (25,5)=0x4A, … (+7) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- teeth at top: (19,21)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- paramecia at top: (6,11)=0x63 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 027 — GO WITH THE FLOW

- Map-backed baseline: **32×32**, time limit **200**, chips required **6**, password `IGGZ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (26,9)=0x22, (27,9)=0x15, (20,19)=0x02, (13,23)=0x02, (21,24)=0x02, (13,25)=0x02, (18,25)=0x02, (13,27)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (26,11)=0x69, (16,14)=0x03, (17,14)=0x03, (18,14)=0x03, (19,14)=0x03, (21,14)=0x68, (18,17)=0x03, (18,18)=0x03, … (+6) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (16,7)=0x0A, (18,8)=0x0A, (14,10)=0x0A, (14,12)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- force floors at top: (11,11)=0x13, (12,11)=0x13, (13,11)=0x13, (11,12)=0x12, (23,12)=0x0D, (11,13)=0x12, (23,13)=0x0D, (11,14)=0x12, … (+40) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+863) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (25,17)=0x28, (21,20)=0x28 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- balls at top: (25,20)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (25,9)=0x4C, (25,12)=0x4C — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.

### Level 028 — PING PONG

- Map-backed baseline: **32×32**, time limit **300**, chips required **22**, password `UJDO`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (4,4)=0x02, (15,4)=0x22, (16,4)=0x15, (17,4)=0x15, (18,4)=0x22, (29,4)=0x02, (4,9)=0x02, (16,9)=0x02, … (+18) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (4,11)=0x03, (5,11)=0x03, (6,11)=0x03, (7,11)=0x03, (8,11)=0x03, (9,11)=0x03, (10,11)=0x03, (11,11)=0x03, … (+88) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (8,9)=0x0A, (9,9)=0x0A, (10,9)=0x0A, (11,9)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (31,2)=0x1B, (4,16)=0x0C, (5,16)=0x0C, (6,16)=0x0C, (7,16)=0x0C, (8,16)=0x0C, (9,16)=0x0C, (10,16)=0x0C, … (+91) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (3,2)=0x14, (4,2)=0x14, (5,2)=0x14, (6,2)=0x14, (7,2)=0x14, (8,2)=0x14, (9,2)=0x14, (10,2)=0x14, … (+104) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+264) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- balls at top: (5,4)=0x4B, (14,4)=0x4B, (19,4)=0x4B, (28,4)=0x4B, (5,9)=0x4B, (15,9)=0x4B, (18,9)=0x4B, (28,9)=0x4B, … (+16) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 029 — ARCTICFLOW

- Map-backed baseline: **32×32**, time limit **400**, chips required **5**, password `QGOL`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,3)=0x15, (28,4)=0x22, (14,13)=0x02, (22,17)=0x02, (19,18)=0x02, (21,18)=0x02, (17,19)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (7,2)=0x03, (8,2)=0x03, (9,2)=0x03, (10,2)=0x03, (11,2)=0x03, (12,2)=0x03, (13,2)=0x03, (14,2)=0x03, … (+385) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (3,6)=0x0A, (5,6)=0x0A, (13,6)=0x0A, (15,6)=0x0A, (23,6)=0x0A, (4,7)=0x0A, (14,7)=0x0A, (23,7)=0x0A, … (+13) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (2,12)=0x1A, (3,12)=0x0C, (4,12)=0x0C, (5,12)=0x1B, (7,12)=0x1A, (2,13)=0x0C, (7,13)=0x0C, (2,14)=0x1D, … (+183) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (13,23)=0x12 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+265) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- paramecia at top: (19,14)=0x60, (31,14)=0x62 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 030 — MISHMESH

- Map-backed baseline: **32×32**, time limit **600**, chips required **36**, password `BQZP`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x02, (8,2)=0x02, (14,2)=0x02, (19,2)=0x02, (25,2)=0x02, (31,2)=0x02, (2,8)=0x02, (8,8)=0x02, … (+30) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x1F, (3,1)=0x01, (4,1)=0x1F, (5,1)=0x01, (6,1)=0x1F, (7,1)=0x01, (8,1)=0x1F, … (+761) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 031 — KNOT

- Map-backed baseline: **32×32**, time limit **29**, chips required **118**, password `RYMS`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (5,3)=0x02, (6,3)=0x02, (7,3)=0x02, (8,3)=0x02, (9,3)=0x02, (10,3)=0x02, (11,3)=0x02, (21,3)=0x02, … (+112) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (4,2)=0x04, (12,2)=0x04, (20,2)=0x04, (28,2)=0x04, (3,3)=0x04, (29,3)=0x04, (11,8)=0x04, (13,8)=0x04, … (+22) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- force floors at top: (4,3)=0x13, (12,3)=0x0D, (20,3)=0x13, (28,3)=0x0D, (28,9)=0x14, (4,15)=0x13, (28,15)=0x12, (12,21)=0x12, … (+1) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+95) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 032 — SCAVENGER HUNT

- Map-backed baseline: **32×32**, time limit **600**, chips required **3**, password `PEFS`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (12,10)=0x02, (5,20)=0x02, (21,22)=0x15, (21,23)=0x22, (28,29)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (25,6)=0x68, (12,9)=0x04, (28,16)=0x6B, (12,29)=0x69, (28,30)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- force floors at top: (5,21)=0x0D — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+875) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 033 — ON THE ROCKS

- Map-backed baseline: **32×32**, time limit **0**, chips required **9**, password `BQSN`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (30,2)=0x22, (31,2)=0x15, (8,5)=0x02, (24,7)=0x02, (2,8)=0x02, (12,14)=0x02, (24,18)=0x02, (4,20)=0x02, … (+3) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (2,2)=0x03, (3,2)=0x03, (4,2)=0x03, (5,2)=0x03, (6,2)=0x03, (7,2)=0x03, (8,2)=0x03, (9,2)=0x03, … (+685) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (13,4)=0x0F, (4,8)=0x11, (27,12)=0x0F, (17,14)=0x11, (14,23)=0x0F, (24,27)=0x0F, (2,30)=0x11 — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (15,7)=0x0C, (16,7)=0x0C, (15,8)=0x0C, (16,8)=0x0C, (15,9)=0x0C, (16,9)=0x0C, (5,11)=0x0C, (6,11)=0x0C, … (+30) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+150) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (12,3)=0x24, (5,7)=0x24, (26,11)=0x24, (18,13)=0x24, (13,22)=0x24, (23,26)=0x24, (3,29)=0x24; bottom: (13,4)=0x31, (4,8)=0x31, (27,12)=0x31, (17,14)=0x31, (14,23)=0x31, (24,27)=0x31, (2,30)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.

### Level 034 — CYPHER

- Map-backed baseline: **32×32**, time limit **350**, chips required **14**, password `NQFI`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (13,3)=0x02, (24,6)=0x15, (25,6)=0x22, (7,8)=0x02, (15,8)=0x02, (2,13)=0x02, (9,13)=0x02, (31,13)=0x02, … (+8) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (6,3)=0x0B, (7,3)=0x0B, (8,3)=0x0B, (9,3)=0x0B, (4,10)=0x0B, (12,10)=0x0B, (21,10)=0x0B, (29,10)=0x0B, … (+33) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+688) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (30,6)=0x40, (14,14)=0x40, (22,14)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.

### Level 035 — LEMMINGS

- Map-backed baseline: **32×32**, time limit **600**, chips required **10**, password `VDTM`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (13,9)=0x02, (16,9)=0x02, (18,9)=0x02, (20,9)=0x02, (12,10)=0x02, (12,12)=0x02, (12,14)=0x02, (12,16)=0x02, … (+4) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (19,22)=0x03, (20,22)=0x03, (21,22)=0x03, (19,23)=0x03, (20,23)=0x03, (21,23)=0x03, (19,24)=0x03, (20,24)=0x03, … (+7) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (29,26)=0x0A, (30,27)=0x0A, (28,28)=0x0A, (24,29)=0x0A, (29,29)=0x0A, (23,30)=0x0A, (26,30)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+215) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- fireballs at top: (23,9)=0x44, (25,9)=0x47, (27,9)=0x46, (14,10)=0x46, (16,10)=0x47, (18,10)=0x47, (20,10)=0x47, (13,11)=0x44, … (+34) — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.

### Level 036 — LADDER

- Map-backed baseline: **32×32**, time limit **350**, chips required **28**, password `NXIS`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x02, (7,2)=0x02, (20,5)=0x02, (25,5)=0x02, (20,7)=0x02, (25,7)=0x02, (20,9)=0x02, (25,9)=0x02, … (+22) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (15,2)=0x16, (20,2)=0x65, (25,2)=0x64, (15,21)=0x16, (18,27)=0x64, (8,31)=0x17 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- blocks and block terrain at top: (3,4)=0x0A, (3,6)=0x0A, (3,8)=0x0A, (3,10)=0x0A, (3,12)=0x0A, (3,14)=0x0A, (3,16)=0x0A, (3,18)=0x0A, … (+5) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+544) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (4,2)=0x25, (4,4)=0x26, (4,6)=0x25, (4,8)=0x26, (4,10)=0x25, (4,12)=0x26, (5,13)=0x23, (6,13)=0x25, … (+10) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (7,3)=0x27, (2,4)=0x27, (5,4)=0x2B, (2,6)=0x27, (5,6)=0x2B, (8,7)=0x27, (2,8)=0x27, (5,8)=0x2B, … (+24); bottom: (14,3)=0x2B, (14,7)=0x2B, (14,11)=0x2B, (14,15)=0x2B, (14,19)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- tanks at top: (14,3)=0x4D, (14,7)=0x4D, (14,11)=0x4D, (14,15)=0x4D, (14,19)=0x4D — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.

### Level 037 — SEEING STARS

- Map-backed baseline: **32×32**, time limit **800**, chips required **56**, password `VQNK`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (31,1)=0x02, (32,1)=0x02, (7,2)=0x02, (8,2)=0x02, (15,2)=0x02, (16,2)=0x02, (20,2)=0x02, (29,2)=0x02, … (+100) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (21,2)=0x03, (28,2)=0x03, (3,3)=0x03, (7,3)=0x03, (8,3)=0x03, (12,3)=0x03, (21,3)=0x03, (28,3)=0x03, … (+52) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (5,6)=0x0A, (6,6)=0x0A, (9,6)=0x0A, (10,6)=0x0A, (24,6)=0x0A, (25,6)=0x0A, (14,7)=0x0A, (14,8)=0x0A, … (+24) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+456) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 038 — SAMPLER

- Map-backed baseline: **32×32**, time limit **500**, chips required **1**, password `BIFA`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (9,11)=0x22, (20,21)=0x15, (10,31)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (10,11)=0x65, (19,11)=0x16, (29,11)=0x17, (30,11)=0x64, (16,17)=0x66, (10,21)=0x67, (19,21)=0x19, (29,21)=0x18 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (20,11)=0x69, (30,21)=0x6B, (19,31)=0x03, (29,31)=0x04, (30,31)=0x68 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- dirt and gravel at top: (5,5)=0x2D, (25,5)=0x2D, (14,6)=0x2D, (16,6)=0x2D, (5,7)=0x2D, (25,7)=0x2D, (15,15)=0x2D, (25,15)=0x2D, … (+12) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- force floors at top: (9,31)=0x14 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+313) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (9,21)=0x25, (20,31)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (30,29)=0x28 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- teleporters at top: (5,6)=0x29, (15,6)=0x29, (25,6)=0x29, (5,16)=0x29, (15,16)=0x29, (25,16)=0x29, (5,26)=0x29, (15,26)=0x29, … (+1) — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bugs at top: (2,6)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (22,7)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (18,16)=0x48, (30,23)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (28,29)=0x4C — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- gliders at top: (10,16)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- teeth at top: (19,27)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- walkers at top: (28,16)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.
- blobs at top: (6,27)=0x5C — `NEEDS CAPTURE`: Compare random movement/seed, speed schedule, blocked moves and collision.
- paramecia at top: (20,6)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 039 — GLUT

- Map-backed baseline: **32×32**, time limit **20**, chips required **29**, password `ICXY`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,2)=0x02, (4,2)=0x02, (5,2)=0x02, (6,2)=0x02, (7,2)=0x02, (8,2)=0x02, (9,2)=0x02, (10,2)=0x02, … (+843); bottom: (7,7)=0x02, (8,7)=0x02, (9,7)=0x02, (10,7)=0x02, (23,7)=0x02, (24,7)=0x02, (25,7)=0x02, (26,7)=0x02, … (+40) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+116) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teeth at top: (7,7)=0x54, (8,7)=0x54, (9,7)=0x54, (10,7)=0x54, (23,7)=0x54, (24,7)=0x54, (25,7)=0x54, (26,7)=0x54, … (+40) — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 040 — FLOORGASBORG

- Map-backed baseline: **32×32**, time limit **200**, chips required **0**, password `YWFH`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (6,14)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (18,7)=0x04, (18,8)=0x04, (9,9)=0x04, (10,9)=0x6A, (11,9)=0x04, (17,9)=0x04, (18,9)=0x04, (19,9)=0x04, … (+149) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (3,11)=0x0C, (4,11)=0x0C, (5,11)=0x0C, (6,11)=0x0C, (7,11)=0x0C, (8,11)=0x0C, (9,11)=0x0C, (3,12)=0x0C, … (+52) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (14,5)=0x13, (15,5)=0x13, (16,5)=0x13, (17,5)=0x13, (18,5)=0x13, (19,5)=0x13, (20,5)=0x0D, (14,6)=0x12, … (+79) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (13,4)=0x1F, (14,4)=0x1F, (15,4)=0x1F, (16,4)=0x1F, (17,4)=0x1F, (18,4)=0x1F, (19,4)=0x1F, (20,4)=0x1F, … (+107) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bombs and thieves at top: (22,9)=0x21, (10,10)=0x21, (11,19)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.

### Level 041 — I.C. YOU

- Map-backed baseline: **32×32**, time limit **250**, chips required **60**, password `GKWD`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (22,2)=0x02, (23,2)=0x02, (25,2)=0x02, (26,2)=0x02, (22,3)=0x02, (23,3)=0x02, (25,3)=0x02, (26,3)=0x02, … (+54) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- blocks and block terrain at top: (7,7)=0x0A, (28,7)=0x0A, (5,9)=0x0A, (26,9)=0x0A, (7,24)=0x0A, (28,24)=0x0A, (5,26)=0x0A, (26,26)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (28,22)=0x0C, (26,29)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+536) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (6,8)=0x29, (27,8)=0x29, (13,16)=0x29, (20,16)=0x29, (6,25)=0x29, (27,25)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- hints at top: (13,18)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- bugs at top: (10,12)=0x43, (13,12)=0x43, (20,12)=0x43, (23,12)=0x42, (15,14)=0x43, (18,14)=0x43, (15,19)=0x41, (18,19)=0x41, … (+4) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- blobs at top: (12,2)=0x5C, (2,28)=0x5C — `NEEDS CAPTURE`: Compare random movement/seed, speed schedule, blocked moves and collision.

### Level 042 — BEWARE OF BUG

- Map-backed baseline: **32×32**, time limit **300**, chips required **27**, password `LMFU`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,3)=0x02, (5,5)=0x02, (30,5)=0x02, (7,7)=0x02, (28,7)=0x02, (9,9)=0x02, (26,9)=0x02, (11,11)=0x02, … (+24) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (32,2)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+508) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (8,18)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (4,23)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- gliders at top: (19,23)=0x51 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- paramecia at top: (19,16)=0x62, (19,31)=0x61 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 043 — LOCK BLOCK

- Map-backed baseline: **32×32**, time limit **200**, chips required **0**, password `UJDP`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (16,21)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (17,8)=0x17, (24,8)=0x17, (6,10)=0x65, (26,11)=0x65, (11,15)=0x64, (17,15)=0x17, (24,15)=0x16, (18,18)=0x65, … (+2) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (19,7)=0x03, (20,7)=0x03, (21,7)=0x03, (17,10)=0x03, (24,10)=0x03, (17,11)=0x03, (24,11)=0x03, (17,12)=0x03, … (+12); bottom: (11,17)=0x04, (12,17)=0x04 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (16,7)=0x0A, (17,7)=0x0A, (23,7)=0x0A, (24,7)=0x0A, (16,8)=0x0A, (23,8)=0x0A, (12,9)=0x0A, (16,14)=0x0A, … (+12) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (9,13)=0x1A, (10,13)=0x0C, (11,13)=0x0C, (12,13)=0x0C, (13,13)=0x0C, (9,14)=0x0C, (9,15)=0x0C, (9,16)=0x0C, … (+1) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (5,7)=0x13, (6,7)=0x13, (7,7)=0x13, (8,7)=0x13, (9,7)=0x13, (10,7)=0x13, (11,7)=0x13, (12,7)=0x13, … (+7) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+344) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- balls at top: (22,11)=0x49, (27,22)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 044 — REFRACTION

- Map-backed baseline: **32×32**, time limit **300**, chips required **32**, password `TXHL`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,3)=0x02, (6,3)=0x02, (10,3)=0x02, (14,3)=0x02, (18,3)=0x02, (22,3)=0x02, (27,3)=0x02, (31,3)=0x02, … (+26) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+373) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (2,10)=0x23, (8,10)=0x25, (12,10)=0x23, (21,10)=0x23, (25,10)=0x25, (31,10)=0x23, (2,12)=0x23, (8,12)=0x26, … (+4) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (24,3)=0x28, (24,7)=0x28, (9,15)=0x28, (9,19)=0x28 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- balls at top: (3,5)=0x4B, (19,5)=0x4B, (5,11)=0x4B, (7,11)=0x4B, (3,17)=0x4B, (20,17)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (2,2)=0x4D, (10,2)=0x4D, (18,2)=0x4D, (27,2)=0x4D, (6,8)=0x4F, (14,8)=0x4F, (22,8)=0x4F, (31,8)=0x4F, … (+8) — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.

### Level 045 — MONSTER LAB

- Map-backed baseline: **32×32**, time limit **300**, chips required **0**, password `OVPZ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (6,31)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (2,2)=0x03, (3,2)=0x03, (4,2)=0x03, (5,2)=0x03, (6,2)=0x03, (7,2)=0x03, (8,2)=0x03, (9,2)=0x03, … (+293) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (20,5)=0x0F, (25,12)=0x0F, (15,16)=0x0E, (28,23)=0x0E, (14,29)=0x0E — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- force floors at top: (29,9)=0x12, (29,10)=0x12, (29,11)=0x12, (29,12)=0x12, (29,13)=0x12, (29,14)=0x12, (29,15)=0x12 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+338) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (3,3)=0x23, (5,3)=0x23, (7,3)=0x23, (9,3)=0x23, (17,3)=0x23, (21,3)=0x23, (31,3)=0x23, (19,5)=0x23, … (+41); bottom: (31,13)=0x23, (29,29)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (15,3)=0x28 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (11,3)=0x24, (17,5)=0x24, (22,12)=0x24, (31,12)=0x24, (13,15)=0x24, (9,17)=0x24, (26,22)=0x24, (12,28)=0x24, … (+6); bottom: (13,3)=0x31, (20,5)=0x31, (25,12)=0x31, (31,14)=0x31, (15,16)=0x31, (9,19)=0x31, (28,23)=0x31, (14,29)=0x31, … (+3) — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (5,26)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (9,18)=0x40, (9,19)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (29,29)=0x45, (30,29)=0x45 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (31,13)=0x48, (31,14)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (12,3)=0x4D, (13,3)=0x4D — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- walkers at top: (6,30)=0x59, (9,30)=0x59 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 046 — THREE DOORS

- Map-backed baseline: **32×32**, time limit **250**, chips required **14**, password `HDQJ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x15, (19,2)=0x22, (20,2)=0x02, (3,12)=0x02, (4,12)=0x02, (5,12)=0x02, (6,12)=0x02, (7,12)=0x02, … (+8) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (25,2)=0x16, (26,2)=0x19, (27,2)=0x18, (28,2)=0x17, (29,2)=0x65, (11,8)=0x67, (8,14)=0x19, (2,15)=0x64, … (+7) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (8,15)=0x03, (2,17)=0x6B, (6,23)=0x04, (2,30)=0x68, (31,31)=0x69 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (8,2)=0x1A, (9,2)=0x0C, (11,2)=0x0C, (12,2)=0x0C, (13,2)=0x0C, (8,3)=0x0C, (29,3)=0x0C, (30,3)=0x0C, … (+220) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (10,2)=0x13, (15,2)=0x13, (9,15)=0x14, (10,15)=0x14, (11,15)=0x14, (12,15)=0x14, (13,15)=0x14, (14,15)=0x14, … (+36) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+495) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (28,4)=0x25, (28,5)=0x25, (28,6)=0x25, (11,7)=0x23, (28,7)=0x25, (28,8)=0x25, (28,9)=0x25, (28,10)=0x25, … (+5) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (16,8)=0x28, (2,21)=0x27; bottom: (5,27)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- teleporters at top: (18,2)=0x29, (9,10)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (14,2)=0x21, (28,29)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (20,6)=0x40, (22,6)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (20,8)=0x44, (22,8)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (26,6)=0x48, (26,13)=0x49, (5,27)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (21,8)=0x4C, (22,11)=0x4F — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- gliders at top: (20,9)=0x50, (22,9)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- teeth at top: (6,2)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- walkers at top: (24,8)=0x58, (26,8)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.
- blobs at top: (24,9)=0x5C, (26,9)=0x5C — `NEEDS CAPTURE`: Compare random movement/seed, speed schedule, blocked moves and collision.
- paramecia at top: (20,7)=0x60, (22,7)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 047 — PIER SEVEN

- Map-backed baseline: **32×32**, time limit **300**, chips required **4**, password `LXPP`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (17,16)=0x02, (18,16)=0x02, (17,17)=0x02, (18,17)=0x02, (30,31)=0x22, (31,31)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (16,6)=0x03, (17,6)=0x03, (18,6)=0x03, (19,6)=0x03, (13,7)=0x03, (14,7)=0x03, (15,7)=0x03, (16,7)=0x03, … (+318) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (9,4)=0x0A, (4,8)=0x0A, (29,9)=0x0A, (3,17)=0x0A, (4,26)=0x0A, (25,27)=0x0A, (11,30)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (15,3)=0x0C, (16,3)=0x0C, (17,3)=0x0C, (18,3)=0x0C, (19,3)=0x0C, (20,3)=0x0C, (4,4)=0x0C, (5,4)=0x0C, … (+101) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+119) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 048 — MUGGER SQUARE

- Map-backed baseline: **32×32**, time limit **300**, chips required **6**, password `JYSF`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (8,6)=0x02, (27,8)=0x02, (17,13)=0x02, (6,15)=0x02, (11,21)=0x22, (13,21)=0x15, (6,26)=0x02, (15,28)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (7,2)=0x04, (8,2)=0x04, (9,2)=0x04, (10,2)=0x04, (11,2)=0x04, (12,2)=0x04, (13,2)=0x04, (14,2)=0x04, … (+284); bottom: (18,2)=0x04, (15,3)=0x04, (12,4)=0x04, (9,5)=0x04, (10,19)=0x03, (9,21)=0x03, (8,23)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (13,24)=0x1A, (14,24)=0x0C, (15,24)=0x0C, (16,24)=0x0C, (17,24)=0x0C, (18,24)=0x0C, (19,24)=0x0C, (20,24)=0x0C, … (+127); bottom: (13,26)=0x0C, (12,28)=0x0C, (14,28)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (26,3)=0x0D, (27,3)=0x0D, (28,3)=0x0D, (29,3)=0x0D, (3,5)=0x13, (3,6)=0x13, (3,7)=0x13, (15,11)=0x13, … (+98) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+283) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (2,2)=0x29, (31,2)=0x29, (17,17)=0x29, (2,31)=0x29, (31,31)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (16,12)=0x21, (17,12)=0x21, (18,12)=0x21, (12,16)=0x21, (22,16)=0x21, (12,17)=0x21, (22,17)=0x21, (12,18)=0x21, … (+4) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- fireballs at top: (18,2)=0x47, (15,3)=0x47, (12,4)=0x47, (9,5)=0x47 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (13,26)=0x48, (12,28)=0x48, (14,28)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- gliders at top: (10,19)=0x50, (9,21)=0x50, (8,23)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- walkers at top: (30,12)=0x58, (31,15)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 049 — PROBLEMS

- Map-backed baseline: **32×32**, time limit **200**, chips required **23**, password `PPXI`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,14)=0x15, (2,15)=0x22, (14,20)=0x02, (23,20)=0x02, (27,20)=0x02, (9,21)=0x02, (17,22)=0x02, (21,23)=0x02, … (+17) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (11,2)=0x65, (30,2)=0x17, (31,2)=0x17, (6,3)=0x65, (9,3)=0x65, (14,3)=0x65, (26,3)=0x65, (31,3)=0x17, … (+28) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- dirt and gravel at top: (2,19)=0x2D, (3,19)=0x2D, (4,19)=0x2D, (5,19)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- force floors at top: (5,16)=0x14, (5,17)=0x14, (5,18)=0x14 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+284) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (3,2)=0x25, (3,3)=0x25, (3,4)=0x25, (3,5)=0x25, (3,6)=0x25, (3,7)=0x25, (3,8)=0x25, (3,9)=0x25, … (+30) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (29,16)=0x2B, (30,18)=0x27 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- balls at top: (2,2)=0x4B, (2,3)=0x4B, (2,4)=0x4B, (2,5)=0x4B, (2,6)=0x4B, (2,7)=0x4B, (2,8)=0x4B, (2,9)=0x4B, … (+15) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- paramecia at top: (4,13)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 050 — DIGDIRT

- Map-backed baseline: **32×32**, time limit **350**, chips required **16**, password `QBDH`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,3)=0x02, (12,3)=0x02, (21,3)=0x02, (30,3)=0x02, (3,12)=0x02, (12,12)=0x02, (21,12)=0x02, (30,12)=0x02, … (+10) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (5,2)=0x0B, (6,2)=0x0B, (7,2)=0x0B, (8,2)=0x0B, (9,2)=0x0B, (10,2)=0x0B, (14,2)=0x0B, (15,2)=0x0B, … (+733) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+128) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (11,3)=0x40, (29,3)=0x40, (2,12)=0x40, (20,12)=0x40, (11,21)=0x40, (29,21)=0x40, (2,30)=0x40, (20,30)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- paramecia at top: (2,3)=0x62, (20,3)=0x62, (11,12)=0x62, (29,12)=0x62, (2,21)=0x62, (20,21)=0x62, (11,30)=0x62, (29,30)=0x62 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 051 — I SLIDE

- Map-backed baseline: **32×32**, time limit **750**, chips required **13**, password `IGGJ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (14,2)=0x02, (2,3)=0x02, (29,4)=0x02, (22,8)=0x02, (8,9)=0x02, (17,11)=0x22, (30,13)=0x02, (4,14)=0x02, … (+7) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- ice and ice corners at top: (2,2)=0x1A, (3,2)=0x0C, (4,2)=0x0C, (5,2)=0x0C, (6,2)=0x1B, (7,2)=0x1A, (8,2)=0x0C, (9,2)=0x0C, … (+869) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+115) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 052 — THE LAST LAUGH

- Map-backed baseline: **32×32**, time limit **400**, chips required **2**, password `PPHT`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,13)=0x02, (18,13)=0x02, (2,15)=0x15, (2,16)=0x22 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (11,3)=0x03, (16,3)=0x03, (14,5)=0x03, (15,5)=0x03, (16,5)=0x03, (14,6)=0x03, (15,6)=0x03, (16,6)=0x03, … (+7) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (6,5)=0x0A, (11,5)=0x0A, (5,13)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- force floors at top: (5,4)=0x14, (7,4)=0x0D, (10,9)=0x0D, (11,9)=0x0D, (12,9)=0x0D, (10,16)=0x14, (11,16)=0x14, (12,16)=0x14, … (+5) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (6,2)=0x01, (11,2)=0x01, (16,2)=0x01, (3,3)=0x01, (4,3)=0x01, (5,3)=0x01, (7,3)=0x01, (8,3)=0x01, … (+160) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (2,10)=0x23, (10,13)=0x26, (12,13)=0x25, (20,13)=0x23, (11,18)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- clone machines at top: (6,3)=0x24, (11,10)=0x24; bottom: (8,4)=0x31, (11,12)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bugs at top: (6,21)=0x40, (10,21)=0x40, (14,21)=0x40, (18,21)=0x40, (4,23)=0x42, (8,23)=0x42, (12,23)=0x42 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (8,4)=0x47, (11,12)=0x46 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (6,7)=0x48, (12,10)=0x49 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- teeth at top: (10,18)=0x55 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 053 — TRAFFIC COP

- Map-backed baseline: **32×32**, time limit **500**, chips required **0**, password `CGNX`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,3)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (5,2)=0x03, (6,2)=0x03, (7,2)=0x03, (8,2)=0x03, (9,2)=0x03, (10,2)=0x03, (12,2)=0x03, (13,2)=0x03, … (+299) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (4,5)=0x0A, (5,5)=0x0A, (6,5)=0x0A, (7,5)=0x0A, (7,6)=0x0A, (4,7)=0x0A, (5,7)=0x0A, (6,7)=0x0A, … (+6) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (28,21)=0x0C, (30,21)=0x0C, (28,22)=0x0C, (30,22)=0x0C, (28,23)=0x0C, (29,23)=0x0C, (30,23)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (29,24)=0x12 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+118) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (2,2)=0x25, (3,2)=0x25, (4,2)=0x25, (2,3)=0x25, (4,3)=0x25, (2,4)=0x25, (3,4)=0x25, (29,21)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- clone machines at top: (17,2)=0x24; bottom: (19,2)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- hints at top: (11,2)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- balls at top: (16,2)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- walkers at top: (19,2)=0x5A — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 054 — GRAIL

- Map-backed baseline: **32×32**, time limit **350**, chips required **68**, password `ZMGC`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (18,15)=0x22, (17,16)=0x22, (18,16)=0x15, (19,16)=0x22, (18,17)=0x22, (5,21)=0x02, (6,21)=0x02, (7,21)=0x02, … (+65) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (11,5)=0x65, (23,7)=0x66, (18,11)=0x17, (12,12)=0x67, (11,16)=0x18, (25,16)=0x19, (18,21)=0x16 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (16,3)=0x03, (3,4)=0x03, (4,4)=0x03, (7,4)=0x03, (8,4)=0x03, (10,4)=0x03, (11,4)=0x03, (12,4)=0x03, … (+69) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (23,4)=0x0A, (7,6)=0x10, (23,6)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- force floors at top: (2,3)=0x32, (3,3)=0x14, (4,3)=0x14, (5,3)=0x14, (6,3)=0x14, (7,3)=0x14, (8,3)=0x14, (9,3)=0x32, … (+298) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+229) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (13,4)=0x24; bottom: (7,6)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- teleporters at top: (18,4)=0x29, (6,16)=0x29, (30,16)=0x29, (18,28)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (16,14)=0x2A, (20,14)=0x2A, (16,18)=0x2A, (20,18)=0x2A — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- walkers at top: (7,25)=0x5B, (27,25)=0x5B — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 055 — POTPOURRI

- Map-backed baseline: **32×32**, time limit **100**, chips required **30**, password `SJES`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (4,2)=0x02, (6,3)=0x02, (2,4)=0x02, (8,4)=0x02, (17,4)=0x22, (18,4)=0x15, (10,5)=0x02, (12,6)=0x02, … (+24); bottom: (3,5)=0x02, (3,16)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (22,6)=0x64, (19,8)=0x64, (8,9)=0x16 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (22,7)=0x03, (24,7)=0x03, (17,8)=0x69, (26,9)=0x69, (9,13)=0x04, (2,14)=0x04, (3,14)=0x04, (4,14)=0x04, … (+19); bottom: (18,10)=0x04 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (3,2)=0x0A, (3,3)=0x0A, (3,4)=0x0A, (3,5)=0x0A, (3,6)=0x0A, (18,10)=0x0A, (19,11)=0x0A, (19,15)=0x0A, … (+4) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (4,4)=0x2D, (14,7)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (9,8)=0x1A, (10,8)=0x0C, (11,8)=0x1B, (13,8)=0x1A, (14,8)=0x0C, (15,8)=0x1B, (9,9)=0x0C, (11,9)=0x0C, … (+28) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+260) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (19,3)=0x28 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- bombs and thieves at top: (23,11)=0x2A; bottom: (19,15)=0x21, (21,16)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- balls at top: (5,2)=0x48, (7,3)=0x48, (22,3)=0x49, (9,4)=0x48, (11,5)=0x48, (13,6)=0x48, (17,6)=0x49, (18,7)=0x49, … (+1) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (21,5)=0x4E, (23,5)=0x4E, (24,9)=0x4D — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.

### Level 056 — DEEPFREEZE

- Map-backed baseline: **32×32**, time limit **250**, chips required **8**, password `FCJE`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (14,3)=0x22, (22,3)=0x02, (30,3)=0x15, (24,5)=0x02, (28,5)=0x02, (14,6)=0x22, (31,7)=0x02, (14,9)=0x22, … (+6) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (2,4)=0x6A — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- dirt and gravel at top: (17,13)=0x2D, (18,13)=0x2D, (17,14)=0x2D, (18,14)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (2,2)=0x1A, (3,2)=0x1B, (4,2)=0x1A, (5,2)=0x1B, (6,2)=0x1A, (7,2)=0x1B, (8,2)=0x1A, (9,2)=0x1B, … (+633) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (7,14)=0x0D, (14,14)=0x12, (16,18)=0x14, (16,23)=0x14, (16,28)=0x13 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+222) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (28,7)=0x25, (8,20)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- teleporters at top: (24,7)=0x29, (23,24)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- balls at top: (18,2)=0x48, (2,13)=0x4B, (31,14)=0x4B, (17,31)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 057 — STRANGE MAZE

- Map-backed baseline: **32×32**, time limit **400**, chips required **35**, password `UBXU`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (7,2)=0x22, (8,2)=0x02, (9,2)=0x22, (16,2)=0x02, (18,2)=0x02, (28,2)=0x02, (3,3)=0x22, (4,3)=0x22, … (+80) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (14,2)=0x16, (17,2)=0x16, (19,2)=0x16, (14,3)=0x16, (16,3)=0x16, (17,3)=0x16, (19,3)=0x16, (14,4)=0x16, … (+142) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (18,13)=0x03, (19,13)=0x03, (20,13)=0x03, (21,13)=0x03, (25,13)=0x03, (29,13)=0x03, (15,14)=0x03, (21,14)=0x03, … (+91) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- walls and panels at top: (1,1)=0x1F, (2,1)=0x1F, (3,1)=0x1F, (4,1)=0x1F, (5,1)=0x1F, (6,1)=0x1F, (7,1)=0x1F, (8,1)=0x1F, … (+194) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at bottom: (23,7)=0x2B, (24,7)=0x2B, (25,7)=0x2B, (27,7)=0x2B, (27,8)=0x2B, (22,9)=0x2B, (23,9)=0x2B, (24,9)=0x2B, … (+33) — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- bombs and thieves at top: (27,2)=0x2A, (31,2)=0x2A, (21,3)=0x2A, (22,3)=0x2A, (23,3)=0x2A, (24,3)=0x2A, (25,3)=0x2A, (27,3)=0x2A, … (+27) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- teeth at top: (2,17)=0x54, (3,17)=0x54, (4,17)=0x54, (5,17)=0x54, (6,17)=0x54, (3,18)=0x54, (7,18)=0x54, (3,19)=0x54, … (+11) — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- walkers at top: (23,7)=0x58, (24,7)=0x58, (25,7)=0x58, (27,7)=0x58, (27,8)=0x58, (22,9)=0x58, (23,9)=0x58, (24,9)=0x58, … (+14) — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 058 — LOOP AROUND

- Map-backed baseline: **32×32**, time limit **600**, chips required **9**, password `YBLT`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (9,9)=0x02, (9,10)=0x02, (23,11)=0x02, (24,11)=0x02, (25,11)=0x02, (24,24)=0x02, (8,25)=0x02, (9,25)=0x02, … (+3) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (7,7)=0x03, (8,7)=0x03, (9,7)=0x03, (23,7)=0x03, (24,7)=0x03, (25,7)=0x03, (7,8)=0x03, (10,8)=0x03, … (+47) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (17,3)=0x1A, (18,3)=0x0C, (19,3)=0x0C, (20,3)=0x0C, (21,3)=0x0C, (27,3)=0x0C, (28,3)=0x0C, (29,3)=0x0C, … (+44) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (3,3)=0x13, (4,3)=0x13, (5,3)=0x13, (6,3)=0x13, (7,3)=0x13, (8,3)=0x13, (9,3)=0x13, (10,3)=0x13, … (+111) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+547) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (15,14)=0x29, (15,19)=0x29, (17,30)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (27,13)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- balls at top: (12,5)=0x49, (20,5)=0x48, (13,10)=0x48, (12,14)=0x49, (17,19)=0x48, (22,20)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 059 — HIDDEN DANGER

- Map-backed baseline: **32×32**, time limit **400**, chips required **8**, password `BLDM`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (30,4)=0x15, (9,8)=0x02, (30,8)=0x22, (9,10)=0x02, (9,12)=0x02, (9,14)=0x02, (9,16)=0x02, (9,18)=0x02, … (+2) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (28,2)=0x65, (30,2)=0x17 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (30,5)=0x03, (30,6)=0x04, (8,26)=0x68, (9,26)=0x69, (10,26)=0x6B — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- dirt and gravel at top: (2,21)=0x2D, (10,21)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (2,2)=0x1A, (3,2)=0x0C, (4,2)=0x0C, (5,2)=0x0C, (6,2)=0x0C, (7,2)=0x0C, (8,2)=0x0C, (9,2)=0x0C, … (+88) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (27,2)=0x13, (30,7)=0x14 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+446) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (2,8)=0x23, (3,8)=0x25, (3,10)=0x25, (3,12)=0x25, (3,14)=0x25, (3,16)=0x25, (3,18)=0x25, (3,20)=0x25, … (+1) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (12,29)=0x2B, (17,30)=0x27 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- teleporters at top: (30,3)=0x29, (25,16)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (29,7)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- fireballs at top: (6,8)=0x44, (8,8)=0x44, (6,10)=0x44, (8,10)=0x44, (6,12)=0x44, (8,12)=0x44, (6,14)=0x44, (8,14)=0x44, … (+7) — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- gliders at top: (6,22)=0x51, (8,22)=0x51, (25,28)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- teeth at top: (12,22)=0x54, (14,22)=0x54, (16,22)=0x54, (12,24)=0x54, (14,24)=0x54, (16,24)=0x54, (18,24)=0x54, (12,26)=0x54, … (+7) — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- walkers at top: (25,12)=0x58, (25,14)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 060 — SCOUNDREL

- Map-backed baseline: **32×32**, time limit **294**, chips required **12**, password `ZYVI`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (23,4)=0x02, (5,7)=0x02, (15,11)=0x02, (20,11)=0x02, (9,13)=0x02, (11,15)=0x02, (25,15)=0x02, (13,17)=0x02, … (+15) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (12,5)=0x67, (20,17)=0x17, (30,17)=0x64, (17,18)=0x19, (20,21)=0x65, (19,25)=0x19, (30,28)=0x16 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (26,12)=0x6A, (12,13)=0x68, (21,17)=0x69, (6,27)=0x04, (4,28)=0x04, (5,28)=0x04, (6,28)=0x04, (7,28)=0x04, … (+1) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- force floors at top: (2,3)=0x12, (31,3)=0x12, (2,4)=0x12, (31,4)=0x12, (2,5)=0x12, (31,5)=0x12, (2,6)=0x12, (31,6)=0x12, … (+41) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+534) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (16,13)=0x25, (18,13)=0x26, (17,16)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- bugs at top: (5,25)=0x41, (9,25)=0x41, (2,26)=0x42, (9,28)=0x42, (2,29)=0x42, (4,31)=0x43, (8,31)=0x43 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- blobs at top: (24,29)=0x5D — `NEEDS CAPTURE`: Compare random movement/seed, speed schedule, blocked moves and collision.

### Level 061 — RINK

- Map-backed baseline: **32×32**, time limit **0**, chips required **16**, password `RMOW`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (27,4)=0x02, (18,5)=0x02, (9,6)=0x02, (29,6)=0x02, (6,9)=0x02, (16,11)=0x02, (28,15)=0x02, (11,16)=0x02, … (+10) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- ice and ice corners at top: (2,1)=0x1A, (3,1)=0x1B, (4,1)=0x1A, (5,1)=0x1B, (6,1)=0x1A, (7,1)=0x1B, (8,1)=0x1A, (9,1)=0x1B, … (+779) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (30,1)=0x01, (31,1)=0x01, (32,1)=0x01, (5,2)=0x1E, (9,2)=0x1F, (13,2)=0x1F, (17,2)=0x1E, … (+210) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 062 — SLO MO

- Map-backed baseline: **32×32**, time limit **300**, chips required **0**, password `TIGW`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (31,12)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+121) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (2,2)=0x25, (6,2)=0x25, (10,2)=0x25, (11,2)=0x26, (12,2)=0x25, (13,2)=0x26, (14,2)=0x25, (15,2)=0x26, … (+739); bottom: (4,5)=0x25, (6,5)=0x25, (19,7)=0x25, (24,8)=0x25, (10,12)=0x25, (27,12)=0x26, (7,14)=0x26, (8,16)=0x25, … (+26) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- balls at top: (29,2)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- teeth at top: (30,27)=0x54, (27,28)=0x54, (29,28)=0x54, (31,28)=0x54, (28,29)=0x54, (30,29)=0x54, (29,30)=0x54, (31,30)=0x54, … (+2) — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- blobs at top: (4,5)=0x5C, (6,5)=0x5C, (19,7)=0x5C, (24,8)=0x5C, (10,12)=0x5C, (27,12)=0x5C, (7,14)=0x5C, (8,16)=0x5C, … (+16) — `NEEDS CAPTURE`: Compare random movement/seed, speed schedule, blocked moves and collision.

### Level 063 — BLOCK FACTORY

- Map-backed baseline: **32×32**, time limit **500**, chips required **7**, password `GOHX`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (18,2)=0x02, (18,4)=0x02, (18,6)=0x02, (18,8)=0x02, (18,10)=0x02, (18,12)=0x02, (4,27)=0x02, (3,31)=0x22, … (+1) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (15,2)=0x03, (16,2)=0x03, (17,2)=0x03, (15,4)=0x03, (16,4)=0x03, (17,4)=0x03, (15,6)=0x03, (16,6)=0x03, … (+10) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (3,3)=0x11, (3,5)=0x11, (3,7)=0x11, (3,9)=0x11, (3,11)=0x11, (3,13)=0x11, (1,26)=0x11 — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (4,3)=0x0C, (5,3)=0x0C, (6,3)=0x1B, (4,5)=0x0C, (5,5)=0x0C, (6,5)=0x0C, (7,5)=0x0C, (8,5)=0x1B, … (+70) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (19,4)=0x0D, (20,4)=0x13, (21,4)=0x0D, (22,4)=0x13, (23,4)=0x0D, (24,4)=0x13, (25,4)=0x0D, (26,4)=0x13, … (+356) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+345) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (4,2)=0x24, (6,4)=0x24, (8,6)=0x24, (10,8)=0x24, (12,10)=0x24, (14,12)=0x24, (4,24)=0x24; bottom: (3,3)=0x31, (3,5)=0x31, (3,7)=0x31, (3,9)=0x31, (3,11)=0x31, (3,13)=0x31, (1,26)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.

### Level 064 — SPOOKS

- Map-backed baseline: **32×32**, time limit **600**, chips required **41**, password `IJPQ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x02, (10,2)=0x02, (22,2)=0x02, (28,2)=0x02, (7,5)=0x02, (25,5)=0x02, (2,6)=0x02, (15,6)=0x02, … (+35) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (3,2)=0x0B, (4,2)=0x0B, (5,2)=0x0B, (6,2)=0x0B, (9,2)=0x0B, (11,2)=0x0B, (12,2)=0x0B, (13,2)=0x0B, … (+626) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+115) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teeth at top: (7,2)=0x54, (25,2)=0x54, (8,3)=0x54, (25,3)=0x54, (3,4)=0x54, (9,4)=0x54, (16,4)=0x54, (18,4)=0x54, … (+88) — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 065 — AMSTERDAM

- Map-backed baseline: **32×32**, time limit **500**, chips required **51**, password `UPUN`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (9,4)=0x02, (13,4)=0x02, (18,4)=0x02, (4,5)=0x02, (21,5)=0x02, (25,5)=0x02, (28,5)=0x02, (18,8)=0x02, … (+45) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- blocks and block terrain at top: (19,4)=0x0A, (27,4)=0x0A, (14,5)=0x0A, (16,5)=0x0A, (5,6)=0x0A, (10,6)=0x0A, (26,6)=0x0A, (29,6)=0x0A, … (+58) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (2,2)=0x1A, (3,2)=0x0C, (4,2)=0x0C, (5,2)=0x0C, (6,2)=0x0C, (8,2)=0x0C, (9,2)=0x0C, (10,2)=0x0C, … (+225) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+484) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 066 — VICTIM

- Map-backed baseline: **32×32**, time limit **300**, chips required **12**, password `ZIKZ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (16,5)=0x22, (25,6)=0x15, (14,7)=0x22, (18,7)=0x22, (15,9)=0x02, (17,9)=0x02, (14,10)=0x02, (18,10)=0x02, … (+14) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (15,14)=0x04, (17,14)=0x04, (16,19)=0x69 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (16,11)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+178) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (16,9)=0x25, (16,10)=0x23, (16,12)=0x23, (12,13)=0x25, (20,13)=0x25; bottom: (16,11)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- teleporters at top: (16,14)=0x29, (11,18)=0x29, (16,18)=0x29, (21,18)=0x29, (16,23)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- teeth at top: (12,15)=0x57, (20,15)=0x55, (12,16)=0x57, (20,16)=0x55, (12,17)=0x57, (20,17)=0x55, (12,18)=0x57, (20,18)=0x55, … (+13) — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 067 — CHIPMINE

- Map-backed baseline: **32×32**, time limit **700**, chips required **28**, password `GGJA`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (18,2)=0x02, (29,4)=0x02, (8,6)=0x02, (29,6)=0x02, (31,10)=0x02, (31,12)=0x02, (6,14)=0x02, (14,14)=0x02, … (+22) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x1F, (2,1)=0x1F, (3,1)=0x1F, (4,1)=0x1F, (5,1)=0x1F, (6,1)=0x1F, (7,1)=0x1F, (8,1)=0x1F, … (+985) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 068 — EENY MINY MOE

- Map-backed baseline: **32×32**, time limit **650**, chips required **52**, password `RTDI`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,8)=0x02, (4,8)=0x02, (6,8)=0x02, (8,8)=0x02, (10,8)=0x02, (12,8)=0x02, (14,8)=0x02, (16,8)=0x02, … (+46) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (31,9)=0x18, (6,10)=0x18, (23,18)=0x66 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (2,6)=0x03, (3,6)=0x03, (4,6)=0x03, (5,6)=0x03, (6,6)=0x03, (7,6)=0x03, (8,6)=0x03, (9,6)=0x03, … (+38) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (2,3)=0x0A, (3,3)=0x0A, (4,3)=0x0A, (5,3)=0x0A, (6,3)=0x0A, (7,3)=0x0A, (8,3)=0x0A, (9,3)=0x0A, … (+36) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (20,11)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- force floors at top: (19,11)=0x12 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+338) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (6,14)=0x2B, (9,14)=0x2B, (12,14)=0x2B, (4,15)=0x2B, (13,16)=0x2B, (4,18)=0x2B, (13,19)=0x2B, (4,21)=0x2B, … (+8) — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (31,29)=0x24; bottom: (23,20)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- gliders at top: (4,13)=0x51, (6,13)=0x51, (8,13)=0x51, (11,13)=0x51, (13,13)=0x51, (14,14)=0x50, (3,15)=0x52, (14,17)=0x50, … (+14) — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- paramecia at top: (23,20)=0x62 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 069 — BOUNCE CITY

- Map-backed baseline: **32×32**, time limit **300**, chips required **0**, password `NLLY`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (31,31)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (20,9)=0x67, (21,9)=0x64, (22,9)=0x16, (29,9)=0x65, (31,21)=0x64, (24,22)=0x19, (28,24)=0x19, (31,25)=0x67, … (+3) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+457) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- balls at top: (30,4)=0x49, (24,6)=0x48, (30,6)=0x49, (27,8)=0x48, (30,8)=0x49, (27,10)=0x48, (29,10)=0x49, (24,11)=0x48, … (+29) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- paramecia at top: (8,2)=0x63, (14,2)=0x63, (4,4)=0x63, (5,8)=0x63, (7,8)=0x63, (9,8)=0x63, (12,8)=0x63, (14,8)=0x63, … (+1) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 070 — NIGHTMARE

- Map-backed baseline: **32×32**, time limit **199**, chips required **0**, password `GCCG`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (24,8)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (23,2)=0x65, (24,15)=0x17, (14,24)=0x19, (10,29)=0x67 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (30,2)=0x03, (31,2)=0x03, (27,3)=0x03, (28,3)=0x03, (31,3)=0x03, (2,4)=0x04, (3,4)=0x04, (20,4)=0x03, … (+102) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (16,12)=0x0A, (30,16)=0x0A, (8,17)=0x0A, (28,17)=0x0A, (28,19)=0x0A, (27,21)=0x0A, (29,21)=0x0A, (4,25)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (23,4)=0x0B, (16,11)=0x2D, (19,17)=0x2D, (7,20)=0x2D, (24,26)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (30,14)=0x0C, (27,15)=0x0C, (28,15)=0x0C, (29,15)=0x0C, (30,15)=0x0C, (25,16)=0x0C, (26,16)=0x0C, (27,16)=0x0C, … (+21) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (29,8)=0x12, (23,17)=0x13, (22,19)=0x0D, (22,20)=0x0D, (22,21)=0x0D — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+492) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (4,10)=0x26, (3,11)=0x23, (6,11)=0x25 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (17,2)=0x27, (11,9)=0x27, (26,10)=0x27, (16,13)=0x27, (9,14)=0x2B, (2,19)=0x27, (19,21)=0x28, (24,22)=0x28, … (+4); bottom: (14,3)=0x2B, (16,9)=0x2B, (31,10)=0x2B, (5,19)=0x2B, (20,26)=0x2B, (7,28)=0x2B, (30,31)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (31,8)=0x24; bottom: (30,10)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (9,19)=0x2A, (10,19)=0x2A, (10,20)=0x2A, (11,20)=0x2A — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (22,2)=0x43, (14,3)=0x43, (12,11)=0x42, (13,11)=0x41, (8,12)=0x42, (9,12)=0x41, (10,12)=0x41, (11,12)=0x41, … (+2) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (16,9)=0x45, (30,10)=0x45, (4,16)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (31,10)=0x48, (26,30)=0x49 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (18,22)=0x4D — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- gliders at top: (6,10)=0x52 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- teeth at top: (2,2)=0x54, (14,7)=0x54, (5,22)=0x54, (30,31)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- walkers at top: (20,26)=0x59, (15,29)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.
- blobs at top: (16,23)=0x5C — `NEEDS CAPTURE`: Compare random movement/seed, speed schedule, blocked moves and collision.
- paramecia at top: (5,19)=0x61, (7,28)=0x63 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 071 — CORRIDOR

- Map-backed baseline: **32×32**, time limit **500**, chips required **23**, password `LAJM`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (14,3)=0x02, (20,3)=0x02, (8,4)=0x02, (26,4)=0x02, (10,6)=0x02, (12,6)=0x02, (22,6)=0x02, (24,6)=0x02, … (+17) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (8,3)=0x67, (26,3)=0x65, (14,4)=0x17, (20,4)=0x17, (16,7)=0x65, (18,9)=0x65, (9,10)=0x16, (25,10)=0x19, … (+14) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (6,2)=0x04, (8,2)=0x04, (10,2)=0x04, (12,2)=0x04, (14,2)=0x04, (16,2)=0x04, (18,2)=0x04, (20,2)=0x04, … (+39) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (13,4)=0x0A, (21,4)=0x0A, (17,5)=0x0A, (11,6)=0x0A, (23,6)=0x0A, (10,7)=0x0A, (12,7)=0x0A, (22,7)=0x0A, … (+7) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, (9,1)=0x01, (10,1)=0x01, (11,1)=0x01, (12,1)=0x01, … (+323) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- walkers at top: (9,19)=0x58, (25,19)=0x58, (14,23)=0x58, (20,23)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 072 — REVERSE ALLEY

- Map-backed baseline: **32×32**, time limit **0**, chips required **35**, password `EKFT`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (14,5)=0x02, (27,5)=0x02, (11,6)=0x02, (18,6)=0x02, (22,6)=0x02, (6,8)=0x02, (20,10)=0x02, (27,10)=0x02, … (+29) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+322) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (2,2)=0x28, (3,2)=0x28, (10,2)=0x28, (11,2)=0x28, (17,2)=0x28, (24,2)=0x28, (30,2)=0x28, (31,2)=0x28, … (+20) — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- tanks at top: (13,4)=0x4E, (17,4)=0x4E, (21,4)=0x4E, (25,4)=0x4E, (4,5)=0x4F, (7,6)=0x4D, (16,7)=0x4C, (20,7)=0x4C, … (+51) — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- paramecia at top: (2,4)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 073 — MORTON

- Map-backed baseline: **32×32**, time limit **600**, chips required **28**, password `QCCR`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,3)=0x02, (30,3)=0x02, (5,5)=0x02, (28,5)=0x02, (7,7)=0x02, (26,7)=0x02, (9,9)=0x02, (24,9)=0x02, … (+22) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+514) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teeth at top: (16,16)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 074 — PLAYTIME

- Map-backed baseline: **32×32**, time limit **400**, chips required **4**, password `MKNH`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (29,6)=0x22, (7,7)=0x02, (28,7)=0x22, (29,7)=0x15, (30,7)=0x22, (29,8)=0x22, (21,9)=0x02, (8,21)=0x02, … (+1) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (19,6)=0x03, (20,6)=0x03, (21,6)=0x03, (22,6)=0x03, (23,6)=0x03, (19,7)=0x03, (20,7)=0x03, (21,7)=0x03, … (+47); bottom: (25,9)=0x03, (22,10)=0x03, (21,12)=0x03, (26,13)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (14,10)=0x0A, (14,12)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (18,5)=0x2D, (19,5)=0x2D, (20,5)=0x2D, (21,5)=0x2D, (22,5)=0x2D, (23,5)=0x2D, (24,5)=0x2D, (18,6)=0x2D, … (+35) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (2,2)=0x1A, (3,2)=0x0C, (4,2)=0x0C, (5,2)=0x0C, (6,2)=0x0C, (7,2)=0x0C, (8,2)=0x0C, (9,2)=0x0C, … (+199) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (4,20)=0x12, (5,20)=0x12, (6,20)=0x12, (7,20)=0x12, (8,20)=0x12, (9,20)=0x13, (10,20)=0x13, (11,20)=0x13, … (+84); bottom: (7,26)=0x0D — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+350) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (27,19)=0x25, (17,22)=0x25, (20,22)=0x26, (17,23)=0x23, (20,24)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (27,21)=0x28, (28,31)=0x27, (29,31)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (27,3)=0x24; bottom: (29,23)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bugs at top: (28,19)=0x41, (19,22)=0x40, (20,25)=0x43 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (4,13)=0x48, (6,13)=0x48, (8,13)=0x48, (10,13)=0x48, (29,23)=0x4B, (9,29)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (15,4)=0x4E, (6,8)=0x4F, (10,8)=0x4F — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- gliders at top: (25,9)=0x50, (22,10)=0x51, (21,12)=0x53, (26,13)=0x53 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- paramecia at top: (14,20)=0x62, (14,25)=0x62, (7,26)=0x63 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 075 — STEAM

- Map-backed baseline: **32×32**, time limit **500**, chips required **0**, password `MJDV`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (3,2)=0x04, (4,2)=0x04, (5,2)=0x04, (6,2)=0x04, (7,2)=0x04, (8,2)=0x04, (9,2)=0x04, (10,2)=0x03, … (+472) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+94) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bombs and thieves at top: (16,2)=0x21, (17,2)=0x21, (18,2)=0x21, (19,2)=0x21, (20,2)=0x21, (21,2)=0x21, (22,2)=0x21, (23,2)=0x21, … (+80) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.

### Level 076 — FOUR PLEX

- Map-backed baseline: **32×32**, time limit **550**, chips required **20**, password `NMRH`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (23,2)=0x02, (30,2)=0x02, (10,3)=0x15, (11,5)=0x22, (10,7)=0x02, (12,7)=0x02, (23,8)=0x02, (23,10)=0x02, … (+14) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (9,9)=0x67, (13,9)=0x64, (7,11)=0x19, (15,11)=0x19, (7,13)=0x17, (15,13)=0x16, (9,14)=0x67, (13,14)=0x65, … (+7) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (1,1)=0x04, (2,1)=0x04, (3,1)=0x04, (4,1)=0x04, (5,1)=0x04, (6,1)=0x04, (7,1)=0x04, (8,1)=0x04, … (+179) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (10,8)=0x0A, (12,8)=0x0A, (10,9)=0x0A, (12,9)=0x0A, (10,10)=0x0A, (12,10)=0x0A, (27,18)=0x0A, (27,20)=0x0A, … (+3) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (5,11)=0x2D, (17,11)=0x2D, (26,29)=0x2D, (22,31)=0x2D, (26,31)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (11,3)=0x0C, (18,17)=0x1A, (18,18)=0x0C, (18,19)=0x0C, (28,19)=0x0C, (29,19)=0x0C, (18,20)=0x0C, (28,20)=0x0C, … (+86) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (25,14)=0x12, (25,15)=0x12, (25,16)=0x12, (23,20)=0x13, (24,20)=0x13, (26,20)=0x13, (5,21)=0x12, (5,22)=0x12, … (+3) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (9,1)=0x01, (10,1)=0x01, (11,1)=0x01, (12,1)=0x01, (13,1)=0x01, (21,1)=0x01, (22,1)=0x01, (23,1)=0x01, … (+405) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (26,5)=0x23, (28,5)=0x23, (31,5)=0x23, (28,6)=0x23, (26,7)=0x23, (27,7)=0x23, (30,7)=0x26, (26,8)=0x26, … (+10) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (29,5)=0x27, (30,5)=0x2B, (28,7)=0x27, (28,8)=0x2B, (29,9)=0x27, (30,9)=0x2B, (28,11)=0x27, (28,12)=0x2B, … (+2) — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (29,15)=0x24, (20,18)=0x24; bottom: (30,15)=0x31, (20,29)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- teleporters at top: (3,23)=0x29, (9,23)=0x29, (3,29)=0x29, (10,29)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (10,2)=0x2A, (30,3)=0x2A, (24,8)=0x2A, (24,10)=0x2A, (24,12)=0x2A, (27,14)=0x2A, (31,20)=0x2A, (31,22)=0x2A, … (+2) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (19,14)=0x40, (25,28)=0x43 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (22,22)=0x48, (22,26)=0x48, (20,29)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- gliders at top: (30,15)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- paramecia at top: (3,14)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 077 — INVINCIBLE CHAMPION

- Map-backed baseline: **32×32**, time limit **500**, chips required **5**, password `FHIC`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (11,9)=0x22, (12,9)=0x15, (17,11)=0x02, (15,12)=0x02, (18,12)=0x02, (15,14)=0x02, (17,14)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (19,15)=0x03, (20,15)=0x03, (21,15)=0x03, (18,16)=0x03, (19,16)=0x03, (21,16)=0x03, (22,16)=0x03, (23,16)=0x03, … (+16) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- dirt and gravel at top: (2,2)=0x0B, (4,2)=0x0B, (5,2)=0x0B, (6,2)=0x0B, (7,2)=0x0B, (8,2)=0x0B, (9,2)=0x0B, (10,2)=0x0B, … (+448) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+216) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (27,15)=0x24; bottom: (29,16)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bugs at top: (15,5)=0x41, (17,5)=0x40, (15,7)=0x42, (17,7)=0x40, (15,9)=0x42, (17,9)=0x40, (18,10)=0x40, (15,11)=0x41, … (+22) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- gliders at top: (29,16)=0x50, (25,29)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 078 — FORCE SQUARE

- Map-backed baseline: **32×32**, time limit **500**, chips required **13**, password `GRMO`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (4,2)=0x22, (10,11)=0x02, (14,11)=0x02, (18,11)=0x02, (22,11)=0x02, (12,15)=0x02, (20,15)=0x02, (10,19)=0x02, … (+7) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (2,2)=0x65, (30,8)=0x19, (30,26)=0x17, (24,31)=0x67 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- force floors at top: (8,9)=0x13, (9,9)=0x13, (10,9)=0x13, (11,9)=0x13, (12,9)=0x13, (13,9)=0x13, (14,9)=0x13, (15,9)=0x13, … (+169) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+287) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (6,3)=0x29, (30,3)=0x29, (16,15)=0x29, (22,30)=0x29, (2,31)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- fireballs at top: (26,16)=0x44, (31,18)=0x46 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- gliders at top: (26,29)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 079 — DRAWN AND QUARTERED

- Map-backed baseline: **32×32**, time limit **300**, chips required **56**, password `JINU`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,2)=0x02, (5,2)=0x02, (7,2)=0x02, (9,2)=0x02, (11,2)=0x02, (13,2)=0x02, (15,2)=0x02, (17,2)=0x02, … (+50) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (3,3)=0x2D, (5,3)=0x2D, (7,3)=0x2D, (9,3)=0x2D, (11,3)=0x2D, (13,3)=0x2D, (15,3)=0x2D, (17,3)=0x2D, … (+535) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+278) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (7,7)=0x29, (17,17)=0x29, (14,18)=0x29, (27,27)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- fireballs at top: (11,5)=0x47, (5,8)=0x44, (14,12)=0x46, (7,15)=0x45, (19,15)=0x44, (29,16)=0x46, (10,21)=0x44, (14,22)=0x46, … (+2) — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.

### Level 080 — VANISHING ACT

- Map-backed baseline: **32×32**, time limit **800**, chips required **17**, password `EVUG`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (25,3)=0x02, (3,4)=0x02, (16,5)=0x02, (10,9)=0x02, (14,11)=0x02, (5,12)=0x02, (30,12)=0x02, (8,14)=0x02, … (+11) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (2,2)=0x0B, (3,2)=0x0B, (4,2)=0x0B, (5,2)=0x0B, (7,2)=0x0B, (8,2)=0x0B, (9,2)=0x0B, (10,2)=0x0B, … (+484); bottom: (18,16)=0x0B — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+504) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 081 — WRITERS BLOCK

- Map-backed baseline: **32×32**, time limit **0**, chips required **56**, password `SCWF`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,2)=0x02, (5,2)=0x02, (7,2)=0x02, (9,2)=0x02, (11,2)=0x02, (13,2)=0x02, (15,2)=0x02, (17,2)=0x02, … (+52) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (3,3)=0x03, (4,3)=0x03, (5,3)=0x03, (6,3)=0x03, (7,3)=0x03, (8,3)=0x03, (9,3)=0x03, (10,3)=0x03, … (+142) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (4,13)=0x0A, (5,13)=0x0A, (6,13)=0x0A, (7,13)=0x0A, (8,13)=0x0A, (9,13)=0x0A, (12,13)=0x0A, (15,13)=0x0A, … (+61) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+236) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 082 — SOCIALIST ACTION

- Map-backed baseline: **32×32**, time limit **999**, chips required **0**, password `LLIO`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (31,13)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (3,27)=0x03, (4,27)=0x03, (3,28)=0x03, (4,28)=0x03, (3,29)=0x03, (4,29)=0x03, (5,29)=0x03, (3,30)=0x03, … (+2) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (30,13)=0x0A, (30,14)=0x0A, (16,15)=0x0A, (30,15)=0x0A, (7,19)=0x0A, (18,19)=0x0A, (23,19)=0x0A, (26,21)=0x0A, … (+4) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (2,15)=0x2D, (3,15)=0x2D, (4,15)=0x2D, (5,15)=0x2D, (6,15)=0x2D, (2,16)=0x2D, (3,16)=0x2D, (4,16)=0x2D, … (+39) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+286) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (4,2)=0x43, (7,2)=0x43, (10,2)=0x43, (13,2)=0x43, (16,2)=0x43, (19,2)=0x43, (22,2)=0x43, (25,2)=0x43, … (+65) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (15,24)=0x4B, (18,24)=0x48, (15,27)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- teeth at top: (8,3)=0x54, (9,3)=0x54, (10,3)=0x54, (11,3)=0x54, (5,27)=0x54, (31,27)=0x54, (2,29)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 083 — UP THE BLOCK

- Map-backed baseline: **32×32**, time limit **400**, chips required **14**, password `OVPJ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (7,2)=0x22, (2,6)=0x15, (25,12)=0x02, (2,14)=0x02, (28,15)=0x02, (5,17)=0x02, (18,20)=0x02, (19,20)=0x02, … (+8) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- blocks and block terrain at top: (23,7)=0x0A, (30,15)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (8,10)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- force floors at top: (23,3)=0x0D, (27,6)=0x0D, (26,10)=0x14, (30,10)=0x14 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+343) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bombs and thieves at top: (8,17)=0x2A, (16,18)=0x2A, (17,18)=0x2A, (18,18)=0x2A, (19,18)=0x2A, (20,18)=0x2A, (21,18)=0x2A, (22,18)=0x2A, … (+35) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- gliders at top: (7,8)=0x52, (6,11)=0x52, (14,11)=0x50, (11,12)=0x53, (8,13)=0x53, (5,14)=0x53, (6,22)=0x50, (27,22)=0x50, … (+5) — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 084 — WARS

- Map-backed baseline: **32×32**, time limit **600**, chips required **5**, password `UVEO`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (11,9)=0x22, (12,9)=0x15, (17,11)=0x02, (15,12)=0x02, (18,12)=0x02, (15,14)=0x02, (17,14)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (12,11)=0x03, (12,15)=0x03, (19,15)=0x03, (20,15)=0x03, (21,15)=0x03, (18,16)=0x03, (19,16)=0x03, (21,16)=0x03, … (+10) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (23,23)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (28,23)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+233) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (28,22)=0x24; bottom: (27,22)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bugs at top: (16,5)=0x41, (15,6)=0x42, (17,6)=0x40, (15,8)=0x42, (17,8)=0x40, (15,10)=0x42, (17,10)=0x40, (14,11)=0x41, … (+22) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (27,22)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (28,24)=0x48, (13,27)=0x48, (15,27)=0x48, (17,27)=0x48, (19,27)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 085 — TELENET

- Map-backed baseline: **32×32**, time limit **300**, chips required **15**, password `LEBX`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (13,3)=0x02, (30,6)=0x02, (20,9)=0x02, (31,9)=0x15, (6,10)=0x02, (27,10)=0x02, (10,13)=0x02, (23,13)=0x02, … (+9) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (11,2)=0x17, (3,6)=0x65, (2,23)=0x64, (29,31)=0x16 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- force floors at top: (15,9)=0x13, (15,10)=0x13, (9,18)=0x0D, (10,18)=0x0D, (30,25)=0x0D, (31,25)=0x0D — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+403) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (6,6)=0x29, (13,6)=0x29, (20,6)=0x29, (27,6)=0x29, (6,13)=0x29, (13,13)=0x29, (20,13)=0x29, (27,13)=0x29, … (+8) — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- fireballs at top: (2,6)=0x44, (12,6)=0x44, (19,6)=0x44, (26,6)=0x44, (5,13)=0x44, (9,13)=0x44, (19,13)=0x44, (26,13)=0x44, … (+8) — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.

### Level 086 — SUICIDE

- Map-backed baseline: **32×32**, time limit **400**, chips required **0**, password `FLHH`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (19,29)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- blocks and block terrain at top: (23,12)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+175) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (23,11)=0x24; bottom: (22,11)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (8,29)=0x2A, (9,29)=0x2A, (10,29)=0x2A, (11,29)=0x2A, (12,29)=0x2A, (13,29)=0x2A, (14,29)=0x2A, (15,29)=0x2A, … (+3) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- balls at top: (19,2)=0x4B, (2,6)=0x48, (31,8)=0x48, (24,16)=0x48, (25,16)=0x48, (26,16)=0x48, (23,17)=0x48, (7,20)=0x48, … (+7) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- gliders at top: (22,11)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 087 — CITYBLOCK

- Map-backed baseline: **32×32**, time limit **0**, chips required **34**, password `YJYS`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x02, (4,2)=0x02, (6,2)=0x02, (8,2)=0x02, (10,2)=0x02, (12,2)=0x02, (14,2)=0x02, (18,2)=0x02, … (+28) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (3,2)=0x03, (5,2)=0x03, (7,2)=0x03, (9,2)=0x03, (11,2)=0x03, (13,2)=0x03, (2,3)=0x03, (3,3)=0x03, … (+134) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (17,3)=0x0A, (18,3)=0x0A, (19,3)=0x0A, (21,3)=0x0A, (22,3)=0x0A, (23,3)=0x0A, (25,3)=0x0A, (26,3)=0x0A, … (+116) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (3,18)=0x0C, (4,18)=0x0C, (5,18)=0x0C, (11,20)=0x0C, (12,20)=0x0C, (15,20)=0x0C, (3,21)=0x0C, (4,21)=0x0C, … (+34) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (6,17)=0x12, (10,18)=0x0D, (6,19)=0x12, (10,19)=0x0D, (6,20)=0x12, (10,21)=0x0D, (6,22)=0x12, (10,22)=0x0D, … (+9) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+344) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (17,2)=0x26, (19,2)=0x26, (21,2)=0x25, (23,2)=0x26, (25,2)=0x25, (27,2)=0x25, (29,2)=0x25, (31,2)=0x26, … (+20) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.

### Level 088 — SPIRALS

- Map-backed baseline: **32×32**, time limit **400**, chips required **12**, password `WZYV`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (19,4)=0x02, (6,5)=0x02, (13,5)=0x02, (27,7)=0x02, (18,10)=0x02, (27,14)=0x02, (8,16)=0x02, (31,16)=0x22, … (+6) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+898) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (10,31)=0x24; bottom: (1,31)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- walkers at top: (1,31)=0x5B — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 089 — BLOCK BUSTER

- Map-backed baseline: **32×32**, time limit **450**, chips required **5**, password `VCZO`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (7,6)=0x02, (11,15)=0x02, (20,15)=0x02, (9,21)=0x02, (9,26)=0x22, (9,27)=0x15, (11,29)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (11,17)=0x17, (14,24)=0x66, (22,24)=0x65, (11,27)=0x18, (11,31)=0x18 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (19,18)=0x03, (20,18)=0x03, (6,19)=0x03, (7,19)=0x03, (8,19)=0x03, (9,19)=0x03, (18,19)=0x03, (19,19)=0x03, … (+13) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (8,8)=0x0F, (20,8)=0x0F, (8,10)=0x0F, (20,10)=0x0F, (8,12)=0x0F, (20,12)=0x0F, (8,14)=0x0F, (20,14)=0x0F, … (+9) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (4,5)=0x0C, (5,8)=0x0C, (6,8)=0x0C, (7,8)=0x0C, (15,8)=0x0C, (16,8)=0x0C, (17,8)=0x0C, (18,8)=0x0C, … (+77) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+393) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (4,8)=0x24, (14,8)=0x24, (4,10)=0x24, (14,10)=0x24, (4,12)=0x24, (14,12)=0x24, (4,14)=0x24, (14,14)=0x24, … (+9); bottom: (8,8)=0x31, (20,8)=0x31, (8,10)=0x31, (20,10)=0x31, (8,12)=0x31, (20,12)=0x31, (8,14)=0x31, (20,14)=0x31, … (+9) — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.

### Level 090 — PLAYHOUSE

- Map-backed baseline: **32×32**, time limit **400**, chips required **121**, password `OLLM`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,3)=0x02, (4,3)=0x02, (5,3)=0x02, (7,3)=0x02, (8,3)=0x02, (9,3)=0x02, (11,3)=0x02, (12,3)=0x02, … (+130) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (16,9)=0x18, (15,14)=0x17, (18,14)=0x17, (9,15)=0x18, (16,15)=0x64, (23,15)=0x18, (16,16)=0x19, (27,16)=0x66, … (+4) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (2,10)=0x03, (3,10)=0x03, (4,10)=0x03, (5,10)=0x03, (6,10)=0x03, (7,10)=0x03, (8,10)=0x03, (9,10)=0x03, … (+108); bottom: (2,11)=0x69, (4,28)=0x04 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (2,11)=0x0A, (31,11)=0x0A, (9,13)=0x0A, (24,13)=0x0A, (10,21)=0x0A, (11,21)=0x0A, (12,21)=0x0A, (13,21)=0x0A, … (+11) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (30,25)=0x0C, (30,26)=0x0C, (15,27)=0x0C, (30,27)=0x0C, (11,28)=0x1A, (12,28)=0x0C, (13,28)=0x0C, (14,28)=0x0C, … (+30) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (6,18)=0x13, (13,25)=0x12, (6,26)=0x13, (7,26)=0x13, (8,26)=0x13, (9,26)=0x13, (10,26)=0x13, (11,26)=0x13, … (+3) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+256) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (12,16)=0x2B, (13,16)=0x2B, (14,16)=0x2B, (15,16)=0x2B, (17,16)=0x2B, (18,16)=0x2B, (19,16)=0x2B, (20,16)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- balls at top: (4,9)=0x4B, (23,9)=0x4B, (7,22)=0x48, (25,22)=0x48, (8,24)=0x48, (24,24)=0x48, (21,26)=0x48, (24,26)=0x48, … (+6) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 091 — JUMPING SWARM

- Map-backed baseline: **32×32**, time limit **400**, chips required **22**, password `JPQG`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (13,4)=0x02, (24,4)=0x02, (2,7)=0x02, (7,7)=0x02, (27,9)=0x02, (12,10)=0x02, (19,10)=0x02, (31,11)=0x02, … (+16) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- force floors at top: (30,2)=0x13, (31,2)=0x0D, (31,3)=0x0D — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+172) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (29,2)=0x23; bottom: (3,2)=0x26 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- clone machines at top: (4,2)=0x24; bottom: (2,2)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- walkers at top: (2,2)=0x5B, (3,2)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 092 — VORTEX

- Map-backed baseline: **32×32**, time limit **500**, chips required **6**, password `DTMI`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,3)=0x02, (17,5)=0x02, (9,11)=0x02, (21,11)=0x02, (17,13)=0x02, (17,21)=0x02, (17,23)=0x22, (18,23)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (30,2)=0x64, (17,22)=0x16, (14,23)=0x66 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- blocks and block terrain at top: (4,12)=0x0A, (2,22)=0x0A, (28,24)=0x0A, (30,24)=0x0A, (28,26)=0x0A, (30,26)=0x0A, (4,27)=0x0A, (11,27)=0x0A, … (+4) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+720) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (3,2)=0x29, (17,11)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- fireballs at top: (8,2)=0x47, (13,2)=0x47, (17,2)=0x47, (21,2)=0x47, (26,2)=0x46, (31,3)=0x46, (10,4)=0x47, (15,4)=0x47, … (+33) — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- walkers at top: (2,6)=0x58, (30,7)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 093 — ROADSIGN

- Map-backed baseline: **32×32**, time limit **800**, chips required **4**, password `REKF`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (11,5)=0x02, (2,8)=0x02, (16,10)=0x02, (20,12)=0x02, (14,16)=0x15, (14,17)=0x22 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (2,7)=0x03, (2,11)=0x03, (3,11)=0x03, (4,11)=0x03, (5,11)=0x03, (6,11)=0x03, (7,11)=0x03, (8,11)=0x03, … (+76) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (20,8)=0x0A, (17,14)=0x0A, (19,14)=0x0A, (3,19)=0x0A, (27,27)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (8,5)=0x2D, (15,12)=0x2D, (8,19)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (1,2)=0x1A, (1,3)=0x1D, (31,7)=0x0C, (31,8)=0x0C, (31,9)=0x0C, (31,10)=0x0C, (31,11)=0x0C, (31,12)=0x0C, … (+15) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (16,13)=0x12, (26,16)=0x14, (14,21)=0x0D, (4,23)=0x12, (6,23)=0x12, (9,24)=0x13, (15,24)=0x14, (9,26)=0x13, … (+5) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+312) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (14,2)=0x23, (8,3)=0x26, (13,6)=0x25, (17,6)=0x25, (14,7)=0x25, (16,7)=0x25, (15,8)=0x25, (2,16)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- bombs and thieves at top: (16,12)=0x2A, (18,12)=0x2A — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- balls at top: (13,4)=0x48, (14,4)=0x48, (15,4)=0x48, (16,4)=0x48, (17,4)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- gliders at top: (14,14)=0x50, (12,15)=0x50, (10,16)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 094 — NOW YOU SEE IT

- Map-backed baseline: **32×32**, time limit **0**, chips required **7**, password `EWCS`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (26,2)=0x02, (24,6)=0x02, (10,14)=0x02, (26,16)=0x02, (14,22)=0x02, (26,26)=0x02, (4,30)=0x02, (15,30)=0x22, … (+1) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x2C, (2,1)=0x2C, (3,1)=0x2C, (4,1)=0x01, (5,1)=0x2C, (6,1)=0x2C, (7,1)=0x2C, (8,1)=0x1F, … (+610) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 095 — FOUR SQUARE

- Map-backed baseline: **32×32**, time limit **350**, chips required **2**, password `BIFQ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (4,6)=0x02, (15,6)=0x02, (21,11)=0x02, (28,11)=0x02, (3,12)=0x02, (14,12)=0x02, (20,12)=0x02, (28,16)=0x02, … (+2) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (9,4)=0x17, (13,6)=0x65, (4,9)=0x19, (14,9)=0x16, (22,11)=0x16, (24,11)=0x17, (25,11)=0x66, (5,12)=0x65, … (+11) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (23,12)=0x03, (21,14)=0x03, (25,14)=0x03, (23,16)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (9,7)=0x0A, (7,9)=0x0A, (11,9)=0x0A, (9,11)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- force floors at top: (18,9)=0x0D, (19,9)=0x14, (20,9)=0x14, (21,9)=0x14, (22,9)=0x14, (23,9)=0x14, (24,9)=0x14, (25,9)=0x14, … (+30) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+218) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (9,9)=0x29, (23,14)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- fireballs at top: (28,14)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- walkers at top: (3,3)=0x58, (5,3)=0x58, (7,3)=0x58, (9,3)=0x58, (11,3)=0x58, (13,3)=0x58, (15,3)=0x58, (3,16)=0x58, … (+6) — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 096 — PARANOIA

- Map-backed baseline: **32×32**, time limit **399**, chips required **46**, password `BIFQ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (24,2)=0x02, (25,2)=0x02, (26,2)=0x02, (25,3)=0x02, (4,5)=0x15, (5,5)=0x22, (19,6)=0x02, (31,6)=0x02, … (+40) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (15,7)=0x64, (7,10)=0x16, (15,12)=0x65, (7,13)=0x17, (25,16)=0x65, (30,16)=0x17 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (23,2)=0x04, (19,3)=0x04, (24,3)=0x04, (26,3)=0x04, (25,4)=0x04, (9,5)=0x04, (11,5)=0x04, (13,5)=0x04, … (+35) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (2,2)=0x1A, (17,2)=0x1B, (17,4)=0x0C, (8,14)=0x1B, (10,14)=0x1A, (11,14)=0x0C, (12,14)=0x1B, (14,14)=0x1A, … (+25) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+495) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (15,8)=0x28, (12,10)=0x28 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- hints at top: (9,10)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- bugs at top: (30,2)=0x41, (2,7)=0x43, (24,7)=0x43, (25,8)=0x42, (20,12)=0x43, (5,14)=0x41 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (25,17)=0x49, (13,18)=0x49, (13,20)=0x49, (29,20)=0x49, (13,22)=0x49, (25,22)=0x49, (13,24)=0x49, (29,24)=0x49, … (+6) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (7,8)=0x4F, (15,11)=0x4D — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- paramecia at top: (15,18)=0x62, (21,18)=0x61, (15,31)=0x63, (21,31)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 097 — METASTABLE TO CHAOS

- Map-backed baseline: **32×32**, time limit **300**, chips required **0**, password `IOCS`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (27,7)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- blocks and block terrain at top: (28,5)=0x0A, (29,5)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (2,2)=0x0B, (3,2)=0x0B, (4,2)=0x0B, (5,2)=0x0B, (7,2)=0x0B, (10,2)=0x0B, (12,2)=0x0B, (13,2)=0x0B, … (+577) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+140) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (24,5)=0x40, (23,6)=0x40, (25,6)=0x40, (22,7)=0x40, (24,7)=0x40, (26,7)=0x40, (21,8)=0x40, (23,8)=0x40, … (+17) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.

### Level 098 — SHRINKING

- Map-backed baseline: **32×32**, time limit **350**, chips required **7**, password `TKWD`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (28,4)=0x22, (5,5)=0x02, (16,5)=0x02, (27,5)=0x22, (28,5)=0x15, (29,5)=0x22, (28,6)=0x22, (5,16)=0x02, … (+4) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- blocks and block terrain at top: (10,7)=0x0A, (11,7)=0x0A, (10,8)=0x0A, (22,8)=0x0A, (23,8)=0x0A, (23,9)=0x0A, (25,9)=0x0A, (26,9)=0x0A, … (+25) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (2,2)=0x1A, (3,2)=0x0C, (5,2)=0x0C, (7,2)=0x1B, (14,3)=0x1A, (15,3)=0x0C, (17,3)=0x0C, (19,3)=0x1B, … (+140); bottom: (4,2)=0x0C, (6,2)=0x0C, (2,3)=0x0C, (7,3)=0x0C, (16,3)=0x0C, (18,3)=0x0C, (28,3)=0x0C, (29,3)=0x0C, … (+60) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (8,3)=0x14, (13,4)=0x13, (20,4)=0x14, (25,4)=0x13, (8,6)=0x14, (13,6)=0x13, (20,6)=0x14, (25,6)=0x13, … (+38) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+152) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- balls at top: (4,2)=0x49, (6,2)=0x49, (2,3)=0x4A, (7,3)=0x48, (16,3)=0x49, (18,3)=0x49, (28,3)=0x49, (29,3)=0x49, … (+60) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 099 — CATACOMBS

- Map-backed baseline: **32×32**, time limit **399**, chips required **8**, password `XUVU`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (18,6)=0x02, (17,7)=0x02, (10,13)=0x02, (18,15)=0x02, (24,16)=0x02, (28,16)=0x22, (29,16)=0x15, (18,17)=0x02, … (+3) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (26,5)=0x19, (26,6)=0x66, (25,7)=0x66, (26,7)=0x64, (26,8)=0x66, (18,9)=0x17, (26,9)=0x19, (18,11)=0x67, … (+31) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (16,8)=0x04, (8,12)=0x03, (4,13)=0x03, (5,13)=0x03, (6,13)=0x03, (7,13)=0x04, (8,13)=0x04, (5,14)=0x03, … (+11); bottom: (6,14)=0x68 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (21,5)=0x0A, (21,9)=0x0A, (13,10)=0x0A, (6,12)=0x0A, (17,12)=0x0A, (6,14)=0x0A, (7,14)=0x0A, (8,14)=0x0A, … (+16) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- force floors at top: (21,7)=0x32, (13,12)=0x32, (7,16)=0x32, (21,16)=0x32, (13,20)=0x32, (21,25)=0x32 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+277) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (21,6)=0x27, (21,8)=0x27, (2,11)=0x27, (13,11)=0x27, (13,13)=0x27, (7,15)=0x27, (21,15)=0x27, (7,17)=0x27, … (+6); bottom: (24,6)=0x2B, (24,8)=0x2B, (7,11)=0x2B, (17,11)=0x2B, (17,13)=0x2B, (12,14)=0x2B, (9,15)=0x2B, (24,15)=0x2B, … (+9) — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (17,6)=0x24, (18,7)=0x24, (17,8)=0x24, (9,11)=0x24, (10,12)=0x24, (9,13)=0x24, (3,15)=0x24, (17,15)=0x24, … (+10); bottom: (19,6)=0x31, (20,7)=0x31, (19,8)=0x31, (11,11)=0x31, (12,12)=0x31, (11,13)=0x31, (5,15)=0x31, (19,15)=0x31, … (+10) — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bugs at top: (17,11)=0x43, (17,13)=0x43, (9,15)=0x43, (9,17)=0x43, (17,19)=0x43, (17,21)=0x43 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (24,6)=0x4B, (24,8)=0x4B, (24,15)=0x4B, (24,17)=0x4B, (24,24)=0x4B, (24,26)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (19,6)=0x4F, (20,7)=0x4F, (19,8)=0x4F, (7,11)=0x4D, (11,11)=0x4F, (12,12)=0x4F, (11,13)=0x4F, (5,15)=0x4F, … (+12) — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- walkers at top: (6,3)=0x59, (6,29)=0x59 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 100 — COLONY

- Map-backed baseline: **32×32**, time limit **0**, chips required **48**, password `QJXR`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (6,3)=0x02, (10,3)=0x02, (14,3)=0x02, (18,3)=0x02, (22,3)=0x02, (26,3)=0x02, (6,7)=0x02, (10,7)=0x02, … (+45) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+415) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- fireballs at top: (5,3)=0x44, (13,3)=0x44, (21,3)=0x44, (29,3)=0x44, (2,7)=0x44, (9,7)=0x44, (17,7)=0x44, (25,7)=0x44, … (+23) — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- gliders at top: (3,3)=0x50, (11,3)=0x50, (19,3)=0x50, (27,3)=0x50, (7,7)=0x50, (15,7)=0x50, (23,7)=0x50, (31,7)=0x50, … (+24) — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 101 — APARTMENT

- Map-backed baseline: **32×32**, time limit **300**, chips required **34**, password `RPIR`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (5,2)=0x02, (29,2)=0x22, (4,3)=0x02, (30,4)=0x15, (8,5)=0x02, (31,5)=0x02, (7,6)=0x02, (27,7)=0x02, … (+28) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+560) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (19,15)=0x42, (14,16)=0x40, (14,18)=0x40, (19,18)=0x42 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.

### Level 102 — ICEHOUSE

- Map-backed baseline: **32×32**, time limit **200**, chips required **3**, password `VDDU`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (6,7)=0x02, (8,7)=0x02, (26,9)=0x02, (7,21)=0x22, (7,27)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (9,6)=0x65, (27,9)=0x67, (26,10)=0x17, (13,16)=0x19, (14,21)=0x19, (15,21)=0x19, (16,21)=0x19, (17,21)=0x19, … (+4) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (6,6)=0x69, (11,6)=0x03, (27,10)=0x68, (7,11)=0x03, (14,15)=0x04, (15,15)=0x03, (16,15)=0x04, (17,15)=0x03, … (+6) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (10,9)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (2,2)=0x0C, (3,2)=0x0C, (4,2)=0x0C, (5,2)=0x1A, (6,2)=0x0C, (7,2)=0x0C, (8,2)=0x0C, (9,2)=0x0C, … (+554); bottom: (10,2)=0x0C, (18,2)=0x0C, (5,13)=0x0C, (5,17)=0x0C, (6,28)=0x0C, (14,28)=0x0C, (22,28)=0x0C, (30,28)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (6,25)=0x13, (7,25)=0x13, (8,25)=0x13, (9,25)=0x13, (10,25)=0x13, (11,25)=0x13, (12,25)=0x13, (13,25)=0x13, … (+8); bottom: (26,2)=0x13 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+291) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (9,26)=0x28, (20,26)=0x28 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (9,9)=0x24, (6,11)=0x24; bottom: (15,10)=0x31, (11,11)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- teleporters at top: (27,2)=0x29, (18,13)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (10,16)=0x21, (10,18)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- fireballs at top: (11,11)=0x45, (14,28)=0x45 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (10,2)=0x4B, (6,10)=0x48, (6,28)=0x49 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (13,26)=0x4D — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- gliders at top: (18,2)=0x53, (5,17)=0x50, (30,28)=0x51 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- teeth at top: (15,10)=0x55 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- walkers at top: (5,13)=0x58, (22,28)=0x59 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 103 — MEMORY

- Map-backed baseline: **32×32**, time limit **600**, chips required **37**, password `PTAC`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (6,2)=0x02, (14,2)=0x02, (22,2)=0x02, (2,3)=0x15, (4,3)=0x22, (18,3)=0x02, (26,3)=0x02, (7,6)=0x02, … (+31) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+374) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (30,2)=0x23, (8,3)=0x26, (10,3)=0x23, (12,3)=0x25, (16,3)=0x26, (24,3)=0x25, (28,3)=0x25, (14,4)=0x26, … (+45) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (11,7)=0x28, (27,7)=0x28, (2,11)=0x28, (23,14)=0x28, (2,19)=0x28, (10,19)=0x28, (18,19)=0x28, (22,26)=0x28, … (+3) — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- tanks at top: (6,4)=0x4F, (20,4)=0x4E, (30,4)=0x4F, (4,5)=0x4F, (11,5)=0x4D, (21,5)=0x4C, (16,8)=0x4F, (25,8)=0x4E, … (+29) — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.

### Level 104 — JAILER

- Map-backed baseline: **32×32**, time limit **300**, chips required **82**, password `KWNL`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (16,2)=0x02, (18,2)=0x02, (4,4)=0x02, (5,4)=0x02, (6,4)=0x02, (7,4)=0x02, (8,4)=0x02, (9,4)=0x02, … (+78) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (22,6)=0x17, (6,12)=0x19, (17,12)=0x18, (17,15)=0x64, (12,17)=0x18, (15,17)=0x66, (19,17)=0x67, (22,17)=0x18, … (+4) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+424) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- balls at top: (17,7)=0x48, (19,11)=0x48, (3,15)=0x4B, (6,17)=0x4B, (27,17)=0x4B, (28,19)=0x4B, (15,24)=0x48, (17,26)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- walkers at top: (5,8)=0x58, (26,8)=0x58, (5,28)=0x58, (26,28)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 105 — SHORT CIRCUIT

- Map-backed baseline: **32×32**, time limit **350**, chips required **37**, password `YNEG`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (13,1)=0x15, (13,2)=0x22, (3,3)=0x22, (6,3)=0x02, (13,3)=0x22, (3,4)=0x22, (5,4)=0x22, (6,4)=0x22, … (+411) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+596) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 106 — KABLAM

- Map-backed baseline: **32×32**, time limit **0**, chips required **0**, password `NXYB`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,30)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (2,2)=0x03, (3,2)=0x03, (4,2)=0x03, (5,2)=0x03, (6,2)=0x03, (7,2)=0x03, (8,2)=0x03, (9,2)=0x03, … (+147) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (5,11)=0x0A, (8,11)=0x0A, (25,11)=0x0A, (28,11)=0x0A, (4,21)=0x0A, (29,21)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+118) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (19,31)=0x24; bottom: (23,31)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (4,3)=0x2A, (5,3)=0x2A, (6,3)=0x2A, (7,3)=0x2A, (8,3)=0x2A, (9,3)=0x2A, (10,3)=0x2A, (11,3)=0x2A, … (+582) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- fireballs at top: (23,31)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (18,31)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 107 — BALLS O FIRE

- Map-backed baseline: **32×32**, time limit **300**, chips required **50**, password `ECRE`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (17,4)=0x22, (18,4)=0x22, (16,5)=0x22, (17,5)=0x15, (18,5)=0x15, (19,5)=0x22, (16,6)=0x22, (17,6)=0x15, … (+54) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (8,19)=0x19, (9,19)=0x19, (25,19)=0x16, (26,19)=0x16, (3,31)=0x64, (30,31)=0x67 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (16,3)=0x04, (17,3)=0x04, (18,3)=0x04, (19,3)=0x04, (15,4)=0x04, (16,4)=0x04, (19,4)=0x04, (20,4)=0x04, … (+128) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (17,19)=0x0A, (5,29)=0x0A, (28,29)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+368) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at bottom: (2,4)=0x2B, (3,4)=0x2B, (4,4)=0x2B, (5,4)=0x2B, (6,4)=0x2B, (28,4)=0x2B, (29,4)=0x2B, (30,4)=0x2B, … (+71) — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- bombs and thieves at top: (8,2)=0x2A, (10,2)=0x2A, (15,2)=0x2A, (16,2)=0x2A, (17,2)=0x2A, (18,2)=0x2A, (19,2)=0x2A, (20,2)=0x2A, … (+41) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- fireballs at top: (15,11)=0x47, (14,14)=0x44, (20,14)=0x46, (17,17)=0x45 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (9,2)=0x4A, (27,2)=0x4B, (27,3)=0x4B, (2,4)=0x48, (3,4)=0x48, (4,4)=0x48, (5,4)=0x48, (6,4)=0x48, … (+87) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 108 — BLOCK OUT

- Map-backed baseline: **32×32**, time limit **350**, chips required **31**, password `LIOC`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (28,11)=0x02, (31,11)=0x02, (28,13)=0x02, (31,13)=0x02, (28,15)=0x02, (31,15)=0x02, (2,17)=0x02, (28,17)=0x02, … (+25) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (3,7)=0x17, (4,14)=0x16, (2,15)=0x64, (20,18)=0x65, (4,24)=0x64, (24,31)=0x16 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (3,3)=0x04, (5,3)=0x04, (7,3)=0x04, (9,3)=0x04, (11,3)=0x04, (13,3)=0x04, (15,3)=0x04, (17,3)=0x04, … (+17) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (19,13)=0x0A, (20,13)=0x0A, (21,13)=0x0A, (19,14)=0x0A, (21,14)=0x0A, (19,15)=0x0A, (20,15)=0x0A, (21,15)=0x0A, … (+19) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (20,3)=0x0C, (20,4)=0x0C, (20,5)=0x0C, (20,6)=0x0C, (20,7)=0x0C, (20,8)=0x0C, (20,9)=0x0C, (20,10)=0x1D, … (+77) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+363) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- balls at top: (22,2)=0x4A, (24,2)=0x4A, (26,2)=0x4A, (28,2)=0x4A, (30,2)=0x4A, (4,4)=0x4A, (5,4)=0x4A, (6,4)=0x4A, … (+30) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 109 — TORTURECHAMBER

- Map-backed baseline: **32×32**, time limit **150**, chips required **4**, password `KZQR`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (7,13)=0x15, (26,13)=0x02, (19,16)=0x02, (9,18)=0x22, (13,18)=0x02, (17,18)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (13,11)=0x04, (19,11)=0x04, (10,12)=0x04, (16,12)=0x04, (22,12)=0x04 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (21,18)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (11,16)=0x2D, (15,16)=0x2D, (16,16)=0x2D, (10,20)=0x2D, (11,20)=0x2D, (15,20)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (23,10)=0x0C, (23,11)=0x0C, (24,11)=0x0C, (25,11)=0x1B, (23,12)=0x0C, (25,12)=0x0C, (24,13)=0x0C, (25,13)=0x0C, … (+12) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (10,11)=0x13, (11,11)=0x13, (12,11)=0x13, (14,11)=0x13, (15,11)=0x13, (16,11)=0x13, (17,11)=0x13, (18,11)=0x13, … (+17) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+879) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (21,17)=0x27, (19,18)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- bombs and thieves at top: (17,16)=0x2A — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- fireballs at top: (11,9)=0x47, (21,9)=0x47, (16,14)=0x45, (13,16)=0x47, (13,20)=0x45 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- gliders at top: (16,9)=0x53, (11,14)=0x51, (21,14)=0x51, (13,17)=0x51, (13,19)=0x53 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 110 — CHILLER

- Map-backed baseline: **32×32**, time limit **399**, chips required **16**, password `XBAO`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (9,6)=0x02, (17,7)=0x02, (21,7)=0x02, (31,10)=0x02, (19,14)=0x02, (2,15)=0x22, (6,15)=0x02, (8,15)=0x02, … (+10) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (7,17)=0x16, (31,23)=0x64 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (2,7)=0x04, (8,7)=0x04, (17,8)=0x03, (21,8)=0x03, (13,14)=0x03, (15,14)=0x03, (20,14)=0x03, (4,17)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (4,5)=0x0A, (6,5)=0x0A, (5,6)=0x0A, (5,7)=0x0A, (5,8)=0x0A, (4,9)=0x0A, (6,9)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (13,2)=0x1A, (14,2)=0x0C, (15,2)=0x0C, (16,2)=0x0C, (17,2)=0x0C, (18,2)=0x0C, (19,2)=0x0C, (20,2)=0x0C, … (+331) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (14,22)=0x13, (15,22)=0x13, (16,22)=0x13, (17,22)=0x13, (18,22)=0x13, (19,22)=0x13, (20,22)=0x13, (21,22)=0x13, … (+26) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+225) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (5,13)=0x23, (2,14)=0x25, (3,22)=0x25, (5,23)=0x26, (9,23)=0x25, (12,23)=0x26 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- bombs and thieves at top: (32,7)=0x2A — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- balls at top: (11,2)=0x48, (3,13)=0x4B, (8,18)=0x4B, (2,20)=0x4B, (7,20)=0x48, (10,20)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 111 — TIME LAPSE

- Map-backed baseline: **32×32**, time limit **0**, chips required **4**, password `KRQJ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (8,28)=0x02, (6,31)=0x02, (15,31)=0x22, (16,31)=0x15, (17,31)=0x22, (25,31)=0x02, (27,31)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (2,5)=0x03, (26,12)=0x03, (27,12)=0x03, (28,12)=0x03, (25,14)=0x03, (26,14)=0x03, (27,14)=0x03, (28,14)=0x03, … (+39); bottom: (17,17)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (15,15)=0x0A, (17,15)=0x0A, (15,19)=0x0A, (17,19)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (14,26)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (12,5)=0x1A, (13,5)=0x0C, (14,5)=0x0C, (15,5)=0x0C, (16,5)=0x0C, (17,5)=0x0C, (18,5)=0x0C, (19,5)=0x0C, … (+48) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (15,7)=0x14, (19,8)=0x12, (18,9)=0x14, (14,12)=0x13, (18,12)=0x13, (19,23)=0x12, (18,24)=0x14 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+299) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (3,5)=0x23, (14,8)=0x26, (15,12)=0x25, (16,12)=0x23, (19,12)=0x25, (8,14)=0x26, (9,14)=0x26, (10,14)=0x26, … (+100); bottom: (15,15)=0x26, (17,15)=0x26, (17,18)=0x26, (15,19)=0x26, (17,19)=0x26 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (16,7)=0x2B, (17,7)=0x27, (23,12)=0x2B, (25,12)=0x27 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (21,4)=0x24, (14,11)=0x24, (17,12)=0x24, (27,17)=0x24; bottom: (19,10)=0x31, (11,13)=0x31, (19,25)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (18,8)=0x2A, (17,9)=0x2A, (17,10)=0x2A, (18,10)=0x2A, (14,22)=0x2A, (15,22)=0x2A, (14,23)=0x2A, (14,24)=0x2A, … (+1) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (19,10)=0x40, (22,12)=0x42, (19,25)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (11,13)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (12,4)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- gliders at top: (17,17)=0x53 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 112 — FORTUNE FAVOURS THE

- Map-backed baseline: **32×32**, time limit **0**, chips required **4**, password `NJLA`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (5,2)=0x15, (5,3)=0x22, (6,6)=0x02, (7,9)=0x02, (6,10)=0x02; bottom: (8,7)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- blocks and block terrain at top: (8,7)=0x0A, (17,15)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- force floors at top: (11,5)=0x0D — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+154) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (9,2)=0x24, (10,4)=0x24; bottom: (11,2)=0x31, (5,7)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bugs at top: (5,7)=0x43 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (11,2)=0x46, (11,4)=0x46, (11,7)=0x46, (11,9)=0x46, (11,11)=0x46, (11,13)=0x46, (11,15)=0x46, (11,17)=0x46, … (+2) — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (8,2)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 113 — OPEN QUESTION

- Map-backed baseline: **32×32**, time limit **500**, chips required **5**, password `PTAS`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (9,7)=0x02, (6,11)=0x02, (22,11)=0x02, (9,14)=0x02, (27,16)=0x02, (14,31)=0x15, (15,31)=0x22 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (30,23)=0x03, (31,23)=0x03, (2,24)=0x03, (3,24)=0x03, (4,24)=0x03, (5,24)=0x03, (6,24)=0x03, (7,24)=0x03, … (+54) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (14,28)=0x0A, (18,28)=0x0A, (14,29)=0x0A, (15,29)=0x0A, (16,29)=0x0A, (17,29)=0x0A, (19,29)=0x0A, (17,30)=0x0A, … (+2) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+269) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (5,23)=0x24; bottom: (2,23)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- hints at top: (17,31)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- fireballs at top: (2,23)=0x47, (3,23)=0x47 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (20,6)=0x48, (25,7)=0x48, (7,9)=0x48, (22,9)=0x48, (23,9)=0x48, (13,10)=0x48, (18,10)=0x4B, (26,10)=0x48, … (+20) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 114 — DECEPTION

- Map-backed baseline: **32×32**, time limit **200**, chips required **12**, password `JWNL`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (24,10)=0x02, (19,12)=0x02, (23,12)=0x02, (13,13)=0x02, (19,14)=0x02, (19,16)=0x02, (23,16)=0x02, (13,17)=0x02, … (+6) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (23,23)=0x19, (14,25)=0x67 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (12,6)=0x04, (16,6)=0x04, (18,6)=0x04, (20,6)=0x04, (22,6)=0x04, (24,6)=0x04, (12,8)=0x04, (15,8)=0x04, … (+11) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (25,10)=0x1B, (25,11)=0x0C, (25,12)=0x0C, (25,13)=0x0C, (25,14)=0x0C, (25,15)=0x0C, (25,16)=0x0C, (25,17)=0x0C, … (+30) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (11,5)=0x01, (12,5)=0x01, (13,5)=0x01, (14,5)=0x01, (15,5)=0x01, (16,5)=0x01, (17,5)=0x01, (18,5)=0x01, … (+191) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- fireballs at top: (13,16)=0x44, (13,18)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- gliders at top: (23,22)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- teeth at top: (14,24)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- paramecia at top: (15,14)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 115 — OVERSEA DELIVERY

- Map-backed baseline: **32×32**, time limit **0**, chips required **0**, password `EGRW`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (8,8)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (2,2)=0x03, (3,2)=0x03, (4,2)=0x03, (5,2)=0x03, (6,2)=0x03, (7,2)=0x03, (8,2)=0x03, (9,2)=0x03, … (+494) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (28,26)=0x0A, (26,28)=0x0A, (30,28)=0x0A, (28,30)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+116) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (5,5)=0x29, (13,5)=0x29, (20,5)=0x29, (28,5)=0x29, (5,13)=0x29, (13,13)=0x29, (20,13)=0x29, (28,13)=0x29, … (+8) — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- hints at top: (9,12)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.

### Level 116 — BLOCK BUSTER II

- Map-backed baseline: **32×32**, time limit **750**, chips required **27**, password `HXMF`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (16,3)=0x02, (18,3)=0x02, (15,4)=0x02, (19,4)=0x02, (14,5)=0x02, (20,5)=0x02, (13,6)=0x02, (21,6)=0x02, … (+21) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (17,2)=0x64, (10,15)=0x16, (13,17)=0x66, (11,27)=0x18, (11,31)=0x18 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (16,2)=0x03, (15,3)=0x03, (17,3)=0x03, (14,4)=0x03, (18,4)=0x03, (13,5)=0x03, (19,5)=0x03, (12,6)=0x03, … (+15) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (22,8)=0x0F, (22,9)=0x0F, (22,10)=0x0F, (22,11)=0x0F, (22,12)=0x0F, (22,13)=0x0F, (22,24)=0x0F, (22,25)=0x0F, … (+4) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (13,2)=0x0C, (14,2)=0x0C, (15,2)=0x0C, (13,3)=0x0C, (14,3)=0x0C, (13,4)=0x0C, (16,4)=0x0C, (17,4)=0x0C, … (+108) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (9,1)=0x01, (10,1)=0x01, (11,1)=0x01, (12,1)=0x01, (13,1)=0x01, (14,1)=0x01, (15,1)=0x01, (16,1)=0x01, … (+198) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (15,8)=0x24, (20,8)=0x24, (16,9)=0x24, (19,9)=0x24, (17,10)=0x24, (18,10)=0x24, (17,11)=0x24, (18,11)=0x24, … (+10); bottom: (22,8)=0x31, (22,9)=0x31, (22,10)=0x31, (22,11)=0x31, (22,12)=0x31, (22,13)=0x31, (22,24)=0x31, (22,25)=0x31, … (+4) — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.

### Level 117 — THE MARSH

- Map-backed baseline: **32×32**, time limit **0**, chips required **0**, password `FPZT`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (31,31)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (4,2)=0x03, (6,2)=0x03, (8,2)=0x03, (10,2)=0x03, (11,2)=0x03, (12,2)=0x03, (13,2)=0x03, (14,2)=0x03, … (+620); bottom: (32,1)=0x04 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (32,1)=0x0A, (3,2)=0x0A, (2,3)=0x0A, (4,3)=0x0A, (3,4)=0x0A, (4,5)=0x0A, (6,5)=0x0A, (8,5)=0x0A, … (+119) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+115) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 118 — MISS DIRECTION

- Map-backed baseline: **32×32**, time limit **300**, chips required **0**, password `OSCW`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (29,6)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (18,2)=0x65, (18,3)=0x16, (6,5)=0x17, (18,16)=0x66, (28,26)=0x18, (30,27)=0x64 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (2,2)=0x03, (3,4)=0x68, (29,7)=0x03, (18,8)=0x68, (18,9)=0x03, (18,13)=0x04, (18,14)=0x69, (24,20)=0x04, … (+2) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (2,3)=0x0C, (2,4)=0x0C, (2,5)=0x0C, (31,5)=0x0C, (9,6)=0x1B, (31,6)=0x0C, (25,7)=0x1A, (26,7)=0x0C, … (+217) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (2,24)=0x0D, (3,24)=0x14, (4,24)=0x14, (5,24)=0x14, (6,24)=0x14, (7,24)=0x14, (8,24)=0x14, (2,25)=0x0D, … (+9) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+369) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (12,5)=0x23, (24,5)=0x23, (18,7)=0x26 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (18,17)=0x27, (20,17)=0x28, (23,17)=0x2B, (22,20)=0x27, (15,23)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- teleporters at top: (3,2)=0x29, (18,11)=0x29, (25,20)=0x29, (29,29)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (18,10)=0x21, (18,12)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (20,8)=0x40, (20,13)=0x40, (23,15)=0x40, (20,18)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (5,3)=0x44, (20,10)=0x44, (22,13)=0x44, (20,16)=0x44, (22,16)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (9,7)=0x4A, (16,13)=0x4A, (15,25)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (23,8)=0x4C, (20,14)=0x4C, (22,14)=0x4C, (21,18)=0x4C — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- gliders at top: (23,7)=0x50, (20,11)=0x50, (23,13)=0x50, (20,15)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- teeth at top: (9,23)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- walkers at top: (22,7)=0x58, (23,11)=0x58, (21,12)=0x58, (21,17)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.
- blobs at top: (21,8)=0x5C, (22,10)=0x5C, (23,12)=0x5C, (21,13)=0x5C — `NEEDS CAPTURE`: Compare random movement/seed, speed schedule, blocked moves and collision.
- paramecia at top: (20,9)=0x60, (23,10)=0x60, (21,15)=0x60, (23,16)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 119 — SLIDE STEP

- Map-backed baseline: **32×32**, time limit **250**, chips required **38**, password `PHTY`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,4)=0x02, (13,6)=0x02, (15,6)=0x02, (17,6)=0x02, (19,6)=0x02, (21,6)=0x02, (23,6)=0x02, (25,6)=0x02, … (+32) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (17,3)=0x69, (2,25)=0x04, (4,25)=0x04, (29,25)=0x04, (31,25)=0x04, (2,27)=0x04, (31,27)=0x04, (2,29)=0x04, … (+5) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (21,11)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (10,6)=0x2D, (31,8)=0x2D, (24,9)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (6,3)=0x0C, (4,4)=0x0C, (8,4)=0x0C, (5,5)=0x1A, (6,5)=0x0C, (7,5)=0x1B, (3,6)=0x0C, (5,6)=0x0C, … (+324) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (11,17)=0x0D, (12,17)=0x0D, (13,17)=0x0D, (14,17)=0x12, (15,17)=0x12, (16,17)=0x12, (17,17)=0x12, (18,17)=0x12, … (+10) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+362) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (15,14)=0x23, (9,24)=0x26, (24,24)=0x26 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- teleporters at top: (3,9)=0x29, (18,14)=0x29, (16,22)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (15,26)=0x2A, (18,26)=0x2A — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (16,5)=0x43, (26,5)=0x43, (16,7)=0x41, (26,7)=0x41 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (16,24)=0x49 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- paramecia at top: (25,12)=0x60, (29,12)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 120 — ALPHABET SOUP

- Map-backed baseline: **32×32**, time limit **0**, chips required **11**, password `FLXP`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (27,7)=0x02, (26,8)=0x02, (28,8)=0x02, (2,13)=0x02, (9,13)=0x02, (31,13)=0x02, (19,14)=0x02, (2,20)=0x02, … (+5) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (11,11)=0x18, (22,11)=0x19, (28,15)=0x67, (14,16)=0x17, (15,17)=0x19, (11,22)=0x19, (5,24)=0x65, (17,26)=0x16, … (+1); bottom: (17,19)=0x66 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (11,2)=0x68, (22,2)=0x04, (11,3)=0x04, (22,3)=0x04, (11,4)=0x04, (22,4)=0x04, (11,5)=0x04, (22,5)=0x04, … (+104); bottom: (3,16)=0x69 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (25,8)=0x10, (29,15)=0x0A, (3,16)=0x0A, (25,16)=0x0A, (28,16)=0x0A, (17,19)=0x0A, (6,29)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (15,5)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (25,3)=0x1A, (26,3)=0x0C, (27,3)=0x0C, (28,3)=0x0C, (29,3)=0x1B, (25,4)=0x0C, (29,4)=0x0C, (25,5)=0x0C, … (+12) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (15,2)=0x12, (15,3)=0x12, (15,4)=0x12, (17,4)=0x13, (18,4)=0x13, (19,4)=0x32, (19,5)=0x0D, (15,6)=0x0D, … (+5) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+412) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (13,2)=0x23, (20,2)=0x23, (13,9)=0x23, (20,9)=0x23, (6,10)=0x26 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (2,4)=0x27, (2,5)=0x27, (2,6)=0x27, (2,7)=0x27, (2,8)=0x27, (2,9)=0x27, (2,15)=0x27, (4,15)=0x2B; bottom: (9,4)=0x2B, (9,5)=0x2B, (9,6)=0x2B, (9,7)=0x2B, (9,8)=0x2B, (9,9)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (28,6)=0x24; bottom: (25,8)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- teleporters at top: (5,2)=0x29, (16,5)=0x29, (27,5)=0x29, (5,15)=0x29, (16,16)=0x29, (27,17)=0x29, (5,27)=0x29, (17,28)=0x29, … (+1) — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (5,4)=0x2A, (6,4)=0x2A, (5,5)=0x2A, (6,5)=0x2A, (5,6)=0x2A, (6,6)=0x2A, (5,7)=0x2A, (6,7)=0x2A, … (+4); bottom: (29,15)=0x2A — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (18,2)=0x40, (16,3)=0x40, (14,4)=0x40, (18,5)=0x40, (13,7)=0x40, (20,7)=0x40, (16,9)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (26,24)=0x47, (30,24)=0x47, (24,26)=0x44, (31,27)=0x46, (24,30)=0x44, (27,31)=0x45, (31,31)=0x46 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (26,2)=0x4B, (31,2)=0x4A, (24,7)=0x4B, (30,7)=0x4A — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (9,4)=0x4D, (9,5)=0x4D, (9,6)=0x4D, (9,7)=0x4D, (9,8)=0x4D, (9,9)=0x4D — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- gliders at top: (9,24)=0x51 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- teeth at top: (27,18)=0x54, (18,24)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- paramecia at top: (13,3)=0x60, (19,3)=0x60, (14,6)=0x60, (17,7)=0x60, (19,9)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 121 — PERFECT MATCH

- Map-backed baseline: **32×32**, time limit **0**, chips required **0**, password `BPYS`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (16,25)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (14,5)=0x03, (2,24)=0x03, (13,24)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (15,3)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (3,23)=0x2D, (15,27)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (15,4)=0x0C, (10,5)=0x0C, (15,5)=0x0C, (17,20)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (4,4)=0x13, (17,4)=0x0D, (5,5)=0x13, (6,5)=0x13, (12,5)=0x0D, (17,5)=0x0D, (6,8)=0x0D, (7,8)=0x14, … (+11) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+290) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (2,17)=0x23, (12,18)=0x23, (17,18)=0x23, (14,25)=0x26 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (10,7)=0x27, (12,7)=0x2B, (15,7)=0x27, (17,7)=0x2B, (10,8)=0x27, (12,8)=0x2B, (15,8)=0x27, (17,8)=0x2B, … (+36) — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (13,2)=0x24, (5,23)=0x24; bottom: (17,3)=0x31, (14,24)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- hints at top: (21,16)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- fireballs at top: (17,3)=0x46, (5,8)=0x44, (4,10)=0x44, (5,12)=0x44, (14,24)=0x46 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (9,7)=0x4B, (14,7)=0x4B, (9,8)=0x4B, (14,8)=0x4B, (9,9)=0x4B, (14,9)=0x4B, (9,10)=0x4B, (14,10)=0x4B, … (+15) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.

### Level 122 — TOTALLY FAIR

- Map-backed baseline: **32×32**, time limit **300**, chips required **6**, password `SJUM`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (29,7)=0x02, (29,11)=0x02, (2,25)=0x15, (2,26)=0x22, (3,27)=0x02, (5,27)=0x02, (7,27)=0x02, (9,27)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (9,4)=0x03, (10,4)=0x03, (11,4)=0x03, (12,4)=0x03, (13,4)=0x03, (14,4)=0x03, (4,5)=0x03, (5,5)=0x03, … (+14) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (29,9)=0x0C, (30,9)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (20,3)=0x13, (31,6)=0x0D, (30,7)=0x13, (31,7)=0x0D, (31,8)=0x0D, (31,9)=0x14, (31,10)=0x12, (30,11)=0x13, … (+2) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+236) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (16,9)=0x27, (28,9)=0x2B, (15,10)=0x2B, (13,12)=0x2B, (14,12)=0x2B, (12,14)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- gliders at top: (3,17)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.
- teeth at top: (6,6)=0x54, (14,20)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 123 — THE PRISONER

- Map-backed baseline: **32×32**, time limit **299**, chips required **3**, password `YKZE`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (28,9)=0x15, (4,14)=0x02, (27,15)=0x22, (28,24)=0x02, (10,29)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (17,10)=0x03, (15,13)=0x03, (16,13)=0x03, (17,13)=0x03, (15,14)=0x03, (15,15)=0x03, (17,15)=0x03, (15,16)=0x03, … (+4) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (20,15)=0x0F — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (15,9)=0x2D, (16,9)=0x2D, (17,9)=0x2D, (13,20)=0x2D, (19,20)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (7,13)=0x0C, (8,13)=0x0C, (9,13)=0x0C, (10,13)=0x0C, (11,13)=0x0C, (21,13)=0x0C, (22,13)=0x0C, (23,13)=0x0C, … (+12) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (15,22)=0x12, (16,22)=0x12, (17,22)=0x12 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (10,2)=0x01, (11,2)=0x01, (12,2)=0x01, (13,2)=0x01, (14,2)=0x01, (15,2)=0x01, (16,2)=0x01, (17,2)=0x01, … (+190) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (12,3)=0x25, (20,3)=0x25, (12,4)=0x25, (15,4)=0x26, (16,4)=0x26, (17,4)=0x26, (20,4)=0x25, (15,7)=0x25, … (+4) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- clone machines at top: (18,13)=0x24, (16,21)=0x24; bottom: (15,10)=0x31, (20,15)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (16,14)=0x2A, (17,14)=0x2A — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- fireballs at top: (15,10)=0x47, (14,11)=0x47 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (17,21)=0x49 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- teeth at top: (10,3)=0x54, (11,3)=0x54, (21,3)=0x54, (22,3)=0x54, (10,4)=0x54, (11,4)=0x54, (21,4)=0x54, (22,4)=0x54, … (+6) — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 124 — FIRETRAP

- Map-backed baseline: **32×32**, time limit **800**, chips required **22**, password `TASX`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (4,3)=0x02, (21,3)=0x02, (23,3)=0x02, (21,7)=0x02, (23,7)=0x02, (8,10)=0x02, (32,10)=0x15, (6,11)=0x02, … (+16) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (2,2)=0x65, (5,16)=0x64, (14,26)=0x16, (5,31)=0x17 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (18,2)=0x69, (24,2)=0x04, (5,3)=0x04, (9,3)=0x04, (13,3)=0x04, (17,3)=0x04, (24,3)=0x04, (24,4)=0x04, … (+91) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (28,3)=0x0A, (3,27)=0x0A, (16,27)=0x0A, (20,27)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (12,8)=0x2D, (3,9)=0x2D, (13,10)=0x2D, (29,13)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (2,8)=0x1A, (3,8)=0x1B, (1,9)=0x1A, (2,9)=0x1C, (18,9)=0x0C, (1,10)=0x1D, (2,10)=0x1B, (18,10)=0x1D, … (+134) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (29,5)=0x14, (29,6)=0x14, (29,7)=0x14 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+327) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (16,10)=0x23, (7,16)=0x25, (21,22)=0x23, (12,28)=0x25, (10,31)=0x26, (14,31)=0x26 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (7,2)=0x28, (11,2)=0x28, (22,5)=0x27, (30,6)=0x27, (5,7)=0x28, (9,7)=0x28, (18,8)=0x27, (28,8)=0x2B, … (+1) — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- fireballs at top: (28,2)=0x47, (13,21)=0x47, (22,22)=0x46, (6,23)=0x44, (14,24)=0x45, (2,28)=0x44 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- tanks at top: (4,4)=0x4F, (8,4)=0x4F, (12,4)=0x4F, (16,4)=0x4F — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.

### Level 125 — MIXED NUTS

- Map-backed baseline: **32×32**, time limit **0**, chips required **71**, password `MYRT`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,4)=0x02, (5,4)=0x02, (9,4)=0x02, (11,4)=0x02, (24,9)=0x22, (13,10)=0x02, (12,11)=0x02, (11,12)=0x02, … (+52); bottom: (11,27)=0x02, (15,27)=0x02, (10,28)=0x02, (12,28)=0x02, (14,28)=0x02, (16,28)=0x02, (11,29)=0x02, (13,29)=0x02, … (+5) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (3,5)=0x03, (5,5)=0x03, (9,5)=0x03, (11,5)=0x03, (12,10)=0x03, (11,11)=0x03, (10,12)=0x03, (9,13)=0x03, … (+5) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (4,7)=0x0A, (6,7)=0x0A, (8,7)=0x0A, (10,7)=0x0A, (15,11)=0x0A, (14,12)=0x0A, (13,13)=0x0A, (12,14)=0x0A, … (+18) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (14,2)=0x2D, (8,10)=0x2D, (2,16)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (21,13)=0x0C, (26,13)=0x0C, (31,13)=0x0C, (21,14)=0x0C, (26,14)=0x0C, (31,14)=0x0C, (21,15)=0x0C, (26,15)=0x0C, … (+94) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (15,2)=0x14, (16,2)=0x14, (17,2)=0x14, (18,2)=0x14, (19,2)=0x14, (20,2)=0x14, (21,2)=0x14, (22,2)=0x14, … (+140) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+389) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (27,3)=0x29, (12,21)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- bombs and thieves at top: (29,12)=0x2A, (23,13)=0x2A, (28,14)=0x2A, (24,15)=0x2A, (29,16)=0x2A, (29,23)=0x2A, (24,24)=0x2A, (28,25)=0x2A, … (+9) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (2,13)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (6,2)=0x4B, (21,30)=0x4B, (29,30)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- gliders at top: (7,5)=0x53 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 126 — BLOCK N ROLL

- Map-backed baseline: **32×32**, time limit **600**, chips required **36**, password `QRLD`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x02, (16,2)=0x02, (21,5)=0x02, (16,6)=0x02, (26,6)=0x02, (27,6)=0x02, (28,6)=0x02, (29,6)=0x02, … (+23); bottom: (24,26)=0x02, (28,26)=0x02, (22,28)=0x02, (26,28)=0x02, (30,28)=0x02, (24,30)=0x02, (28,30)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (3,2)=0x03, (5,2)=0x03, (7,2)=0x03, (9,2)=0x03, (11,2)=0x03, (13,2)=0x03, (15,2)=0x03, (4,3)=0x03, … (+84); bottom: (22,26)=0x04, (26,26)=0x04, (30,26)=0x04, (24,28)=0x04, (28,28)=0x04, (22,30)=0x04, (26,30)=0x04, (30,30)=0x04 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (4,2)=0x0A, (6,2)=0x0A, (8,2)=0x0A, (10,2)=0x0A, (12,2)=0x0A, (14,2)=0x0A, (3,3)=0x0A, (5,3)=0x0A, … (+75) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (26,2)=0x2D, (29,3)=0x2D, (26,4)=0x2D, (27,4)=0x2D, (28,4)=0x2D, (29,4)=0x2D, (30,4)=0x2D, (31,4)=0x2D, … (+7) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+399) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (29,2)=0x43, (29,5)=0x43, (28,8)=0x43, (21,28)=0x40, (31,28)=0x42 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- walkers at top: (13,12)=0x58, (18,12)=0x58, (22,12)=0x58, (14,17)=0x58, (18,17)=0x58, (22,17)=0x58, (26,17)=0x58, (17,21)=0x58, … (+3) — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 127 — SKELZIE

- Map-backed baseline: **32×32**, time limit **500**, chips required **21**, password `JMWZ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (17,4)=0x02, (16,5)=0x02, (6,6)=0x02, (27,6)=0x02, (4,15)=0x02, (13,15)=0x02, (16,15)=0x22, (19,15)=0x02, … (+15) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (16,2)=0x65, (29,4)=0x64, (4,7)=0x18, (29,7)=0x17, (4,26)=0x16, (26,28)=0x19, (4,29)=0x67, (29,29)=0x66 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+283) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (4,4)=0x23, (16,26)=0x25 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (14,2)=0x2B, (18,2)=0x2B, (15,3)=0x2B, (17,3)=0x2B, (15,5)=0x2B, (17,5)=0x2B, (14,6)=0x2B, (18,6)=0x2B, … (+31) — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- teleporters at top: (16,4)=0x29, (16,13)=0x29, (4,16)=0x29, (13,16)=0x29, (19,16)=0x29, (29,16)=0x29, (16,19)=0x29, (16,29)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- balls at top: (16,12)=0x4A, (12,16)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- blobs at top: (9,9)=0x5F, (23,9)=0x5F, (9,24)=0x5F, (23,24)=0x5F — `NEEDS CAPTURE`: Compare random movement/seed, speed schedule, blocked moves and collision.

### Level 128 — ALL FULL

- Map-backed baseline: **32×32**, time limit **400**, chips required **182**, password `FTLA`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x02, (3,2)=0x02, (4,2)=0x02, (5,2)=0x02, (6,2)=0x02, (7,2)=0x02, (8,2)=0x02, (19,2)=0x02, … (+180) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (13,9)=0x19, (13,10)=0x19, (12,13)=0x19, (12,14)=0x19, (30,16)=0x67, (19,17)=0x19, (13,21)=0x67, (2,29)=0x67, … (+2) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (14,13)=0x68, (16,14)=0x03, (16,15)=0x03, (21,16)=0x03, (13,17)=0x6B, (26,18)=0x04, (27,18)=0x04, (28,18)=0x04, … (+64) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (12,29)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (21,15)=0x2D, (12,16)=0x2D, (9,22)=0x2D, (13,23)=0x2D, (6,27)=0x2D, (10,29)=0x2D, (10,31)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (10,23)=0x0C, (10,24)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (21,18)=0x13, (31,18)=0x0D, (22,19)=0x13, (30,19)=0x0D, (15,20)=0x0D, (23,20)=0x13, (29,20)=0x0D, (16,21)=0x14, … (+35) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+401) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (5,23)=0x26, (5,24)=0x23, (10,26)=0x26, (4,29)=0x26, (17,31)=0x26 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- clone machines at top: (14,14)=0x24, (18,15)=0x24, (8,18)=0x24, (10,22)=0x24, (8,26)=0x24; bottom: (18,14)=0x31, (28,16)=0x31, (10,18)=0x31, (4,23)=0x31, (2,31)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (12,15)=0x21, (19,18)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (23,11)=0x43, (21,12)=0x43, (10,18)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (3,18)=0x47, (5,18)=0x47, (7,18)=0x47, (22,18)=0x47, (23,19)=0x47, (5,20)=0x47, (24,20)=0x47, (25,21)=0x47, … (+5) — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (12,2)=0x4B, (18,14)=0x4A, (28,16)=0x49, (17,25)=0x49, (14,27)=0x49 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- gliders at top: (24,9)=0x53 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 129 — LOBSTER TRAP

- Map-backed baseline: **32×32**, time limit **300**, chips required **6**, password `HEAN`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (29,4)=0x02, (29,5)=0x02, (29,7)=0x02, (29,8)=0x02, (29,10)=0x02, (29,11)=0x02, (28,13)=0x22, (29,13)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (26,29)=0x03, (27,29)=0x03, (26,30)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- dirt and gravel at top: (2,2)=0x0B, (3,2)=0x0B, (5,2)=0x0B, (8,2)=0x0B, (10,2)=0x0B, (11,2)=0x0B, (12,2)=0x0B, (13,2)=0x0B, … (+500) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+170) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (26,4)=0x43, (28,4)=0x42, (25,5)=0x40, (27,5)=0x41, (26,6)=0x42, (25,7)=0x40, (27,7)=0x43, (26,8)=0x41, … (+6) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- gliders at top: (26,17)=0x51, (27,18)=0x51, (25,19)=0x50, (24,20)=0x52, (27,20)=0x50, (20,21)=0x53, (23,21)=0x53, (25,21)=0x50, … (+14) — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 130 — ICE CUBE

- Map-backed baseline: **32×32**, time limit **0**, chips required **32**, password `XHIZ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (17,5)=0x02, (12,6)=0x02, (21,6)=0x02, (8,8)=0x02, (25,8)=0x02, (15,10)=0x02, (18,10)=0x02, (11,11)=0x02, … (+26) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- ice and ice corners at top: (3,3)=0x1A, (4,3)=0x1B, (5,3)=0x1A, (6,3)=0x1B, (7,3)=0x1A, (8,3)=0x1C, (9,3)=0x1A, (10,3)=0x1B, … (+740) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+118) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- bugs at top: (21,31)=0x41 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.

### Level 131 — TOTALLY UNFAIR

- Map-backed baseline: **32×32**, time limit **60**, chips required **6**, password `FIRD`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (29,7)=0x02, (29,11)=0x02, (2,25)=0x15, (2,26)=0x22, (3,27)=0x02, (5,27)=0x02, (7,27)=0x02, (9,27)=0x02 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (9,4)=0x03, (10,4)=0x03, (11,4)=0x03, (12,4)=0x03, (13,4)=0x03, (14,4)=0x03, (4,5)=0x03, (5,5)=0x03, … (+14) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (29,9)=0x0C, (30,9)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (20,3)=0x13, (31,6)=0x0D, (30,7)=0x13, (31,7)=0x0D, (31,8)=0x0D, (31,9)=0x14, (31,10)=0x12, (30,11)=0x13, … (+2) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+238) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (16,9)=0x27, (28,9)=0x2B, (15,10)=0x2B, (13,12)=0x2B, (14,12)=0x2B, (12,14)=0x2B — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- hints at top: (5,21)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- teeth at top: (6,6)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.

### Level 132 — MIX UP

- Map-backed baseline: **32×32**, time limit **999**, chips required **28**, password `ZYFA`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (20,18)=0x22, (21,18)=0x22, (22,18)=0x22, (20,19)=0x22, (21,19)=0x15, (22,19)=0x22, (20,20)=0x22, (21,20)=0x22, … (+29) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (23,4)=0x66, (12,10)=0x65, (21,13)=0x17, (28,20)=0x64, (6,23)=0x64, (5,26)=0x16, (10,30)=0x16, (15,31)=0x18 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (8,10)=0x03, (9,10)=0x03, (10,10)=0x03, (11,10)=0x03, (22,15)=0x03, (23,15)=0x03, (24,15)=0x03, (25,15)=0x03, … (+19) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (6,5)=0x0A, (12,5)=0x0A, (14,5)=0x0A, (6,6)=0x0A, (23,6)=0x0A, (6,7)=0x0A, (17,7)=0x0A, (25,7)=0x0A, … (+25) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (25,17)=0x2D, (26,17)=0x2D, (27,17)=0x2D, (28,17)=0x2D, (29,17)=0x2D, (30,17)=0x2D, (31,17)=0x2D, (25,18)=0x2D, … (+32) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (2,2)=0x1A, (3,2)=0x0C, (4,2)=0x0C, (5,2)=0x0C, (6,2)=0x0C, (7,2)=0x0C, (8,2)=0x0C, (9,2)=0x0C, … (+80) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (19,10)=0x12, (31,16)=0x12, (18,23)=0x12, (19,23)=0x12, (17,24)=0x13, (18,24)=0x12, (19,24)=0x12, (20,24)=0x14, … (+5) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+459) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (31,25)=0x23, (27,26)=0x23, (28,27)=0x23, (30,27)=0x23, (24,28)=0x25, (26,28)=0x23, (28,29)=0x23, (30,29)=0x23, … (+2) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- clone machines at top: (24,26)=0x24, (24,30)=0x24; bottom: (29,26)=0x31, (29,30)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (24,4)=0x2A, (25,4)=0x2A, (26,4)=0x2A, (27,4)=0x2A, (28,4)=0x2A, (29,4)=0x2A, (28,5)=0x2A, (29,5)=0x2A, … (+2) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (19,19)=0x42, (23,19)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (24,29)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- teeth at top: (27,21)=0x54, (8,22)=0x54 — `NEEDS CAPTURE`: Compare player tracking, tick gating, tie-breaking, collision and movement cadence.
- walkers at top: (29,26)=0x5A, (29,30)=0x58 — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 133 — BLOBDANCE

- Map-backed baseline: **32×32**, time limit **0**, chips required **30**, password `TIGG`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (6,3)=0x02, (12,3)=0x02, (20,3)=0x02, (28,3)=0x02, (4,7)=0x02, (10,7)=0x02, (30,7)=0x02, (22,8)=0x02, … (+24) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (2,2)=0x2D, (3,2)=0x2D, (4,2)=0x2D, (2,3)=0x2D, (4,3)=0x2D, (10,3)=0x2D, (14,3)=0x2D, (22,3)=0x2D, … (+75); bottom: (3,3)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+274) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- blobs at top: (22,4)=0x5C, (29,4)=0x5C, (6,6)=0x5C, (13,6)=0x5C, (20,6)=0x5C, (28,7)=0x5C, (5,11)=0x5C, (13,11)=0x5C, … (+47) — `NEEDS CAPTURE`: Compare random movement/seed, speed schedule, blocked moves and collision.

### Level 134 — PAIN

- Map-backed baseline: **32×32**, time limit **0**, chips required **7**, password `XPPH`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (4,10)=0x02, (6,10)=0x02, (18,10)=0x02, (23,10)=0x02, (28,10)=0x02, (29,10)=0x02, (30,27)=0x02, (30,30)=0x22, … (+3) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (2,13)=0x03, (3,13)=0x03, (4,13)=0x03, (5,13)=0x03, (6,13)=0x03, (6,14)=0x03, (7,14)=0x03, (8,14)=0x03, … (+170); bottom: (27,30)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (7,9)=0x0A, (15,9)=0x0A, (10,11)=0x0A, (2,15)=0x0A, (3,15)=0x0A, (4,15)=0x0A, (5,15)=0x0A, (10,15)=0x0A, … (+88) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (27,5)=0x0B, (29,5)=0x0B, (28,7)=0x0B — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+369) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- traps and tank controls at top: (9,8)=0x2B, (11,10)=0x27 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- bugs at top: (13,8)=0x40, (17,8)=0x40, (29,8)=0x40, (9,10)=0x40, (13,10)=0x40, (16,10)=0x40, (19,10)=0x40, (21,10)=0x40, … (+2) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- gliders at top: (5,22)=0x50, (27,30)=0x50 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 135 — TRUST ME

- Map-backed baseline: **32×32**, time limit **300**, chips required **0**, password `LYWO`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (1,2)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (1,1)=0x67, (32,1)=0x67, (2,2)=0x19, (7,7)=0x67, (18,7)=0x67, (5,8)=0x67, (11,10)=0x67, (14,10)=0x67, … (+28) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (23,2)=0x6B, (18,6)=0x6B, (19,8)=0x6B, (13,9)=0x03, (17,10)=0x03, (31,10)=0x03, (14,11)=0x6B, (18,17)=0x6B, … (+3) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (6,4)=0x0A, (16,4)=0x0A, (17,4)=0x0A, (18,4)=0x0A, (15,5)=0x0A, (16,5)=0x0A, (18,5)=0x0A, (16,6)=0x0A, … (+1) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (11,3)=0x2D, (13,14)=0x2D, (5,21)=0x2D, (18,23)=0x2D, (2,31)=0x0B — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (22,16)=0x1A, (23,16)=0x0C, (24,16)=0x0C, (25,16)=0x0C, (26,16)=0x0C, (27,16)=0x1B, (28,16)=0x1A, (29,16)=0x0C, … (+153) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (12,2)=0x13, (30,2)=0x14, (4,5)=0x0D, (5,5)=0x0D, (4,6)=0x0D, (5,6)=0x0D, (4,7)=0x13, (5,7)=0x0D, … (+6) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (2,1)=0x1F, (3,1)=0x1F, (4,1)=0x1F, (5,1)=0x1F, (6,1)=0x1F, (7,1)=0x1F, (8,1)=0x1F, (9,1)=0x1F, … (+520); bottom: (8,14)=0x1E — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (11,6)=0x24, (9,9)=0x24, (11,24)=0x24; bottom: (13,6)=0x31, (3,11)=0x31, (21,26)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bugs at top: (13,6)=0x41, (3,11)=0x43, (16,11)=0x40, (8,14)=0x40 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (30,7)=0x48, (24,8)=0x48, (29,8)=0x48, (4,9)=0x4B, (28,9)=0x48, (25,10)=0x48, (26,11)=0x48, (9,19)=0x4B, … (+10) — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (6,20)=0x4C — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- paramecia at top: (8,23)=0x60, (6,24)=0x62, (2,25)=0x62, (8,25)=0x60, (5,26)=0x62, (12,26)=0x60, (21,26)=0x60, (8,27)=0x62, … (+4) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 136 — DOUBLEMAZE

- Map-backed baseline: **32×32**, time limit **0**, chips required **38**, password `LUZL`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,1)=0x15, (2,2)=0x02, (8,2)=0x02, (12,2)=0x02, (20,2)=0x02, (24,2)=0x02, (1,3)=0x15, (9,3)=0x22, … (+33) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- ice and ice corners at top: (3,2)=0x0C, (5,2)=0x0C, (7,2)=0x0C, (9,2)=0x0C, (11,2)=0x0C, (13,2)=0x0C, (15,2)=0x0C, (17,2)=0x0C, … (+442) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, (9,1)=0x01, … (+448) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 137 — GOLDKEY

- Map-backed baseline: **32×32**, time limit **450**, chips required **0**, password `HPPX`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (32,2)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (29,2)=0x19, (3,4)=0x67, (31,6)=0x19, (21,7)=0x19, (21,8)=0x19, (10,9)=0x19, (11,9)=0x19, (12,9)=0x19, … (+25) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (9,3)=0x68, (20,5)=0x04, (24,6)=0x03, (25,6)=0x03, (26,6)=0x03, (27,6)=0x03, (28,6)=0x03, (30,6)=0x03, … (+113) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (6,18)=0x0A — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- dirt and gravel at top: (18,5)=0x2D, (23,7)=0x2D, (20,16)=0x2D, (30,16)=0x2D, (30,19)=0x2D, (19,24)=0x2D, (30,31)=0x2D — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- force floors at top: (9,2)=0x0D, (9,4)=0x0D, (9,5)=0x0D, (9,6)=0x0D, (12,6)=0x0D, (3,7)=0x14, (4,7)=0x14, (5,7)=0x14, … (+186) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x1F, (2,1)=0x1F, (3,1)=0x1F, (4,1)=0x1F, (5,1)=0x1F, (6,1)=0x1F, (7,1)=0x1F, (8,1)=0x1F, … (+305) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (24,7)=0x24; bottom: (14,2)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (29,6)=0x21, (3,9)=0x21 — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.
- bugs at top: (2,2)=0x43, (4,2)=0x43, (6,2)=0x43, (26,26)=0x42, (20,29)=0x42 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- fireballs at top: (14,2)=0x47 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (10,2)=0x4A, (11,4)=0x4A, (14,4)=0x4A, (13,5)=0x4A, (15,5)=0x4A, (31,7)=0x49 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- paramecia at top: (11,18)=0x63, (13,18)=0x63, (15,18)=0x63, (17,18)=0x63, (19,18)=0x63, (21,18)=0x63, (23,18)=0x63, (25,18)=0x63, … (+15) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 138 — PARTIAL POST

- Map-backed baseline: **32×32**, time limit **300**, chips required **0**, password `LUJT`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (8,5)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- blocks and block terrain at top: (15,14)=0x0A, (17,14)=0x0A, (19,14)=0x0A, (16,15)=0x0A, (18,15)=0x0A, (15,16)=0x0A, (17,16)=0x0A, (19,16)=0x0A, … (+8) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+546) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (7,5)=0x29, (14,5)=0x29, (26,5)=0x29, (3,17)=0x29, (14,17)=0x29, (26,17)=0x29, (3,28)=0x29, (14,28)=0x29, … (+1) — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- hints at top: (6,16)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.

### Level 139 — YORKHOUSE

- Map-backed baseline: **32×32**, time limit **0**, chips required **388**, password `VLHH`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,3)=0x02, (3,3)=0x02, (4,3)=0x02, (5,3)=0x02, (6,3)=0x02, (7,3)=0x02, (8,3)=0x02, (9,3)=0x02, … (+398) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (16,22)=0x0B, (17,22)=0x0B, (18,22)=0x0B, (19,22)=0x0B, (16,23)=0x0B, (17,23)=0x0B, (18,23)=0x0B, (19,23)=0x0B, … (+16) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (9,11)=0x1A, (10,11)=0x0C, (11,11)=0x1B, (9,12)=0x0C, (10,12)=0x0C, (11,12)=0x0C, (9,13)=0x0C, (10,13)=0x0C, … (+47) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+411) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- walkers at top: (13,5)=0x58, (28,6)=0x58, (3,7)=0x58, (20,9)=0x58, (14,13)=0x58, (3,15)=0x58, (30,20)=0x58, (3,25)=0x58, … (+2) — `NEEDS CAPTURE`: Compare random-choice sequence/seed, blocked moves, collision and movement cadence.

### Level 140 — ICEDEATH

- Map-backed baseline: **32×32**, time limit **300**, chips required **0**, password `SJUK`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (15,13)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (19,2)=0x03, (20,2)=0x03, (23,2)=0x03, (24,2)=0x03, (25,2)=0x03, (26,2)=0x03, (27,2)=0x03, (31,2)=0x6A, … (+123) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- ice and ice corners at top: (2,2)=0x0C, (3,2)=0x0C, (5,2)=0x0C, (6,2)=0x0C, (7,2)=0x0C, (8,2)=0x0C, (10,2)=0x0C, (11,2)=0x0C, … (+654) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+116) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 141 — UNDERGROUND

- Map-backed baseline: **32×32**, time limit **0**, chips required **15**, password `MCJE`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (30,2)=0x22, (31,2)=0x15, (4,3)=0x02, (14,5)=0x02, (24,6)=0x02, (4,8)=0x02, (14,11)=0x02, (27,11)=0x02, … (+9) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- dirt and gravel at top: (2,2)=0x0B, (3,2)=0x0B, (4,2)=0x0B, (5,2)=0x0B, (6,2)=0x0B, (7,2)=0x0B, (8,2)=0x0B, (9,2)=0x0B, … (+713) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+118) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- paramecia at top: (6,3)=0x60, (27,3)=0x60, (11,5)=0x60, (17,5)=0x60, (21,6)=0x60, (30,7)=0x60, (8,8)=0x60, (14,9)=0x60, … (+26) — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 142 — PENTAGRAM

- Map-backed baseline: **32×32**, time limit **0**, chips required **10**, password `UCRY`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (22,2)=0x02, (4,4)=0x02, (19,12)=0x02, (13,13)=0x02, (16,17)=0x22, (17,17)=0x15, (21,17)=0x02, (12,19)=0x02, … (+4) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- ice and ice corners at top: (15,9)=0x0C, (22,12)=0x0C, (9,15)=0x0C, (21,21)=0x0C, (12,22)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+997); bottom: (17,16)=0x08 — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- hints at top: (18,16)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.

### Level 143 — STRIPES?

- Map-backed baseline: **32×32**, time limit **0**, chips required **23**, password `OKOR`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (2,2)=0x02, (10,2)=0x02, (17,2)=0x02, (25,2)=0x02, (10,8)=0x02, (25,8)=0x02, (31,8)=0x02, (2,9)=0x02, … (+18) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- walls and panels at top: (1,1)=0x05, (2,1)=0x01, (3,1)=0x05, (4,1)=0x05, (5,1)=0x01, (6,1)=0x05, (7,1)=0x05, (8,1)=0x01, … (+394) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 144 — FIREFLIES

- Map-backed baseline: **32×32**, time limit **0**, chips required **30**, password `GVXQ`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (3,2)=0x02, (22,3)=0x02, (31,3)=0x02, (27,5)=0x02, (5,6)=0x02, (13,7)=0x02, (23,7)=0x02, (22,9)=0x02, … (+24) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (1,1)=0x04, (5,1)=0x04, (6,1)=0x04, (7,1)=0x04, (15,1)=0x04, (16,1)=0x04, (17,1)=0x04, (18,1)=0x04, … (+340) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- dirt and gravel at top: (5,2)=0x2D, (6,2)=0x2D, (7,2)=0x2D, (24,3)=0x2D, (26,3)=0x2D, (27,3)=0x2D, (28,3)=0x2D, (5,4)=0x2D, … (+188) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- walls and panels at top: (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (8,1)=0x01, (9,1)=0x01, (10,1)=0x01, (11,1)=0x01, (12,1)=0x01, … (+191) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- fireballs at top: (11,2)=0x47, (31,11)=0x46, (14,12)=0x45, (21,14)=0x44, (12,19)=0x46, (19,21)=0x47, (2,22)=0x44, (22,31)=0x45 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.

### Level 145 — Thanks to...

- Map-backed baseline: **32×32**, time limit **0**, chips required **0**, password `TONY`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (32,32)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (25,18)=0x69, (1,20)=0x04, (2,20)=0x04, (3,20)=0x04, (4,20)=0x04, (5,20)=0x04, (6,20)=0x04, (7,20)=0x04, … (+121) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- dirt and gravel at top: (1,13)=0x2D, (2,13)=0x2D, (3,13)=0x2D, (4,13)=0x2D, (5,13)=0x2D, (6,13)=0x2D, (7,13)=0x2D, (8,13)=0x2D, … (+215) — `NEEDS CAPTURE`: Compare removal/retention, underlying tile restoration, movement eligibility and repeated entry.
- ice and ice corners at top: (2,1)=0x0C, (3,1)=0x0C, (4,1)=0x0C, (5,1)=0x0C, (6,1)=0x0C, (7,1)=0x0C, (8,1)=0x0C, (9,1)=0x0C, … (+139); bottom: (1,26)=0x0C, (6,27)=0x0C, (11,28)=0x0C, (20,29)=0x0C, (28,30)=0x0C — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (9,20)=0x32, (10,20)=0x32, (11,20)=0x32, (13,20)=0x32, (14,20)=0x32, (19,20)=0x32, (21,20)=0x32, (9,21)=0x32, … (+86) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,2)=0x01, (2,2)=0x01, (3,2)=0x01, (5,2)=0x01, (7,2)=0x01, (10,2)=0x01, (13,2)=0x01, (16,2)=0x01, … (+68) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (1,8)=0x23, (2,8)=0x23, (3,8)=0x23, (4,8)=0x23, (5,8)=0x23, (6,8)=0x25, (7,8)=0x25, (8,8)=0x23, … (+149) — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (7,14)=0x28 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- teleporters at top: (32,1)=0x29, (1,7)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- hints at top: (31,32)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.
- bugs at top: (12,14)=0x41, (11,15)=0x42, (13,15)=0x40, (11,17)=0x42, (13,17)=0x40, (12,18)=0x43 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision/death and movement cadence.
- balls at top: (23,15)=0x4A, (21,18)=0x48 — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (15,14)=0x4E — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- paramecia at top: (7,18)=0x60 — `NEEDS CAPTURE`: Compare turn order, preferred turn, collision and movement cadence.

### Level 146 — CAKE WALK

- Map-backed baseline: **32×32**, time limit **999**, chips required **13**, password `JHEN`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (10,3)=0x02, (12,3)=0x02, (9,4)=0x02, (11,4)=0x02, (13,4)=0x02, (2,18)=0x02, (4,18)=0x02, (8,18)=0x02, … (+14) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (17,2)=0x67, (17,3)=0x18, (2,4)=0x18, (3,4)=0x18, (27,8)=0x66, (17,16)=0x19, (17,17)=0x16, (6,18)=0x64, … (+14) — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (26,3)=0x04, (27,3)=0x04, (28,3)=0x04, (29,3)=0x04, (30,3)=0x04, (31,3)=0x04, (2,13)=0x03, (3,13)=0x03, … (+241); bottom: (27,22)=0x03, (31,22)=0x03 — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (2,2)=0x0A, (3,2)=0x0A, (16,2)=0x0A, (2,3)=0x0A, (3,3)=0x0A, (9,3)=0x0A, (16,3)=0x0A, (3,5)=0x0A, … (+47) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (16,13)=0x1B, (18,13)=0x1A, (16,14)=0x0C, (18,14)=0x0C, (16,15)=0x0C, (18,15)=0x0C, (16,16)=0x0C, (18,16)=0x1D, … (+9) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- force floors at top: (18,17)=0x13, (16,18)=0x14, (18,18)=0x13, (16,19)=0x14, (18,19)=0x13, (16,20)=0x14, (18,20)=0x13, (24,20)=0x32, … (+99) — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+267) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- toggle controls at top: (21,2)=0x26, (22,2)=0x25, (26,2)=0x23 — `NEEDS CAPTURE`: Compare button activation order, wall/floor swap scope, repeated activation and interaction with creatures/blocks.
- traps and tank controls at top: (20,3)=0x28 — `NEEDS CAPTURE`: Compare linkage, button timing, trapped-creature release/hold behavior and creature turn on control activation.
- clone machines at top: (30,2)=0x24; bottom: (19,2)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- fireballs at top: (19,2)=0x47, (9,20)=0x46, (9,22)=0x46, (9,24)=0x46, (9,26)=0x46 — `NEEDS CAPTURE`: Compare turn order, preferred turn, hazards/boots and movement cadence.
- balls at top: (27,2)=0x4B, (16,4)=0x4A, (18,4)=0x4A, (17,10)=0x4B — `NEEDS CAPTURE`: Compare turn order, reversal rules, collision and movement cadence.
- tanks at top: (6,7)=0x4D — `NEEDS CAPTURE`: Compare orientation, control-button response, collision and movement cadence.
- gliders at top: (27,22)=0x50, (31,22)=0x52 — `NEEDS CAPTURE`: Compare turn order, water/fire terrain, collision and movement cadence.

### Level 147 — FORCE FIELD

- Map-backed baseline: **32×32**, time limit **0**, chips required **24**, password `COZA`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (1,2)=0x15, (1,3)=0x15, (1,4)=0x15, (5,4)=0x02, (13,4)=0x02, (21,4)=0x02, (29,4)=0x02, (1,5)=0x15, … (+46) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- water, fire and boots at top: (17,16)=0x6B — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- force floors at top: (2,2)=0x13, (3,2)=0x0D, (4,2)=0x14, (5,2)=0x14, (6,2)=0x14, (7,2)=0x14, (8,2)=0x14, (9,2)=0x14, … (+636); bottom: (3,16)=0x13 — `NEEDS CAPTURE`: Compare directional/random force, RNG seed, forced movement cadence, manual input precedence and wall collision.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+316) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.

### Level 148 — MIND BLOCK

- Map-backed baseline: **32×32**, time limit **0**, chips required **12**, password `RGSK`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at top: (8,8)=0x02, (10,8)=0x02, (18,8)=0x02, (25,8)=0x22, (26,8)=0x15, (25,10)=0x02, (20,13)=0x02, (22,13)=0x02, … (+6) — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- blocks and block terrain at top: (17,18)=0x0E — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- walls and panels at top: (7,7)=0x01, (8,7)=0x01, (9,7)=0x01, (10,7)=0x01, (11,7)=0x01, (12,7)=0x01, (13,7)=0x01, (14,7)=0x01, … (+95) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- clone machines at top: (16,15)=0x24; bottom: (17,18)=0x31 — `NEEDS CAPTURE`: Compare link mapping, activation source, clone direction/state, creation timing, collisions and repeated activation.
- bombs and thieves at top: (1,1)=0x2A, (2,1)=0x2A, (3,1)=0x2A, (4,1)=0x2A, (5,1)=0x2A, (6,1)=0x2A, (7,1)=0x2A, (8,1)=0x2A, … (+844) — `NEEDS CAPTURE`: Compare destruction/consumption, inventory stripping, persistence after leaving/re-entering, death and sound.

### Level 149 — SPECIAL

- Map-backed baseline: **32×32**, time limit **999**, chips required **0**, password `DIGW`.
- Input and tick — `NEEDS CAPTURE`: record key-down/key-up independently; compare a short press, held key, repeated OS keydown, simultaneous direction keys, direction change while moving, blocked direction, and input during forced motion. Compare accepted movement tick, movement interval, initial tick phase, timer decrement boundary, pause/resume, and restart/level-load tick reset behavior. Verify one gameplay tick per intended scheduler step independent of render rate.
- State/order — `NEEDS CAPTURE`: compare before/after player coordinates and facing, top/bottom tile IDs, inventory, chip count, creature positions/directions, RNG state or seed, outcome, and emitted sound for each action. Include action followed by one tick and by the next creature tick to expose ordering differences.
- chips, sockets, exit at bottom: (12,2)=0x15 — `NEEDS CAPTURE`: Compare pickup, requirement decrement, socket/exit gating, completion and terminal input.
- colored keys and doors at top: (9,5)=0x17, (6,8)=0x19, (11,8)=0x67, (12,8)=0x18, (9,11)=0x16, (23,21)=0x66, (20,24)=0x64, (23,27)=0x65 — `NEEDS CAPTURE`: Capture each key/door color; compare pickup, inventory, door consumption, sound and same-tick input.
- water, fire and boots at top: (22,5)=0x03, (24,5)=0x03, (5,6)=0x03, (22,6)=0x03, (24,6)=0x03, (5,7)=0x03, (13,7)=0x03, (14,7)=0x03, … (+100); bottom: (15,2)=0x04, (4,3)=0x04, (7,3)=0x04, (30,4)=0x04, (20,5)=0x04, (18,6)=0x04, (27,6)=0x04, (16,10)=0x04, … (+21) — `NEEDS CAPTURE`: Compare hazard entry with/without each relevant boot, death/survival, block displacement and resulting water tile.
- blocks and block terrain at top: (12,2)=0x0A, (15,2)=0x0A, (19,2)=0x0A, (4,3)=0x0A, (7,3)=0x0A, (26,3)=0x0A, (2,4)=0x0A, (28,4)=0x0A, … (+47) — `NEEDS CAPTURE`: Compare push eligibility, direction, block movement/tick scheduling, water/fire conversion and player follow-through.
- ice and ice corners at top: (9,3)=0x1A, (10,3)=0x0C, (11,3)=0x0C, (12,3)=0x0C, (13,3)=0x0C, (14,3)=0x0C, (15,3)=0x0C, (16,3)=0x0C, … (+130) — `NEEDS CAPTURE`: Record forced direction, corner routing, input buffering, blocked turn and exact number/order of forced steps.
- walls and panels at top: (1,1)=0x01, (2,1)=0x01, (3,1)=0x01, (4,1)=0x01, (5,1)=0x01, (6,1)=0x01, (7,1)=0x01, (8,1)=0x01, … (+239) — `NEEDS CAPTURE`: Compare blocked entry, hidden/fake reveal, one-way panel direction, popup/panel transition and blocked-move sound.
- teleporters at top: (23,24)=0x29, (4,29)=0x29 — `NEEDS CAPTURE`: Compare destination scan order, blocked exits, occupied exits, facing, input timing and teleport sound.
- hints at bottom: (2,4)=0x2F, (28,4)=0x2F, (26,30)=0x2F — `NEEDS CAPTURE`: Compare activation key/click, hint text, pause/tick behavior while the hint is shown, and dismissal input.

## Existing evidence to reuse

- `tests/fixtures/README.md` documents eight native-winning routes; Level 2 has a 53-move original-played route with captured key-down timestamps and a recorded 18-tick initial phase. That phase is a fixture parameter, not a universal original startup value.
- `tests/fixtures/level2-key-capture.json` and `scripts/replay_recorded_key_events.py` provide one reproducible physical-key timing case. Extend this method to all levels and input edge cases; do not treat a route win as proof of complete rules parity.
- `tests/fixtures/native-known-routes.json` supplies routes for selected levels. They are useful scenario seeds, but only the Level 2 route is documented as played in the original executable; native route success alone is not an original/native comparison.
- Consult `T:/ChipsChallenge/CHIPS.EXE-master/CHIPS.EXE-master/{movement.asm,logic.asm,constants.asm,seg2.asm}` for authoritative scheduling, movement eligibility, key routing and tile IDs. Use the original application for observations where the disassembly is ambiguous.

## Completion rule

Do not report the campaign as parity-complete while any campaign mechanic has only a code-reading argument or native-only replay. Every observed mismatch must point to an original capture, native replay, exact input/tick trace, differing state/event, a focused regression scenario, and the implementation location to adjust. Preserve uncertain and untested behavior as `NEEDS CAPTURE`.
