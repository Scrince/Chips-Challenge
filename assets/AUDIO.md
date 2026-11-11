# Audio assets and playback

The runtime now uses the supplied original WAV effects and MIDI music files, copied byte-for-byte into `assets/audio/`. `ASSET_MANIFEST.json` records each source path and SHA-256. The Windows runtime loads WAVs through SDL3 and plays MIDI natively with the Windows Multimedia sequencer (MCI); it does not launch or emulate the original game. The first supplied MIDI track is used on odd-numbered levels and the second on even-numbered levels. macOS and Linux currently play the WAV effects but leave MIDI playback unavailable until a native sequencer backend is added.

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

The audio settings retain independent volume and mute controls for effects and music. Audio-device initialization remains optional; unavailable audio devices do not prevent the game from starting. All included audio in this directory is part of the user-supplied original game files; do not re-encode it as part of routine builds.
