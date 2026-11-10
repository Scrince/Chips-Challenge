#include "audio.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace chips {
namespace {
constexpr int kSampleRate = 22050;
constexpr int kChannels = 2;

struct Tone {
    float startFrequency;
    float endFrequency;
    float seconds;
    float gain;
    float attackSeconds;
    float releaseSeconds;
    enum class Shape { sine, bell, triangle, square, saw } shape;
};

constexpr Tone note(float startFrequency, float endFrequency, float seconds, float gain,
                    Tone::Shape shape, float attack = 0.004F, float release = 0.018F) {
    return {startFrequency, endFrequency, seconds, gain, attack, release, shape};
}

std::array<Tone, 4> tonesFor(AudioCue cue, std::size_t& count) {
    switch (cue) {
        case AudioCue::move:
            count = 1;
            return {{note(640.0F, 510.0F, 0.026F, 0.075F, Tone::Shape::triangle, 0.001F, 0.012F)}};
        case AudioCue::blocked:
            count = 1;
            return {{note(145.0F, 92.0F, 0.075F, 0.105F, Tone::Shape::square, 0.002F, 0.035F)}};
        case AudioCue::pickup:
            count = 2;
            return {{note(659.25F, 659.25F, 0.070F, 0.105F, Tone::Shape::bell, 0.001F, 0.052F),
                     note(987.77F, 987.77F, 0.105F, 0.095F, Tone::Shape::bell, 0.001F, 0.082F)}};
        case AudioCue::keyPickup:
            count = 2;
            return {{note(880.0F, 1174.66F, 0.055F, 0.095F, Tone::Shape::bell, 0.001F, 0.038F),
                     note(1318.51F, 1318.51F, 0.105F, 0.085F, Tone::Shape::sine, 0.001F, 0.082F)}};
        case AudioCue::bootPickup:
            count = 2;
            return {{note(392.0F, 523.25F, 0.080F, 0.095F, Tone::Shape::triangle, 0.002F, 0.038F),
                     note(587.33F, 587.33F, 0.085F, 0.085F, Tone::Shape::square, 0.002F, 0.045F)}};
        case AudioCue::switchTile:
            count = 3;
            return {{note(196.0F, 196.0F, 0.055F, 0.085F, Tone::Shape::square, 0.001F, 0.014F),
                     note(293.66F, 293.66F, 0.065F, 0.090F, Tone::Shape::triangle, 0.001F, 0.020F),
                     note(440.0F, 587.33F, 0.105F, 0.090F, Tone::Shape::saw, 0.003F, 0.045F)}};
        case AudioCue::teleport:
            count = 2;
            return {{note(330.0F, 990.0F, 0.105F, 0.085F, Tone::Shape::sine, 0.008F, 0.035F),
                     note(880.0F, 440.0F, 0.090F, 0.070F, Tone::Shape::bell, 0.001F, 0.060F)}};
        case AudioCue::death:
            count = 3;
            return {{note(392.0F, 330.0F, 0.085F, 0.105F, Tone::Shape::triangle, 0.002F, 0.024F),
                     note(293.66F, 220.0F, 0.100F, 0.110F, Tone::Shape::triangle, 0.002F, 0.032F),
                     note(196.0F, 98.0F, 0.190F, 0.115F, Tone::Shape::saw, 0.004F, 0.085F)}};
        case AudioCue::victory:
            count = 4;
            return {{note(392.0F, 392.0F, 0.085F, 0.095F, Tone::Shape::bell, 0.001F, 0.050F),
                     note(493.88F, 493.88F, 0.085F, 0.095F, Tone::Shape::bell, 0.001F, 0.050F),
                     note(587.33F, 587.33F, 0.085F, 0.100F, Tone::Shape::bell, 0.001F, 0.050F),
                     note(783.99F, 783.99F, 0.220F, 0.110F, Tone::Shape::bell, 0.002F, 0.140F)}};
    }
    count = 0;
    return {};
}

float shapeSample(Tone::Shape shape, float phase) {
    constexpr float tau = 6.28318530718F;
    const float sine = std::sin(phase * tau);
    switch (shape) {
        case Tone::Shape::sine: return sine;
        case Tone::Shape::bell:
            return 0.74F * sine + 0.20F * std::sin(phase * tau * 2.0F)
                 + 0.06F * std::sin(phase * tau * 3.0F);
        case Tone::Shape::triangle: return 0.63661977F * std::asin(sine);
        case Tone::Shape::square: return sine >= 0.0F ? 1.0F : -1.0F;
        case Tone::Shape::saw: return 2.0F * phase - 1.0F;
    }
    return 0.0F;
}

std::vector<float> originalMusicLoop() {
    constexpr float beatSeconds = 60.0F / 116.0F;
    constexpr std::array<int, 32> melodyMidi{
        72, 76, 79, 76, 69, 72, 76, 79, 65, 69, 72, 69, 67, 71, 74, 79,
        72, 74, 76, 79, 69, 67, 64, 72, 65, 67, 69, 72, 67, 64, 62, 60};
    constexpr std::array<int, 8> bassMidi{36, 33, 29, 31, 36, 33, 29, 31};
    std::array<float, 32> melodyFrequencies{};
    std::array<float, 8> bassFrequencies{};
    for (std::size_t i = 0; i < melodyFrequencies.size(); ++i)
        melodyFrequencies[i] = 440.0F * std::pow(2.0F,
            static_cast<float>(melodyMidi[i] - 69) / 12.0F);
    for (std::size_t i = 0; i < bassFrequencies.size(); ++i)
        bassFrequencies[i] = 440.0F * std::pow(2.0F,
            static_cast<float>(bassMidi[i] - 69) / 12.0F);
    const auto totalFrames = static_cast<std::size_t>(
        std::lround(beatSeconds * 32.0F * static_cast<float>(kSampleRate)));
    std::vector<float> samples(totalFrames * kChannels);
    constexpr float tau = 6.28318530718F;
    for (std::size_t frame = 0; frame < totalFrames; ++frame) {
        const float time = static_cast<float>(frame) / static_cast<float>(kSampleRate);
        const float beatPosition = time / beatSeconds;
        const auto beat = std::min<std::size_t>(31, static_cast<std::size_t>(beatPosition));
        const float beatProgress = beatPosition - static_cast<float>(beat);
        const float melodyFrequency = melodyFrequencies[beat];
        const float bassFrequency = bassFrequencies[beat / 4];
        const float melodyPhase = time * melodyFrequency;
        const float bassPhase = time * bassFrequency;
        const float attack = std::min(1.0F, beatProgress * 24.0F);
        const float release = std::min(1.0F, (1.0F - beatProgress) * 10.0F);
        const float bassEnvelope = std::min(1.0F, beatProgress * 40.0F) *
                                   std::min(1.0F, (1.0F - beatProgress) * 8.0F);
        const float melody = (std::sin(melodyPhase * tau) * 0.72F +
            std::sin(melodyPhase * tau * 2.0F) * 0.20F +
            std::sin(melodyPhase * tau * 3.0F) * 0.08F) * attack * release * 0.16F;
        const float bass = (0.63661977F * std::asin(std::sin(bassPhase * tau))) *
                           bassEnvelope * 0.095F;
        const float sample = melody + bass;
        samples[frame * 2] = sample * 0.96F;
        samples[frame * 2 + 1] = sample;
    }
    return samples;
}

const std::vector<float>& musicLoop() {
    static const auto samples = originalMusicLoop();
    return samples;
}
}

