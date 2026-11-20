# Changelog

## 1.0.2 — 2026-11-18

- Added v1.0.2 release packages for Windows x64, macOS ARM64/x86_64/universal, Linux x86_64, and WebAssembly under `releases/1.0.2/`, with SHA-256 manifests in the platform folders.
- Verified Windows, macOS, and Linux Release builds with all 15 CTest checks passing on each platform; native campaign and packaged-art validation also passed. The optimized WebAssembly release completed its build-artifact, JavaScript syntax, and audio checks before packaging.
- Added a native title and campaign flow with Continue and New Game actions, paged level selection and previews, password access, and paged Best Times.
- Added custom `CHIPS.DAT` level-pack loading with validation before switching campaigns and separate saved progress for each pack.
- Added focus-loss pausing, mouse-directed board movement, controller hot-plug handling, and mouse/keyboard/gamepad navigation for menus and result actions.
- Added persisted settings for remappable controls, effects/music volume and mute, display scale, fullscreen, and reduced motion.
- Added in-game controls and gameplay help, bundled antialiased Noto Sans for interface text, updated circuit-board backgrounds, and original win/death illustrations.
- Added original WAV effects and MIDI music across native platforms, with FluidSynth support on Linux; WebAssembly uses the bundled SoundFont synthesizer.
- Added a browser file picker for loading custom level packs in the WebAssembly build, including cancellation and read-error handling.
- Added F10 and backquote keyboard access to the in-game menus so browser users can reach level-pack actions without relying on browser-reserved shortcuts.
- Replaced the browser's oscillator MIDI approximation with a SoundFont General MIDI synthesizer using the included GeneralUser GS bank.
- Added the browser synthesizer and its sound bank licenses to the WebAssembly package; music now uses sampled instruments and MIDI sequencing in an AudioWorklet.
- Kept browser audio-context resume synchronous with the initial user gesture so autoplay restrictions do not silence background music.
- Connected the SoundFont synthesizer's output to Web Audio so scheduled background MIDI reaches the browser speakers.
- Opened level-pack dialogs directly from trusted browser input events for Safari compatibility.
- Hardened browser music for background-tab suspension and page restoration, added a panning fallback for older Web Audio implementations, and rejected truncated MIDI events safely.
- Fixed browser music playback after delayed timers so the MIDI scheduler resumes at the current song position instead of restarting a short phrase.
- Wired the WebAssembly Help menu to show the bundled manual in an in-game browser dialog.
- Fixed Windows release packaging to include the original WAV effects and MIDI background tracks.
- Excluded the CCLP2 campaign data and disclaimer from release packages; releases include only the original campaign by default.
- Added touch-to-click support for touch browsers and kept pinch zoom available for accessibility.
- Opened the WebAssembly level-pack picker directly from SDL finger events so Firefox touch input retains its browser gesture.
- Routed browser startup through SDL's main wrapper and explicitly linked IDBFS so optimized WebAssembly builds persist saves in IndexedDB.
- Added an optimized WebAssembly release build script and versioned ZIP with an index page, favicon, font credits, and third-party notices.
- Added a one-click Windows launcher that starts a local WebAssembly server and opens the game in the default browser.
- Added matching Linux and macOS launchers that start a local server and open the game in the default browser.
- Kept the Debug WebAssembly development build separate from the optimized release package.
- Routed browser fullscreen changes through the original trusted input gesture and synchronized the game state with browser fullscreen events.
- Removed macOS AppleDouble metadata from the WebAssembly asset filesystem and added checks for the required campaign, font, and audio assets.
- Enabled SDL3 WAV effects, browser MIDI playback, and IndexedDB profile storage in the WebAssembly build.
