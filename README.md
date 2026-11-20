# Chips Challenge Native

A C++20 remake of Chip's Challenge built with SDL3. The game runs on Windows, macOS, Linux, and in modern browsers through WebAssembly.

## Features

- The original 149-level campaign, loaded from `data/CHIPS.DAT`.
- Deterministic game rules with replayable inputs and engine tests.
- Keyboard, mouse, touch, and gamepad controls; saved progress and settings.
- Native audio and browser MIDI playback.
- Custom compatible level-pack loading.

## Releases

Version 1.0.2 includes Windows x64, macOS ARM64 and Intel, Linux x86_64, and WebAssembly packages in [`releases/1.0.2/`](releases/1.0.2/). Each archive has a detached OpenPGP signature, and the aggregate checksum manifest is signed. See [`docs/RELEASE_SIGNING.md`](docs/RELEASE_SIGNING.md) for verification. Windows Authenticode signing and macOS notarization are not configured.

## Build

CMake 3.24 or newer, Git, and a C++20 compiler are required. CMake fetches SDL3 at a pinned release commit.

### Windows

With Visual Studio 2022 or MinGW-w64 installed, build and package with:

```powershell
./scripts/build-windows.ps1 -Package
```

The package is written under `build/windows/packages/`. Add `-Launch` to run the built game.

### macOS

Install the Apple Command Line Tools and CMake, then run:

```sh
cmake -S . -B build/macos -DCMAKE_BUILD_TYPE=Release
cmake --build build/macos --parallel
ctest --test-dir build/macos --output-on-failure
```

Run `bash ./scripts/build-macos-release.sh` on macOS to create the architecture-specific packages.

### Linux

On Ubuntu 24.04, install the compiler and SDL3 backend dependencies listed in [the native workflow](.github/workflows/native-build.yml), then build and test:

```sh
cmake -S . -B build/linux -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux --parallel
ctest --test-dir build/linux --output-on-failure
```

Run `bash ./scripts/build-linux-release.sh` to create the release tarball.

### WebAssembly

Install and activate the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html), then run:

```sh
bash ./scripts/build-webassembly.sh
python3 -m http.server 8000 --directory build/webassembly
```

Open `http://localhost:8000/chips-native.html`. The browser build must be served over HTTP or HTTPS; it does not run from `file://`. For an optimized release package, use `bash ./scripts/build-webassembly-release.sh`.

## Tests and project notes

Run native checks with `ctest --test-dir <build-directory> --output-on-failure`. GitHub Actions builds the native targets and runs browser smoke checks in Chromium, Firefox, and WebKit.

See [`ARCHITECTURE.md`](ARCHITECTURE.md) for the code layout, [`docs/mechanics-parity.md`](docs/mechanics-parity.md) for the comparison process, and [`CHANGELOG.md`](CHANGELOG.md) for release changes. Detailed implementation notes are kept in [`docs/implementation-history.md`](docs/implementation-history.md).