AudioSystem::~AudioSystem() {
    shutdown();
}

void AudioSystem::shutdown() {
    if (stream_) {
        SDL_DestroyAudioStream(stream_);
        stream_ = nullptr;
    }
    if (musicStream_) {
        SDL_DestroyAudioStream(musicStream_);
        musicStream_ = nullptr;
    }
}

bool AudioSystem::initialize() {
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_Log("Audio is unavailable; continuing without sound: %s", SDL_GetError());
        return false;
    }
    const SDL_AudioSpec spec{SDL_AUDIO_F32, kChannels, kSampleRate};
    stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!stream_) {
        SDL_Log("Could not open audio output; continuing without sound: %s", SDL_GetError());
        return false;
    }
    updateGain();
    if (!SDL_ResumeAudioStreamDevice(stream_)) {
        SDL_Log("Could not start audio output: %s", SDL_GetError());
        SDL_DestroyAudioStream(stream_);
        stream_ = nullptr;
        return false;
    }
    musicStream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (musicStream_) {
        updateMusicGain();
        if (!SDL_ResumeAudioStreamDevice(musicStream_)) {
            SDL_Log("Could not start music output; continuing with sound effects only: %s", SDL_GetError());
            SDL_DestroyAudioStream(musicStream_);
            musicStream_ = nullptr;
        }
    } else {
        SDL_Log("Could not open music output; continuing with sound effects only: %s", SDL_GetError());
    }
    update();
    return true;
}

