# Chips Challenge Native

A fresh C++20/SDL3 desktop rewrite. It does not launch or depend on the legacy executable or a compatibility layer.

The continuing implementation prompt, phased work plan, and acceptance evidence are in [`prompt.txt`](prompt.txt).

## Current status

Current verification (2026-11-10): the Windows x64 Release build and portable package pass all 15 CTest checks. The scoreboard shows each level's original hint text when Chip stands on a Hint tile; campaign validation confirms all 21 hint strings exactly match their CHIPS.DAT field-7 bytes and all matching Hint tiles are present. The check suite also covers the decoded campaign, Level 2 timing, the canonical Dirt-over-Floor bridge route, Level 3 boot positions, known route replays, and source-derived creature/movement cases. Full mechanics parity remains under source-to-engine review; unresolved behavior is tracked in `ARCHITECTURE.md`.

Teleports now snap Chip and creature sprites directly to the destination, even when the portal exit is adjacent and could be mistaken for ordinary one-cell movement. This only affects rendering; simulation positions and timing remain source-driven.

The application presents win, death, and timeout states in a result card with the engine message. Enter advances after a win or retries after a loss; on the final campaign level, confirmation returns to the title menu. `R` remains an alternate restart control. The campaign selector previews the selected level from its decoded lower and upper map layers. At startup and after controller hot-plug events, the first available SDL gamepad is opened; if it is unplugged, the app falls back to another connected gamepad.

The title menu provides separate **Continue Level** and **New Game (Level 1)** actions. New Game is available by mouse, keyboard/gamepad menu navigation, or F2; it starts the first campaign level without clearing saved progress.

Menus, prompts, help, and result dialogs use bundled Noto Sans rendered with antialiasing. Pixel-art sprites and the digital scoreboard remain pixel-based.

The current Windows x64 V1.0.1 package is `dist/chips-native-windows-x64-Release.zip`, built with `scripts/build-windows.ps1`. It includes the game, runtime libraries, campaign data, art assets, Noto Sans, and the offline HTML help manual. SHA-256: `26707994E3F18BC6F112DB213F230471BB47930D9C954C27B4D796550AC4EA39`. See [`CHANGELOG.md`](CHANGELOG.md) for release changes. macOS/Linux builds remain unverified on this host.

An active level pauses automatically when the window loses focus or is minimized. Explicit pause works from the keyboard, gamepad, or menu. Every pause discards movement waiting in the input buffer and clears controller repeat state; a stick or D-pad direction held through the pause is ignored until released. Play resumes only when the player chooses Resume or presses the pause control.

The Help menu and F1 open a bundled offline HTML manual with the original WinHelp topics and illustrations, searchable contents, and links between topics. It requires no legacy Windows Help viewer. Clicking a board tile sets a destination for classic axis-prioritized mouse movement; a teal marker shows the selected destination, and keyboard/gamepad movement cancels it. The password prompt can be canceled with Escape.

The Level menu includes **Open Level Pack...** for selecting a compatible `CHIPS.DAT` through the native file picker. A pack is decoded before it replaces the current game; invalid packs leave the active campaign intact. Each pack keeps its own progress profile keyed to the pack contents. This action is also available from the command line by passing the pack path as the first argument.


## Recent implementation updates

Creature teleporter exit selection now mirrors the original `EnterTeleport` argument path: Fire exits are accepted during routing even for Bugs and Walkers, then those creatures stop on the portal when their next slip step is blocked by Fire. Both ordinary and already-slipping movement are covered. The Teeth pursuit fallback also matches the source's final facing reset when both axes are blocked. The fresh Windows x64 Release package passed campaign/art validation and all 13 CTest checks. ZIP SHA-256: `D386FE3B6B90555BC7F85BDF8EC4CAD4F1EE86147F701D103130B58429D0B89F`.

Result cards use the supplied original art: `CHIPEND.bmp` on wins and the atlas splash, burned-Chip, or skeleton pose for deaths. Timeouts show the message without a death illustration, matching the original timeout path. Messages stay in a separate text column beside any illustration.

The title menu, gameplay, and scoreboard use the same smooth high-resolution recreation of the original green circuit-board background. The menu version is dimmed behind the slate menu card; both views crop to fill the window without stretching. The original low-resolution art remains unchanged as the source/reference image.

