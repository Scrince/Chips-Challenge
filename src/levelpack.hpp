#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace chips {
inline constexpr int kBoardWidth = 32;
inline constexpr int kBoardHeight = 32;
inline constexpr int kBoardCells = kBoardWidth * kBoardHeight;
inline constexpr std::size_t kCampaignLevelCount = 149;

struct OptionalField {
    std::uint8_t id{};
    std::vector<std::uint8_t> raw;
};

struct Level {
    std::uint16_t number{};
    std::uint16_t timeLimit{};
    std::uint16_t chipsRequired{};
    std::uint16_t mapType{};
    std::array<std::uint8_t, kBoardCells> top{};
    std::array<std::uint8_t, kBoardCells> bottom{};
    std::vector<OptionalField> fields;
    std::string title;
    std::string hint;
    std::string password;

    [[nodiscard]] std::uint8_t fieldTile(int x, int y) const;
};

class LevelPackError final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

[[nodiscard]] std::vector<Level> loadLevelPack(const std::filesystem::path& path);
}