void AudioSystem::update() {
    if (!musicStream_ || musicMuted_ || musicVolume_ <= 0.0F) return;
    const auto& loop = musicLoop();
    const auto loopBytes = static_cast<int>(loop.size() * sizeof(float));
    const int availableBytes = SDL_GetAudioStreamAvailable(musicStream_);
    if (availableBytes < 0) {
        disableMusic("Could not query background-music stream");
        return;
    }
    if (availableBytes < loopBytes)
        queueMusic();
}

void AudioSystem::queueMusic() {
    if (!musicStream_ || musicMuted_ || musicVolume_ <= 0.0F) return;
    const auto& loop = musicLoop();
    const auto loopBytes = static_cast<int>(loop.size() * sizeof(float));
    for (int i = 0; i < 2; ++i) {
        if (!SDL_PutAudioStreamData(musicStream_, loop.data(), loopBytes)) {
            disableMusic("Could not queue original background music");
            return;
        }
    }
}

void AudioSystem::disableMusic(const char* reason) {
    if (!musicStream_) return;
    SDL_Log("%s; continuing without background music: %s", reason, SDL_GetError());
    SDL_DestroyAudioStream(musicStream_);
    musicStream_ = nullptr;
}

void AudioSystem::play(AudioCue cue) {
    if (!stream_ || muted_ || volume_ <= 0.0F) return;
    std::size_t toneCount{};
    const auto tones = tonesFor(cue, toneCount);
    std::size_t totalFrames = 0;
    for (std::size_t i = 0; i < toneCount; ++i)
        totalFrames += static_cast<std::size_t>(tones[i].seconds * kSampleRate);
    std::vector<float> samples(totalFrames * kChannels);
    std::size_t frame = 0;
    for (std::size_t toneIndex = 0; toneIndex < toneCount; ++toneIndex) {
        const auto& tone = tones[toneIndex];
        const auto frames = static_cast<std::size_t>(tone.seconds * kSampleRate);
        float phase = 0.0F;
        for (std::size_t i = 0; i < frames; ++i, ++frame) {
            const float progress = frames > 1
                ? static_cast<float>(i) / static_cast<float>(frames - 1) : 0.0F;
            const float frequency = tone.startFrequency +
                (tone.endFrequency - tone.startFrequency) * progress;
            phase += frequency / static_cast<float>(kSampleRate);
            phase -= std::floor(phase);
            const float attack = tone.attackSeconds > 0.0F
                ? std::min(1.0F, static_cast<float>(i) /
                    (tone.attackSeconds * static_cast<float>(kSampleRate))) : 1.0F;
            const float release = tone.releaseSeconds > 0.0F
                ? std::min(1.0F, static_cast<float>(frames - i) /
                    (tone.releaseSeconds * static_cast<float>(kSampleRate))) : 1.0F;
            const float sample = shapeSample(tone.shape, phase) * attack * release * tone.gain;
            samples[frame * 2] = sample;
            samples[frame * 2 + 1] = sample;
        }
    }
    const auto bytes = static_cast<int>(samples.size() * sizeof(float));
    if (!SDL_PutAudioStreamData(stream_, samples.data(), bytes))
        SDL_Log("Could not queue sound effect: %s", SDL_GetError());
}

void AudioSystem::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0F, 1.0F);
    updateGain();
}

void AudioSystem::setMuted(bool muted) {
    muted_ = muted;
    updateGain();
}

void AudioSystem::setMusicVolume(float volume) {
    musicVolume_ = std::clamp(volume, 0.0F, 1.0F);
    updateMusicGain();
    update();
}

void AudioSystem::setMusicMuted(bool muted) {
    musicMuted_ = muted;
    updateMusicGain();
    update();
}

void AudioSystem::updateGain() {
    if (stream_ && !SDL_SetAudioStreamGain(stream_, muted_ ? 0.0F : volume_))
        SDL_Log("Could not update sound-effect volume: %s", SDL_GetError());
}

void AudioSystem::updateMusicGain() {
    if (musicStream_ && !SDL_SetAudioStreamGain(musicStream_, musicMuted_ ? 0.0F : musicVolume_))
        disableMusic("Could not update background-music volume");
}
}
