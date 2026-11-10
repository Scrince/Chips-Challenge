#pragma once

#include "engine.hpp"

#include <cstdint>
#include <vector>

namespace chips {

enum class ReplayOperation : std::uint8_t { queueMove, pressMove, advanceTicks, setMouseTarget, settleInput };

struct ReplayCommand {
    ReplayOperation operation{ReplayOperation::advanceTicks};
    Direction direction{Direction::north};
    unsigned ticks{};
    int targetPosition{};

    [[nodiscard]] static ReplayCommand move(Direction direction);
    [[nodiscard]] static ReplayCommand press(Direction direction);
    [[nodiscard]] static ReplayCommand advance(unsigned ticks = 1);
    [[nodiscard]] static ReplayCommand settle(unsigned maxTicks = 128);
    [[nodiscard]] static ReplayCommand mouseTarget(int position);
};

struct ReplayScript {
    std::uint16_t levelNumber{};
    std::uint32_t randomSeed{1};
    std::uint32_t initialGameTicks{};
    std::vector<ReplayCommand> commands;
};

// Stable, compact description of a state after a replay command. The digest
// covers dynamic board layers, inventory, entities, links, timers, and RNG.
struct StateSnapshot {
    std::uint64_t digest{};
    std::uint32_t gameTicks{};
    std::uint16_t levelNumber{};
    int playerPosition{};
    unsigned chipsLeft{};
    unsigned secondsLeft{};
    Outcome outcome{Outcome::playing};
    DeathCause deathCause{DeathCause::none};
};

[[nodiscard]] StateSnapshot snapshot(const GameState& game);
[[nodiscard]] std::vector<StateSnapshot> runReplay(const Level& level,
                                                   const ReplayScript& script);

} // namespace chips
