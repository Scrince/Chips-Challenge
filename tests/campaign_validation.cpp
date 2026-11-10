#include "engine.hpp"
#include "levelpack.hpp"
#include "replay.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <utility>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

namespace {
std::uint16_t readLittleWord(const std::uint8_t* bytes) {
    return static_cast<std::uint16_t>(bytes[0] |
        (static_cast<std::uint16_t>(bytes[1]) << 8U));
}

struct ExpectedTrapLink {
    int button{};
    int trap{};
    bool closed{};
};

struct ExpectedCloneLink {
    int button{};
    int machine{};
    std::uint8_t prototype{};
};

chips::ReplayScript makeCampaignReplay(std::uint16_t levelNumber) {
    chips::ReplayScript script;
    script.levelNumber = levelNumber;
    script.randomSeed = 0xC0FFEEU + levelNumber;
    script.commands.reserve(256);

    // A repeatable mixed input stream exercises player turns, creature turns,
    // timers, and slipping entities without depending on wall-clock timing.
    std::uint32_t sequence = 0x9E3779B9U ^ levelNumber;
    constexpr chips::Direction directions[]{chips::Direction::north, chips::Direction::east,
                                             chips::Direction::south, chips::Direction::west};
    for (unsigned step = 0; step < 128; ++step) {
        sequence = sequence * 1664525U + 1013904223U;
        const auto direction = directions[(sequence >> 24U) & 3U];
        script.commands.push_back((sequence & 0x3U) == 0
            ? chips::ReplayCommand::press(direction)
            : chips::ReplayCommand::move(direction));
        script.commands.push_back(chips::ReplayCommand::advance(8));
    }
    return script;
}

bool validRuntimeReferences(const chips::GameState& state, std::string& reason) {
    std::array<bool, chips::kBoardCells> occupiedByCreature{};
    const auto validPosition = [](int position) {
        return position >= 0 && position < chips::kBoardCells;
    };
    if (!validPosition(state.playerPosition)) {
        reason = "player position is outside the board";
        return false;
    }
    for (std::size_t i = 0; i < state.creatures.size(); ++i) {
        const auto& creature = state.creatures[i];
        if (!validPosition(creature.position)) {
            reason = "creature " + std::to_string(i) + " position is outside the board";
            return false;
        }
        if (creature.tile < 0x40 || creature.tile > 0x63) {
            reason = "creature " + std::to_string(i) + " has a non-creature tile ID";
            return false;
        }
        if (creature.tankDirection < -2 || creature.tankDirection > 3 ||
            creature.movementDirection < -1 || creature.movementDirection > 3) {
            reason = "creature " + std::to_string(i) + " has an invalid movement vector";
            return false;
        }
        if (!creature.alive) continue;
        const auto position = static_cast<std::size_t>(creature.position);
        if (occupiedByCreature[position]) {
            reason = "multiple live creatures occupy cell " + std::to_string(creature.position);
            return false;
        }
        occupiedByCreature[position] = true;
        if (state.surface(creature.position) != creature.tile) {
            reason = "live creature " + std::to_string(i) +
                " has a visible sprite that differs from its logical position or facing";
            return false;
        }
    }
    for (std::size_t i = 0; i < state.blockDirections.size(); ++i) {
        if (state.blockDirections[i] < -1 || state.blockDirections[i] > 3) {
            reason = "block at board index " + std::to_string(i) + " has an invalid direction";
            return false;
        }
    }
    for (std::size_t i = 0; i < state.slipOrder.size(); ++i) {
        const auto& slipper = state.slipOrder[i];
        if (slipper.kind == chips::SlipperKind::block) {
            if (!validPosition(slipper.entity) ||
                state.top[static_cast<std::size_t>(slipper.entity)] != 0x0A) {
                reason = "slipper " + std::to_string(i) + " references a missing block";
                return false;
            }
        } else if (slipper.kind == chips::SlipperKind::creature) {
            if (slipper.entity < 0 ||
                static_cast<std::size_t>(slipper.entity) >= state.creatures.size() ||
                !state.creatures[static_cast<std::size_t>(slipper.entity)].alive) {
                reason = "slipper " + std::to_string(i) + " references a missing creature";
                return false;
            }
        } else {
            reason = "slipper " + std::to_string(i) + " has an unknown entity kind";
            return false;
        }
    }
    for (std::size_t i = 0; i < state.traps.size(); ++i) {
        if (!validPosition(state.traps[i].buttonPosition) || !validPosition(state.traps[i].trapPosition)) {
            reason = "trap link " + std::to_string(i) + " is outside the board";
            return false;
        }
    }
    for (std::size_t i = 0; i < state.cloneLinks.size(); ++i) {
        if (!validPosition(state.cloneLinks[i].buttonPosition) ||
            !validPosition(state.cloneLinks[i].machinePosition)) {
            reason = "clone link " + std::to_string(i) + " is outside the board";
            return false;
        }
    }
    for (const auto position : state.teleports) {
        if (!validPosition(position)) {
            reason = "teleporter index is outside the board";
            return false;
        }
    }
    for (const auto position : state.togglePositions) {
        if (!validPosition(position)) {
            reason = "toggle index is outside the board";
            return false;
        }
    }
    return true;
}

bool validInitialSpritePlacement(const chips::Level& level, const chips::GameState& state,
                                std::string& reason) {
    const auto facingFor = [](std::uint8_t tile) {
        switch (tile) {
            case 0x6C: return chips::Direction::north;
            case 0x6D: return chips::Direction::west;
            case 0x6E: return chips::Direction::south;
            default: return chips::Direction::east;
        }
    };

    int expectedPlayer = -1;
    std::uint8_t playerTile = 0;
    for (int position = 0; position < chips::kBoardCells; ++position) {
        const auto index = static_cast<std::size_t>(position);
        const auto upper = level.top[index];
        if (upper >= 0x6C && upper <= 0x6F) {
            if (expectedPlayer >= 0) {
                reason = "level has multiple canonical player sprites";
                return false;
            }
            expectedPlayer = position;
            playerTile = upper;
        }
    }

    std::vector<std::pair<int, std::uint8_t>> expectedCreatures;
    for (const auto& field : level.fields) {
        if (field.id != 10) continue;
        if (field.raw.size() % 2 != 0) {
            reason = "initial-creature metadata has an incomplete coordinate";
            return false;
        }
        for (std::size_t offset = 0; offset < field.raw.size(); offset += 2) {
            if (field.raw[offset] >= chips::kBoardWidth ||
                field.raw[offset + 1] >= chips::kBoardHeight) {
                reason = "initial-creature coordinate is outside the board";
                return false;
            }
            const int position = static_cast<int>(field.raw[offset + 1] * chips::kBoardWidth +
                                                  field.raw[offset]);
            const auto index = static_cast<std::size_t>(position);
            const auto tile = level.top[index];
            if (tile < 0x40 || tile > 0x63 || level.bottom[index] == 0x31) continue;
            expectedCreatures.emplace_back(position, tile);
            const auto underlay = level.bottom[index];
            if (underlay >= 0x6C && underlay <= 0x6F) {
                if (expectedPlayer >= 0) {
                    reason = "level has multiple canonical player sprites";
                    return false;
                }
                expectedPlayer = position;
                playerTile = underlay;
            }
        }
    }

    if (expectedPlayer < 0 || state.playerPosition != expectedPlayer ||
        state.facing != facingFor(playerTile)) {
        reason = "player sprite position or initial facing differs from decoded map";
        return false;
    }
    if (state.playerSpriteTile() != playerTile) {
        reason = "rendered player sprite frame differs from decoded directional sprite";
        return false;
    }
    auto expectedUpperLayer = level.top;
    expectedUpperLayer[static_cast<std::size_t>(expectedPlayer)] =
        level.bottom[static_cast<std::size_t>(expectedPlayer)];
    for (const auto& creature : expectedCreatures)
        expectedUpperLayer[static_cast<std::size_t>(creature.first)] =
            level.bottom[static_cast<std::size_t>(creature.first)];
    if (state.top != expectedUpperLayer || state.bottom != level.bottom) {
        reason = "initialization changed a map tile outside its player/creature sprite overlays";
        return false;
    }
    if (state.creatures.size() != expectedCreatures.size()) {
        reason = "live creature count differs from decoded creature sprites/metadata";
        return false;
    }
    for (std::size_t i = 0; i < expectedCreatures.size(); ++i) {
        const auto& expected = expectedCreatures[i];
        const auto& actual = state.creatures[i];
        if (!actual.alive || actual.position != expected.first || actual.tile != expected.second ||
            state.surface(actual.position) != expected.second) {
            reason = "creature sprite " + std::to_string(i) +
                " differs from its decoded map coordinate, layer, or facing";
            return false;
        }
    }
    return true;
}

bool stressCampaignLevel(const chips::Level& level, std::uint32_t seed,
                         std::uint64_t& steps, std::uint64_t& ticks,
                         std::uint64_t& restarts, std::string& reason) {
    auto first = chips::GameState::start(level, seed & 0xFFFFU, seed);
    auto second = first;
    std::uint32_t sequence = seed ^ 0x9E3779B9U;
    constexpr chips::Direction directions[]{chips::Direction::north, chips::Direction::east,
        chips::Direction::south, chips::Direction::west};
    for (unsigned operation = 0; operation < 256; ++operation) {
        sequence = sequence * 1664525U + 1013904223U;
        const auto direction = directions[(sequence >> 29U) & 3U];
        const unsigned advance = 1U + ((sequence >> 20U) & 3U);
        if ((sequence & 0x3U) == 0) {
            (void)first.pressMove(direction);
            (void)second.pressMove(direction);
        } else {
            first.queueMove(direction);
            second.queueMove(direction);
        }
        first.advanceTicks(advance);
        second.advanceTicks(advance);
        ++steps;
        ticks += advance;

        if (first.outcome != second.outcome || chips::snapshot(first).digest != chips::snapshot(second).digest) {
            reason = "restarted stress replay diverged after operation " + std::to_string(operation + 1);
            return false;
        }
        if (!validRuntimeReferences(first, reason)) return false;
        if (!validRuntimeReferences(second, reason)) return false;

        if (first.outcome != chips::Outcome::playing) {
            first.restart();
            second.restart();
            ++restarts;
            if (chips::snapshot(first).digest != chips::snapshot(second).digest) {
                reason = "restart state diverged after operation " + std::to_string(operation + 1);
                return false;
            }
        }
    }
    return true;
}
}

