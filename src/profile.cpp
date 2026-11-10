#include "profile.hpp"
#include "platform_file.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace chips {
namespace {
constexpr char kMagic[] = "CCPROF01";
constexpr std::uint16_t kVersion = 8;
constexpr std::size_t kVersion1Size = 8 + 2 + 2 + 2 + kCampaignLevelCount + kCampaignLevelCount * 4;
constexpr std::size_t kVersion2Size = kVersion1Size + 2;
constexpr std::size_t kVersion3Size = kVersion2Size + 4 * 2;
constexpr std::size_t kExpectedSize = kVersion3Size + 2;
constexpr std::size_t kVersion5FixedSize = 8 + 2 + 2 + 2 + 2 + 2 + 8 + 2;
constexpr std::size_t kVersion6FixedSize = kVersion5FixedSize + 1;
constexpr std::size_t kVersion7FixedSize = kVersion6FixedSize + 5;
constexpr std::size_t kVersion8FixedSize = kVersion7FixedSize + 2;
constexpr std::size_t kMaximumSupportedProfileSize = kVersion8FixedSize + 0xFFFFU * 5U;

void appendWord(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value & 0xFF));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8));
}

void appendDword(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8)
        bytes.push_back(static_cast<std::uint8_t>(value >> shift));
}

std::uint16_t wordAt(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset] |
        (static_cast<std::uint16_t>(bytes[offset + 1]) << 8));
}

