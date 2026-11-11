#include "audio.hpp"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>

int main() {
    chips::AudioSystem audio;
    if (!audio.initialize(std::filesystem::path(CHIPS_TEST_RESOURCE_ROOT))) {
        std::cerr << "Dummy SDL audio backend failed to initialize: " << SDL_GetError() << '\n';
        SDL_Quit();
        return EXIT_FAILURE;
    }
    audio.setVolume(0.4F);
    for (const auto cue : {chips::AudioCue::move, chips::AudioCue::blocked,
             chips::AudioCue::pickup, chips::AudioCue::keyPickup,
             chips::AudioCue::bootPickup, chips::AudioCue::switchTile,
             chips::AudioCue::teleport, chips::AudioCue::death,
             chips::AudioCue::levelComplete, chips::AudioCue::door,
             chips::AudioCue::thief, chips::AudioCue::splash})
        audio.play(cue);
    if (audio.loadedCueCount() != 11) {
        std::cerr << "Expected all eleven original WAV-backed sound cues to load; got "
                  << audio.loadedCueCount() << '\n';
        audio.shutdown();
        SDL_Quit();
        return EXIT_FAILURE;
    }
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
    std::cout << "Original WAV effects loaded and SDL audio controls passed.\n";
    return EXIT_SUCCESS;
}
