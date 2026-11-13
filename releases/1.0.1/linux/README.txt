Linux x86_64 release

Built and validated in Docker using Ubuntu 24.04. Extract Chips-Challenge-1.0.1-Linux-x86_64.tar.gz, then run Chips-Challenge/chips-native from its folder.

The archive includes the game, SDL runtimes, assets, campaign data, and app-specific files. The host needs Linux system libraries for SDL X11/Wayland, graphics, and audio backends, plus libfluidsynth3 and a General MIDI soundfont. Install fluid-soundfont-gm, or set FLUIDSYNTH_SOUNDFONT to an SF2/SF3 file.