std::uint32_t dwordAt(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
        (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
        (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

CampaignProfile defaults(std::string* warning, const std::string& message, std::size_t slots) {
    if (warning) *warning = message;
    CampaignProfile profile;
    profile.completed.assign(slots, 0);
    profile.bestTimes.assign(slots, 0xFFFFFFFFU);
    return profile;
}
}

CampaignProfile loadProfile(const std::filesystem::path& path, std::string* warning,
                            std::size_t progressSlots) {
    if (warning) warning->clear();
    if (progressSlots == 0 || progressSlots > 0xFFFFU)
        throw std::invalid_argument("Profile progress slot count must be in the range 1..65535");
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::error_code error;
        if (!std::filesystem::exists(path, error) && !error)
            return defaults(nullptr, {}, progressSlots);
        return defaults(warning, "Could not read profile; starting with fresh progress.", progressSlots);
    }
    file.seekg(0, std::ios::end);
    const auto end = file.tellg();
    if (end == std::streampos(-1))
        return defaults(warning, "Could not determine profile size; starting with fresh progress.", progressSlots);
    const auto byteCount = static_cast<std::streamoff>(end);
    if (byteCount < 0 || static_cast<std::uintmax_t>(byteCount) > kMaximumSupportedProfileSize)
        return defaults(warning, "Profile exceeds the supported size; starting with fresh progress.", progressSlots);
    if (byteCount < 10)
        return defaults(warning, "Profile has an invalid size; starting with fresh progress.", progressSlots);

    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(byteCount));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!file || file.gcount() != static_cast<std::streamsize>(bytes.size()))
        return defaults(warning, "Could not read complete profile; starting with fresh progress.", progressSlots);
    if (bytes.size() < 10)
        return defaults(warning, "Profile has an invalid size; starting with fresh progress.", progressSlots);
    for (std::size_t i = 0; i < sizeof(kMagic) - 1; ++i) {
        if (bytes[i] != static_cast<std::uint8_t>(kMagic[i]))
            return defaults(warning, "Profile signature is invalid; starting with fresh progress.", progressSlots);
    }
    const auto version = wordAt(bytes, 8);
    if ((version == 1 && bytes.size() != kVersion1Size) ||
        (version == 2 && bytes.size() != kVersion2Size) ||
        (version == 3 && bytes.size() != kVersion3Size) ||
        (version == 4 && bytes.size() != kExpectedSize) ||
        (version == 5 && bytes.size() < kVersion5FixedSize) ||
        (version == 6 && bytes.size() < kVersion6FixedSize) ||
        (version == 7 && bytes.size() < kVersion7FixedSize) ||
        (version == 8 && bytes.size() < kVersion8FixedSize) ||
        (version < 1 || version > kVersion))
        return defaults(warning, "Profile version or size is unsupported; starting with fresh progress.", progressSlots);

    const std::size_t storedSlots = version >= 5 ? wordAt(bytes, 14) : kCampaignLevelCount;
    const std::size_t storedPrefix = version >= 5 ? 16 : 14;
    const std::size_t storedSize = version >= 5
        ? (version >= 8 ? kVersion8FixedSize : version >= 7 ? kVersion7FixedSize :
           version >= 6 ? kVersion6FixedSize : kVersion5FixedSize) + storedSlots * 5 :
          (version == 1 ? kVersion1Size : version == 2 ? kVersion2Size :
           version == 3 ? kVersion3Size : kExpectedSize);
    if (storedSlots == 0 || storedSlots > 0xFFFFU || bytes.size() != storedSize)
        return defaults(warning, "Profile progress data has an invalid size; starting with fresh progress.", progressSlots);

    CampaignProfile profile;
    profile.completed.assign(progressSlots, 0);
    profile.bestTimes.assign(progressSlots, 0xFFFFFFFFU);
    profile.highestUnlocked = wordAt(bytes, 10);
    profile.lastSelectedLevel = wordAt(bytes, 12);
    if (profile.highestUnlocked == 0 || profile.highestUnlocked > storedSlots ||
        profile.lastSelectedLevel == 0 || profile.lastSelectedLevel > storedSlots)
        return defaults(warning, "Profile progress is out of range; starting with fresh progress.", progressSlots);
    std::size_t offset = storedPrefix;
    const auto overlappingSlots = std::min(storedSlots, progressSlots);
    for (std::size_t i = 0; i < storedSlots; ++i) {
        if (bytes[offset] > 1)
            return defaults(warning, "Profile completion data is invalid; starting with fresh progress.", progressSlots);
        if (i < overlappingSlots) profile.completed[i] = bytes[offset];
        ++offset;
    }
    for (std::size_t i = 0; i < storedSlots; ++i) {
        const auto time = dwordAt(bytes, offset);
        if (i < overlappingSlots) profile.bestTimes[i] = time;
        offset += 4;
    }
    if (version >= 5 && offset != 16 + storedSlots * 5)
        return defaults(warning, "Profile progress data is truncated; starting with fresh progress.", progressSlots);
    if (version >= 2) {
        profile.effectsVolumePercent = bytes[offset++];
        if (profile.effectsVolumePercent > 100 || bytes[offset] > 1)
            return defaults(warning, "Profile audio settings are invalid; starting with fresh progress.", progressSlots);
        profile.effectsMuted = bytes[offset] != 0;
        ++offset;
    }
    if (version >= 3) {
        for (auto& binding : profile.movementBindings) {
            binding = wordAt(bytes, offset);
            offset += 2;
            if (binding == 0 || binding >= 512)
                return defaults(warning, "Profile movement bindings are invalid; starting with fresh progress.", progressSlots);
        }
        for (std::size_t i = 0; i < profile.movementBindings.size(); ++i) {
            for (std::size_t j = i + 1; j < profile.movementBindings.size(); ++j) {
                if (profile.movementBindings[i] == profile.movementBindings[j])
                    return defaults(warning, "Profile movement bindings are duplicated; starting with fresh progress.", progressSlots);
            }
        }
    }
    if (version >= 4) {
        profile.uiScalePercent = wordAt(bytes, offset);
        offset += 2;
        if (profile.uiScalePercent < 100 || profile.uiScalePercent > 200 ||
            (profile.uiScalePercent - 100) % 25 != 0)
            return defaults(warning, "Profile display scale is invalid; starting with fresh progress.", progressSlots);
    }
    if (version >= 6) {
        if (bytes[offset] > 1)
            return defaults(warning, "Profile accessibility settings are invalid; starting with fresh progress.", progressSlots);
        profile.reducedMotion = bytes[offset] != 0;
        ++offset;
    }
    if (version >= 7) {
        if (bytes[offset] > 1)
            return defaults(warning, "Profile display mode is invalid; starting with fresh progress.", progressSlots);
        profile.fullscreen = bytes[offset++] != 0;
        profile.windowWidth = wordAt(bytes, offset);
        offset += 2;
        profile.windowHeight = wordAt(bytes, offset);
        offset += 2;
        if (profile.windowWidth < 640 || profile.windowWidth > 8192 ||
            profile.windowHeight < 480 || profile.windowHeight > 8192)
            return defaults(warning, "Profile window size is invalid; starting with fresh progress.", progressSlots);
    }
    if (version >= 8) {
        profile.musicVolumePercent = bytes[offset++];
        if (profile.musicVolumePercent > 100 || bytes[offset] > 1)
            return defaults(warning, "Profile music settings are invalid; starting with fresh progress.", progressSlots);
        profile.musicMuted = bytes[offset++] != 0;
    }
    profile.highestUnlocked = static_cast<std::uint16_t>(std::min<std::size_t>(profile.highestUnlocked, progressSlots));
    profile.lastSelectedLevel = static_cast<std::uint16_t>(std::min<std::size_t>(profile.lastSelectedLevel, progressSlots));
    return profile;
}

bool recordLevelSelection(CampaignProfile& profile, std::size_t levelIndex,
                          std::size_t campaignLevelCount) {
    if (campaignLevelCount == 0 || campaignLevelCount > 0xFFFFU ||
        levelIndex >= campaignLevelCount || levelIndex >= profile.completed.size())
        return false;
    profile.lastSelectedLevel = static_cast<std::uint16_t>(levelIndex + 1);
    return true;
}

