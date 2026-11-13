#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <filesystem>
#include <cstddef>
#include <vector>

#ifdef __APPLE__
#include <AudioToolbox/AudioToolbox.h>
#endif

namespace chips {
enum class AudioCue { move, blocked, pickup, keyPickup, bootPickup, switchTile, teleport, death, levelComplete,
                      door, thief, splash };

class AudioSystem {
public:
    AudioSystem() = default;
    ~AudioSystem();
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    // Audio is optional: failure to open a playback device never prevents startup.
    [[nodiscard]] bool initialize(const std::filesystem::path& resourceRoot);
    void shutdown();
    void play(AudioCue cue);
    void startMusic(unsigned levelNumber);
    void stopMusic();
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
    [[nodiscard]] std::size_t loadedCueCount() const;

private:
    void updateGain();
    void updateMusicGain();
    std::filesystem::path resourceRoot_;
    std::array<std::vector<float>, 12> effects_{};
    SDL_AudioStream* stream_{};
    float volume_{0.65F};
    bool muted_{};
    float musicVolume_{0.20F};
    bool musicMuted_{};
#ifdef _WIN32
    bool midiOpen_{};
    bool midiPlaying_{};
    Uint64 nextMidiStatusCheck_{};
    Uint64 midiTrackStartedAt_{};
    Uint64 midiTrackLengthMs_{};
#elif defined(__APPLE__)
    MusicSequence musicSequence_{};
    MusicPlayer musicPlayer_{};
    AUGraph musicGraph_{};
    AudioUnit musicSynth_{};
    MusicTimeStamp midiTrackLengthBeats_{};
    bool midiOpen_{};
    bool midiPlaying_{};
#endif
};
}