int main() {
    try {
        const auto levels = chips::loadLevelPack(CHIPS_TEST_PACK_PATH);
        if (levels.size() != 149) {
            std::cerr << "Expected 149 campaign levels, parsed " << levels.size() << '\n';
            return EXIT_FAILURE;
        }

        std::size_t hintMetadataCount = 0;
        std::size_t hintTileLevelCount = 0;
        for (const auto& level : levels) {
            const auto hintFieldCount = std::count_if(level.fields.begin(), level.fields.end(),
                [](const chips::OptionalField& field) { return field.id == 7; });
            const bool hasHintTile =
                std::find(level.top.begin(), level.top.end(), 0x2F) != level.top.end() ||
                std::find(level.bottom.begin(), level.bottom.end(), 0x2F) != level.bottom.end();
            if (hintFieldCount > 1 || hasHintTile != (hintFieldCount == 1)) {
                std::cerr << "Level " << level.number
                          << " has a mismatched Hint tile and original hint field\n";
                return EXIT_FAILURE;
            }
            if (!hasHintTile) continue;

            ++hintTileLevelCount;
            ++hintMetadataCount;
            const auto field = std::find_if(level.fields.begin(), level.fields.end(),
                [](const chips::OptionalField& value) { return value.id == 7; });
            std::string sourceHint;
            for (const auto byte : field->raw) {
                if (byte == 0) break;
                if (byte >= 0x80) {
                    std::cerr << "Level " << level.number
                              << " has a non-ASCII hint byte that needs an encoding audit\n";
                    return EXIT_FAILURE;
                }
                sourceHint.push_back(static_cast<char>(byte));
            }
            if (level.hint != sourceHint) {
                std::cerr << "Level " << level.number
                          << " hint text differs from its original field-7 bytes\n";
                return EXIT_FAILURE;
            }
        }
        if (hintMetadataCount != 21 || hintTileLevelCount != 21) {
            std::cerr << "Expected all 21 original hint fields and hint-bearing levels; found "
                      << hintMetadataCount << " fields and " << hintTileLevelCount << " levels\n";
            return EXIT_FAILURE;
        }

        struct ExpectedBootPickup {
            int x;
            int y;
            std::uint8_t tile;
        };
        constexpr std::array<ExpectedBootPickup, 4> lessonThreeBoots{{
            {15, 11, 0x6A}, {12, 14, 0x69}, {19, 14, 0x6B}, {16, 16, 0x68}
        }};
        const auto& lessonThree = levels[2];
        std::size_t bootPickupCount = 0;
        bool bootInLowerLayer = false;
        for (std::size_t position = 0; position < chips::kBoardCells; ++position) {
            if (lessonThree.top[position] >= 0x68 && lessonThree.top[position] <= 0x6B)
                ++bootPickupCount;
            if (lessonThree.bottom[position] >= 0x68 && lessonThree.bottom[position] <= 0x6B)
                bootInLowerLayer = true;
        }
        if (lessonThree.number != 3 || lessonThree.title != "LESSON 3" ||
            bootPickupCount != lessonThreeBoots.size() || bootInLowerLayer) {
            std::cerr << "Level 3 boot pickup count/layer mismatch\n";
            return EXIT_FAILURE;
        }
        for (const auto& pickup : lessonThreeBoots) {
            const auto position = static_cast<std::size_t>(pickup.y * chips::kBoardWidth + pickup.x);
            if (lessonThree.top[position] != pickup.tile) {
                std::cerr << "Level 3 boot tile 0x" << std::hex
                          << static_cast<unsigned>(pickup.tile) << std::dec
                          << " missing from expected map coordinate (" << pickup.x << ','
                          << pickup.y << ")\n";
                return EXIT_FAILURE;
            }
        }
        auto observedLessonThree = chips::GameState::start(lessonThree);
        if (observedLessonThree.playerPosition != 15 * chips::kBoardWidth + 15 ||
            observedLessonThree.pressMove(chips::Direction::east) != chips::MoveResult::moved ||
            observedLessonThree.playerPosition != 15 * chips::kBoardWidth + 16 ||
            observedLessonThree.facing != chips::Direction::east ||
            observedLessonThree.message != "Hint: " + lessonThree.hint ||
            observedLessonThree.chipsLeft != lessonThree.chipsRequired ||
            std::any_of(observedLessonThree.boots.begin(), observedLessonThree.boots.end(),
                        [](bool collected) { return collected; })) {
            std::cerr << "Level 3 source-observed eastward move or Hint tile differs from the campaign\n";
            return EXIT_FAILURE;
        }

        // Replay the observed Level 3 route from its original start: Chip
        // steps east onto Hint, waits through the movement gate, then steps
        // south to collect the Flippers at (16,16).
        auto observedBootPickup = chips::GameState::start(lessonThree);
        if (observedBootPickup.pressMove(chips::Direction::east) != chips::MoveResult::moved ||
            observedBootPickup.playerPosition != 15 * chips::kBoardWidth + 16 ||
            observedBootPickup.message != "Hint: " + lessonThree.hint) {
            std::cerr << "Level 3 eastward Hint move differs from the original route\n";
            return EXIT_FAILURE;
        }
        observedBootPickup.advanceTicks(2);
        if (observedBootPickup.pressMove(chips::Direction::south) != chips::MoveResult::moved ||
            observedBootPickup.playerPosition != 16 * chips::kBoardWidth + 16 ||
            observedBootPickup.playerPickupThisTick != chips::PickupKind::boot ||
            !observedBootPickup.boots[0] ||
            std::any_of(observedBootPickup.boots.begin() + 1, observedBootPickup.boots.end(),
                        [](bool collected) { return collected; }) ||
            observedBootPickup.top[static_cast<std::size_t>(16 * chips::kBoardWidth + 16)] !=
                lessonThree.bottom[static_cast<std::size_t>(16 * chips::kBoardWidth + 16)]) {
            std::cerr << "Level 3 eastward Flipper pickup differs from the original map/action\n";
            return EXIT_FAILURE;
        }

        // Continue the same real-level route west across the curved ice loop.
        // The IceWallSW corner turns Chip north; IceWallNW turns Chip
        // east again, where the floor at (14,13) ends the forced slide.
        auto observedIceSlide = chips::GameState::start(lessonThree);
        constexpr std::array routeToIce{
            chips::Direction::east, chips::Direction::south,
            chips::Direction::west, chips::Direction::west, chips::Direction::west
        };
        bool routeAccepted = true;
        for (const auto direction : routeToIce) {
            if (observedIceSlide.pressMove(direction) != chips::MoveResult::moved) {
                routeAccepted = false;
                break;
            }
            observedIceSlide.advanceTicks(2);
        }
        for (unsigned settleTick = 0; settleTick < 64 && observedIceSlide.sliding &&
             observedIceSlide.outcome == chips::Outcome::playing; ++settleTick)
            observedIceSlide.advanceTicks(1);
        if (!routeAccepted ||
            observedIceSlide.playerPosition != 13 * chips::kBoardWidth + 14 ||
            observedIceSlide.facing != chips::Direction::east ||
            observedIceSlide.floor(observedIceSlide.playerPosition) != 0x00 ||
            observedIceSlide.sliding || observedIceSlide.outcome != chips::Outcome::playing) {
            std::cerr << "Level 3 curved ice route did not settle at the source-observed endpoint (14,13)\n";
            return EXIT_FAILURE;
        }
        while (observedIceSlide.idleTickCount < 2 &&
               observedIceSlide.outcome == chips::Outcome::playing)
            observedIceSlide.advanceTicks(1);
        if (observedIceSlide.facing != chips::Direction::east ||
            observedIceSlide.playerSpriteTile() != 0x6F) {
            std::cerr << "Level 3 Chip turned south before three idle even ticks\n";
            return EXIT_FAILURE;
        }
        while (observedIceSlide.idleTickCount < 3 &&
               observedIceSlide.outcome == chips::Outcome::playing)
            observedIceSlide.advanceTicks(1);
        if (observedIceSlide.facing != chips::Direction::south ||
            observedIceSlide.playerSpriteTile() != 0x6E) {
            std::cerr << "Level 3 idle facing or player sprite differs after three idle even ticks\n";
            return EXIT_FAILURE;
        }

        // Exercise Level 4's actual TankButton and field-10 Tank starts.
        // Place Chip on the decoded adjacent Floor so this isolates the
        // button action without inventing a replacement map or relying on
        // uncontrolled Tank timing during a long route.
        const auto& lessonFour = levels[3];
        constexpr int lessonFourTankButton = 6 * chips::kBoardWidth + 5;
        constexpr int lessonFourApproach = 6 * chips::kBoardWidth + 6;
        auto observedTankButton = chips::GameState::start(lessonFour);
        std::vector<std::size_t> lessonFourTanks;
        for (std::size_t i = 0; i < observedTankButton.creatures.size(); ++i) {
            const auto& creature = observedTankButton.creatures[i];
            if (creature.alive && (creature.tile & 0xFC) == 0x4C)
                lessonFourTanks.push_back(i);
        }
        const int originalPlayerPosition = observedTankButton.playerPosition;
        const auto originalPlayerIndex = static_cast<std::size_t>(originalPlayerPosition);
        const auto approachIndex = static_cast<std::size_t>(lessonFourApproach);
        const auto tankButtonIndex = static_cast<std::size_t>(lessonFourTankButton);
        if (lessonFour.number != 4 || lessonFour.title != "LESSON 4" ||
            lessonFour.top[tankButtonIndex] != 0x28 ||
            lessonFour.top[approachIndex] != 0x00 ||
            lessonFourTanks.size() != 2 ||
            std::any_of(lessonFourTanks.begin(), lessonFourTanks.end(),
                [&observedTankButton](std::size_t i) {
                    return observedTankButton.creatures[i].tile != 0x4C ||
                           observedTankButton.creatures[i].tankDirection != 0;
                })) {
            std::cerr << "Level 4 TankButton, approach Floor, or initial Tank state differs from the campaign\n";
            return EXIT_FAILURE;
        }
        observedTankButton.top[originalPlayerIndex] = lessonFour.bottom[originalPlayerIndex];
        observedTankButton.playerPosition = lessonFourApproach;
        observedTankButton.top[approachIndex] = 0x6E;
        const auto tankButtonMove = observedTankButton.pressMove(chips::Direction::west);
        if (tankButtonMove != chips::MoveResult::moved ||
            observedTankButton.playerPosition != lessonFourTankButton ||
            observedTankButton.top[tankButtonIndex] != lessonFour.top[tankButtonIndex] ||
            observedTankButton.bottom[tankButtonIndex] != 0x28 ||
            observedTankButton.playerSpriteTile() != 0x6D ||
            observedTankButton.message != "Tanks reversed direction.") {
            std::cerr << "Level 4 source-map TankButton activation mismatch: result="
                      << static_cast<unsigned>(tankButtonMove)
                      << " player=" << observedTankButton.playerPosition
                      << " expected=" << lessonFourTankButton
                      << " upper=0x" << std::hex
                      << static_cast<unsigned>(observedTankButton.top[tankButtonIndex])
                      << " lower=0x"
                      << static_cast<unsigned>(observedTankButton.bottom[tankButtonIndex])
                      << " sprite=0x" << static_cast<unsigned>(observedTankButton.playerSpriteTile())
                      << std::dec << " message=\"" << observedTankButton.message << "\"\n";
            return EXIT_FAILURE;
        }
        for (const auto tankIndex : lessonFourTanks) {
            const auto& tank = observedTankButton.creatures[tankIndex];
            if (tank.tile != 0x4E || tank.tankDirection != 2 ||
                tank.movementDirection != 2 ||
                observedTankButton.surface(tank.position) != 0x4E) {
                std::cerr << "Level 4 TankButton failed to reverse a canonical Tank and its movement vector\n";
                return EXIT_FAILURE;
            }
        }
        observedTankButton.advanceTicks(2);
        constexpr std::array<int, 2> lessonFourReversedTankPositions{{
            6 * chips::kBoardWidth + 8, 10 * chips::kBoardWidth + 8
        }};
        std::array<int, 2> actualTankPositions{};
        for (std::size_t i = 0; i < lessonFourTanks.size(); ++i) {
            const auto& tank = observedTankButton.creatures[lessonFourTanks[i]];
            actualTankPositions[i] = tank.position;
            if (tank.tile != 0x4E || tank.tankDirection != 2 ||
                tank.movementDirection != 2 ||
                observedTankButton.surface(tank.position) != 0x4E) {
                std::cerr << "Level 4 reversed Tank did not retain its southward heading\n";
                return EXIT_FAILURE;
            }
        }
        std::sort(actualTankPositions.begin(), actualTankPositions.end());
        if (actualTankPositions != lessonFourReversedTankPositions) {
            std::cerr << "Level 4 reversed Tanks did not occupy the canonical southward destinations\n";
            return EXIT_FAILURE;
        }

        // Check Level 4's real ToggleButton against every original ToggleWall
        // and ToggleFloor coordinate. Activating the map's north ToggleButton
        // must exchange the live upper and lower terrain IDs in the row-major
        // ToggleList, and a second press must restore the original board.
        struct ExpectedLevelFourToggle {
            int x;
            int y;
            std::uint8_t initial;
            std::uint8_t alternate;
        };
        constexpr std::array<ExpectedLevelFourToggle, 6> lessonFourToggles{{
            {22, 6, 0x26, 0x25}, {19, 9, 0x25, 0x26},
            {22, 10, 0x25, 0x26}, {22, 12, 0x25, 0x26},
            {19, 13, 0x26, 0x25}, {22, 16, 0x26, 0x25}
        }};
        const int lessonFourToggleButton = 8 * chips::kBoardWidth + 21;
        const int lessonFourToggleApproach = 7 * chips::kBoardWidth + 21;
        auto observedToggleButton = chips::GameState::start(lessonFour);
        const int toggleStartPosition = observedToggleButton.playerPosition;
        observedToggleButton.top[static_cast<std::size_t>(toggleStartPosition)] =
            lessonFour.bottom[static_cast<std::size_t>(toggleStartPosition)];
        observedToggleButton.playerPosition = lessonFourToggleApproach;
        if (lessonFour.top[static_cast<std::size_t>(lessonFourToggleButton)] != 0x23 ||
            lessonFour.top[static_cast<std::size_t>(lessonFourToggleApproach)] != 0x00 ||
            observedToggleButton.togglePositions.size() != lessonFourToggles.size() ||
            observedToggleButton.pressMove(chips::Direction::south) != chips::MoveResult::moved ||
            observedToggleButton.playerPosition != lessonFourToggleButton ||
            !observedToggleButton.toggleState ||
            observedToggleButton.playerSpriteTile() != 0x6E) {
            std::cerr << "Level 4 source-map ToggleButton setup or first activation differs\n";
            return EXIT_FAILURE;
        }
        for (const auto& toggle : lessonFourToggles) {
            const auto position = static_cast<std::size_t>(toggle.y * chips::kBoardWidth + toggle.x);
            if (lessonFour.top[position] != toggle.initial ||
                observedToggleButton.top[position] != toggle.alternate ||
                observedToggleButton.bottom[position] != lessonFour.bottom[position]) {
                std::cerr << "Level 4 ToggleButton failed to swap the canonical terrain at ("
                          << toggle.x << ',' << toggle.y << ")\n";
                return EXIT_FAILURE;
            }
        }
        observedToggleButton.advanceTicks(2);
        if (observedToggleButton.pressMove(chips::Direction::north) != chips::MoveResult::moved ||
            !observedToggleButton.toggleState) {
            std::cerr << "Level 4 Chip could not leave the activated ToggleButton\n";
            return EXIT_FAILURE;
        }
        observedToggleButton.advanceTicks(2);
        if (observedToggleButton.pressMove(chips::Direction::south) != chips::MoveResult::moved ||
            observedToggleButton.toggleState) {
            std::cerr << "Level 4 second ToggleButton activation did not restore its initial state\n";
            return EXIT_FAILURE;
        }
        for (const auto& toggle : lessonFourToggles) {
            const auto position = static_cast<std::size_t>(toggle.y * chips::kBoardWidth + toggle.x);
            if (observedToggleButton.top[position] != toggle.initial ||
                observedToggleButton.bottom[position] != lessonFour.bottom[position]) {
                std::cerr << "Level 4 second ToggleButton activation did not restore ("
                          << toggle.x << ',' << toggle.y << ")\n";
                return EXIT_FAILURE;
            }
        }

        struct ExpectedBugTrapStart {
            std::uint16_t level;
            int x;
            int y;
            bool linked;
        };
        constexpr std::array<ExpectedBugTrapStart, 7> bugTrapStarts{{
            {70, 13, 2, true},
            {99, 16, 10, true}, {99, 16, 12, true}, {99, 8, 14, true},
            {99, 8, 16, true}, {99, 16, 18, false}, {99, 16, 20, true}
        }};
        for (const auto& start : bugTrapStarts) {
            const auto& level = levels[static_cast<std::size_t>(start.level - 1)];
            const int position = start.y * chips::kBoardWidth + start.x;
            const auto index = static_cast<std::size_t>(position);
            const auto initialState = chips::GameState::start(level);
            const bool registeredBug = std::any_of(initialState.creatures.begin(),
                initialState.creatures.end(), [position](const chips::Creature& creature) {
                    return creature.alive && creature.position == position && creature.tile == 0x43;
                });
            const bool linkedTrap = std::any_of(initialState.traps.begin(), initialState.traps.end(),
                [position](const chips::TrapLink& trap) { return trap.trapPosition == position; });
            if (level.number != start.level || level.top[index] != 0x43 ||
                level.bottom[index] != 0x2B || !registeredBug || linkedTrap != start.linked) {
                std::cerr << "Canonical Bug-on-trap start mismatch in level " << start.level
                          << " at (" << start.x << ',' << start.y << ")\n";
                return EXIT_FAILURE;
            }
        }

        // Level 70 orders a free Bug immediately before the constrained Bug
        // at (13,2). The first Bug moves east on the same MonsterLoop pass;
        // BugMovement on the linked trap reuses that shared vector, so its
        // rendered facing must also remain east while the closed trap holds it.
        auto observedLevel70 = chips::GameState::start(levels[69], 0, 1);
        observedLevel70.advanceTicks(2);
        const int level70MovingBugPosition = 1 * chips::kBoardWidth + 22;
        const int level70TrapBugPosition = 2 * chips::kBoardWidth + 13;
        const auto movingBug = std::find_if(observedLevel70.creatures.begin(),
            observedLevel70.creatures.end(), [level70MovingBugPosition](const chips::Creature& creature) {
                return creature.alive && creature.position == level70MovingBugPosition;
            });
        const auto constrainedBug = std::find_if(observedLevel70.creatures.begin(),
            observedLevel70.creatures.end(), [level70TrapBugPosition](const chips::Creature& creature) {
                return creature.alive && creature.position == level70TrapBugPosition;
            });
        if (movingBug == observedLevel70.creatures.end() || movingBug->tile != 0x43 ||
            constrainedBug == observedLevel70.creatures.end() || constrainedBug->tile != 0x43 ||
            observedLevel70.surface(level70TrapBugPosition) != 0x43 ||
            observedLevel70.bottom[static_cast<std::size_t>(level70TrapBugPosition)] != 0x2B) {
            std::cerr << "Level 70 trap Bug did not inherit the preceding Bug's eastward facing sprite\n";
            return EXIT_FAILURE;
        }

        // In the original executable, one safe Down key press on Level 99
        // scrolls the viewport while the two visible Bugs on linked traps at
        // (8,14) and (8,16) remain in their world cells. Keep this campaign
        // action as a source-observed trap-start regression.
        auto observedLevel99 = chips::GameState::start(levels[98]);
        struct ExpectedObservedTrapBug {
            int position;
            std::uint8_t tile;
        };
        constexpr std::array<ExpectedObservedTrapBug, 6> observedTrapBugs{{
            {10 * chips::kBoardWidth + 16, 0x41},
            {12 * chips::kBoardWidth + 16, 0x41},
            {14 * chips::kBoardWidth + 8, 0x41},
            {16 * chips::kBoardWidth + 8, 0x43},
            {18 * chips::kBoardWidth + 16, 0x41},
            {20 * chips::kBoardWidth + 16, 0x41},
        }};
        if (observedLevel99.playerPosition != 17 * chips::kBoardWidth + 2 ||
            observedLevel99.pressMove(chips::Direction::south) != chips::MoveResult::moved) {
            std::cerr << "Level 99 source-observed downward move setup changed\n";
            return EXIT_FAILURE;
        }
        observedLevel99.advanceTicks(2);
        for (const auto& expected : observedTrapBugs) {
            const int position = expected.position;
            const auto bug = std::find_if(observedLevel99.creatures.begin(),
                observedLevel99.creatures.end(), [position](const chips::Creature& creature) {
                    return creature.alive && creature.position == position &&
                           (creature.tile & 0xFC) == 0x40;
                });
            if (bug == observedLevel99.creatures.end() ||
                bug->tile != expected.tile ||
                observedLevel99.surface(position) != expected.tile) {
                std::cerr << "Level 99 Bug moved off its linked trap after the observed Down input at ("
                          << position % chips::kBoardWidth << ','
                          << position / chips::kBoardWidth << ") or has the wrong inherited facing sprite\n";
                return EXIT_FAILURE;
            }
        }

        std::size_t optionalFieldCount = 0;
        std::size_t creatureCount = 0;
        std::uint64_t stressSteps = 0;
        std::uint64_t stressTicks = 0;
        std::uint64_t stressRestarts = 0;
        for (std::size_t i = 0; i < levels.size(); ++i) {
            const auto& level = levels[i];
            if (level.number != i + 1 || level.title.empty()) {
                std::cerr << "Invalid order or missing title at campaign index " << i + 1 << '\n';
                return EXIT_FAILURE;
            }
            if (level.top.size() != chips::kBoardCells || level.bottom.size() != chips::kBoardCells) {
                std::cerr << "Invalid layer dimensions in level " << level.number << '\n';
                return EXIT_FAILURE;
            }
            optionalFieldCount += level.fields.size();
            const auto state = chips::GameState::start(level);
            if (state.outcome != chips::Outcome::playing || state.level != &level) {
                std::cerr << "Level " << level.number << " failed initial-state construction\n";
                return EXIT_FAILURE;
            }
            std::string spritePlacementError;
            if (!validInitialSpritePlacement(level, state, spritePlacementError)) {
                std::cerr << "Initial sprite placement mismatch in level " << level.number
                          << ": " << spritePlacementError << '\n';
                return EXIT_FAILURE;
            }
            creatureCount += state.creatures.size();
            std::vector<std::pair<int, std::uint8_t>> metadataCreatures;
            std::vector<ExpectedTrapLink> metadataTraps;
            std::vector<ExpectedCloneLink> metadataClones;
            for (const auto& field : level.fields) {
                if (field.id == 10) {
                    if (field.raw.size() % 2 != 0) {
                        std::cerr << "Malformed creature-coordinate field in level " << level.number << '\n';
                        return EXIT_FAILURE;
                    }
                    for (std::size_t offset = 0; offset < field.raw.size(); offset += 2) {
                        const auto x = field.raw[offset];
                        const auto y = field.raw[offset + 1];
                        if (x >= chips::kBoardWidth || y >= chips::kBoardHeight) {
                            std::cerr << "Creature coordinate outside board in level " << level.number << '\n';
                            return EXIT_FAILURE;
                        }
                        const int position = static_cast<int>(y * chips::kBoardWidth + x);
                        const auto tile = level.top[static_cast<std::size_t>(position)];
                        if (tile >= 0x40 && tile <= 0x63 &&
                            level.bottom[static_cast<std::size_t>(position)] != 0x31)
                            metadataCreatures.emplace_back(position, tile);
                    }
                } else if (field.id == 4) {
                    if (field.raw.size() % 10 != 0) {
                        std::cerr << "Malformed trap-link field in level " << level.number << '\n';
                        return EXIT_FAILURE;
                    }
                    for (std::size_t offset = 0; offset < field.raw.size(); offset += 10) {
                        const auto* record = field.raw.data() + offset;
                        const auto buttonX = readLittleWord(record);
                        const auto buttonY = readLittleWord(record + 2);
                        const auto trapX = readLittleWord(record + 4);
                        const auto trapY = readLittleWord(record + 6);
                        if (buttonX >= chips::kBoardWidth || buttonY >= chips::kBoardHeight ||
                            trapX >= chips::kBoardWidth || trapY >= chips::kBoardHeight) {
                            std::cerr << "Trap-link coordinate outside board in level " << level.number << '\n';
                            return EXIT_FAILURE;
                        }
                        metadataTraps.push_back({
                            static_cast<int>(buttonY * chips::kBoardWidth + buttonX),
                            static_cast<int>(trapY * chips::kBoardWidth + trapX),
                            readLittleWord(record + 8) != 0});
                    }
                } else if (field.id == 5) {
                    if (field.raw.size() % 8 != 0) {
                        std::cerr << "Malformed clone-link field in level " << level.number << '\n';
                        return EXIT_FAILURE;
                    }
                    for (std::size_t offset = 0; offset < field.raw.size(); offset += 8) {
                        const auto* record = field.raw.data() + offset;
                        const auto buttonX = readLittleWord(record);
                        const auto buttonY = readLittleWord(record + 2);
                        const auto machineX = readLittleWord(record + 4);
                        const auto machineY = readLittleWord(record + 6);
                        if (buttonX >= chips::kBoardWidth || buttonY >= chips::kBoardHeight ||
                            machineX >= chips::kBoardWidth || machineY >= chips::kBoardHeight) {
                            std::cerr << "Clone-link coordinate outside board in level " << level.number << '\n';
                            return EXIT_FAILURE;
                        }
                        const int button = static_cast<int>(buttonY * chips::kBoardWidth + buttonX);
                        const int machine = static_cast<int>(machineY * chips::kBoardWidth + machineX);
                        metadataClones.push_back({button, machine,
                            level.top[static_cast<std::size_t>(machine)]});
                    }
                }
            }
            if (state.creatures.size() != metadataCreatures.size()) {
                std::cerr << "Initial creature count disagrees with field 10 in level "
                          << level.number << '\n';
                return EXIT_FAILURE;
            }
            // NewMonster calls EnterTrap when a registered creature starts on
            // a trap. That startup action sets the entire contiguous trap span
            // from the live upper-layer buttons, so its effective flags can
            // differ from the values stored in optional field 4.
            for (std::size_t begin = 0; begin < metadataTraps.size();) {
                std::size_t end = begin + 1;
                while (end < metadataTraps.size() &&
                       metadataTraps[end].trap == metadataTraps[begin].trap)
                    ++end;
                const bool creatureStartsHere =
                    level.bottom[static_cast<std::size_t>(metadataTraps[begin].trap)] == 0x2B &&
                    std::any_of(metadataCreatures.begin(),
                    metadataCreatures.end(), [&](const auto& creature) {
                        return creature.first == metadataTraps[begin].trap;
                    });
                if (creatureStartsHere) {
                    const bool closed = std::all_of(metadataTraps.begin() +
                        static_cast<std::ptrdiff_t>(begin), metadataTraps.begin() +
                        static_cast<std::ptrdiff_t>(end), [&](const ExpectedTrapLink& link) {
                            return level.top[static_cast<std::size_t>(link.button)] == 0x27;
                        });
                    for (std::size_t index = begin; index < end; ++index)
                        metadataTraps[index].closed = closed;
                }
                begin = end;
            }
            for (std::size_t index = 0; index < metadataCreatures.size(); ++index) {
                if (state.creatures[index].position != metadataCreatures[index].first ||
                    state.creatures[index].tile != metadataCreatures[index].second ||
                    state.creatures[index].movementDirection !=
                        static_cast<std::int8_t>(metadataCreatures[index].second & 0x03)) {
                    std::cerr << "Initial creature state disagrees with field 10 in level "
                              << level.number << '\n';
                    return EXIT_FAILURE;
                }
            }
            if (state.traps.size() != metadataTraps.size()) {
                std::cerr << "Trap-link count disagrees with field 4 in level " << level.number << '\n';
                return EXIT_FAILURE;
            }
            for (std::size_t index = 0; index < metadataTraps.size(); ++index) {
                const auto& actual = state.traps[index];
                const auto& expected = metadataTraps[index];
                if (actual.buttonPosition != expected.button || actual.trapPosition != expected.trap ||
                    actual.closed != expected.closed) {
                    std::cerr << "Trap-link order/coordinates/flag disagree with field 4 in level "
                              << level.number << '\n';
                    return EXIT_FAILURE;
                }
            }
            if (state.cloneLinks.size() != metadataClones.size()) {
                std::cerr << "Clone-link count disagrees with field 5 in level " << level.number << '\n';
                return EXIT_FAILURE;
            }
            for (std::size_t index = 0; index < metadataClones.size(); ++index) {
                const auto& actual = state.cloneLinks[index];
                const auto& expected = metadataClones[index];
                if (actual.buttonPosition != expected.button || actual.machinePosition != expected.machine ||
                    actual.prototypeTile != expected.prototype) {
                    std::cerr << "Clone-link order/coordinates/template disagree with field 5 in level "
                              << level.number << '\n';
                    return EXIT_FAILURE;
                }
            }
            std::vector<int> expectedTeleports;
            std::vector<int> expectedToggles;
            for (int position = 0; position < chips::kBoardCells; ++position) {
                const auto upper = level.top[static_cast<std::size_t>(position)];
                const auto boardTile = upper >= 0x40 && upper <= 0x63
                    ? level.bottom[static_cast<std::size_t>(position)] : upper;
                if (boardTile == 0x29) expectedTeleports.push_back(position);
                if (boardTile == 0x25 || boardTile == 0x26) expectedToggles.push_back(position);
            }
            if (state.teleports != expectedTeleports || state.togglePositions != expectedToggles) {
                std::cerr << "Board device index disagrees with source layers in level "
                          << level.number << '\n';
                return EXIT_FAILURE;
            }
            if (state.chipsLeft != level.chipsRequired || state.secondsLeft != level.timeLimit) {
                std::cerr << "Initial metadata mismatch in level " << level.number << '\n';
                return EXIT_FAILURE;
            }

            const auto replay = makeCampaignReplay(level.number);
            const auto firstRun = chips::runReplay(level, replay);
            const auto secondRun = chips::runReplay(level, replay);
            if (firstRun.size() != secondRun.size()) {
                std::cerr << "Replay snapshot count mismatch in level " << level.number << '\n';
                return EXIT_FAILURE;
            }
            for (std::size_t command = 0; command < firstRun.size(); ++command) {
                if (firstRun[command].digest != secondRun[command].digest) {
                    std::cerr << "Seeded replay diverged in level " << level.number
                              << " after command " << command + 1 << '\n';
                    return EXIT_FAILURE;
                }
            }
            auto auditedState = chips::GameState::start(level, replay.initialGameTicks, replay.randomSeed);
            std::string invariantFailure;
            if (!validRuntimeReferences(auditedState, invariantFailure)) {
                std::cerr << "Invalid initial runtime state in level " << level.number
                          << ": " << invariantFailure << '\n';
                return EXIT_FAILURE;
            }
            for (std::size_t commandIndex = 0; commandIndex < replay.commands.size(); ++commandIndex) {
                const auto& command = replay.commands[commandIndex];
                if (command.operation == chips::ReplayOperation::queueMove)
                    auditedState.queueMove(command.direction);
                else if (command.operation == chips::ReplayOperation::pressMove)
                    (void)auditedState.pressMove(command.direction);
                else if (command.operation == chips::ReplayOperation::advanceTicks)
                    auditedState.advanceTicks(command.ticks);
                const auto auditedSnapshot = chips::snapshot(auditedState);
                if (auditedSnapshot.digest != firstRun[commandIndex].digest) {
                    std::cerr << "Direct campaign replay diverged in level " << level.number
                              << " after command " << commandIndex + 1 << '\n';
                    return EXIT_FAILURE;
                }
                if (!validRuntimeReferences(auditedState, invariantFailure)) {
                    std::cerr << "Invalid runtime state in level " << level.number
                              << " after command " << commandIndex + 1 << ": "
                              << invariantFailure << '\n';
                    return EXIT_FAILURE;
                }
            }

            for (std::uint32_t scenario = 0; scenario < 3; ++scenario) {
                const auto stressSeed = 0xA341316CU ^
                    (static_cast<std::uint32_t>(level.number) * 0x9E3779B9U) ^
                    (scenario * 0x85EBCA6BU);
                if (!stressCampaignLevel(level, stressSeed, stressSteps, stressTicks,
                                         stressRestarts, invariantFailure)) {
                    std::cerr << "Campaign gameplay stress failed in level " << level.number
                              << ", scenario " << scenario + 1 << ": " << invariantFailure << '\n';
                    return EXIT_FAILURE;
                }
            }
        }

        std::cout << "Validated 149 ordered levels; all 32x32 layers decoded; "
                  << hintMetadataCount << " original hint strings exactly match their field-7 bytes "
                  << "and all corresponding Hint tiles are present; "
                  << optionalFieldCount << " optional records retained; trap/clone "
                  << "link records and creature starts match decoded metadata; "
                  << creatureCount << " creature starts initialized; deterministic "
                  << "seeded replays and runtime-reference checks passed for all campaign levels; "
                  << stressSteps << " stress input operations, " << stressTicks
                  << " requested ticks, and " << stressRestarts << " terminal-state restarts "
                  << "passed across three additional seeds per level.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "Campaign validation failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