bool recordLevelCompletion(CampaignProfile& profile, std::size_t levelIndex,
                           std::size_t campaignLevelCount, bool timed,
                           std::uint32_t elapsedSeconds) {
    if (campaignLevelCount == 0 || campaignLevelCount > 0xFFFFU ||
        levelIndex >= campaignLevelCount ||
        levelIndex >= profile.completed.size() || levelIndex >= profile.bestTimes.size())
        return false;

    profile.completed[levelIndex] = 1;
    const auto nextUnlock = std::min(levelIndex + 2, campaignLevelCount);
    profile.highestUnlocked = static_cast<std::uint16_t>(
        std::max<std::size_t>(profile.highestUnlocked, nextUnlock));
    if (timed) {
        auto& best = profile.bestTimes[levelIndex];
        if (best == 0xFFFFFFFFU || elapsedSeconds < best) best = elapsedSeconds;
    }
    return true;
}

void saveProfile(const std::filesystem::path& path, const CampaignProfile& profile) {
    if (profile.completed.empty() || profile.completed.size() > 0xFFFFU ||
        profile.bestTimes.size() != profile.completed.size() ||
        profile.highestUnlocked == 0 || profile.highestUnlocked > profile.completed.size() ||
        profile.lastSelectedLevel == 0 || profile.lastSelectedLevel > profile.completed.size() ||
        profile.effectsVolumePercent > 100 || profile.musicVolumePercent > 100 || profile.uiScalePercent < 100 ||
        profile.uiScalePercent > 200 || (profile.uiScalePercent - 100) % 25 != 0 ||
        profile.windowWidth < 640 || profile.windowWidth > 8192 ||
        profile.windowHeight < 480 || profile.windowHeight > 8192)
        throw std::runtime_error("Campaign profile contains an out-of-range level, audio, display, or movement setting");
    if (std::any_of(profile.completed.begin(), profile.completed.end(),
                    [](std::uint8_t value) { return value > 1; }))
        throw std::runtime_error("Profile completion data must contain only zero or one");
    for (std::size_t i = 0; i < profile.movementBindings.size(); ++i) {
        if (profile.movementBindings[i] == 0 || profile.movementBindings[i] >= 512)
            throw std::runtime_error("Campaign profile contains an invalid movement binding");
        for (std::size_t j = i + 1; j < profile.movementBindings.size(); ++j) {
            if (profile.movementBindings[i] == profile.movementBindings[j])
                throw std::runtime_error("Campaign profile contains duplicate movement bindings");
        }
    }
    std::vector<std::uint8_t> bytes;
    bytes.reserve(kVersion8FixedSize + profile.completed.size() * 5);
    for (std::size_t i = 0; i < sizeof(kMagic) - 1; ++i)
        bytes.push_back(static_cast<std::uint8_t>(kMagic[i]));
    appendWord(bytes, kVersion);
    appendWord(bytes, profile.highestUnlocked);
    appendWord(bytes, profile.lastSelectedLevel);
    appendWord(bytes, static_cast<std::uint16_t>(profile.completed.size()));
    for (const bool completed : profile.completed) bytes.push_back(completed ? 1 : 0);
    for (const auto time : profile.bestTimes) appendDword(bytes, time);
    bytes.push_back(profile.effectsVolumePercent);
    bytes.push_back(profile.effectsMuted ? 1 : 0);
    for (const auto binding : profile.movementBindings) appendWord(bytes, binding);
    appendWord(bytes, profile.uiScalePercent);
    bytes.push_back(profile.reducedMotion ? 1 : 0);
    bytes.push_back(profile.fullscreen ? 1 : 0);
    appendWord(bytes, profile.windowWidth);
    appendWord(bytes, profile.windowHeight);
    bytes.push_back(profile.musicVolumePercent);
    bytes.push_back(profile.musicMuted ? 1 : 0);

    std::error_code error;
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) throw std::runtime_error("Cannot create profile directory: " + error.message());
    }
    auto temporary = path;
    temporary += ".tmp";
    {
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        if (!file) throw std::runtime_error("Cannot create temporary campaign profile");
        file.write(reinterpret_cast<const char*>(bytes.data()),
                   static_cast<std::streamsize>(bytes.size()));
        file.flush();
        if (!file) throw std::runtime_error("Could not write campaign profile");
    }
    if (const auto replaceError = platform::replaceFileAtomically(temporary, path); replaceError) {
        std::filesystem::remove(temporary, error);
        throw std::runtime_error("Could not atomically replace campaign profile: " + replaceError.message());
    }
}
}