Added an opt-in F8 simulation inspector for tick/RNG/outcome/death cause, player position and tile layers, inventory, slide state, creature positions, and slip-queue entries. It freezes simulation while open and ignores board input; F8/Escape or controller Back closes it. The earlier inspector layout was visually reviewed at 1024x688, where the text fit and the timer stayed stable; the added death-cause label has been compiled but not visually reviewed. Tests were not run. The portable ZIP is refreshed from this build.

The campaign selector and password-entry dialog were visually reviewed in the Windows Release build at 1024x688. The selector fits its campaign list and selected-map preview, and the password card keeps its entry prompt and submit/cancel guidance inside the card. No clipping or overlap was visible. No code or package change was needed for this review.

Normal key and boot pickups now check the lower tile with the original `ChipCanEnterTile` entry rules before collecting the visible item. This prevents an item above a blocking tile from being collected through it. The Windows x64 Release target built successfully; tests and constructed layered-pickup scenarios were not run. The portable ZIP was refreshed from that build. ZIP SHA-256: `2AF31B91AEADBB620ED09239494B8852DA4C85284F939A25FA70C85EA67D4EED`.

The board adds restrained pixel-aligned glints to water, fire, teleporter, chip, key, and boot sprites, using the original atlas and cleaned item art underneath. The accents animate independently of simulation timing and are disabled by Reduced Motion. A Windows Release build succeeded, and the active Lesson 2 board was visually inspected at 1024x688. The portable ZIP was refreshed from that build; campaign validators and tests were not run. ZIP SHA-256: `3E751EA0C7E355AB48C3FFA6492130ACFC6631B4CB551B55FCF4FA9B69ABDF25`.

`SlipLoop` now attempts movement for every block with an active slipper entry, even if its underlay changes to ordinary terrain. A blocked attempt on ordinary terrain keeps the block's current heading and requeues it, matching the disassembled `SlideMovement` path. The Windows package build and campaign/art validators passed after this source-based correction; CTest was not run in that pass.

Creature `SlipLoop` now also attempts queued movement after the underlay changes, and retains blocked ordinary-terrain slippers by recreating them at the queue tail. This follows the `MoveMonster` and `SlideMovement` path in the disassembly. The Windows x64 application builds with this correction; the test suite was not run.

The asset review sheet at `docs/art/tile-index.png` labels the 112 gameplay sprites by tile ID. Regenerate it after importing the source atlas with `python scripts/export_art_reference.py`. The source artwork remains the basis for future cleanup variants; see `assets/ART.md`.

Chip's four land poses use the exact original `DrawTile` sprite/mask extraction; swimming uses the original opaque water/flipper atlas tiles. The original transparent sheet remains packaged as a fallback for all sprite IDs.

Board pickups and inventory use cleaned key/boot art indexed to original IDs `0x64`–`0x6B`, with exact original `DrawTile` frames as fallback. Ice corners and force floors (`0x1A`–`0x1D`) continue to use original atlas terrain artwork.

Creature overlays use a cleaned 6x6 sheet indexed to original IDs `0x40`–`0x63`; the imported original image/mask sheet remains available as fallback. The asset manifest records both cleaned-sheet hashes and original-art provenance.

The game now uses the supplied original WAV effects and CHIP01/CHIP02 MIDI tracks. Windows plays MIDI with the Windows Multimedia sequencer, and macOS uses AudioToolbox's General MIDI software synthesizer; SDL3 handles WAV effects on both platforms. Linux currently has effects playback only and needs a native MIDI backend. Settings provide independent persisted mute and volume controls for effects and music.

Switch, door, and socket audio now uses an explicit per-tick engine event, so clone and tank buttons sound like switches even when their status messages use different wording or are replaced by another action in the same tick.

Pickup cues are driven by the successful tile action rather than inventory deltas, so a thief removing boots cannot play the boot-acquisition sound.

The UI uses bundled Noto Sans with antialiased rendering for menus, help, settings, dialogs, and result labels. The original pixel artwork and digital scoreboard remain unchanged.

Settings now include mouse-clickable volume and display-scale sliders, a fullscreen/windowed toggle, and the reduced-motion option. Click the sound-effects row to toggle mute; F11 also toggles fullscreen. Keyboard shortcuts remain available, and the selected values persist in the user profile.

