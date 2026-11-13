#include "audio.hpp"

#include <algorithm>
#include <array>
#include <cwchar>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#elif defined(__APPLE__)
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace chips {
namespace {
constexpr int kSampleRate = 22050;
constexpr int kChannels = 2;
constexpr std::size_t kCueCount = 12;

std::string pathToUtf8(const std::filesystem::path& path) {
    const auto encoded = path.u8string();
    return {reinterpret_cast<const char*>(encoded.data()), encoded.size()};
}

const char* wavName(AudioCue cue) {
    switch (cue) {
        case AudioCue::move: return nullptr;
        case AudioCue::blocked: return "OOF3.WAV";
        case AudioCue::pickup: return "CLICK3.WAV";
        case AudioCue::keyPickup:
        case AudioCue::bootPickup: return "BLIP2.WAV";
        case AudioCue::switchTile: return "POP2.WAV";
        case AudioCue::teleport: return "TELEPORT.WAV";
        case AudioCue::death: return "BUMMER.WAV";
        case AudioCue::levelComplete: return "DITTY1.WAV";
        case AudioCue::door: return "DOOR.WAV";
        case AudioCue::thief: return "STRIKE.WAV";
        case AudioCue::splash: return "WATER2.WAV";
    }
    return nullptr;
}

std::size_t cueIndex(AudioCue cue) {
    return static_cast<std::size_t>(cue);
}

#ifdef _WIN32
bool sendMci(const std::wstring& command) {
    wchar_t error[256]{};
    const auto result = mciSendStringW(command.c_str(), nullptr, 0, nullptr);
    if (result == 0) return true;
    mciGetErrorStringW(result, error, static_cast<UINT>(std::size(error)));
    SDL_Log("MIDI playback command failed: %ls", error);
    return false;
}

bool playMciSequence() {
    if (!sendMci(L"seek chips_native_music to start")) return false;
    return sendMci(L"play chips_native_music repeat") ||
           sendMci(L"play chips_native_music");
}

bool queryMciNumber(const std::wstring& command, Uint64& value) {
    wchar_t response[64]{};
    if (mciSendStringW(command.c_str(), response,
                       static_cast<UINT>(std::size(response)), nullptr) != 0)
        return false;
    wchar_t* end{};
    const auto parsed = std::wcstoull(response, &end, 10);
    if (end == response) return false;
    value = static_cast<Uint64>(parsed);
    return true;
}
#endif
}

AudioSystem::~AudioSystem() { shutdown(); }

void AudioSystem::shutdown() {
    stopMusic();
#ifdef _WIN32
    // stopMusic closes the active MCI sequence.
#endif
    for (auto& samples : effects_) samples.clear();
    if (stream_) {
        SDL_DestroyAudioStream(stream_);
        stream_ = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool AudioSystem::initialize(const std::filesystem::path& resourceRoot) {
    resourceRoot_ = resourceRoot;
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_Log("Audio is unavailable; continuing without sound: %s", SDL_GetError());
        return false;
    }
    const SDL_AudioSpec outputSpec{SDL_AUDIO_F32, kChannels, kSampleRate};
    stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &outputSpec, nullptr, nullptr);
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

    std::array<bool, kCueCount> loaded{};
    for (std::size_t i = 0; i < kCueCount; ++i) {
        const auto cue = static_cast<AudioCue>(i);
        const auto* name = wavName(cue);
        if (!name) continue;
        const auto file = resourceRoot_ / "assets" / "audio" / name;
        const auto fileUtf8 = pathToUtf8(file);
        SDL_AudioSpec sourceSpec{};
        Uint8* sourceData{};
        Uint32 sourceLength{};
        if (!SDL_LoadWAV(fileUtf8.c_str(), &sourceSpec, &sourceData, &sourceLength)) {
            SDL_Log("Unable to load original sound %s: %s", fileUtf8.c_str(), SDL_GetError());
            continue;
        }
        SDL_AudioStream* converter = SDL_CreateAudioStream(&sourceSpec, &outputSpec);
        if (!converter || !SDL_PutAudioStreamData(converter, sourceData,
                                                   static_cast<int>(sourceLength)) ||
            !SDL_FlushAudioStream(converter)) {
            SDL_Log("Unable to convert original sound %s: %s", fileUtf8.c_str(), SDL_GetError());
            if (converter) SDL_DestroyAudioStream(converter);
            SDL_free(sourceData);
            continue;
        }
        const int convertedLength = SDL_GetAudioStreamAvailable(converter);
        if (convertedLength > 0) {
            auto& samples = effects_[i];
            samples.resize(static_cast<std::size_t>(convertedLength) / sizeof(float));
            const int bytesRead = SDL_GetAudioStreamData(converter, samples.data(), convertedLength);
            if (bytesRead != convertedLength) samples.clear();
            else loaded[i] = true;
        }
        SDL_DestroyAudioStream(converter);
        SDL_free(sourceData);
        if (!loaded[i]) SDL_Log("Original sound %s contained no playable samples", name);
    }
    SDL_Log("Loaded original sound cues: %s", std::count(loaded.begin(), loaded.end(), true) > 0
        ? "yes" : "none");
    return true;
}

void AudioSystem::play(AudioCue cue) {
    if (!stream_ || muted_ || volume_ <= 0.0F) return;
    const auto index = cueIndex(cue);
    if (index >= effects_.size() || effects_[index].empty()) return;
    const auto& samples = effects_[index];
    const auto bytes = static_cast<int>(samples.size() * sizeof(float));
    if (!SDL_PutAudioStreamData(stream_, samples.data(), bytes))
        SDL_Log("Could not queue original sound effect: %s", SDL_GetError());
}

std::size_t AudioSystem::loadedCueCount() const {
    return static_cast<std::size_t>(std::count_if(effects_.begin(), effects_.end(),
        [](const auto& samples) { return !samples.empty(); }));
}

void AudioSystem::startMusic(unsigned levelNumber) {
#ifdef _WIN32
    stopMusic();
    if (midiOutGetNumDevs() == 0) {
        SDL_Log("Background MIDI is unavailable: Windows reports no MIDI output device");
        return;
    }
    const wchar_t* filename = (levelNumber % 2 == 0) ? L"CHIP02.MID" : L"CHIP01.MID";
    const auto audioFile = resourceRoot_ / "assets" / "audio" / filename;
    if (!std::filesystem::is_regular_file(audioFile)) {
        SDL_Log("Background MIDI file is missing: %s", pathToUtf8(audioFile).c_str());
        return;
    }
    const auto path = audioFile.wstring();
    std::wstring command = L"open \"" + path + L"\" type sequencer alias chips_native_music";
    if (!sendMci(command)) return;
    midiOpen_ = true;
    // Durations are calculated from the supplied SMF tempo maps. Keep this
    // timer as a fallback because some Windows MCI drivers report stale mode
    // and position values after a sequence reaches its end.
    midiTrackLengthMs_ = (levelNumber % 2 == 0) ? 45517U : 36461U;
    sendMci(L"set chips_native_music time format milliseconds");
    SDL_Log("Loaded MIDI background track length: %llu ms",
            static_cast<unsigned long long>(midiTrackLengthMs_));
    updateMusicGain();
#elif defined(__APPLE__)
    stopMusic();
    const auto filename = (levelNumber % 2 == 0) ? "CHIP02.MID" : "CHIP01.MID";
    const auto audioFile = resourceRoot_ / "assets" / "audio" / filename;
    if (!std::filesystem::is_regular_file(audioFile)) {
        SDL_Log("Background MIDI file is missing: %s", pathToUtf8(audioFile).c_str());
        return;
    }

    const auto pathUtf8 = pathToUtf8(audioFile);
    CFURLRef fileUrl = CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault, reinterpret_cast<const UInt8*>(pathUtf8.data()),
        static_cast<CFIndex>(pathUtf8.size()), false);
    if (!fileUrl) {
        SDL_Log("Could not create a file URL for background MIDI: %s", pathUtf8.c_str());
        return;
    }

    OSStatus status = NewMusicSequence(&musicSequence_);
    if (status == noErr)
        status = MusicSequenceFileLoad(musicSequence_, fileUrl, kMusicSequenceFile_MIDIType,
                                       kMusicSequenceLoadSMF_ChannelsToTracks);
    CFRelease(fileUrl);
    if (status != noErr) {
        SDL_Log("Could not load background MIDI %s (AudioToolbox error %d)",
                pathUtf8.c_str(), static_cast<int>(status));
        stopMusic();
        return;
    }

    UInt32 trackCount{};
    status = MusicSequenceGetTrackCount(musicSequence_, &trackCount);
    if (status == noErr) {
        for (UInt32 index = 0; index < trackCount; ++index) {
            MusicTrack track{};
            MusicTimeStamp trackLength{};
            UInt32 propertySize = sizeof(trackLength);
            status = MusicSequenceGetIndTrack(musicSequence_, index, &track);
            if (status != noErr) break;
            status = MusicTrackGetProperty(track, kSequenceTrackProperty_TrackLength,
                                           &trackLength, &propertySize);
            if (status != noErr) break;
            midiTrackLengthBeats_ = std::max(midiTrackLengthBeats_, trackLength);
        }
    }
    if (status == noErr && midiTrackLengthBeats_ <= 0.0) {
        SDL_Log("Background MIDI %s has no playable track length", filename);
        stopMusic();
        return;
    }

    if (status == noErr) status = MusicSequenceGetAUGraph(musicSequence_, &musicGraph_);
    if (status == noErr) status = AUGraphOpen(musicGraph_);
    if (status == noErr) status = AUGraphInitialize(musicGraph_);
    if (status == noErr) {
        UInt32 nodeCount{};
        status = AUGraphGetNodeCount(musicGraph_, &nodeCount);
        for (UInt32 index = 0; status == noErr && index < nodeCount; ++index) {
            AUNode node{};
            AudioComponentDescription description{};
            status = AUGraphGetIndNode(musicGraph_, index, &node);
            if (status == noErr) status = AUGraphNodeInfo(musicGraph_, node, &description,
                                                          &musicSynth_);
            if (status == noErr && description.componentType == kAudioUnitType_MusicDevice)
                break;
            musicSynth_ = nullptr;
        }
        if (status == noErr && !musicSynth_) status = kAudioToolboxError_NoTrackDestination;
    }
    if (status == noErr) status = NewMusicPlayer(&musicPlayer_);
    if (status == noErr) status = MusicPlayerSetSequence(musicPlayer_, musicSequence_);
    if (status == noErr) status = AUGraphStart(musicGraph_);
    if (status != noErr) {
        SDL_Log("Could not initialize macOS MIDI playback (AudioToolbox error %d)",
                static_cast<int>(status));
        stopMusic();
        return;
    }

    midiOpen_ = true;
    SDL_Log("Loaded macOS background MIDI track %s", filename);
    updateMusicGain();
#else
    (void)levelNumber;
#endif
}

