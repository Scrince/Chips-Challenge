#pragma once

#include "levelpack.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace chips {

struct CampaignProfile {
    std::uint16_t highestUnlocked{1};
    std::uint16_t lastSelectedLevel{1};
    std::vector<std::uint8_t> completed = std::vector<std::uint8_t>(kCampaignLevelCount, 0);
    // UINT32_MAX means no timed completion has been recorded; zero is valid.
    std::vector<std::uint32_t> bestTimes =
        std::vector<std::uint32_t>(kCampaignLevelCount, 0xFFFFFFFFU);
    std::uint8_t effectsVolumePercent{65};
    bool effectsMuted{};
    std::uint8_t musicVolumePercent{20};
    bool musicMuted{};
    // Presentation scale percent; this never changes simulation coordinates or timing.
    std::uint16_t uiScalePercent{100};
    // Disable presentation-only movement interpolation; simulation timing is unchanged.
    bool reducedMotion{};
    bool fullscreen{};
    std::uint16_t windowWidth{1280};
    std::uint16_t windowHeight{820};
    // SDL scancode values for W, D, S, A (north, east, south, west).
    // Arrow keys remain available as fixed alternate movement controls.
    std::array<std::uint16_t, 4> movementBindings{26, 7, 22, 4};
};

[[nodiscard]] CampaignProfile loadProfile(const std::filesystem::path& path,
                                          std::string* warning = nullptr,
                                          std::size_t progressSlots = kCampaignLevelCount);
[[nodiscard]] bool recordLevelSelection(CampaignProfile& profile,
                                        std::size_t levelIndex,
                                        std::size_t campaignLevelCount);
// Record the persistent campaign effects of a completed level. Returns false
// when the index or profile storage cannot represent this level.
[[nodiscard]] bool recordLevelCompletion(CampaignProfile& profile,
                                         std::size_t levelIndex,
                                         std::size_t campaignLevelCount,
                                         bool timed,
                                         std::uint32_t elapsedSeconds);
void saveProfile(const std::filesystem::path& path, const CampaignProfile& profile);
}