Pause and level-result overlays now present clickable Resume, Restart, Controls, Next Level, Replay, Retry, and Menu actions. Their button geometry is shared with mouse hit testing so the drawn and clickable areas stay aligned as the window resizes. Hovering with the mouse highlights the action under the pointer. Arrow keys or WASD and the gamepad D-pad/stick move the highlighted action; Enter/Space or the gamepad confirm button activates it. The final campaign result initially highlights Menu, preserving the previous confirmation behavior, while the other actions remain available.

The paged campaign selector now draws clickable previous/next buttons when a pack has more than 156 levels, so large custom packs can be browsed with a mouse as well as keyboard or gamepad.

The title, settings, and campaign overlays use dark slate cards with teal borders and focus rows while retaining the original gameplay atlas and scoreboard. The title menu caption pairs the supplied game icon with the CHIP'S CHALLENGE title; the Windows executable embeds multi-resolution sizes of the same icon. Manual inspection at the default 1024x688 window confirmed that the title instruction and campaign selector heading fit.

The gameplay status panel and top menu now use the same slate-and-teal visual language. The original gameplay atlas, digital counters, and cleaned key/boot sprites remain in use; inventory labels and HUD help/status text have more legible sizing and spacing. Manual inspection of the native 1024x688 window confirmed the inventory labels and three-line level message remain within the panel.

The Best Times overlay now pages through the full set of recorded timed-level results instead of stopping after the first nine. Page Up/Down or Left/Right moves between pages, gamepad shoulder buttons work as well, and the overlay consumes gameplay input until it is closed with Escape or the gamepad confirm/back button. The left-stick direction uses axis hysteresis so diagonal holds do not jitter between two directions.

Completing the final level shows a distinct Campaign Complete result. Its actions replay the last level, open campaign selection, or return to the title menu; keyboard/gamepad confirmation also returns to the menu instead of wrapping straight back to Level 1.

## Build on Windows

Use Visual Studio 2022 C++ Build Tools or an x64 MinGW-w64 toolchain, Git, and CMake 3.24 or newer. MSVC builds select the static C/C++ runtime so the portable ZIP does not need a separately installed Visual C++ redistributable. Python 3.10+ enables the independent decoded-campaign cross-check in CTest. CMake fetches SDL3 from its official repository at the immutable 3.4.18 release commit `829a65d769d935c4852f8159e964312c0957260a`; the full Git checkout is used because CMake's shallow-fetch mode only accepts branch or tag names.

For the default Visual Studio 2022 x64 Release build, run `./scripts/build-windows.ps1`. Pass `-Launch` to start the resulting native app, or set `-BuildDirectory`, `-Generator`, `-Architecture`, and `-Configuration` to select another build location or configuration. Builds default to four parallel workers; change this with `-BuildParallelism` if the machine needs a different limit. If CMake is not on `PATH`, the script checks the selected build's CMake cache and common Windows CMake install locations, then verifies version 3.24 or newer before configuring. The script also supports single-configuration generators such as `MinGW Makefiles`; select an x64 compiler/toolchain when using those.

Add `-Package` to create a portable Windows x64 ZIP under `dist` (or choose another project-relative destination with `-PackageDirectory`). Before archiving, the script verifies that staged `CHIPS.DAT` has the same SHA-256 as the repository campaign, then validates the staged 149-level campaign, original tile sheet, scoreboard digits, app icon, and asset manifest, including recorded source and converted-image hashes. The archive contains the native executable, required runtime DLLs, campaign data, converted original art assets, provenance manifest, README, and SDL3 third-party license notice. Example: `./scripts/build-windows.ps1 -BuildDirectory build-mingw -Generator "MinGW Makefiles" -Package`.

Visual Studio:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
.\build\Release\chips-native.exe
.\build\Release\chips-native.exe --validate .\build\Release\data\CHIPS.DAT
```

MinGW-w64 (example paths for MSYS2 UCRT64):

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles" `
  -DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/mingw32-make.exe `
  -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe `
  -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe
