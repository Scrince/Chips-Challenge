# Audio provenance

The remake does not bundle the supplied legacy WAV or MIDI files. `src/audio.cpp` synthesizes its effects at runtime using SDL3 audio streams. Each cue has its own pitch contour, tone shape, gain, and attack/release envelope: movement and switching use compact retro waveforms, pickups and victory use a brighter bell-like tone, and teleport travel has a rising/falling sweep. These newly authored cues cover movement, blocked movement, item collection, switches, opening keyed doors and sockets, teleport travel, death/timeout, and victory. No music is currently played; original music can be added later without replacing or deriving from the legacy soundtrack.

The machine-readable `assets/ASSET_MANIFEST.json` records the runtime audio source and the legacy-media exclusion policy. The audio implementation is project-authored C++ and uses no external sound samples. No separate license is currently declared for project-created assets.

Sound effects default to 65% volume and enabled. Open the in-game settings with `O`; press `M` to mute or unmute, and `-` / `=` to change volume in 5% steps. The settings panel reports that music is currently off. The settings are stored in the versioned per-user campaign profile. Audio-device initialization is optional, so a missing playback device does not prevent the game from launching.
