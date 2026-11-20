# Changelog

## 1.0.2 — 2026-11-18

### Gameplay and progress

- Added campaign continuation, new-game flow, paged level selection, password access, and best-time pages.
- Added custom `CHIPS.DAT` loading with validation and separate progress for each pack.
- Added focus-loss pausing, mouse-directed movement, controller hot-plug support, and keyboard, mouse, touch, and gamepad navigation.
- Added saved controls, audio levels, display scale, fullscreen, reduced-motion, and progress settings.

### Interface and audio

- Added in-game controls and gameplay help, refreshed menus and dialogs, and added win and death illustrations.
- Added the supplied WAV effects and MIDI music to native builds. WebAssembly uses a bundled SoundFont synthesizer.
- Improved browser audio startup, tab suspension, touch input, file selection, fullscreen, and IndexedDB save handling.

### Packages

- Published Windows x64, macOS ARM64 and Intel, Linux x86_64, and WebAssembly packages.
- Added platform launchers for running the browser package locally and included required notices and credits.
- Excluded CCLP2 data from the default packages; the original campaign remains the default.

For build and verification details, see the [README](README.md) and [architecture overview](ARCHITECTURE.md).