cmake --build build-mingw --parallel
.\build-mingw\chips-native.exe
```

Run the engine regressions and full-campaign parse/initialization check with `ctest --test-dir build-mingw --output-on-failure` (or replace `build-mingw` with `build` for the Visual Studio configuration). A clean MinGW directory was configured, built, and tested on Windows using the documented pinned SDL3 dependency; all twelve CTest checks passed. The campaign check runs a fixed seeded replay twice on every level, compares full-state digests, checks runtime references, then runs three additional seeded movement/restart scenarios per level. The latest Windows run covered 114,432 stress inputs, 286,055 requested ticks, and 5,913 restarts. These checks exercise deterministic state integrity across the campaign; they do not establish that every level can be completed or that every classic edge case is faithful. When Python 3.10+ is available, CTest compares every decoded campaign field between the bundled independent reference parser and the native decoder.

Run that comparison directly and save its machine-readable report with:

```powershell
python .\scripts\compare_campaign.py `
  --native .\build-mingw\chips-levelpack-dump.exe `
  --pack .\data\CHIPS.DAT `
  --report .\build-mingw\campaign-comparison.json
```

The comparison includes each level's number, timing and chip metadata, map type, title, hint, both 1024-tile layers, and every optional field's raw bytes, including unknown field IDs. The native target and focused synthetic engine checks were built with MinGW-w64 in this workspace. When invoking MSYS2 tools from PowerShell, include `C:\msys64\ucrt64\bin` and `C:\msys64\usr\bin` in `PATH` so the compiler subprocesses can load their runtime DLLs.

The exact campaign pack is bundled at `data/CHIPS.DAT`; CMake copies it, SDL3, the original-art PNGs, and the MinGW runtime DLLs beside the executable, so the project does not need sibling reference folders to run. To open another compatible pack, pass its path as the first argument. `--validate` checks that a pack parses, initializes every level, and contains the expected 149 sequential campaign levels; `--validate-atlas <png>` checks the converted original tile sheet and direct tile-ID mapping, and `--validate-digits <png>` checks the original scoreboard digit strip.

The separate `chips-replay` executable runs engine replays without opening SDL or a window: `chips-replay <CHIPS.DAT> <replay.txt>`. A script declares `level`, `seed`, and optional `initial_ticks` values, then lists `move N|E|S|W` (queue a direction for a simulation tick), `press N|E|S|W` (apply an immediate physical input like the original key-down path), `target <cell>` (zero-based board cell 0–1023), `advance <ticks>`, and `settle <maxTicks>` operations. `settle` advances the deterministic scheduler until the current voluntary move/forced slide has finished and Chip can accept another input, or errors at the tick limit. It prints one JSON object per command, including the stable state digest, tick, player position and visible/floor/top/bottom tile IDs, inventory, live creature positions, remaining chips/time, outcome, and typed death cause. `tools/example-replay.txt` is a runnable example; this interface is intended for exact bug reports and behavior comparisons.

For development, `scripts/validate_known_routes.py` feeds the solved routes from the neighboring independent analysis project through `chips-replay` and writes a machine-readable result report. Example from the repository root:

```powershell
python chips-native/scripts/validate_known_routes.py `
  --runner chips-native/build-menu-ui/chips-replay.exe `
  --pack chips-native/data/CHIPS.DAT `
  --routes cc-speedrun/results/known-routes.json `
  --report chips-native/build-menu-ui/known-route-validation.json
```

This is a diagnostic cross-check, not an authoritative completion suite: the neighboring solver has its own partial rules model, routes may be based on real-time input timing, and this replay spaces native inputs two simulation ticks apart. After correcting block-to-water conversion, the retained `known-routes.json` cache completes 5 of 9 routes at that interval: Levels 1, 2, 6, 10, and 75. Level 2 now reaches a native win in 53 moves. The latest bounded search has also produced a different Level 3 route that completes natively. The native CTest route fixture includes seven verified routes for Levels 1, 2, 3, 6, 8, 10, and 75. Failures identify concrete replay scenarios for investigation; they do not by themselves distinguish a native rules defect from a solver-model or timing difference. The tool is optional and is not needed to build or run the native game.