void AudioSystem::stopMusic() {
#ifdef _WIN32
    if (midiOpen_) {
        if (midiPlaying_) sendMci(L"stop chips_native_music");
        sendMci(L"close chips_native_music");
        midiOpen_ = false;
        midiPlaying_ = false;
        midiTrackLengthMs_ = 0;
    }
#elif defined(__APPLE__)
    if (musicPlayer_) {
        if (midiPlaying_) MusicPlayerStop(musicPlayer_);
        DisposeMusicPlayer(musicPlayer_);
        musicPlayer_ = nullptr;
    }
    if (musicGraph_) {
        AUGraphStop(musicGraph_);
        AUGraphUninitialize(musicGraph_);
        AUGraphClose(musicGraph_);
    }
    if (musicSequence_) {
        MusicSequenceSetAUGraph(musicSequence_, nullptr);
        DisposeMusicSequence(musicSequence_);
        musicSequence_ = nullptr;
    }
    if (musicGraph_) {
        DisposeAUGraph(musicGraph_);
        musicGraph_ = nullptr;
    }
    musicSynth_ = nullptr;
    midiTrackLengthBeats_ = 0.0;
    midiOpen_ = false;
    midiPlaying_ = false;
#endif
}

void AudioSystem::update() {
#ifdef _WIN32
    if (!midiOpen_ || musicMuted_ || musicVolume_ <= 0.0F) return;
    const Uint64 now = SDL_GetTicks();
    if (now < nextMidiStatusCheck_) return;
    nextMidiStatusCheck_ = now + 250;

    wchar_t mode[32]{};
    const auto result = mciSendStringW(L"status chips_native_music mode", mode,
                                       static_cast<UINT>(std::size(mode)), nullptr);
    if (result != 0) {
        wchar_t error[256]{};
        mciGetErrorStringW(result, error, static_cast<UINT>(std::size(error)));
        SDL_Log("Could not check MIDI playback status: %ls", error);
        return;
    }
    bool trackEnded = std::wcscmp(mode, L"playing") != 0;
    if (!trackEnded && midiTrackLengthMs_ > 0) {
        Uint64 position{};
        if (queryMciNumber(L"status chips_native_music position", position) &&
            position >= midiTrackLengthMs_)
            trackEnded = true;
        const Uint64 elapsed = now - midiTrackStartedAt_;
        if (elapsed >= midiTrackLengthMs_ + 100)
            trackEnded = true;
    }
    if (trackEnded) {
        sendMci(L"stop chips_native_music");
        if (playMciSequence()) {
            midiPlaying_ = true;
            midiTrackStartedAt_ = now;
        } else {
            midiPlaying_ = false;
            SDL_Log("Unable to restart the level's MIDI background music.");
        }
    }
#elif defined(__APPLE__)
    if (!midiOpen_ || !midiPlaying_ || !musicPlayer_) return;
    MusicTimeStamp currentBeat{};
    if (MusicPlayerGetTime(musicPlayer_, &currentBeat) == noErr &&
        currentBeat >= midiTrackLengthBeats_) {
        const auto seekStatus = MusicPlayerSetTime(musicPlayer_, 0.0);
        const auto startStatus = seekStatus == noErr ? MusicPlayerStart(musicPlayer_) : seekStatus;
        if (startStatus != noErr)
            SDL_Log("Could not loop macOS background MIDI (AudioToolbox error %d)",
                    static_cast<int>(startStatus));
    }
#endif
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
}

