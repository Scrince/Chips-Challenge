#pragma once

#include "levelpack.hpp"

#include <optional>
#include <string_view>
#include <vector>

namespace chips {

// Returns the first campaign level whose password matches after applying the
// game's case-insensitive, whitespace-insensitive input rules.
[[nodiscard]] std::optional<std::size_t> findLevelByPassword(
    std::string_view input, const std::vector<Level>& levels);

// Compares an entered password to one specific level, preserving campaign
// passwords that are intentionally shared by multiple levels.
[[nodiscard]] bool passwordMatches(std::string_view input, std::string_view password);

} // namespace chips
