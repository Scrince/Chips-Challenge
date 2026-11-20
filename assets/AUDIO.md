# Audio assets and playback

The runtime uses the supplied original WAV effects and MIDI music files, copied byte-for-byte into `assets/audio/`. `ASSET_MANIFEST.json` records each source path and SHA-256. The Windows runtime loads WAVs through SDL3 and plays MIDI natively with the Windows Multimedia sequencer (MCI); it does not launch or emulate the original game. macOS uses AudioToolbox, and Linux uses FluidSynth with a system General MIDI soundfont. Linux users can install `fluid-soundfont-gm` or set `FLUIDSYNTH_SOUNDFONT` to an SF2/SF3 file. The WebAssembly build uses the browser SpessaSynth SoundFont engine with GeneralUser GS, giving it sampled GM instruments instead of the earlier oscillator approximations. The first supplied MIDI track is used on odd-numbered levels and the second on even-numbered levels.

The WebAssembly release includes the GeneralUser GS 2.0.3 sound bank and its license in `soundfonts/`. It also includes pinned browser synthesizer dependencies and their licenses under `vendor/`. The native and browser synthesizers use different sound banks and engines, so their instrument samples and effects will not be identical.

Current cue mapping:

| Game event | Original file |
| --- | --- |
| Blocked movement | `OOF3.WAV` |
| Ordinary movement | Silent, matching the original cue table |
| Chip pickup | `CLICK3.WAV` |
| Key and boot pickup | `BLIP2.WAV` |
| Toggle/clone/tank switch | `POP2.WAV` |
| Keyed door and socket | `DOOR.WAV` |
| Teleport | `TELEPORT.WAV` |
| Death or timeout | `BUMMER.WAV` |
| Level completion | `DITTY1.WAV` |
| Thief | `STRIKE.WAV` |
| Block or clone fills water | `WATER2.WAV` |

Walking into water does not play the splash cue. On level completion, background MIDI stops and `DITTY1.WAV` plays as the completion cue.

The audio settings retain independent volume and mute controls for effects and music. Audio-device initialization remains optional; unavailable audio devices do not prevent the game from starting. The WAV and MIDI files in `assets/audio/` are part of the user-supplied original game files; do not re-encode them as part of routine builds. The WebAssembly SoundFont is distributed separately under its own license.
