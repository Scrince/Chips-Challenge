#include "audio.hpp"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <iostream>

int main() {
    chips::AudioSystem audio;
    if (!audio.initialize()) {
        std::cerr << "Dummy SDL audio backend failed to initialize: " << SDL_GetError() << '\n';
        SDL_Quit();
        return EXIT_FAILURE;
    }
    audio.setVolume(0.4F);
    audio.play(chips::AudioCue::move);
    audio.play(chips::AudioCue::blocked);
    audio.play(chips::AudioCue::pickup);
    audio.play(chips::AudioCue::switchTile);
    audio.play(chips::AudioCue::death);
    audio.play(chips::AudioCue::victory);
    audio.setMuted(true);
    audio.play(chips::AudioCue::move);
    audio.setMuted(false);
    audio.setVolume(1.5F);
    if (audio.muted() || audio.volume() != 1.0F || !audio.available()) {
        std::cerr << "Audio controls did not retain clamped runtime state\n";
        audio.shutdown();
        SDL_Quit();
        return EXIT_FAILURE;
    }
    audio.shutdown();
    SDL_Quit();
    std::cout << "SDL audio stream and all procedural cues passed.\n";
    return EXIT_SUCCESS;
}