Progress is stored in SDL's per-user preference directory as `campaign.ccprofile`. An alternate pack uses `pack-<FNV-1a-of-pack-bytes>.ccprofile`, keeping its progress separate. Profile version 8 stores a variable number of level-completion flags and best times; versions 1-7 migrate from earlier layouts. The level selector displays 156 entries per page, with Page Up/Page Down, gamepad shoulder buttons, on-screen arrows, or the mouse wheel changing pages. Its selected-level preview renders the exact decoded 32x32 two-layer map and shows the chip requirement, time limit, and recorded best time. Levels beyond the saved campaign progression are marked as password-required; selecting one opens a targeted password prompt, and a valid password records access. The standalone password entry remains available, and custom levels without password metadata can be selected directly. The Best Times overlay also shows clickable arrows when multiple pages of completed times exist; keyboard, gamepad, and mouse-wheel paging are supported. The profile also stores the last selected level, highest unlocked level, independent effects/music volume and mute, movement scancodes, presentation scale, reduced-motion preference, fullscreen mode, and the last windowed size. Reduced Motion snaps moving sprites directly to their simulated tile positions without changing game timing. Writes replace the previous profile atomically. Missing profiles start fresh; invalid or unsupported profiles are ignored with a visible status message and SDL log entry.

### macOS (native development build)

Install Apple's Command Line Tools (`xcode-select --install`) and CMake 3.24+, Git, and Python 3.10+ (the last is optional for the independent campaign cross-check):

```sh
brew install cmake git python
cmake -S . -B build-macos -DCMAKE_BUILD_TYPE=Release
cmake --build build-macos --parallel
ctest --test-dir build-macos --output-on-failure
```

Build for the machine's native architecture. A universal Intel/Apple Silicon binary, `.app` bundle, signing, and notarization are not configured yet.

### Linux (Ubuntu 24.04 or compatible Debian)

Install a C++20 compiler, CMake 3.24+, Git, Python 3.10+, `pkg-config`, and SDL3's desktop backend development headers. The command below uses Ubuntu 24.04 packages; on another Debian-based system, confirm its CMake package is at least 3.24. SDL 3.4.18's documented Ubuntu set for X11/Wayland and common audio/video backends is:

```sh
sudo apt-get update
sudo apt-get install build-essential git pkg-config cmake python3 \
  libasound2-dev libpulse-dev libx11-dev libxext-dev libxrandr-dev libxcursor-dev \
  libxfixes-dev libxi-dev libxss-dev libxtst-dev libxkbcommon-dev libdrm-dev \
  libgbm-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libdbus-1-dev \
  libibus-1.0-dev libudev-dev libusb-1.0-0-dev libpipewire-0.3-dev \
  libwayland-dev libdecor-0-dev
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux --parallel
ctest --test-dir build-linux --output-on-failure
```

SDL can build with fewer dependencies, but missing headers disable native backends and can leave the app without a usable desktop video or audio driver. On macOS and Linux, CMake now has a relocatable install layout that places the executable, SDL3 shared library, campaign data, and assets together. After building, stage it with `cmake --install build-linux --prefix ./stage-linux`, then launch `./stage-linux/chips-native`; use the corresponding build directory on macOS. The native macOS/Linux CI jobs also stage this layout after compiling. This is intended for portable unpacked builds and does not yet add a platform installer, desktop entry, or application bundle. The existing Windows ZIP packaging remains unchanged.

The `Native builds` GitHub Actions workflow configures, builds, and runs CTest on Windows x64, macOS, and Linux for pushes, pull requests, and manual dispatch. Its Linux job installs SDL3's X11/Wayland, gamepad/HID, and common audio/video backend headers before CMake configures SDL. The Windows job launches the built executable's validation commands against the bundled campaign and art; the macOS/Linux jobs stage and launch the installed executable to validate the same resources in the relocatable layout. Treat macOS/Linux as verified only after their corresponding workflow jobs pass; a checked-in workflow alone is not build evidence.

## Architecture direction