void AudioSystem::setMusicMuted(bool muted) {
    musicMuted_ = muted;
    updateMusicGain();
}

void AudioSystem::updateGain() {
    if (stream_ && !SDL_SetAudioStreamGain(stream_, muted_ ? 0.0F : volume_))
        SDL_Log("Could not update sound-effect volume: %s", SDL_GetError());
}

void AudioSystem::updateMusicGain() {
#ifdef _WIN32
    if (!midiOpen_) return;
    const auto volume = musicMuted_ ? 0 : static_cast<int>(musicVolume_ * 1000.0F);
    sendMci(L"setaudio chips_native_music volume to " + std::to_wstring(volume));
    if (musicMuted_ || musicVolume_ <= 0.0F) {
        if (midiPlaying_) sendMci(L"stop chips_native_music");
        midiPlaying_ = false;
    } else if (!midiPlaying_) {
        if (!playMciSequence()) {
            SDL_Log("Unable to start the level's MIDI background music.");
            sendMci(L"close chips_native_music");
            midiOpen_ = false;
        } else {
            midiPlaying_ = true;
            nextMidiStatusCheck_ = SDL_GetTicks() + 250;
            midiTrackStartedAt_ = SDL_GetTicks();
        }
    }
#elif defined(__APPLE__)
    if (!midiOpen_ || !musicPlayer_) return;
    const bool shouldPlay = !musicMuted_ && musicVolume_ > 0.0F;
    if (musicSynth_) {
        const auto linearVolume = shouldPlay ? musicVolume_ : 0.0F;
        const auto decibels = linearVolume > 0.0F ? 20.0F * std::log10(linearVolume) : -120.0F;
        const auto status = AudioUnitSetParameter(musicSynth_, kMusicDeviceParam_Volume,
                                                  kAudioUnitScope_Global, 0, decibels, 0);
        if (status != noErr)
            SDL_Log("Could not set macOS background music volume (AudioToolbox error %d)",
                    static_cast<int>(status));
    }
    if (shouldPlay && !midiPlaying_) {
        const auto seekStatus = MusicPlayerSetTime(musicPlayer_, 0.0);
        const auto startStatus = seekStatus == noErr ? MusicPlayerStart(musicPlayer_) : seekStatus;
        if (startStatus == noErr) midiPlaying_ = true;
        else SDL_Log("Could not start macOS background MIDI (AudioToolbox error %d)",
                     static_cast<int>(startStatus));
    } else if (!shouldPlay && midiPlaying_) {
        MusicPlayerStop(musicPlayer_);
        midiPlaying_ = false;
    }
#endif
}
}
