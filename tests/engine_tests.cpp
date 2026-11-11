#include "engine.hpp"
#include "levelpack.hpp"
#include "replay.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using namespace chips;

Level blankLevel() {
    Level level;
    level.number = 1;
    level.timeLimit = 0;
    level.chipsRequired = 0;
    level.top.fill(0x00);
    level.bottom.fill(0x00);
    for (int x = 0; x < kBoardWidth; ++x) {
        level.top[static_cast<std::size_t>(x)] = 0x01;
        level.top[static_cast<std::size_t>((kBoardHeight - 1) * kBoardWidth + x)] = 0x01;
    }
    for (int y = 0; y < kBoardHeight; ++y) {
        level.top[static_cast<std::size_t>(y * kBoardWidth)] = 0x01;
        level.top[static_cast<std::size_t>(y * kBoardWidth + kBoardWidth - 1)] = 0x01;
    }
    return level;
}

int at(int x, int y) { return y * kBoardWidth + x; }

int neighborForMonsterDirection(int position, int direction) {
    static constexpr int dx[4]{0, -1, 0, 1}; // creature tile order: N, W, S, E
    static constexpr int dy[4]{-1, 0, 1, 0};
    return position + dy[direction] * kBoardWidth + dx[direction];
}

GameState startWithMapMonsters(Level& level) {
    const auto hasMonsterField = std::any_of(level.fields.begin(), level.fields.end(),
        [](const OptionalField& field) { return field.id == 10; });
    if (!hasMonsterField) {
        OptionalField field;
        field.id = 10;
        for (int position = 0; position < kBoardCells; ++position) {
            const auto tile = level.top[static_cast<std::size_t>(position)];
            if (tile < 0x40 || tile > 0x63) continue;
            field.raw.push_back(static_cast<std::uint8_t>(position % kBoardWidth));
            field.raw.push_back(static_cast<std::uint8_t>(position / kBoardWidth));
        }
        level.fields.push_back(std::move(field));
    }
    return GameState::start(level);
}

bool check(bool condition, const std::string& label) {
    if (!condition) std::cerr << "FAIL: " << label << '\n';
    return condition;
}
}

int main() {
    int failures = 0;
    {
        constexpr std::array<Direction, 4> facings{
            Direction::north, Direction::east, Direction::south, Direction::west};
        constexpr std::array<std::uint8_t, 4> landSprites{0x6C, 0x6F, 0x6E, 0x6D};
        constexpr std::array<std::uint8_t, 4> swimSprites{0x3C, 0x3F, 0x3E, 0x3D};
        for (std::size_t i = 0; i < facings.size(); ++i) {
            failures += !check(chipSpriteTile(facings[i], false) == landSprites[i],
                "Chip facing selects the source land sprite for direction " + std::to_string(i));
            failures += !check(chipSpriteTile(facings[i], true) == swimSprites[i],
                "Chip facing selects the source swimming sprite for direction " + std::to_string(i));
        }
        auto level = blankLevel();
        const auto playerPosition = at(2, 2);
        level.top[static_cast<std::size_t>(playerPosition)] = 0x6F;
        auto game = GameState::start(level);
        for (std::size_t i = 0; i < facings.size(); ++i) {
            game.facing = facings[i];
            game.top[static_cast<std::size_t>(playerPosition)] = 0x03;
            failures += !check(game.playerSpriteTile() == swimSprites[i],
                "GameState selects its swimming sprite from the Water layer");
            game.top[static_cast<std::size_t>(playerPosition)] = 0x0A;
            game.bottom[static_cast<std::size_t>(playerPosition)] = landSprites[i];
            failures += !check(game.playerSpriteTile() == landSprites[i],
                "GameState selects its land sprite when Block preserves a land pose");
            game.bottom[static_cast<std::size_t>(playerPosition)] = swimSprites[i];
            failures += !check(game.playerSpriteTile() == swimSprites[i],
                "GameState preserves a swimming pose under Block");
            game.bottom[static_cast<std::size_t>(playerPosition)] = 0x00;
        }
    }
    {
        auto idleLevel = blankLevel();
        idleLevel.top[at(1, 1)] = 0x6C; // Chip starts facing north.
        auto idle = GameState::start(idleLevel);
        idle.advanceTicks(5); // Two eligible even ticks.
        failures += !check(idle.facing == Direction::north,
                           "Chip keeps facing north before the third idle even tick");
        idle.advanceTicks(1);
        failures += !check(idle.facing == Direction::south,
                           "Chip turns south after three idle even ticks");
        (void)idle.move(Direction::east);
        idle.advanceTicks(4);
        failures += !check(idle.facing == Direction::east,
                           "a movement attempt resets the idle-facing timer");
        idle.gameTicks = 1; // The next eligible DoTick is even.
        idle.idleTickCount = 0xFFFFU;
        idle.facing = Direction::north;
        idle.advanceTicks(1);
        failures += !check(idle.idleTickCount == 0 && idle.facing == Direction::south,
                           "idle-facing counter wraps as the original 16-bit word");
    }
    {
        auto monsterLevel = blankLevel();
        monsterLevel.top[at(4, 5)] = 0x6F;
        monsterLevel.top[at(5, 5)] = 0x4F; // East-facing Tank.
        monsterLevel.bottom[at(5, 5)] = 0x0C; // Ice beneath the Tank.
        auto monster = startWithMapMonsters(monsterLevel);
        // Model an upper-layer change made while the creature occupies this
        // cell. MoveMonster restores Upper from Lower when it leaves.
        monster.top[at(5, 5)] = 0x00;
        monster.gameTicks = 1;
        monster.advanceTicks(1); // Even tick: MonsterLoop moves the Tank east.
        failures += !check(monster.creatures[0].position == at(6, 5) &&
                           monster.top[at(5, 5)] == 0x0C &&
                           monster.bottom[at(5, 5)] == 0x00,
                           "MonsterLoop restores the creature's vacated upper layer from its terrain underlay");
        (void)monster.move(Direction::east);
        failures += !check(monster.playerPosition == at(5, 5) && monster.sliding &&
                           monster.outcome == Outcome::playing,
                           "Chip enters the vacated ice tile instead of colliding with a stale creature sprite");

        auto slipLevel = blankLevel();
        slipLevel.top[at(4, 5)] = 0x6F;
        slipLevel.top[at(5, 5)] = 0x43; // East-facing Bug on Ice.
        slipLevel.bottom[at(5, 5)] = 0x0C;
        auto slipping = startWithMapMonsters(slipLevel);
        slipping.top[at(5, 5)] = 0x00;
        slipping.slipOrder.push_back({SlipperKind::creature, 0, 3});
        slipping.advanceTicks(1); // Odd tick: only SlipLoop moves the Bug east.
        failures += !check(slipping.creatures[0].position == at(6, 5) &&
                           slipping.top[at(5, 5)] == 0x0C &&
                           slipping.bottom[at(5, 5)] == 0x00,
                           "SlipLoop restores the creature's vacated upper layer from its terrain underlay");
        (void)slipping.move(Direction::east);
        failures += !check(slipping.playerPosition == at(5, 5) && slipping.sliding &&
                           slipping.outcome == Outcome::playing,
                           "Chip can enter the terrain restored by a creature SlipLoop move");

        for (const auto item : {std::uint8_t{0x64}, std::uint8_t{0x68}}) {
            auto itemLevel = blankLevel();
            itemLevel.top[at(2, 3)] = 0x6F;
            itemLevel.top[at(5, 5)] = 0x4F; // East-facing Tank.
            itemLevel.top[at(6, 5)] = item;
            auto itemGame = startWithMapMonsters(itemLevel);
            itemGame.gameTicks = 1;
            itemGame.advanceTicks(1); // Even tick: Tank attempts to enter the item.
            const bool inventoryUntouched = item == 0x64
                ? std::all_of(itemGame.keys.begin(), itemGame.keys.end(),
                              [](std::uint8_t count) { return count == 0; })
                : std::none_of(itemGame.boots.begin(), itemGame.boots.end(),
                               [](bool owned) { return owned; });
            if (item <= 0x67) {
                failures += !check(itemGame.creatures[0].position == at(6, 5) &&
                                   itemGame.surface(at(6, 5)) == 0x4F &&
                                   itemGame.floor(at(6, 5)) == item &&
                                   inventoryUntouched,
                                   "MonsterLoop covers a passable key without collecting it");
                itemGame.advanceTicks(2); // Next even tick: Tank leaves the key.
                failures += !check(itemGame.creatures[0].position == at(7, 5) &&
                                   itemGame.surface(at(6, 5)) == item &&
                                   itemGame.floor(at(6, 5)) == 0x00,
                                   "MonsterLoop restores a covered key when the creature leaves");
            } else {
                failures += !check(itemGame.creatures[0].position == at(5, 5) &&
                                   itemGame.surface(at(6, 5)) == item &&
                                   itemGame.floor(at(6, 5)) == 0x00 &&
                                   inventoryUntouched,
                                   "MonsterLoop blocks a boot pickup without collecting it");
            }
        }
    }
    {
        auto level = blankLevel();
        level.chipsRequired = 1;
        level.top[at(2, 2)] = 0x6F;
        level.top[at(3, 2)] = 0x02;
        level.top[at(4, 2)] = 0x22; // Socket
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(3, 2) && game.chipsLeft == 0,
                           "Chip collects the required chip before reaching the socket");
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(4, 2) && game.outcome == Outcome::playing,
                           "socket becomes enterable after all required chips are collected");
        failures += !check(game.message == "Socket opened.",
                           "successful socket entry produces explicit open-door feedback");
        (void)game.move(Direction::west);
        failures += !check(game.top[at(4, 2)] == 0x00 && game.bottom[at(4, 2)] == 0x00,
                           "an entered socket is removed like a door when Chip leaves it");

        auto lockedLevel = blankLevel();
        lockedLevel.chipsRequired = 1;
        lockedLevel.top[at(2, 2)] = 0x6F;
        lockedLevel.top[at(3, 2)] = 0x22;
        auto locked = startWithMapMonsters(lockedLevel);
        (void)locked.move(Direction::east);
        failures += !check(locked.playerPosition == at(2, 2) && locked.chipsLeft == 1,
                           "socket rejects entry while required chips remain");
    }
    {
        auto level = blankLevel();
        level.number = 1;
        level.chipsRequired = 2;
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x02;
        level.top[at(3, 1)] = 0x02;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.chipsLeft == 1,
                           "chip counter counts down from two after the first pickup");
        (void)game.move(Direction::east);
        failures += !check(game.chipsLeft == 0,
                           "chip counter counts down to zero after the second pickup");
        (void)game.move(Direction::west);
        (void)game.move(Direction::east);
        failures += !check(game.chipsLeft == 0,
                           "chip counter stays at zero and never becomes negative");
    }
    {
        // CheckPanelWalls uses separate direction tables for entering and
        // leaving the four one-way panels and PanelSE. Exhaust both paths
        // against the original assembly's flag=1/flag=0 rules.
        constexpr std::array<std::uint8_t, 5> panels{0x06, 0x07, 0x08, 0x09, 0x30};
        constexpr std::array<Direction, 4> directions{
            Direction::north, Direction::east, Direction::south, Direction::west};
        constexpr std::array<std::array<bool, 4>, 5> entering{{
            {{true, true, false, true}},  // PanelN
            {{true, false, true, true}},  // PanelW
            {{false, true, true, true}},  // PanelS
            {{true, true, true, false}},  // PanelE
            {{false, true, true, false}}  // PanelSE
        }};
        constexpr std::array<std::array<bool, 4>, 5> exiting{{
            {{false, true, true, true}},  // PanelN
            {{true, true, true, false}},  // PanelW
            {{true, true, false, true}},  // PanelS
            {{true, false, true, true}},  // PanelE
            {{true, false, false, true}}  // PanelSE
        }};
        const auto offset = [](Direction direction) {
            switch (direction) {
                case Direction::north: return std::pair{0, -1};
                case Direction::east: return std::pair{1, 0};
                case Direction::south: return std::pair{0, 1};
                case Direction::west: return std::pair{-1, 0};
            }
            return std::pair{0, 0};
        };
        for (std::size_t panel = 0; panel < panels.size(); ++panel) {
            for (std::size_t heading = 0; heading < directions.size(); ++heading) {
                const auto direction = directions[heading];
                const auto [dx, dy] = offset(direction);

                auto entryLevel = blankLevel();
                entryLevel.top[at(10 - dx, 10 - dy)] = 0x6F;
                entryLevel.top[at(10, 10)] = panels[panel];
                auto entry = startWithMapMonsters(entryLevel);
                const auto entryResult = entry.move(direction);
                failures += !check((entryResult == MoveResult::moved) == entering[panel][heading] &&
                                   (entry.playerPosition == (entering[panel][heading]
                                       ? at(10, 10) : at(10 - dx, 10 - dy))),
                    "player panel entry matches CheckPanelWalls for tile " +
                    std::to_string(panels[panel]) + " heading " + std::to_string(heading));

                auto exitLevel = blankLevel();
                exitLevel.top[at(10, 10)] = 0x6F;
                exitLevel.bottom[at(10, 10)] = panels[panel];
                auto exit = startWithMapMonsters(exitLevel);
                const auto exitResult = exit.move(direction);
                failures += !check((exitResult == MoveResult::moved) == exiting[panel][heading] &&
                                   (exit.playerPosition == (exiting[panel][heading]
                                       ? at(10 + dx, 10 + dy) : at(10, 10))),
                    "player panel exit matches CheckPanelWalls for tile " +
                    std::to_string(panels[panel]) + " heading " + std::to_string(heading));
            }
        }
    }
    {
        auto level = blankLevel();
        level.top[at(2, 2)] = 0x6F;
        level.top[at(5, 5)] = 0x40; // Bug listed in field 10.
        level.bottom[at(5, 5)] = 0x29;
        level.top[at(6, 5)] = 0x44; // Unlisted sprite remains static.
        level.bottom[at(6, 5)] = 0x25;
        level.top[at(10, 10)] = 0x29;
        level.fields.push_back({10, {5, 5}});
        auto game = GameState::start(level);
        failures += !check(game.creatures.size() == 1 &&
                           game.creatures[0].position == at(5, 5) &&
                           game.surface(at(6, 5)) == 0x44,
                           "field 10 alone registers live creatures");
        failures += !check(game.teleports.size() == 2 &&
                           std::find(game.teleports.begin(), game.teleports.end(), at(5, 5)) != game.teleports.end(),
                           "board initialization finds a teleport under a creature sprite");
        failures += !check(std::find(game.togglePositions.begin(), game.togglePositions.end(), at(6, 5)) !=
                               game.togglePositions.end(),
                           "board initialization finds toggle terrain under an unregistered creature sprite");
    }
    {
        auto level = blankLevel();
        level.top[at(2, 2)] = 0x6F;
        level.top[at(5, 5)] = 0x40;
        auto game = GameState::start(level);
        failures += !check(game.creatures.empty() && game.top[at(5, 5)] == 0x40,
                           "creature-looking map tile without field 10 is not activated");
    }
    {
        // Block eligibility values copied from the original TileTable's
        // block column in data.asm (0x00 through 0x6F). Values 2 are the
        // directional panel/ice-wall cases; choose a legal entering vector.
        constexpr std::array<std::uint8_t, 0x70> originalBlockEligibility{
            1,0,0,1,1,0,2,2,2,2,0,0,1,1,0,0,
            0,0,1,1,1,1,0,0,0,0,2,2,2,2,0,0,
            0,0,0,1,1,0,1,1,1,1,1,1,0,1,0,1,
            2,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1
        };
        for (unsigned tile = 0; tile < originalBlockEligibility.size(); ++tile) {
            auto level = blankLevel();
            auto direction = Direction::east;
            switch (tile) {
                case 0x07: case 0x09: case 0x1A: case 0x1B:
                    direction = Direction::north;
                    break;
                case 0x1C:
                    direction = Direction::east;
                    break;
                case 0x1D:
                    direction = Direction::south;
                    break;
                default: break;
            }
            const int player = at(12, 12);
            const int dx = direction == Direction::east ? 1 : direction == Direction::west ? -1 : 0;
            const int dy = direction == Direction::south ? 1 : direction == Direction::north ? -1 : 0;
            const int block = at(12 + dx, 12 + dy);
            const int destination = at(12 + 2 * dx, 12 + 2 * dy);
            level.top[static_cast<std::size_t>(player)] = 0x6F;
            level.top[static_cast<std::size_t>(block)] = 0x0A;
            if (tile < 0x6C)
                level.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);
            auto game = startWithMapMonsters(level);
            if (tile >= 0x6C)
                game.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);
            (void)game.move(direction);
            const bool entered = game.playerPosition == block;
            const bool expected = originalBlockEligibility[tile] != 0;
            failures += !check(entered == expected,
                "block TileTable eligibility for tile ID " + std::to_string(tile));
        }
        auto cloneMachineFloor = blankLevel();
        cloneMachineFloor.top[at(12, 12)] = 0x6F;
        cloneMachineFloor.top[at(13, 12)] = 0x0A;
        cloneMachineFloor.top[at(14, 12)] = 0x00;
        cloneMachineFloor.bottom[at(14, 12)] = 0x31;
        auto blockedMachineFloor = startWithMapMonsters(cloneMachineFloor);
        (void)blockedMachineFloor.move(Direction::east);
        failures += !check(blockedMachineFloor.playerPosition == at(12, 12),
                           "BlockCanEnterTile rejects a CloneMachine on the lower layer");

        auto blockOnMachine = blankLevel();
        blockOnMachine.top[at(12, 12)] = 0x6F;
        blockOnMachine.top[at(13, 12)] = 0x0A;
        blockOnMachine.bottom[at(13, 12)] = 0x31;
        auto pushedOffMachine = startWithMapMonsters(blockOnMachine);
        failures += !check(pushedOffMachine.move(Direction::east) == MoveResult::blocked &&
                           pushedOffMachine.playerPosition == at(12, 12) &&
                           pushedOffMachine.top[at(13, 12)] == 0x0A &&
                           pushedOffMachine.bottom[at(13, 12)] == 0x31 &&
                           pushedOffMachine.top[at(14, 12)] == 0x0A,
                           "pushing a block off a CloneMachine preserves the machine's upper template");

        auto playerOverMachine = blankLevel();
        playerOverMachine.top[at(12, 12)] = 0x01;
        playerOverMachine.top[at(13, 12)] = 0x0A;
        playerOverMachine.bottom[at(13, 12)] = 0x0C;
        playerOverMachine.top[at(14, 12)] = 0x6F;
        playerOverMachine.bottom[at(14, 12)] = 0x31;
        auto slidingBlock = startWithMapMonsters(playerOverMachine);
        slidingBlock.blockDirections[at(13, 12)] = static_cast<std::int8_t>(Direction::east);
        slidingBlock.slipOrder.push_back({SlipperKind::block, at(13, 12)});
        slidingBlock.advanceTicks();
        failures += !check(slidingBlock.top[at(13, 12)] == 0x0A &&
                           slidingBlock.playerPosition == at(14, 12) &&
                           slidingBlock.outcome == Outcome::playing,
                           "sliding block cannot enter Chip over a lower CloneMachine");
    }
    {
        // Monster eligibility from the original TileTable's monster column.
        // Water/fire are eligible action tiles for a ball but kill it after
        // entry; swimming-Chip sprites are transparent and use their lower tile.
        constexpr std::array<std::uint8_t, 0x70> originalMonsterEligibility{
            1,0,0,2,2,0,2,2,2,2,0,0,1,1,0,0,
            0,0,1,1,1,0,0,0,0,0,2,2,2,2,0,0,
            0,0,0,1,1,0,1,1,1,1,1,1,0,0,0,1,
            2,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1
        };
        constexpr std::array<std::uint8_t, 9> creatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60
        };
        constexpr std::array<const char*, 9> creatureNames{
            "Bug", "Fireball", "Ball", "Tank", "Glider", "Teeth", "Walker", "Blob", "Paramecium"
        };
        for (std::size_t kindIndex = 0; kindIndex < creatureKinds.size(); ++kindIndex) {
            for (unsigned tile = 0; tile < originalMonsterEligibility.size(); ++tile) {
                auto level = blankLevel();
                auto direction = Direction::east;
                switch (tile) {
                    case 0x07: case 0x09: case 0x1A: case 0x1B:
                        direction = Direction::north;
                        break;
                    case 0x1C:
                        direction = Direction::east;
                        break;
                    case 0x1D:
                        direction = Direction::south;
                        break;
                    default: break;
                }
                const int dx = direction == Direction::east ? 1 : direction == Direction::west ? -1 : 0;
                const int dy = direction == Direction::south ? 1 : direction == Direction::north ? -1 : 0;
                const int source = at(10, 10);
                const int destination = at(10 + dx, 10 + dy);
                const int directionCode = direction == Direction::north ? 0 : direction == Direction::west ? 1
                    : direction == Direction::south ? 2 : 3;
                level.top[at(20, 20)] = 0x6F;
                level.top[source] = static_cast<std::uint8_t>(creatureKinds[kindIndex] | directionCode);
                level.bottom[source] = 0x0C; // Drive the exact entry vector through SlipLoop.
                if (tile < 0x6C)
                    level.top[destination] = static_cast<std::uint8_t>(tile);
                auto game = startWithMapMonsters(level);
                if (tile >= 0x6C)
                    game.top[destination] = static_cast<std::uint8_t>(tile);
                game.slipOrder.push_back({SlipperKind::creature, 0});
                game.advanceTicks(1);
                const auto& creature = game.creatures[0];
                const bool entered = creature.position == destination || !creature.alive;
                bool expected = originalMonsterEligibility[tile] != 0;
                // Fire's TileTable eligibility is conditional: Bugs and
                // Walkers cannot enter it; other families enter and burn out
                // except Fireballs, which survive.
                if (tile == 0x04 && (creatureKinds[kindIndex] == 0x40 ||
                                     creatureKinds[kindIndex] == 0x58))
                    expected = false;
                const std::string label = std::string(creatureNames[kindIndex]) +
                    " TileTable eligibility for tile ID " + std::to_string(tile);
                failures += !check(entered == expected, label);
            }
        }
    }
    {
        // Chip eligibility from the original TileTable player column. Special
        // entries use legal directions and sufficient keys/boots/chips; FakeWall
        // and HiddenWall intentionally reveal then reject entry.
        constexpr std::array<std::uint8_t, 0x70> originalChipEligibility{
            1,0,1,2,2,0,2,2,2,2,2,1,2,2,0,0,
            0,0,2,2,2,1,2,2,2,2,2,2,2,2,2,2,
            0,2,2,1,1,0,1,1,1,1,1,1,2,1,2,1,
            2,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0
        };
        for (unsigned tile = 0; tile < originalChipEligibility.size(); ++tile) {
            auto level = blankLevel();
            level.top[at(12, 12)] = 0x6F;
            const int destination = at(13, 12);
            if (tile < 0x6C) level.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);
            if (tile == 0x02) level.chipsRequired = 1;
            auto game = startWithMapMonsters(level);
            if (tile >= 0x6C) game.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);
            if (tile >= 0x16 && tile <= 0x19)
                game.keys[static_cast<std::size_t>(tile - 0x16)] = 1;
            if (tile == 0x03) game.boots[0] = true;
            if (tile == 0x04) game.boots[1] = true;

            auto direction = Direction::east;
            switch (tile) {
                case 0x07: case 0x09: case 0x1A: case 0x1B:
                    direction = Direction::north;
                    break;
                case 0x1C:
                    direction = Direction::east;
                    break;
                case 0x1D:
                    direction = Direction::south;
                    break;
                default: break;
            }
            // Keep the destination adjacent while using a legal wall-entry vector.
            if (direction != Direction::east) {
                game.playerPosition = at(12, 12);
                const int dx = direction == Direction::west ? -1 : direction == Direction::east ? 1 : 0;
                const int dy = direction == Direction::north ? -1 : direction == Direction::south ? 1 : 0;
                const int shiftedDestination = at(12 + dx, 12 + dy);
                game.top[static_cast<std::size_t>(destination)] = 0x00;
                game.top[static_cast<std::size_t>(shiftedDestination)] = static_cast<std::uint8_t>(tile);
            }
            if (tile >= 0x40 && tile <= 0x63) {
                // The initial scan extracts creature sprites from the map.
                const auto moved = game.move(direction);
                failures += !check(moved == MoveResult::died,
                    "player TileTable creature collision for tile ID " + std::to_string(tile));
                continue;
            }
            const auto moved = game.move(direction);
            bool expected = originalChipEligibility[tile] != 0;
            if (tile == 0x1F || tile == 0x2C) expected = false;
            failures += !check((moved != MoveResult::blocked) == expected,
                "player TileTable eligibility for tile ID " + std::to_string(tile));

            // Creature sprite overlays are transparent for eligibility, so
            // also exercise the same lower-tile rule before the collision
            // action. Block-under-creature and nested creature sprites are
            // excluded because the original recursive path does not model
            // those as valid two-layer map states.
            if (tile == 0x0A || (tile >= 0x40 && tile <= 0x63)) continue;
            auto overlayLevel = blankLevel();
            const int overlayDestination = at(13, 12);
            const int dx = direction == Direction::east ? 1
                : direction == Direction::west ? -1 : 0;
            const int dy = direction == Direction::south ? 1
                : direction == Direction::north ? -1 : 0;
            overlayLevel.top[static_cast<std::size_t>(at(13 - dx, 12 - dy))] = 0x6F;
            overlayLevel.top[static_cast<std::size_t>(overlayDestination)] = 0x40;
            if (tile < 0x6C)
                overlayLevel.bottom[static_cast<std::size_t>(overlayDestination)] =
                    static_cast<std::uint8_t>(tile);
            auto overlayGame = startWithMapMonsters(overlayLevel);
            // Player sprite IDs in a lower layer are interpreted as another
            // player start during level initialization. Install these unusual
            // underlays after initialization so this matrix tests entry rules
            // instead of accidentally making the fixture an invalid two-start map.
            if (tile >= 0x6C)
                overlayGame.bottom[static_cast<std::size_t>(overlayDestination)] =
                    static_cast<std::uint8_t>(tile);
            if (tile >= 0x16 && tile <= 0x19)
                overlayGame.keys[static_cast<std::size_t>(tile - 0x16)] = 1;
            const auto overlayResult = overlayGame.move(direction);
            bool overlayExpected = originalChipEligibility[tile] != 0;
            if (tile == 0x1F || tile == 0x2C) overlayExpected = false;
            failures += !check((overlayResult == MoveResult::died) == overlayExpected &&
                               (overlayExpected || overlayResult == MoveResult::blocked),
                "creature overlay lower-terrain eligibility for tile ID " + std::to_string(tile));
        }
    }
    {
        const auto makeCreatureOverlayLevel = [](std::uint8_t underlay) {
            auto level = blankLevel();
            level.top[at(1, 1)] = 0x6F;
            level.top[at(2, 1)] = 0x40; // Bug overlay
            level.bottom[at(2, 1)] = underlay;
            return level;
        };

        auto wallLevel = makeCreatureOverlayLevel(0x25); // ToggleWall is impassable while active.
        auto wallCollision = startWithMapMonsters(wallLevel);
        failures += !check(wallCollision.move(Direction::east) == MoveResult::blocked &&
                           wallCollision.outcome == Outcome::playing &&
                           wallCollision.playerPosition == at(1, 1),
                           "Chip cannot collide with a creature through an active lower-layer ToggleWall");

        auto floorLevel = makeCreatureOverlayLevel(0x26); // ToggleFloor is enterable.
        auto floorCollision = startWithMapMonsters(floorLevel);
        failures += !check(floorCollision.move(Direction::east) == MoveResult::died &&
                           floorCollision.playerPosition == at(2, 1) &&
                           !floorCollision.playerCoveredByCreature,
                           "Chip collides with a creature when its lower-layer toggle terrain is open");

        auto overlapLevel = blankLevel();
        overlapLevel.top[at(2, 1)] = 0x40;
        overlapLevel.bottom[at(2, 1)] = 0x6F;
        auto overlap = startWithMapMonsters(overlapLevel);
        failures += !check(overlap.pendingAutopsy && overlap.playerCoveredByCreature,
                           "a creature initially sharing Chip's tile retains the source upper-layer order");

        auto lockedDoorLevel = makeCreatureOverlayLevel(0x16);
        auto lockedDoor = startWithMapMonsters(lockedDoorLevel);
        failures += !check(lockedDoor.move(Direction::east) == MoveResult::blocked &&
                           lockedDoor.outcome == Outcome::playing,
                           "a creature sprite does not bypass a locked lower-layer door");
        auto keyedDoor = startWithMapMonsters(lockedDoorLevel);
        keyedDoor.keys[0] = 1;
        failures += !check(keyedDoor.move(Direction::east) == MoveResult::died &&
                           keyedDoor.keys[0] == 1,
                           "lower-layer door validation allows collision without consuming the key");

        auto lockedSocketLevel = makeCreatureOverlayLevel(0x22);
        lockedSocketLevel.chipsRequired = 1;
        auto lockedSocket = startWithMapMonsters(lockedSocketLevel);
        failures += !check(lockedSocket.move(Direction::east) == MoveResult::blocked &&
                           lockedSocket.chipsLeft == 1 && lockedSocket.outcome == Outcome::playing,
                           "a creature sprite does not bypass a lower-layer socket chip requirement");
    }
    {
        // Monster TileTable rows for the creature and inventory/sprite ranges.
        // Creature sprites block monsters, keys are traversable, boots block,
        // and directional Chip sprites read their lower terrain.
        constexpr std::array<std::uint8_t, 0x30> originalMonsterOverlayEligibility{
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1
        };
        constexpr std::array<std::uint8_t, 9> creatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60
        };
        constexpr std::array<const char*, 9> creatureNames{
            "Bug", "Fireball", "Ball", "Tank", "Glider", "Teeth", "Walker", "Blob", "Paramecium"
        };
        for (std::size_t kindIndex = 0; kindIndex < creatureKinds.size(); ++kindIndex) {
            for (unsigned offset = 0; offset < originalMonsterOverlayEligibility.size(); ++offset) {
                const auto tile = static_cast<std::uint8_t>(0x40 + offset);
                auto level = blankLevel();
                const int source = at(10, 10);
                const int destination = at(11, 10);
                level.top[at(20, 20)] = 0x6F;
                level.top[source] = static_cast<std::uint8_t>(creatureKinds[kindIndex] | 3U);
                level.bottom[source] = 0x0C;
                auto game = startWithMapMonsters(level);
                // Keep the tested overlay out of the initial creature scan;
                // this checks the lower-level TileTable rule in isolation.
                game.top[destination] = tile;
                game.slipOrder.push_back({SlipperKind::creature, 0});
                game.advanceTicks(1);
                const bool entered = game.creatures[0].position == destination || !game.creatures[0].alive;
                const bool expected = originalMonsterOverlayEligibility[offset] != 0;
                const std::string label = std::string(creatureNames[kindIndex]) +
                    " TileTable eligibility for overlay tile ID " + std::to_string(tile);
                failures += !check(entered == expected, label);
            }
        }

        auto chipSpriteOverWall = blankLevel();
        chipSpriteOverWall.top[at(20, 20)] = 0x6F;
        chipSpriteOverWall.top[at(10, 10)] = 0x4B;
        chipSpriteOverWall.bottom[at(10, 10)] = 0x0C;
        chipSpriteOverWall.bottom[at(11, 10)] = 0x01;
        auto blockedSpriteTerrain = startWithMapMonsters(chipSpriteOverWall);
        blockedSpriteTerrain.top[at(11, 10)] = 0x6C;
        blockedSpriteTerrain.slipOrder.push_back({SlipperKind::creature, 0});
        blockedSpriteTerrain.advanceTicks(1);
        failures += !check(blockedSpriteTerrain.creatures[0].position != at(11, 10),
                           "Chip sprite overlay still obeys its lower wall terrain");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(4, 4)] = 0x5C; // random-moving Blob exercises the replay seed
        ReplayScript script;
        script.levelNumber = level.number;
        script.randomSeed = 1234567U;
        script.initialGameTicks = 3;
        script.commands = {ReplayCommand::move(Direction::east), ReplayCommand::advance(2),
                           ReplayCommand::advance(2), ReplayCommand::move(Direction::south),
                           ReplayCommand::advance(2), ReplayCommand::advance(2)};
        const auto first = runReplay(level, script);
        const auto second = runReplay(level, script);
        failures += !check(first.size() == script.commands.size() && first.size() == second.size(),
                           "replay emits one snapshot per command");
        bool identical = first.size() == second.size();
        for (std::size_t i = 0; identical && i < first.size(); ++i)
            identical = first[i].digest == second[i].digest && first[i].gameTicks == second[i].gameTicks;
        failures += !check(identical, "same seed and input replay to identical snapshots");
        failures += !check(first.size() > 1 && first[1].playerPosition == at(2, 1) &&
                           first[1].gameTicks == 5,
                           "replay snapshot records its explicit initial tick phase and expected player position");
        auto changedInput = script;
        changedInput.commands[0] = ReplayCommand::move(Direction::west);
        failures += !check(first.back().digest != runReplay(level, changedInput).back().digest,
                           "snapshot digest changes when replayed gameplay changes");

        auto eventLevel = blankLevel();
        eventLevel.top[at(1, 1)] = 0x6F;
        ReplayScript eventScript;
        eventScript.levelNumber = eventLevel.number;
        eventScript.commands = {ReplayCommand::press(Direction::east),
                                ReplayCommand::press(Direction::south),
                                ReplayCommand::advance(3)};
        const auto eventSnapshots = runReplay(eventLevel, eventScript);
        failures += !check(eventSnapshots.size() == 3 &&
                           eventSnapshots[0].playerPosition == at(2, 1) &&
                           eventSnapshots[1].playerPosition == at(2, 1) &&
                           eventSnapshots[2].playerPosition == at(2, 2),
                           "event-time replay reproduces immediate movement and its one-key buffer");

        script.randomSeed++;
        const auto differentSeed = runReplay(level, script);
        failures += !check(first.back().digest != differentSeed.back().digest,
                           "snapshot includes deterministic random state");
        auto northVector = startWithMapMonsters(level);
        auto southVector = northVector;
        northVector.creatures[0].movementDirection = 0;
        southVector.creatures[0].movementDirection = 2;
        failures += !check(northVector.creatures[0].tile == southVector.creatures[0].tile &&
                           northVector.creatures[0].position == southVector.creatures[0].position &&
                           snapshot(northVector).digest != snapshot(southVector).digest,
                           "replay digest distinguishes hidden creature movement vectors");
        script.levelNumber++;
        bool rejected = false;
        try { (void)runReplay(level, script); }
        catch (const std::invalid_argument&) { rejected = true; }
        failures += !check(rejected, "replay refuses mismatched level identity");
    }
    {
        auto blockedLevel = blankLevel();
        blockedLevel.top[at(5, 5)] = 0x6F;
        blockedLevel.top[at(5, 4)] = 0x01;
        auto blocked = startWithMapMonsters(blockedLevel);
        failures += !check(blocked.move(Direction::north) == MoveResult::blocked &&
                           blocked.playerPosition == at(5, 5) &&
                           blocked.facing == Direction::north,
                           "blocked MoveChip updates the dry player facing without moving");

        auto swimmingLevel = blankLevel();
        swimmingLevel.top[at(5, 5)] = 0x6F;
        swimmingLevel.bottom[at(5, 5)] = 0x03;
        swimmingLevel.top[at(5, 4)] = 0x01;
        auto swimmingBlocked = startWithMapMonsters(swimmingLevel);
        swimmingBlocked.boots[0] = true;
        failures += !check(swimmingBlocked.move(Direction::north) == MoveResult::blocked &&
                           swimmingBlocked.playerPosition == at(5, 5) &&
                           swimmingBlocked.floor(at(5, 5)) == 0x03 &&
                           swimmingBlocked.facing == Direction::north,
                           "blocked MoveChip updates facing while Chip keeps the swimming pose");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x02;
        level.top[at(3, 1)] = 0x15;
        level.chipsRequired = 1;
        auto game = startWithMapMonsters(level);
        failures += !check(game.move(Direction::east) == MoveResult::moved &&
                           game.playerPosition == at(2, 1) && game.chipsLeft == 0,
                           "chip pickup and counter");
        failures += !check(game.move(Direction::east) == MoveResult::won &&
                           game.outcome == Outcome::won,
                           "exit unlock and win");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x15;
        level.chipsRequired = 1;
        auto game = startWithMapMonsters(level);
        failures += !check(game.move(Direction::east) == MoveResult::won &&
                           game.outcome == Outcome::won && game.chipsLeft == 1,
                           "reaching the exit completes the level with chips remaining");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x64;
        level.top[at(3, 1)] = 0x16;
        level.top[at(4, 1)] = 0x65;
        level.top[at(5, 1)] = 0x17;
        level.top[at(6, 1)] = 0x67;
        level.top[at(7, 1)] = 0x19;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.keys[0] == 1, "blue key pickup");
        (void)game.move(Direction::east);
        failures += !check(game.keys[0] == 0 && game.surface(at(3, 1)) == 0,
                           "blue door consumes key");
        (void)game.move(Direction::east);
        failures += !check(game.keys[1] == 1, "red key pickup");
        (void)game.move(Direction::east);
        failures += !check(game.keys[1] == 0, "red door consumes key");
        (void)game.move(Direction::east);
        failures += !check(game.keys[3] == 1, "yellow key pickup");
        (void)game.move(Direction::east);
        failures += !check(game.keys[3] == 0, "yellow door consumes key");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x64;
        level.top[at(3, 1)] = 0x64;
        level.top[at(4, 1)] = 0x16;
        level.top[at(5, 1)] = 0x16;
        level.top[at(6, 1)] = 0x16;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        (void)game.move(Direction::east);
        failures += !check(game.keys[0] == 2,
                           "repeated same-color key pickups increment the original inventory count");
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(4, 1) && game.keys[0] == 1,
                           "first matching door consumes one key when Chip enters");
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(5, 1) && game.keys[0] == 0,
                           "second matching door consumes the remaining key");
        const auto blocked = game.move(Direction::east);
        failures += !check(blocked == MoveResult::blocked && game.playerPosition == at(5, 1) &&
                           game.keys[0] == 0,
                           "door blocks entry after matching keys are exhausted");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x64; // BlueKey overlays a ToggleButton.
        level.bottom[at(2, 1)] = 0x23;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.keys[0] == 1 && !game.toggleState &&
                           game.top[at(2, 1)] == 0x23 && game.bottom[at(2, 1)] == 0x23,
                           "pickup over a button collects the item without activating the underlay");
        (void)game.move(Direction::west);
        failures += !check(game.top[at(2, 1)] == 0x23 && game.bottom[at(2, 1)] == 0x00,
                           "leaving the pickup reveals its preserved button underlay");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x66;
        level.top[at(3, 1)] = 0x18;
        level.top[at(4, 1)] = 0x18;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        (void)game.move(Direction::east);
        (void)game.move(Direction::east);
        failures += !check(game.keys[2] == 1 && game.playerPosition == at(4, 1),
                           "green key remains reusable");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x21;
        auto game = startWithMapMonsters(level);
        game.keys = {2, 1, 3, 4};
        game.boots = {true, true, true, true};
        (void)game.move(Direction::east);
        failures += !check(game.keys == std::array<unsigned, 4>{2, 1, 3, 4},
                           "thief preserves every colored key");
        failures += !check(game.boots == std::array<bool, 4>{false, false, false, false},
                           "thief removes every boot type");
        failures += !check(game.top[at(2, 1)] == 0x21 && game.thiefActivatedThisTick,
                           "thief tile remains on the map after activation");
        game.boots[0] = true;
        (void)game.move(Direction::west);
        failures += !check(game.top[at(2, 1)] == 0x21,
                           "leaving an activated thief preserves the thief tile");
        (void)game.move(Direction::east);
        failures += !check(game.top[at(2, 1)] == 0x21 && game.thiefActivatedThisTick &&
                           game.boots == std::array<bool, 4>{false, false, false, false},
                           "re-entering the persistent thief activates it again");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x0A;
        level.top[at(3, 1)] = 0x03;
        level.bottom[at(3, 1)] = 0x03; // MoveBlock must preserve the destination underlay.
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.surface(at(3, 1)) == 0x0B && game.top[at(3, 1)] == 0x0B &&
                           game.bottom[at(3, 1)] == 0x03,
                           "block converts water to dirt and preserves its lower layer");
        (void)game.move(Direction::east);
        failures += !check(game.outcome == Outcome::playing && game.playerPosition == at(3, 1) &&
                           game.floor(at(3, 1)) == 0x03 && game.top[at(3, 1)] == 0x03 &&
                           game.bottom[at(3, 1)] == 0x03,
                           "Dirt over retained Water lets Chip enter without flippers, as in MoveChip");
        failures += !check(game.playerSpriteTile() == 0x3F,
                           "Chip switches to the original east-facing swimming sprite over Dirt/Water");
        (void)game.move(Direction::east);
        failures += !check(game.outcome == Outcome::playing && game.playerPosition == at(4, 1) &&
                           game.top[at(3, 1)] == 0x03 && game.bottom[at(3, 1)] == 0x00,
                           "leaving Dirt over Water restores Water over Floor");
        failures += !check(game.playerSpriteTile() == 0x6F,
                           "Chip returns to the original east-facing land sprite after leaving Water");
        (void)game.move(Direction::west);
        failures += !check(game.outcome == Outcome::dead && game.deathCause == DeathCause::water &&
                           game.playerPosition == at(3, 1),
                           "re-entering the restored water after using the block bridge drowns Chip");
        failures += !check(game.playerSpriteTile() == 0x3D,
                           "Chip keeps the original west-facing swimming sprite on water death");
        failures += !check(game.playerDisplayTile() == 0x33,
                           "water death displays the original ChipSplash tile at Chip's map position");

        auto firePushLevel = blankLevel();
        firePushLevel.top[at(1, 1)] = 0x6F;
        firePushLevel.top[at(2, 1)] = 0x0A;
        firePushLevel.top[at(3, 1)] = 0x04;
        auto firePush = startWithMapMonsters(firePushLevel);
        failures += !check(firePush.move(Direction::east) == MoveResult::moved &&
                           firePush.playerPosition == at(2, 1) && firePush.top[at(3, 1)] == 0x0A &&
                           firePush.bottom[at(3, 1)] == 0x04,
                           "pushed block remains over fire");

        auto fireSlideLevel = blankLevel();
        fireSlideLevel.top[at(7, 5)] = 0x6F;
        fireSlideLevel.top[at(5, 5)] = 0x0A;
        fireSlideLevel.bottom[at(5, 5)] = 0x0C;
        fireSlideLevel.top[at(5, 4)] = 0x04;
        auto fireSlide = startWithMapMonsters(fireSlideLevel);
        fireSlide.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::north);
        fireSlide.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        fireSlide.advanceTicks(1);
        failures += !check(fireSlide.top[at(5, 4)] == 0x0A && fireSlide.bottom[at(5, 4)] == 0x04 &&
                           fireSlide.slipOrder.empty(),
                           "sliding block remains over fire");

        auto waterSlideLevel = blankLevel();
        waterSlideLevel.top[at(7, 5)] = 0x6F;
        waterSlideLevel.top[at(5, 5)] = 0x0A;
        waterSlideLevel.bottom[at(5, 5)] = 0x0C;
        waterSlideLevel.top[at(5, 4)] = 0x03;
        waterSlideLevel.bottom[at(5, 4)] = 0x03;
        auto waterSlide = startWithMapMonsters(waterSlideLevel);
        waterSlide.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::north);
        waterSlide.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        waterSlide.advanceTicks(1);
        failures += !check(waterSlide.top[at(5, 4)] == 0x0B &&
                           waterSlide.bottom[at(5, 4)] == 0x03 && waterSlide.slipOrder.empty(),
                           "sliding block fills water with dirt and preserves its underlay");
    }
    {
        auto level = blankLevel();
        level.top[at(7, 5)] = 0x6F;
        level.top[at(5, 5)] = 0x0A;
        level.bottom[at(5, 5)] = 0x0C;
        level.top[at(6, 5)] = 0x0C;
        auto game = startWithMapMonsters(level);
        game.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        game.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        game.advanceTicks(2);
        failures += !check(game.outcome == Outcome::dead && game.playerPosition == at(7, 5) &&
                           game.top[at(7, 5)] == 0x0A && game.bottom[at(7, 5)] == 0x6F,
                           "sliding block squishes Chip on his visible sprite tile");
        failures += !check(game.move(Direction::west) == MoveResult::ignored,
                           "Chip cannot leave after a block squishes him");

        auto swimmingLevel = blankLevel();
        swimmingLevel.top[at(7, 5)] = 0x6F;
        swimmingLevel.bottom[at(7, 5)] = 0x03;
        swimmingLevel.top[at(5, 5)] = 0x0A;
        swimmingLevel.bottom[at(5, 5)] = 0x0C;
        swimmingLevel.top[at(6, 5)] = 0x0C;
        auto swimming = startWithMapMonsters(swimmingLevel);
        swimming.boots[0] = true;
        swimming.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        swimming.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        swimming.advanceTicks(2);
        failures += !check(swimming.outcome == Outcome::dead &&
                           swimming.top[at(7, 5)] == 0x0A && swimming.bottom[at(7, 5)] == 0x3F,
                           "sliding block copies Chip's swimming sprite under the block before crushing him");
    }
    {
        auto level = blankLevel();
        level.top[at(13, 5)] = 0x6F;
        level.top[at(5, 5)] = 0x0A;
        level.bottom[at(5, 5)] = 0x0C;
        level.top[at(6, 5)] = 0x29;
        level.top[at(12, 5)] = 0x29;
        auto game = startWithMapMonsters(level);
        game.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        game.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        game.advanceTicks(1);
        failures += !check(game.top[at(12, 5)] == 0x0A && game.bottom[at(12, 5)] == 0x29 &&
                           game.slipOrder.size() == 1,
                           "sliding block teleports toward a Chip-enterable exit square");
        game.advanceTicks(1);
        failures += !check(game.outcome == Outcome::dead && game.top[at(13, 5)] == 0x0A &&
                           game.bottom[at(13, 5)] == 0x6F && game.playerPosition == at(13, 5),
                           "teleported block squishes Chip on its exit square");
        failures += !check(game.move(Direction::north) == MoveResult::ignored,
                           "Chip cannot leave after a teleported block squishes him");
    }
    {
        auto level = blankLevel();
        level.top[at(7, 5)] = 0x6F;
        level.top[at(5, 5)] = 0x0A;
        level.bottom[at(5, 5)] = 0x0C;
        level.top[at(6, 5)] = 0x0C;
        auto game = startWithMapMonsters(level);
        game.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        game.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        game.advanceTicks(2);
        failures += !check(game.outcome == Outcome::dead && game.playerPosition == at(7, 5) &&
                           game.top[at(7, 5)] == 0x0A && game.bottom[at(7, 5)] == 0x6F,
                           "sliding block squishes Chip when landing over his copied sprite");
        failures += !check(game.move(Direction::north) == MoveResult::ignored,
                           "Chip cannot escape after a block squishes him");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x0A;
        level.top[at(3, 1)] = 0x2A;
        level.bottom[at(3, 1)] = 0x2D;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.outcome == Outcome::playing && game.playerPosition == at(2, 1) &&
                           game.top[at(3, 1)] == 0x00 && game.bottom[at(3, 1)] == 0x2D,
                           "pushed block is consumed by a bomb and leaves upper-layer floor");
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(3, 1) && game.outcome == Outcome::playing,
                           "Chip can enter the floor left by a consumed bomb block");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x0A;
        level.bottom[at(2, 1)] = 0x64;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(2, 1) && game.keys[0] == 1 &&
                           game.top[at(2, 1)] == 0x00 && game.bottom[at(2, 1)] == 0x00,
                           "pushing a block off a key collects it and clears the source square");
        (void)game.move(Direction::west);
        (void)game.move(Direction::east);
        failures += !check(game.keys[0] == 1,
                           "a key under a pushed block cannot be collected repeatedly");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x0A;
        level.top[at(3, 1)] = 0x64; // Blue key initially hidden by the pushed block.
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(2, 1) && game.keys[0] == 0 &&
                           game.top[at(3, 1)] == 0x0A && game.bottom[at(3, 1)] == 0x64,
                           "pushed block preserves a key beneath its destination without collecting it");
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(3, 1) && game.keys[0] == 1 &&
                           game.top[at(3, 1)] == 0x00 && game.top[at(4, 1)] == 0x0A,
                           "Chip collects the hidden key when the pushed block leaves its square");
    }
    {
        constexpr std::array<std::uint8_t, 8> itemTiles{
            0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B};
        for (const auto itemTile : itemTiles) {
            auto level = blankLevel();
            level.top[at(1, 1)] = 0x6F;
            level.top[at(2, 1)] = 0x0A;
            level.top[at(3, 1)] = itemTile;
            auto game = startWithMapMonsters(level);
            (void)game.move(Direction::east);
            failures += !check(game.playerPosition == at(2, 1) &&
                               std::all_of(game.keys.begin(), game.keys.end(), [](unsigned count) {
                                   return count == 0;
                               }) &&
                               std::none_of(game.boots.begin(), game.boots.end(), [](bool boot) {
                                   return boot;
                               }) &&
                               game.top[at(3, 1)] == 0x0A && game.bottom[at(3, 1)] == itemTile,
                               "pushed block preserves item 0x" + std::to_string(itemTile) +
                               " without collecting it");
            (void)game.move(Direction::east);
            const auto itemIndex = static_cast<std::size_t>(itemTile < 0x68
                ? itemTile - 0x64 : itemTile - 0x68);
            const bool collected = itemTile < 0x68
                ? game.keys[itemIndex] == 1
                : game.boots[itemIndex];
            failures += !check(game.playerPosition == at(3, 1) && collected &&
                               game.top[at(3, 1)] == 0x00 && game.bottom[at(3, 1)] == 0x00,
                               "Chip collects item 0x" + std::to_string(itemTile) +
                               " when the block vacates its square");
        }
    }
    {
        const auto makeSlidingBlockLevel = [] {
            auto level = blankLevel();
            level.top[at(4, 5)] = 0x6F;
            level.top[at(5, 5)] = 0x0A;
            level.bottom[at(5, 5)] = 0x0C;
            return level;
        };
        auto sameDirectionLevel = makeSlidingBlockLevel();
        auto sameDirection = startWithMapMonsters(sameDirectionLevel);
        sameDirection.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        sameDirection.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        failures += !check(sameDirection.move(Direction::east) == MoveResult::blocked &&
                           sameDirection.playerPosition == at(4, 5) &&
                           sameDirection.top[at(5, 5)] == 0x0A,
                           "Chip cannot push a block slipping in the same direction");

        auto oppositeDirectionLevel = makeSlidingBlockLevel();
        auto oppositeDirection = startWithMapMonsters(oppositeDirectionLevel);
        oppositeDirection.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::west);
        oppositeDirection.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        failures += !check(oppositeDirection.move(Direction::east) == MoveResult::blocked &&
                           oppositeDirection.playerPosition == at(4, 5) &&
                           oppositeDirection.top[at(5, 5)] == 0x0A,
                           "Chip cannot push a block slipping in the opposite direction");

        auto failedPushLevel = makeSlidingBlockLevel();
        failedPushLevel.top[at(6, 5)] = 0x01;
        auto failedPush = startWithMapMonsters(failedPushLevel);
        failedPush.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::north);
        failedPush.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        failures += !check(failedPush.move(Direction::east) == MoveResult::blocked &&
                           failedPush.playerPosition == at(4, 5) &&
                           failedPush.top[at(5, 5)] == 0x0A &&
                           failedPush.slipOrder.empty(),
                           "a blocked push removes the block slip entry after MoveBlock fails");

        auto panelPushLevel = blankLevel();
        panelPushLevel.top[at(5, 6)] = 0x6F;
        panelPushLevel.top[at(5, 5)] = 0x0A;
        panelPushLevel.bottom[at(5, 5)] = 0x06; // PanelN blocks northward exit.
        auto panelPush = startWithMapMonsters(panelPushLevel);
        panelPush.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        panelPush.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        failures += !check(panelPush.move(Direction::north) == MoveResult::blocked &&
                           panelPush.top[at(5, 5)] == 0x0A &&
                           panelPush.slipOrder.empty(),
                           "a panel-blocked push removes the block slip entry in MoveBlock cleanup");

        auto perpendicularLevel = makeSlidingBlockLevel();
        auto perpendicular = startWithMapMonsters(perpendicularLevel);
        perpendicular.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::north);
        perpendicular.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        failures += !check(perpendicular.move(Direction::east) == MoveResult::moved &&
                           perpendicular.playerPosition == at(5, 5) &&
                           perpendicular.top[at(6, 5)] == 0x0A &&
                           perpendicular.slipOrder.empty(),
                           "Chip can push a slipping block perpendicular to its motion");
    }
    {
        // SlideMovement's source switch handles each IceWall corner by the
        // incoming vector. The panel admission check blocks the two closed
        // approaches; the other two map to the exact forced directions below.
        constexpr int blocked = -1;
        constexpr std::array<std::array<int, 4>, 4> expected{{
            {{static_cast<int>(Direction::east), blocked, blocked,
              static_cast<int>(Direction::south)}}, // NW
            {{static_cast<int>(Direction::west), static_cast<int>(Direction::south),
              blocked, blocked}}, // NE
            {{blocked, static_cast<int>(Direction::north),
              static_cast<int>(Direction::west), blocked}}, // SE
            {{blocked, blocked, static_cast<int>(Direction::east),
              static_cast<int>(Direction::north)}} // SW
        }};
        constexpr std::array<std::uint8_t, 4> corners{0x1A, 0x1B, 0x1C, 0x1D};
        constexpr std::array<Direction, 4> incoming{
            Direction::north, Direction::east, Direction::south, Direction::west};
        for (std::size_t corner = 0; corner < corners.size(); ++corner) {
            for (std::size_t entry = 0; entry < incoming.size(); ++entry) {
                auto level = blankLevel();
                const int chipX = 10 - (incoming[entry] == Direction::east ? 1
                    : incoming[entry] == Direction::west ? -1 : 0);
                const int chipY = 10 - (incoming[entry] == Direction::south ? 1
                    : incoming[entry] == Direction::north ? -1 : 0);
                level.top[at(chipX, chipY)] = 0x6F;
                level.top[at(10, 10)] = corners[corner];
                auto game = startWithMapMonsters(level);
                const auto result = game.move(incoming[entry]);
                const int expectedDirection = expected[corner][entry];
                if (expectedDirection == blocked) {
                    failures += !check(result == MoveResult::blocked &&
                                       game.playerPosition == at(chipX, chipY) && !game.sliding,
                        "IceWall corner rejects its closed approach for tile " +
                        std::to_string(corners[corner]) + " and direction " +
                        std::to_string(entry));
                } else {
                    failures += !check(result == MoveResult::moved &&
                                       game.playerPosition == at(10, 10) && game.sliding &&
                                       static_cast<int>(game.slideDirection) == expectedDirection,
                        "Chip SlideMovement corner redirect matches source for tile " +
                        std::to_string(corners[corner]) + " and direction " +
                        std::to_string(entry));
                }
            }
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x0C;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(2, 1) && game.sliding,
                           "entering ice starts a separate per-tick slide phase");
        game.advanceTicks(1);
        failures += !check(game.playerPosition == at(3, 1), "ice advances one square per simulation tick");
        auto skated = startWithMapMonsters(level);
        skated.boots[2] = true;
        (void)skated.move(Direction::east);
        failures += !check(skated.playerPosition == at(2, 1), "ice skates stop forced motion");

        constexpr std::array<std::pair<std::uint8_t, Direction>, 5> iceSurfaces{{
            {0x0C, Direction::east}, {0x1A, Direction::north}, {0x1B, Direction::north},
            {0x1C, Direction::east}, {0x1D, Direction::south}
        }};
        constexpr std::array<std::uint8_t, 5> forceSurfaces{0x0D, 0x12, 0x13, 0x14, 0x32};
        const auto makeBootSurfaceLevel = [](std::uint8_t tile, Direction direction) {
            auto testLevel = blankLevel();
            const int sourceX = 10 - (direction == Direction::east ? 1 : direction == Direction::west ? -1 : 0);
            const int sourceY = 10 - (direction == Direction::south ? 1 : direction == Direction::north ? -1 : 0);
            testLevel.top[at(sourceX, sourceY)] = 0x6F;
            testLevel.top[at(10, 10)] = tile;
            return testLevel;
        };
        for (const auto [tile, direction] : iceSurfaces) {
            auto levelForTile = makeBootSurfaceLevel(tile, direction);
            auto withoutSkates = startWithMapMonsters(levelForTile);
            (void)withoutSkates.move(direction);
            auto withSkates = startWithMapMonsters(levelForTile);
            withSkates.boots[2] = true;
            (void)withSkates.move(direction);
            auto wrongBoot = startWithMapMonsters(levelForTile);
            wrongBoot.boots[3] = true;
            (void)wrongBoot.move(direction);
            failures += !check(withoutSkates.sliding && !withSkates.sliding && wrongBoot.sliding,
                               "IceSkates alone suppress forced movement on tile ID " + std::to_string(tile));
        }
        for (const auto tile : forceSurfaces) {
            auto levelForTile = makeBootSurfaceLevel(tile, Direction::east);
            auto withoutSuctionBoots = startWithMapMonsters(levelForTile);
            (void)withoutSuctionBoots.move(Direction::east);
            auto withSuctionBoots = startWithMapMonsters(levelForTile);
            withSuctionBoots.boots[3] = true;
            (void)withSuctionBoots.move(Direction::east);
            auto wrongBoot = startWithMapMonsters(levelForTile);
            wrongBoot.boots[2] = true;
            (void)wrongBoot.move(Direction::east);
            failures += !check(withoutSuctionBoots.sliding && !withSuctionBoots.sliding && wrongBoot.sliding,
                               "SuctionBoots alone suppress forced movement on tile ID " + std::to_string(tile));
        }

        auto bufferedLevel = blankLevel();
        bufferedLevel.top[at(1, 1)] = 0x6F;
        auto buffered = startWithMapMonsters(bufferedLevel);
        buffered.queueMove(Direction::east);
        buffered.advanceTicks(1);
        buffered.queueMove(Direction::south);
        failures += !check(buffered.playerPosition == at(2, 1) && buffered.chipHasMoved &&
                           buffered.pendingDirection == Direction::south,
                           "a second key is buffered after Chip moves on an odd tick");
        buffered.advanceTicks(1);
        failures += !check(buffered.playerPosition == at(2, 1) && !buffered.chipHasMoved &&
                           buffered.pendingDirection == Direction::south,
                           "the even tick clears the movement gate without consuming its buffer");
        buffered.advanceTicks(1);
        failures += !check(buffered.playerPosition == at(2, 2) && buffered.chipHasMoved &&
                           !buffered.pendingDirection,
                           "the buffered key executes on the next eligible odd tick");

        auto immediateLevel = blankLevel();
        immediateLevel.top[at(1, 1)] = 0x6F;
        auto immediate = startWithMapMonsters(immediateLevel);
        failures += !check(immediate.pressMove(Direction::east) == MoveResult::moved &&
                           immediate.playerPosition == at(2, 1) && immediate.chipHasMoved,
                           "physical movement input moves Chip immediately when the original turn gate is open");
        failures += !check(immediate.pressMove(Direction::south) == MoveResult::ignored &&
                           immediate.pressMove(Direction::west) == MoveResult::ignored &&
                           immediate.pendingDirection == Direction::south,
                           "physical input buffers only the first key while Chip has already moved");
        immediate.advanceTicks(2);
        failures += !check(immediate.playerPosition == at(2, 1) && !immediate.chipHasMoved,
                           "immediate movement input waits for the original even-tick gate reset");
        immediate.advanceTicks(1);
        failures += !check(immediate.playerPosition == at(2, 2) && !immediate.pendingDirection,
                           "the buffered physical key executes on the next eligible odd tick");

        auto immediateIceLevel = blankLevel();
        immediateIceLevel.top[at(3, 3)] = 0x6F;
        immediateIceLevel.bottom[at(3, 3)] = 0x0C;
        auto immediateIce = startWithMapMonsters(immediateIceLevel);
        immediateIce.sliding = true;
        immediateIce.slideDirection = Direction::east;
        failures += !check(immediateIce.pressMove(Direction::north) == MoveResult::ignored &&
                           immediateIce.playerPosition == at(3, 3) && immediateIce.sliding &&
                           immediateIce.chipHasMoved,
                           "a key press during an ice slide consumes the gate but cannot steer Chip");

        auto immediateForceLevel = blankLevel();
        immediateForceLevel.top[at(3, 3)] = 0x6F;
        immediateForceLevel.bottom[at(3, 3)] = 0x13;
        auto immediateForce = startWithMapMonsters(immediateForceLevel);
        immediateForce.sliding = true;
        immediateForce.slideDirection = Direction::east;
        failures += !check(immediateForce.pressMove(Direction::east) == MoveResult::ignored &&
                           immediateForce.playerPosition == at(3, 3) && immediateForce.sliding,
                           "a matching key press during forced-floor motion is redundant");
        auto steerForceLevel = blankLevel();
        steerForceLevel.top[at(3, 3)] = 0x6F;
        steerForceLevel.bottom[at(3, 3)] = 0x13;
        auto steerForce = startWithMapMonsters(steerForceLevel);
        steerForce.sliding = true;
        steerForce.slideDirection = Direction::east;
        failures += !check(steerForce.pressMove(Direction::north) == MoveResult::moved &&
                           steerForce.playerPosition == at(3, 2),
                           "a perpendicular key press can steer Chip off a force floor");

        auto longIce = blankLevel();
        longIce.top[at(1, 1)] = 0x6F;
        for (int x = 2; x <= 6; ++x) longIce.top[at(x, 1)] = 0x0C;
        auto ticked = startWithMapMonsters(longIce);
        ticked.queueMove(Direction::east);
        ticked.advanceTicks(2);
        failures += !check(ticked.playerPosition == at(4, 1) && ticked.sliding,
                           "odd-tick queued entry into ice slides in that same tick");
        ticked.advanceTicks(2);
        failures += !check(ticked.playerPosition == at(6, 1),
                           "an established slide advances once on each simulation tick");

        auto mouseDuringSlideLevel = blankLevel();
        mouseDuringSlideLevel.top[at(1, 3)] = 0x6F;
        mouseDuringSlideLevel.top[at(2, 3)] = 0x0C;
        mouseDuringSlideLevel.top[at(3, 3)] = 0x0C;
        auto mouseDuringSlide = startWithMapMonsters(mouseDuringSlideLevel);
        (void)mouseDuringSlide.pressMove(Direction::east);
        failures += !check(mouseDuringSlide.sliding && mouseDuringSlide.chipHasMoved &&
                           mouseDuringSlide.playerPosition == at(2, 3),
                           "a voluntary move onto ice starts the original slide input gate");
        mouseDuringSlide.setMouseTarget(at(4, 4));
        mouseDuringSlide.advanceTicks(1);
        failures += !check(mouseDuringSlide.playerPosition == at(3, 3) &&
                           mouseDuringSlide.sliding && mouseDuringSlide.mouseTargetPosition == at(4, 4),
                           "mouse movement waits when the post-slide ChipHasMoved gate is cleared");
        mouseDuringSlide.advanceTicks(1);
        failures += !check(mouseDuringSlide.playerPosition == at(4, 4) &&
                           !mouseDuringSlide.mouseTargetPosition,
                           "deferred mouse movement resumes after the original slide gate");

        auto steerLevel = blankLevel();
        steerLevel.top[at(1, 3)] = 0x6F;
        for (int x = 2; x <= 3; ++x) steerLevel.top[at(x, 3)] = 0x0C;
        auto steered = startWithMapMonsters(steerLevel);
        steered.gameTicks = 1;
        steered.queueMove(Direction::east);
        steered.advanceTicks(1);
        failures += !check(steered.playerPosition == at(3, 3) && steered.sliding,
                           "Chip starts sliding before the steering input arrives");
        steered.queueMove(Direction::north);
        steered.advanceTicks(1);
        failures += !check(steered.playerPosition == at(4, 2) && !steered.sliding &&
                           !steered.pendingDirection,
                           "buffered perpendicular input steers Chip immediately after a slide step");

        auto continueLevel = blankLevel();
        continueLevel.top[at(1, 3)] = 0x6F;
        for (int x = 2; x <= 6; ++x) continueLevel.top[at(x, 3)] = 0x0C;
        auto continueSlide = startWithMapMonsters(continueLevel);
        continueSlide.queueMove(Direction::east);
        continueSlide.advanceTicks(2);
        continueSlide.queueMove(Direction::east);
        continueSlide.advanceTicks(1);
        failures += !check(continueSlide.playerPosition == at(5, 3) && continueSlide.sliding &&
                           !continueSlide.pendingDirection,
                           "buffered input matching the slide direction is discarded");

        auto ignoredIceInput = startWithMapMonsters(continueLevel);
        ignoredIceInput.queueMove(Direction::east);
        ignoredIceInput.advanceTicks(2);
        ignoredIceInput.queueMove(Direction::north);
        ignoredIceInput.advanceTicks(1);
        failures += !check(ignoredIceInput.playerPosition == at(5, 3) && ignoredIceInput.sliding &&
                           !ignoredIceInput.pendingDirection,
                           "voluntary input cannot steer Chip while standing on ice");

        auto blockedSteerLevel = blankLevel();
        blockedSteerLevel.top[at(1, 3)] = 0x6F;
        for (int x = 2; x <= 6; ++x) blockedSteerLevel.top[at(x, 3)] = 0x13;
        blockedSteerLevel.top[at(5, 2)] = 0x01;
        auto blockedSteer = startWithMapMonsters(blockedSteerLevel);
        blockedSteer.queueMove(Direction::east);
        blockedSteer.advanceTicks(2);
        blockedSteer.queueMove(Direction::north);
        blockedSteer.advanceTicks(1);
        failures += !check(blockedSteer.playerPosition == at(5, 3) && blockedSteer.sliding &&
                           blockedSteer.slideDirection == Direction::east &&
                           !blockedSteer.pendingDirection,
                           "blocked steering input preserves the ongoing slide");

        auto reversalLevel = blankLevel();
        reversalLevel.top[at(1, 1)] = 0x6F;
        reversalLevel.top[at(2, 1)] = 0x0C;
        reversalLevel.top[at(3, 1)] = 0x01;
        auto reversal = startWithMapMonsters(reversalLevel);
        (void)reversal.move(Direction::east);
        reversal.advanceTicks(1);
        failures += !check(reversal.playerPosition == at(1, 1) && !reversal.sliding,
                           "blocked ice movement retries once in reverse and stops on ordinary floor");

        auto reverseOntoIce = blankLevel();
        reverseOntoIce.top[at(3, 5)] = 0x6F;
        reverseOntoIce.bottom[at(3, 5)] = 0x0C;
        reverseOntoIce.top[at(4, 5)] = 0x0C;
        reverseOntoIce.top[at(5, 5)] = 0x01;
        auto continuedReverse = startWithMapMonsters(reverseOntoIce);
        (void)continuedReverse.move(Direction::east);
        continuedReverse.advanceTicks(1);
        failures += !check(continuedReverse.playerPosition == at(3, 5) && continuedReverse.sliding &&
                           continuedReverse.slideDirection == Direction::west,
                           "successful ice reversal preserves its heading when the destination is ice");
        continuedReverse.advanceTicks(1);
        failures += !check(continuedReverse.playerPosition == at(2, 5) && !continuedReverse.sliding,
                           "reversed ice heading carries into the next simulation tick");

        auto blockedForceLevel = blankLevel();
        blockedForceLevel.top[at(1, 1)] = 0x6F;
        blockedForceLevel.top[at(2, 1)] = 0x13; // East force floor points at the wall.
        blockedForceLevel.top[at(3, 1)] = 0x01;
        auto blockedForce = startWithMapMonsters(blockedForceLevel);
        (void)blockedForce.move(Direction::east);
        blockedForce.advanceTicks(1);
        failures += !check(blockedForce.playerPosition == at(2, 1) && blockedForce.sliding &&
                           blockedForce.slideDirection == Direction::east,
                           "blocked force-floor slide retries and retains the forced heading");
        blockedForce.advanceTicks(1);
        failures += !check(blockedForce.playerPosition == at(2, 1) && blockedForce.sliding,
                           "blocked force-floor slide retries on subsequent ticks");

        auto randomForceLevel = blankLevel();
        randomForceLevel.top[at(1, 1)] = 0x6F;
        randomForceLevel.top[at(2, 1)] = 0x32;
        auto randomForce = startWithMapMonsters(randomForceLevel);
        (void)randomForce.move(Direction::east);
        randomForce.top[at(1, 1)] = 0x01;
        randomForce.top[at(3, 1)] = 0x01;
        randomForce.top[at(2, 2)] = 0x01;
        const auto stateBeforeBlockedRetries = randomForce.randomState;
        auto expectedRandomState = stateBeforeBlockedRetries;
        for (int retry = 0; retry < 2; ++retry)
            expectedRandomState = expectedRandomState * 214013U + 2531011U;
        randomForce.advanceTicks(1);
        failures += !check(randomForce.playerPosition == at(2, 1) && randomForce.sliding &&
                           randomForce.randomState == expectedRandomState,
                           "blocked random-force slide consumes both source retry draws");

        // Source SlideMovement calls RandInt(4), mapping 0,1,2,3 to
        // north,south,west,east. Exercise every result with seeds whose first
        // Microsoft CRT rand() value selects that result.
        constexpr std::array<std::pair<std::uint32_t, Direction>, 4> forceCases{{
            {3U, Direction::north}, {1U, Direction::south},
            {5U, Direction::west}, {4U, Direction::east},
        }};
        for (const auto& [seed, expectedDirection] : forceCases) {
            auto forceLevel = blankLevel();
            forceLevel.top[at(1, 2)] = 0x6F;
            forceLevel.top[at(2, 2)] = 0x32;
            auto force = GameState::start(forceLevel, 0, seed);
            (void)force.move(Direction::east);
            failures += !check(force.playerPosition == at(2, 2) && force.sliding &&
                               force.slideDirection == expectedDirection,
                               "random-force output follows original RandInt direction mapping");
        }

        auto blockedPortalLevel = blankLevel();
        blockedPortalLevel.top[at(1, 1)] = 0x6F;
        blockedPortalLevel.top[at(2, 1)] = 0x29; // Unpaired teleporter preserves the entered heading.
        blockedPortalLevel.top[at(3, 1)] = 0x01;
        auto blockedPortal = startWithMapMonsters(blockedPortalLevel);
        (void)blockedPortal.move(Direction::east);
        blockedPortal.advanceTicks(1);
        failures += !check(blockedPortal.playerPosition == at(1, 1) && !blockedPortal.sliding,
                           "blocked teleporter slide reverses through the portal to the previous square");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x0A;
        level.top[at(3, 1)] = 0x0C;
        level.top[at(4, 1)] = 0x0C;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        game.advanceTicks(1);
        failures += !check(game.playerPosition == at(2, 1) && game.top[at(4, 1)] == 0x0A &&
                           game.top[at(3, 1)] == 0x0C,
                           "pushed block slides once in the slip-list phase");
        game.advanceTicks(1);
        failures += !check(game.top[at(5, 1)] == 0x0A && game.top[at(4, 1)] == 0x0C,
                           "block momentum advances on the next simulation tick");

        auto layeredBlockLevel = blankLevel();
        layeredBlockLevel.top[at(1, 1)] = 0x6F;
        layeredBlockLevel.top[at(5, 5)] = 0x0A;
        layeredBlockLevel.bottom[at(5, 5)] = 0x0C;
        auto layeredBlock = startWithMapMonsters(layeredBlockLevel);
        layeredBlock.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        layeredBlock.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        layeredBlock.advanceTicks(1);
        failures += !check(layeredBlock.top[at(5, 5)] == 0x0C && layeredBlock.bottom[at(5, 5)] == 0x00 &&
                           layeredBlock.top[at(6, 5)] == 0x0A,
                           "sliding block resets the vacated lower layer to floor");

        auto forceLevel = blankLevel();
        forceLevel.top[at(1, 1)] = 0x6F;
        forceLevel.top[at(4, 1)] = 0x0A;
        forceLevel.bottom[at(4, 1)] = 0x13; // East force floor beneath a stationary block
        auto forced = startWithMapMonsters(forceLevel);
        forced.boots[3] = true; // Keep Chip still while the block is tested.
        forced.advanceTicks(5);
        failures += !check(forced.top[at(4, 1)] == 0x0A && forced.top[at(5, 1)] == 0x00,
                           "stationary block is not added to the slip list by a force floor");

        constexpr std::array<std::pair<std::uint8_t, Direction>, 4> forceArrows{{
            {0x0D, Direction::south}, {0x12, Direction::north},
            {0x13, Direction::east}, {0x14, Direction::west}}};
        for (const auto& [tile, expectedDirection] : forceArrows) {
            auto pushedForceLevel = blankLevel();
            pushedForceLevel.top[at(1, 1)] = 0x6F;
            pushedForceLevel.top[at(2, 1)] = 0x0A;
            pushedForceLevel.top[at(3, 1)] = tile; // Enter every arrow while pushing east.
            auto pushedForce = startWithMapMonsters(pushedForceLevel);
            pushedForce.boots[3] = true; // Chip's boots do not affect block motion.
            (void)pushedForce.move(Direction::east);
            failures += !check(pushedForce.blockDirections[at(3, 1)] ==
                                   static_cast<std::int8_t>(expectedDirection) &&
                               !pushedForce.slipOrder.empty(),
                               "pushed block takes force-floor arrow direction " +
                                   std::to_string(tile));
        }

        auto arrivingForceBlockLevel = blankLevel();
        arrivingForceBlockLevel.top[at(1, 1)] = 0x6F;
        arrivingForceBlockLevel.top[at(5, 5)] = 0x0A;
        arrivingForceBlockLevel.bottom[at(5, 5)] = 0x0C;
        arrivingForceBlockLevel.top[at(6, 5)] = 0x12;
        auto arrivingForceBlock = startWithMapMonsters(arrivingForceBlockLevel);
        arrivingForceBlock.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        arrivingForceBlock.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        arrivingForceBlock.advanceTicks(1);
        failures += !check(arrivingForceBlock.top[at(6, 5)] == 0x0A &&
                           arrivingForceBlock.blockDirections[at(6, 5)] ==
                               static_cast<std::int8_t>(Direction::north),
                           "sliding block changes heading on a directional force floor");

        auto cornerLevel = blankLevel();
        cornerLevel.top[at(3, 3)] = 0x6F;
        cornerLevel.top[at(3, 2)] = 0x0A;
        cornerLevel.top[at(3, 1)] = 0x1A; // Entered from the south while moving north.
        auto corner = startWithMapMonsters(cornerLevel);
        (void)corner.move(Direction::north);
        corner.advanceTicks(1);
        failures += !check(corner.top[at(4, 1)] == 0x0A && corner.top[at(3, 1)] == 0x1A,
                           "slipping block follows an ice corner onto its exit path");

        auto blockedIceBlockLevel = blankLevel();
        blockedIceBlockLevel.top[at(1, 1)] = 0x6F;
        blockedIceBlockLevel.top[at(5, 5)] = 0x0A;
        blockedIceBlockLevel.bottom[at(5, 5)] = 0x0C;
        blockedIceBlockLevel.top[at(6, 5)] = 0x01;
        auto blockedIceBlock = startWithMapMonsters(blockedIceBlockLevel);
        blockedIceBlock.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        blockedIceBlock.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        blockedIceBlock.advanceTicks(1);
        failures += !check(blockedIceBlock.top[at(4, 5)] == 0x0A &&
                           blockedIceBlock.top[at(5, 5)] == 0x0C &&
                           blockedIceBlock.slipOrder.empty(),
                           "sliding block retries backward immediately when blocked on ice");

        constexpr std::array<std::uint8_t, 4> blockedBlockCorners{0x1A, 0x1B, 0x1C, 0x1D};
        constexpr std::array<int, 4> blockDx{0, 1, 0, -1};
        constexpr std::array<int, 4> blockDy{-1, 0, 1, 0};
        for (const auto cornerTile : blockedBlockCorners) {
            for (int directionCode = 0; directionCode < 4; ++directionCode) {
                auto blockedCornerLevel = blankLevel();
                const int blockPosition = at(5, 5);
                blockedCornerLevel.top[at(1, 1)] = 0x6F;
                blockedCornerLevel.top[blockPosition] = 0x0A;
                blockedCornerLevel.bottom[blockPosition] = cornerTile;
                for (std::size_t side = 0; side < blockDx.size(); ++side) {
                    blockedCornerLevel.top[at(5 + blockDx[side], 5 + blockDy[side])] = 0x01;
                }
                auto blockedCornerBlock = startWithMapMonsters(blockedCornerLevel);
                blockedCornerBlock.blockDirections[blockPosition] = static_cast<std::int8_t>(directionCode);
                blockedCornerBlock.slipOrder.push_back({SlipperKind::block, blockPosition});
                blockedCornerBlock.advanceTicks(1);
                failures += !check(blockedCornerBlock.top[blockPosition] == 0x0A &&
                                   blockedCornerBlock.blockDirections[blockPosition] == directionCode &&
                                   blockedCornerBlock.slipOrder.size() == 1 &&
                                   blockedCornerBlock.slipOrder[0].entity == blockPosition,
                                   "doubly blocked sliding block restores its heading on ice corner " +
                                       std::to_string(cornerTile) + " from direction " +
                                       std::to_string(directionCode));
            }
        }

        constexpr std::array<std::array<std::uint8_t, 4>, 4> cornerRetryDirections{{
            {{0, 2, 1, 3}}, // IceWallNW
            {{0, 1, 3, 2}}, // IceWallNE
            {{3, 1, 2, 0}}, // IceWallSE
            {{1, 0, 2, 3}}  // IceWallSW
        }};
        for (std::size_t cornerIndex = 0; cornerIndex < blockedBlockCorners.size(); ++cornerIndex) {
            for (int directionCode = 0; directionCode < 4; ++directionCode) {
                auto retryCornerLevel = blankLevel();
                const int source = at(5, 5);
                const auto retryDirection = cornerRetryDirections[cornerIndex][static_cast<std::size_t>(directionCode)];
                retryCornerLevel.top[at(1, 1)] = 0x6F;
                retryCornerLevel.top[source] = 0x0A;
                retryCornerLevel.bottom[source] = blockedBlockCorners[cornerIndex];
                const int blockedX = 5 + blockDx[static_cast<std::size_t>(directionCode)];
                const int blockedY = 5 + blockDy[static_cast<std::size_t>(directionCode)];
                retryCornerLevel.top[at(blockedX, blockedY)] = 0x01;

                auto retryCornerBlock = startWithMapMonsters(retryCornerLevel);
                retryCornerBlock.blockDirections[source] = static_cast<std::int8_t>(directionCode);
                retryCornerBlock.slipOrder.push_back({SlipperKind::block, source});
                retryCornerBlock.advanceTicks(1);

                const bool redirected = retryDirection != directionCode;
                const int expectedPosition = redirected
                    ? at(5 + blockDx[retryDirection], 5 + blockDy[retryDirection]) : source;
                failures += !check(retryCornerBlock.top[expectedPosition] == 0x0A &&
                                   retryCornerBlock.blockDirections[expectedPosition] ==
                                       static_cast<std::int8_t>(redirected ? -1 : retryDirection) &&
                                   retryCornerBlock.top[source] == (redirected ? blockedBlockCorners[cornerIndex] : 0x0A) &&
                                   (redirected ? retryCornerBlock.slipOrder.empty()
                                               : retryCornerBlock.slipOrder.size() == 1),
                                   "blocked block corner retry follows SlideMovement transform for tile " +
                                       std::to_string(blockedBlockCorners[cornerIndex]) + " from direction " +
                                       std::to_string(directionCode));
            }
        }

        auto randomBlockLevel = blankLevel();
        randomBlockLevel.top[at(1, 1)] = 0x6F;
        randomBlockLevel.top[at(5, 5)] = 0x0A;
        randomBlockLevel.bottom[at(5, 5)] = 0x32;
        auto randomBlock = startWithMapMonsters(randomBlockLevel);
        randomBlock.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        randomBlock.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        randomBlock.advanceTicks(1);
        failures += !check(randomBlock.top[at(6, 5)] == 0x0A && randomBlock.randomState == 1U,
                           "block preserves its stored random-force heading without redrawing");

        auto blockedForceBlockLevel = blankLevel();
        blockedForceBlockLevel.top[at(1, 1)] = 0x6F;
        blockedForceBlockLevel.top[at(5, 5)] = 0x0A;
        blockedForceBlockLevel.bottom[at(5, 5)] = 0x13;
        blockedForceBlockLevel.top[at(6, 5)] = 0x01;
        auto blockedForceBlock = startWithMapMonsters(blockedForceBlockLevel);
        blockedForceBlock.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        blockedForceBlock.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        blockedForceBlock.advanceTicks(1);
        failures += !check(blockedForceBlock.top[at(5, 5)] == 0x0A &&
                           blockedForceBlock.blockDirections[at(5, 5)] ==
                               static_cast<std::int8_t>(Direction::east) &&
                           blockedForceBlock.slipOrder.size() == 1,
                           "blocked block on a directional force floor retains its slip state");
        blockedForceBlock.top[at(6, 5)] = 0x00;
        blockedForceBlock.advanceTicks(1);
        failures += !check(blockedForceBlock.top[at(6, 5)] == 0x0A &&
                           blockedForceBlock.slipOrder.empty(),
                           "force-floor block resumes after its path opens");

        constexpr std::array<std::pair<std::uint32_t, Direction>, 4> blockedRandomBlockCases{{
            {3U, Direction::north}, {1U, Direction::south},
            {5U, Direction::west}, {4U, Direction::east}
        }};
        for (const auto& [seed, expectedDirection] : blockedRandomBlockCases) {
            auto blockedRandomBlockLevel = blankLevel();
            const int source = at(5, 5);
            blockedRandomBlockLevel.top[at(1, 1)] = 0x6F;
            blockedRandomBlockLevel.top[source] = 0x0A;
            blockedRandomBlockLevel.bottom[source] = 0x32;
            blockedRandomBlockLevel.top[at(6, 5)] = 0x01;
            auto blockedRandomBlock = GameState::start(blockedRandomBlockLevel, 0, seed);
            blockedRandomBlock.blockDirections[source] = static_cast<std::int8_t>(Direction::east);
            blockedRandomBlock.slipOrder.push_back({SlipperKind::block, source});
            blockedRandomBlock.advanceTicks(1);
            const auto expectedRandomState = seed * 214013U + 2531011U;
            failures += !check(blockedRandomBlock.top[source] == 0x0A &&
                               blockedRandomBlock.blockDirections[source] ==
                                   static_cast<std::int8_t>(expectedDirection) &&
                               blockedRandomBlock.randomState == expectedRandomState &&
                               blockedRandomBlock.slipOrder.size() == 1 &&
                               blockedRandomBlock.slipOrder[0].entity == source,
                               "blocked random-force block consumes one CRT draw and retains the new vector");
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x03;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.outcome == Outcome::dead, "water kills without flippers");
        auto fireLevel = blankLevel();
        fireLevel.top[at(1, 1)] = 0x6F;
        fireLevel.top[at(2, 1)] = 0x04;
        auto burned = startWithMapMonsters(fireLevel);
        (void)burned.move(Direction::east);
        failures += !check(burned.outcome == Outcome::dead && burned.playerDisplayTile() == 0x34,
                           "fire death displays the original ChipBurned tile at Chip's map position");
        auto swimmer = startWithMapMonsters(level);
        swimmer.boots[0] = true;
        (void)swimmer.move(Direction::east);
        failures += !check(swimmer.outcome == Outcome::playing, "flippers protect from water");
    }
    {
        auto level = blankLevel();
        level.timeLimit = 1;
        level.top[at(1, 1)] = 0x6F;
        auto game = startWithMapMonsters(level);
        game.advanceTicks(9);
        failures += !check(game.outcome == Outcome::playing && game.secondsLeft == 1,
                           "timer does not expire before ten ticks");
        game.advanceTicks();
        failures += !check(game.outcome == Outcome::timedOut && game.secondsLeft == 0,
                           "timer expires at ten ticks per second");

        auto simultaneousWinLevel = blankLevel();
        simultaneousWinLevel.timeLimit = 1;
        simultaneousWinLevel.top[at(1, 1)] = 0x6F;
        simultaneousWinLevel.top[at(2, 1)] = 0x15;
        auto simultaneousWin = startWithMapMonsters(simultaneousWinLevel);
        simultaneousWin.gameTicks = 9;
        simultaneousWin.queueMove(Direction::east);
        simultaneousWin.advanceTicks(1);
        failures += !check(simultaneousWin.outcome == Outcome::timedOut && simultaneousWin.secondsLeft == 0,
                           "timeout phase after a same-tick win takes precedence");

        auto simultaneousDeathLevel = blankLevel();
        simultaneousDeathLevel.timeLimit = 1;
        simultaneousDeathLevel.top[at(1, 1)] = 0x6F;
        simultaneousDeathLevel.top[at(2, 1)] = 0x03;
        auto simultaneousDeath = startWithMapMonsters(simultaneousDeathLevel);
        simultaneousDeath.gameTicks = 9;
        simultaneousDeath.queueMove(Direction::east);
        simultaneousDeath.advanceTicks(1);
        failures += !check(simultaneousDeath.outcome == Outcome::timedOut && simultaneousDeath.secondsLeft == 0,
                           "timeout phase after a same-tick hazard death takes precedence");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x40;
        auto game = startWithMapMonsters(level);
        failures += !check(game.creatures.size() == 1, "creature extracted from map layer");
        game.gameTicks = 1; // The next DoTick is even and runs MonsterLoop.
        game.advanceTicks(1);
        failures += !check(game.creatures[0].position == at(4, 5),
                           "bug takes its left turn on an even simulation tick");
        game.advanceTicks(2);
        failures += !check(game.creatures[0].position == at(4, 6),
                           "ordinary creatures wait through the odd tick before the next MonsterLoop pass");
        game.gameTicks = 3;
        game.randomState = 0x12345678U;
        game.restart();
        failures += !check(game.creatures[0].position == at(5, 5) && game.gameTicks == 3 &&
                           game.randomState == 0x12345678U,
                           "restart resets level state while retaining global tick and random phases");
        game.advanceTicks(1);
        failures += !check(game.gameTicks == 4 && game.creatures[0].position == at(4, 5),
                           "restarted level continues the global even-tick creature cadence");
    }
    {
        constexpr std::array<int, 4> dx{0, -1, 0, 1};
        constexpr std::array<int, 4> dy{-1, 0, 1, 0};
        for (int facing = 0; facing < 4; ++facing) {
            const std::array<int, 4> tries{
                (facing + 1) & 3, facing, (facing + 3) & 3, (facing + 2) & 3};
            for (int fallback = 0; fallback < 4; ++fallback) {
                auto level = blankLevel();
                level.top[at(1, 1)] = 0x6F;
                const int start = at(5, 5);
                level.top[static_cast<std::size_t>(start)] = static_cast<std::uint8_t>(0x40 + facing);
                for (const int direction : tries) {
                    const int neighbor = start + dy[static_cast<std::size_t>(direction)] * kBoardWidth +
                                         dx[static_cast<std::size_t>(direction)];
                    level.top[static_cast<std::size_t>(neighbor)] = 0x01;
                }
                const int chosenDirection = tries[static_cast<std::size_t>(fallback)];
                const int destination = start + dy[static_cast<std::size_t>(chosenDirection)] * kBoardWidth +
                                        dx[static_cast<std::size_t>(chosenDirection)];
                level.top[static_cast<std::size_t>(destination)] = 0x00;
                auto game = startWithMapMonsters(level);
                game.gameTicks = 1;
                game.advanceTicks(1);
                failures += !check(game.creatures.size() == 1 &&
                                   game.creatures[0].position == destination &&
                                   game.creatures[0].tile == static_cast<std::uint8_t>(0x40 + chosenDirection),
                    "bug fallback follows left/straight/right/reverse order for facing " +
                    std::to_string(facing) + " attempt " + std::to_string(fallback));
            }
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x43; // east-facing bug starts over ice
        level.bottom[at(5, 5)] = 0x0C;
        auto game = startWithMapMonsters(level);
        failures += !check(game.slipOrder.empty(),
                           "a creature initialized on ice does not begin with an active slip entry");
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.creatures[0].position == at(5, 4) && game.slipOrder.empty(),
                           "a creature starting on ice takes its ordinary AI turn before it can slip");
    }
    {
        auto level = blankLevel();
        level.top[at(3, 3)] = 0x6F;
        level.bottom[at(3, 3)] = 0x0C; // Chip is sliding east on ice.
        level.top[at(2, 3)] = 0x4F; // East-facing tank catches him first.
        auto game = startWithMapMonsters(level);
        game.sliding = true;
        game.slideDirection = Direction::east;
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.outcome == Outcome::dead && game.playerPosition == at(4, 3),
                           "DoTick completes Chip's forced slide after MonsterLoop records Autopsy");
        failures += !check(game.move(Direction::north) == MoveResult::ignored,
                           "ordinary player input remains disabled after the death tick");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x40; // Bug moves on even ticks.
        auto game = startWithMapMonsters(level);
        game.gameTicks = 0xFFFFU;
        game.advanceTicks(1);
        failures += !check(game.gameTicks == 0 && game.creatures[0].position == at(4, 5),
                           "global 16-bit tick wrap preserves the original even-tick creature phase");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(8, 8)] = 0x54; // Teeth pursues north on tick values divisible by four.
        level.top[at(12, 12)] = 0x5C; // Blob also takes its gated random turn at tick zero.
        auto game = startWithMapMonsters(level);
        game.gameTicks = 0xFFFEU;
        game.advanceTicks(2); // Odd 0xFFFF, then even 0 after the 16-bit wrap.
        failures += !check(game.gameTicks == 0 && game.creatures.size() == 2 &&
                           game.creatures[0].position == at(8, 7) &&
                           game.creatures[1].position != at(12, 12),
                           "Teeth and Blob run their tick-zero gate when the global tick wraps");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        auto game = GameState::start(level);
        game.gameTicks = 0xFFFFU;
        game.secondsLeft = 7;
        game.secondsElapsed = 4;
        game.advanceTicks(1);
        failures += !check(game.gameTicks == 0 && game.secondsLeft == 7 && game.secondsElapsed == 4,
                           "timer skips the 16-bit tick-zero wrap exactly as source DoTick does");
        game.advanceTicks(9); // Tick values 1 through 9 after wrap.
        failures += !check(game.gameTicks == 9 && game.secondsLeft == 7 && game.secondsElapsed == 4,
                           "timer remains unchanged until the next nonzero multiple of ten");
        game.advanceTicks(1);
        failures += !check(game.gameTicks == 10 && game.secondsLeft == 6 && game.secondsElapsed == 5,
                           "timer resumes its normal one-second decrement at tick ten after wrap");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(4, 4)] = 0x50; // Glider facing north
        level.top[at(4, 3)] = 0x03;
        auto game = startWithMapMonsters(level);
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.creatures[0].alive && game.creatures[0].position == at(4, 3),
                           "glider crosses water");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x40; // Bug turns west into water and dies first.
        level.top[at(4, 5)] = 0x03;
        level.top[at(8, 8)] = 0x4F; // Following east-facing Tank still gets this turn.
        auto game = startWithMapMonsters(level);
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.creatures.size() == 2 && !game.creatures[0].alive &&
                           game.creatures[0].position == at(4, 5) &&
                           game.creatures[1].alive && game.creatures[1].position == at(9, 8),
                           "removing a creature during MonsterLoop does not skip the next creature's turn");
    }
    {
        auto fireballLevel = blankLevel();
        fireballLevel.top[at(1, 1)] = 0x6F;
        fireballLevel.top[at(5, 5)] = 0x44; // North-facing Fireball.
        fireballLevel.top[at(5, 4)] = 0x01; // Straight is blocked; right is open.
        auto fireball = startWithMapMonsters(fireballLevel);
        fireball.gameTicks = 1;
        fireball.advanceTicks(1);
        failures += !check(fireball.creatures[0].position == at(6, 5),
                           "fireball tries straight then turns right");

        auto ballLevel = blankLevel();
        ballLevel.top[at(1, 1)] = 0x6F;
        ballLevel.top[at(5, 5)] = 0x48; // North-facing Ball.
        ballLevel.top[at(5, 4)] = 0x01; // Straight is blocked, so it reverses.
        auto ball = startWithMapMonsters(ballLevel);
        ball.gameTicks = 1;
        ball.advanceTicks(1);
        failures += !check(ball.creatures[0].position == at(5, 6),
                           "ball reverses only after its forward move is blocked");

        auto gliderTurnLevel = blankLevel();
        gliderTurnLevel.top[at(1, 1)] = 0x6F;
        gliderTurnLevel.top[at(5, 5)] = 0x50; // North-facing Glider.
        gliderTurnLevel.top[at(5, 4)] = 0x01; // Straight and left are blocked.
        gliderTurnLevel.top[at(4, 5)] = 0x01;
        auto gliderTurn = startWithMapMonsters(gliderTurnLevel);
        gliderTurn.gameTicks = 1;
        gliderTurn.advanceTicks(1);
        failures += !check(gliderTurn.creatures[0].position == at(6, 5),
                           "glider tries straight, left, then right");

        auto blockedGliderLevel = blankLevel();
        blockedGliderLevel.top[at(1, 1)] = 0x6F;
        blockedGliderLevel.top[at(5, 5)] = 0x50; // North-facing Glider.
        for (const auto position : {at(5, 4), at(4, 5), at(6, 5), at(5, 6)})
            blockedGliderLevel.top[static_cast<std::size_t>(position)] = 0x01;
        auto blockedGlider = startWithMapMonsters(blockedGliderLevel);
        blockedGlider.gameTicks = 1;
        blockedGlider.advanceTicks(1);
        failures += !check(blockedGlider.creatures[0].position == at(5, 5) &&
                           blockedGlider.creatures[0].tile == 0x52,
                           "glider attempts reverse last and retains its final attempted facing when blocked");

        auto constrainedGliderLevel = blankLevel();
        constrainedGliderLevel.top[at(1, 1)] = 0x6F;
        constrainedGliderLevel.top[at(2, 1)] = 0x24;
        constrainedGliderLevel.top[at(5, 5)] = 0x50;
        constrainedGliderLevel.bottom[at(5, 5)] = 0x31;
        OptionalField constrainedGliderLink;
        constrainedGliderLink.id = 5;
        constrainedGliderLink.raw = {2, 0, 1, 0, 5, 0, 5, 0};
        constrainedGliderLevel.fields.push_back(constrainedGliderLink);
        auto constrainedGlider = startWithMapMonsters(constrainedGliderLevel);
        (void)constrainedGlider.move(Direction::east);
        constrainedGlider.top[at(5, 4)] = 0x01;
        constrainedGlider.gameTicks = 1;
        constrainedGlider.advanceTicks(1);
        failures += !check(constrainedGlider.creatures[0].position == at(5, 5) &&
                           constrainedGlider.creatures[0].tile == 0x50,
                           "glider on a clone machine does not turn after a blocked forward attempt");
    }
    {
        struct MovementOrder {
            std::uint8_t kind;
            std::array<int, 4> relativeTurns;
            int count;
            const char* name;
        };
        // Direction codes use the original N/W/S/E order. Relative offsets
        // mirror TurnLeft (+1), TurnRight (+3), and TurnAround (+2).
        constexpr std::array<MovementOrder, 5> orders{{
            {0x40, {{1, 0, 3, 2}}, 4, "Bug"},
            {0x44, {{0, 3, 1, 2}}, 4, "Fireball"},
            {0x48, {{0, 2, 0, 0}}, 2, "Ball"},
            {0x50, {{0, 1, 3, 2}}, 4, "Glider"},
            {0x60, {{3, 0, 1, 2}}, 4, "Paramecium"},
        }};
        const int source = at(10, 10);
        for (const auto& order : orders) {
            for (int facing = 0; facing < 4; ++facing) {
                for (int attempt = 0; attempt < order.count; ++attempt) {
                    auto level = blankLevel();
                    level.top[at(1, 1)] = 0x6F;
                    level.top[static_cast<std::size_t>(source)] =
                        static_cast<std::uint8_t>(order.kind | facing);
                    for (int direction = 0; direction < 4; ++direction)
                        level.top[static_cast<std::size_t>(neighborForMonsterDirection(source, direction))] = 0x01;
                    const int expectedDirection =
                        (facing + order.relativeTurns[static_cast<std::size_t>(attempt)]) & 3;
                    const int destination = neighborForMonsterDirection(source, expectedDirection);
                    level.top[static_cast<std::size_t>(destination)] = 0x00;
                    auto game = startWithMapMonsters(level);
                    game.gameTicks = 1;
                    game.advanceTicks(1);
                    failures += !check(game.creatures[0].position == destination &&
                                       game.creatures[0].tile == (order.kind | expectedDirection) &&
                                       game.creatures[0].movementDirection == expectedDirection,
                                       std::string(order.name) + " follows source fallback order from facing " +
                                       std::to_string(facing));
                }
            }
        }

        for (int facing = 0; facing < 4; ++facing) {
            auto blockedLevel = blankLevel();
            blockedLevel.top[at(1, 1)] = 0x6F;
            blockedLevel.top[static_cast<std::size_t>(source)] =
                static_cast<std::uint8_t>(0x40 | facing);
            for (int direction = 0; direction < 4; ++direction)
                blockedLevel.top[static_cast<std::size_t>(neighborForMonsterDirection(source, direction))] = 0x01;
            auto game = startWithMapMonsters(blockedLevel);
            game.gameTicks = 1;
            game.advanceTicks(1);
            const int initialDirection = facing;
            const int lastAttemptDirection = (facing + 2) & 3;
            failures += !check(game.creatures[0].position == source &&
                               game.creatures[0].tile == (0x40 | lastAttemptDirection) &&
                               game.creatures[0].movementDirection == initialDirection,
                               "fully blocked Bug retains source sprite facing without changing its stored vector");
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(3, 3)] = 0x5B; // east-facing Walker
        level.top[at(4, 3)] = 0x3C; // swimming-Chip overlay on water
        level.bottom[at(4, 3)] = 0x03;
        auto game = startWithMapMonsters(level);
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(!game.creatures[0].alive && game.creatures[0].position == at(4, 3) &&
                           game.top[at(4, 3)] == 0x3C,
                           "drowned creature state retains its attempted destination");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x43; // Bug facing east: first turns north.
        level.top[at(5, 4)] = 0x01;
        level.top[at(6, 5)] = 0x04; // Bugs must treat fire as a wall.
        auto game = startWithMapMonsters(level);
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.creatures[0].alive && game.creatures[0].position == at(5, 6),
                           "bug avoids fire and uses its next available turn");

        level.top[at(5, 5)] = 0x5B; // Walker facing east.
        auto walker = startWithMapMonsters(level);
        walker.advanceTicks(2);
        failures += !check(walker.creatures[0].alive && walker.creatures[0].position != at(6, 5),
                           "walker cannot enter fire");
    }
    {
        auto rightBlocked = blankLevel();
        rightBlocked.top[at(1, 1)] = 0x6F;
        rightBlocked.top[at(5, 5)] = 0x60; // north-facing Paramecium
        rightBlocked.top[at(6, 5)] = 0x01; // right is tried before straight
        auto paraForward = startWithMapMonsters(rightBlocked);
        paraForward.gameTicks = 1;
        paraForward.advanceTicks(1);
        failures += !check(paraForward.creatures[0].position == at(5, 4) &&
                           (paraForward.creatures[0].tile & 0x03) == 0,
                           "Paramecium tries right before straight and succeeds on its forward fallback");

        auto firstTwoBlocked = blankLevel();
        firstTwoBlocked.top[at(1, 1)] = 0x6F;
        firstTwoBlocked.top[at(5, 5)] = 0x60;
        firstTwoBlocked.top[at(6, 5)] = 0x01; // right
        firstTwoBlocked.top[at(5, 4)] = 0x01; // straight
        auto paraLeft = startWithMapMonsters(firstTwoBlocked);
        paraLeft.gameTicks = 1;
        paraLeft.advanceTicks(1);
        failures += !check(paraLeft.creatures[0].position == at(4, 5) &&
                           (paraLeft.creatures[0].tile & 0x03) == 1,
                           "Paramecium tries left after right and straight are blocked");
    }
    {
        auto bombLevel = blankLevel();
        bombLevel.top[at(1, 1)] = 0x6F;
        bombLevel.top[at(5, 5)] = 0x4F; // east-facing tank
        bombLevel.top[at(6, 5)] = 0x2A;
        bombLevel.bottom[at(6, 5)] = 0x0C; // unusual but valid retained underlay
        auto tank = startWithMapMonsters(bombLevel);
        tank.advanceTicks(2);
        failures += !check(!tank.creatures[0].alive && tank.creatures[0].position == at(6, 5) &&
                           tank.top[at(6, 5)] == 0x00 && tank.bottom[at(6, 5)] == 0x0C,
                           "ordinary creature bomb action leaves visible floor over the retained underlay");

        auto slidingBombLevel = blankLevel();
        slidingBombLevel.top[at(1, 1)] = 0x6F;
        slidingBombLevel.top[at(5, 5)] = 0x43; // east-facing bug
        slidingBombLevel.bottom[at(5, 5)] = 0x0C;
        slidingBombLevel.top[at(6, 5)] = 0x2A;
        slidingBombLevel.bottom[at(6, 5)] = 0x0C;
        auto slidingBug = startWithMapMonsters(slidingBombLevel);
        slidingBug.slipOrder.push_back({SlipperKind::creature, 0});
        slidingBug.advanceTicks(1);
        failures += !check(!slidingBug.creatures[0].alive &&
                           slidingBug.creatures[0].position == at(6, 5) &&
                           slidingBug.top[at(6, 5)] == 0x00 && slidingBug.bottom[at(6, 5)] == 0x0C &&
                           slidingBug.slipOrder.empty(),
                           "slipping creature bomb action leaves visible floor over the retained underlay");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(8, 8)] = 0x54; // Teeth facing north
        auto game = startWithMapMonsters(level);
        game.playerPosition = at(8, 10);
        game.advanceTicks(4);
        failures += !check(game.creatures[0].position == at(8, 9),
                           "teeth move on their tick-mod-four turn");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x54;
        auto diagonal = startWithMapMonsters(level);
        diagonal.playerPosition = at(7, 7);
        diagonal.advanceTicks(4);
        failures += !check(diagonal.creatures[0].position == at(5, 6),
                           "teeth choose vertical pursuit when axis distances tie");

        level.top[at(5, 6)] = 0x01;
        level.top[at(6, 5)] = 0x01;
        auto blocked = startWithMapMonsters(level);
        blocked.playerPosition = at(7, 7);
        blocked.advanceTicks(4);
        failures += !check(blocked.creatures[0].position == at(5, 5) &&
                           (blocked.creatures[0].tile & 0x03) == 2,
                           "teeth face south, the preferred vertical axis, after both tied pursuit steps are blocked");

        auto coincident = blankLevel();
        coincident.top[at(1, 1)] = 0x6F;
        coincident.top[at(8, 8)] = 0x57; // east-facing Teeth
        auto overlapping = startWithMapMonsters(coincident);
        overlapping.playerPosition = at(8, 8);
        overlapping.gameTicks = 3;
        overlapping.advanceTicks(1);
        failures += !check(overlapping.creatures[0].position == at(8, 7) &&
                           (overlapping.creatures[0].tile & 0x03) == 0,
                           "coincident Teeth and Chip use the source's northward zero-distance pursuit fallback");
    }
    {
        // Pursuit and blocked-direction fallback use signed Chip-relative
        // distances. Cover all quadrants and both axis-priority outcomes.
        constexpr int teethX = 10;
        constexpr int teethY = 10;
        for (const int dxMagnitude : {1, 2, 3}) {
            for (const int dyMagnitude : {1, 2, 3}) {
                if (dxMagnitude == dyMagnitude) continue; // tie has its own regression above
                for (const int xSign : {-1, 1}) {
                    for (const int ySign : {-1, 1}) {
                        const int dx = dxMagnitude * xSign;
                        const int dy = dyMagnitude * ySign;
                        const int horizontal = dx < 0 ? 1 : 3; // tile direction W/E
                        const int vertical = dy < 0 ? 0 : 2;   // tile direction N/S
                        const int preferred = dyMagnitude > dxMagnitude ? vertical : horizontal;
                        const int alternate = preferred == vertical ? horizontal : vertical;
                        const int playerX = teethX + dx;
                        const int playerY = teethY + dy;

                        auto fallbackLevel = blankLevel();
                        fallbackLevel.top[at(1, 1)] = 0x6F;
                        fallbackLevel.top[at(teethX, teethY)] = 0x54;
                        fallbackLevel.top[static_cast<std::size_t>(neighborForMonsterDirection(
                            at(teethX, teethY), preferred))] = 0x01;
                        auto fallback = startWithMapMonsters(fallbackLevel);
                        fallback.playerPosition = at(playerX, playerY);
                        fallback.gameTicks = 3;
                        fallback.advanceTicks(1);
                        const int fallbackPosition = neighborForMonsterDirection(at(teethX, teethY), alternate);
                        failures += !check(fallback.creatures[0].position == fallbackPosition &&
                                           (fallback.creatures[0].tile & 0x03) == alternate,
                                           "Teeth pursues the alternate axis after the preferred step is blocked");

                        fallbackLevel.top[static_cast<std::size_t>(neighborForMonsterDirection(
                            at(teethX, teethY), alternate))] = 0x01;
                        auto blocked = startWithMapMonsters(fallbackLevel);
                        blocked.playerPosition = at(playerX, playerY);
                        blocked.gameTicks = 3;
                        blocked.advanceTicks(1);
                        failures += !check(blocked.creatures[0].position == at(teethX, teethY) &&
                                           (blocked.creatures[0].tile & 0x03) == preferred,
                                           "Teeth reset to the preferred axis when both pursuit steps are blocked");
                    }
                }
            }
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(3, 3)] = 0x40; // bug turns left (west) onto the button
        level.top[at(2, 3)] = 0x23;
        level.top[at(8, 8)] = 0x25;
        auto game = startWithMapMonsters(level);
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.creatures[0].position == at(2, 3) && game.toggleState &&
                           game.top[at(8, 8)] == 0x26 && game.bottom[at(2, 3)] == 0x23,
                           "creature activates a toggle button it enters");
        failures += !check(game.surface(at(3, 3)) == 0x00 && game.top[at(3, 3)] == 0x00,
                           "creature departure restores the original floor tile without a ghost sprite");
        game.advanceTicks(2);
        failures += !check(game.creatures[0].position == at(2, 4) &&
                           game.bottom[at(2, 3)] == 0x00,
                           "creature leaves its previous tile layer as floor");

        auto collisionLevel = blankLevel();
        collisionLevel.top[at(2, 3)] = 0x6F;
        collisionLevel.bottom[at(2, 3)] = 0x23;
        collisionLevel.top[at(2, 4)] = 0x4C; // tank enters the button square from the south
        collisionLevel.top[at(8, 8)] = 0x25;
        auto collision = startWithMapMonsters(collisionLevel);
        collision.gameTicks = 1;
        collision.advanceTicks(1);
        failures += !check(collision.outcome == Outcome::dead && collision.toggleState &&
                           collision.top[at(8, 8)] == 0x26,
                           "creature activates Chip's toggle button before post-move capture");

        auto slidingCollisionLevel = blankLevel();
        slidingCollisionLevel.top[at(2, 3)] = 0x6F;
        slidingCollisionLevel.bottom[at(2, 3)] = 0x23;
        slidingCollisionLevel.top[at(3, 3)] = 0x41; // west-sliding bug enters the button
        slidingCollisionLevel.bottom[at(3, 3)] = 0x0C;
        slidingCollisionLevel.top[at(8, 8)] = 0x25;
        auto slidingCollision = startWithMapMonsters(slidingCollisionLevel);
        slidingCollision.slipOrder.push_back({SlipperKind::creature, 0});
        slidingCollision.advanceTicks(1);
        failures += !check(slidingCollision.outcome == Outcome::dead &&
                           slidingCollision.toggleState && slidingCollision.top[at(8, 8)] == 0x26,
                           "slipping creature activates Chip's toggle button before post-move capture");

        auto cloneButtonSlipLevel = blankLevel();
        cloneButtonSlipLevel.top[at(1, 1)] = 0x6F;
        cloneButtonSlipLevel.top[at(5, 5)] = 0x43; // east-facing Bug already sliding
        cloneButtonSlipLevel.bottom[at(5, 5)] = 0x0C;
        cloneButtonSlipLevel.top[at(6, 5)] = 0x24;
        cloneButtonSlipLevel.top[at(9, 9)] = 0x4C; // north-facing Tank clone template
        cloneButtonSlipLevel.bottom[at(9, 9)] = 0x31;
        OptionalField cloneButtonSlipLink;
        cloneButtonSlipLink.id = 5;
        cloneButtonSlipLink.raw = {6, 0, 5, 0, 9, 0, 9, 0};
        cloneButtonSlipLevel.fields.push_back(cloneButtonSlipLink);
        auto cloneButtonSlip = startWithMapMonsters(cloneButtonSlipLevel);
        cloneButtonSlip.slipOrder.push_back({SlipperKind::creature, 0});
        cloneButtonSlip.advanceTicks(1);
        failures += !check(cloneButtonSlip.creatures.size() == 2 &&
                           cloneButtonSlip.creatures[0].position == at(6, 5) &&
                           cloneButtonSlip.creatures[1].position == at(9, 9) &&
                           cloneButtonSlip.creatures[1].tile == 0x4C &&
                           cloneButtonSlip.slipOrder.empty(),
                           "slipping creature activates CloneButton and the new clone waits for a later pass");

        auto trapButtonSlipLevel = blankLevel();
        trapButtonSlipLevel.top[at(1, 1)] = 0x6F;
        trapButtonSlipLevel.top[at(5, 5)] = 0x43;
        trapButtonSlipLevel.bottom[at(5, 5)] = 0x0C;
        trapButtonSlipLevel.top[at(6, 5)] = 0x27;
        OptionalField trapButtonSlipLink;
        trapButtonSlipLink.id = 4;
        trapButtonSlipLink.raw = {6, 0, 5, 0, 9, 0, 9, 0, 1, 0};
        trapButtonSlipLevel.fields.push_back(trapButtonSlipLink);
        auto trapButtonSlip = startWithMapMonsters(trapButtonSlipLevel);
        trapButtonSlip.slipOrder.push_back({SlipperKind::creature, 0});
        trapButtonSlip.advanceTicks(1);
        failures += !check(trapButtonSlip.creatures[0].position == at(6, 5) &&
                           trapButtonSlip.traps.size() == 1 && !trapButtonSlip.traps[0].closed,
                           "slipping creature opens its linked TrapButton");

        auto tankButtonSlipLevel = blankLevel();
        tankButtonSlipLevel.top[at(1, 1)] = 0x6F;
        tankButtonSlipLevel.top[at(5, 5)] = 0x43;
        tankButtonSlipLevel.bottom[at(5, 5)] = 0x0C;
        tankButtonSlipLevel.top[at(6, 5)] = 0x28;
        tankButtonSlipLevel.top[at(10, 10)] = 0x4F; // east-facing Tank held on an unlinked trap
        tankButtonSlipLevel.bottom[at(10, 10)] = 0x2B;
        auto tankButtonSlip = startWithMapMonsters(tankButtonSlipLevel);
        tankButtonSlip.slipOrder.push_back({SlipperKind::creature, 0});
        tankButtonSlip.advanceTicks(1);
        failures += !check(tankButtonSlip.creatures.size() == 2 &&
                           tankButtonSlip.creatures[0].position == at(6, 5) &&
                           tankButtonSlip.creatures[1].position == at(10, 10) &&
                           tankButtonSlip.creatures[1].tile == 0x4D &&
                           tankButtonSlip.creatures[1].tankDirection == 1,
                           "slipping creature reverses a Tank through TankButton");

        constexpr std::array<std::uint8_t, 9> slippingButtonCreatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60
        };
        constexpr std::array<std::uint8_t, 4> creatureButtons{0x23, 0x24, 0x27, 0x28};
        int checkedCreatureButtonSlipCases = 0;
        for (const auto kind : slippingButtonCreatureKinds) {
            for (int facing = 0; facing < 4; ++facing) {
                for (const auto button : creatureButtons) {
                    auto level = blankLevel();
                    level.top[at(1, 1)] = 0x6F;
                    level.top[at(5, 5)] = static_cast<std::uint8_t>(kind | facing);
                    level.bottom[at(5, 5)] = 0x0C;
                    level.top[at(6, 5)] = button;
                    if (button == 0x23) {
                        level.top[at(8, 8)] = 0x25;
                    } else if (button == 0x24) {
                        level.top[at(9, 9)] = 0x4C;
                        level.bottom[at(9, 9)] = 0x31;
                        OptionalField cloneLink;
                        cloneLink.id = 5;
                        cloneLink.raw = {6, 0, 5, 0, 9, 0, 9, 0};
                        level.fields.push_back(cloneLink);
                    } else if (button == 0x27) {
                        OptionalField trapLink;
                        trapLink.id = 4;
                        trapLink.raw = {6, 0, 5, 0, 9, 0, 9, 0, 1, 0};
                        level.fields.push_back(trapLink);
                    } else {
                        level.top[at(10, 10)] = 0x4F;
                        level.bottom[at(10, 10)] = 0x2B;
                    }
                    auto game = startWithMapMonsters(level);
                    game.slipOrder.push_back({SlipperKind::creature, 0, 3});
                    game.advanceTicks(1); // SlipLoop runs on every tick; heading is east.

                    bool actionSucceeded = false;
                    if (button == 0x23) {
                        actionSucceeded = game.toggleState && game.top[at(8, 8)] == 0x26;
                    } else if (button == 0x24) {
                        actionSucceeded = game.creatures.size() == 2 &&
                            game.creatures[1].position == at(9, 9) && game.creatures[1].tile == 0x4C;
                    } else if (button == 0x27) {
                        actionSucceeded = game.traps.size() == 1 && !game.traps[0].closed;
                    } else {
                        actionSucceeded = game.creatures.size() == 2 &&
                            game.creatures[1].position == at(10, 10) &&
                            game.creatures[1].tile == 0x4D && game.creatures[1].tankDirection == 1;
                    }
                    failures += !check(game.creatures[0].alive &&
                                       game.creatures[0].position == at(6, 5) && actionSucceeded,
                        "slipping " + std::to_string(kind) + " facing " + std::to_string(facing) +
                        " activates button " + std::to_string(button));
                    ++checkedCreatureButtonSlipCases;
                }
            }
        }
        failures += !check(checkedCreatureButtonSlipCases == 9 * 4 * 4,
                           "SlipLoop button matrix covers every species, facing, and button type");

        // MonsterLoop has a separate MoveMonster button-dispatch path from
        // SlipLoop. Exercise every creature family against every button with
        // a heading/position that makes its ordinary movement enter east.
        constexpr std::array<std::uint8_t, 9> ordinaryButtonCreatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60
        };
        int checkedOrdinaryButtonCases = 0;
        for (const auto kind : ordinaryButtonCreatureKinds) {
            for (const auto button : creatureButtons) {
                auto level = blankLevel();
                level.top[at(1, 1)] = 0x6F;
                const auto initialTile = kind == 0x40 ? static_cast<std::uint8_t>(kind | 2)
                    : kind == 0x60 ? static_cast<std::uint8_t>(kind)
                    : static_cast<std::uint8_t>(kind | 3);
                level.top[at(5, 5)] = initialTile;
                level.top[at(6, 5)] = button;
                // Teeth pursue an eastward Chip position; Blobs use seed 1,
                // whose first cardinal sample points east.
                if (button == 0x23) {
                    level.top[at(8, 8)] = 0x25;
                } else if (button == 0x24) {
                    level.top[at(9, 9)] = 0x4C;
                    level.bottom[at(9, 9)] = 0x31;
                    OptionalField cloneLink;
                    cloneLink.id = 5;
                    cloneLink.raw = {6, 0, 5, 0, 9, 0, 9, 0};
                    level.fields.push_back(cloneLink);
                } else if (button == 0x27) {
                    level.top[at(9, 9)] = 0x2B;
                    OptionalField trapLink;
                    trapLink.id = 4;
                    trapLink.raw = {6, 0, 5, 0, 9, 0, 9, 0, 1, 0};
                    level.fields.push_back(trapLink);
                } else {
                    level.top[at(10, 10)] = 0x4F; // East-facing Tank on an unlinked trap.
                    level.bottom[at(10, 10)] = 0x2B;
                }

                auto game = startWithMapMonsters(level);
                game.playerPosition = at(11, 5);
                game.gameTicks = 1;
                if (kind == 0x54 || kind == 0x5C) game.gameTicks = 3;
                game.advanceTicks(1); // MonsterLoop runs at tick four.
                bool actionSucceeded = false;
                if (button == 0x23) {
                    actionSucceeded = game.toggleState && game.top[at(8, 8)] == 0x26;
                } else if (button == 0x24) {
                    actionSucceeded = game.creatures.size() == 2 &&
                        game.creatures[1].position == at(9, 9) && game.creatures[1].tile == 0x4C;
                } else if (button == 0x27) {
                    actionSucceeded = game.traps.size() == 1 && !game.traps[0].closed;
                } else {
                    const auto tank = std::find_if(game.creatures.begin(), game.creatures.end(),
                        [](const Creature& creature) {
                            return (creature.tile & 0xFC) == 0x4C && creature.position == at(10, 10);
                        });
                    actionSucceeded = tank != game.creatures.end() && tank->tile == 0x4D &&
                        tank->tankDirection == 1;
                }
                failures += !check(game.creatures[0].position == at(6, 5) && actionSucceeded,
                    "MonsterLoop " + std::to_string(kind) + " activates button " +
                    std::to_string(button));
                ++checkedOrdinaryButtonCases;
            }
        }
        failures += !check(checkedOrdinaryButtonCases == 9 * 4,
                           "MonsterLoop button matrix covers all creature families and button types");

        auto slipTrapCascadeLevel = blankLevel();
        slipTrapCascadeLevel.top[at(1, 1)] = 0x6F;
        slipTrapCascadeLevel.top[at(5, 5)] = 0x40; // First slipper enters the TrapButton.
        slipTrapCascadeLevel.top[at(6, 5)] = 0x27;
        slipTrapCascadeLevel.top[at(9, 9)] = 0x40; // Second slipper waits on the linked trap.
        slipTrapCascadeLevel.bottom[at(9, 9)] = 0x2B;
        OptionalField slipTrapLink;
        slipTrapLink.id = 4;
        slipTrapLink.raw = {6, 0, 5, 0, 9, 0, 9, 0, 1, 0};
        slipTrapCascadeLevel.fields.push_back(slipTrapLink);
        auto slipTrapCascade = startWithMapMonsters(slipTrapCascadeLevel);
        failures += !check(slipTrapCascade.traps.size() == 1 && slipTrapCascade.traps[0].closed,
                           "linked trap starts closed while its button is present");
        slipTrapCascade.slipOrder.push_back({SlipperKind::creature, 0, 3});
        slipTrapCascade.slipOrder.push_back({SlipperKind::creature, 1, 3});
        slipTrapCascade.advanceTicks(1);
        failures += !check(slipTrapCascade.creatures.size() == 2 &&
                           slipTrapCascade.creatures[0].position == at(6, 5) &&
                           slipTrapCascade.creatures[1].position == at(10, 9) &&
                           !slipTrapCascade.traps[0].closed && slipTrapCascade.slipOrder.empty(),
                           "SlipLoop reprocesses the shifted entry so TrapButton opens a linked trap for the next creature in the same pass");

        auto slipToggleCascadeLevel = blankLevel();
        slipToggleCascadeLevel.top[at(1, 1)] = 0x6F;
        slipToggleCascadeLevel.top[at(5, 5)] = 0x40;
        slipToggleCascadeLevel.top[at(6, 5)] = 0x23;
        slipToggleCascadeLevel.top[at(9, 9)] = 0x40;
        slipToggleCascadeLevel.top[at(10, 9)] = 0x25;
        auto slipToggleCascade = startWithMapMonsters(slipToggleCascadeLevel);
        slipToggleCascade.slipOrder.push_back({SlipperKind::creature, 0, 3});
        slipToggleCascade.slipOrder.push_back({SlipperKind::creature, 1, 3});
        slipToggleCascade.advanceTicks(1);
        failures += !check(slipToggleCascade.creatures[0].position == at(6, 5) &&
                           slipToggleCascade.creatures[1].position == at(10, 9) &&
                           slipToggleCascade.top[at(10, 9)] == 0x26 &&
                           slipToggleCascade.toggleState && slipToggleCascade.slipOrder.empty(),
                           "SlipLoop reprocesses the next creature after ToggleButton changes a blocked ToggleWall to ToggleFloor");

        auto slipCloneCascadeLevel = blankLevel();
        slipCloneCascadeLevel.top[at(1, 1)] = 0x6F;
        slipCloneCascadeLevel.top[at(5, 5)] = 0x40;
        slipCloneCascadeLevel.top[at(6, 5)] = 0x24;
        slipCloneCascadeLevel.top[at(9, 5)] = 0x43; // Clone exits east onto open Floor.
        slipCloneCascadeLevel.bottom[at(9, 5)] = 0x31;
        slipCloneCascadeLevel.top[at(9, 9)] = 0x40;
        OptionalField slipCloneLink;
        slipCloneLink.id = 5;
        slipCloneLink.raw = {6, 0, 5, 0, 9, 0, 5, 0};
        slipCloneCascadeLevel.fields.push_back(slipCloneLink);
        auto slipCloneCascade = startWithMapMonsters(slipCloneCascadeLevel);
        slipCloneCascade.slipOrder.push_back({SlipperKind::creature, 0, 3});
        slipCloneCascade.slipOrder.push_back({SlipperKind::creature, 1, 3});
        slipCloneCascade.advanceTicks(1);
        failures += !check(slipCloneCascade.creatures.size() == 3 &&
                           slipCloneCascade.creatures[0].position == at(6, 5) &&
                           slipCloneCascade.creatures[1].position == at(10, 9) &&
                           slipCloneCascade.creatures[2].position == at(9, 5) &&
                           slipCloneCascade.creatures[2].tile == 0x43 &&
                           slipCloneCascade.slipOrder.empty(),
                           "SlipLoop clone-button append does not invalidate the next queued creature or move the new clone early");

        for (std::uint8_t underChip = 0x64; underChip <= 0x6B; ++underChip) {
            auto itemCollisionLevel = blankLevel();
            itemCollisionLevel.top[at(2, 3)] = 0x6F;
            itemCollisionLevel.bottom[at(2, 3)] = underChip;
            itemCollisionLevel.top[at(2, 4)] = 0x4C; // north-facing Tank approaches Chip
            auto itemCollision = startWithMapMonsters(itemCollisionLevel);
            itemCollision.gameTicks = 1;
            itemCollision.advanceTicks(1);
            const bool isKey = underChip <= 0x67;
            failures += !check(itemCollision.outcome == (isKey ? Outcome::dead : Outcome::playing) &&
                               itemCollision.creatures[0].position == (isKey ? at(2, 3) : at(2, 4)),
                               "creature collision follows TileTable passability beneath Chip for item " +
                               std::to_string(underChip));
        }

        auto sameTickDeathLevel = blankLevel();
        sameTickDeathLevel.top[at(2, 3)] = 0x6F;
        sameTickDeathLevel.top[at(2, 4)] = 0x4C; // first creature catches Chip
        sameTickDeathLevel.top[at(5, 5)] = 0x43; // second creature is already slipping east
        sameTickDeathLevel.bottom[at(5, 5)] = 0x0C;
        sameTickDeathLevel.top[at(8, 8)] = 0x43; // later ordinary creature turns north after turning left
        auto sameTickDeath = startWithMapMonsters(sameTickDeathLevel);
        sameTickDeath.slipOrder.push_back({SlipperKind::creature, 1});
        sameTickDeath.gameTicks = 1;
        sameTickDeath.advanceTicks(1);
        failures += !check(sameTickDeath.outcome == Outcome::dead &&
                           sameTickDeath.playerCoveredByCreature &&
                           sameTickDeath.creatures[1].position == at(6, 5) &&
                           sameTickDeath.creatures[2].position == at(8, 7) &&
                           sameTickDeath.slipOrder.empty(),
                           "later creatures finish MonsterLoop and SlipLoop after Chip's same-tick autopsy");

        auto trapCollisionLevel = blankLevel();
        trapCollisionLevel.top[at(2, 3)] = 0x6F;
        trapCollisionLevel.bottom[at(2, 3)] = 0x2B;
        trapCollisionLevel.top[at(2, 4)] = 0x4C; // tank moves north onto Chip
        auto trapCollision = startWithMapMonsters(trapCollisionLevel);
        trapCollision.gameTicks = 1;
        trapCollision.advanceTicks(1);
        failures += !check(trapCollision.outcome == Outcome::dead,
                           "creature catches Chip while Chip stands on a trap");

        auto slidingTrapCollisionLevel = blankLevel();
        slidingTrapCollisionLevel.top[at(2, 3)] = 0x6F;
        slidingTrapCollisionLevel.bottom[at(2, 3)] = 0x2B;
        slidingTrapCollisionLevel.top[at(3, 3)] = 0x41; // bug is already slipping west
        slidingTrapCollisionLevel.bottom[at(3, 3)] = 0x0C;
        auto slidingTrapCollision = startWithMapMonsters(slidingTrapCollisionLevel);
        slidingTrapCollision.slipOrder.push_back({SlipperKind::creature, 0});
        slidingTrapCollision.advanceTicks(1);
        failures += !check(slidingTrapCollision.outcome == Outcome::dead,
                           "slipping creature catches Chip on a trap");
        failures += !check(slidingTrapCollision.playerCoveredByCreature,
                           "a slipping creature remains visually above Chip after contact");

        auto teleporterCollisionLevel = blankLevel();
        teleporterCollisionLevel.top[at(1, 1)] = 0x6F;
        teleporterCollisionLevel.top[at(3, 3)] = 0x29;
        teleporterCollisionLevel.top[at(8, 5)] = 0x29;
        teleporterCollisionLevel.top[at(4, 3)] = 0x4D; // tank enters west onto Chip's teleporter
        auto teleporterCollision = startWithMapMonsters(teleporterCollisionLevel);
        teleporterCollision.playerPosition = at(3, 3);
        teleporterCollision.gameTicks = 1;
        teleporterCollision.advanceTicks(1);
        failures += !check(teleporterCollision.outcome == Outcome::playing &&
                           teleporterCollision.creatures[0].position == at(7, 5),
                           "creature contacting Chip on a teleporter exits before the Chip collision check");
        failures += !check(teleporterCollision.creatures[0].teleportedThisTick,
                           "MonsterLoop exposes teleporter relocation for presentation");

        auto slidingTeleporterCollisionLevel = blankLevel();
        slidingTeleporterCollisionLevel.top[at(1, 1)] = 0x6F;
        slidingTeleporterCollisionLevel.top[at(3, 3)] = 0x29;
        slidingTeleporterCollisionLevel.top[at(8, 5)] = 0x29;
        slidingTeleporterCollisionLevel.top[at(4, 3)] = 0x41; // bug is slipping west
        slidingTeleporterCollisionLevel.bottom[at(4, 3)] = 0x0C;
        auto slidingTeleporterCollision = startWithMapMonsters(slidingTeleporterCollisionLevel);
        slidingTeleporterCollision.playerPosition = at(3, 3);
        slidingTeleporterCollision.slipOrder.push_back({SlipperKind::creature, 0});
        slidingTeleporterCollision.advanceTicks(1);
        failures += !check(slidingTeleporterCollision.outcome == Outcome::playing &&
                           slidingTeleporterCollision.creatures[0].position == at(8, 5) &&
                           slidingTeleporterCollision.slipOrder.size() == 1,
                           "slipping creature teleports away from Chip before collision resolution");
        failures += !check(slidingTeleporterCollision.creatures[0].teleportedThisTick,
                           "SlipLoop exposes teleporter relocation for presentation");

        auto singlePortalSlideCollisionLevel = blankLevel();
        singlePortalSlideCollisionLevel.top[at(1, 1)] = 0x6F;
        singlePortalSlideCollisionLevel.top[at(3, 3)] = 0x29;
        singlePortalSlideCollisionLevel.top[at(4, 3)] = 0x41; // west-facing Bug already sliding
        singlePortalSlideCollisionLevel.bottom[at(4, 3)] = 0x0C;
        auto singlePortalSlideCollision = startWithMapMonsters(singlePortalSlideCollisionLevel);
        singlePortalSlideCollision.playerPosition = at(3, 3);
        singlePortalSlideCollision.slipOrder.push_back({SlipperKind::creature, 0});
        singlePortalSlideCollision.advanceTicks(1);
        failures += !check(singlePortalSlideCollision.outcome == Outcome::dead &&
                           singlePortalSlideCollision.creatures[0].position == at(3, 3) &&
                           singlePortalSlideCollision.slipOrder.size() == 1,
                           "slipping creature enters Chip's unpaired teleporter before post-move collision");

        auto blockedTeleporterCollisionLevel = blankLevel();
        blockedTeleporterCollisionLevel.top[at(1, 1)] = 0x6F;
        blockedTeleporterCollisionLevel.top[at(3, 3)] = 0x29;
        blockedTeleporterCollisionLevel.top[at(4, 3)] = 0x4D;
        auto blockedTeleporterCollision = startWithMapMonsters(blockedTeleporterCollisionLevel);
        blockedTeleporterCollision.playerPosition = at(3, 3);
        blockedTeleporterCollision.gameTicks = 1;
        blockedTeleporterCollision.advanceTicks(1);
        failures += !check(blockedTeleporterCollision.outcome == Outcome::dead,
                           "creature catches Chip on a teleporter with no available exit");

        constexpr std::array<std::uint8_t, 9> hazardCreatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60
        };
        constexpr std::array<const char*, 9> hazardCreatureNames{
            "Bug", "Fireball", "Ball", "Tank", "Glider", "Teeth", "Walker", "Blob", "Paramecium"
        };
        for (const auto hazard : {std::uint8_t{0x03}, std::uint8_t{0x04}}) {
            for (std::size_t i = 0; i < hazardCreatureKinds.size(); ++i) {
                auto level = blankLevel();
                const int player = at(6, 5);
                const int creature = at(5, 5);
                level.top[player] = 0x6F;
                level.bottom[player] = hazard;
                level.top[creature] = static_cast<std::uint8_t>(hazardCreatureKinds[i] | 3U);
                level.top[at(5, 4)] = 0x01;
                level.top[at(5, 6)] = 0x01;
                level.top[at(4, 5)] = 0x01;
                auto game = startWithMapMonsters(level);
                game.gameTicks = 3; // even MonsterLoop pass and Teeth/Blob gate
                game.advanceTicks(1);

                const bool fire = hazard == 0x04;
                const bool survivesHazard = fire ? hazardCreatureKinds[i] == 0x44
                                                  : hazardCreatureKinds[i] == 0x50;
                const bool blockedByFire = fire && (hazardCreatureKinds[i] == 0x40 ||
                                                     hazardCreatureKinds[i] == 0x58);
                const auto expectedOutcome = survivesHazard ? Outcome::dead : Outcome::playing;
                const bool expectedAlive = survivesHazard || blockedByFire;
                const int expectedPosition = blockedByFire ? creature : player;
                failures += !check(game.outcome == expectedOutcome &&
                                   game.creatures[0].alive == expectedAlive &&
                                   game.creatures[0].position == expectedPosition,
                    std::string(hazardCreatureNames[i]) +
                    (fire ? " ordinary turn resolves Fire before Chip collision"
                          : " ordinary turn resolves Water before Chip collision"));
            }
        }
        for (const auto hazard : {std::uint8_t{0x03}, std::uint8_t{0x04}}) {
            for (std::size_t i = 0; i < hazardCreatureKinds.size(); ++i) {
                auto level = blankLevel();
                const int player = at(3, 3);
                const int creature = at(2, 3);
                level.top[player] = 0x6F;
                level.bottom[player] = hazard;
                level.top[creature] = static_cast<std::uint8_t>(hazardCreatureKinds[i] | 3U);
                level.bottom[creature] = 0x0C;
                auto game = startWithMapMonsters(level);
                game.slipOrder.push_back({SlipperKind::creature, 0});
                game.advanceTicks(1);

                const bool fire = hazard == 0x04;
                const bool hazardSurvivor = fire ? hazardCreatureKinds[i] == 0x44
                                                 : hazardCreatureKinds[i] == 0x50;
                const bool hazardBlocked = fire && (hazardCreatureKinds[i] == 0x40 ||
                                                    hazardCreatureKinds[i] == 0x58);
                const auto expectedOutcome = hazardSurvivor ? Outcome::dead : Outcome::playing;
                const bool expectedAlive = hazardSurvivor || hazardBlocked;
                const std::string label = std::string(hazardCreatureNames[i]) +
                    (fire ? " Fire collision resolves hazard before Chip autopsy"
                          : " Water collision resolves hazard before Chip autopsy");
                failures += !check(game.outcome == expectedOutcome &&
                                   game.creatures[0].alive == expectedAlive, label);
            }
        }

        auto waterContactLevel = blankLevel();
        waterContactLevel.top[at(2, 3)] = 0x6F;
        waterContactLevel.bottom[at(2, 3)] = 0x03;
        waterContactLevel.top[at(2, 4)] = 0x4C; // tank enters water under Chip
        auto waterContact = startWithMapMonsters(waterContactLevel);
        waterContact.boots[0] = true;
        waterContact.gameTicks = 1;
        waterContact.advanceTicks(1);
        failures += !check(waterContact.outcome == Outcome::playing && !waterContact.creatures[0].alive &&
                           waterContact.creatures[0].position == at(2, 3),
                           "non-glider drowns on Chip's water tile and retains the destination position");

        auto hazardDeparture = blankLevel();
        hazardDeparture.top[at(1, 1)] = 0x6F;
        hazardDeparture.top[at(3, 3)] = 0x4F; // east-facing tank starts on Hint.
        hazardDeparture.bottom[at(3, 3)] = 0x2F;
        hazardDeparture.top[at(4, 3)] = 0x03;
        auto drownedOnHintDeparture = startWithMapMonsters(hazardDeparture);
        drownedOnHintDeparture.gameTicks = 1;
        drownedOnHintDeparture.advanceTicks(1);
        failures += !check(!drownedOnHintDeparture.creatures[0].alive &&
                           drownedOnHintDeparture.creatures[0].position == at(4, 3) &&
                           drownedOnHintDeparture.top[at(3, 3)] == 0x2F &&
                           drownedOnHintDeparture.bottom[at(3, 3)] == 0x00,
                           "hazard death clears the departed lower layer and preserves the visible Hint");

        auto cloneDeparture = blankLevel();
        cloneDeparture.top[at(1, 1)] = 0x6F;
        cloneDeparture.top[at(3, 3)] = 0x4F;
        cloneDeparture.top[at(4, 3)] = 0x03;
        auto drownedOnCloneDeparture = startWithMapMonsters(cloneDeparture);
        drownedOnCloneDeparture.bottom[at(3, 3)] = 0x31;
        drownedOnCloneDeparture.gameTicks = 1;
        drownedOnCloneDeparture.advanceTicks(1);
        failures += !check(!drownedOnCloneDeparture.creatures[0].alive &&
                           drownedOnCloneDeparture.creatures[0].position == at(4, 3) &&
                           drownedOnCloneDeparture.bottom[at(3, 3)] == 0x31,
                           "hazard death preserves a clone machine beneath the departed creature");

        auto fireContactLevel = blankLevel();
        fireContactLevel.top[at(2, 3)] = 0x6F;
        fireContactLevel.bottom[at(2, 3)] = 0x04;
        fireContactLevel.top[at(3, 3)] = 0x4D; // tank is slipping west into Chip on fire
        fireContactLevel.bottom[at(3, 3)] = 0x0C;
        auto fireContact = startWithMapMonsters(fireContactLevel);
        fireContact.boots[1] = true;
        fireContact.slipOrder.push_back({SlipperKind::creature, 0});
        fireContact.advanceTicks(1);
        failures += !check(fireContact.outcome == Outcome::playing && !fireContact.creatures[0].alive &&
                           fireContact.creatures[0].position == at(2, 3) &&
                           fireContact.bottom[at(3, 3)] == 0x00,
                           "slipping fire death clears the departed lower layer and retains the destination");

        auto panelContactLevel = blankLevel();
        panelContactLevel.top[at(2, 3)] = 0x6F;
        panelContactLevel.bottom[at(2, 3)] = 0x06; // PanelN permits entry from the north
        panelContactLevel.top[at(2, 4)] = 0x4C;
        auto panelContact = startWithMapMonsters(panelContactLevel);
        panelContact.gameTicks = 1;
        panelContact.advanceTicks(1);
        failures += !check(panelContact.outcome == Outcome::dead,
                           "creature catches Chip when it can enter Chip's panel tile");

        auto blockedPanelContactLevel = blankLevel();
        blockedPanelContactLevel.top[at(2, 3)] = 0x6F;
        blockedPanelContactLevel.bottom[at(2, 3)] = 0x06;
        blockedPanelContactLevel.top[at(2, 2)] = 0x4E; // tank approaches from the north; blocked
        auto blockedPanelContact = startWithMapMonsters(blockedPanelContactLevel);
        blockedPanelContact.advanceTicks(1);
        failures += !check(blockedPanelContact.outcome == Outcome::playing &&
                           blockedPanelContact.creatures[0].position == at(2, 2),
                           "one-way panel blocks creature contact from its forbidden side");

        auto slidingPanelContactLevel = blankLevel();
        slidingPanelContactLevel.top[at(3, 3)] = 0x6F;
        slidingPanelContactLevel.bottom[at(3, 3)] = 0x30; // PanelSE permits eastward entry
        slidingPanelContactLevel.top[at(2, 3)] = 0x43; // bug is slipping east
        slidingPanelContactLevel.bottom[at(2, 3)] = 0x0C;
        auto slidingPanelContact = startWithMapMonsters(slidingPanelContactLevel);
        slidingPanelContact.slipOrder.push_back({SlipperKind::creature, 0});
        slidingPanelContact.advanceTicks(1);
        failures += !check(slidingPanelContact.outcome == Outcome::dead,
                           "slipping creature catches Chip on an enterable panel tile");

        auto panelExitSlipLevel = blankLevel();
        panelExitSlipLevel.top[at(1, 1)] = 0x6F;
        panelExitSlipLevel.top[at(5, 5)] = 0x40; // north-facing bug on a PanelN tile
        panelExitSlipLevel.bottom[at(5, 5)] = 0x06;
        auto panelExitSlip = startWithMapMonsters(panelExitSlipLevel);
        panelExitSlip.slipOrder.push_back({SlipperKind::creature, 0});
        panelExitSlip.advanceTicks(1);
        failures += !check(panelExitSlip.creatures[0].position == at(5, 5) &&
                           panelExitSlip.creatures[0].tile == 0x40 &&
                           panelExitSlip.slipOrder.size() == 1,
                           "slipping creature retains its heading when a one-way panel blocks its exit");
        panelExitSlip.advanceTicks(1);
        failures += !check(panelExitSlip.creatures[0].position == at(5, 5) &&
                           panelExitSlip.creatures[0].tile == 0x40 &&
                           panelExitSlip.slipOrder.size() == 1,
                           "a north-facing panel slipper remains blocked without changing its heading");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(4, 3)] = 0x40; // bug turns west onto two consecutive ice tiles
        level.top[at(3, 3)] = 0x0C;
        level.top[at(2, 3)] = 0x0C;
        level.top[at(1, 3)] = 0x0C;
        auto game = startWithMapMonsters(level);
        game.advanceTicks(4);
        failures += !check(game.creatures[0].position == at(2, 3) &&
                           game.bottom[at(3, 3)] == 0x00,
                           "creature reverses immediately when blocked at the end of an ice run");

        auto doubleBlockedLevel = blankLevel();
        doubleBlockedLevel.top[at(1, 1)] = 0x6F;
        doubleBlockedLevel.top[at(5, 5)] = 0x43; // east-facing bug on ice
        doubleBlockedLevel.bottom[at(5, 5)] = 0x0C;
        doubleBlockedLevel.top[at(4, 5)] = 0x01;
        doubleBlockedLevel.top[at(6, 5)] = 0x01;
        auto doubleBlocked = startWithMapMonsters(doubleBlockedLevel);
        doubleBlocked.slipOrder.push_back({SlipperKind::creature, 0});
        doubleBlocked.advanceTicks(1);
        failures += !check(doubleBlocked.creatures[0].position == at(5, 5) &&
                           doubleBlocked.creatures[0].tile == 0x41 &&
                           doubleBlocked.slipOrder.size() == 1,
                           "a doubly blocked creature keeps its second-attempt facing until the next slide");
        doubleBlocked.top[at(6, 5)] = 0x00;
        doubleBlocked.advanceTicks(1);
        failures += !check(doubleBlocked.creatures[0].position == at(6, 5),
                           "a double-blocked ice creature resumes along the restored heading");

        constexpr std::array<std::uint8_t, 4> corners{0x1A, 0x1B, 0x1C, 0x1D};
        constexpr std::array<std::array<std::uint8_t, 4>, 4> secondAttemptFacing{{
            {{0, 1, 3, 2}}, {{0, 2, 1, 3}}, {{1, 0, 2, 3}}, {{3, 1, 2, 0}}
        }};
        constexpr std::array<int, 4> monsterDx{0, -1, 0, 1};
        constexpr std::array<int, 4> monsterDy{-1, 0, 1, 0};
        for (const auto cornerTile : corners) {
            for (int directionCode = 0; directionCode < 4; ++directionCode) {
                auto cornerRetryLevel = blankLevel();
                cornerRetryLevel.top[at(1, 1)] = 0x6F;
                cornerRetryLevel.top[at(5, 5)] = static_cast<std::uint8_t>(0x40 | directionCode);
                cornerRetryLevel.bottom[at(5, 5)] = cornerTile;
                for (int blockedDirection = 0; blockedDirection < 4; ++blockedDirection) {
                    const int x = 5 + monsterDx[static_cast<std::size_t>(blockedDirection)];
                    const int y = 5 + monsterDy[static_cast<std::size_t>(blockedDirection)];
                    cornerRetryLevel.top[at(x, y)] = 0x01;
                }
                auto cornerRetry = startWithMapMonsters(cornerRetryLevel);
                cornerRetry.slipOrder.push_back({SlipperKind::creature, 0});
                cornerRetry.advanceTicks(1);
                const auto expectedFacing = secondAttemptFacing[cornerTile - 0x1A]
                    [static_cast<std::size_t>(directionCode)];
                failures += !check(cornerRetry.creatures[0].position == at(5, 5) &&
                                   cornerRetry.creatures[0].tile == (0x40 | expectedFacing) &&
                                   cornerRetry.slipOrder.size() == 1,
                                   "a doubly blocked corner preserves the facing from its second move attempt");
            }
        }

        auto teleportLevel = blankLevel();
        teleportLevel.top[at(1, 1)] = 0x6F;
        teleportLevel.top[at(4, 3)] = 0x41; // already headed west on ForceW
        teleportLevel.bottom[at(4, 3)] = 0x14;
        teleportLevel.top[at(3, 3)] = 0x29;
        teleportLevel.top[at(8, 5)] = 0x29;
        auto teleportSlider = startWithMapMonsters(teleportLevel);
        teleportSlider.slipOrder.push_back({SlipperKind::creature, 0});
        teleportSlider.advanceTicks(1);
        failures += !check(teleportSlider.creatures[0].position == at(8, 5) &&
                           teleportSlider.bottom[at(4, 3)] == 0x00 &&
                           teleportSlider.bottom[at(8, 5)] == 0x29 &&
                           teleportSlider.slipOrder.size() == 1,
                           "sliding creature enters a linked teleporter and retains its slip state");
        teleportSlider.advanceTicks(1);
        failures += !check(teleportSlider.creatures[0].position == at(7, 5) &&
                           teleportSlider.top[at(8, 5)] == 0x29 &&
                           teleportSlider.slipOrder.empty(),
                           "sliding creature advances away from the teleporter on its next slip phase");

        auto blockedPortalSlideLevel = blankLevel();
        blockedPortalSlideLevel.top[at(1, 1)] = 0x6F;
        blockedPortalSlideLevel.top[at(5, 5)] = 0x43; // east-facing bug already slipping on a portal
        blockedPortalSlideLevel.bottom[at(5, 5)] = 0x29;
        blockedPortalSlideLevel.top[at(6, 5)] = 0x01;
        auto blockedPortalSlide = startWithMapMonsters(blockedPortalSlideLevel);
        blockedPortalSlide.slipOrder.push_back({SlipperKind::creature, 0});
        blockedPortalSlide.advanceTicks(1);
        failures += !check(blockedPortalSlide.creatures[0].position == at(5, 5) &&
                           blockedPortalSlide.creatures[0].tile == 0x43 &&
                           blockedPortalSlide.slipOrder.size() == 1,
                           "a slipping creature retains its heading after a blocked step on a teleporter");
        blockedPortalSlide.top[at(6, 5)] = 0x00;
        blockedPortalSlide.advanceTicks(1);
        failures += !check(blockedPortalSlide.creatures[0].position == at(6, 5) &&
                           blockedPortalSlide.slipOrder.empty(),
                           "a teleporter slipper resumes along its stored heading when unblocked");

        auto coveredPortalLevel = blankLevel();
        coveredPortalLevel.top[at(1, 1)] = 0x6F;
        coveredPortalLevel.top[at(4, 3)] = 0x41; // bug slips west into the first portal
        coveredPortalLevel.bottom[at(4, 3)] = 0x14;
        coveredPortalLevel.top[at(3, 3)] = 0x29;
        coveredPortalLevel.top[at(8, 5)] = 0x29;
        coveredPortalLevel.top[at(12, 8)] = 0x29;
        auto coveredPortal = startWithMapMonsters(coveredPortalLevel);
        coveredPortal.top[at(12, 8)] = 0x0A;
        coveredPortal.bottom[at(12, 8)] = 0x29;
        coveredPortal.slipOrder.push_back({SlipperKind::creature, 0});
        coveredPortal.advanceTicks(1);
        failures += !check(coveredPortal.creatures[0].position == at(8, 5),
                           "slipping creature skips a candidate portal covered by a block");

        auto reversalLevel = blankLevel();
        reversalLevel.top[at(1, 1)] = 0x6F;
        reversalLevel.top[at(5, 5)] = 0x43; // bug faces east toward the blocked edge
        reversalLevel.bottom[at(5, 5)] = 0x0C;
        reversalLevel.top[at(6, 5)] = 0x01;
        auto reversal = startWithMapMonsters(reversalLevel);
        reversal.slipOrder.push_back({SlipperKind::creature, 0});
        reversal.advanceTicks(1);
        failures += !check(reversal.creatures[0].position == at(4, 5) &&
                           reversal.slipOrder.empty(),
                           "creature blocked on ice retries backward in the same slip pass");

        auto blockedForceLevel = blankLevel();
        blockedForceLevel.top[at(1, 1)] = 0x6F;
        blockedForceLevel.top[at(5, 5)] = 0x43; // east-facing bug
        blockedForceLevel.bottom[at(5, 5)] = 0x13;
        blockedForceLevel.top[at(6, 5)] = 0x01;
        auto blockedForce = startWithMapMonsters(blockedForceLevel);
        blockedForce.slipOrder.push_back({SlipperKind::creature, 0});
        blockedForce.advanceTicks(1);
        failures += !check(blockedForce.creatures[0].position == at(5, 5) &&
                           blockedForce.creatures[0].tile == 0x43 &&
                           blockedForce.slipOrder.size() == 1,
                           "blocked creature on a directional force floor retains its slip state");
        blockedForce.top[at(6, 5)] = 0x00;
        blockedForce.advanceTicks(1);
        failures += !check(blockedForce.creatures[0].position == at(6, 5) &&
                           blockedForce.slipOrder.empty(),
                           "force-floor creature resumes after its path opens");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x0A;
        level.bottom[at(5, 5)] = 0x0C;
        level.top[at(4, 5)] = 0x01;
        level.top[at(6, 5)] = 0x01;
        level.top[at(10, 10)] = 0x0A;
        level.bottom[at(10, 10)] = 0x0C;
        level.top[at(11, 10)] = 0x0C;
        auto game = startWithMapMonsters(level);
        game.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        game.blockDirections[at(10, 10)] = static_cast<std::int8_t>(Direction::east);
        game.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        game.slipOrder.push_back({SlipperKind::block, at(10, 10)});
        game.advanceTicks(1);
        failures += !check(game.slipOrder.size() == 2 &&
                           game.slipOrder[0].entity == at(10, 10) &&
                           game.slipOrder[1].entity == at(5, 5) &&
                           game.top[at(10, 10)] == 0x0A && game.top[at(11, 10)] == 0x0C,
                           "blocked sliding block is removed and re-added at the tail before SlipLoop advances its cursor");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x0A;
        level.bottom[at(5, 5)] = 0x0C;
        level.top[at(6, 5)] = 0x2A; // The first queued block is consumed.
        level.top[at(10, 10)] = 0x0A;
        level.bottom[at(10, 10)] = 0x0C;
        level.top[at(11, 10)] = 0x00;
        auto game = startWithMapMonsters(level);
        game.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        game.blockDirections[at(10, 10)] = static_cast<std::int8_t>(Direction::east);
        game.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        game.slipOrder.push_back({SlipperKind::block, at(10, 10)});
        game.advanceTicks(1);
        failures += !check(game.top[at(6, 5)] == 0x00 &&
                           game.top[at(5, 5)] == 0x0C && game.bottom[at(5, 5)] == 0x00 &&
                           game.top[at(11, 10)] == 0x0A && game.bottom[at(11, 10)] == 0x00 &&
                           game.slipOrder.empty(),
                           "SlipLoop processes the next queued block in the same tick after the first is consumed by a bomb");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x0A;
        level.bottom[at(5, 5)] = 0x0C;
        level.top[at(6, 5)] = 0x2A;
        level.top[at(10, 10)] = 0x43; // East-facing Bug on Ice.
        level.bottom[at(10, 10)] = 0x0C;
        level.top[at(11, 10)] = 0x00;
        auto mixedGame = startWithMapMonsters(level);
        mixedGame.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        mixedGame.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        mixedGame.slipOrder.push_back({SlipperKind::creature, 0,
                                       3}); // Original creature direction code: N, W, S, E.
        mixedGame.advanceTicks(1); // Odd tick: isolate shared SlipLoop processing.
        failures += !check(mixedGame.top[at(6, 5)] == 0x00 && mixedGame.creatures[0].alive &&
                           mixedGame.creatures[0].position == at(11, 10) &&
                           mixedGame.creatures[0].tile == 0x43 && mixedGame.slipOrder.empty(),
                           "a bomb-consumed block does not skip the creature shifted into its SlipLoop queue slot");

        constexpr std::array<std::pair<std::uint8_t, std::uint8_t>, 3> creatureDeathCases{{
            {0x03, 0x43}, // Bug drowns in Water.
            {0x04, 0x4F}, // Tank burns in Fire.
            {0x2A, 0x43}, // Bug is consumed by Bomb.
        }};
        for (const auto [hazard, creatureSprite] : creatureDeathCases) {
            auto creatureFirstLevel = blankLevel();
            creatureFirstLevel.top[at(1, 1)] = 0x6F;
            creatureFirstLevel.top[at(5, 5)] = creatureSprite;
            creatureFirstLevel.bottom[at(5, 5)] = 0x0C;
            creatureFirstLevel.top[at(6, 5)] = hazard;
            creatureFirstLevel.top[at(10, 10)] = 0x0A;
            creatureFirstLevel.bottom[at(10, 10)] = 0x0C;
            creatureFirstLevel.top[at(11, 10)] = 0x00;
            auto creatureFirstGame = startWithMapMonsters(creatureFirstLevel);
            creatureFirstGame.blockDirections[at(10, 10)] = static_cast<std::int8_t>(Direction::east);
            creatureFirstGame.slipOrder.push_back({SlipperKind::creature, 0, 3});
            creatureFirstGame.slipOrder.push_back({SlipperKind::block, at(10, 10)});
            creatureFirstGame.advanceTicks(1); // Odd tick: isolate shared SlipLoop processing.
            failures += !check(!creatureFirstGame.creatures[0].alive &&
                               creatureFirstGame.creatures[0].position == at(6, 5) &&
                               creatureFirstGame.top[at(11, 10)] == 0x0A &&
                               creatureFirstGame.slipOrder.empty(),
                               "a hazard-removed creature does not skip the block shifted into its SlipLoop queue slot for tile 0x" +
                                   std::to_string(hazard));
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x43; // east-facing bug on a teleporter
        level.bottom[at(5, 5)] = 0x29;
        level.top[at(6, 5)] = 0x01;
        level.top[at(10, 10)] = 0x43; // east-facing bug on ice
        level.bottom[at(10, 10)] = 0x0C;
        level.top[at(11, 10)] = 0x0C;
        auto game = startWithMapMonsters(level);
        game.slipOrder.push_back({SlipperKind::creature, 0});
        game.slipOrder.push_back({SlipperKind::creature, 1});
        game.advanceTicks(1);
        failures += !check(game.slipOrder.size() == 2 &&
                           game.slipOrder[0].entity == 1 && game.slipOrder[1].entity == 0 &&
                           game.creatures[0].position == at(5, 5) &&
                           game.creatures[1].position == at(10, 10),
                           "blocked creature slipper is removed and re-added at the tail before SlipLoop advances its cursor");
    }
    {
        auto blockFirstLevel = blankLevel();
        blockFirstLevel.top[at(1, 1)] = 0x6F;
        blockFirstLevel.top[at(5, 5)] = 0x0A;
        blockFirstLevel.bottom[at(5, 5)] = 0x0C;
        blockFirstLevel.top[at(7, 5)] = 0x41; // West-facing Bug.
        blockFirstLevel.bottom[at(7, 5)] = 0x0C;
        auto blockFirst = startWithMapMonsters(blockFirstLevel);
        blockFirst.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        blockFirst.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        blockFirst.slipOrder.push_back({SlipperKind::creature, 0});
        blockFirst.advanceTicks(1); // Odd tick: process only the shared SlipLoop.
        failures += !check(blockFirst.top[at(6, 5)] == 0x0A &&
                           blockFirst.creatures[0].position == at(8, 5) &&
                           blockFirst.slipOrder.empty(),
                           "block-first SlipLoop ordering lets the block claim the shared square before the Bug reverses");

        auto creatureFirstLevel = blankLevel();
        creatureFirstLevel.top[at(1, 1)] = 0x6F;
        creatureFirstLevel.top[at(5, 5)] = 0x0A;
        creatureFirstLevel.bottom[at(5, 5)] = 0x0C;
        creatureFirstLevel.top[at(7, 5)] = 0x41;
        creatureFirstLevel.bottom[at(7, 5)] = 0x0C;
        auto creatureFirst = startWithMapMonsters(creatureFirstLevel);
        creatureFirst.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        creatureFirst.slipOrder.push_back({SlipperKind::creature, 0});
        creatureFirst.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        creatureFirst.advanceTicks(1);
        failures += !check(creatureFirst.creatures[0].position == at(6, 5) &&
                           creatureFirst.top[at(4, 5)] == 0x0A &&
                           creatureFirst.slipOrder.empty(),
                           "creature-first SlipLoop ordering reserves the shared square and reverses the block on Ice");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(4, 5)] = 0x58; // north-facing walker has a clear straight path
        auto game = startWithMapMonsters(level);
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.creatures[0].position == at(4, 4) && game.randomState == 1U,
                           "walker takes its straight path before consuming random turns");

        auto randomTurnLevel = blankLevel();
        randomTurnLevel.top[at(1, 1)] = 0x6F;
        randomTurnLevel.top[at(5, 5)] = 0x58; // North-facing Walker.
        randomTurnLevel.top[at(5, 4)] = 0x01; // Force a random fallback.
        auto randomTurn = startWithMapMonsters(randomTurnLevel);
        randomTurn.gameTicks = 1;
        randomTurn.advanceTicks(1);
        failures += !check(randomTurn.creatures[0].position == at(5, 6) &&
                           randomTurn.creatures[0].tile == 0x5A &&
                           randomTurn.randomState == 2745024U,
                           "walker uses source CRT rand modulo three for its first fallback turn");
    }
    {
        // RandInt returns CRT rand() modulo three. Exercise each of its three
        // Walker turn values at all facings, then ensure a repeated random
        // value is consumed but never causes a direction to be tried twice.
        constexpr int source = 10 * kBoardWidth + 10;
        constexpr std::array<std::uint32_t, 3> seeds{2U, 6U, 1U};
        constexpr std::array<std::uint32_t, 3> postRandomStates{2959037U, 3815089U, 2745024U};
        for (int facing = 0; facing < 4; ++facing) {
            for (int turn = 0; turn < 3; ++turn) {
                auto level = blankLevel();
                level.top[at(1, 1)] = 0x6F;
                level.top[static_cast<std::size_t>(source)] = static_cast<std::uint8_t>(0x58 | facing);
                for (int direction = 0; direction < 4; ++direction)
                    level.top[static_cast<std::size_t>(neighborForMonsterDirection(source, direction))] = 0x01;
                const int offset = turn == 0 ? 1 : turn == 1 ? 3 : 2;
                const int expectedDirection = (facing + offset) & 3;
                const int destination = neighborForMonsterDirection(source, expectedDirection);
                level.top[static_cast<std::size_t>(destination)] = 0x00;
                auto game = startWithMapMonsters(level);
                game.randomState = seeds[static_cast<std::size_t>(turn)];
                game.gameTicks = 1;
                game.advanceTicks(1);
                failures += !check(game.creatures[0].position == destination &&
                                   (game.creatures[0].tile & 0x03) == expectedDirection &&
                                   game.randomState == postRandomStates[static_cast<std::size_t>(turn)],
                                   "Walker CRT turn selection covers every facing and turn value");
            }
        }

        auto repeatedChoiceLevel = blankLevel();
        repeatedChoiceLevel.top[at(1, 1)] = 0x6F;
        repeatedChoiceLevel.top[at(5, 5)] = 0x58;
        repeatedChoiceLevel.top[at(5, 4)] = 0x01; // straight blocked
        repeatedChoiceLevel.top[at(5, 6)] = 0x01; // first random choice is blocked reverse
        repeatedChoiceLevel.top[at(4, 5)] = 0x01;
        auto repeatedChoice = startWithMapMonsters(repeatedChoiceLevel);
        repeatedChoice.randomState = 1U; // CRT choices: reverse, reverse again, right
        repeatedChoice.gameTicks = 1;
        repeatedChoice.advanceTicks(1);
        failures += !check(repeatedChoice.creatures[0].position == at(6, 5) &&
                           repeatedChoice.randomState == 415139642U,
                           "Walker skips duplicate random direction and retries the next distinct turn");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x40; // north-facing bug
        level.top[at(4, 5)] = 0x01;
        level.top[at(5, 4)] = 0x01;
        level.top[at(6, 5)] = 0x01;
        level.top[at(5, 6)] = 0x01;
        auto blockedBug = startWithMapMonsters(level);
        blockedBug.gameTicks = 1;
        blockedBug.advanceTicks(1);
        failures += !check(blockedBug.creatures[0].position == at(5, 5) &&
                           blockedBug.creatures[0].tile == 0x42,
                           "bug keeps the facing from its final blocked movement attempt");

        auto blockedFacing = blankLevel();
        blockedFacing.top[at(1, 1)] = 0x6F;
        blockedFacing.top[at(5, 5)] = 0x40; // north movement vector and facing
        blockedFacing.top[at(5, 4)] = 0x01;
        blockedFacing.top[at(4, 5)] = 0x01;
        blockedFacing.top[at(6, 5)] = 0x01;
        blockedFacing.top[at(5, 6)] = 0x01;
        auto blockedFacingBug = startWithMapMonsters(blockedFacing);
        blockedFacingBug.gameTicks = 1;
        blockedFacingBug.advanceTicks(1);
        failures += !check(blockedFacingBug.creatures[0].tile == 0x42 &&
                           blockedFacingBug.creatures[0].movementDirection == 0,
                           "blocked bug changes facing while retaining its stored north movement vector");
        blockedFacingBug.top[at(4, 5)] = 0x00;
        blockedFacingBug.top[at(6, 5)] = 0x00;
        blockedFacingBug.advanceTicks(2);
        failures += !check(blockedFacingBug.creatures[0].position == at(4, 5) &&
                           blockedFacingBug.creatures[0].movementDirection == 1,
                           "next Bug turn begins from stored north vector and chooses left before right");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x5C;
        level.bottom[at(5, 5)] = 0x2B; // an open trap; the blob's north-facing path is blocked
        level.top[at(5, 4)] = 0x01;
        OptionalField openTrap;
        openTrap.id = 4;
        openTrap.raw = {7, 0, 7, 0, 5, 0, 5, 0, 0, 0};
        level.fields.push_back(openTrap);
        auto game = startWithMapMonsters(level);
        game.advanceTicks(4);
        failures += !check(game.creatures[0].position == at(6, 5) &&
                           game.creatures[0].tile == 0x5F && game.randomState == 1030492215U,
                           "blob samples CRT random x/y vectors and moves east off an open trap");

        auto blockedBlobLevel = blankLevel();
        blockedBlobLevel.top[at(1, 1)] = 0x6F;
        blockedBlobLevel.top[at(5, 5)] = 0x5C;
        blockedBlobLevel.top[at(6, 5)] = 0x01; // initial random east attempt blocked
        blockedBlobLevel.top[at(5, 4)] = 0x01; // first random left turn blocked
        auto blockedBlob = startWithMapMonsters(blockedBlobLevel);
        blockedBlob.advanceTicks(4);
        failures += !check(blockedBlob.creatures[0].position == at(5, 6) &&
                           blockedBlob.creatures[0].tile == 0x5E &&
                           blockedBlob.randomState == 1766988168U,
                           "blob randomly retries distinct turns after its initial vector is blocked");
    }
    {
        // The Blob draws x/y independently from RandInt(3), rejects zero or
        // diagonal vectors, then chooses a random left/right/reverse fallback.
        // These fixed seeds cover all four initial vectors and all three turns.
        constexpr int source = 10 * kBoardWidth + 10;
        constexpr std::array<std::array<std::uint32_t, 3>, 4> seeds{{
            {{99U, 20U, 8U}},       // initial north, then left/right/reverse
            {{5U, 50U, 38U}},       // initial west
            {{85U, 73U, 198U}},     // initial south
            {{100U, 9U, 134U}},    // initial east
        }};
        constexpr std::array<std::array<std::uint32_t, 3>, 4> states{{
            {{3479131364U, 1184483641U, 20429437U}},
            {{803157710U, 1947135503U, 783081299U}},
            {{4268551774U, 3104497570U, 3418902131U}},
            {{354910409U, 1191175778U, 1505580339U}},
        }};
        for (int initial = 0; initial < 4; ++initial) {
            for (int turn = 0; turn < 3; ++turn) {
                auto level = blankLevel();
                level.top[at(1, 1)] = 0x6F;
                level.top[static_cast<std::size_t>(source)] = 0x5C;
                for (int direction = 0; direction < 4; ++direction)
                    level.top[static_cast<std::size_t>(neighborForMonsterDirection(source, direction))] = 0x01;
                const int initialDirection = initial;
                const int initialTarget = neighborForMonsterDirection(source, initialDirection);
                const int fallbackDirection = (initialDirection + (turn == 0 ? 1 : turn == 1 ? 3 : 2)) & 3;
                const int fallbackTarget = neighborForMonsterDirection(source, fallbackDirection);
                level.top[static_cast<std::size_t>(initialTarget)] = 0x01;
                level.top[static_cast<std::size_t>(fallbackTarget)] = 0x00;
                auto game = startWithMapMonsters(level);
                game.randomState = seeds[static_cast<std::size_t>(initial)][static_cast<std::size_t>(turn)];
                game.gameTicks = 3;
                game.advanceTicks(1);
                failures += !check(game.creatures[0].position == fallbackTarget &&
                                   (game.creatures[0].tile & 0x03) == fallbackDirection &&
                                   game.randomState == states[static_cast<std::size_t>(initial)][static_cast<std::size_t>(turn)],
                                   "Blob CRT seed covers initial vector and random fallback turn");
            }
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x43; // east-facing bug
        level.top[at(5, 4)] = 0x01;
        level.top[at(5, 6)] = 0x01;
        level.top[at(4, 5)] = 0x01;
        level.top[at(6, 5)] = 0x32; // TileTable blocks ForceRandom for monsters
        auto game = startWithMapMonsters(level);
        game.advanceTicks(2);
        failures += !check(game.creatures[0].position == at(5, 5) && game.randomState == 1U,
                           "creatures cannot enter random force floors");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x43; // east-facing bug
        level.top[at(5, 4)] = 0x01;
        level.top[at(5, 6)] = 0x01;
        level.top[at(4, 5)] = 0x01;
        level.top[at(6, 5)] = 0x02; // collectible chip, not Chip's sprite
        auto game = startWithMapMonsters(level);
        game.advanceTicks(2);
        failures += !check(game.creatures[0].position == at(5, 5) && game.top[at(6, 5)] == 0x02,
                           "creature treats the collectible chip tile as solid");
    }
    {
        auto level = blankLevel();
        level.top[at(5, 5)] = 0x6F;
        level.bottom[at(5, 5)] = 0x32;
        level.top[at(6, 5)] = 0x45; // west-facing fireball
        level.top[at(6, 4)] = 0x01;
        level.top[at(7, 5)] = 0x01;
        level.top[at(6, 6)] = 0x01;
        auto game = startWithMapMonsters(level);
        game.advanceTicks(2);
        failures += !check(game.outcome == Outcome::playing && game.playerPosition == at(5, 5),
                           "creature cannot catch Chip while Chip occupies a blocked random force floor");
    }
    {
        for (const auto terrain : {std::uint8_t{0x0E}, std::uint8_t{0x2D}}) {
            auto level = blankLevel();
            level.top[at(5, 5)] = 0x6F;
            level.bottom[at(5, 5)] = terrain;
            level.top[at(6, 5)] = 0x45; // west-facing fireball
            level.top[at(6, 4)] = 0x01;
            level.top[at(7, 5)] = 0x01;
            level.top[at(6, 6)] = 0x01;
            auto game = startWithMapMonsters(level);
            game.advanceTicks(2);
            failures += !check(game.outcome == Outcome::playing && game.playerPosition == at(5, 5),
                               "creatures cannot catch Chip on blocked block-template or gravel terrain");
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x43; // east-facing bug
        level.top[at(5, 4)] = 0x01;
        level.top[at(5, 6)] = 0x01;
        level.top[at(4, 5)] = 0x01;
        level.bottom[at(6, 5)] = 0x31; // visible upper layer remains ordinary floor
        auto game = startWithMapMonsters(level);
        game.advanceTicks(2);
        failures += !check(game.creatures[0].position == at(5, 5),
                           "creatures cannot enter a clone machine beneath an ordinary upper tile");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x4D; // west-facing tank overlays a clone machine.
        auto game = startWithMapMonsters(level);
        game.bottom[at(2, 1)] = 0x31;
        failures += !check(game.move(Direction::east) == MoveResult::blocked &&
                           game.outcome == Outcome::playing && game.playerPosition == at(1, 1) &&
                           game.creatures[0].alive,
                           "clone-machine lower tile blocks Chip before creature collision");
    }
    {
        // ChipCanEnterTile rejects the CloneMachine lower layer before any
        // visible-tile action (hazard, reveal, popup, pickup, or teleport).
        for (const std::uint8_t visible : {std::uint8_t{0x03}, std::uint8_t{0x1F},
                                           std::uint8_t{0x2C}, std::uint8_t{0x2E},
                                           std::uint8_t{0x29}, std::uint8_t{0x64}}) {
            auto level = blankLevel();
            level.top[at(1, 1)] = 0x6F;
            level.top[at(2, 1)] = visible;
            level.bottom[at(2, 1)] = 0x31;
            if (visible == 0x29) level.top[at(6, 1)] = 0x29;
            auto game = startWithMapMonsters(level);
            const auto result = game.move(Direction::east);
            failures += !check(result == MoveResult::blocked && game.outcome == Outcome::playing &&
                               game.playerPosition == at(1, 1) && game.top[at(2, 1)] == visible,
                               "CloneMachine lower layer precedes visible tile action " +
                               std::to_string(visible));
            if (visible == 0x64)
                failures += !check(game.keys[0] == 0,
                                   "Chip does not collect a key displayed over a CloneMachine");
        }
    }
    {
        // Keys and boots are transparent to ChipCanEnterTile. The visible
        // pickup action must not bypass eligibility on the lower layer.
        constexpr std::array<std::uint8_t, 4> blockedUnderlays{
            0x01, // Wall
            0x16, // locked BlueDoor
            0x25, // active ToggleWall
            0x31  // CloneMachine
        };
        for (std::uint8_t pickup = 0x64; pickup <= 0x6B; ++pickup) {
            for (const auto underlay : blockedUnderlays) {
                auto level = blankLevel();
                level.top[at(1, 1)] = 0x6F;
                level.top[at(2, 1)] = pickup;
                level.bottom[at(2, 1)] = underlay;
                auto game = startWithMapMonsters(level);
                failures += !check(game.move(Direction::east) == MoveResult::blocked &&
                                   game.outcome == Outcome::playing &&
                                   game.playerPosition == at(1, 1) &&
                                   game.top[at(2, 1)] == pickup &&
                                   game.bottom[at(2, 1)] == underlay &&
                                   game.keys == std::array<unsigned, 4>{0, 0, 0, 0} &&
                                   game.boots == std::array<bool, 4>{false, false, false, false},
                                   "key/boot " + std::to_string(pickup) +
                                   " cannot activate over blocking underlay " +
                                   std::to_string(underlay));
            }

            auto floorLevel = blankLevel();
            floorLevel.top[at(1, 1)] = 0x6F;
            floorLevel.top[at(2, 1)] = pickup;
            auto floorPickup = startWithMapMonsters(floorLevel);
            (void)floorPickup.move(Direction::east);
            const bool isKey = pickup <= 0x67;
            const auto itemIndex = static_cast<std::size_t>(
                isKey ? pickup - 0x64 : pickup - 0x68);
            failures += !check(floorPickup.playerPosition == at(2, 1) &&
                               floorPickup.top[at(2, 1)] == 0x00 &&
                               (isKey ? floorPickup.keys[itemIndex] == 1
                                      : floorPickup.boots[itemIndex]),
                               "key/boot " + std::to_string(pickup) +
                               " is collected over ordinary floor");
        }

        auto keyedDoorLevel = blankLevel();
        keyedDoorLevel.top[at(1, 1)] = 0x6F;
        keyedDoorLevel.top[at(2, 1)] = 0x64; // BlueKey over a BlueDoor.
        keyedDoorLevel.bottom[at(2, 1)] = 0x16;
        auto keyedDoor = startWithMapMonsters(keyedDoorLevel);
        keyedDoor.keys[0] = 1;
        (void)keyedDoor.move(Direction::east);
        failures += !check(keyedDoor.playerPosition == at(2, 1) &&
                           keyedDoor.keys[0] == 2 && keyedDoor.top[at(2, 1)] == 0x16,
                           "an existing key permits the transparent pickup over its matching door");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x60; // paramecium faces north; first tries right/east
        auto game = startWithMapMonsters(level);
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.creatures[0].position == at(6, 5) &&
                           (game.creatures[0].tile & 0x03) == 3,
                           "paramecium turns right before trying straight");

        level.top[at(5, 5)] = 0x60;
        level.top[at(6, 5)] = 0x01; // right blocked; straight north is open
        auto forwardFallback = startWithMapMonsters(level);
        forwardFallback.gameTicks = 1;
        forwardFallback.advanceTicks(1);
        failures += !check(forwardFallback.creatures[0].position == at(5, 4),
                           "paramecium tries forward after its right turn is blocked");
    }
    {
        // For each deterministic turn-order family, leave only one candidate
        // direction open and block every earlier candidate. This checks every
        // facing and fallback rank against MonsterLoop's source order.
        struct MovementOrder {
            std::uint8_t kind;
            std::array<int, 4> turnOffsets;
            int count;
            const char* name;
        };
        const std::array<MovementOrder, 6> orders{{
            {0x40, {1, 0, 3, 2}, 4, "Bug"},
            {0x44, {0, 3, 1, 2}, 4, "Fireball"},
            {0x48, {0, 2, 0, 0}, 2, "Ball"},
            {0x4C, {0, 0, 0, 0}, 1, "Tank"},
            {0x50, {0, 1, 3, 2}, 4, "Glider"},
            {0x60, {3, 0, 1, 2}, 4, "Paramecium"},
        }};
        constexpr int source = 10 * kBoardWidth + 10;
        for (const auto& order : orders) {
            for (int facing = 0; facing < 4; ++facing) {
                for (int rank = 0; rank < order.count; ++rank) {
                    auto level = blankLevel();
                    level.top[at(1, 1)] = 0x6F;
                    level.top[static_cast<std::size_t>(source)] =
                        static_cast<std::uint8_t>(order.kind | facing);
                    for (int direction = 0; direction < 4; ++direction)
                        level.top[static_cast<std::size_t>(neighborForMonsterDirection(source, direction))] = 0x01;
                    const int expectedDirection = (facing + order.turnOffsets[static_cast<std::size_t>(rank)]) & 3;
                    const int destination = neighborForMonsterDirection(source, expectedDirection);
                    level.top[static_cast<std::size_t>(destination)] = 0x00;
                    auto game = startWithMapMonsters(level);
                    game.gameTicks = 1;
                    game.advanceTicks(1);
                    const std::string label = std::string(order.name) + " facing " +
                        std::to_string(facing) + " selects movement-order candidate " +
                        std::to_string(rank + 1);
                    failures += !check(game.creatures.size() == 1 &&
                                       game.creatures[0].position == destination &&
                                       (game.creatures[0].tile & 0x03) == expectedDirection,
                                       label);
                }
            }
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x48; // ball faces north
        level.bottom[at(5, 5)] = 0x2B;
        level.top[at(5, 4)] = 0x01;
        auto game = startWithMapMonsters(level);
        game.advanceTicks(2);
        failures += !check(game.creatures[0].position == at(5, 5),
                           "ball does not reverse while trapped when its forward path is blocked");
    }
    {
        // With no prior MoveMonster attempt in this loop pass, the original
        // Bug routine leaves xnewdir/ynewdir uninitialized on a CloneMachine.
        // Keep that undefined case deterministic using the prototype vector.
        for (int facing = 0; facing < 4; ++facing) {
            auto level = blankLevel();
            level.top[at(1, 1)] = 0x6F;
            level.top[at(2, 1)] = 0x24; // linked clone button
            constexpr int source = 10 * kBoardWidth + 10;
            level.top[static_cast<std::size_t>(source)] = static_cast<std::uint8_t>(0x40 | facing);
            level.bottom[static_cast<std::size_t>(source)] = 0x31;
            OptionalField link;
            link.id = 5;
            link.raw = {2, 0, 1, 0, 10, 0, 10, 0};
            level.fields.push_back(link);
            auto game = startWithMapMonsters(level);
            (void)game.move(Direction::east);
            const int destination = neighborForMonsterDirection(source, facing);
            game.gameTicks = 1;
            game.advanceTicks(1);
            failures += !check(game.creatures.size() == 1 && game.creatures[0].alive &&
                               game.creatures[0].position == destination &&
                               (game.creatures[0].tile & 0x03) == facing,
                               "Bug on a clone machine uses deterministic stored direction " +
                               std::to_string(facing));
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x24; // CloneButton emits the Bug after the Ball.
        level.top[at(5, 5)] = 0x4B; // Ball faces east.
        level.top[at(6, 5)] = 0x01; // Straight attempt blocked.
        level.top[at(4, 5)] = 0x01; // Reverse attempt blocked last.
        constexpr int bugSource = 10 * kBoardWidth + 10;
        level.top[static_cast<std::size_t>(bugSource)] = 0x40; // Bug faces north.
        level.bottom[static_cast<std::size_t>(bugSource)] = 0x31; // CloneMachine.
        OptionalField orderedMonsters;
        orderedMonsters.id = 10;
        orderedMonsters.raw = {5, 5}; // Ball is processed before the emitted Bug.
        level.fields.push_back(orderedMonsters);
        OptionalField cloneLink;
        cloneLink.id = 5;
        cloneLink.raw = {2, 0, 1, 0, 10, 0, 10, 0};
        level.fields.push_back(cloneLink);
        auto game = GameState::start(level);
        (void)game.move(Direction::east);
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.creatures.size() == 2 &&
                           game.creatures[0].position == at(5, 5) &&
                           game.creatures[1].position == bugSource - 1 &&
                           (game.creatures[1].tile & 0x03) == 1,
                           std::string("clone-spawned Bug reuses the preceding monster's last attempted direction ") +
                           "(count=" + std::to_string(game.creatures.size()) +
                           (game.creatures.size() > 1 ? ", bug-cell=" + std::to_string(game.creatures[1].position) +
                               ", facing=" + std::to_string(game.creatures[1].tile & 0x03) : "") + ")");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x24; // Emits a Bug facing west.
        level.top[at(5, 5)] = 0x4B; // Ball approaches eastward.
        level.top[at(6, 5)] = 0x12; // ForceNorth rewrites SlideMovement's vector.
        constexpr int machine = 10 * kBoardWidth + 10;
        level.top[static_cast<std::size_t>(machine)] = 0x41;
        level.bottom[static_cast<std::size_t>(machine)] = 0x31;
        OptionalField monsters;
        monsters.id = 10;
        monsters.raw = {5, 5};
        level.fields.push_back(monsters);
        OptionalField clone;
        clone.id = 5;
        clone.raw = {2, 0, 1, 0, 10, 0, 10, 0};
        level.fields.push_back(clone);
        auto game = GameState::start(level);
        (void)game.move(Direction::east);
        game.gameTicks = 1;
        game.advanceTicks(1);
        const auto& bug = game.creatures[1];
        failures += !check(bug.position == machine - kBoardWidth &&
                           (bug.tile & 0x03) == 0 &&
                           game.creatures[0].movementDirection == 0,
                           "constrained Bug inherits the force-floor direction rewritten by the prior MoveMonster");
    }
    {
        for (const auto [kind, name] : {std::pair{0x54, "Teeth"}, std::pair{0x60, "Paramecium"}}) {
            auto level = blankLevel();
            level.top[at(1, 1)] = 0x6F;
            level.top[at(5, 5)] = 0x4B; // Ball faces east.
            level.top[at(6, 5)] = 0x01; // Straight attempt blocked.
            level.top[at(4, 5)] = 0x01; // Reverse attempt blocked last, facing west.
            constexpr int source = 10 * kBoardWidth + 10;
            level.top[static_cast<std::size_t>(source)] = static_cast<std::uint8_t>(kind); // Prototype faces north.
            level.bottom[static_cast<std::size_t>(source)] = 0x2B; // Open linked trap.
            OptionalField openTrap;
            openTrap.id = 4;
            openTrap.raw = {20, 0, 20, 0, 10, 0, 10, 0, 0, 0};
            level.fields.push_back(openTrap);
            auto game = startWithMapMonsters(level);
            game.gameTicks = 3; // Tick 4 runs both families.
            game.advanceTicks(1);
            failures += !check(game.creatures.size() == 2 &&
                               game.creatures[0].position == at(5, 5) &&
                               game.creatures[1].position == source - 1 &&
                               (game.creatures[1].tile & 0x03) == 1,
                               std::string(name) +
                               " on an open trap reuses the preceding creature's last attempted vector");
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        const std::array<std::uint8_t, 9> kinds{0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60};
        for (std::size_t i = 0; i < kinds.size(); ++i) level.top[at(3 + static_cast<int>(i), 5)] = kinds[i];
        auto game = startWithMapMonsters(level);
        failures += !check(game.creatures.size() == kinds.size(), "all nine creature families load");
        game.advanceTicks(2);
        bool moved = false;
        for (const auto& creature : game.creatures) moved = moved || creature.position != at(3 + static_cast<int>(&creature - game.creatures.data()), 5);
        failures += !check(moved, "creature scheduler advances the loaded families");
    }
    {
        constexpr std::array<std::uint8_t, 9> kinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60
        };
        constexpr int source = 10 * kBoardWidth + 10;
        const int east = neighborForMonsterDirection(source, 3);
        for (const auto kind : kinds) {
            auto blockedLevel = blankLevel();
            blockedLevel.top[at(20, 20)] = 0x6F;
            blockedLevel.top[static_cast<std::size_t>(source)] = static_cast<std::uint8_t>(kind | 3U);
            blockedLevel.top[static_cast<std::size_t>(east)] = 0x0A;
            for (const int direction : {0, 1, 2})
                blockedLevel.top[static_cast<std::size_t>(neighborForMonsterDirection(source, direction))] = 0x01;
            auto blocked = startWithMapMonsters(blockedLevel);
            blocked.gameTicks = 3; // Tick 4 runs every family, including Teeth and Blob.
            blocked.advanceTicks(1);
            failures += !check(blocked.creatures.size() == 1 && blocked.creatures[0].alive &&
                               blocked.creatures[0].position == source && blocked.top[east] == 0x0A,
                               "ordinary " + std::to_string(kind) + " turn treats a block as an occupied tile");

            auto slidingLevel = blankLevel();
            slidingLevel.top[at(20, 20)] = 0x6F;
            slidingLevel.top[static_cast<std::size_t>(source)] = static_cast<std::uint8_t>(kind | 3U);
            slidingLevel.bottom[static_cast<std::size_t>(source)] = 0x0C;
            slidingLevel.top[static_cast<std::size_t>(east)] = 0x0A;
            auto sliding = startWithMapMonsters(slidingLevel);
            sliding.gameTicks = 1;
            sliding.slipOrder.push_back({SlipperKind::creature, 0});
            sliding.advanceTicks(1);
            failures += !check(sliding.creatures.size() == 1 && sliding.creatures[0].alive &&
                               sliding.creatures[0].position == source - 1 &&
                               sliding.slipOrder.empty() && sliding.top[east] == 0x0A,
                               "sliding " + std::to_string(kind) + " reverses off ice when a block stops it");
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x23;
        level.top[at(3, 1)] = 0x25;
        level.top[at(4, 1)] = 0x26;
        level.top[at(2, 2)] = 0x28;
        level.top[at(6, 6)] = 0x4C;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.top[at(3, 1)] == 0x26 && game.top[at(4, 1)] == 0x25,
                           "toggle button swaps toggle walls and floors");
        (void)game.move(Direction::south);
        failures += !check(game.creatures[0].tile == 0x4E && game.creatures[0].tankDirection == 2,
                           "tank button reverses tank direction by 180 degrees");

        auto hiddenToggleLevel = blankLevel();
        hiddenToggleLevel.top[at(1, 1)] = 0x6F;
        hiddenToggleLevel.top[at(2, 1)] = 0x23;
        hiddenToggleLevel.top[at(5, 5)] = 0x0A; // toggle wall is hidden beneath a block
        hiddenToggleLevel.bottom[at(5, 5)] = 0x25;
        auto hiddenToggle = startWithMapMonsters(hiddenToggleLevel);
        (void)hiddenToggle.move(Direction::east);
        failures += !check(hiddenToggle.bottom[at(5, 5)] == 0x25,
                           "toggle button does not alter lower terrain omitted from the original toggle list");

        auto revealedToggleLevel = blankLevel();
        revealedToggleLevel.top[at(1, 1)] = 0x6F;
        revealedToggleLevel.top[at(2, 1)] = 0x23;
        revealedToggleLevel.top[at(4, 2)] = 0x0A; // Block hides a ToggleWall at initialization.
        revealedToggleLevel.bottom[at(4, 2)] = 0x25;
        auto revealedToggle = startWithMapMonsters(revealedToggleLevel);
        failures += !check(std::find(revealedToggle.togglePositions.begin(),
                                     revealedToggle.togglePositions.end(), at(4, 2)) ==
                               revealedToggle.togglePositions.end(),
                           "InitBoard omits a ToggleWall hidden below a block");
        (void)revealedToggle.move(Direction::east);  // Press ToggleButton.
        (void)revealedToggle.move(Direction::south);
        (void)revealedToggle.move(Direction::east);
        (void)revealedToggle.move(Direction::east);  // Push block and expose the wall.
        failures += !check(revealedToggle.top[at(4, 2)] == 0x25 &&
                           revealedToggle.bottom[at(4, 2)] == 0x00,
                           "moving a block exposes its unregistered ToggleWall underlay");
        (void)revealedToggle.move(Direction::north);
        (void)revealedToggle.move(Direction::west);  // Press ToggleButton again.
        failures += !check(revealedToggle.top[at(4, 2)] == 0x25 &&
                           revealedToggle.toggleState == false,
                           "later ToggleButton presses do not register a wall exposed after InitBoard");

        auto creatureToggleLevel = blankLevel();
        creatureToggleLevel.top[at(1, 1)] = 0x6F;
        creatureToggleLevel.top[at(2, 1)] = 0x23;
        creatureToggleLevel.top[at(5, 5)] = 0x4C;
        creatureToggleLevel.bottom[at(5, 5)] = 0x25;
        auto creatureToggle = startWithMapMonsters(creatureToggleLevel);
        (void)creatureToggle.move(Direction::east);
        failures += !check(creatureToggle.bottom[at(5, 5)] == 0x26,
                           "toggle scan includes lower terrain beneath a creature");

        auto layeredToggleLevel = blankLevel();
        layeredToggleLevel.top[at(1, 1)] = 0x6F;
        layeredToggleLevel.top[at(2, 1)] = 0x23;
        layeredToggleLevel.top[at(5, 5)] = 0x25;
        layeredToggleLevel.bottom[at(5, 5)] = 0x26;
        auto layeredToggle = startWithMapMonsters(layeredToggleLevel);
        failures += !check(std::count(layeredToggle.togglePositions.begin(),
                                      layeredToggle.togglePositions.end(), at(5, 5)) == 1,
                           "toggle scan registers one coordinate even when both layers are toggle tiles");
        (void)layeredToggle.move(Direction::east);
        failures += !check(layeredToggle.top[at(5, 5)] == 0x26 &&
                           layeredToggle.bottom[at(5, 5)] == 0x25,
                           "toggle activation swaps both layers at a registered coordinate");
        (void)layeredToggle.move(Direction::west);
        (void)layeredToggle.move(Direction::east);
        failures += !check(layeredToggle.top[at(5, 5)] == 0x25 &&
                           layeredToggle.bottom[at(5, 5)] == 0x26,
                           "second toggle activation restores both original layer values");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 2)] = 0x6F;
        level.top[at(3, 3)] = 0x4C; // north-facing tank runs into a wall
        level.top[at(3, 2)] = 0x01;
        level.top[at(2, 2)] = 0x28;
        auto game = startWithMapMonsters(level);
        game.advanceTicks(2);
        failures += !check(game.creatures[0].position == at(3, 3) &&
                           game.creatures[0].tile == 0x4C && game.creatures[0].tankDirection == -2,
                           "blocked tank stops while retaining its north-facing sprite");
        game.advanceTicks(4);
        failures += !check(game.creatures[0].position == at(3, 3) && game.creatures[0].tile == 0x4C,
                           "stopped tank remains stationary on later creature turns");
        (void)game.move(Direction::east);
        game.gameTicks = 7;
        game.advanceTicks(1);
        failures += !check(game.creatures[0].position == at(3, 4) &&
                           game.creatures[0].tile == 0x4E && game.creatures[0].tankDirection == 2,
                           "tank button reverses and restarts a stopped tank");

        auto cloneMachineLevel = blankLevel();
        cloneMachineLevel.top[at(1, 1)] = 0x6F;
        cloneMachineLevel.top[at(5, 5)] = 0x4F; // east-facing tank
        auto cloneMachineTank = startWithMapMonsters(cloneMachineLevel);
        // Initial field-10 creatures over machines are skipped by the source;
        // place a live tank on the machine as if it arrived there later.
        cloneMachineTank.bottom[at(5, 5)] = 0x31;
        cloneMachineTank.top[at(6, 5)] = 0x01;
        cloneMachineTank.advanceTicks(2);
        failures += !check(cloneMachineTank.creatures[0].position == at(5, 5) &&
                           cloneMachineTank.creatures[0].tile == 0x4F &&
                           cloneMachineTank.creatures[0].tankDirection == -2,
                           "blocked tank stops on a clone machine while retaining its facing sprite");
        cloneMachineTank.top[at(6, 5)] = 0x00;
        cloneMachineTank.advanceTicks(2);
        failures += !check(cloneMachineTank.creatures[0].position == at(5, 5) &&
                           cloneMachineTank.creatures[0].tile == 0x4F &&
                           cloneMachineTank.creatures[0].tankDirection == -2,
                           "stopped clone-machine tank stays stopped after the path opens");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(4, 2)] = 0x4E; // south-facing tank enters an east force floor
        level.top[at(4, 3)] = 0x13;
        auto game = startWithMapMonsters(level);
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.creatures[0].position == at(5, 3) &&
                           game.creatures[0].tankDirection == 3,
                           "tank direction follows force-floor motion during its slide");
        game.advanceTicks(2);
        failures += !check(game.creatures[0].position == at(6, 3),
                           "tank continues in its force-floor heading on the next game tick");
    }
    {
        auto level = blankLevel();
        const int source = at(4, 4);
        level.top[at(1, 1)] = 0x6F;
        level.top[static_cast<std::size_t>(source)] = 0x4F; // east-facing tank
        level.bottom[static_cast<std::size_t>(source)] = 0x0C;
        level.top[static_cast<std::size_t>(at(5, 4))] = 0x01;
        level.top[static_cast<std::size_t>(at(3, 4))] = 0x01;
        auto blockedTankSlip = startWithMapMonsters(level);
        blockedTankSlip.gameTicks = 1;
        // A Tank Button may reverse Monster.xdir while the independent slipper
        // entry still points east. Both ice-slide attempts then fail.
        blockedTankSlip.creatures[0].tankDirection = 1;
        blockedTankSlip.creatures[0].movementDirection = 1;
        blockedTankSlip.slipOrder.push_back({SlipperKind::creature, 0, 3});
        blockedTankSlip.advanceTicks(1);
        failures += !check(blockedTankSlip.creatures[0].position == source &&
                           blockedTankSlip.creatures[0].tile == 0x4D &&
                           blockedTankSlip.creatures[0].tankDirection == 1 &&
                           blockedTankSlip.creatures[0].movementDirection == 1,
                           "blocked tank slip preserves movement state (tile=" +
                           std::to_string(blockedTankSlip.creatures[0].tile) + ", tank=" +
                           std::to_string(blockedTankSlip.creatures[0].tankDirection) + ", move=" +
                           std::to_string(blockedTankSlip.creatures[0].movementDirection) + ", pos=" +
                           std::to_string(blockedTankSlip.creatures[0].position) + ")");
    }
    {
        auto level = blankLevel();
        const int source = at(4, 4);
        level.top[at(1, 1)] = 0x6F;
        level.top[static_cast<std::size_t>(source)] = 0x4F;
        level.bottom[static_cast<std::size_t>(source)] = 0x0C;
        auto tankLeavesIce = startWithMapMonsters(level);
        tankLeavesIce.gameTicks = 1;
        tankLeavesIce.creatures[0].tankDirection = 1;
        tankLeavesIce.creatures[0].movementDirection = 1;
        tankLeavesIce.slipOrder.push_back({SlipperKind::creature, 0, 3});
        tankLeavesIce.advanceTicks(1);
        failures += !check(tankLeavesIce.creatures[0].position == at(5, 4) &&
                           tankLeavesIce.creatures[0].tankDirection == 3 &&
                           tankLeavesIce.creatures[0].movementDirection == 3,
                           "successful tank slide commits its slipper vector after leaving ice");

        auto portalLevel = blankLevel();
        portalLevel.top[at(1, 1)] = 0x6F;
        portalLevel.top[static_cast<std::size_t>(source)] = 0x4F;
        portalLevel.bottom[static_cast<std::size_t>(source)] = 0x0C;
        portalLevel.top[static_cast<std::size_t>(at(5, 4))] = 0x29;
        portalLevel.top[static_cast<std::size_t>(at(9, 4))] = 0x29;
        auto tankTeleports = startWithMapMonsters(portalLevel);
        tankTeleports.gameTicks = 1;
        tankTeleports.creatures[0].tankDirection = 1;
        tankTeleports.creatures[0].movementDirection = 1;
        tankTeleports.slipOrder.push_back({SlipperKind::creature, 0, 3});
        tankTeleports.advanceTicks(1);
        failures += !check(tankTeleports.creatures[0].position == at(9, 4) &&
                           tankTeleports.creatures[0].tankDirection == 3 &&
                           tankTeleports.creatures[0].movementDirection == 3,
                           "successful tank teleport commits its slipper vector");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x0A;
        level.top[at(3, 1)] = 0x23;
        level.top[at(4, 1)] = 0x25;
        level.top[at(5, 1)] = 0x26;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(2, 1) && game.surface(at(3, 1)) == 0x0A,
                           "pushed block lands on toggle button");
        failures += !check(game.top[at(4, 1)] == 0x26 && game.top[at(5, 1)] == 0x25,
                           "pushed block activates toggle button after movement");

        auto delayedPushButtonsLevel = blankLevel();
        delayedPushButtonsLevel.top[at(1, 1)] = 0x6F;
        delayedPushButtonsLevel.top[at(2, 1)] = 0x0A;
        delayedPushButtonsLevel.bottom[at(2, 1)] = 0x24; // Chip enters this CloneButton after the push.
        delayedPushButtonsLevel.top[at(3, 1)] = 0x28; // Block lands on TankButton, pressed afterward.
        delayedPushButtonsLevel.top[at(8, 8)] = 0x4C; // North-facing Tank clone template.
        delayedPushButtonsLevel.bottom[at(8, 8)] = 0x31;
        OptionalField delayedPushCloneLink;
        delayedPushCloneLink.id = 5;
        delayedPushCloneLink.raw = {2, 0, 1, 0, 8, 0, 8, 0};
        delayedPushButtonsLevel.fields.push_back(delayedPushCloneLink);
        auto delayedPushButtons = startWithMapMonsters(delayedPushButtonsLevel);
        (void)delayedPushButtons.move(Direction::east);
        failures += !check(delayedPushButtons.playerPosition == at(2, 1) &&
                           delayedPushButtons.creatures.size() == 1 &&
                           delayedPushButtons.creatures[0].tile == 0x4E &&
                           delayedPushButtons.creatures[0].tankDirection == 2 &&
                           delayedPushButtons.top[at(8, 8)] == 0x4E,
                           "push completes Chip's CloneButton before delayed TankButton reverses its new clone");

        auto mixedSlipToggleLevel = blankLevel();
        mixedSlipToggleLevel.top[at(1, 1)] = 0x6F;
        mixedSlipToggleLevel.top[at(2, 4)] = 0x0A;
        mixedSlipToggleLevel.bottom[at(2, 4)] = 0x15; // ForceEast moves the block onto the button.
        mixedSlipToggleLevel.top[at(3, 4)] = 0x23;
        mixedSlipToggleLevel.top[at(4, 4)] = 0x43; // East-facing Bug is next in SlipList.
        mixedSlipToggleLevel.top[at(5, 4)] = 0x25; // Initially closed ToggleWall.
        auto mixedSlipToggle = startWithMapMonsters(mixedSlipToggleLevel);
        mixedSlipToggle.blockDirections[at(2, 4)] = static_cast<std::int8_t>(Direction::east);
        mixedSlipToggle.slipOrder.push_back({SlipperKind::block, at(2, 4)});
        mixedSlipToggle.slipOrder.push_back({SlipperKind::creature, 0, 3});
        mixedSlipToggle.advanceTicks(1);
        failures += !check(mixedSlipToggle.toggleState && mixedSlipToggle.top[at(5, 4)] == 0x26 &&
                           mixedSlipToggle.creatures[0].position == at(5, 4) &&
                           mixedSlipToggle.slipOrder.empty(),
                           "block toggling a wall lets the next queued creature enter it in the same SlipLoop pass");
    }
    {
        auto level = blankLevel();
        level.top[at(2, 2)] = 0x6F;
        level.top[at(2, 3)] = 0x06; // PanelN rejects entry from its south side
        auto southEntry = startWithMapMonsters(level);
        failures += !check(southEntry.move(Direction::south) == MoveResult::blocked,
                           "north panel blocks entry from the south");

        auto northLevel = blankLevel();
        northLevel.top[at(2, 2)] = 0x6F;
        northLevel.top[at(2, 1)] = 0x06;
        auto northEntry = startWithMapMonsters(northLevel);
        failures += !check(northEntry.move(Direction::north) == MoveResult::moved &&
                           northEntry.playerPosition == at(2, 1),
                           "north panel allows entry from the north");

        auto panelLevel = blankLevel();
        panelLevel.top[at(2, 1)] = 0x6F;
        panelLevel.bottom[at(2, 1)] = 0x06;
        auto panelExit = startWithMapMonsters(panelLevel);
        failures += !check(panelExit.move(Direction::north) == MoveResult::blocked &&
                           panelExit.move(Direction::south) == MoveResult::moved,
                           "north panel blocks northward exit and allows southward exit");

        auto upperPanelExitLevel = blankLevel();
        upperPanelExitLevel.top[at(1, 2)] = 0x6F;
        upperPanelExitLevel.top[at(2, 2)] = 0x06;
        auto upperPanelExit = startWithMapMonsters(upperPanelExitLevel);
        failures += !check(upperPanelExit.move(Direction::east) == MoveResult::moved &&
                           upperPanelExit.move(Direction::north) == MoveResult::blocked,
                           "panel exit restriction uses the player's visible tile when it is on the upper layer");

        auto iceCornerExitLevel = blankLevel();
        iceCornerExitLevel.top[at(2, 3)] = 0x6F;
        iceCornerExitLevel.top[at(2, 2)] = 0x1A;
        iceCornerExitLevel.bottom[at(2, 2)] = 0x1A;
        auto iceCornerExit = startWithMapMonsters(iceCornerExitLevel);
        iceCornerExit.boots[2] = true;
        failures += !check(iceCornerExit.move(Direction::north) == MoveResult::moved &&
                           iceCornerExit.move(Direction::north) == MoveResult::moved &&
                           iceCornerExit.playerPosition == at(2, 1),
                           "ice corners do not inherit one-way panel exit blocking");

        auto blockPanelExitLevel = blankLevel();
        blockPanelExitLevel.top[at(4, 5)] = 0x6F;
        blockPanelExitLevel.top[at(5, 5)] = 0x0A;
        blockPanelExitLevel.bottom[at(5, 5)] = 0x06; // PanelN blocks northward exit.
        auto blockPanelExit = startWithMapMonsters(blockPanelExitLevel);
        blockPanelExit.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::north);
        blockPanelExit.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        blockPanelExit.advanceTicks(1);
        failures += !check(blockPanelExit.top[at(5, 5)] == 0x0A &&
                           blockPanelExit.slipOrder.size() == 1 &&
                           blockPanelExit.blockDirections[at(5, 5)] ==
                               static_cast<std::int8_t>(Direction::north),
                           "slipping block retains its heading when a one-way panel blocks its exit");
        failures += !check(blockPanelExit.move(Direction::east) == MoveResult::moved &&
                           blockPanelExit.top[at(6, 5)] == 0x0A &&
                           blockPanelExit.slipOrder.empty(),
                           "Chip can redirect a slipping block perpendicular to its heading off a panel");

        auto southeastLevel = blankLevel();
        southeastLevel.top[at(1, 2)] = 0x6F;
        southeastLevel.top[at(2, 2)] = 0x30;
        auto southeastEntry = startWithMapMonsters(southeastLevel);
        failures += !check(southeastEntry.move(Direction::east) == MoveResult::moved,
                           "southeast panel allows entry from the west");
        auto blockedSoutheastLevel = blankLevel();
        blockedSoutheastLevel.top[at(3, 2)] = 0x6F;
        blockedSoutheastLevel.top[at(2, 2)] = 0x30;
        auto blockedSoutheastEntry = startWithMapMonsters(blockedSoutheastLevel);
        failures += !check(blockedSoutheastEntry.move(Direction::west) == MoveResult::blocked,
                           "southeast panel blocks entry from the east");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x1F;
        auto fakeWall = startWithMapMonsters(level);
        failures += !check(fakeWall.move(Direction::east) == MoveResult::blocked &&
                           fakeWall.top[at(2, 1)] == 0x01 && fakeWall.playerPosition == at(1, 1),
                           "fake wall reveals as a solid wall when touched");

        for (const std::uint8_t blockedTile : {std::uint8_t{0x20}, std::uint8_t{0x33}, std::uint8_t{0x39}}) {
            auto blockedTileLevel = blankLevel();
            blockedTileLevel.top[at(1, 1)] = 0x6F;
            blockedTileLevel.top[at(2, 1)] = blockedTile;
            auto blockedTileGame = startWithMapMonsters(blockedTileLevel);
            failures += !check(blockedTileGame.move(Direction::east) == MoveResult::blocked &&
                               blockedTileGame.playerPosition == at(1, 1),
                               "unused and animated-only tile IDs remain blocked to Chip");
        }

        level.top[at(2, 1)] = 0x2C;
        auto hiddenWall = startWithMapMonsters(level);
        failures += !check(hiddenWall.move(Direction::east) == MoveResult::blocked &&
                           hiddenWall.top[at(2, 1)] == 0x01,
                           "hidden wall reveals as a solid wall when touched");

        level.top[at(2, 1)] = 0x2E;
        auto popupWall = startWithMapMonsters(level);
        failures += !check(popupWall.move(Direction::east) == MoveResult::moved &&
                           popupWall.playerPosition == at(2, 1) && popupWall.top[at(2, 1)] == 0x01,
                           "popup wall rises as Chip passes onto the tile");
        (void)popupWall.move(Direction::west);
        failures += !check(popupWall.move(Direction::east) == MoveResult::blocked,
                           "risen popup wall blocks a later return");

        level.top[at(2, 1)] = 0x1E;
        level.bottom[at(2, 1)] = 0x03;
        auto fakeFloor = startWithMapMonsters(level);
        failures += !check(fakeFloor.move(Direction::east) == MoveResult::moved &&
                           fakeFloor.outcome == Outcome::playing && fakeFloor.top[at(2, 1)] == 0x00,
                           "fake floor becomes ordinary floor rather than revealing its underlayer");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x2F;
        level.hint = "Walk north after collecting the key.";
        level.timeLimit = 60;
        auto game = startWithMapMonsters(level);
        const auto initialTime = game.secondsLeft;
        failures += !check(game.move(Direction::east) == MoveResult::moved &&
                           game.playerPosition == at(2, 1) && game.message == "Hint: " + level.hint &&
                           game.standingOnHint(),
                           "Hint entry exposes its original text and activates the scoreboard hint state");
        failures += !check(game.secondsLeft == initialTime,
                           "reading a hint does not alter the level timer");
        failures += !check(game.move(Direction::east) == MoveResult::moved && !game.standingOnHint(),
                           "leaving a Hint tile hides the scoreboard hint state");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.bottom[at(1, 1)] = 0x2F;
        const auto game = startWithMapMonsters(level);
        failures += !check(game.standingOnHint(),
                           "a level that starts on a Hint tile shows its hint immediately");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x27;
        level.top[at(3, 1)] = 0x2B;
        OptionalField links;
        links.id = 4;
        links.raw = {2, 0, 1, 0, 3, 0, 1, 0, 1, 0}; // button (2,1) -> closed trap (3,1)
        level.fields.push_back(links);
        auto game = startWithMapMonsters(level);
        failures += !check(game.traps.size() == 1 && game.traps[0].closed,
                           "trap links load from level metadata");
        (void)game.move(Direction::east);
        failures += !check(!game.traps[0].closed && game.playerPosition == at(2, 1),
                           "trap button opens its linked trap");
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(3, 1) && game.traps[0].closed,
                           "entering a linked trap closes it behind the player");
        failures += !check(game.move(Direction::west) == MoveResult::blocked,
                           "closed trap holds the player until its button is pressed");

        auto samePassReleaseLevel = blankLevel();
        samePassReleaseLevel.top[at(1, 1)] = 0x6F;
        samePassReleaseLevel.top[at(1, 4)] = 0x4F; // East-facing Tank reaches the button first.
        samePassReleaseLevel.top[at(2, 4)] = 0x27;
        samePassReleaseLevel.top[at(5, 4)] = 0x40; // Bug starts on the linked closed trap.
        samePassReleaseLevel.bottom[at(5, 4)] = 0x2B;
        OptionalField samePassReleaseLink;
        samePassReleaseLink.id = 4;
        samePassReleaseLink.raw = {2, 0, 4, 0, 5, 0, 4, 0, 1, 0};
        samePassReleaseLevel.fields.push_back(samePassReleaseLink);
        auto samePassRelease = startWithMapMonsters(samePassReleaseLevel);
        samePassRelease.gameTicks = 1; // Run MonsterLoop on the next tick.
        samePassRelease.advanceTicks(1);
        failures += !check(samePassRelease.creatures.size() == 2 &&
                           samePassRelease.creatures[0].position == at(2, 4) &&
                           samePassRelease.creatures[1].position == at(6, 4) &&
                           !samePassRelease.traps[0].closed,
                           "Tank opening a trap button releases a later creature in the same MonsterLoop pass");

        auto samePassToggleLevel = blankLevel();
        samePassToggleLevel.top[at(1, 1)] = 0x6F;
        samePassToggleLevel.top[at(1, 4)] = 0x4F; // First Tank presses ToggleButton.
        samePassToggleLevel.top[at(2, 4)] = 0x23;
        samePassToggleLevel.top[at(4, 4)] = 0x25; // Second Tank is blocked until toggled.
        samePassToggleLevel.top[at(5, 4)] = 0x4D; // West-facing Tank moves after the first.
        auto samePassToggle = startWithMapMonsters(samePassToggleLevel);
        samePassToggle.gameTicks = 1;
        samePassToggle.advanceTicks(1);
        failures += !check(samePassToggle.creatures.size() == 2 &&
                           samePassToggle.creatures[0].position == at(2, 4) &&
                           samePassToggle.creatures[1].position == at(4, 4) &&
                           samePassToggle.top[at(4, 4)] == 0x26 && samePassToggle.toggleState,
                           "MonsterLoop ToggleButton changes a wall before the later creature's move");

        auto contestedDestinationLevel = blankLevel();
        contestedDestinationLevel.top[at(1, 1)] = 0x6F;
        contestedDestinationLevel.top[at(5, 4)] = 0x4E; // South-facing Tank has earlier row-major order.
        contestedDestinationLevel.top[at(4, 5)] = 0x4F; // East-facing Tank targets the same cell second.
        auto contestedDestination = startWithMapMonsters(contestedDestinationLevel);
        contestedDestination.gameTicks = 1;
        contestedDestination.advanceTicks(1);
        failures += !check(contestedDestination.creatures.size() == 2 &&
                           contestedDestination.creatures[0].position == at(5, 5) &&
                           contestedDestination.creatures[1].position == at(4, 5) &&
                           contestedDestination.creatures[1].tile == 0x4F &&
                           contestedDestination.creatures[1].tankDirection == -2,
                           "earlier Tank claims the shared destination and later Tank stops on occupied tile");

        auto clearedBombDestinationLevel = blankLevel();
        clearedBombDestinationLevel.top[at(1, 1)] = 0x6F;
        clearedBombDestinationLevel.top[at(5, 4)] = 0x4E; // Earlier Tank enters the bomb from the north.
        clearedBombDestinationLevel.top[at(4, 5)] = 0x4F; // Later Tank approaches the same bomb from the west.
        clearedBombDestinationLevel.top[at(5, 5)] = 0x2A;
        auto clearedBombDestination = startWithMapMonsters(clearedBombDestinationLevel);
        clearedBombDestination.gameTicks = 1;
        clearedBombDestination.advanceTicks(1);
        failures += !check(clearedBombDestination.creatures.size() == 2 &&
                           !clearedBombDestination.creatures[0].alive &&
                           clearedBombDestination.creatures[0].position == at(5, 5) &&
                           clearedBombDestination.creatures[1].alive &&
                           clearedBombDestination.creatures[1].position == at(5, 5) &&
                           clearedBombDestination.top[at(5, 5)] == 0x00,
                           "later Tank enters the Floor cleared by an earlier creature's bomb death in the same pass");

        auto coveredButtonLevel = blankLevel();
        coveredButtonLevel.top[at(1, 1)] = 0x6F;
        coveredButtonLevel.top[at(2, 1)] = 0x2B;
        coveredButtonLevel.bottom[at(2, 1)] = 0x2B;
        coveredButtonLevel.top[at(5, 5)] = 0x49; // Ball sprite covers the linked button.
        coveredButtonLevel.bottom[at(5, 5)] = 0x27;
        OptionalField coveredButtonLink;
        coveredButtonLink.id = 4;
        coveredButtonLink.raw = {5, 0, 5, 0, 2, 0, 1, 0, 0, 0};
        coveredButtonLevel.fields.push_back(coveredButtonLink);
        auto coveredButton = startWithMapMonsters(coveredButtonLevel);
        (void)coveredButton.move(Direction::east);
        failures += !check(coveredButton.playerPosition == at(2, 1) && !coveredButton.traps[0].closed,
                           "EnterTrap does not treat a button under a creature as a live upper-layer button");

        auto closedLevel = blankLevel();
        closedLevel.top[at(2, 1)] = 0x6F;
        closedLevel.bottom[at(2, 1)] = 0x2B;
        OptionalField closedLink;
        closedLink.id = 4;
        closedLink.raw = {8, 0, 8, 0, 2, 0, 1, 0, 1, 0};
        closedLevel.fields.push_back(closedLink);
        auto closedGame = startWithMapMonsters(closedLevel);
        (void)closedGame.move(Direction::east);
        failures += !check(closedGame.playerPosition == at(2, 1), "closed linked trap blocks leaving");

        auto unlinkedLevel = blankLevel();
        unlinkedLevel.top[at(2, 1)] = 0x6F;
        unlinkedLevel.bottom[at(2, 1)] = 0x2B;
        auto unlinkedGame = startWithMapMonsters(unlinkedLevel);
        failures += !check(unlinkedGame.move(Direction::east) == MoveResult::blocked &&
                           unlinkedGame.playerPosition == at(2, 1),
                           "unlinked trap blocks Chip from leaving");

        auto playerLayerLevel = blankLevel();
        playerLayerLevel.top[at(2, 2)] = 0x6F;
        playerLayerLevel.bottom[at(2, 2)] = 0x2B;
        playerLayerLevel.top[at(5, 5)] = 0x27;
        OptionalField playerLayerLink;
        playerLayerLink.id = 4;
        playerLayerLink.raw = {5, 0, 5, 0, 2, 0, 2, 0, 0, 0};
        playerLayerLevel.fields.push_back(playerLayerLink);
        auto playerLayers = startWithMapMonsters(playerLayerLevel);
        (void)playerLayers.move(Direction::east);
        failures += !check(playerLayers.top[at(2, 2)] == 0x2B &&
                           playerLayers.bottom[at(2, 2)] == 0x00,
                           "Chip leaves the visible trap in Upper and resets its vacated Lower tile to floor");
        (void)playerLayers.move(Direction::west);
        failures += !check(playerLayers.playerPosition == at(2, 2) &&
                           playerLayers.bottom[at(2, 2)] == 0x2B,
                           "Chip restores the tile underfoot in Lower when re-entering a trap");

        auto portalLayerLevel = blankLevel();
        portalLayerLevel.top[at(1, 1)] = 0x6F;
        portalLayerLevel.top[at(2, 1)] = 0x29;
        portalLayerLevel.top[at(7, 1)] = 0x29;
        portalLayerLevel.bottom[at(7, 1)] = 0x2B;
        portalLayerLevel.top[at(5, 5)] = 0x27;
        OptionalField portalTrap;
        portalTrap.id = 4;
        portalTrap.raw = {5, 0, 5, 0, 7, 0, 1, 0, 0, 0};
        portalLayerLevel.fields.push_back(portalTrap);
        auto portalLayers = startWithMapMonsters(portalLayerLevel);
        (void)portalLayers.move(Direction::east);
        failures += !check(portalLayers.playerPosition == at(7, 1) &&
                           portalLayers.bottom[at(7, 1)] == 0x29 && !portalLayers.traps[0].closed,
                           "teleport arrival replaces the destination lower tile before trap-link handling");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x27;
        level.top[at(5, 1)] = 0x27;
        level.top[at(4, 1)] = 0x2B;
        level.bottom[at(4, 1)] = 0x2B;
        OptionalField links;
        links.id = 4;
        links.raw = {
            2, 0, 1, 0, 4, 0, 1, 0, 1, 0,
            5, 0, 1, 0, 4, 0, 1, 0, 1, 0}; // two buttons share one trap destination
        level.fields.push_back(links);
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east); // open both records through the first button
        failures += !check(game.traps.size() == 2 && !game.traps[0].closed && !game.traps[1].closed,
                           "PressTrapButton opens the full contiguous group sharing its trap destination");
        (void)game.move(Direction::east);
        (void)game.move(Direction::east); // entering the trap closes the destination group
        failures += !check(game.traps.size() == 2 && game.traps[0].closed && game.traps[1].closed,
                           "trap entry updates every link sharing its destination");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x27;
        level.top[at(4, 1)] = 0x2B;
        level.bottom[at(4, 1)] = 0x2B;
        level.top[at(2, 3)] = 0x27;
        level.top[at(4, 3)] = 0x2B;
        level.bottom[at(4, 3)] = 0x2B;
        OptionalField links;
        links.id = 4;
        links.raw = {
            2, 0, 1, 0, 4, 0, 1, 0, 1, 0,
            2, 0, 3, 0, 4, 0, 3, 0, 1, 0};
        level.fields.push_back(links);
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.traps.size() == 2 && !game.traps[0].closed && game.traps[1].closed,
                           "PressTrapButton leaves a different trap-destination group closed");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x0A;
        level.top[at(3, 1)] = 0x2B;
        level.bottom[at(3, 1)] = 0x2B;
        level.top[at(5, 5)] = 0x27;
        OptionalField link;
        link.id = 4;
        link.raw = {5, 0, 5, 0, 3, 0, 1, 0, 1, 0};
        level.fields.push_back(link);
        auto closed = startWithMapMonsters(level);
        failures += !check(closed.move(Direction::east) == MoveResult::moved &&
                           closed.top[at(3, 1)] == 0x0A && closed.traps[0].closed,
                           "block can enter a closed linked trap and is held inside it");

        link.raw[8] = 0;
        level.fields[0] = link;
        auto open = startWithMapMonsters(level);
        (void)open.move(Direction::east);
        failures += !check(open.top[at(3, 1)] == 0x0A && open.traps[0].closed,
                           "block may enter an open linked trap and its linked button closes it");

        auto pushIntoOpenTrap = blankLevel();
        pushIntoOpenTrap.top[at(5, 6)] = 0x6C; // Chip faces north
        pushIntoOpenTrap.top[at(5, 5)] = 0x0A;
        pushIntoOpenTrap.top[at(5, 4)] = 0x2B;
        pushIntoOpenTrap.bottom[at(5, 4)] = 0x2B;
        OptionalField openTrapLink;
        openTrapLink.id = 4;
        openTrapLink.raw = {8, 0, 8, 0, 5, 0, 4, 0, 0, 0};
        pushIntoOpenTrap.fields.push_back(openTrapLink);
        auto redirectedSlider = startWithMapMonsters(pushIntoOpenTrap);
        redirectedSlider.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        redirectedSlider.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        failures += !check(redirectedSlider.move(Direction::north) == MoveResult::moved &&
                           redirectedSlider.playerPosition == at(5, 5) &&
                           redirectedSlider.blockDirections[at(5, 4)] ==
                               static_cast<std::int8_t>(Direction::north) &&
                           redirectedSlider.slipOrder.size() == 1 &&
                           redirectedSlider.slipOrder[0].entity == at(5, 4),
                           "pushing a sliding block into a trap preserves its slip entry and push direction");
        redirectedSlider.advanceTicks(1);
        failures += !check(redirectedSlider.top[at(5, 3)] == 0x0A &&
                           redirectedSlider.slipOrder.empty(),
                           "sliding block continues out of an open trap along the push direction");

        auto stuckLevel = blankLevel();
        stuckLevel.top[at(1, 1)] = 0x6F;
        stuckLevel.top[at(2, 1)] = 0x0A;
        stuckLevel.bottom[at(2, 1)] = 0x2B;
        OptionalField closedSourceTrap;
        closedSourceTrap.id = 4;
        closedSourceTrap.raw = {5, 0, 5, 0, 2, 0, 1, 0, 1, 0};
        stuckLevel.fields.push_back(closedSourceTrap);
        auto stuck = startWithMapMonsters(stuckLevel);
        failures += !check(stuck.move(Direction::east) == MoveResult::blocked &&
                           stuck.playerPosition == at(1, 1) && stuck.top[at(2, 1)] == 0x0A,
                           "block cannot be pushed out of a closed linked trap");

        auto unlinkedBlockLevel = blankLevel();
        unlinkedBlockLevel.top[at(1, 1)] = 0x6F;
        unlinkedBlockLevel.top[at(5, 5)] = 0x0A;
        unlinkedBlockLevel.bottom[at(5, 5)] = 0x2B;
        auto unlinkedBlock = startWithMapMonsters(unlinkedBlockLevel);
        unlinkedBlock.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        unlinkedBlock.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        unlinkedBlock.advanceTicks(1);
        failures += !check(unlinkedBlock.top[at(5, 5)] == 0x0A &&
                           unlinkedBlock.slipOrder.size() == 1,
                           "sliding block remains in an unlinked trap");

        auto coveredSourceButtonLevel = blankLevel();
        coveredSourceButtonLevel.top[at(1, 3)] = 0x6F;
        coveredSourceButtonLevel.top[at(2, 1)] = 0x0A;
        coveredSourceButtonLevel.bottom[at(2, 1)] = 0x27; // Block covers the link's button.
        coveredSourceButtonLevel.top[at(3, 1)] = 0x2B;
        coveredSourceButtonLevel.bottom[at(3, 1)] = 0x2B;
        OptionalField coveredSourceLink;
        coveredSourceLink.id = 4;
        coveredSourceLink.raw = {2, 0, 1, 0, 3, 0, 1, 0, 0, 0}; // Initially open.
        coveredSourceButtonLevel.fields.push_back(coveredSourceLink);
        auto coveredSourceButton = startWithMapMonsters(coveredSourceButtonLevel);
        failures += !check(!coveredSourceButton.traps[0].closed,
                           "covered-source-button trap fixture starts open");
        coveredSourceButton.blockDirections[at(2, 1)] = static_cast<std::int8_t>(Direction::east);
        coveredSourceButton.slipOrder.push_back({SlipperKind::block, at(2, 1)});
        coveredSourceButton.advanceTicks(1);
        failures += !check(coveredSourceButton.top[at(2, 1)] == 0x27 &&
                           coveredSourceButton.bottom[at(2, 1)] == 0x00 &&
                           coveredSourceButton.top[at(3, 1)] == 0x0A &&
                           coveredSourceButton.bottom[at(3, 1)] == 0x2B &&
                           coveredSourceButton.traps[0].closed &&
                           coveredSourceButton.slipOrder.size() == 1 &&
                           coveredSourceButton.slipOrder[0].entity == at(3, 1),
                           "sliding block enters its linked trap, closes it while covering its source button, and keeps its slipper");

        auto slidingLevel = blankLevel();
        slidingLevel.top[at(4, 5)] = 0x6F;
        slidingLevel.top[at(5, 5)] = 0x27;
        slidingLevel.top[at(6, 6)] = 0x0A;
        slidingLevel.bottom[at(6, 6)] = 0x2B;
        auto closedSlidingTrap = closedSourceTrap;
        closedSlidingTrap.raw[4] = 6;
        closedSlidingTrap.raw[6] = 6;
        slidingLevel.fields.push_back(closedSlidingTrap);
        auto trappedSlider = startWithMapMonsters(slidingLevel);
        trappedSlider.blockDirections[at(6, 6)] = static_cast<std::int8_t>(Direction::east);
        trappedSlider.slipOrder.push_back({SlipperKind::block, at(6, 6)});
        trappedSlider.advanceTicks(1);
        failures += !check(trappedSlider.top[at(6, 6)] == 0x0A &&
                           trappedSlider.slipOrder.size() == 1 &&
                           trappedSlider.blockDirections[at(6, 6)] ==
                               static_cast<std::int8_t>(Direction::east),
                           "sliding block retains direction while trapped in a closed trap");
        failures += !check(trappedSlider.move(Direction::east) == MoveResult::moved &&
                           !trappedSlider.traps[0].closed,
                           "linked button can open a trap holding a sliding block");
        trappedSlider.advanceTicks(1);
        failures += !check(trappedSlider.top[at(7, 6)] == 0x0A && trappedSlider.slipOrder.empty(),
                           "sliding block resumes once its trap opens");

        auto queuedTrapBlocksLevel = blankLevel();
        queuedTrapBlocksLevel.top[at(1, 1)] = 0x6F;
        queuedTrapBlocksLevel.top[at(2, 1)] = 0x27;
        queuedTrapBlocksLevel.top[at(5, 5)] = 0x0A;
        queuedTrapBlocksLevel.bottom[at(5, 5)] = 0x2B;
        queuedTrapBlocksLevel.top[at(10, 10)] = 0x0A;
        queuedTrapBlocksLevel.bottom[at(10, 10)] = 0x0C;
        queuedTrapBlocksLevel.top[at(11, 10)] = 0x00;
        OptionalField queuedTrapLink;
        queuedTrapLink.id = 4;
        queuedTrapLink.raw = {2, 0, 1, 0, 5, 0, 5, 0, 1, 0};
        queuedTrapBlocksLevel.fields.push_back(queuedTrapLink);
        auto queuedTrapBlocks = startWithMapMonsters(queuedTrapBlocksLevel);
        queuedTrapBlocks.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        queuedTrapBlocks.blockDirections[at(10, 10)] = static_cast<std::int8_t>(Direction::east);
        queuedTrapBlocks.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        queuedTrapBlocks.slipOrder.push_back({SlipperKind::block, at(10, 10)});
        queuedTrapBlocks.advanceTicks(1);
        failures += !check(queuedTrapBlocks.traps[0].closed &&
                           queuedTrapBlocks.top[at(10, 10)] == 0x0A &&
                           queuedTrapBlocks.slipOrder.size() == 2 &&
                           queuedTrapBlocks.slipOrder[0].entity == at(10, 10) &&
                           queuedTrapBlocks.slipOrder[1].entity == at(5, 5),
                           "a block held by a closed trap is requeued behind the next block without moving it early");
        queuedTrapBlocks.advanceTicks(1);
        failures += !check(queuedTrapBlocks.top[at(11, 10)] == 0x0A &&
                           queuedTrapBlocks.slipOrder.size() == 1 &&
                           queuedTrapBlocks.slipOrder[0].entity == at(5, 5),
                           "the block queued behind a trapped block moves on the following slip tick");

        auto creatureTrapLevel = blankLevel();
        creatureTrapLevel.top[at(1, 1)] = 0x6F;
        creatureTrapLevel.top[at(2, 2)] = 0x43; // bug heading east
        creatureTrapLevel.bottom[at(2, 2)] = 0x0C;
        creatureTrapLevel.top[at(3, 2)] = 0x2B;
        creatureTrapLevel.bottom[at(3, 2)] = 0x2B;
        creatureTrapLevel.top[at(6, 6)] = 0x27;
        OptionalField creatureTrapLink;
        creatureTrapLink.id = 4;
        creatureTrapLink.raw = {6, 0, 6, 0, 3, 0, 2, 0, 1, 0};
        creatureTrapLevel.fields.push_back(creatureTrapLink);
        auto trappedCreature = startWithMapMonsters(creatureTrapLevel);
        trappedCreature.slipOrder.push_back({SlipperKind::creature, 0});
        trappedCreature.advanceTicks(1);
        failures += !check(trappedCreature.creatures[0].position == at(3, 2) &&
                           trappedCreature.traps[0].closed && trappedCreature.slipOrder.empty(),
                           "slipping creature loses its slip entry when entering a closed trap");
        trappedCreature.advanceTicks(2);
        failures += !check(trappedCreature.creatures[0].position == at(3, 2) &&
                           trappedCreature.slipOrder.empty(),
                           "normal creature turn holds a creature in its closed trap");

        auto unlinkedCreatureLevel = blankLevel();
        unlinkedCreatureLevel.top[at(1, 1)] = 0x6F;
        unlinkedCreatureLevel.top[at(5, 5)] = 0x43;
        unlinkedCreatureLevel.bottom[at(5, 5)] = 0x2B;
        auto unlinkedCreature = startWithMapMonsters(unlinkedCreatureLevel);
        unlinkedCreature.slipOrder.push_back({SlipperKind::creature, 0});
        unlinkedCreature.advanceTicks(1);
        failures += !check(unlinkedCreature.creatures[0].position == at(5, 5) &&
                           unlinkedCreature.creatures[0].tile == 0x43 &&
                           unlinkedCreature.slipOrder.size() == 1,
                           "slipping creature on an unlinked trap retains its heading and slip entry");

        auto underlayTrapLevel = blankLevel();
        underlayTrapLevel.top[at(5, 2)] = 0x6F;
        underlayTrapLevel.top[at(2, 2)] = 0x43; // bug heading east
        underlayTrapLevel.top[at(2, 1)] = 0x00; // ordinary visible floor
        underlayTrapLevel.bottom[at(2, 1)] = 0x2B;
        underlayTrapLevel.top[at(6, 6)] = 0x43; // covers the linked trap button
        underlayTrapLevel.bottom[at(6, 6)] = 0x27;
        OptionalField underlayTrapLink;
        underlayTrapLink.id = 4;
        underlayTrapLink.raw = {6, 0, 6, 0, 2, 0, 1, 0, 1, 0};
        underlayTrapLevel.fields.push_back(underlayTrapLink);
        auto underlayTrap = startWithMapMonsters(underlayTrapLevel);
        underlayTrap.advanceTicks(2);
        failures += !check(underlayTrap.creatures[0].position == at(2, 1) &&
                           underlayTrap.traps[0].closed && underlayTrap.bottom[at(2, 1)] == 0x00,
                           "creature crossing ordinary upper terrain over a trap does not trigger Trap action");

        auto closedTrapSlipLevel = blankLevel();
        closedTrapSlipLevel.top[at(1, 1)] = 0x6F;
        closedTrapSlipLevel.top[at(2, 1)] = 0x27;
        closedTrapSlipLevel.top[at(6, 6)] = 0x43; // east-facing bug over a closed trap
        closedTrapSlipLevel.bottom[at(6, 6)] = 0x2B;
        OptionalField closedTrapSlipLink;
        closedTrapSlipLink.id = 4;
        closedTrapSlipLink.raw = {2, 0, 1, 0, 6, 0, 6, 0, 1, 0};
        closedTrapSlipLevel.fields.push_back(closedTrapSlipLink);
        auto closedTrapSlip = startWithMapMonsters(closedTrapSlipLevel);
        closedTrapSlip.slipOrder.push_back({SlipperKind::creature, 0});
        closedTrapSlip.advanceTicks(1);
        failures += !check(closedTrapSlip.creatures[0].position == at(6, 6) &&
                           closedTrapSlip.creatures[0].tile == 0x43 &&
                           closedTrapSlip.slipOrder.size() == 1,
                           "a creature blocked inside a closed trap retains its slip heading");
        (void)closedTrapSlip.move(Direction::east);
        closedTrapSlip.advanceTicks(1);
        failures += !check(closedTrapSlip.creatures[0].position == at(7, 6) &&
                           closedTrapSlip.slipOrder.empty(),
                           "a trapped creature resumes along its stored heading when the trap opens");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x24;
        level.top[at(5, 5)] = 0x40;
        level.bottom[at(5, 5)] = 0x31;
        OptionalField links;
        links.id = 5;
        links.raw = {2, 0, 1, 0, 5, 0, 5, 0}; // button (2,1) -> clone machine (5,5)
        level.fields.push_back(links);
        auto game = startWithMapMonsters(level);
        failures += !check(game.cloneLinks.size() == 1 && game.creatures.empty(),
                           "clone links load while the machine prototype stays out of the live creature list");
        (void)game.move(Direction::east);
        failures += !check(game.creatures.size() == 1 && game.creatures[0].position == at(5, 5) &&
                           game.creatures[0].tile == 0x40,
                           "clone button recreates its linked creature on the machine");

        auto rotatedLevel = blankLevel();
        rotatedLevel.top[at(1, 1)] = 0x6F;
        rotatedLevel.top[at(2, 1)] = 0x24;
        rotatedLevel.top[at(3, 1)] = 0x28;
        rotatedLevel.top[at(4, 1)] = 0x24;
        rotatedLevel.top[at(5, 5)] = 0x4F; // east-facing tank clone template
        rotatedLevel.bottom[at(5, 5)] = 0x31;
        OptionalField rotatedLinks;
        rotatedLinks.id = 5;
        rotatedLinks.raw = {2, 0, 1, 0, 5, 0, 5, 0,
                            4, 0, 1, 0, 5, 0, 5, 0};
        rotatedLevel.fields.push_back(rotatedLinks);
        auto rotatedClone = startWithMapMonsters(rotatedLevel);
        (void)rotatedClone.move(Direction::east); // create the first tank at the machine
        (void)rotatedClone.move(Direction::east); // rotate it with the TankButton
        failures += !check(rotatedClone.creatures.size() == 1 &&
                           rotatedClone.creatures[0].tile == 0x4D &&
                           rotatedClone.top[at(5, 5)] == 0x4D,
                           "TankButton updates the live tank-clone template on its machine");
        rotatedClone.gameTicks = 1;
        rotatedClone.advanceTicks(1); // tank leaves west while the machine retains its template
        failures += !check(rotatedClone.creatures[0].position == at(4, 5) &&
                           rotatedClone.top[at(5, 5)] == 0x4D,
                           "a tank leaving its clone machine preserves the rotated template");
        (void)rotatedClone.move(Direction::east); // create the next tank from live machine data
        failures += !check(rotatedClone.creatures.size() == 2 &&
                           rotatedClone.creatures[1].position == at(5, 5) &&
                           rotatedClone.creatures[1].tile == 0x4D,
                           "later clone activation uses the rotated machine tile");

        auto blockedBlobLevel = blankLevel();
        blockedBlobLevel.top[at(1, 1)] = 0x6F;
        blockedBlobLevel.top[at(2, 1)] = 0x24;
        blockedBlobLevel.top[at(5, 5)] = 0x5C; // north-facing Blob clone template
        blockedBlobLevel.bottom[at(5, 5)] = 0x31;
        OptionalField blobLink;
        blobLink.id = 5;
        blobLink.raw = {2, 0, 1, 0, 5, 0, 5, 0};
        blockedBlobLevel.fields.push_back(blobLink);
        auto blockedBlob = startWithMapMonsters(blockedBlobLevel);
        (void)blockedBlob.move(Direction::east); // spawn the Blob on its machine
        for (const auto position : {at(5, 4), at(6, 5), at(5, 6), at(4, 5)})
            blockedBlob.top[static_cast<std::size_t>(position)] = 0x01;
        const auto blobRandomBeforeBlockedTurn = blockedBlob.randomState;
        blockedBlob.gameTicks = 3;
        blockedBlob.advanceTicks(1); // even MonsterLoop; every random move is blocked
        failures += !check(blockedBlob.creatures.size() == 1 && blockedBlob.creatures[0].alive &&
                           blockedBlob.creatures[0].position == at(5, 5) &&
                           blockedBlob.top[at(5, 5)] == blockedBlob.creatures[0].tile &&
                           blockedBlob.top[at(5, 5)] == 0x5D &&
                           blockedBlob.randomState != blobRandomBeforeBlockedTurn &&
                           blockedBlob.randomState == 3750785579U,
                           "blocked Blob exhausts random fallback turns on a clone machine and updates its template");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(4, 5)] = 0x4F; // east-facing tank presses the button
        level.top[at(5, 5)] = 0x24;
        level.top[at(10, 10)] = 0x43; // clone machine's east-facing prototype
        level.bottom[at(10, 10)] = 0x31;
        OptionalField clone;
        clone.id = 5;
        clone.raw = {5, 0, 5, 0, 10, 0, 10, 0};
        level.fields.push_back(clone);
        auto game = startWithMapMonsters(level);
        game.gameTicks = 1;
        failures += !check(game.creatures.size() == 1 && game.surface(at(10, 10)) == 0x43,
                           "clone-machine prototype remains visible but is excluded from live creatures");
        game.advanceTicks(1);
        failures += !check(game.creatures.size() == 2 && game.creatures.size() > 1 &&
                           game.creatures[1].position == at(10, 10),
                           "creature-triggered clone waits at its machine until the next game tick");
        game.advanceTicks(2);
        failures += !check(game.creatures.size() > 1 && game.creatures[1].position == at(11, 10) &&
                           game.surface(at(10, 10)) == 0x43,
                           "clone moves on the following pass and leaves its machine prototype visible");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x4B; // Ball moves east onto CloneButton.
        level.top[at(6, 5)] = 0x24;
        constexpr int machine = 10 * kBoardWidth + 10;
        level.top[static_cast<std::size_t>(machine)] = 0x40; // Clone emits a north Bug.
        level.bottom[static_cast<std::size_t>(machine)] = 0x31;
        constexpr int trappedTeeth = 15 * kBoardWidth + 15;
        level.top[static_cast<std::size_t>(trappedTeeth)] = 0x54; // Prototype faces north.
        level.bottom[static_cast<std::size_t>(trappedTeeth)] = 0x2B;
        OptionalField clone;
        clone.id = 5;
        clone.raw = {6, 0, 5, 0, 10, 0, 10, 0};
        level.fields.push_back(clone);
        OptionalField openTrap;
        openTrap.id = 4;
        openTrap.raw = {20, 0, 20, 0, 15, 0, 15, 0, 0, 0};
        level.fields.push_back(openTrap);
        auto game = startWithMapMonsters(level);
        game.gameTicks = 3;
        game.advanceTicks(1);
        failures += !check(game.creatures.size() == 3 &&
                           game.creatures[0].position == at(6, 5) &&
                           game.creatures[1].position == trappedTeeth + 1 &&
                           game.creatures[2].position == machine,
                           "CloneButton growth preserves the current MonsterLoop direction and indices");
    }
    {
        const auto makeCloneLevel = [](std::uint8_t prototype, std::uint8_t outputTile) {
            auto level = blankLevel();
            level.top[at(1, 1)] = 0x6F;
            level.top[at(2, 1)] = 0x24;
            level.top[at(5, 5)] = prototype;
            level.bottom[at(5, 5)] = 0x31;
            level.top[at(6, 5)] = outputTile;
            OptionalField link;
            link.id = 5;
            link.raw = {2, 0, 1, 0, 5, 0, 5, 0};
            level.fields.push_back(link);
            return level;
        };
        auto allowedFireLevel = makeCloneLevel(0x4B, 0x04); // east-facing ball; burns after it emerges
        auto allowedFire = startWithMapMonsters(allowedFireLevel);
        (void)allowedFire.move(Direction::east);
        failures += !check(allowedFire.creatures.size() == 1 &&
                           allowedFire.creatures[0].position == at(5, 5),
                           "clone machine permits a creature whose next move enters fire");

        auto blockedFireLevel = makeCloneLevel(0x43, 0x04); // east-facing bug; fire blocks entry
        auto blockedFire = startWithMapMonsters(blockedFireLevel);
        (void)blockedFire.move(Direction::east);
        failures += !check(blockedFire.creatures.empty(),
                           "clone machine does not emit a bug into a blocked fire entry");

        auto waterLevel = makeCloneLevel(0x43, 0x03); // MonsterCanEnterTile accepts water, then bug drowns
        auto waterClone = startWithMapMonsters(waterLevel);
        (void)waterClone.move(Direction::east);
        failures += !check(waterClone.creatures.size() == 1 && waterClone.creatures[0].alive,
                           "clone machine emits a bug when its output enters water");
        waterClone.advanceTicks(2);
        failures += !check(!waterClone.creatures[0].alive,
                           "cloned bug drowns after entering water");

        auto panelLevel = makeCloneLevel(0x4B, 0x06); // eastward entry through PanelN is allowed
        auto panel = startWithMapMonsters(panelLevel);
        (void)panel.move(Direction::east);
        failures += !check(panel.creatures.size() == 1,
                           "clone machine uses directional panel entry rules");

        auto queuedCloneLevel = makeCloneLevel(0x43, 0x00); // east-facing bug template
        queuedCloneLevel.top[at(6, 5)] = 0x43; // identical clone blocks the output square
        auto queuedClone = startWithMapMonsters(queuedCloneLevel);
        (void)queuedClone.move(Direction::east);
        failures += !check(queuedClone.creatures.size() == 2 &&
                           queuedClone.creatures[0].position == at(6, 5) &&
                           queuedClone.creatures[1].position == at(5, 5),
                           "clone machine emits behind an identical clone blocking its output");
    }
    {
        // Exhaustively compare clone-output acceptance with MonsterCanEnterTile
        // in the supplied disassembly. Keeping field 10 empty prevents test
        // target sprites from being initialized as live monsters.
        const auto sourceAllowsEntry = [](std::uint8_t creature, std::uint8_t tile,
                                          int direction) {
            // MonsterCanEnterTile reads the Lower layer for swimming-chip
            // sprites; this matrix gives those cells a floor underlay.
            if (tile >= 0x3C && tile <= 0x3F) tile = 0x00;
            // CheckPanelWalls(..., entering=1), where creature direction is
            // encoded N, W, S, E as 0..3 in the original tile ID.
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                case 0x03: return true; // accepted; non-gliders drown on their move
                case 0x04: return creature != 0x40 && creature != 0x58;
                default:
                    return tile == 0x00 || tile == 0x0C || tile == 0x0D ||
                        (tile >= 0x12 && tile <= 0x14) || tile == 0x23 ||
                        tile == 0x24 || tile == 0x26 ||
                        (tile >= 0x27 && tile <= 0x2B) || tile == 0x2F ||
                        (tile >= 0x3C && tile <= 0x3F) ||
                        (tile >= 0x64 && tile <= 0x67) ||
                        (tile >= 0x6C && tile <= 0x6F);
            }
        };
        constexpr std::array<std::uint8_t, 9> creatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60};
        int checkedCases = 0;
        for (const auto kind : creatureKinds) {
            for (int direction = 0; direction < 4; ++direction) {
                const auto prototype = static_cast<std::uint8_t>(kind + direction);
                // 0x6C..0x6F are player-start markers and cannot coexist with
                // the test player's start in a valid level.
                for (int target = 0; target <= 0x6B; ++target) {
                    auto level = blankLevel();
                    level.top[at(1, 1)] = 0x6F;
                    level.top[at(2, 1)] = 0x24;
                    level.top[at(10, 10)] = prototype;
                    level.bottom[at(10, 10)] = 0x31;
                    const int output = neighborForMonsterDirection(at(10, 10), direction);
                    level.top[static_cast<std::size_t>(output)] =
                        static_cast<std::uint8_t>(target);
                    OptionalField link;
                    link.id = 5;
                    link.raw = {2, 0, 1, 0, 10, 0, 10, 0};
                    level.fields.push_back(link);
                    OptionalField noMapMonsters;
                    noMapMonsters.id = 10;
                    level.fields.push_back(noMapMonsters);

                    auto game = GameState::start(level);
                    (void)game.move(Direction::east);
                    const bool expected = sourceAllowsEntry(kind,
                        static_cast<std::uint8_t>(target), direction) ||
                        target == prototype; // PressCloneButton's identical-clone queue exception
                    if (game.creatures.size() != (expected ? 1U : 0U)) {
                        failures += !check(false,
                            "clone output matches source MonsterCanEnterTile for creature " +
                            std::to_string(kind) + ", direction " + std::to_string(direction) +
                            ", tile " + std::to_string(target));
                    }
                    ++checkedCases;
                }
            }
        }
        failures += !check(checkedCases == 9 * 4 * 0x6C,
                           "clone-output terrain matrix covers all creature families and non-start tile IDs");
    }
    {
        // Source-derived BlockCanEnterTile admission oracle from TileTable's
        // block column (byte 2 of each six-byte row). Exercise the actual
        // CloneButton -> MoveBlock route, including every authored tile ID and
        // all four block-template directions. The only special admissions in
        // this column are panels/ice walls, which additionally use
        // CheckPanelWalls for the attempted direction.
        constexpr std::array<std::uint8_t, 0x70> sourceBlockAdmission{
            1,0,0,1,1,0,2,2,2,2,0,0,1,1,0,0,
            0,0,1,1,1,1,0,0,0,0,2,2,2,2,0,0,
            0,0,0,1,1,0,1,1,1,1,1,1,0,1,0,1,
            2,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1
        };
        const auto sourcePanelEntry = [](std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                default: return false;
            }
        };
        int checkedCases = 0;
        for (int direction = 0; direction < 4; ++direction) {
            for (unsigned tile = 0; tile < sourceBlockAdmission.size(); ++tile) {
                auto level = blankLevel();
                level.top[at(1, 1)] = 0x6F;
                level.top[at(2, 1)] = 0x24;
                const int machine = at(10, 10);
                const int output = neighborForMonsterDirection(machine, direction);
                level.top[static_cast<std::size_t>(machine)] =
                    static_cast<std::uint8_t>(0x0E + direction);
                level.bottom[static_cast<std::size_t>(machine)] = 0x31;
                if (tile < 0x6C)
                    level.top[static_cast<std::size_t>(output)] = static_cast<std::uint8_t>(tile);
                OptionalField cloneLink;
                cloneLink.id = 5;
                cloneLink.raw = {2, 0, 1, 0, 10, 0, 10, 0};
                level.fields.push_back(cloneLink);
                auto game = startWithMapMonsters(level);
                // Player-start IDs are normally consumed by initialization;
                // install them afterward so this case checks BlockCanEnterTile.
                if (tile >= 0x6C)
                    game.top[static_cast<std::size_t>(output)] = static_cast<std::uint8_t>(tile);
                (void)game.move(Direction::east);

                const bool expected = sourceBlockAdmission[tile] == 1 ||
                    (sourceBlockAdmission[tile] == 2 && sourcePanelEntry(
                        static_cast<std::uint8_t>(tile), direction));
                const bool emitted = game.message == "Clone block created." ||
                    game.message == "The clone block filled the water with dirt." ||
                    game.message == "The clone block covers the fire.";
                failures += !check(emitted == expected,
                    "clone-block output matches source BlockCanEnterTile for direction " +
                    std::to_string(direction) + ", tile " + std::to_string(tile));
                ++checkedCases;
            }
        }
        failures += !check(checkedCases == 4 * 0x70,
                           "clone-block admission matrix covers all four directions and tile IDs 0x00-0x6F");
    }
    {
        // SlipLoop uses the same source BlockCanEnterTile table as a pushed
        // block. Test all destination IDs and travel headings through the
        // actual ordered slipper path, with player/creature IDs installed
        // after board initialization so they remain ordinary visible targets.
        constexpr std::array<std::uint8_t, 0x70> sourceBlockAdmission{
            1,0,0,1,1,0,2,2,2,2,0,0,1,1,0,0,
            0,0,1,1,1,1,0,0,0,0,2,2,2,2,0,0,
            0,0,0,1,1,0,1,1,1,1,1,1,0,1,0,1,
            2,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1
        };
        const auto sourcePanelEntry = [](std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                default: return false;
            }
        };
        int checkedCases = 0;
        for (int direction = 0; direction < 4; ++direction) {
            for (unsigned tile = 0; tile < sourceBlockAdmission.size(); ++tile) {
                auto level = blankLevel();
                const int source = at(10, 10);
                const int blockDirection = direction == 1 ? 3 : direction == 3 ? 1 : direction;
                const int destination = neighborForMonsterDirection(source, blockDirection);
                level.top[at(1, 1)] = 0x6F;
                level.top[static_cast<std::size_t>(source)] = 0x0A;
                OptionalField noMapMonsters;
                noMapMonsters.id = 10;
                level.fields.push_back(noMapMonsters);
                auto game = GameState::start(level);
                game.top[static_cast<std::size_t>(source)] = 0x0A;
                game.bottom[static_cast<std::size_t>(source)] = 0x00;
                game.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);
                game.bottom[static_cast<std::size_t>(destination)] = 0x00;
                game.blockDirections[static_cast<std::size_t>(source)] =
                    static_cast<std::int8_t>(direction);
                game.slipOrder.push_back({SlipperKind::block, source});
                game.advanceTicks(1);

                const bool expected = sourceBlockAdmission[tile] == 1 ||
                    (sourceBlockAdmission[tile] == 2 && sourcePanelEntry(
                        static_cast<std::uint8_t>(tile), blockDirection));
                const auto expectedUpper = tile == 0x03 ? 0x0BU
                    : tile == 0x2A ? 0x00U : 0x0AU;
                const bool moved = game.top[static_cast<std::size_t>(source)] != 0x0A;
                const bool actionMatches = game.top[static_cast<std::size_t>(destination)] == expectedUpper;
                failures += !check(moved == expected && (!expected || actionMatches),
                    "sliding block matches source BlockCanEnterTile for direction " +
                    std::to_string(direction) + ", tile " + std::to_string(tile));
                if (expected && tile == 0x03)
                    failures += !check(game.bottom[static_cast<std::size_t>(destination)] == 0x00,
                        "sliding block changes only the visible Water layer to Dirt");
                ++checkedCases;
            }
        }
        failures += !check(checkedCases == 4 * 0x70,
                           "sliding-block admission matrix covers all four headings and tile IDs 0x00-0x6F");
    }
    {
        // MoveBlock applies CheckPanelWalls with flag=0 to the block's source
        // terrain before testing its destination. Cover all five source panel
        // types and all four headings through the actual SlipLoop path.
        const auto sourcePanelExit = [](std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 0; // PanelN: cannot leave north.
                case 0x07: return direction != 3; // PanelW: cannot leave west.
                case 0x08: return direction != 2; // PanelS: cannot leave south.
                case 0x09: return direction != 1; // PanelE: cannot leave east.
                case 0x30: return direction == 0 || direction == 3; // PanelSE.
                default: return true;
            }
        };
        constexpr std::array<std::uint8_t, 5> sourcePanels{0x06, 0x07, 0x08, 0x09, 0x30};
        int checkedCases = 0;
        for (const auto panel : sourcePanels) {
            for (int direction = 0; direction < 4; ++direction) {
                auto level = blankLevel();
                const int source = at(10, 10);
                const int destination = neighborForMonsterDirection(source,
                    direction == 1 ? 3 : direction == 3 ? 1 : direction);
                level.top[at(1, 1)] = 0x6F;
                level.top[static_cast<std::size_t>(source)] = 0x0A;
                OptionalField noMapMonsters;
                noMapMonsters.id = 10;
                level.fields.push_back(noMapMonsters);
                auto game = GameState::start(level);
                game.top[static_cast<std::size_t>(source)] = 0x0A;
                game.bottom[static_cast<std::size_t>(source)] = panel;
                game.top[static_cast<std::size_t>(destination)] = 0x00;
                game.bottom[static_cast<std::size_t>(destination)] = 0x00;
                game.blockDirections[static_cast<std::size_t>(source)] =
                    static_cast<std::int8_t>(direction);
                game.slipOrder.push_back({SlipperKind::block, source});
                game.advanceTicks(1);

                const bool shouldLeave = sourcePanelExit(panel, direction);
                const bool left = game.top[static_cast<std::size_t>(source)] != 0x0A;
                failures += !check(left == shouldLeave,
                    "sliding block source-panel exit matches CheckPanelWalls for panel " +
                    std::to_string(panel) + ", direction " + std::to_string(direction));
                if (shouldLeave)
                    failures += !check(game.top[static_cast<std::size_t>(destination)] == 0x0A,
                        "sliding block enters its floor destination after a valid panel exit");
                ++checkedCases;
            }
        }
        failures += !check(checkedCases == 5 * 4,
                           "sliding-block source-panel matrix covers five panels and four headings");
    }
    {
        // Exercise the normal Chip-push path against the original block
        // admission table, separately from CloneButton and SlipLoop movement.
        constexpr std::array<std::uint8_t, 0x70> sourceBlockAdmission{
            1,0,0,1,1,0,2,2,2,2,0,0,1,1,0,0,
            0,0,1,1,1,1,0,0,0,0,2,2,2,2,0,0,
            0,0,0,1,1,0,1,1,1,1,1,1,0,1,0,1,
            2,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1
        };
        const auto sourcePanelEntry = [](std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                default: return false;
            }
        };
        int checkedCases = 0;
        for (int direction = 0; direction < 4; ++direction) {
            for (unsigned tile = 0; tile < sourceBlockAdmission.size(); ++tile) {
                auto level = blankLevel();
                const int source = at(10, 10);
                const std::array<int, 4> dx{0, 1, 0, -1};
                const std::array<int, 4> dy{-1, 0, 1, 0};
                const int blockPosition = source + dy[static_cast<std::size_t>((direction + 2) & 3)] * kBoardWidth +
                                          dx[static_cast<std::size_t>((direction + 2) & 3)];
                const int destination = source + dy[static_cast<std::size_t>(direction)] * kBoardWidth +
                                        dx[static_cast<std::size_t>(direction)];
                level.top[static_cast<std::size_t>(blockPosition)] = 0x6F;
                level.top[static_cast<std::size_t>(source)] = 0x0A;
                auto game = GameState::start(level);
                // Install all synthetic destinations after initialization so
                // player-start sprite IDs remain ordinary target tiles here.
                game.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);

                const auto moveDirection = static_cast<Direction>(direction);
                const auto result = game.move(moveDirection);
                const int sourceDirection = direction == 1 ? 3 : direction == 3 ? 1 : direction;
                const bool expected = sourceBlockAdmission[tile] == 1 ||
                    (sourceBlockAdmission[tile] == 2 && sourcePanelEntry(
                        static_cast<std::uint8_t>(tile), sourceDirection));
                failures += !check((result != MoveResult::blocked) == expected &&
                                   (game.playerPosition == source) == expected,
                    "Chip block push matches source BlockCanEnterTile for direction " +
                    std::to_string(direction) + ", tile " + std::to_string(tile));
                if (expected) {
                    const auto expectedUpper = tile == 0x03 ? 0x0BU
                        : tile == 0x2A ? 0x00U : 0x0AU;
                    failures += !check(game.top[static_cast<std::size_t>(destination)] == expectedUpper,
                        "Chip-pushed block applies the source tile action for tile " + std::to_string(tile));
                    if (tile != 0x03 && tile != 0x2A)
                        failures += !check(game.bottom[static_cast<std::size_t>(destination)] == tile,
                            "Chip-pushed block preserves the entered lower tile for tile " +
                            std::to_string(tile));
                }
                ++checkedCases;
            }
        }
        failures += !check(checkedCases == 4 * 0x70,
                           "Chip-push admission matrix covers all four headings and tile IDs 0x00-0x6F");

        int checkedLayerPairs = 0;
        bool layerMatrixMatches = true;
        std::string firstLayerMismatch;
        const std::array<int, 4> directionX{0, 1, 0, -1};
        const std::array<int, 4> directionY{-1, 0, 1, 0};
        for (int direction = 0; direction < 4; ++direction) {
            const int source = at(10, 10);
            const int reverse = (direction + 2) & 3;
            const int playerStart = source + directionY[static_cast<std::size_t>(reverse)] * kBoardWidth +
                                    directionX[static_cast<std::size_t>(reverse)];
            const int destination = source + directionY[static_cast<std::size_t>(direction)] * kBoardWidth +
                                   directionX[static_cast<std::size_t>(direction)];
            const int sourceDirection = direction == 1 ? 3 : direction == 3 ? 1 : direction;
            for (unsigned upper = 0; upper < sourceBlockAdmission.size(); ++upper) {
                const bool terrainAdmits = sourceBlockAdmission[upper] == 1 ||
                    (sourceBlockAdmission[upper] == 2 && sourcePanelEntry(
                        static_cast<std::uint8_t>(upper), sourceDirection));
                for (unsigned lower = 0; lower < 0x70; ++lower) {
                    auto level = blankLevel();
                    level.top[static_cast<std::size_t>(playerStart)] = 0x6F;
                    level.top[static_cast<std::size_t>(source)] = 0x0A;
                    auto game = GameState::start(level);
                    game.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(upper);
                    game.bottom[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(lower);
                    const auto result = game.move(static_cast<Direction>(direction));
                    const bool expected = lower != 0x31 && terrainAdmits;
                    bool matches = (result != MoveResult::blocked) == expected &&
                        (game.playerPosition == source) == expected;
                    if (expected) {
                        const auto expectedUpper = upper == 0x03 ? 0x0BU
                            : upper == 0x2A ? 0x00U : 0x0AU;
                        const auto expectedLower = upper == 0x03 || upper == 0x2A
                            ? static_cast<std::uint8_t>(lower) : static_cast<std::uint8_t>(upper);
                        matches = matches && game.top[static_cast<std::size_t>(destination)] == expectedUpper &&
                            game.bottom[static_cast<std::size_t>(destination)] == expectedLower;
                    } else {
                        matches = matches && game.top[static_cast<std::size_t>(destination)] == upper &&
                            game.bottom[static_cast<std::size_t>(destination)] == lower;
                    }
                    if (!matches && firstLayerMismatch.empty()) {
                        firstLayerMismatch = " (heading " + std::to_string(direction) + ", upper " +
                            std::to_string(upper) + ", lower " + std::to_string(lower) + ")";
                        layerMatrixMatches = false;
                    }
                    ++checkedLayerPairs;
                }
            }
        }
        failures += !check(layerMatrixMatches && checkedLayerPairs == 4 * 0x70 * 0x70,
            "Chip-push two-layer matrix matches source admission/actions for every upper/lower pair" +
            firstLayerMismatch);
    }
    {
        // SlipLoop-only source TileTable matrix. The queued slip entry makes
        // MonsterLoop skip this creature on the even tick, isolating its slide.
        // Register only the controlled source creature so sprite tile IDs do
        // not accidentally create additional map creatures in the fixture.
        const auto sourceAllowsEntry = [](std::uint8_t creature, std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                case 0x03: return true; // Water admits all; most creatures drown after entry.
                case 0x04: return creature != 0x40 && creature != 0x58;
                default:
                    return tile == 0x00 || tile == 0x0C || tile == 0x0D ||
                        (tile >= 0x12 && tile <= 0x14) || tile == 0x23 ||
                        tile == 0x24 || tile == 0x26 ||
                        (tile >= 0x27 && tile <= 0x2B) || tile == 0x2F ||
                        (tile >= 0x3C && tile <= 0x3F) ||
                        (tile >= 0x64 && tile <= 0x67) ||
                        (tile >= 0x6C && tile <= 0x6F);
            }
        };
        constexpr std::array<std::uint8_t, 9> creatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60};
        constexpr std::array<std::uint8_t, 4> sourceForceTiles{0x12, 0x14, 0x0D, 0x13};
        int checkedCases = 0;
        for (const auto kind : creatureKinds) {
            for (int direction = 0; direction < 4; ++direction) {
                for (unsigned tile = 0; tile <= 0x6F; ++tile) {
                    if (tile == 0x29) continue; // Portals have dedicated paired-exit coverage.
                    auto level = blankLevel();
                    const int source = at(10, 10);
                    level.top[at(1, 1)] = 0x6F;
                    level.top[static_cast<std::size_t>(source)] =
                        static_cast<std::uint8_t>(kind + direction);
                    level.bottom[static_cast<std::size_t>(source)] = sourceForceTiles[direction];
                    const int destination = neighborForMonsterDirection(source, direction);
                    if (tile < 0x6C)
                        level.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);
                    OptionalField monsterStarts;
                    monsterStarts.id = 10;
                    monsterStarts.raw = {10, 10};
                    level.fields.push_back(monsterStarts);
                    auto game = GameState::start(level);
                    if (tile >= 0x6C)
                        game.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);
                    game.gameTicks = 1;
                    game.slipOrder.push_back({SlipperKind::creature, 0,
                                               static_cast<std::int8_t>(direction)});
                    game.advanceTicks(1);

                    const bool expected = sourceAllowsEntry(kind, static_cast<std::uint8_t>(tile), direction);
                    const auto& creature = game.creatures[0];
                    const bool entered = creature.position != source || !creature.alive;
                    failures += !check(entered == expected,
                        "SlipLoop matches source MonsterCanEnterTile for creature " +
                        std::to_string(kind) + ", direction " + std::to_string(direction) +
                        ", tile " + std::to_string(tile));
                    if (expected && (tile == 0x03 || tile == 0x04 || tile == 0x2A)) {
                        const bool shouldSurvive = tile == 0x03 ? kind == 0x50
                            : tile == 0x04 ? kind == 0x44 : false;
                        failures += !check(creature.alive == shouldSurvive,
                            "SlipLoop resolves hazard action for creature " +
                            std::to_string(kind) + ", tile " + std::to_string(tile));
                    }
                    ++checkedCases;
                }
            }
        }
        failures += !check(checkedCases == 9 * 4 * (0x70 - 1),
                           "SlipLoop matrix covers every creature, heading, and non-portal tile ID");
    }
    {
        // Exercise layered admission and action in the creature SlipLoop.
        // This is a separate path from MonsterLoop: it starts from the shared
        // slippery-entity queue and sees the exact upper/lower destination.
        const auto sourceAllowsEntry = [](std::uint8_t creature, std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                case 0x03: return true;
                case 0x04: return creature != 0x40 && creature != 0x58;
                default:
                    return tile == 0x00 || tile == 0x0C || tile == 0x0D ||
                        (tile >= 0x12 && tile <= 0x14) || tile == 0x23 ||
                        tile == 0x24 || tile == 0x26 ||
                        (tile >= 0x27 && tile <= 0x2B) || tile == 0x2F ||
                        (tile >= 0x3C && tile <= 0x3F) ||
                        (tile >= 0x64 && tile <= 0x67) ||
                        (tile >= 0x6C && tile <= 0x6F);
            }
        };
        constexpr std::array<std::uint8_t, 9> creatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60};
        constexpr std::array<std::uint8_t, 4> sourceForceTiles{0x12, 0x14, 0x0D, 0x13};
        int checkedCases = 0;
        std::string firstMismatch;
        for (const auto kind : creatureKinds) {
            for (int direction = 0; direction < 4; ++direction) {
                for (unsigned upper = 0; upper <= 0x6F; ++upper) {
                    for (unsigned lower = 0; lower <= 0x6F; ++lower) {
                        const auto upperTile = static_cast<std::uint8_t>(upper);
                        const auto lowerTile = static_cast<std::uint8_t>(lower);
                        const bool chipOverlay = (upperTile >= 0x3C && upperTile <= 0x3F) ||
                            (upperTile >= 0x6C && upperTile <= 0x6F);
                        const auto inspectedTile = chipOverlay ? lowerTile : upperTile;
                        // Paired portal search has its own ordered-routing tests;
                        // avoid inventing a portal registry after level loading.
                        if (inspectedTile == 0x29) continue;

                        auto level = blankLevel();
                        const int source = at(10, 10);
                        const int destination = neighborForMonsterDirection(source, direction);
                        level.top[at(1, 1)] = 0x6F;
                        level.top[static_cast<std::size_t>(source)] =
                            static_cast<std::uint8_t>(kind + direction);
                        level.bottom[static_cast<std::size_t>(source)] = sourceForceTiles[direction];
                        OptionalField monsterStarts;
                        monsterStarts.id = 10;
                        monsterStarts.raw = {10, 10};
                        level.fields.push_back(monsterStarts);
                        auto game = GameState::start(level);
                        game.top[static_cast<std::size_t>(destination)] = upperTile;
                        game.bottom[static_cast<std::size_t>(destination)] = lowerTile;
                        game.gameTicks = 1; // Odd tick: only the slipping creature acts.
                        game.slipOrder.push_back({SlipperKind::creature, 0,
                            static_cast<std::int8_t>(direction)});
                        game.advanceTicks(1);

                        const bool expected = lowerTile != 0x31 &&
                            sourceAllowsEntry(kind, inspectedTile, direction);
                        const auto& creature = game.creatures[0];
                        const bool entered = creature.position != source || !creature.alive;
                        const bool matches = entered == expected &&
                            (!(expected && inspectedTile == 0x03) || creature.alive == (kind == 0x50)) &&
                            (!(expected && inspectedTile == 0x04) || creature.alive == (kind == 0x44)) &&
                            (!(expected && inspectedTile == 0x2A) || !creature.alive);
                        if (!matches && firstMismatch.empty()) {
                            firstMismatch = " (creature " + std::to_string(kind) + ", heading " +
                                std::to_string(direction) + ", upper " + std::to_string(upper) +
                                ", lower " + std::to_string(lower) + ", tile " +
                                std::to_string(inspectedTile) + ", entered " +
                                std::to_string(entered) + ", expected " + std::to_string(expected) + ")";
                        }
                        ++checkedCases;
                    }
                }
            }
        }
        failures += !check(firstMismatch.empty() && checkedCases == 9 * 4 * (0x70 * 0x70 - 120),
            "two-layer SlipLoop admission and hazard matrix matches source TileTable" + firstMismatch);
    }
    {
        // Source collision follows the same creature TileTable admission
        // when Chip's visible sprite covers the destination. Cross all
        // species, headings, and underlays through the actual SlipLoop path.
        const auto sourceAllowsEntry = [](std::uint8_t creature, std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                case 0x03: return true;
                case 0x04: return creature != 0x40 && creature != 0x58;
                default:
                    return tile == 0x00 || tile == 0x0C || tile == 0x0D ||
                        (tile >= 0x12 && tile <= 0x14) || tile == 0x23 ||
                        tile == 0x24 || tile == 0x26 ||
                        (tile >= 0x27 && tile <= 0x2B) || tile == 0x2F ||
                        (tile >= 0x3C && tile <= 0x3F) ||
                        (tile >= 0x64 && tile <= 0x67) ||
                        (tile >= 0x6C && tile <= 0x6F);
            }
        };
        constexpr std::array<std::uint8_t, 9> creatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60};
        constexpr std::array<std::uint8_t, 4> sourceForceTiles{0x12, 0x14, 0x0D, 0x13};
        int checkedCases = 0;
        std::string firstMismatch;
        for (const auto kind : creatureKinds) {
            for (int direction = 0; direction < 4; ++direction) {
                for (unsigned underlay = 0; underlay <= 0x6F; ++underlay) {
                    if (underlay == 0x29) continue; // Portal contact has its own paired-exit scenarios.
                    const auto tile = static_cast<std::uint8_t>(underlay);
                    const int source = at(10, 10);
                    const int player = neighborForMonsterDirection(source, direction);
                    auto level = blankLevel();
                    level.top[static_cast<std::size_t>(player)] = 0x6F;
                    level.bottom[static_cast<std::size_t>(player)] = tile;
                    level.top[static_cast<std::size_t>(source)] =
                        static_cast<std::uint8_t>(kind + direction);
                    level.bottom[static_cast<std::size_t>(source)] = sourceForceTiles[direction];
                    OptionalField monsterStarts;
                    monsterStarts.id = 10;
                    monsterStarts.raw = {10, 10};
                    level.fields.push_back(monsterStarts);
                    auto game = GameState::start(level);
                    game.gameTicks = 1;
                    game.slipOrder.push_back({SlipperKind::creature, 0,
                        static_cast<std::int8_t>(direction)});
                    game.advanceTicks(1);

                    const bool admitted = tile != 0x31 && sourceAllowsEntry(kind, tile, direction);
                    const bool creatureSurvives = tile == 0x03 ? kind == 0x50
                        : tile == 0x04 ? (kind == 0x44 || kind == 0x40 || kind == 0x58)
                        : tile == 0x2A ? false : true;
                    const bool expectedDeath = admitted && creatureSurvives;
                    const auto& creature = game.creatures[0];
                    const bool matches = (game.outcome == Outcome::dead) == expectedDeath &&
                        (!admitted || creature.position != source || !creature.alive) &&
                        (!(admitted && (tile == 0x03 || tile == 0x04 || tile == 0x2A)) ||
                            creature.alive == creatureSurvives);
                    if (!matches && firstMismatch.empty()) {
                        firstMismatch = " (creature " + std::to_string(kind) + ", heading " +
                            std::to_string(direction) + ", Chip underlay " + std::to_string(underlay) +
                            ", admitted " + std::to_string(admitted) + ", alive " +
                            std::to_string(creature.alive) + ", outcome " +
                            std::to_string(static_cast<int>(game.outcome)) + ")";
                    }
                    ++checkedCases;
                }
            }
        }
        failures += !check(firstMismatch.empty() && checkedCases == 9 * 4 * (0x70 - 1),
            "creature-versus-Chip SlipLoop collision and hazard matrix matches source" + firstMismatch);
    }
    {
        // A block can leave Chip's sprite visible after Chip has moved away.
        // MoveMonster's autopsy checks that destination sprite after tile
        // admission and hazard actions, so exercise the same ghost-sprite case
        // for every creature, heading, and possible underlay tile.
        const auto sourceAllowsEntry = [](std::uint8_t creature, std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                case 0x03: return true;
                case 0x04: return creature != 0x40 && creature != 0x58;
                default:
                    return tile == 0x00 || tile == 0x0C || tile == 0x0D ||
                        (tile >= 0x12 && tile <= 0x14) || tile == 0x23 ||
                        tile == 0x24 || tile == 0x26 ||
                        (tile >= 0x27 && tile <= 0x2B) || tile == 0x2F ||
                        (tile >= 0x3C && tile <= 0x3F) ||
                        (tile >= 0x64 && tile <= 0x67) ||
                        (tile >= 0x6C && tile <= 0x6F);
            }
        };
        constexpr std::array<std::uint8_t, 9> creatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60};
        constexpr std::array<std::uint8_t, 4> sourceForceTiles{0x12, 0x14, 0x0D, 0x13};
        int checkedCases = 0;
        std::string firstMismatch;
        for (const auto kind : creatureKinds) {
            for (int direction = 0; direction < 4; ++direction) {
                for (unsigned underlay = 0; underlay <= 0x6F; ++underlay) {
                    if (underlay == 0x29) continue; // Paired portal routing is covered separately.
                    const auto tile = static_cast<std::uint8_t>(underlay);
                    const int source = at(10, 10);
                    const int destination = neighborForMonsterDirection(source, direction);
                    auto level = blankLevel();
                    level.top[at(1, 1)] = 0x6F;
                    level.top[static_cast<std::size_t>(source)] =
                        static_cast<std::uint8_t>(kind + direction);
                    level.bottom[static_cast<std::size_t>(source)] = sourceForceTiles[direction];
                    OptionalField monsterStarts;
                    monsterStarts.id = 10;
                    monsterStarts.raw = {10, 10};
                    level.fields.push_back(monsterStarts);
                    auto game = GameState::start(level);
                    game.top[static_cast<std::size_t>(destination)] = tile == 0x03 ? 0x3F : 0x6F;
                    game.bottom[static_cast<std::size_t>(destination)] = tile;
                    game.gameTicks = 1;
                    game.slipOrder.push_back({SlipperKind::creature, 0,
                        static_cast<std::int8_t>(direction)});
                    game.advanceTicks(1);

                    const bool admitted = tile != 0x31 && sourceAllowsEntry(kind, tile, direction);
                    const bool survivesAction = tile == 0x03 ? kind == 0x50
                        : tile == 0x04 ? (kind == 0x44 || kind == 0x40 || kind == 0x58)
                        : tile == 0x2A ? false : true;
                    const bool expectedDeath = admitted && survivesAction;
                    const auto& creature = game.creatures[0];
                    const bool matches = (game.outcome == Outcome::dead) == expectedDeath &&
                        (!admitted || survivesAction || !creature.alive) &&
                        (!(admitted && (tile == 0x03 || tile == 0x04 || tile == 0x2A)) ||
                            creature.alive == survivesAction);
                    if (!matches && firstMismatch.empty()) {
                        firstMismatch = " (creature " + std::to_string(kind) + ", heading " +
                            std::to_string(direction) + ", ghost underlay " + std::to_string(underlay) +
                            ", admitted " + std::to_string(admitted) + ", alive " +
                            std::to_string(creature.alive) + ", outcome " +
                            std::to_string(static_cast<int>(game.outcome)) + ")";
                    }
                    ++checkedCases;
                }
            }
        }
        failures += !check(firstMismatch.empty() && checkedCases == 9 * 4 * (0x70 - 1),
            "post-move creature autopsy on ghost Chip sprites matches source ordering" + firstMismatch);
    }
    {
        // Exercise real AI movement into Chip (not just SlipLoop collision)
        // for all creature families, headings, and Chip underlay IDs.
        const auto sourceAllowsEntry = [](std::uint8_t creature, std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                case 0x03: return true;
                case 0x04: return creature != 0x40 && creature != 0x58;
                default:
                    return tile == 0x00 || tile == 0x0C || tile == 0x0D ||
                        (tile >= 0x12 && tile <= 0x14) || tile == 0x23 ||
                        tile == 0x24 || tile == 0x26 ||
                        (tile >= 0x27 && tile <= 0x2B) || tile == 0x2F ||
                        (tile >= 0x3C && tile <= 0x3F) ||
                        (tile >= 0x64 && tile <= 0x67) ||
                        (tile >= 0x6C && tile <= 0x6F);
            }
        };
        constexpr std::array<std::uint8_t, 9> creatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60};
        constexpr std::array<std::uint32_t, 4> blobSeeds{99U, 5U, 85U, 100U};
        int checkedCases = 0;
        std::string firstMismatch;
        for (const auto kind : creatureKinds) {
            for (int targetDirection = 0; targetDirection < 4; ++targetDirection) {
                for (unsigned underlay = 0; underlay <= 0x6F; ++underlay) {
                    if (underlay == 0x29) continue; // Paired portal routing is audited separately.
                    const auto tile = static_cast<std::uint8_t>(underlay);
                    auto level = blankLevel();
                    const int source = at(10, 10);
                    const int player = neighborForMonsterDirection(source, targetDirection);
                    level.top[static_cast<std::size_t>(player)] = 0x6F;
                    level.bottom[static_cast<std::size_t>(player)] = tile;
                    int facing = targetDirection;
                    if ((kind & 0xFC) == 0x40) facing = (targetDirection + 3) & 3; // Bug turns left first.
                    else if ((kind & 0xFC) == 0x60) facing = (targetDirection + 1) & 3; // Paramecium turns right first.
                    level.top[static_cast<std::size_t>(source)] =
                        static_cast<std::uint8_t>(kind + facing);
                    for (int direction = 0; direction < 4; ++direction) {
                        if (direction == targetDirection) continue;
                        level.top[static_cast<std::size_t>(neighborForMonsterDirection(source, direction))] = 0x01;
                    }
                    OptionalField monsterStarts;
                    monsterStarts.id = 10;
                    monsterStarts.raw = {10, 10};
                    level.fields.push_back(monsterStarts);
                    auto game = GameState::start(level, 3,
                        (kind & 0xFC) == 0x5C ? blobSeeds[static_cast<std::size_t>(targetDirection)] : 1U);
                    game.advanceTicks(1); // Tick four activates Teeth/Blob and runs MonsterLoop.

                    const bool admitted = sourceAllowsEntry(kind, tile, targetDirection);
                    const bool survivesAction = tile == 0x03 ? kind == 0x50
                        : tile == 0x04 ? (kind == 0x44 || kind == 0x40 || kind == 0x58)
                        : tile == 0x2A ? false : true;
                    const bool expectedDeath = admitted && survivesAction;
                    const auto& creature = game.creatures[0];
                    const bool matches = (game.outcome == Outcome::dead) == expectedDeath &&
                        (!admitted || survivesAction || !creature.alive) &&
                        (!(admitted && (tile == 0x03 || tile == 0x04 || tile == 0x2A)) ||
                            creature.alive == survivesAction);
                    if (!matches && firstMismatch.empty()) {
                        firstMismatch = " (creature " + std::to_string(kind) + ", target heading " +
                            std::to_string(targetDirection) + ", Chip underlay " +
                            std::to_string(underlay) + ", admitted " + std::to_string(admitted) +
                            ", alive " + std::to_string(creature.alive) + ", outcome " +
                            std::to_string(static_cast<int>(game.outcome)) + ", death expected " +
                            std::to_string(expectedDeath) + ", source " + std::to_string(source) +
                            ", creature position " + std::to_string(creature.position) +
                            ", player position " + std::to_string(game.playerPosition) +
                            ", player floor " + std::to_string(game.floor(game.playerPosition)) +
                            ", creature tile " + std::to_string(creature.tile) +
                            ", message " + game.message + ")";
                    }
                    ++checkedCases;
                }
            }
        }
        failures += !check(firstMismatch.empty() && checkedCases == 9 * 4 * (0x70 - 1),
            "MonsterLoop AI-to-Chip collision matrix matches source admission and action ordering" + firstMismatch);
    }
    {
        // Exercise the ordinary even-tick MonsterLoop path against the source
        // MonsterCanEnterTile table. A Tank gives one straight attempt, while
        // explicit initial-creature metadata prevents tested sprite IDs from
        // becoming additional live monsters.
        const auto sourceAllowsEntry = [](std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                case 0x03: return true;
                case 0x04: return true; // Tanks enter Fire, then burn.
                default:
                    return tile == 0x00 || tile == 0x0C || tile == 0x0D ||
                        (tile >= 0x12 && tile <= 0x14) || tile == 0x23 ||
                        tile == 0x24 || tile == 0x26 ||
                        (tile >= 0x27 && tile <= 0x2B) || tile == 0x2F ||
                        (tile >= 0x3C && tile <= 0x3F) ||
                        (tile >= 0x64 && tile <= 0x67) ||
                        (tile >= 0x6C && tile <= 0x6F);
            }
        };
        constexpr std::array<std::uint8_t, 4> tankTiles{0x4C, 0x4D, 0x4E, 0x4F};
        int checkedCases = 0;
        for (int direction = 0; direction < 4; ++direction) {
            for (unsigned tile = 0; tile <= 0x6F; ++tile) {
                if (tile == 0x29) continue; // Portal destination order is tested separately.
                auto level = blankLevel();
                const int source = at(10, 10);
                const int destination = neighborForMonsterDirection(source, direction);
                level.top[at(1, 1)] = 0x6F;
                level.top[static_cast<std::size_t>(source)] = tankTiles[static_cast<std::size_t>(direction)];
                if (tile < 0x6C)
                    level.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);
                OptionalField monsterStarts;
                monsterStarts.id = 10;
                monsterStarts.raw = {10, 10};
                level.fields.push_back(monsterStarts);
                auto game = GameState::start(level);
                if (tile >= 0x6C)
                    game.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);
                game.gameTicks = 1;
                game.advanceTicks(1); // Even tick runs MonsterLoop once.

                const bool expected = sourceAllowsEntry(static_cast<std::uint8_t>(tile), direction);
                const auto& tank = game.creatures[0];
                const bool forcedFloor = tile == 0x0D || (tile >= 0x12 && tile <= 0x14);
                const int forcedDirectionCode = tile == 0x0D ? 2 : tile == 0x12 ? 0 : tile == 0x13 ? 3 : 1;
                const bool entered = tank.position != source || !tank.alive ||
                    (forcedFloor && tank.tankDirection == forcedDirectionCode);
                failures += !check(entered == expected,
                    "MonsterLoop tank admission matches source TileTable for direction " +
                    std::to_string(direction) + ", tile " + std::to_string(tile));
                if (expected && (tile == 0x03 || tile == 0x04))
                    failures += !check(!tank.alive,
                        "MonsterLoop tank applies Water/Fire death action after admission");
                if (tile == 0x03 || tile == 0x04) {
                    failures += !check(game.top[static_cast<std::size_t>(destination)] == tile &&
                                       game.bottom[static_cast<std::size_t>(destination)] == 0x00 &&
                                       game.top[static_cast<std::size_t>(source)] == 0x00,
                        "MonsterLoop hazard death preserves the hazard sprite and clears the departed creature sprite");
                }
                if (tile == 0x2A) {
                    failures += !check(game.top[static_cast<std::size_t>(destination)] == 0x00 &&
                                       game.bottom[static_cast<std::size_t>(destination)] == 0x00,
                        "MonsterLoop bomb death leaves the source Floor sprite instead of a creature or bomb sprite");
                }
                ++checkedCases;
            }
        }
        failures += !check(checkedCases == 4 * (0x70 - 1),
                           "MonsterLoop admission matrix covers all headings and non-portal tile IDs");
    }
    {
        // MonsterCanEnterTile checks the lower layer before its table lookup
        // only for CloneMachine and directional Chip/swimming sprites. Cross
        // every ordinary upper/lower tile pair on the real MonsterLoop path
        // so layered terrain and item sprites cannot hide admission errors.
        const auto sourceAllowsEntry = [](std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                case 0x03: case 0x04: case 0x2A: return true;
                default:
                    return tile == 0x00 || tile == 0x0C || tile == 0x0D ||
                        (tile >= 0x12 && tile <= 0x14) || tile == 0x23 ||
                        tile == 0x24 || tile == 0x26 ||
                        (tile >= 0x27 && tile <= 0x2B) || tile == 0x2F ||
                        (tile >= 0x3C && tile <= 0x3F) ||
                        (tile >= 0x64 && tile <= 0x67) ||
                        (tile >= 0x6C && tile <= 0x6F);
            }
        };
        constexpr std::array<std::uint8_t, 4> tankTiles{0x4C, 0x4D, 0x4E, 0x4F};
        int checkedCases = 0;
        for (int direction = 0; direction < 4; ++direction) {
            for (unsigned upper = 0; upper <= 0x6F; ++upper) {
                for (unsigned lower = 0; lower <= 0x6F; ++lower) {
                    const auto upperTile = static_cast<std::uint8_t>(upper);
                    const auto lowerTile = static_cast<std::uint8_t>(lower);
                    const bool chipOverlay = (upperTile >= 0x3C && upperTile <= 0x3F) ||
                        (upperTile >= 0x6C && upperTile <= 0x6F);
                    const auto inspectedTile = chipOverlay ? lowerTile : upperTile;
                    auto level = blankLevel();
                    const int source = at(10, 10);
                    const int destination = neighborForMonsterDirection(source, direction);
                    level.top[at(1, 1)] = 0x6F;
                    level.top[static_cast<std::size_t>(source)] = tankTiles[static_cast<std::size_t>(direction)];
                    OptionalField monsterStarts;
                    monsterStarts.id = 10;
                    monsterStarts.raw = {10, 10};
                    level.fields.push_back(monsterStarts);
                    auto game = GameState::start(level);
                    game.top[static_cast<std::size_t>(destination)] = upperTile;
                    game.bottom[static_cast<std::size_t>(destination)] = lowerTile;
                    game.gameTicks = 1;
                    game.advanceTicks(1);

                    const bool expected = lowerTile != 0x31 &&
                        sourceAllowsEntry(inspectedTile, direction);
                    const bool forcedFloor = inspectedTile == 0x0D ||
                        (inspectedTile >= 0x12 && inspectedTile <= 0x14);
                    const int forcedDirectionCode = inspectedTile == 0x0D ? 2 :
                        inspectedTile == 0x12 ? 0 : inspectedTile == 0x13 ? 3 : 1;
                    const auto& tank = game.creatures[0];
                    const bool entered = tank.position != source || !tank.alive ||
                        (forcedFloor && tank.tankDirection == forcedDirectionCode);
                    failures += !check(entered == expected,
                        "two-layer MonsterLoop TileTable admission for upper " +
                        std::to_string(upper) + ", lower " + std::to_string(lower) +
                        ", direction " + std::to_string(direction));
                    if (expected && (inspectedTile == 0x03 || inspectedTile == 0x04 || inspectedTile == 0x2A))
                        failures += !check(!tank.alive,
                            "two-layer MonsterLoop resolves Water/Fire/Bomb action after entry");
                    ++checkedCases;
                }
            }
        }
        failures += !check(checkedCases == 4 * 0x70 * 0x70,
                           "two-layer MonsterLoop matrix covers all upper/lower IDs and headings");
    }
    {
        // MoveMonster collision ordering: first apply MonsterCanEnterTile and
        // the destination action, then run its post-move Chip-sprite autopsy.
        // A straight-moving Tank isolates that path across Chip underlays.
        const auto sourceAllowsEntry = [](std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2;
                case 0x07: return direction != 3;
                case 0x08: return direction != 0;
                case 0x09: return direction != 1;
                case 0x1A: return direction == 0 || direction == 1;
                case 0x1B: return direction == 0 || direction == 3;
                case 0x1C: return direction == 2 || direction == 3;
                case 0x1D: return direction == 2 || direction == 1;
                case 0x30: return direction == 2 || direction == 3;
                case 0x03: case 0x04: case 0x2A: return true;
                default:
                    return tile == 0x00 || tile == 0x0C || tile == 0x0D ||
                        (tile >= 0x12 && tile <= 0x14) || tile == 0x23 ||
                        tile == 0x24 || tile == 0x26 ||
                        (tile >= 0x27 && tile <= 0x2B) || tile == 0x2F ||
                        (tile >= 0x64 && tile <= 0x67);
            }
        };
        constexpr std::array<std::uint8_t, 4> tankTiles{0x4C, 0x4D, 0x4E, 0x4F};
        int checkedCases = 0;
        for (int direction = 0; direction < 4; ++direction) {
            for (unsigned tile = 0; tile <= 0x6B; ++tile) {
                if (tile == 0x29 || (tile >= 0x33 && tile <= 0x63)) continue;
                if (tile > 0x32 && tile < 0x64) continue;
                auto level = blankLevel();
                const int source = at(10, 10);
                const int player = neighborForMonsterDirection(source, direction);
                level.top[at(1, 1)] = 0x00;
                level.top[static_cast<std::size_t>(player)] = 0x6F;
                level.bottom[static_cast<std::size_t>(player)] = static_cast<std::uint8_t>(tile);
                level.top[static_cast<std::size_t>(source)] = tankTiles[static_cast<std::size_t>(direction)];
                OptionalField monsterStarts;
                monsterStarts.id = 10;
                monsterStarts.raw = {10, 10};
                level.fields.push_back(monsterStarts);
                auto game = GameState::start(level);
                game.gameTicks = 1;
                game.advanceTicks(1);

                const bool admitted = sourceAllowsEntry(static_cast<std::uint8_t>(tile), direction);
                const bool creatureDiesFirst = tile == 0x03 || tile == 0x04 || tile == 0x2A;
                const bool shouldCatchChip = admitted && !creatureDiesFirst;
                failures += !check((game.outcome == Outcome::dead) == shouldCatchChip,
                    "MoveMonster autopsy matches source for Chip underlay " +
                    std::to_string(tile) + ", direction " + std::to_string(direction) +
                    " (outcome=" + std::to_string(static_cast<int>(game.outcome)) +
                    ", player=" + std::to_string(game.playerPosition) +
                    ", creature=" + std::to_string(game.creatures[0].position) +
                    ", floor=" + std::to_string(game.floor(game.playerPosition)) + ")");
                if (admitted && creatureDiesFirst)
                    failures += !check(!game.creatures[0].alive && game.outcome == Outcome::playing,
                        "Water/Fire/Bomb action precedes Chip autopsy for underlay " + std::to_string(tile));
                ++checkedCases;
            }
        }
        failures += !check(checkedCases == 4 * (0x33 + 8 - 1),
                           "MoveMonster autopsy matrix covers all terrain, key, and boot underlays except portals");
    }    {
        // When a falling or moving block lands on Chip, MoveBlock dispatches
        // the visible Chip/swimming sprite's action (TileTable action 1), then
        // copies that sprite into Lower before crushing Chip. It must not look
        // through Chip and apply Water/Bomb actions from the underlying tile.
        for (const auto underlay : {std::uint8_t{0x03}, std::uint8_t{0x04}, std::uint8_t{0x2A}}) {
            const auto expectedPlayerSprite = underlay == 0x03 ? 0x3F : 0x6F;

            auto cloneLevel = blankLevel();
            cloneLevel.top[at(1, 1)] = 0x6F;
            cloneLevel.top[at(2, 1)] = 0x24;
            cloneLevel.bottom[at(2, 1)] = underlay;
            cloneLevel.top[at(2, 2)] = 0x0E; // Block template points north onto the button/Chip.
            cloneLevel.bottom[at(2, 2)] = 0x31;
            OptionalField cloneLink;
            cloneLink.id = 5;
            cloneLink.raw = {2, 0, 1, 0, 2, 0, 2, 0};
            cloneLevel.fields.push_back(cloneLink);
            auto cloneGame = GameState::start(cloneLevel);
            (void)cloneGame.move(Direction::east);
            const int cloneTarget = at(2, 1);
            failures += !check(cloneGame.outcome == Outcome::dead &&
                               cloneGame.top[static_cast<std::size_t>(cloneTarget)] == 0x0A &&
                               cloneGame.bottom[static_cast<std::size_t>(cloneTarget)] == expectedPlayerSprite,
                "clone block crush stores Chip sprite over underlying tile " + std::to_string(underlay));

            auto slidingLevel = blankLevel();
            slidingLevel.top[at(6, 5)] = 0x6F;
            slidingLevel.bottom[at(6, 5)] = underlay;
            auto slidingGame = GameState::start(slidingLevel);
            const int blockSource = at(5, 5);
            const int playerTarget = at(6, 5);
            slidingGame.top[static_cast<std::size_t>(blockSource)] = 0x0A;
            slidingGame.bottom[static_cast<std::size_t>(blockSource)] = 0x13;
            slidingGame.blockDirections[static_cast<std::size_t>(blockSource)] = static_cast<std::int8_t>(Direction::east);
            slidingGame.slipOrder.push_back({SlipperKind::block, blockSource});
            slidingGame.advanceTicks(1);
            failures += !check(slidingGame.outcome == Outcome::dead &&
                               slidingGame.top[static_cast<std::size_t>(playerTarget)] == 0x0A &&
                               slidingGame.bottom[static_cast<std::size_t>(playerTarget)] == expectedPlayerSprite,
                "sliding block crush stores Chip sprite over underlying tile " + std::to_string(underlay));
        }
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x24;
        level.top[at(5, 5)] = 0x40;
        level.bottom[at(5, 5)] = 0x31;
        level.top[at(5, 4)] = 0x2B;
        level.bottom[at(5, 4)] = 0x2B;
        level.top[at(7, 7)] = 0x27;
        OptionalField clone;
        clone.id = 5;
        clone.raw = {2, 0, 1, 0, 5, 0, 5, 0};
        OptionalField trap;
        trap.id = 4;
        trap.raw = {7, 0, 7, 0, 5, 0, 4, 0, 0, 0};
        level.fields.push_back(clone);
        level.fields.push_back(trap);
        auto openTrap = startWithMapMonsters(level);
        (void)openTrap.move(Direction::east);
        failures += !check(openTrap.creatures.size() == 1,
                           "clone can enter an open linked trap");
        openTrap.advanceTicks(2);
        failures += !check(openTrap.creatures[0].position == at(5, 4) && openTrap.slipOrder.empty(),
                           "cloned creature enters an open trap without starting a slide");

        level.fields[1].raw[8] = 1;
        auto closedTrap = startWithMapMonsters(level);
        (void)closedTrap.move(Direction::east);
        failures += !check(closedTrap.creatures.size() == 1,
                           "clone can spawn when its output path enters a closed linked trap");
        closedTrap.advanceTicks(2);
        failures += !check(closedTrap.creatures[0].position == at(5, 4) && closedTrap.traps[0].closed &&
                           closedTrap.slipOrder.empty(),
                           "cloned creature enters a closed trap and stops its slide there");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x4D; // east-facing Tank on a closed trap
        level.bottom[at(5, 5)] = 0x2B;
        level.top[at(7, 7)] = 0x27;
        OptionalField trap;
        trap.id = 4;
        trap.raw = {7, 0, 7, 0, 5, 0, 5, 0, 1, 0}; // initially closed
        level.fields.push_back(trap);
        auto game = startWithMapMonsters(level);
        // Tank direction vectors are separate from the displayed sprite.
        // The source attempts its vector, updates facing, then remains held
        // because this trap connection is closed.
        game.creatures[0].movementDirection = 0;
        game.creatures[0].tankDirection = 0;
        game.gameTicks = 1; // next tick is an even-tick MonsterLoop pass
        game.advanceTicks(1);
        failures += !check(game.creatures[0].position == at(5, 5) &&
                           game.creatures[0].tile == 0x4C &&
                           game.creatures[0].movementDirection == 0 &&
                           game.creatures[0].tankDirection == 0,
                           "Tank updates its facing but keeps moving vector while held by closed trap");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x5C; // blob
        level.bottom[at(5, 5)] = 0x2B;
        level.top[at(7, 7)] = 0x27;
        OptionalField trap;
        trap.id = 4;
        trap.raw = {7, 0, 7, 0, 5, 0, 5, 0, 1, 0}; // initially closed
        level.fields.push_back(trap);
        auto game = startWithMapMonsters(level);
        const auto initialRandomState = game.randomState;
        game.advanceTicks(4);
        failures += !check(game.creatures.size() == 1 && game.creatures[0].position == at(5, 5),
                           "blob remains on a closed trap");
        failures += !check(game.traps.size() == 1 && game.traps[0].closed,
                           "blob test trap begins closed");
        failures += !check(game.randomState != initialRandomState &&
                           game.randomState == 3750785579U && game.creatures[0].tile == 0x5D,
                           "blob exhausts the source-ordered random fallback turns inside a closed trap");
    }
    {
        constexpr std::array<std::uint8_t, 9> creatureKinds{
            0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60};
        int checkedTrapStarts = 0;
        for (const auto kind : creatureKinds) {
            for (int direction = 0; direction < 4; ++direction) {
                auto level = blankLevel();
                const int playerStart = at(1, 1);
                const int creatureStart = at(8, 8);
                level.top[playerStart] = 0x6F;
                level.top[creatureStart] = static_cast<std::uint8_t>(kind + direction);
                level.bottom[creatureStart] = 0x2B;
                const int trapButton = at(10, 10);
                level.top[trapButton] = 0x27;
                OptionalField link;
                link.id = 4;
                link.raw = {10, 0, 10, 0, 8, 0, 8, 0, 1, 0};
                level.fields.push_back(link);
                auto game = startWithMapMonsters(level);
                const bool delayedFamily = kind == 0x54 || kind == 0x5C;
                game.gameTicks = delayedFamily ? 3U : 1U;
                game.advanceTicks(1); // even-tick MonsterLoop; delayed species reach tick 4
                const auto found = std::find_if(game.creatures.begin(), game.creatures.end(),
                    [creatureStart](const Creature& creature) {
                        return creature.alive && creature.position == creatureStart;
                    });
                failures += !check(found != game.creatures.end(),
                    "creature " + std::to_string(kind) + " facing " + std::to_string(direction) +
                    " remains held on a closed trap");
                ++checkedTrapStarts;
            }
        }
        failures += !check(checkedTrapStarts == 9 * 4,
                           "closed-trap hold matrix covers every creature family and facing");

        // CloneMachine is a source constraint on fallback choices, not a
        // closed trap: a creature can take its first valid direction out.
        int checkedCloneMachineExits = 0;
        int checkedCloneMachineNoFallbacks = 0;
        for (const auto kind : creatureKinds) {
            for (int direction = 0; direction < 4; ++direction) {
                const int creatureStart = at(8, 8);
                const int firstTarget = neighborForMonsterDirection(creatureStart, direction);
                auto exitLevel = blankLevel();
                exitLevel.top[at(1, 1)] = 0x6F;
                exitLevel.top[at(2, 1)] = 0x24;
                exitLevel.top[creatureStart] = static_cast<std::uint8_t>(kind + direction);
                exitLevel.bottom[creatureStart] = 0x31;
                OptionalField exitCloneLink;
                exitCloneLink.id = 5;
                exitCloneLink.raw = {2, 0, 1, 0, 8, 0, 8, 0};
                exitLevel.fields.push_back(exitCloneLink);
                for (int candidate = 0; candidate < 4; ++candidate) {
                    if (candidate == direction) continue;
                    exitLevel.top[static_cast<std::size_t>(neighborForMonsterDirection(creatureStart, candidate))] = 0x01;
                }
                auto canLeave = GameState::start(exitLevel);
                (void)canLeave.move(Direction::east); // clone is emitted on the normal gameplay path
                canLeave.advanceTicks(kind == 0x54 || kind == 0x5C ? 4U : 2U);
                const auto exited = std::find_if(canLeave.creatures.begin(), canLeave.creatures.end(),
                    [firstTarget](const Creature& creature) {
                        return creature.alive && creature.position == firstTarget;
                    });
                failures += !check(exited != canLeave.creatures.end(),
                    "creature " + std::to_string(kind) + " facing " + std::to_string(direction) +
                    " can leave its CloneMachine on a valid first attempt");
                ++checkedCloneMachineExits;

                if (kind == 0x5C) continue; // Blob deliberately keeps retrying on CloneMachines.
                auto blockedLevel = blankLevel();
                blockedLevel.top[at(1, 1)] = 0x6F;
                blockedLevel.top[at(2, 1)] = 0x24;
                blockedLevel.top[creatureStart] = static_cast<std::uint8_t>(kind + direction);
                blockedLevel.bottom[creatureStart] = 0x31;
                for (int candidate = 0; candidate < 4; ++candidate) {
                    if (candidate == direction) continue;
                    blockedLevel.top[static_cast<std::size_t>(neighborForMonsterDirection(creatureStart, candidate))] = 0x00;
                }
                OptionalField blockedCloneLink;
                blockedCloneLink.id = 5;
                blockedCloneLink.raw = {2, 0, 1, 0, 8, 0, 8, 0};
                blockedLevel.fields.push_back(blockedCloneLink);
                auto noFallback = GameState::start(blockedLevel);
                (void)noFallback.move(Direction::east);
                // The emitted creature must retain its map template while
                // the live map makes only its alternate directions passable.
                noFallback.top[static_cast<std::size_t>(firstTarget)] = 0x01;
                noFallback.advanceTicks(kind == 0x54 ? 4U : 2U);
                const auto held = std::find_if(noFallback.creatures.begin(), noFallback.creatures.end(),
                    [creatureStart](const Creature& creature) {
                        return creature.alive && creature.position == creatureStart;
                    });
                failures += !check(held != noFallback.creatures.end(),
                    "creature " + std::to_string(kind) + " facing " + std::to_string(direction) +
                    " does not turn after a blocked CloneMachine departure");
                ++checkedCloneMachineNoFallbacks;
            }
        }
        failures += !check(checkedCloneMachineExits == 9 * 4 && checkedCloneMachineNoFallbacks == 8 * 4,
                           "CloneMachine matrix covers direct exits and every non-Blob no-fallback case");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x24;
        level.top[at(5, 5)] = 0x0E; // block prototype points north
        level.bottom[at(5, 5)] = 0x31;
        OptionalField links;
        links.id = 5;
        links.raw = {2, 0, 1, 0, 5, 0, 5, 0};
        level.fields.push_back(links);
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.top[at(5, 5)] == 0x0E && game.top[at(5, 4)] == 0x0A,
                           "clone button emits a normal block and preserves its directional machine template");

        level.top[at(5, 4)] = 0x0C;
        auto slidingClone = startWithMapMonsters(level);
        (void)slidingClone.move(Direction::east);
        failures += !check(slidingClone.top[at(5, 4)] == 0x0A &&
                           slidingClone.blockDirections[at(5, 4)] == static_cast<std::int8_t>(Direction::north),
                           "clone block starts a slip when emitted onto ice");
        slidingClone.advanceTicks(1);
        failures += !check(slidingClone.top[at(5, 3)] == 0x0A &&
                           slidingClone.top[at(5, 4)] == 0x0C && slidingClone.bottom[at(5, 4)] == 0x00,
                           "cloned block advances in the ordered slip phase");

        level.top[at(5, 4)] = 0x03;
        level.bottom[at(5, 4)] = 0x03;
        auto cloneWater = startWithMapMonsters(level);
        (void)cloneWater.move(Direction::east);
        failures += !check(cloneWater.top[at(5, 4)] == 0x0B && cloneWater.bottom[at(5, 4)] == 0x03,
                           "cloned block fills water with dirt and preserves its underlay");

        level.top[at(5, 4)] = 0x04;
        auto cloneFire = startWithMapMonsters(level);
        (void)cloneFire.move(Direction::east);
        failures += !check(cloneFire.top[at(5, 4)] == 0x0A && cloneFire.bottom[at(5, 4)] == 0x04,
                           "clone block remains over fire");
        failures += !check(cloneFire.message == "The clone block covers the fire.",
                           "clone-block feedback reflects the retained fire terrain");

        auto cloneTeleportLevel = blankLevel();
        cloneTeleportLevel.top[at(1, 1)] = 0x6F;
        cloneTeleportLevel.top[at(2, 1)] = 0x24;
        cloneTeleportLevel.top[at(5, 5)] = 0x0E; // Clone block points north.
        cloneTeleportLevel.bottom[at(5, 5)] = 0x31;
        cloneTeleportLevel.top[at(5, 4)] = 0x29; // Output portal, first in map order.
        cloneTeleportLevel.top[at(8, 10)] = 0x29; // Reverse scan fallback.
        cloneTeleportLevel.top[at(10, 10)] = 0x29; // First reverse candidate is blocked.
        cloneTeleportLevel.top[at(10, 9)] = 0x01;
        OptionalField cloneTeleportLink;
        cloneTeleportLink.id = 5;
        cloneTeleportLink.raw = {2, 0, 1, 0, 5, 0, 5, 0};
        cloneTeleportLevel.fields.push_back(cloneTeleportLink);
        auto cloneTeleport = startWithMapMonsters(cloneTeleportLevel);
        (void)cloneTeleport.move(Direction::east);
        failures += !check(cloneTeleport.top[at(5, 5)] == 0x0E &&
                           cloneTeleport.top[at(5, 4)] == 0x29 &&
                           cloneTeleport.top[at(8, 10)] == 0x0A &&
                           cloneTeleport.bottom[at(8, 10)] == 0x29 &&
                           cloneTeleport.top[at(10, 10)] == 0x29 &&
                           cloneTeleport.slipOrder.size() == 1 &&
                           cloneTeleport.slipOrder[0].entity == at(8, 10) &&
                           cloneTeleport.blockDirections[at(8, 10)] ==
                               static_cast<std::int8_t>(Direction::north),
                           "clone block teleports using reverse portal search and skips blocked exits");
        cloneTeleport.advanceTicks(1);
        failures += !check(cloneTeleport.top[at(8, 9)] == 0x0A &&
                           cloneTeleport.top[at(8, 10)] == 0x29 &&
                           cloneTeleport.bottom[at(8, 10)] == 0x00 &&
                           cloneTeleport.slipOrder.empty(),
                           "clone block continues north from the selected exit portal");

        level.top[at(5, 4)] = 0x13; // East force floor; the clone block template enters north.
        auto cloneForce = startWithMapMonsters(level);
        (void)cloneForce.move(Direction::east);
        failures += !check(cloneForce.blockDirections[at(5, 4)] ==
                               static_cast<std::int8_t>(Direction::east) &&
                           !cloneForce.slipOrder.empty(),
                           "clone block takes the directional force-floor heading");

        auto cloneTrapLevel = blankLevel();
        cloneTrapLevel.top[at(1, 1)] = 0x6F;
        cloneTrapLevel.top[at(2, 1)] = 0x24;
        cloneTrapLevel.top[at(5, 5)] = 0x0E; // clone block points north
        cloneTrapLevel.bottom[at(5, 5)] = 0x31;
        cloneTrapLevel.top[at(5, 4)] = 0x2B;
        cloneTrapLevel.bottom[at(5, 4)] = 0x2B;
        cloneTrapLevel.top[at(7, 7)] = 0x27;
        OptionalField cloneTrapMachine;
        cloneTrapMachine.id = 5;
        cloneTrapMachine.raw = {2, 0, 1, 0, 5, 0, 5, 0};
        OptionalField cloneTrapLink;
        cloneTrapLink.id = 4;
        cloneTrapLink.raw = {7, 0, 7, 0, 5, 0, 4, 0, 0, 0}; // open linked trap
        cloneTrapLevel.fields.push_back(cloneTrapMachine);
        cloneTrapLevel.fields.push_back(cloneTrapLink);
        auto cloneTrap = startWithMapMonsters(cloneTrapLevel);
        (void)cloneTrap.move(Direction::east);
        failures += !check(cloneTrap.top[at(5, 4)] == 0x0A &&
                           cloneTrap.blockDirections[at(5, 4)] == -1 &&
                           cloneTrap.slipOrder.empty(),
                           "clone block entering a trap does not acquire a new slip entry");
        cloneTrap.advanceTicks(1);
        failures += !check(cloneTrap.top[at(5, 4)] == 0x0A &&
                           cloneTrap.bottom[at(5, 4)] == 0x2B &&
                           cloneTrap.slipOrder.empty(),
                           "stationary clone block remains on its trap during the slip phase");

        auto blockOntoChipLevel = blankLevel();
        blockOntoChipLevel.top[at(1, 1)] = 0x6F;
        blockOntoChipLevel.top[at(2, 1)] = 0x24;
        blockOntoChipLevel.top[at(3, 1)] = 0x0F; // clone block points west onto Chip
        blockOntoChipLevel.bottom[at(3, 1)] = 0x31;
        OptionalField blockOntoChipLink;
        blockOntoChipLink.id = 5;
        blockOntoChipLink.raw = {2, 0, 1, 0, 3, 0, 1, 0};
        blockOntoChipLevel.fields.push_back(blockOntoChipLink);
        auto blockOntoChip = startWithMapMonsters(blockOntoChipLevel);
        failures += !check(blockOntoChip.move(Direction::east) == MoveResult::died &&
                           blockOntoChip.outcome == Outcome::dead &&
                           blockOntoChip.playerPosition == at(2, 1) &&
                           blockOntoChip.top[at(2, 1)] == 0x0A &&
                           blockOntoChip.bottom[at(2, 1)] == 0x6F &&
                           blockOntoChip.slipOrder.empty(),
                           "clone block uses MoveBlock and squishes Chip on the output square");
        failures += !check(blockOntoChip.move(Direction::west) == MoveResult::ignored,
                           "Chip cannot leave a clone block after being squished");

    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x24; // First CloneButton.
        level.top[at(5, 5)] = 0x0E; // First machine emits north.
        level.bottom[at(5, 5)] = 0x31;
        level.top[at(5, 4)] = 0x24; // First block presses the second CloneButton.
        level.top[at(8, 5)] = 0x11; // Second machine emits east.
        level.bottom[at(8, 5)] = 0x31;
        OptionalField links;
        links.id = 5;
        links.raw = {
            2, 0, 1, 0, 5, 0, 5, 0, // button (2,1) -> machine (5,5)
            5, 0, 4, 0, 8, 0, 5, 0  // button (5,4) -> machine (8,5)
        };
        level.fields.push_back(links);
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(2, 1) &&
                           game.top[at(5, 5)] == 0x0E && game.bottom[at(5, 5)] == 0x31 &&
                           game.top[at(5, 4)] == 0x0A && game.bottom[at(5, 4)] == 0x24 &&
                           game.top[at(8, 5)] == 0x11 && game.bottom[at(8, 5)] == 0x31 &&
                           game.top[at(9, 5)] == 0x0A && game.bottom[at(9, 5)] == 0x00 &&
                           game.slipOrder.empty(),
                           "clone block immediately activates a linked CloneButton and emits its block");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(5, 5)] = 0x0A;
        level.bottom[at(5, 5)] = 0x13; // moving east on a force floor
        level.top[at(6, 5)] = 0x24;
        level.top[at(8, 5)] = 0x11; // clone machine emits an eastbound block
        level.bottom[at(8, 5)] = 0x31;
        level.top[at(9, 5)] = 0x0C; // clone output starts sliding on ice
        OptionalField cloneLink;
        cloneLink.id = 5;
        cloneLink.raw = {6, 0, 5, 0, 8, 0, 5, 0};
        level.fields.push_back(cloneLink);

        auto game = startWithMapMonsters(level);
        game.blockDirections[at(5, 5)] = static_cast<std::int8_t>(Direction::east);
        game.slipOrder.push_back({SlipperKind::block, at(5, 5)});
        game.advanceTicks(1);
        failures += !check(game.top[at(6, 5)] == 0x0A && game.top[at(9, 5)] == 0x0A &&
                           game.top[at(10, 5)] == 0x00 && game.slipOrder.size() == 1 &&
                           game.slipOrder[0].entity == at(9, 5),
                           "slip loop skips a newly cloned slipper when its net list length is unchanged");
        game.advanceTicks(1);
        failures += !check(game.top[at(10, 5)] == 0x0A && game.slipOrder.empty(),
                           "newly cloned slipper advances on the following slip-loop tick");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x29;
        level.top[at(6, 1)] = 0x29;
        level.top[at(10, 1)] = 0x29;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(10, 1) && game.sliding &&
                           game.playerTeleportedThisTick,
                           "teleporter searches linked destinations in reverse map order");
        game.advanceTicks(1);
        failures += !check(game.playerPosition == at(11, 1) && !game.sliding,
                           "Chip continues in its original direction after teleporting");

        level.top[at(11, 1)] = 0x01; // block the first candidate's east-facing exit
        auto upperWallOverlay = startWithMapMonsters(level);
        (void)upperWallOverlay.move(Direction::east);
        failures += !check(upperWallOverlay.playerPosition == at(6, 1) && upperWallOverlay.sliding,
                           "teleporter rejects an upper wall and tries the prior destination");

        level.top[at(11, 1)] = 0x00;
        level.bottom[at(11, 1)] = 0x01; // hidden lower wall; upper Floor remains enterable
        auto fallback = startWithMapMonsters(level);
        (void)fallback.move(Direction::east);
        failures += !check(fallback.playerPosition == at(10, 1),
                           "teleporter validates the upper floor over a lower wall");

        level.top[at(7, 1)] = 0x01;
        level.top[at(11, 1)] = 0x01; // both alternate east-facing upper layers are blocked
        auto blocked = startWithMapMonsters(level);
        (void)blocked.move(Direction::east);
        failures += !check(blocked.playerPosition == at(2, 1),
                           "teleporter leaves Chip on the entry tile when all exits are blocked");
        failures += !check(blocked.sliding && blocked.slideDirection == Direction::east,
                           "failed teleporter routing still starts Chip's entry-direction slide");
        blocked.advanceTicks(1);
        failures += !check(blocked.playerPosition == at(3, 1) && !blocked.sliding,
                           "Chip slides off a blocked paired teleporter on the following tick");

        auto swimSpriteExit = blankLevel();
        swimSpriteExit.top[at(1, 1)] = 0x6F;
        swimSpriteExit.top[at(2, 1)] = 0x29;
        swimSpriteExit.top[at(6, 1)] = 0x29;
        swimSpriteExit.top[at(7, 1)] = 0x3C; // Chip sprites are blocked by ChipCanEnterTile.
        auto swimSpriteBlocked = startWithMapMonsters(swimSpriteExit);
        (void)swimSpriteBlocked.move(Direction::east);
        failures += !check(swimSpriteBlocked.playerPosition == at(2, 1) && swimSpriteBlocked.sliding,
                           "teleporter rejects a swimming-Chip sprite at the exit");

        auto fakeFloorExit = blankLevel();
        fakeFloorExit.top[at(1, 1)] = 0x6F;
        fakeFloorExit.top[at(2, 1)] = 0x29;
        fakeFloorExit.top[at(6, 1)] = 0x29;
        fakeFloorExit.top[at(7, 1)] = 0x01;
        fakeFloorExit.bottom[at(7, 1)] = 0x1E;
        auto fakeFloorTeleport = startWithMapMonsters(fakeFloorExit);
        (void)fakeFloorTeleport.move(Direction::east);
        failures += !check(fakeFloorTeleport.playerPosition == at(2, 1) &&
                           fakeFloorTeleport.top[at(7, 1)] == 0x01,
                           "teleport validation rejects visible upper wall over FakeFloor");

        auto hiddenWallExit = blankLevel();
        hiddenWallExit.top[at(1, 1)] = 0x6F;
        hiddenWallExit.top[at(2, 1)] = 0x29;
        hiddenWallExit.top[at(6, 1)] = 0x29;
        hiddenWallExit.bottom[at(7, 1)] = 0x2C;
        auto hiddenWallTeleport = startWithMapMonsters(hiddenWallExit);
        (void)hiddenWallTeleport.move(Direction::east);
        failures += !check(hiddenWallTeleport.playerPosition == at(6, 1) &&
                           hiddenWallTeleport.top[at(7, 1)] == 0x00,
                           "teleport validation uses visible upper Floor over a hidden lower wall");

        auto thiefExit = blankLevel();
        thiefExit.top[at(1, 1)] = 0x6F;
        thiefExit.top[at(2, 1)] = 0x29;
        thiefExit.top[at(6, 1)] = 0x29;
        thiefExit.bottom[at(7, 1)] = 0x21;
        auto thiefTeleport = startWithMapMonsters(thiefExit);
        thiefTeleport.boots.fill(true);
        thiefTeleport.keys.fill(1);
        (void)thiefTeleport.move(Direction::east);
        failures += !check(thiefTeleport.playerPosition == at(6, 1) &&
                           std::all_of(thiefTeleport.boots.begin(), thiefTeleport.boots.end(),
                                       [](bool hasBoot) { return hasBoot; }) &&
                           std::all_of(thiefTeleport.keys.begin(), thiefTeleport.keys.end(),
                                       [](unsigned count) { return count == 1; }),
                           "teleport validation uses visible upper Floor without lower Thief effects");

        auto doorExit = blankLevel();
        doorExit.top[at(1, 1)] = 0x6F;
        doorExit.top[at(2, 1)] = 0x29;
        doorExit.top[at(6, 1)] = 0x29;
        doorExit.bottom[at(7, 1)] = 0x16;
        auto doorTeleport = startWithMapMonsters(doorExit);
        doorTeleport.keys[0] = 1;
        (void)doorTeleport.move(Direction::east);
        failures += !check(doorTeleport.playerPosition == at(6, 1) && doorTeleport.keys[0] == 1,
                           "teleport door validation checks but does not consume a key");
        doorTeleport.advanceTicks(1);
        failures += !check(doorTeleport.playerPosition == at(7, 1) && doorTeleport.keys[0] == 1,
                           "visible upper Floor over a lower door does not consume a key");

        auto visibleDoorExit = blankLevel();
        visibleDoorExit.top[at(1, 1)] = 0x6F;
        visibleDoorExit.top[at(2, 1)] = 0x29;
        visibleDoorExit.top[at(6, 1)] = 0x29;
        visibleDoorExit.top[at(7, 1)] = 0x16;
        auto visibleDoorTeleport = startWithMapMonsters(visibleDoorExit);
        visibleDoorTeleport.keys[0] = 1;
        (void)visibleDoorTeleport.move(Direction::east);
        failures += !check(visibleDoorTeleport.playerPosition == at(6, 1) &&
                           visibleDoorTeleport.keys[0] == 1,
                           "visible door key remains until the teleporter slide enters the door");
        visibleDoorTeleport.advanceTicks(1);
        failures += !check(visibleDoorTeleport.playerPosition == at(7, 1) &&
                           visibleDoorTeleport.keys[0] == 0 &&
                           visibleDoorTeleport.top[at(7, 1)] == 0x00,
                           "teleporter slide consumes the key when Chip actually enters the visible door");

        auto popupUnderlayExit = blankLevel();
        popupUnderlayExit.top[at(1, 1)] = 0x6F;
        popupUnderlayExit.top[at(2, 1)] = 0x29;
        popupUnderlayExit.top[at(6, 1)] = 0x29;
        popupUnderlayExit.bottom[at(7, 1)] = 0x2E;
        auto popupUnderlayTeleport = startWithMapMonsters(popupUnderlayExit);
        (void)popupUnderlayTeleport.move(Direction::east);
        failures += !check(popupUnderlayTeleport.playerPosition == at(6, 1) &&
                           popupUnderlayTeleport.top[at(7, 1)] == 0x00,
                           "teleport validation uses visible upper Floor over lower PopupWall");

        auto hazardExit = blankLevel();
        hazardExit.top[at(1, 1)] = 0x6F;
        hazardExit.top[at(2, 1)] = 0x29;
        hazardExit.top[at(6, 1)] = 0x29;
        hazardExit.top[at(10, 1)] = 0x29;
        hazardExit.top[at(11, 1)] = 0x03;
        auto teleportsIntoWater = startWithMapMonsters(hazardExit);
        (void)teleportsIntoWater.move(Direction::east);
        failures += !check(teleportsIntoWater.playerPosition == at(10, 1) && teleportsIntoWater.sliding,
                           "teleporter validation accepts a water exit before its hazard action");
        teleportsIntoWater.advanceTicks(1);
        failures += !check(teleportsIntoWater.outcome == Outcome::dead &&
                           teleportsIntoWater.playerPosition == at(11, 1),
                           "Chip drowns when the accepted teleporter exit step enters water");

        auto monsterExit = blankLevel();
        monsterExit.top[at(1, 1)] = 0x6F;
        monsterExit.top[at(2, 1)] = 0x29;
        monsterExit.top[at(6, 1)] = 0x29;
        monsterExit.top[at(7, 1)] = 0x48; // Ball occupies the east exit over Floor.
        auto teleportsIntoMonster = startWithMapMonsters(monsterExit);
        teleportsIntoMonster.gameTicks = 1;
        (void)teleportsIntoMonster.move(Direction::east);
        failures += !check(teleportsIntoMonster.playerPosition == at(6, 1) &&
                           teleportsIntoMonster.sliding,
                           "teleporter accepts a creature-occupied exit over passable terrain");
        teleportsIntoMonster.advanceTicks(1);
        failures += !check(teleportsIntoMonster.outcome == Outcome::playing &&
                           teleportsIntoMonster.playerPosition == at(7, 1) &&
                           teleportsIntoMonster.creatures[0].position == at(7, 2),
                           "MonsterLoop moves before Chip's teleporter slide step");

        auto panelExit = blankLevel();
        panelExit.top[at(2, 1)] = 0x6E; // enter teleporter heading south
        panelExit.top[at(2, 2)] = 0x29;
        panelExit.top[at(6, 2)] = 0x29;
        panelExit.top[at(10, 2)] = 0x29;
        panelExit.bottom[at(10, 3)] = 0x06; // PanelN rejects entry from the south
        auto panelFallback = startWithMapMonsters(panelExit);
        (void)panelFallback.move(Direction::south);
        failures += !check(panelFallback.playerPosition == at(10, 2),
                           "teleporter validates upper Floor over a lower directional panel");

        auto popupExit = blankLevel();
        popupExit.top[at(1, 1)] = 0x6F;
        popupExit.top[at(2, 1)] = 0x29;
        popupExit.top[at(6, 1)] = 0x29;
        popupExit.top[at(10, 1)] = 0x29;
        popupExit.top[at(11, 1)] = 0x2E;
        auto teleportsToPopup = startWithMapMonsters(popupExit);
        (void)teleportsToPopup.move(Direction::east);
        failures += !check(teleportsToPopup.playerPosition == at(10, 1),
                           "teleporter validation accepts a popup wall before it rises");
        teleportsToPopup.advanceTicks(1);
        failures += !check(teleportsToPopup.playerPosition == at(11, 1) &&
                           teleportsToPopup.top[at(11, 1)] == 0x01,
                           "popup wall rises as Chip enters after teleport arrival");

        auto blockAtTeleportExit = blankLevel();
        blockAtTeleportExit.top[at(1, 1)] = 0x6F;
        blockAtTeleportExit.top[at(2, 1)] = 0x29;
        blockAtTeleportExit.top[at(8, 5)] = 0x29;
        blockAtTeleportExit.top[at(9, 5)] = 0x0A;
        auto pushesBlockOnTeleport = startWithMapMonsters(blockAtTeleportExit);
        (void)pushesBlockOnTeleport.move(Direction::east);
        failures += !check(pushesBlockOnTeleport.playerPosition == at(8, 5) &&
                           pushesBlockOnTeleport.sliding &&
                           pushesBlockOnTeleport.top[at(9, 5)] == 0x00 &&
                           pushesBlockOnTeleport.top[at(10, 5)] == 0x0A,
                           "teleporter pushes a block out of its exit square");
        pushesBlockOnTeleport.advanceTicks(1);
        failures += !check(pushesBlockOnTeleport.playerPosition == at(9, 5),
                           "Chip slides into the square vacated by the pushed exit block");

        auto blockedBlockExit = blankLevel();
        blockedBlockExit.top[at(1, 1)] = 0x6F;
        blockedBlockExit.top[at(2, 1)] = 0x29;
        blockedBlockExit.top[at(6, 1)] = 0x29;
        blockedBlockExit.top[at(10, 1)] = 0x29;
        blockedBlockExit.top[at(11, 1)] = 0x0A;
        blockedBlockExit.top[at(12, 1)] = 0x01;
        auto blockFallback = startWithMapMonsters(blockedBlockExit);
        (void)blockFallback.move(Direction::east);
        failures += !check(blockFallback.playerPosition == at(6, 1) &&
                           blockFallback.top[at(11, 1)] == 0x0A,
                           "teleporter rejects a block exit that cannot be pushed and tries the prior pair");

        auto coveredPortalLevel = blankLevel();
        coveredPortalLevel.top[at(1, 1)] = 0x6F;
        coveredPortalLevel.top[at(2, 1)] = 0x29;
        coveredPortalLevel.top[at(6, 1)] = 0x29;
        coveredPortalLevel.top[at(10, 1)] = 0x29;
        auto coveredPortal = startWithMapMonsters(coveredPortalLevel);
        coveredPortal.top[at(10, 1)] = 0x0A;
        coveredPortal.bottom[at(10, 1)] = 0x29;
        (void)coveredPortal.move(Direction::east);
        failures += !check(coveredPortal.playerPosition == at(6, 1),
                           "teleporter skips a candidate portal covered by a block");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(2, 1)] = 0x0A;
        level.top[at(3, 1)] = 0x29;
        level.top[at(8, 5)] = 0x29;
        auto game = startWithMapMonsters(level);
        (void)game.move(Direction::east);
        failures += !check(game.playerPosition == at(2, 1) && game.top[at(8, 5)] == 0x0A &&
                           game.bottom[at(8, 5)] == 0x29 && game.top[at(3, 1)] == 0x29,
                           "pushed block teleports to the previous available teleporter");
        game.advanceTicks(1);
        failures += !check(game.top[at(9, 5)] == 0x0A && game.top[at(8, 5)] == 0x29,
                           "block continues in its direction after arriving on a teleporter");

        auto blockedTemplate = blankLevel();
        blockedTemplate.top[at(1, 1)] = 0x6F;
        blockedTemplate.top[at(2, 1)] = 0x0E;
        blockedTemplate.bottom[at(2, 1)] = 0x31;
        auto templateGame = startWithMapMonsters(blockedTemplate);
        failures += !check(templateGame.move(Direction::east) == MoveResult::blocked &&
                           templateGame.playerPosition == at(1, 1),
                           "directional clone-machine template is not walkable or pushable");
    }
    {
        auto level = blankLevel();
        level.top[at(1, 1)] = 0x6F;
        level.top[at(3, 1)] = 0x29;
        level.top[at(8, 5)] = 0x29;
        level.top[at(3, 2)] = 0x40; // bug faces north; first tries west, then north
        level.top[at(2, 2)] = 0x01;
        auto game = startWithMapMonsters(level);
        game.gameTicks = 1;
        game.advanceTicks(1);
        failures += !check(game.creatures[0].position == at(8, 4),
                           "creature exits a paired teleporter and continues in its direction");

        auto sequentialPortalLevel = blankLevel();
        sequentialPortalLevel.top[at(1, 1)] = 0x6F;
        sequentialPortalLevel.top[at(5, 5)] = 0x29; // Entry portal.
        sequentialPortalLevel.top[at(10, 5)] = 0x29; // Second available north exit.
        sequentialPortalLevel.top[at(15, 5)] = 0x29; // First candidate in reverse list order.
        sequentialPortalLevel.top[at(4, 5)] = 0x4F; // East-facing Tank reaches the entry first.
        sequentialPortalLevel.top[at(5, 6)] = 0x4C; // North-facing Tank enters it second.
        auto sequentialPortals = startWithMapMonsters(sequentialPortalLevel);
        sequentialPortals.gameTicks = 1;
        sequentialPortals.advanceTicks(1);
        failures += !check(sequentialPortals.creatures.size() == 2 &&
                           sequentialPortals.creatures[0].position == at(16, 5) &&
                           sequentialPortals.creatures[1].position == at(10, 4) &&
                           sequentialPortals.creatures[0].teleportedThisTick &&
                           sequentialPortals.creatures[1].teleportedThisTick,
                           "second creature skips the exit claimed by the first teleporter before same-tick SlipLoop steps both exits");

        auto coveredPortalLevel = blankLevel();
        coveredPortalLevel.top[at(1, 1)] = 0x6F;
        coveredPortalLevel.top[at(2, 1)] = 0x29;
        coveredPortalLevel.top[at(6, 1)] = 0x29;
        coveredPortalLevel.top[at(10, 1)] = 0x29;
        coveredPortalLevel.top[at(7, 1)] = 0x49; // ball moves west into the x=6 portal
        auto coveredPortal = startWithMapMonsters(coveredPortalLevel);
        coveredPortal.top[at(2, 1)] = 0x0A;
        coveredPortal.bottom[at(2, 1)] = 0x29;
        coveredPortal.gameTicks = 1;
        coveredPortal.advanceTicks(1);
        failures += !check(coveredPortal.creatures[0].position == at(9, 1),
                           "creature teleporter skips a candidate portal covered by a block");

        auto cloneMachinePortalLevel = blankLevel();
        cloneMachinePortalLevel.top[at(20, 20)] = 0x6F;
        cloneMachinePortalLevel.top[at(5, 5)] = 0x43; // east-facing Bug starts on Ice
        cloneMachinePortalLevel.bottom[at(5, 5)] = 0x0C;
        cloneMachinePortalLevel.top[at(6, 5)] = 0x29; // visible portal over a CloneMachine
        cloneMachinePortalLevel.bottom[at(6, 5)] = 0x31;
        cloneMachinePortalLevel.top[at(10, 10)] = 0x29; // paired portal with an open east exit
        auto cloneMachinePortal = startWithMapMonsters(cloneMachinePortalLevel);
        cloneMachinePortal.slipOrder.push_back({SlipperKind::creature, 0});
        cloneMachinePortal.gameTicks = 1; // SlipLoop runs; MonsterLoop does not.
        cloneMachinePortal.advanceTicks(1);
        failures += !check(cloneMachinePortal.creatures[0].position == at(4, 5) &&
                           cloneMachinePortal.creatures[0].tile == 0x41 &&
                           cloneMachinePortal.slipOrder.empty() &&
                           cloneMachinePortal.top[at(6, 5)] == 0x29 &&
                           cloneMachinePortal.bottom[at(6, 5)] == 0x31,
                           "slipping creature rejects Teleport over CloneMachine before reversing on Ice");

        auto burningExit = blankLevel();
        burningExit.top[at(1, 1)] = 0x6F;
        burningExit.top[at(3, 1)] = 0x29;
        burningExit.top[at(8, 5)] = 0x29;
        burningExit.top[at(12, 8)] = 0x29;
        burningExit.top[at(12, 7)] = 0x04;
        burningExit.top[at(3, 2)] = 0x48; // Ball faces north toward the entry.
        auto ball = startWithMapMonsters(burningExit);
        ball.gameTicks = 1;
        ball.advanceTicks(1);
        failures += !check(!ball.creatures[0].alive && ball.creatures[0].position == at(12, 7),
                           "ball teleports, then burns on the fire tile along its exit path");

        auto bugBurningExit = blankLevel();
        bugBurningExit.top[at(1, 1)] = 0x6F;
        bugBurningExit.top[at(3, 1)] = 0x29;
        bugBurningExit.top[at(8, 5)] = 0x29;
        bugBurningExit.top[at(12, 8)] = 0x29;
        bugBurningExit.top[at(12, 7)] = 0x04;
        bugBurningExit.top[at(3, 2)] = 0x40; // Bug tries west, then north into the entry portal.
        bugBurningExit.top[at(2, 2)] = 0x01;
        auto bugRoutesThroughFire = startWithMapMonsters(bugBurningExit);
        bugRoutesThroughFire.gameTicks = 1;
        bugRoutesThroughFire.advanceTicks(1);
        failures += !check(bugRoutesThroughFire.creatures[0].alive &&
                           bugRoutesThroughFire.creatures[0].position == at(12, 8) &&
                           std::any_of(bugRoutesThroughFire.slipOrder.begin(),
                                       bugRoutesThroughFire.slipOrder.end(), [](const SlipperEntry& slip) {
                                           return slip.kind == SlipperKind::creature && slip.entity == 0;
                                       }),
                           "ordinary Bug teleport accepts a Fire exit, then remains on the portal when Fire blocks its slip");

        auto slidingBugBurningExit = blankLevel();
        slidingBugBurningExit.top[at(1, 1)] = 0x6F;
        slidingBugBurningExit.top[at(3, 1)] = 0x29;
        slidingBugBurningExit.top[at(8, 5)] = 0x29;
        slidingBugBurningExit.top[at(12, 8)] = 0x29;
        slidingBugBurningExit.top[at(12, 7)] = 0x04;
        slidingBugBurningExit.top[at(3, 2)] = 0x40;
        slidingBugBurningExit.bottom[at(3, 2)] = 0x0C;
        auto slidingBugRoutesThroughFire = startWithMapMonsters(slidingBugBurningExit);
        slidingBugRoutesThroughFire.slipOrder.push_back({SlipperKind::creature, 0, 0});
        slidingBugRoutesThroughFire.advanceTicks(1);
        failures += !check(slidingBugRoutesThroughFire.creatures[0].alive &&
                           slidingBugRoutesThroughFire.creatures[0].position == at(12, 8) &&
                           slidingBugRoutesThroughFire.slipOrder.size() == 1,
                           "slipping Bug teleport accepts a Fire exit and retains its blocked portal slipper");

        auto drowningExit = blankLevel();
        drowningExit.top[at(1, 1)] = 0x6F;
        drowningExit.top[at(3, 1)] = 0x29;
        drowningExit.top[at(8, 5)] = 0x29;
        drowningExit.top[at(12, 8)] = 0x29;
        drowningExit.top[at(12, 7)] = 0x03;
        drowningExit.top[at(3, 2)] = 0x48; // Ball teleports north toward the candidate exit.
        auto waterBall = startWithMapMonsters(drowningExit);
        waterBall.gameTicks = 1;
        waterBall.advanceTicks(1);
        failures += !check(!waterBall.creatures[0].alive && waterBall.creatures[0].position == at(12, 7),
                           "ball accepts a water teleporter exit, then drowns while leaving it");

        auto occupiedPortalLevel = blankLevel();
        occupiedPortalLevel.top[at(1, 1)] = 0x6F;
        occupiedPortalLevel.top[at(3, 3)] = 0x29;
        occupiedPortalLevel.top[at(8, 5)] = 0x29;
        occupiedPortalLevel.top[at(12, 8)] = 0x29;
        occupiedPortalLevel.top[at(12, 7)] = 0x01; // invalidate the later portal's north exit
        occupiedPortalLevel.top[at(3, 4)] = 0x4C; // tank moves north into the entry portal
        auto occupiedPortal = startWithMapMonsters(occupiedPortalLevel);
        occupiedPortal.playerPosition = at(8, 5); // Chip occupies the otherwise valid candidate portal
        occupiedPortal.gameTicks = 1;
        occupiedPortal.advanceTicks(1);
        failures += !check(occupiedPortal.creatures[0].position == at(3, 2) &&
                           occupiedPortal.slipOrder.empty(),
                           "creature skips Chip's portal, enters its source portal, and continues sliding");

        auto slidingOccupiedPortalLevel = blankLevel();
        slidingOccupiedPortalLevel.top[at(1, 1)] = 0x6F;
        slidingOccupiedPortalLevel.top[at(8, 5)] = 0x29;
        slidingOccupiedPortalLevel.top[at(2, 4)] = 0x29;
        slidingOccupiedPortalLevel.top[at(3, 4)] = 0x41; // bug already sliding west into the portal
        slidingOccupiedPortalLevel.bottom[at(3, 4)] = 0x14;
        auto slidingOccupiedPortal = startWithMapMonsters(slidingOccupiedPortalLevel);
        slidingOccupiedPortal.playerPosition = at(8, 5);
        slidingOccupiedPortal.slipOrder.push_back({SlipperKind::creature, 0});
        slidingOccupiedPortal.advanceTicks(1);
        failures += !check(slidingOccupiedPortal.creatures[0].position == at(2, 4) &&
                           slidingOccupiedPortal.slipOrder.size() == 1,
                           "slipping creature enters a portal when Chip occupies its only destination");
    }
    {
        // Source TileTable/ChipCanEnterTile admission audit for direct Chip
        // moves. The action may kill/win or alter the tile after admission;
        // those are still accepted moves. Locked doors, hidden walls and
        // non-player sprites remain blocked in this default-inventory setup.
        const auto sourceAllowsEntry = [](std::uint8_t tile, int direction) {
            switch (tile) {
                case 0x06: return direction != 2; // PanelN blocks entry from south.
                case 0x07: return direction != 1; // PanelW blocks entry from east.
                case 0x08: return direction != 0; // PanelS blocks entry from north.
                case 0x09: return direction != 3; // PanelE blocks entry from west.
                case 0x1A: return direction == 0 || direction == 3;
                case 0x1B: return direction == 0 || direction == 1;
                case 0x1C: return direction == 2 || direction == 1;
                case 0x1D: return direction == 2 || direction == 3;
                case 0x30: return direction == 2 || direction == 1;
                case 0x01: case 0x05: case 0x0E: case 0x0F: case 0x10: case 0x11:
                case 0x16: case 0x17: case 0x18: case 0x19: case 0x1F: case 0x20:
                case 0x25: case 0x2C: case 0x31:
                    return false;
                default:
                    return !((tile >= 0x33 && tile <= 0x3F) ||
                             (tile >= 0x6C && tile <= 0x6F));
            }
        };
        constexpr std::array<Direction, 4> directions{
            Direction::north, Direction::east, Direction::south, Direction::west};
        int checkedCases = 0;
        for (unsigned tile = 0; tile <= 0x6B; ++tile) {
            for (int direction = 0; direction < 4; ++direction) {
                auto level = blankLevel();
                const int source = at(10, 10);
                constexpr std::array<int, 4> creatureDirectionCodes{0, 3, 2, 1};
                const int destination = neighborForMonsterDirection(
                    source, creatureDirectionCodes[static_cast<std::size_t>(direction)]);
                level.top[static_cast<std::size_t>(source)] = 0x6F;
                level.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(tile);
                auto game = startWithMapMonsters(level);
                const auto expected = sourceAllowsEntry(static_cast<std::uint8_t>(tile), direction);
                (void)game.move(directions[static_cast<std::size_t>(direction)]);
                const bool actuallyEntered = game.playerPosition == destination || game.outcome != Outcome::playing;
                failures += !check(actuallyEntered == expected,
                    "ChipCanEnterTile source admission for tile " + std::to_string(tile) +
                    ", direction " + std::to_string(direction));
                ++checkedCases;
            }
        }
        failures += !check(checkedCases == 4 * 0x6C,
                           "Chip movement admission matrix covers all four directions and valid tile IDs");

        // Transparent creature/key/boot sprites recurse into the lower layer
        // for ChipCanEnterTile admission. Exercise that recursive path against
        // every terrain ID for all four entry directions. Block-as-lower is
        // excluded because the source recursively reads that same lower cell;
        // it is not a valid terrain underlay for these transparent sprites.
        int layeredCases = 0;
        for (unsigned sprite = 0x40; sprite <= 0x6B; ++sprite) {
            for (unsigned lower = 0; lower <= 0x32; ++lower) {
                if (lower == 0x0A) continue;
                for (int direction = 0; direction < 4; ++direction) {
                    auto level = blankLevel();
                    const int source = at(10, 10);
                    constexpr std::array<int, 4> creatureDirectionCodes{0, 3, 2, 1};
                    const int destination = neighborForMonsterDirection(
                        source, creatureDirectionCodes[static_cast<std::size_t>(direction)]);
                    level.top[static_cast<std::size_t>(source)] = 0x6F;
                    level.top[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(sprite);
                    level.bottom[static_cast<std::size_t>(destination)] = static_cast<std::uint8_t>(lower);
                    auto game = startWithMapMonsters(level);
                    const bool expected = sourceAllowsEntry(
                        static_cast<std::uint8_t>(lower), direction);
                    (void)game.move(directions[static_cast<std::size_t>(direction)]);
                    const bool actuallyEntered = game.playerPosition == destination ||
                        game.outcome != Outcome::playing;
                    failures += !check(actuallyEntered == expected,
                        "transparent sprite 0x" + std::to_string(sprite) +
                        " over lower tile 0x" + std::to_string(lower) +
                        " follows recursive ChipCanEnterTile rules in direction " +
                        std::to_string(direction));
                    if (expected && sprite >= 0x64) {
                        const bool collected = sprite <= 0x67
                            ? game.keys[static_cast<std::size_t>(sprite - 0x64)] == 1
                            : game.boots[static_cast<std::size_t>(sprite - 0x68)];
                        failures += !check(collected && game.outcome == Outcome::playing,
                            "accepted key/boot sprite collects without applying its lower terrain action");
                    }
                    if (expected && sprite >= 0x40 && sprite <= 0x63)
                        failures += !check(game.outcome == Outcome::dead,
                            "accepted creature sprite retains its collision action after lower recursion");
                    ++layeredCases;
                }
            }
        }
        failures += !check(layeredCases == 0x2C * (0x33 - 1) * 4,
                           "transparent-sprite lower-layer matrix covers all terrain and entry headings");
    }

    {
        const auto campaign = loadLevelPack(CHIPS_TEST_PACK_PATH);
        const auto level = std::find_if(campaign.begin(), campaign.end(),
            [](const Level& candidate) { return candidate.number == 2; });
        failures += !check(level != campaign.end(), "canonical campaign contains Lesson 2");
        if (level != campaign.end()) {
            auto game = GameState::start(*level, 0, 1);
            const auto step = [&game](Direction direction) {
                (void)game.pressMove(direction);
                game.advanceTicks(2);
            };
            // Move the eastern Level 2 block north, then push it west into the
            // actual water band at (17,11), matching the source MoveBlock path.
            for (const auto direction : {Direction::south, Direction::west, Direction::north,
                                         Direction::east, Direction::north,
                                         Direction::west, Direction::west, Direction::west})
                step(direction);
            const int bridge = at(17, 11);
            failures += !check(game.outcome == Outcome::playing && game.playerPosition == at(18, 11) &&
                               game.top[static_cast<std::size_t>(bridge)] == 0x0B &&
                               game.bottom[static_cast<std::size_t>(bridge)] == 0x00,
                "the real Lesson 2 water tile becomes Dirt over Floor when the block enters it");
            step(Direction::west);
            failures += !check(game.outcome == Outcome::playing && game.playerPosition == bridge &&
                               game.playerSpriteTile() == 0x6D &&
                               game.top[static_cast<std::size_t>(bridge)] == 0x00 &&
                               game.bottom[static_cast<std::size_t>(bridge)] == 0x00,
                "Chip steps onto the actual Lesson 2 Dirt-over-Floor bridge alive with the land sprite");
            step(Direction::east);
            failures += !check(game.outcome == Outcome::playing && game.playerPosition == at(18, 11) &&
                               game.top[static_cast<std::size_t>(bridge)] == 0x00 &&
                               game.bottom[static_cast<std::size_t>(bridge)] == 0x00,
                "leaving the actual Lesson 2 bridge reveals its source Floor layer");
        }
    }

    {
        const auto campaign = loadLevelPack(CHIPS_TEST_PACK_PATH);
        const auto level = std::find_if(campaign.begin(), campaign.end(),
            [](const Level& candidate) { return candidate.number == 3; });
        failures += !check(level != campaign.end(), "canonical campaign contains Lesson 3");
        if (level != campaign.end()) {
            // These source tile IDs, coordinates, and the four-entry count
            // guard against polished-item art accidentally painting a boot
            // over the lesson's force floors or ice corners.
            const std::array<std::pair<int, std::uint8_t>, 4> expectedBoots{{
                {at(15, 11), 0x6A}, {at(12, 14), 0x69},
                {at(19, 14), 0x6B}, {at(16, 16), 0x68},
            }};
            std::array<unsigned, 4> found{};
            for (std::size_t index = 0; index < level->top.size(); ++index) {
                for (const auto tile : {level->top[index], level->bottom[index]}) {
                    if (tile >= 0x68 && tile <= 0x6B) {
                        ++found[static_cast<std::size_t>(tile - 0x68)];
                        const auto expected = std::find_if(expectedBoots.begin(), expectedBoots.end(),
                            [index, tile](const auto& entry) {
                                return static_cast<std::size_t>(entry.first) == index && entry.second == tile;
                            });
                        failures += !check(expected != expectedBoots.end(),
                            "Lesson 3 contains no boot sprite outside its four canonical coordinates");
                    }
                }
            }
            for (const auto& [position, tile] : expectedBoots) {
                failures += !check(level->top[static_cast<std::size_t>(position)] == tile,
                    "Lesson 3 canonical boot ID is at its decoded source coordinate");
            }
            failures += !check(found == std::array<unsigned, 4>{1, 1, 1, 1},
                "Lesson 3 contains exactly one pickup of each canonical boot ID");
        }
    }

    {
        const auto campaign = loadLevelPack(CHIPS_TEST_PACK_PATH);
        const auto level = std::find_if(campaign.begin(), campaign.end(),
            [](const Level& candidate) { return candidate.number == 9; });
        failures += !check(level != campaign.end(), "canonical campaign contains Nuts and Bolts");
        if (level != campaign.end()) {
            struct MoveCase {
                int x;
                int y;
                Direction direction;
                int expectedX;
                int expectedY;
                const char* label;
            };
            constexpr MoveCase cases[]{
                {3, 7, Direction::north, 3, 6, "single Up press at Level 9 (3,7)"},
                {7, 5, Direction::west, 6, 5, "single Left press at Level 9 (7,5)"},
            };
            for (const auto& moveCase : cases) {
                auto game = GameState::start(*level);
                const int source = at(moveCase.x, moveCase.y);
                const int destination = at(moveCase.expectedX, moveCase.expectedY);
                game.playerPosition = source;
                game.chipHasMoved = false;
                game.pendingDirection.reset();
                game.sliding = false;
                failures += !check(game.surface(source) == 0x2D &&
                                   game.surface(destination) == 0x2D,
                    std::string(moveCase.label) + " uses the canonical Gravel cells");
                const auto result = game.pressMove(moveCase.direction);
                failures += !check(result == MoveResult::moved &&
                                   game.playerPosition == destination &&
                                   !game.pendingDirection,
                    std::string(moveCase.label) + " changes exactly one intended map cell");
            }

            for (const auto& moveCase : cases) {
                const int source = at(moveCase.x, moveCase.y);
                const int destination = at(moveCase.expectedX, moveCase.expectedY);
                for (std::uint32_t tickPhase = 0; tickPhase < 4; ++tickPhase) {
                    auto game = GameState::start(*level, tickPhase);
                    game.playerPosition = source;
                    game.chipHasMoved = false;
                    game.pendingDirection.reset();
                    game.sliding = false;
                    const auto result = game.pressMove(moveCase.direction);
                    failures += !check(result == MoveResult::moved &&
                                       game.playerPosition == destination,
                        std::string(moveCase.label) + " immediate key event is one cell at tick phase " +
                            std::to_string(tickPhase));
                    game.advanceTicks(2);
                    failures += !check(game.playerPosition == destination,
                        std::string(moveCase.label) + " nearby creature turns do not displace Chip at tick phase " +
                            std::to_string(tickPhase));
                }
            }

            for (const auto& moveCase : cases) {
                const int source = at(moveCase.x, moveCase.y);
                const int destination = at(moveCase.expectedX, moveCase.expectedY);
                for (std::uint32_t tickPhase = 0; tickPhase < 4; ++tickPhase) {
                    auto game = GameState::start(*level, tickPhase);
                    game.playerPosition = source;
                    game.chipHasMoved = true; // Physical press arrives inside a closed turn.
                    game.pendingDirection.reset();
                    game.sliding = false;
                    game.creatures.clear();
                    game.slipOrder.clear();
                    const auto result = game.pressMove(moveCase.direction);
                    failures += !check(result == MoveResult::ignored &&
                                       game.playerPosition == source &&
                                       game.pendingDirection == moveCase.direction,
                        std::string(moveCase.label) + " buffers one press while its turn is closed");
                    game.advanceTicks(4);
                    failures += !check(game.playerPosition == destination && !game.pendingDirection,
                        std::string(moveCase.label) + " buffered press moves exactly one cell across tick phase " +
                            std::to_string(tickPhase));
                }
            }
        }
    }

    if (failures != 0) {
        std::cerr << failures << " engine checks failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All engine checks passed.\n";
    return EXIT_SUCCESS;
}