- `src/levelpack.hpp`, `src/levelpack.cpp`: bounded binary level-pack reader and raw optional-field preservation.
- `src/engine.hpp`, `src/engine.cpp`: SDL-independent core state, deterministic fixed-tick simulation, tile interactions, and nine creature families.
- `src/replay.hpp`, `src/replay.cpp`: deterministic input scripts and compact state snapshots for repeatable engine scenarios.
- `src/password.hpp`, `src/password.cpp`: shared campaign-password normalization and lookup used by the menu and checked against every bundled level.
- `src/audio.hpp`, `src/audio.cpp`: optional SDL3 WAV playback with original effects and a Windows-native MIDI backend.
- `src/app_state.hpp`: shared application and presentation state.
- `src/platform_command_line.hpp`, `src/platform_command_line.cpp`: owned UTF-8 command arguments, using the native wide command line on Windows.
- `src/platform_clock.hpp`, `src/platform_clock.cpp`: session RNG seeding, matching Windows uptime milliseconds and using a monotonic fallback on macOS/Linux.
- `src/main.cpp`: SDL3 window/event/input loop, title menu, campaign selection, imported campaign display, pause, settings, and controls.
- `src/renderer.hpp`, `src/renderer.cpp`: SDL board/HUD/overlay renderer, original tile-sheet lookup, output-size-aware layout, and reduced-motion presentation option.
- `assets/art`: RGBA runtime conversions of the supplied original tile sheet, scoreboard digits, and game icon.
- SDL3 provides the native window, input, audio, and per-user preference path across supported desktop systems. `src/platform_file.hpp` and `src/platform_file.cpp` isolate atomic profile replacement: Windows uses `MoveFileExW`, while POSIX systems use `rename`.
- Menus, campaign navigation, HUD, pause/results, settings, and help are implemented in the application and renderer modules. SDL handles desktop windowing, input, audio, and preference paths; small adapters isolate native UTF-8 command-line handling, startup-time seeding, and atomic profile replacement. Verify macOS/Linux with their native CI jobs before treating those builds as supported releases.

Keep the supplied legacy executable, assembly, and sounds outside this game's runtime dependencies. The supplied original artwork is intentionally converted and used at runtime. Use `CHIPS.DAT` as the exact map/metadata source and the annotated disassembly as behavioral reference.



Live movement input now follows the original `MoveChip` key-down path: Chip moves immediately when the turn gate is open, and only one later input is buffered. Forced slide movement remains on the separate involuntary path. Level 3's four boot pickups are `0x68`–`0x6B`; ice-corner/force-floor IDs `0x1A`–`0x1D` now retain their correct terrain sprites. The native rebuild succeeded and all 12 CTest checks passed.

Source comparison also corrected sliding input: key presses do nothing on ice, ice corners, and teleporters while a slide is active; matching force-floor input is ignored, while a different direction may steer Chip. Regression checks and all 12 CTest checks pass.

Replay scripts support both tick-queued `move` inputs and immediate key-down-style `press` inputs. The recorded Level 2 route now uses `press` and wins natively in 53 moves; all seven checked campaign routes pass.

The per-level campaign validator now mixes immediate physical-style presses with queued moves across its deterministic replays and seeded stress runs. All 12 CTest checks pass, including the 149-level validation and exact decoded-map cross-check.

For source-game key captures in `key_edges_us_v1` format, `scripts/replay_recorded_key_events.py` replays every key-down at its timestamp-derived position on the native 110 ms tick clock. `--pre-record-ticks` models game time that elapsed before the recorder started, and `--phase-ms` models the recorder/timer boundary phase; these values must come from capture context before treating a replay as a parity result. The event conversion itself is covered by CTest.

The renderer's transparent sprite selection is an SDL-independent mapping tested across all 256 tile IDs. Cleaned creature frames cover `0x40`–`0x63`, cleaned item frames cover `0x64`–`0x6B`, and exact source image/mask frames provide fallback for IDs `0x40`–`0x6F`. Level 3 ice corners and force floors `0x1A`–`0x1D` retain original atlas terrain art.

Campaign validation also pins the four Level 3 boot tile IDs to their exact `CHIPS.DAT` coordinates and confirms they occur only in the upper map layer. This is paired with the full 149-level decoded-map comparison and a visual check of the current native Level 3 board against the supplied reference.

Clone-machine coverage now also includes chained output: a clone-emitted block presses a second linked CloneButton and immediately emits its block. The native Windows engine and all 13 CTest checks pass after the regression was added.

Creature movement now follows the original `MoveMonster` source-layer cleanup in both MonsterLoop and SlipLoop: on departure, Upper is restored from Lower and Lower becomes Floor, except when the creature leaves a CloneMachine. Regressions verify the restored terrain and Chip's movement onto the vacated square. The Windows Release build and all 15 CTest checks pass, including campaign decoding and replay validation.

Campaign runtime validation also checks that every live creature's displayed sprite/facing matches its logical board position and that live creatures do not overlap during the deterministic and seeded scenarios exercised across all 149 levels. All 15 Windows CTest checks pass with these invariants enabled.
