# Architecture

The game is a native C++20 application built on SDL3. The simulation is independent of rendering and advances in fixed ticks, which keeps game rules deterministic and makes scenarios replayable.

## Main components

- **Level data** — `src/levelpack.cpp` reads `data/CHIPS.DAT`, preserves optional fields, and validates the 149-level campaign.
- **Game engine** — `src/engine.cpp` handles movement, terrain, creatures, buttons, linked devices, timers, and game outcomes without SDL dependencies.
- **Replay and tests** — `src/replay.cpp` runs deterministic input sequences. CTest covers the engine, campaign data, profile storage, audio helpers, and UI-independent rendering logic.
- **Application and renderer** — `src/main.cpp` connects SDL input and audio to the game; `src/renderer.cpp` draws the board and interface.
- **Platform support** — small modules isolate file replacement, command-line handling, and clock behavior. SDL supplies windowing and device input on desktop platforms.
- **Assets** — `assets/` contains runtime artwork and audio with provenance and hashes in `assets/ASSET_MANIFEST.json`.

## Mechanics parity

The engine covers all nine creature families and the original two-layer map format. Tests compare decoded campaign data and exercise movement and interaction cases against the supplied behavioral references. Block slip-queue ordering, retained entries, and deletion/cascade behavior have focused regression coverage. Unusual combinations of multiple mechanics still need review; passing tests do not establish complete parity.

See [`docs/mechanics-parity.md`](docs/mechanics-parity.md) for the evidence process and per-level coverage, and [`docs/implementation-history.md`](docs/implementation-history.md) for detailed historical findings.

## Continuous integration

The native workflow builds Windows, macOS, and Linux targets and runs CTest. The WebAssembly workflow builds the browser package and opens it in Chromium, Firefox, and WebKit for smoke checks.