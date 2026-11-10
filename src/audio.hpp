#pragma once

#include <SDL3/SDL.h>

namespace chips {
enum class AudioCue { move, blocked, pickup, keyPickup, bootPickup, switchTile, teleport, death, victory };

class AudioSystem {
public:
    AudioSystem() = default;
    ~AudioSystem();
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    // Audio is optional: failure to open a playback device never prevents startup.
    [[nodiscard]] bool initialize();
    void shutdown();
    void play(AudioCue cue);
    void update();
    void setVolume(float volume);
    void setMuted(bool muted);
    void setMusicVolume(float volume);
    void setMusicMuted(bool muted);
    [[nodiscard]] float volume() const { return volume_; }
    [[nodiscard]] bool muted() const { return muted_; }
    [[nodiscard]] float musicVolume() const { return musicVolume_; }
    [[nodiscard]] bool musicMuted() const { return musicMuted_; }
    [[nodiscard]] bool available() const { return stream_ != nullptr; }

private:
    void updateGain();
    void updateMusicGain();
    void queueMusic();
    void disableMusic(const char* reason);
    SDL_AudioStream* stream_{};
    SDL_AudioStream* musicStream_{};
    float volume_{0.65F};
    bool muted_{};
    float musicVolume_{0.20F};
    bool musicMuted_{};
};
}
