Linux x86_64 release

Built and validated in an Ubuntu 24.04 x86_64 Docker container. Extract
Chips-Challenge-1.0.2-Linux-x86_64.tar.gz, then run ./chips-native from the
extracted folder.

The archive includes the game, SDL runtimes, assets, campaign data, and
app-specific files. The host needs Linux system libraries for SDL X11/Wayland,
graphics, and audio backends, plus libfluidsynth3 and a General MIDI soundfont.
Install fluid-soundfont-gm, or set FLUIDSYNTH_SOUNDFONT to an SF2/SF3 file.

SHA-256 for the archive is recorded in SHA256SUMS.txt.
