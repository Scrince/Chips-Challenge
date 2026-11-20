#!/usr/bin/env python3
"""Check that a WebAssembly build contains all browser runtime artifacts."""

from __future__ import annotations

import pathlib
import re
import sys


def fail(message: str) -> int:
    print(f"WebAssembly build check failed: {message}", file=sys.stderr)
    return 1


def main() -> int:
    if len(sys.argv) != 2:
        return fail("usage: webassembly_build_test.py <build-directory>")
    build_dir = pathlib.Path(sys.argv[1]).resolve()
    html = build_dir / "chips-native.html"
    js = build_dir / "chips-native.js"
    midi_player = build_dir / "midi-player.js"
    synth_module = build_dir / "soundfont-midi.js"
    vendor_dir = build_dir / "vendor"
    soundfont = build_dir / "soundfonts" / "GeneralUser-GS.sf2"
    favicon = build_dir / "favicon.ico"
    wasm = build_dir / "chips-native.wasm"
    data = build_dir / "chips-native.data"
    for artifact in (html, js, midi_player, synth_module, favicon, wasm, data, soundfont):
        if not artifact.is_file() or artifact.stat().st_size == 0:
            return fail(f"missing or empty artifact: {artifact.name}")
    for artifact in (
        vendor_dir / "spessasynth_lib" / "index.js",
        vendor_dir / "spessasynth_lib" / "spessasynth_processor.min.js",
        vendor_dir / "spessasynth_core" / "index.js",
        vendor_dir / "fflate" / "browser.js",
        vendor_dir / "stb-vorbis" / "index.js",
    ):
        if not artifact.is_file() or artifact.stat().st_size == 0:
            return fail(f"missing or empty browser synthesizer dependency: {artifact.relative_to(build_dir)}")

    wasm_bytes = wasm.read_bytes()
    if wasm_bytes[:8] != b"\0asm\x01\0\0\0":
        return fail("chips-native.wasm does not have a WebAssembly v1 header")
    page = html.read_text(encoding="utf-8")
    has_canvas = re.search(r"<canvas\b[^>]*\bid=(?:\"canvas\"|'canvas'|canvas(?=[\s>]))", page, re.IGNORECASE)
    if not has_canvas or "chips-native.js" not in page or "midi-player.js" not in page:
        return fail("browser page is missing its game canvas or runtime scripts")
    if "favicon.ico" not in page:
        return fail("browser page does not provide its packaged application icon")
    shell = pathlib.Path(__file__).resolve().parents[1] / "web" / "wasm-shell.html"
    shell_text = shell.read_text(encoding="utf-8")
    main_source = pathlib.Path(__file__).resolve().parents[1] / "src" / "main.cpp"
    main_text = main_source.read_text(encoding="utf-8")
    if "#include <SDL3/SDL_main.h>" not in main_text:
        return fail("WebAssembly startup bypasses SDL's persistent-filesystem initialization")
    if "user-scalable=no" in shell_text or "touchAction = 'manipulation'" not in shell_text:
        return fail("browser shell disables accessible zoom gestures")
    if "SDL_HINT_TOUCH_MOUSE_EVENTS" not in main_text or "SDL_SetHint" not in main_text:
        return fail("touch-only browsers are not wired to the game's pointer controls")
    if "SDL_EVENT_FINGER_DOWN" not in main_text:
        return fail("touch level-pack selection is missing its synchronous browser-gesture path")
    if "new FileReader()" not in main_text or "file.arrayBuffer" not in main_text:
        return fail("level-pack loading is missing compatibility file-reading paths")
    if "requestBrowserFullscreenFromGesture" not in main_text or "SDL_EVENT_WINDOW_ENTER_FULLSCREEN" not in main_text:
        return fail("browser fullscreen requests are not synchronized with user gestures")
    cmake = (pathlib.Path(__file__).resolve().parents[1] / "CMakeLists.txt").read_text(encoding="utf-8")
    if 'SHELL:-sMIN_WEBGL_VERSION=1' not in cmake:
        return fail("browser graphics do not provide the WebGL 1 compatibility floor")
    if 'SHELL:-lidbfs.js' not in cmake or 'SHELL:-sEXPORTED_RUNTIME_METHODS=IDBFS' not in cmake:
        return fail("optimized WebAssembly builds do not retain the IndexedDB filesystem implementation")
    player = midi_player.read_text(encoding="utf-8")
    synth_source = synth_module.read_text(encoding="utf-8")
    if "chipsStartMidiMusic" not in player or "chipsUnlockMusicAudio" not in player:
        return fail("browser MIDI bridge does not expose playback and audio-unlock entry points")
    if "WorkletSynthesizer" not in synth_source or "Sequencer" not in synth_source:
        return fail("browser MIDI playback is missing SoundFont synthesis or MIDI sequencing")
    if "GeneralUser-GS.sf2" not in synth_source or soundfont.stat().st_size < 1_000_000:
        return fail("browser General MIDI sound bank is missing or incomplete")
    if '"spessasynth_lib"' not in page or '"stb-vorbis"' not in page:
        return fail("browser page is missing its local synthesizer import map")
    loader = js.read_text(encoding="utf-8", errors="replace")
    if "chips-native.wasm" not in loader or "chips-native.data" not in loader:
        return fail("loader is not wired to the WebAssembly and preloaded asset files")
    if "/data/CCLP2.DAT" in loader or "/data/CCLP2-disclaimer.txt" in loader:
        return fail("WebAssembly payload includes CCLP2 files that are not part of the release")
    if "instantiateStreaming" not in loader or "instantiateArrayBuffer" not in loader:
        return fail("loader is missing its non-streaming WebAssembly fallback")
    if not re.search(r"Module\[['\"]_chipsWebLevelPackDialogResult['\"]\]", loader):
        return fail("browser file picker callback is not exported to the WebAssembly runtime")
    if not re.search(r"Module\[['\"]_chipsBrowserFullscreenRequestFailed['\"]\]", loader):
        return fail("browser fullscreen failure callback is not exported to the runtime")
    required_assets = (
        '"/assets/audio/CLICK3.WAV"',
        '"/assets/audio/CHIP01.MID"',
        '"/assets/audio/CHIP02.MID"',
        '"/assets/fonts/NotoSans.ttf"',
        '"/data/CHIPS.DAT"',
    )
    missing_assets = [asset for asset in required_assets if asset not in loader]
    if missing_assets:
        return fail("preloaded filesystem is missing: " + ", ".join(missing_assets))
    if '"/assets/._' in loader or '"/data/._' in loader:
        return fail("AppleDouble metadata files were included in the browser payload")
    if b"emscripten audio driver" not in wasm_bytes.lower() and "emscripten audio driver" not in loader.lower():
        return fail("SDL3 Emscripten audio backend was not linked into the runtime")
    if b"/libsdl" not in wasm_bytes:
        return fail("SDL3 IndexedDB profile storage was not enabled")
    if "IDBFS" not in loader or "indexedDB" not in loader:
        return fail("Emscripten IndexedDB filesystem support is missing from the browser runtime")

    print("WebAssembly artifacts present: HTML, JavaScript, MIDI player, WASM, preloaded data, SDL3 WebAudio, and IndexedDB storage")
    print(f"WASM: {wasm.stat().st_size:,} bytes; asset filesystem: {data.stat().st_size:,} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
