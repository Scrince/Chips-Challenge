#include "engine.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <utility>

namespace chips {
namespace {
constexpr std::array<int, 4> kDx{0, 1, 0, -1};
constexpr std::array<int, 4> kDy{-1, 0, 1, 0};
constexpr std::array<int, 4> kMonsterDx{0, -1, 0, 1}; // N, W, S, E tile encoding
constexpr std::array<int, 4> kMonsterDy{-1, 0, 1, 0};
std::size_t dirIndex(Direction direction) {
    return static_cast<std::size_t>(direction);
}

int neighbor(int position, Direction direction) {
    const int x = position % kBoardWidth + kDx[dirIndex(direction)];
    const int y = position / kBoardWidth + kDy[dirIndex(direction)];
    if (x < 0 || x >= kBoardWidth || y < 0 || y >= kBoardHeight) return -1;
    return y * kBoardWidth + x;
}

int colorIndex(std::uint8_t tile, std::uint8_t first) {
    return static_cast<int>(tile - first);
}

std::uint16_t readLittleWord(const std::uint8_t* bytes) {
    return static_cast<std::uint16_t>(bytes[0] | (static_cast<unsigned>(bytes[1]) << 8));
}

Direction cornerDirection(std::uint8_t tile, Direction direction) {
    // The four ice-corner tiles redirect only the two directions that enter
    // the corner's open mouth; other approaches reverse.
    switch (tile) {
        case 0x1A:
            if (direction == Direction::west) return Direction::south;
            if (direction == Direction::north) return Direction::east;
            break;
        case 0x1B:
            if (direction == Direction::north) return Direction::west;
            if (direction == Direction::east) return Direction::south;
            break;
        case 0x1C:
            if (direction == Direction::south) return Direction::west;
            if (direction == Direction::east) return Direction::north;
            break;
        case 0x1D:
            if (direction == Direction::west) return Direction::north;
            if (direction == Direction::south) return Direction::east;
            break;
        default: break;
    }
    return static_cast<Direction>((static_cast<unsigned>(direction) + 2U) % 4U);
}

Direction randomForceDirection(std::uint32_t& state) {
    state = state * 214013U + 2531011U; // 16-bit Microsoft CRT rand state
    const auto value = (state >> 16U) & 0x7FFFU;
    constexpr std::array<Direction, 4> directions{
        Direction::north, Direction::south, Direction::west, Direction::east};
    return directions[value % directions.size()];
}

Direction blockSlideDirection(std::uint8_t tile, Direction incoming,
                              std::uint32_t& randomState) {
    switch (tile) {
        case 0x0D: return Direction::south;
        case 0x12: return Direction::north;
        case 0x13: return Direction::east;
        case 0x14: return Direction::west;
        case 0x32: return randomForceDirection(randomState);
        default:
            return tile >= 0x1A && tile <= 0x1D
                ? cornerDirection(tile, incoming) : incoming;
    }
}

std::uint32_t nextRandom(std::uint32_t& state) {
    state = state * 214013U + 2531011U;
    return (state >> 16U) & 0x7FFFU;
}

Direction reverseSlideDirection(std::uint8_t terrain, Direction direction) {
    const auto reversed = static_cast<Direction>((static_cast<unsigned>(direction) + 2U) % 4U);
    return terrain >= 0x1A && terrain <= 0x1D ? cornerDirection(terrain, reversed) : reversed;
}

bool isPanelOrIceWall(std::uint8_t tile) {
    return (tile >= 0x06 && tile <= 0x09) || (tile >= 0x1A && tile <= 0x1D) || tile == 0x30;
}

bool isOneWayPanel(std::uint8_t tile) {
    return (tile >= 0x06 && tile <= 0x09) || tile == 0x30;
}

bool panelAllows(std::uint8_t tile, Direction movement, bool entering) {
    // This table mirrors CheckPanelWalls' separate entering (flag=1) and
    // exiting (flag=0) paths for the original four panels and PanelSE.
    switch (tile) {
        case 0x06: return entering ? movement != Direction::south : movement != Direction::north;
        case 0x07: return entering ? movement != Direction::east : movement != Direction::west;
        case 0x08: return entering ? movement != Direction::north : movement != Direction::south;
        case 0x09: return entering ? movement != Direction::west : movement != Direction::east;
        case 0x1A: return entering ? (movement == Direction::north || movement == Direction::west)
                                   : (movement == Direction::south || movement == Direction::east);
        case 0x1B: return entering ? (movement == Direction::north || movement == Direction::east)
                                   : (movement == Direction::south || movement == Direction::west);
        case 0x1C: return entering ? (movement == Direction::south || movement == Direction::east)
                                   : (movement == Direction::north || movement == Direction::west);
        case 0x1D: return entering ? (movement == Direction::south || movement == Direction::west)
                                   : (movement == Direction::north || movement == Direction::east);
        case 0x30: return entering ? (movement == Direction::south || movement == Direction::east)
                                   : (movement == Direction::north || movement == Direction::west);
        default: return true;
    }
}

bool blockMayEnterTile(std::uint8_t tile) {
    return tile == 0x00 || tile == 0x03 || tile == 0x04 || tile == 0x0C || tile == 0x0D ||
        (tile >= 0x12 && tile <= 0x15) ||
        (tile >= 0x23 && tile <= 0x24) || tile == 0x26 ||
        (tile >= 0x27 && tile <= 0x2B) || tile == 0x2D || tile == 0x2F ||
        tile == 0x32 || (tile >= 0x3C && tile <= 0x3F) ||
        (tile >= 0x64 && tile <= 0x6F) || isPanelOrIceWall(tile);
}

Direction monsterDirection(int code) {
    constexpr std::array<Direction, 4> directions{
        Direction::north, Direction::west, Direction::south, Direction::east};
    return directions[static_cast<std::size_t>(code & 3)];
}

int monsterCode(Direction direction) {
    switch (direction) {
        case Direction::north: return 0;
        case Direction::west: return 1;
        case Direction::south: return 2;
        case Direction::east: return 3;
    }
    return 0;
}

bool isSlipperyTerrain(std::uint8_t tile) {
    return tile == 0x0C || tile == 0x0D || (tile >= 0x12 && tile <= 0x14) ||
        (tile >= 0x1A && tile <= 0x1D) || tile == 0x32;
}

bool isCreatureSlipTerrain(std::uint8_t tile) {
    // Teleporters preserve a creature's current SlideMovement. Traps use a
    // separate MoveMonster action that removes its slip entry after entry.
    // MonsterCanEnterTile blocks ForceRandom (0x32) for every creature type.
    return (isSlipperyTerrain(tile) && tile != 0x32) || tile == 0x29;
}

bool isCreatureOverlayTile(std::uint8_t tile) {
    // Only Chip's directional and swimming sprites read through to Lower.
    // The collectible ICChip tile (0x02) is a solid tile for monsters.
    return (tile >= 0x3C && tile <= 0x3F) || (tile >= 0x6C && tile <= 0x6F);
}

bool isPlayerSpriteTile(std::uint8_t tile) {
    return (tile >= 0x3C && tile <= 0x3F) || (tile >= 0x6C && tile <= 0x6F);
}

bool creatureMayEnterTile(std::uint8_t creatureKind, std::uint8_t tile, Direction direction) {
    if (isPanelOrIceWall(tile)) return panelAllows(tile, direction, true);
    // MonsterCanEnterTile reports water as enterable for every creature; its
    // action then drowns non-gliders. Callers that actually move a creature
    // resolve that action separately, while clone/teleport validation only
    // asks whether the candidate exit is accepted.
    if (tile == 0x03) return true;
    if (tile == 0x04) return creatureKind != 0x40 && creatureKind != 0x58;
    return tile == 0x00 || tile == 0x0C || tile == 0x0D ||
        (tile >= 0x12 && tile <= 0x14) || tile == 0x23 || tile == 0x24 ||
        tile == 0x26 || tile == 0x27 || tile == 0x28 || tile == 0x29 ||
        tile == 0x2A || tile == 0x2B || tile == 0x2F ||
        (tile >= 0x3C && tile <= 0x3F) || (tile >= 0x6C && tile <= 0x6F) ||
        (tile >= 0x64 && tile <= 0x67);
}

bool creatureCanCatchPlayer(std::uint8_t creatureKind, std::uint8_t playerFloor,
                            Direction direction) {
    // A teleporter collision is resolved by its portal-routing path before
    // MonsterLoop/SlipLoop performs the post-move Chip-sprite autopsy.
    if (playerFloor == 0x29 || !creatureMayEnterTile(creatureKind, playerFloor, direction))
        return false;
    // MoveMonster applies Water, Fire, and Bomb actions before its autopsy.
    // Only a glider survives Water, only a fireball survives Fire, and every
    // creature is destroyed by a bomb before Chip's sprite can be eaten.
    if (playerFloor == 0x03) return creatureKind == 0x50;
    if (playerFloor == 0x04) return creatureKind == 0x44;
    if (playerFloor == 0x2A) return false;
    return true;
}
}

GameState GameState::start(const Level& source, std::uint32_t initialGameTicks,
                           std::uint32_t initialRandomState) {
    GameState state;
    state.gameTicks = initialGameTicks & 0xFFFFU;
    state.randomState = initialRandomState;
    state.blockDirections.fill(-1);
    state.level = &source;
    state.top = source.top;
    state.bottom = source.bottom;
    for (int position = 0; position < kBoardCells; ++position) {
        const auto upper = source.top[static_cast<std::size_t>(position)];
        // InitBoard scans below creature sprites when collecting teleports and
        // toggle tiles, whether or not field 10 registered that sprite live.
        const auto boardTile = upper >= 0x40 && upper <= 0x63
            ? source.bottom[static_cast<std::size_t>(position)] : upper;
        if (boardTile == 0x29) state.teleports.push_back(position);
        if (boardTile == 0x25 || boardTile == 0x26)
            state.togglePositions.push_back(position);
    }
    state.chipsLeft = source.chipsRequired;
    state.secondsLeft = source.timeLimit;
    state.outcome = Outcome::playing;
    state.message = "Collect the required chips and reach the exit.";
    for (const auto& field : source.fields) {
        if (field.id == 4) {
            if (field.raw.size() % 10 != 0) {
                throw LevelPackError("Malformed trap-link field in level " + std::to_string(source.number));
            }
            for (std::size_t offset = 0; offset < field.raw.size(); offset += 10) {
                const auto* record = field.raw.data() + offset;
                const auto buttonX = readLittleWord(record);
                const auto buttonY = readLittleWord(record + 2);
                const auto trapX = readLittleWord(record + 4);
                const auto trapY = readLittleWord(record + 6);
                if (buttonX >= kBoardWidth || buttonY >= kBoardHeight ||
                    trapX >= kBoardWidth || trapY >= kBoardHeight) {
                    throw LevelPackError("Trap-link coordinate outside 32x32 board in level " +
                                         std::to_string(source.number));
                }
                state.traps.push_back({static_cast<int>(buttonY * kBoardWidth + buttonX),
                                       static_cast<int>(trapY * kBoardWidth + trapX),
                                       readLittleWord(record + 8) != 0});
            }
        } else if (field.id == 10) {
            if (field.raw.size() % 2 != 0) {
                throw LevelPackError("Malformed initial-creature field in level " +
                                     std::to_string(source.number));
            }
            for (std::size_t offset = 0; offset < field.raw.size(); offset += 2) {
                const auto x = field.raw[offset];
                const auto y = field.raw[offset + 1];
                if (x >= kBoardWidth || y >= kBoardHeight) {
                    throw LevelPackError("Initial-creature coordinate outside 32x32 board in level " +
                                         std::to_string(source.number));
                }
                const int position = static_cast<int>(y * kBoardWidth + x);
                const auto tile = source.top[static_cast<std::size_t>(position)];
                if (tile < 0x40 || tile > 0x63 ||
                    source.bottom[static_cast<std::size_t>(position)] == 0x31)
                    continue; // NewMonster ignores stale and clone-machine starts.
                state.creatures.push_back({tile, position, true,
                    static_cast<std::int8_t>((tile & 0xFC) == 0x4C ? tile & 0x03 : -1),
                    static_cast<std::int8_t>(tile & 0x03)});
            }
        } else if (field.id == 5) {
            if (field.raw.size() % 8 != 0) {
                throw LevelPackError("Malformed clone-link field in level " + std::to_string(source.number));
            }
            for (std::size_t offset = 0; offset < field.raw.size(); offset += 8) {
                const auto* record = field.raw.data() + offset;
                const auto buttonX = readLittleWord(record);
                const auto buttonY = readLittleWord(record + 2);
                const auto machineX = readLittleWord(record + 4);
                const auto machineY = readLittleWord(record + 6);
                if (buttonX >= kBoardWidth || buttonY >= kBoardHeight ||
                    machineX >= kBoardWidth || machineY >= kBoardHeight) {
                    throw LevelPackError("Clone-link coordinate outside 32x32 board in level " +
                                         std::to_string(source.number));
                }
                const int machinePosition = static_cast<int>(machineY * kBoardWidth + machineX);
                state.cloneLinks.push_back({static_cast<int>(buttonY * kBoardWidth + buttonX),
                                            machinePosition,
                                            source.top[static_cast<std::size_t>(machinePosition)]});
            }
        }
    }
    bool foundPlayer = false;
    int firstPlayerPosition = -1;
    for (int i = 0; i < kBoardCells; ++i) {
        auto& tile = state.top[static_cast<std::size_t>(i)];
        if (tile >= 0x6C && tile <= 0x6F) {
            if (foundPlayer) {
                throw LevelPackError("Level " + std::to_string(source.number) +
                    " has multiple player start tiles at cells " + std::to_string(firstPlayerPosition) +
                    " and " + std::to_string(i));
            }
            state.playerPosition = i;
            firstPlayerPosition = i;
            state.facing = tile == 0x6C ? Direction::north
                         : tile == 0x6D ? Direction::west
                         : tile == 0x6E ? Direction::south : Direction::east;
            tile = state.bottom[static_cast<std::size_t>(i)];
            foundPlayer = true;
        } else if (tile >= 0x40 && tile <= 0x63) {
            const bool registered = std::any_of(state.creatures.begin(), state.creatures.end(),
                [i](const Creature& creature) { return creature.position == i; });
            if (registered) {
                const auto underlay = state.bottom[static_cast<std::size_t>(i)];
                tile = underlay;
                if (underlay >= 0x6C && underlay <= 0x6F) {
                    if (foundPlayer) {
                        throw LevelPackError("Level " + std::to_string(source.number) +
                            " has multiple player start tiles at cells " +
                            std::to_string(firstPlayerPosition) + " and " + std::to_string(i));
                    }
                    state.playerPosition = i;
                    firstPlayerPosition = i;
                    state.facing = underlay == 0x6C ? Direction::north
                                 : underlay == 0x6D ? Direction::west
                                 : underlay == 0x6E ? Direction::south : Direction::east;
                    state.pendingAutopsy = true;
                    state.playerCoveredByCreature = true;
                    foundPlayer = true;
                }
            }
        }
    }
    if (!foundPlayer) {
        state.outcome = Outcome::dead;
        state.deathCause = DeathCause::other;
        state.message = "This level has no player start tile.";
    }
    // InitBoard creates every registered monster before scanning the board.
    // NewMonster calls EnterTrap immediately for a monster whose lower tile is
    // a trap, with (-1,-1) as its source. Apply those initial link-state updates
    // only after all optional fields have been decoded, since field order is
    // not significant in CHIPS.DAT.
    for (const auto& creature : state.creatures) {
        if (state.bottom[static_cast<std::size_t>(creature.position)] == 0x2B)
            state.enterLinkedTrap(creature.position, -1);
    }
    return state;
}

std::uint8_t GameState::surface(int position) const {
    if (position < 0 || position >= kBoardCells) return 0x01;
    for (const auto& creature : creatures) {
        if (creature.alive && creature.position == position) return creature.tile;
    }
    return top[static_cast<std::size_t>(position)];
}

std::uint8_t GameState::floor(int position) const {
    if (position < 0 || position >= kBoardCells) return 0x01;
    const auto index = static_cast<std::size_t>(position);
    // MoveBlock copies Chip's visible sprite into Lower when a clone block
    // lands on Chip. MoveChip reads that lower sprite when leaving the square.
    if (position == playerPosition && top[index] == 0x0A && isPlayerSpriteTile(bottom[index]))
        return bottom[index];
    const auto tile = surface(position);
    if (tile >= 0x40 && tile <= 0x6F) return bottom[index];
    return tile;
}

bool GameState::standingOnHint() const {
    return outcome == Outcome::playing && floor(playerPosition) == 0x2F;
}

std::uint8_t GameState::playerSpriteTile() const {
    const auto index = static_cast<std::size_t>(playerPosition);
    const auto storedSprite = bottom[index];
    const bool swimming = top[index] == 0x03 || storedSprite == 0x03 ||
        (storedSprite >= 0x3C && storedSprite <= 0x3F);
    return chipSpriteTile(facing, swimming);
}

std::uint8_t GameState::playerDisplayTile() const {
    if (outcome == Outcome::dead) {
        if (deathCause == DeathCause::water) return 0x33; // ChipSplash
        if (deathCause == DeathCause::fire) return 0x34;  // ChipBurned
    }
    return playerSpriteTile();
}

bool GameState::playerHasBoot(unsigned boot) const {
    return boot < boots.size() && boots[boot];
}

void GameState::queueMove(Direction direction) {
    if (outcome == Outcome::playing && !pendingDirection) {
        mouseTargetPosition.reset();
        pendingDirection = direction;
    }
}

MoveResult GameState::pressMove(Direction direction) {
    if (outcome != Outcome::playing) return MoveResult::ignored;
    if (chipHasMoved) {
        if (!pendingDirection) pendingDirection = direction;
        return MoveResult::ignored;
    }
    pendingDirection.reset();
    mouseTargetPosition.reset();
    chipHasMoved = true;
    playerAttemptedThisTick = true;
    playerMovedThisTick = false;
    playerTeleportedThisTick = false;
    switchActivatedThisTick = false;
    doorOpenedThisTick = false;
    thiefActivatedThisTick = false;
    splashThisTick = false;
    playerPickupThisTick = PickupKind::none;
    return moveInternal(direction, true);
}

void GameState::setMouseTarget(int position) {
    if (outcome != Outcome::playing || position < 0 || position >= kBoardCells) return;
    pendingDirection.reset();
    mouseTargetPosition = position;
}

void GameState::advanceMouseTarget() {
    if (!mouseTargetPosition) return;
    if (outcome != Outcome::playing) {
        mouseTargetPosition.reset();
        return;
    }

    const int dx = *mouseTargetPosition % kBoardWidth - playerPosition % kBoardWidth;
    const int dy = *mouseTargetPosition / kBoardWidth - playerPosition / kBoardWidth;
    if (dx == 0 && dy == 0) {
        mouseTargetPosition.reset();
        chipHasMoved = false;
        return;
    }

    const auto axisDirection = [](int delta, bool vertical) {
        if (vertical) return delta > 0 ? Direction::south : Direction::north;
        return delta > 0 ? Direction::east : Direction::west;
    };
    const bool verticalFirst = std::abs(dy) >= std::abs(dx);
    const Direction first = axisDirection(verticalFirst ? dy : dx, verticalFirst);
    const auto attemptMouseStep = [this](Direction direction) {
        if (!sliding) return moveInternal(direction);

        const auto terrain = floor(playerPosition);
        const bool iceOrPortal = terrain == 0x0C || (terrain >= 0x1A && terrain <= 0x1D) || terrain == 0x29;
        if (iceOrPortal || direction == slideDirection) return MoveResult::blocked;

        const bool wasSliding = sliding;
        const auto oldSlideDirection = slideDirection;
        const auto oldFacing = facing;
        sliding = false;
        const auto result = moveInternal(direction);
        if (result == MoveResult::blocked) {
            sliding = wasSliding;
            slideDirection = oldSlideDirection;
            facing = oldFacing;
        }
        return result;
    };

    playerAttemptedThisTick = true;
    auto result = attemptMouseStep(first);
    if (result == MoveResult::blocked && mouseTargetPosition && outcome == Outcome::playing) {
        const bool horizontalFirst = !verticalFirst;
        const int secondaryDelta = horizontalFirst ? dy : dx;
        if (secondaryDelta == 0) {
            mouseTargetPosition.reset();
        } else {
            chipHasMoved = false;
            result = attemptMouseStep(axisDirection(secondaryDelta, horizontalFirst));
            if (result == MoveResult::blocked) mouseTargetPosition.reset();
        }
    }

    if (!mouseTargetPosition || outcome != Outcome::playing || playerPosition == *mouseTargetPosition)
        mouseTargetPosition.reset();
    // The source mouse-target path clears ChipHasMoved after its attempt so it
    // can advance again on the next tick independently of keyboard buffering.
    chipHasMoved = false;
}

void GameState::advanceTicks(unsigned ticks) {
    for (unsigned i = 0; i < ticks && outcome == Outcome::playing; ++i) {
        playerAttemptedThisTick = false;
        playerMovedThisTick = false;
        playerTeleportedThisTick = false;
        switchActivatedThisTick = false;
        doorOpenedThisTick = false;
        thiefActivatedThisTick = false;
        splashThisTick = false;
        playerPickupThisTick = PickupKind::none;
        for (auto& creature : creatures) creature.teleportedThisTick = false;
        gameTicks = (gameTicks + 1U) & 0xFFFFU; // Original CurrentTick is a wrapping 16-bit word.
        const bool oddTick = gameTicks % 2 != 0;
        bool tryMouseBeforeMonsters = false;
        if (oddTick) {
            // DoTick services buffered keyboard input on odd ticks when Chip
            // is not already sliding or marked as having moved this turn.
            if (!sliding && !chipHasMoved && pendingDirection) {
                const auto direction = *pendingDirection;
                pendingDirection.reset();
                chipHasMoved = true;
                playerAttemptedThisTick = true;
                (void)move(direction);
            }
        } else if (!sliding) {
            if (chipHasMoved) {
                chipHasMoved = false;
            } else if (pendingDirection) {
                const auto direction = *pendingDirection;
                pendingDirection.reset();
                chipHasMoved = true;
                playerAttemptedThisTick = true;
                (void)move(direction);
            } else {
                // DoTick's idle branch runs only on even ticks when Chip is
                // stationary and has no buffered input. The original checks
                // for a mouse target before applying its idle-facing update.
                if (mouseTargetPosition) tryMouseBeforeMonsters = true;
                else {
                    if (idleTickCount >= 2) facing = Direction::south;
                    ++idleTickCount;
                }
            }
        }
        if (tryMouseBeforeMonsters) advanceMouseTarget();
        // DoTick's even-tick branch calls MonsterLoop; its odd-tick branch
        // skips directly to Chip sliding. Creature turns therefore occur on
        // alternating simulation ticks. Teeth and Blobs add their tick%4 gate.
        // DoTick calls MonsterLoop on even ticks even if the buffered player
        // move earlier in this same tick set Autopsy. MonsterLoop itself also
        // continues its captured creature range after one monster catches Chip.
        if (!oddTick) {
            moveCreatures();
            if (pendingAutopsy) {
                pendingAutopsy = false;
                if (outcome == Outcome::playing) {
                    playerCoveredByCreature = true;
                    kill(Outcome::dead, "A creature caught you.");
                }
            }
        }
        const bool enteredChipSlidePhase = sliding;
        bool skipMouseMovementAfterSlide = false;
        if (sliding) {
            if (outcome == Outcome::playing) (void)moveInternal(slideDirection, false);
            else if (outcome == Outcome::dead) (void)moveSlideAfterDeath(slideDirection);
            // DoTick tries one buffered move immediately after Chip's slide
            // step. A matching input is redundant while the slide continues;
            // a different input can steer from the newly occupied square.
            if (chipHasMoved) {
                // The original clears this gate after Chip's slide phase and
                // skips the buffered-key and mouse movement phases entirely.
                chipHasMoved = false;
                skipMouseMovementAfterSlide = true;
            } else if (pendingDirection && outcome == Outcome::playing) {
                const auto requested = *pendingDirection;
                pendingDirection.reset();
                const auto surface = floor(playerPosition);
                const bool forcedFloor = surface == 0x0D ||
                    (surface >= 0x12 && surface <= 0x14) || surface == 0x32;
                const bool canSteerForcedFloor = sliding && forcedFloor &&
                    requested != slideDirection;
                if (!sliding || canSteerForcedFloor) {
                    const auto continuedDirection = slideDirection;
                    const auto continuedFacing = facing;
                    const bool resumeSlideIfBlocked = sliding;
                    if (resumeSlideIfBlocked) sliding = false;
                    const auto result = move(requested);
                    if (resumeSlideIfBlocked && result == MoveResult::blocked &&
                        outcome == Outcome::playing) {
                        sliding = true;
                        slideDirection = continuedDirection;
                        facing = continuedFacing;
                    }
                    chipHasMoved = true;
                    playerAttemptedThisTick = true;
                }
            }
        }
        if (enteredChipSlidePhase && !skipMouseMovementAfterSlide &&
            !pendingDirection && mouseTargetPosition &&
            outcome == Outcome::playing)
            advanceMouseTarget();
        // SlipLoop runs every tick, after Chip's own sliding movement.
        // SlipLoop is the final movement phase and still runs on a tick where
        // MonsterLoop recorded Chip's death; only subsequent ticks are halted.
        moveBlocks();
        // The original timer runs after movement and SlipLoop even when that
        // same tick recorded a win or a hazard death. At zero, timeout takes
        // precedence over the movement result for this tick.
        // DoTick explicitly skips countdown when its wrapping 16-bit tick is
        // zero. Without this guard, the wrap at 65535 -> 0 would consume an
        // extra second because zero is also divisible by ten.
        if (gameTicks != 0 && gameTicks % 10 == 0 && secondsLeft > 0) {
            --secondsLeft;
            ++secondsElapsed;
            if (secondsLeft == 0) kill(Outcome::timedOut, "Time ran out.");
        }
    }
}

bool GameState::passableFloor(std::uint8_t tile) const {
    // This is ChipCanEnterTile's ordinary TileTable eligibility set. Special
    // actions (hazards, doors, fake/hidden/popup terrain and panels) are
    // resolved by tryMove before/after this predicate; unlisted IDs are solid.
    // Socket (0x22) is conditionally enterable once the required chips are
    // collected; the original .socket action rejects entry while any remain.
    if (tile == 0x22) return chipsLeft == 0;
    return tile == 0x00 || tile == 0x02 || tile == 0x03 || tile == 0x04 ||
        (tile >= 0x06 && tile <= 0x0D) ||
        (tile >= 0x12 && tile <= 0x1F) ||
        (tile >= 0x21 && tile <= 0x24) || tile == 0x26 ||
        (tile >= 0x27 && tile <= 0x30) || tile == 0x32 ||
        (tile >= 0x64 && tile <= 0x6B);
}

bool GameState::chipCanEnterVisibleTile(int position, Direction direction) {
    if (position < 0 || position >= kBoardCells) return false;
    const auto index = static_cast<std::size_t>(position);
    if (bottom[index] == 0x31) return false; // ChipCanEnterTile rejects clone machines first.
    // EnterTeleport pushes flags in reverse argument order: upperOrLower=1,
    // flag1=0. ChipCanEnterTile therefore validates the visible Upper tile
    // without applying entry actions. The original recursively reads Lower
    // for a Block or transparent item/creature, but Chip's own land/swim
    // sprites remain solid to Chip (their player-table eligibility is zero).
    const auto upperTile = position == playerPosition ? playerSpriteTile() : surface(position);
    const auto tile = upperTile == 0x0A || (upperTile >= 0x40 && upperTile <= 0x6B)
        ? bottom[index] : upperTile;
    if (isPanelOrIceWall(tile) && !panelAllows(tile, direction, true)) return false;
    if (tile >= 0x16 && tile <= 0x19) {
    // This is ChipCanEnterTile with its side-effect flag cleared. EnterTeleport
    // uses that mode for candidate exits, and ordinary transparent pickups use
    // it recursively to validate the tile below the visible item.
    // A door requires its key here, but the actual slide step consumes it
    // later if Chip really enters the visible door tile.
        return keys[static_cast<std::size_t>(tile - 0x16)] != 0;
    }
    if (tile == 0x22) return chipsLeft == 0;
    if (tile == 0x1E) return true; // FakeFloor reveals only on an actual entry.
    if (tile == 0x1F || tile == 0x2C) return false; // Hidden walls reject without revealing.
    if (tile == 0x21) return true; // Thief effects run only on actual entry.
    if (tile == 0x2E) return true; // PopupWall rises only on actual entry.

    // These are the impassable entries in ChipCanEnterTile's column of
    // TileTable, plus animated-only IDs that are not walkable.
    if (tile == 0x01 || tile == 0x05 || (tile >= 0x0E && tile <= 0x11) ||
        tile == 0x1F || tile == 0x20 || tile == 0x25 || tile == 0x2C || tile == 0x31 ||
        (tile >= 0x33 && tile <= 0x3F) || (tile >= 0x6C && tile <= 0x6F))
        return false;

    // All other authored map tiles are enterable by Chip during teleport
    // validation. Their action (including hazards, bombs, popup walls, and
    // chained teleports) runs when the following slide step actually enters.
    return tile == 0x00 || tile == 0x02 || tile == 0x03 || tile == 0x04 ||
        (tile >= 0x06 && tile <= 0x0D) || (tile >= 0x12 && tile <= 0x15) ||
        (tile >= 0x1A && tile <= 0x1E) || tile == 0x21 ||
        (tile >= 0x23 && tile <= 0x24) || tile == 0x26 ||
        (tile >= 0x27 && tile <= 0x30) || tile == 0x32 ||
        (tile >= 0x64 && tile <= 0x6B);
}

void GameState::kill(Outcome reason, std::string why, DeathCause cause) {
    outcome = reason;
    deathCause = reason == Outcome::timedOut ? DeathCause::timeout
        : reason == Outcome::dead ? cause : DeathCause::none;
    mouseTargetPosition.reset();
    message = std::move(why);
}

MoveResult GameState::move(char direction) {
    switch (direction) {
        case 'N': case 'n': case 'U': case 'u': return move(Direction::north);
        case 'E': case 'e': case 'R': case 'r': return move(Direction::east);
        case 'S': case 's': case 'D': case 'd': return move(Direction::south);
        case 'W': case 'w': case 'L': case 'l': return move(Direction::west);
        default: return MoveResult::ignored;
    }
}

MoveResult GameState::move(Direction direction) {
    if (outcome != Outcome::playing) return MoveResult::ignored;
    return moveInternal(direction, true);
}

MoveResult GameState::moveSlideAfterDeath(Direction direction) {
    if (outcome != Outcome::dead || !sliding) return MoveResult::ignored;
    return moveInternal(direction);
}

MoveResult GameState::moveInternal(Direction direction, bool voluntaryInput) {
    idleTickCount = 0;
    const auto startingOutcome = outcome;
    const bool wasSliding = sliding;
    if (wasSliding && voluntaryInput) {
        const auto standingTile = floor(playerPosition);
        const bool onIceOrTeleport = standingTile == 0x0C ||
            (standingTile >= 0x1A && standingTile <= 0x1D) || standingTile == 0x29;
        if (onIceOrTeleport) return MoveResult::ignored;
        const bool onForceFloor = standingTile == 0x0D ||
            (standingTile >= 0x12 && standingTile <= 0x14) || standingTile == 0x32;
        if (onForceFloor && direction == slideDirection) return MoveResult::ignored;
    }
    const auto actualDirection = wasSliding && !voluntaryInput ? slideDirection : direction;
    facing = actualDirection;
    const int previousPosition = playerPosition;
    if (!tryMove(actualDirection)) {
        if (!wasSliding || voluntaryInput) return MoveResult::blocked;
        const auto terrain = floor(playerPosition);
        // DoTick retries every blocked Chip slide once in reverse. Its
        // SlideMovement call then re-applies the current surface: corners
        // redirect, force arrows restore their heading, and ForceRandom draws
        // a fresh direction. Teleporters and traps preserve the reversal.
        const auto reversed = blockSlideDirection(terrain,
            static_cast<Direction>((static_cast<unsigned>(actualDirection) + 2U) % 4U),
            randomState);
        slideDirection = reversed;
        facing = reversed;
        if (!tryMove(reversed)) {
            // DoTick reverses once more and calls SlideMovement even though it
            // does not attempt a third move. Preserve the resulting heading for
            // the next tick (notably, force floors and random floors reapply).
            const auto restored = blockSlideDirection(terrain,
                static_cast<Direction>((static_cast<unsigned>(reversed) + 2U) % 4U),
                randomState);
            slideDirection = restored;
            facing = restored;
            sliding = true;
            return MoveResult::blocked;
        }
    }
    if (playerPosition != previousPosition) playerMovedThisTick = true;
    // A hazard reached by Chip's own MoveChip call reloads the level in the
    // original before DoTick reaches its slide phase. Preserve a slide only
    // when DoTick entered this forced step after MonsterLoop had already
    // recorded Chip's death; that same-tick post-collision slide is source
    // behavior and uses moveSlideAfterDeath (startingOutcome == dead).
    if (startingOutcome == Outcome::playing && outcome == Outcome::dead)
        sliding = false;
    // MoveChip pops the vacated upper tile into the visible layer and resets
    // its lower layer to Floor; at the destination it places the pre-Chip
    // upper tile in Lower. Chip is an explicit position here, so reproduce
    // only those layer writes and leave its rendered overlay to the app.
    const auto previous = static_cast<std::size_t>(previousPosition);
    if (top[previous] == 0x0A && isPlayerSpriteTile(bottom[previous]))
        top[previous] = bottom[previous];
    bottom[previous] = 0x00;
    const auto destination = static_cast<std::size_t>(playerPosition);
    const bool blockCoversChip = top[destination] == 0x0A && isPlayerSpriteTile(bottom[destination]);
    if (!blockCoversChip) bottom[destination] = top[destination];
    if (surface(playerPosition) == 0x2B)
        enterLinkedTrap(playerPosition, previousPosition);
    if (outcome == Outcome::won) return MoveResult::won;
    if (outcome != Outcome::playing) return MoveResult::died;
    // A blocked-slide retry may have changed slideDirection and succeeded in reverse.
    // Carry that live heading into the destination terrain's slide setup.
    const auto completedDirection = wasSliding && !voluntaryInput ? slideDirection : actualDirection;
    const auto result = finishMove(completedDirection, previousPosition);
    return result;
}

void GameState::queueBlockSlip(int position) {
    const auto active = std::find_if(slipOrder.begin(), slipOrder.end(), [position](const SlipperEntry& entry) {
        return entry.kind == SlipperKind::block && entry.entity == position;
    });
    if (active == slipOrder.end()) slipOrder.push_back({SlipperKind::block, position, -1});
}

void GameState::queueCreatureSlip(std::size_t creatureIndex) {
    const auto active = std::find_if(slipOrder.begin(), slipOrder.end(), [creatureIndex](const SlipperEntry& entry) {
        return entry.kind == SlipperKind::creature && entry.entity == static_cast<int>(creatureIndex);
    });
    if (active == slipOrder.end()) {
        const auto& creature = creatures[creatureIndex];
        const int direction = creature.movementDirection >= 0
            ? creature.movementDirection : creature.tile & 0x03;
        slipOrder.push_back({SlipperKind::creature, static_cast<int>(creatureIndex),
                             static_cast<std::int8_t>(direction)});
    }
}

void GameState::moveBlocks() {
    // The source SlipLoop walks blocks and monsters in one insertion-ordered list.
    for (std::size_t slip = 0; slip < slipOrder.size();) {
        const std::size_t previousSlipCount = slipOrder.size();
        const std::size_t previousSlipIndex = slip;
        const auto finishSlipIteration = [&]() {
            // SlipLoop advances its list cursor only when the slipper count is
            // unchanged. A changed count makes it revisit this index, which
            // may now identify a shifted entry or a slipper added by a button.
            if (slipOrder.size() == previousSlipCount) {
                if (slip == previousSlipIndex) ++slip;
            } else {
                slip = previousSlipIndex;
            }
        };
        if (slipOrder[slip].kind == SlipperKind::creature) {
            moveCreatureSlip(slip);
            finishSlipIteration();
            continue;
        }
        const int position = slipOrder[slip].entity;
        const auto source = static_cast<std::size_t>(position);
        if (top[source] != 0x0A || blockDirections[source] < 0) {
            slipOrder.erase(slipOrder.begin() + static_cast<std::ptrdiff_t>(slip));
            finishSlipIteration();
            continue;
        }
        const auto requeueBlockedBlock = [&]() {
            const auto active = std::find_if(slipOrder.begin(), slipOrder.end(), [position](const SlipperEntry& entry) {
                return entry.kind == SlipperKind::block && entry.entity == position;
            });
            if (active != slipOrder.end() && active + 1 != slipOrder.end()) {
                const auto entry = *active;
                slipOrder.erase(active);
                slipOrder.push_back(entry);
            }
        };
        if (bottom[source] == 0x2B && trapExitBlocked(position)) {
            // MoveBlock's blocked path deletes this slip entry; SlipLoop then
            // calls SlideMovement on the same trap, which appends it again.
            requeueBlockedBlock();
            finishSlipIteration();
            continue;
        }
        int directionCode = blockDirections[source];
        if (directionCode < 0) {
            slipOrder.erase(slipOrder.begin() + static_cast<std::ptrdiff_t>(slip));
            finishSlipIteration();
            continue;
        }
        const auto terrain = bottom[source];
        const bool ice = terrain == 0x0C || (terrain >= 0x1A && terrain <= 0x1D);
        // SlipLoop tries every queued block once per tick, even if another
        // action has changed the terrain under it to an ordinary tile. On a
        // blocked MoveBlock, the subsequent SlideMovement keeps its heading
        // on that surface and recreates the slipper at the queue tail.
        const auto retryBlockedBlock = [&]() {
            if (ice) {
                directionCode = static_cast<int>(reverseSlideDirection(
                    terrain, static_cast<Direction>(directionCode)));
            } else if (terrain == 0x0D) directionCode = static_cast<int>(Direction::south);
            else if (terrain == 0x12) directionCode = static_cast<int>(Direction::north);
            else if (terrain == 0x13) directionCode = static_cast<int>(Direction::east);
            else if (terrain == 0x14) directionCode = static_cast<int>(Direction::west);
            else if (terrain == 0x32)
                directionCode = static_cast<int>(randomForceDirection(randomState));
            blockDirections[source] = static_cast<std::int8_t>(directionCode);
            // A blocked MoveBlock deletes the active entry. Its subsequent
            // stationary SlideMovement recreates it at the tail of SlipList.
            requeueBlockedBlock();
            finishSlipIteration();
        };

        // MoveBlock applies CheckPanelWalls to the block's source tile too.
        // A blocked panel exit retains this slipper and its current heading.
        if (isOneWayPanel(terrain) &&
            !panelAllows(terrain, static_cast<Direction>(directionCode), false)) {
            retryBlockedBlock();
            continue;
        }

        auto direction = static_cast<Direction>(directionCode);
        const auto findDestination = [&](Direction stepDirection) -> std::optional<int> {
            int candidate = neighbor(position, stepDirection);
            if (candidate < 0) return std::nullopt;
            // MoveBlock validates the entered tile before invoking
            // EnterTeleport, including the lower-layer CloneMachine guard.
            if (!blockCanEnter(candidate, stepDirection)) return std::nullopt;
            // MoveBlock tests the visible upper tile. Chip's sprite is
            // block-enterable, even when the terrain beneath him is a portal.
            if (candidate != playerPosition && surface(candidate) == 0x29) {
                if (const auto routed = blockTeleportDestination(candidate, stepDirection))
                    candidate = *routed;
            }
            // EnterTeleport checks the tile beyond its chosen portal. It does
            // not re-run BlockCanEnterTile on that portal's lower layer.
            return candidate;
        };
        auto next = findDestination(direction);
        if (!next && ice) {
            const auto originalDirectionCode = directionCode;
            directionCode = static_cast<int>(reverseSlideDirection(terrain, direction));
            direction = static_cast<Direction>(directionCode);
            // The first failed MoveBlock removes the entry before the
            // immediate reverse attempt; SlideMovement re-adds it at the tail.
            requeueBlockedBlock();
            next = findDestination(direction);
            if (!next) {
                // SlipLoop retries backward immediately on ice. If that is
                // blocked too, it restores the original heading for later.
                blockDirections[source] = static_cast<std::int8_t>(originalDirectionCode);
                finishSlipIteration();
                continue;
            }
        } else if (!next) {
            retryBlockedBlock();
            continue;
        }
        const int destination = *next;
        const auto target = static_cast<std::size_t>(destination);
        // MoveBlock dispatches BlockCanEnterTile's action from the visible Upper
        // sprite. When that is Chip, use his live walking/swimming sprite;
        // action 1 copies it into Lower before the block replaces Upper.
        const auto targetSprite = destination == playerPosition
            ? playerSpriteTile() : surface(destination);
        const auto targetAction = targetSprite;

        // Keep the clone-machine upper template if a restored/custom state
        // places a moving block over its machine underlay.
        if (bottom[source] != 0x31) {
            top[source] = bottom[source];
            bottom[source] = 0x00;
        }
        blockDirections[source] = -1;
        bool continuesSliding = false;
        if (targetAction == 0x03) {
            // MoveBlock's Water action changes only the block tile to Dirt;
            // it leaves the destination's lower layer untouched.
            top[target] = 0x0B;
        } else if (targetAction == 0x2A) {
            top[target] = 0x00;
            blockDirections[target] = -1;
        } else {
            bottom[target] = targetSprite;
            top[target] = 0x0A;
            const auto targetFloor = bottom[target];
            const bool slides = targetFloor == 0x0C || (targetFloor >= 0x1A && targetFloor <= 0x1D) ||
                (targetFloor >= 0x0D && targetFloor <= 0x14) || targetFloor == 0x29 ||
                targetFloor == 0x2B || targetFloor == 0x32;
            const auto slideDirection = blockSlideDirection(targetFloor, direction, randomState);
            blockDirections[target] = slides ? static_cast<std::int8_t>(slideDirection) : -1;
            continuesSliding = slides;
        }
        if (targetSprite == 0x23 || targetSprite == 0x24 || targetSprite == 0x27 || targetSprite == 0x28)
            activateButton(targetSprite, destination);
        if (targetSprite == 0x2B) enterLinkedTrap(destination, position);
        if (destination == playerPosition)
            kill(Outcome::dead, "A block crushed you.");
        const auto activeSlip = std::find_if(slipOrder.begin(), slipOrder.end(), [position](const SlipperEntry& entry) {
            return entry.kind == SlipperKind::block && entry.entity == position;
        });
        if (continuesSliding) {
            if (activeSlip != slipOrder.end()) activeSlip->entity = destination;
        } else if (activeSlip != slipOrder.end()) {
            slipOrder.erase(activeSlip);
        }
        finishSlipIteration();
    }
}

bool GameState::trapExitBlocked(int position) const {
    const auto link = std::find_if(traps.begin(), traps.end(), [position](const TrapLink& item) {
        return item.trapPosition == position;
    });
    // The original FindTrap path blocks movement if no link record exists.
    return link == traps.end() || link->closed;
}

bool GameState::blockCanEnter(int position, Direction direction) const {
    if (position < 0 || position >= kBoardCells) return false;
    const auto index = static_cast<std::size_t>(position);
    // BlockCanEnterTile rejects a clone machine on the lower layer before it
    // examines the visible tile, including a Chip sprite covering the machine.
    if (bottom[index] == 0x31) return false;
    if (position == playerPosition) return blockMayEnterTile(playerSpriteTile());
    const auto tile = surface(position);
    if (!blockMayEnterTile(tile)) return false;
    return !isPanelOrIceWall(tile) || panelAllows(tile, direction, true);
}

std::optional<int> GameState::blockTeleportDestination(int entry, Direction direction) const {
    const auto entryTeleport = std::find(teleports.begin(), teleports.end(), entry);
    if (entryTeleport == teleports.end() || teleports.size() < 2) return std::nullopt;
    const auto entryIndex = static_cast<std::size_t>(entryTeleport - teleports.begin());
    std::size_t candidateIndex = (entryIndex + teleports.size() - 1) % teleports.size();
    while (candidateIndex != entryIndex) {
        const int candidate = teleports[candidateIndex];
        const int exit = neighbor(candidate, direction);
        if (surface(candidate) == 0x29 && candidate != playerPosition && exit >= 0 &&
            blockCanEnter(exit, direction)) return candidate;
        candidateIndex = (candidateIndex + teleports.size() - 1) % teleports.size();
    }
    return std::nullopt;
}

void GameState::enterLinkedTrap(int position, int sourcePosition) {
    if (position < 0 || position >= kBoardCells ||
        (bottom[static_cast<std::size_t>(position)] != 0x2B &&
         top[static_cast<std::size_t>(position)] != 0x2B))
        return;
    const auto hasTrapButton = [this](int at) {
        if (at < 0 || at >= kBoardCells) return false;
        const auto index = static_cast<std::size_t>(at);
        // EnterTrap checks the live Upper layer only. A normalized floor in
        // top[] can be covered by Chip or a creature in this state model, and
        // a block can cover a button in bottom[]; those are not button sprites.
        if (at == playerPosition || top[index] != 0x27) return false;
        return std::none_of(creatures.begin(), creatures.end(), [at](const Creature& creature) {
            return creature.alive && creature.position == at;
        });
    };
    // EnterTrap finds the contiguous records that share this destination and
    // sets every flag in the span at once. Endpoints that still contain a trap
    // button are ignored; any other endpoint must match the actual source tile.
    auto spanBegin = std::find_if(traps.begin(), traps.end(), [position](const TrapLink& link) {
        return link.trapPosition == position;
    });
    if (spanBegin == traps.end()) return;
    auto spanEnd = spanBegin;
    while (spanEnd != traps.end() && spanEnd->trapPosition == position) ++spanEnd;
    bool closed = true;
    for (auto link = spanBegin; link != spanEnd; ++link) {
        if (!hasTrapButton(link->buttonPosition) && link->buttonPosition != sourcePosition)
            closed = false;
    }
    for (auto link = spanBegin; link != spanEnd; ++link) link->closed = closed;
}

void GameState::activateCloneButton(int buttonPosition) {
    const auto link = std::find_if(cloneLinks.begin(), cloneLinks.end(), [buttonPosition](const CloneLink& item) {
        return item.buttonPosition == buttonPosition;
    });
    if (link == cloneLinks.end()) return;
    const int machinePosition = link->machinePosition;
    // PressCloneButton reads the machine's live Upper tile. A TankButton can
    // rotate a tank while it occupies a clone machine, changing that template.
    const auto tile = surface(machinePosition);
    if (tile >= 0x40 && tile <= 0x63) {
        const int direction = tile & 3;
        const int x = machinePosition % kBoardWidth + kMonsterDx[static_cast<std::size_t>(direction)];
        const int y = machinePosition / kBoardWidth + kMonsterDy[static_cast<std::size_t>(direction)];
        if (x < 0 || x >= kBoardWidth || y < 0 || y >= kBoardHeight) return;
        const int ahead = y * kBoardWidth + x;
        const auto kind = tile & 0xFC;
        const auto surfaceTile = surface(ahead);
        const auto terrain = isCreatureOverlayTile(surfaceTile)
            ? bottom[static_cast<std::size_t>(ahead)] : surfaceTile;
        const bool canEnter = bottom[static_cast<std::size_t>(ahead)] != 0x31 &&
            creatureMayEnterTile(kind, terrain, monsterDirection(direction));
        // PressCloneButton has one deliberate exception: when entry fails but
        // the upper tile already equals the exact clone template, it continues
        // to the machine-occupancy check. This lets a newly emitted clone queue
        // behind an identical clone blocking the output square.
        if (!canEnter && surfaceTile != tile) return;
        const bool sourceOccupied = std::any_of(creatures.begin(), creatures.end(), [machinePosition](const Creature& c) {
            return c.alive && c.position == machinePosition;
        });
        if (sourceOccupied) return;
        creatures.push_back({tile, machinePosition, true,
            static_cast<std::int8_t>((tile & 0xFC) == 0x4C ? tile & 0x03 : -1),
            static_cast<std::int8_t>(tile & 0x03)});
        // NewMonster routes a clone spawned onto a trap through EnterTrap
        // using (-1,-1) as the source position, just like initial monsters.
        if (bottom[static_cast<std::size_t>(machinePosition)] == 0x2B)
            enterLinkedTrap(machinePosition, -1);
        message = "Clone created.";
        return;
    }

    if (tile >= 0x0E && tile <= 0x11) {
        const int direction = static_cast<int>(tile - 0x0E); // BlockN, BlockW, BlockS, BlockE
        const int x = machinePosition % kBoardWidth + kMonsterDx[static_cast<std::size_t>(direction)];
        const int y = machinePosition / kBoardWidth + kMonsterDy[static_cast<std::size_t>(direction)];
        if (x < 0 || x >= kBoardWidth || y < 0 || y >= kBoardHeight) return;
        const int ahead = y * kBoardWidth + x;
        if (bottom[static_cast<std::size_t>(ahead)] == 0x31 ||
            std::any_of(creatures.begin(), creatures.end(), [ahead](const Creature& c) {
                return c.alive && c.position == ahead;
            })) return;
        const auto travelDirection = monsterDirection(direction);
        if (!blockCanEnter(ahead, travelDirection)) return;
        const auto terrain = surface(ahead);
        int blockDestination = ahead;
        if (ahead != playerPosition && terrain == 0x29) {
            if (const auto routed = blockTeleportDestination(ahead, travelDirection))
                blockDestination = *routed;
        }
        if (blockDestination < 0 || blockDestination >= kBoardCells) return;
        const auto landingSprite = blockDestination == playerPosition
            ? playerSpriteTile() : surface(blockDestination);
        const auto landingAction = landingSprite;
        if (landingAction == 0x03) {
            splashThisTick = true;
            const auto target = static_cast<std::size_t>(blockDestination);
            // MoveBlock's Water action replaces the block's upper tile with
            // Dirt and preserves the destination's lower-layer terrain.
            top[target] = 0x0B;
        } else if (landingAction == 0x2A) {
            top[static_cast<std::size_t>(blockDestination)] = 0x00;
        } else {
            bottom[static_cast<std::size_t>(blockDestination)] = landingSprite;
            top[static_cast<std::size_t>(blockDestination)] = 0x0A;
            const auto targetFloor = bottom[static_cast<std::size_t>(blockDestination)];
            const bool slides = targetFloor == 0x0C || (targetFloor >= 0x1A && targetFloor <= 0x1D) ||
                (targetFloor >= 0x0D && targetFloor <= 0x14) || targetFloor == 0x29 || targetFloor == 0x32;
            if (slides) {
                const auto slideDirection = blockSlideDirection(targetFloor, travelDirection, randomState);
                blockDirections[static_cast<std::size_t>(blockDestination)] = static_cast<std::int8_t>(slideDirection);
                queueBlockSlip(blockDestination);
            }
            if (landingSprite == 0x23 || landingSprite == 0x24 || landingSprite == 0x27 || landingSprite == 0x28)
                activateButton(landingSprite, blockDestination);
            if (landingSprite == 0x2B) enterLinkedTrap(blockDestination, machinePosition);
        }
        message = landingAction == 0x03 ? "The clone block filled the water with dirt."
            : landingAction == 0x04 ? "The clone block covers the fire."
            : "Clone block created.";
        if (blockDestination == playerPosition)
            kill(Outcome::dead, "A block crushed you.");
    }
}

void GameState::activateButton(std::uint8_t tile, int position) {
    switchActivatedThisTick = true;
    if (tile == 0x23) {
        toggleState = !toggleState;
        for (const int position : togglePositions) {
            const auto index = static_cast<std::size_t>(position);
            const auto swapToggleTiles = [](std::uint8_t& value) {
                if (value == 0x25) value = 0x26;
                else if (value == 0x26) value = 0x25;
            };
            swapToggleTiles(top[index]);
            swapToggleTiles(bottom[index]);
        }
        message = "Toggle floors and walls changed.";
    } else if (tile == 0x24) {
        activateCloneButton(position);
    } else if (tile == 0x27) {
        const auto buttonLink = std::find_if(traps.begin(), traps.end(), [position](const TrapLink& item) {
            return item.buttonPosition == position;
        });
        if (buttonLink != traps.end()) {
            const int connectedTrap = buttonLink->trapPosition;
            auto spanBegin = std::find_if(traps.begin(), traps.end(), [connectedTrap](const TrapLink& item) {
                return item.trapPosition == connectedTrap;
            });
            auto spanEnd = spanBegin;
            while (spanEnd != traps.end() && spanEnd->trapPosition == connectedTrap) ++spanEnd;
            for (auto item = spanBegin; item != spanEnd; ++item) item->closed = false;
        }
        message = "Trap opened.";
    } else if (tile == 0x28) {
        for (auto& creature : creatures) {
            if (creature.alive && (creature.tile & 0xFC) == 0x4C) {
                const auto reversed = static_cast<std::uint8_t>(((creature.tile & 0x03) + 2) & 0x03);
                creature.tile = static_cast<std::uint8_t>(0x4C | reversed);
                creature.tankDirection = static_cast<std::int8_t>(reversed);
                creature.movementDirection = static_cast<std::int8_t>(reversed);
                if (bottom[static_cast<std::size_t>(creature.position)] == 0x31)
                    top[static_cast<std::size_t>(creature.position)] = creature.tile;
            }
        }
        message = "Tanks reversed direction.";
    }
}

void GameState::moveCreatures() {
    const auto creatureCount = creatures.size();
    // MonsterLoop reuses xnewdir/ynewdir for every creature in a pass. Most
    // branches write them before MoveMonster; BugMovement on a trap or clone
    // machine skips TurnLeft and accidentally reads the previous attempt's
    // values. Keep that defined, same-pass carry here. If no earlier creature
    // attempted a move, the original reads uninitialized stack data, so retain
    // the deterministic stored-vector fallback for that undefined case.
    int previousMoveDirection = -1;
    for (std::size_t i = 0; i < creatureCount; ++i) {
        auto& creature = creatures[i];
        if (!creature.alive) continue;
        if (std::any_of(slipOrder.begin(), slipOrder.end(), [i](const SlipperEntry& entry) {
                return entry.kind == SlipperKind::creature && entry.entity == static_cast<int>(i);
            }))
            continue; // MonsterLoop skips monsters already handled by SlipLoop.
        const auto kind = creature.tile & 0xFC;
        const int direction = (kind == 0x4C && creature.tankDirection >= 0)
            ? creature.tankDirection
            : creature.movementDirection >= 0 ? creature.movementDirection : creature.tile & 0x03;
        std::array<int, 4> choices{};
        int choiceCount = 0;
        unsigned walkerAttemptMask = 0;
        unsigned blobAttemptMask = 0;
        int blobInitialDirection = direction;
        const auto setChoices = [&](std::initializer_list<int> values) {
            choiceCount = 0;
            for (int value : values) choices[static_cast<std::size_t>(choiceCount++)] = value;
        };
        const auto sourceIsConstrained = [&]() {
            const auto below = bottom[static_cast<std::size_t>(creature.position)];
            return below == 0x2B || below == 0x31; // trap or clone machine
        };
        const bool sourceTrapClosed = bottom[static_cast<std::size_t>(creature.position)] == 0x2B &&
                                      trapExitBlocked(creature.position);
        // MoveMonster rejects every departure from a closed trap. Keep the
        // attempted directions flowing through SetTileDir before rejecting
        // the move: the source updates the creature's facing even when the
        // closed trap prevents movement. Blob continues its random fallback
        // attempts, while other families make only their normal attempts.
        const bool constrained = sourceIsConstrained();
        switch (kind) {
            case 0x40: // bug: left, straight, right, reverse
                // BugMovement skips TurnLeft on a trap/clone tile and reuses
                // MonsterLoop's shared direction locals from the preceding
                // MoveMonster attempt when one exists in this pass.
                if (constrained) setChoices({previousMoveDirection >= 0 ? previousMoveDirection : direction});
                else setChoices({(direction + 1) & 3, direction, (direction + 3) & 3, (direction + 2) & 3});
                break;
            case 0x44: // fireball: straight, right, left, reverse
                if (constrained) setChoices({direction});
                else setChoices({direction, (direction + 3) & 3, (direction + 1) & 3, (direction + 2) & 3});
                break;
            case 0x48: // ball: straight, then reverse
                if (constrained) setChoices({direction});
                else setChoices({direction, (direction + 2) & 3});
                break;
            case 0x4C: // tank: advance straight until a blocked move stops it
                if (creature.tankDirection != -2) setChoices({direction});
                break;
            case 0x50: // glider: straight, left, right, reverse
                if (constrained) setChoices({direction});
                else setChoices({direction, (direction + 1) & 3, (direction + 3) & 3, (direction + 2) & 3});
                break;
            case 0x54: { // teeth: pursue Chip on alternating creature turns
                if (gameTicks % 4 != 0) continue;
                // TeethMovement takes the same shared xnewdir/ynewdir path
                // as BugMovement when its source tile is a trap or clone
                // machine. If an earlier creature attempted a move in this
                // pass, Teeth inherits that last vector; only the first
                // constrained creature needs the deterministic stored-vector
                // fallback for the original's uninitialized-local case.
                if (constrained)
                    setChoices({previousMoveDirection >= 0 ? previousMoveDirection : direction});
                else {
                    const int dx = playerPosition % kBoardWidth - creature.position % kBoardWidth;
                    const int dy = playerPosition / kBoardWidth - creature.position / kBoardWidth;
                    int preferred = -1, alternate = -1;
                    // The classic routine chooses vertical when distances tie.
                    if (std::abs(dx) > std::abs(dy)) {
                        preferred = dx < 0 ? 1 : dx > 0 ? 3 : -1;
                        alternate = dy < 0 ? 0 : dy > 0 ? 2 : -1;
                    } else {
                        preferred = dy < 0 ? 0 : dy > 0 ? 2 : -1;
                        alternate = dx < 0 ? 1 : dx > 0 ? 3 : -1;
                    }
                    // TeethMovement's zero-distance tie falls through to
                    // north, even when both coordinates already match.
                    if (dx == 0 && dy == 0) preferred = 0;
                    if (preferred < 0) setChoices({direction});
                    else if (alternate < 0) setChoices({preferred});
                    else setChoices({preferred, alternate});
                }
                break;
            }
            case 0x58: // walker randomly tries left/right/reverse, without repeats
                if (constrained) setChoices({direction});
                else {
                    setChoices({direction}); // the straight attempt is always first
                    choiceCount = 4; // then try each of the three turns in random order
                }
                break;
            case 0x5C: // blob: random cardinal vector, then random untried turns if blocked
                if (gameTicks % 4 != 0) continue;
                {
                    // Unlike the other movement routines, BlobMovement has
                    // no trap/clone-machine early-out after a blocked move.
                    // It always chooses a random vector and continues trying
                    // distinct random turns until it moves or exhausts them.
                    int xdir{};
                    int ydir{};
                    do {
                        xdir = static_cast<int>(nextRandom(randomState) % 3U) - 1;
                        ydir = static_cast<int>(nextRandom(randomState) % 3U) - 1;
                    } while ((xdir == 0) == (ydir == 0));
                    blobInitialDirection = xdir < 0 ? 1 : xdir > 0 ? 3 : ydir < 0 ? 0 : 2;
                    choices[0] = blobInitialDirection;
                    choiceCount = 4;
                }
                break;
            case 0x60: // paramecium: right, straight, left, reverse
                // ParameciumMovement also skips its normal turn and reuses
                // the MonsterLoop direction locals when constrained.
                if (constrained)
                    setChoices({previousMoveDirection >= 0 ? previousMoveDirection : direction});
                else setChoices({(direction + 3) & 3, direction, (direction + 1) & 3, (direction + 2) & 3});
                break;
            default: continue;
        }

        const int oldPosition = creature.position;
        bool moved = false;
        for (int choiceIndex = 0; choiceIndex < choiceCount; ++choiceIndex) {
            int code = choices[static_cast<std::size_t>(choiceIndex)];
            if (kind == 0x58 && !constrained && choiceIndex > 0) {
                unsigned turn{};
                do {
                    turn = nextRandom(randomState) % 3U;
                } while ((walkerAttemptMask & (1U << turn)) != 0);
                walkerAttemptMask |= 1U << turn;
                code = (direction + (turn == 0 ? 1 : turn == 1 ? 3 : 2)) & 3;
            }
            if (kind == 0x5C && choiceIndex > 0) {
                unsigned turn{};
                do {
                    turn = nextRandom(randomState) % 3U;
                } while ((blobAttemptMask & (1U << turn)) != 0);
                blobAttemptMask |= 1U << turn;
                code = (blobInitialDirection + (turn == 0 ? 1 : turn == 1 ? 3 : 2)) & 3;
            }
            // The original calls SetTileDir before each MoveMonster attempt.
            // Failed attempts still turn the displayed monster, so retain the
            // last attempted direction when every candidate is blocked.
            creature.tile = static_cast<std::uint8_t>((kind & 0xFC) | code);
            previousMoveDirection = code;
            if (bottom[static_cast<std::size_t>(oldPosition)] == 0x31)
                top[static_cast<std::size_t>(oldPosition)] = creature.tile;
            const auto moveDirection = monsterDirection(code);
            if (sourceTrapClosed) continue;
            const auto sourceTerrain = bottom[static_cast<std::size_t>(oldPosition)];
            if (isOneWayPanel(sourceTerrain) && !panelAllows(sourceTerrain, moveDirection, false)) continue;
            const int x = oldPosition % kBoardWidth + kMonsterDx[static_cast<std::size_t>(code)];
            const int y = oldPosition / kBoardWidth + kMonsterDy[static_cast<std::size_t>(code)];
            if (x < 0 || x >= kBoardWidth || y < 0 || y >= kBoardHeight) continue;
            const int target = y * kBoardWidth + x;
            bool occupied = false;
            for (std::size_t j = 0; j < creatures.size(); ++j) {
                if (j != i && creatures[j].alive && creatures[j].position == target) {
                    occupied = true;
                    break;
                }
            }
            // MonsterCanEnterTile uses the Upper creature sprite directly;
            // unlike ChipCanEnterTile, it does not read through creature IDs.
            // The TileTable marks those IDs blocked for monsters.
            if (occupied) continue;

            const auto surfaceTile = surface(target);
            const auto tile = isCreatureOverlayTile(surfaceTile)
                ? bottom[static_cast<std::size_t>(target)] : surfaceTile;
            // MonsterCanEnterTile rejects clone-machine lower layers before
            // looking up the visible upper tile.
            if (bottom[static_cast<std::size_t>(target)] == 0x31) continue;
            if (isPanelOrIceWall(tile) && !panelAllows(tile, moveDirection, true)) continue;
            bool catchesPlayer = false;
            if (target == playerPosition) {
                const auto playerFloor = floor(playerPosition);
                catchesPlayer = creatureCanCatchPlayer(static_cast<std::uint8_t>(kind), playerFloor,
                                                       moveDirection);
                // On a teleporter the original MonsterCanEnterTile path
                // routes the creature before the post-move Chip check. On
                // water/fire, continue through the normal hazard action so
                // creatures die or are blocked before Chip collision. Bombs
                // also need their action: the bomb kills the creature first.
                if (!catchesPlayer && playerFloor != 0x29 &&
                    playerFloor != 0x03 && playerFloor != 0x04 && tile != 0x2A)
                    continue;
            }

            if (tile == 0x2A) {
                // MoveMonster's bomb action writes Floor to the visible
                // layer, regardless of the terrain retained underneath.
                top[static_cast<std::size_t>(target)] = 0x00;
                if (bottom[static_cast<std::size_t>(oldPosition)] != 0x31) {
                    top[static_cast<std::size_t>(oldPosition)] = bottom[static_cast<std::size_t>(oldPosition)];
                    bottom[static_cast<std::size_t>(oldPosition)] = 0x00;
                }
                creature.position = target;
                creature.alive = false;
                moved = true;
                break;
            }
            if (tile == 0x03 && kind != 0x50) {
                if (bottom[static_cast<std::size_t>(oldPosition)] != 0x31) {
                    top[static_cast<std::size_t>(oldPosition)] = bottom[static_cast<std::size_t>(oldPosition)];
                    bottom[static_cast<std::size_t>(oldPosition)] = 0x00;
                }
                creature.position = target;
                creature.alive = false; // only gliders can cross water
                moved = true;
                break;
            }
            if (tile == 0x04) {
                // The original monster tile table blocks bugs and walkers from
                // fire; fireballs enter safely, while other creatures burn up.
                if (kind == 0x40 || kind == 0x58) continue;
                if (kind != 0x44) {
                    if (bottom[static_cast<std::size_t>(oldPosition)] != 0x31) {
                        top[static_cast<std::size_t>(oldPosition)] = bottom[static_cast<std::size_t>(oldPosition)];
                        bottom[static_cast<std::size_t>(oldPosition)] = 0x00;
                    }
                    creature.position = target;
                    creature.alive = false;
                    moved = true;
                    break;
                }
            }
            if (tile == 0x29) {
                const auto entry = std::find(teleports.begin(), teleports.end(), target);
                bool teleported = false;
                if (entry != teleports.end() && teleports.size() > 1) {
                    const auto entryIndex = static_cast<std::size_t>(entry - teleports.begin());
                    std::size_t candidateIndex = (entryIndex + teleports.size() - 1) % teleports.size();
                    while (candidateIndex != entryIndex) {
                        const int candidate = teleports[candidateIndex];
                        const int exitX = candidate % kBoardWidth + kMonsterDx[static_cast<std::size_t>(code)];
                        const int exitY = candidate / kBoardWidth + kMonsterDy[static_cast<std::size_t>(code)];
                        const int exit = exitX >= 0 && exitX < kBoardWidth && exitY >= 0 && exitY < kBoardHeight
                            ? exitY * kBoardWidth + exitX : -1;
                        const bool candidateOccupied = std::any_of(creatures.begin(), creatures.end(),
                            [candidate](const Creature& other) { return other.alive && other.position == candidate; });
                        const bool exitOccupied = std::any_of(creatures.begin(), creatures.end(),
                            [exit](const Creature& other) { return other.alive && other.position == exit; });
                        bool exitAllowed = false;
                    // EnterTeleport requires the alternate portal's visible
                    // upper tile to still be Teleport; a block can cover one.
                    if (surface(candidate) == 0x29 &&
                        exit >= 0 && exit < kBoardCells && !exitOccupied) {
                            const auto exitIndex = static_cast<std::size_t>(exit);
                            const auto exitSurface = surface(exit);
                            const auto exitTile = isCreatureOverlayTile(exitSurface)
                                ? bottom[exitIndex] : exitSurface;
                            const bool blockedMachine = bottom[exitIndex] == 0x31;
                            const auto moveDirection = monsterDirection(code);
                            // EnterTeleport checks the actual Upper sprite via
                            // MonsterCanEnterTile; a creature sprite there
                            // blocks this exit even though its underlay is floor.
                            // EnterTeleport passes the entry's visible
                            // Teleport tile as MonsterCanEnterTile's first
                            // argument while validating a monster exit. This
                            // makes Fire a valid candidate here even for
                            // Bugs/Walkers; the actual species hazard rule is
                            // applied on the following SlipLoop step.
                            const bool tilePass = creatureMayEnterTile(0x29, exitTile, moveDirection);
                            exitAllowed = candidate != playerPosition && !candidateOccupied && !blockedMachine &&
                                          tilePass;
                        }
                        if (exitAllowed) {
                            if (bottom[static_cast<std::size_t>(oldPosition)] != 0x31) {
                                top[static_cast<std::size_t>(oldPosition)] = bottom[static_cast<std::size_t>(oldPosition)];
                                bottom[static_cast<std::size_t>(oldPosition)] = 0x00;
                            }
                            bottom[static_cast<std::size_t>(candidate)] = 0x29;
                            creature.position = candidate;
                            creature.tile = static_cast<std::uint8_t>(kind | code);
                            creature.movementDirection = static_cast<std::int8_t>(code);
                            creature.teleportedThisTick = true;
                            previousMoveDirection = creature.movementDirection;
                            queueCreatureSlip(i);
                            teleported = true;
                            moved = true;
                            break;
                        }
                        candidateIndex = (candidateIndex + teleports.size() - 1) % teleports.size();
                    }
                }
                if (teleported) break;
                if (bottom[static_cast<std::size_t>(oldPosition)] != 0x31) {
                    top[static_cast<std::size_t>(oldPosition)] = bottom[static_cast<std::size_t>(oldPosition)];
                    bottom[static_cast<std::size_t>(oldPosition)] = 0x00;
                }
                bottom[static_cast<std::size_t>(target)] = 0x29;
                creature.position = target;
                creature.tile = static_cast<std::uint8_t>(kind | code);
                creature.movementDirection = static_cast<std::int8_t>(code);
                previousMoveDirection = creature.movementDirection;
                queueCreatureSlip(i);
                moved = true;
                if (target == playerPosition) {
                    playerCoveredByCreature = true;
                    kill(Outcome::dead, "A creature caught you.");
                    break;
                }
                break;
            }
            const bool passable = creatureMayEnterTile(kind, tile, moveDirection);
            if (!passable) continue;
            if (tile == 0x2B) enterLinkedTrap(target, oldPosition);
            if (bottom[static_cast<std::size_t>(oldPosition)] != 0x31) {
                top[static_cast<std::size_t>(oldPosition)] = bottom[static_cast<std::size_t>(oldPosition)];
                bottom[static_cast<std::size_t>(oldPosition)] = 0x00;
            }
            bottom[static_cast<std::size_t>(target)] = tile;
            creature.position = target;
            creature.tile = static_cast<std::uint8_t>((creature.tile & 0xFC) | code);
            creature.movementDirection = static_cast<std::int8_t>(code);
            if (isCreatureSlipTerrain(tile)) {
                auto slideDirection = moveDirection;
                if (tile == 0x0D) slideDirection = Direction::south;
                else if (tile == 0x12) slideDirection = Direction::north;
                else if (tile == 0x13) slideDirection = Direction::east;
                else if (tile == 0x14) slideDirection = Direction::west;
                else if (tile == 0x32) slideDirection = randomForceDirection(randomState);
                else if (tile >= 0x1A && tile <= 0x1D)
                    slideDirection = cornerDirection(tile, moveDirection);
                const int slideCode = monsterCode(slideDirection);
                creature.tile = static_cast<std::uint8_t>((creature.tile & 0xFC) | slideCode);
                creature.movementDirection = static_cast<std::int8_t>(slideCode);
                if (kind == 0x4C) creature.tankDirection = static_cast<std::int8_t>(slideCode);
                queueCreatureSlip(i);
            }
            if (tile == 0x23 || tile == 0x24 || tile == 0x27 || tile == 0x28) {
                activateButton(tile, target);
                // A CloneButton can append to `creatures`, reallocating the
                // vector and invalidating the reference held for this turn.
                // Reacquire the current monster by its stable loop index.
                previousMoveDirection = creatures[i].movementDirection;
            } else {
                previousMoveDirection = creature.movementDirection;
            }
            // MoveMonster passes the direction locals by pointer to
            // SlideMovement. Force floors and ice corners can rewrite them;
            // the next creature in MonsterLoop reads that final vector if it
            // is a constrained Bug.
            // MoveMonster's original autopsy checks the copied lower-layer
            // player sprite after hazards and buttons have resolved. A block
            // can leave that sprite visible after Chip moves away.
            if (catchesPlayer || isPlayerSpriteTile(top[static_cast<std::size_t>(target)])) {
                playerCoveredByCreature = true;
                kill(Outcome::dead, "A creature caught you.");
                break;
            }
            moved = true;
            break;
        }
        // TankMovement stops after a blocked MoveMonster attempt unless the
        // source tile is a closed or unlinked trap (handled above). Clone
        // machines do not have this exception: the sprite keeps its facing,
        // while the movement vector is cleared.
        if (!moved && kind == 0x4C && !sourceTrapClosed)
            creature.tankDirection = -2;
        // TeethMovement has a final failedMove path after both pursuit axes
        // are blocked. That path calls SetTileDir again with the preferred
        // axis, so the sprite ends the turn facing preferred rather than the
        // secondary direction attempted immediately before it. Teeth on a
        // trap or clone machine skip that fallback in the original.
        if (!moved && kind == 0x54 && !constrained && choiceCount > 1)
            creature.tile = static_cast<std::uint8_t>((kind & 0xFC) | choices[0]);
        (void)moved;
    }
}

void GameState::moveCreatureSlip(std::size_t& slip, bool retrying) {
        if (slip >= slipOrder.size() || slipOrder[slip].kind != SlipperKind::creature) { ++slip; return; }
        const auto creatureIndex = static_cast<std::size_t>(slipOrder[slip].entity);
        if (creatureIndex >= creatures.size() || !creatures[creatureIndex].alive) {
            slipOrder.erase(slipOrder.begin() + static_cast<std::ptrdiff_t>(slip));
            return;
        }
        auto& creature = creatures[creatureIndex];
        const int source = creature.position;
        const auto sourceIndex = static_cast<std::size_t>(source);
        auto terrain = bottom[sourceIndex];

        auto directionCode = slipOrder[slip].direction >= 0
            ? static_cast<int>(slipOrder[slip].direction)
            : creature.movementDirection >= 0
                ? static_cast<int>(creature.movementDirection) : static_cast<int>(creature.tile & 0x03);
        const auto storeSlipDirection = [&](int code) {
            const auto active = std::find_if(slipOrder.begin(), slipOrder.end(), [creatureIndex](const SlipperEntry& entry) {
                return entry.kind == SlipperKind::creature && entry.entity == static_cast<int>(creatureIndex);
            });
            if (active != slipOrder.end()) active->direction = static_cast<std::int8_t>(code);
        };
        auto direction = monsterDirection(directionCode);
        const auto kind = static_cast<std::uint8_t>(creature.tile & 0xFC);
        const std::size_t originalSlipIndex = slip;
        const auto requeueBlockedCreature = [&]() {
            const auto active = std::find_if(slipOrder.begin(), slipOrder.end(), [creatureIndex](const SlipperEntry& entry) {
                return entry.kind == SlipperKind::creature && entry.entity == static_cast<int>(creatureIndex);
            });
            if (active != slipOrder.end() && active + 1 != slipOrder.end()) {
                const auto entry = *active;
                slipOrder.erase(active);
                slipOrder.push_back(entry);
            }
        };
        creature.tile = static_cast<std::uint8_t>((creature.tile & 0xFC) | directionCode);
        if (bottom[sourceIndex] == 0x31) top[sourceIndex] = creature.tile;

        const auto stopOrReverse = [&]() {
            if (terrain == 0x0C || (terrain >= 0x1A && terrain <= 0x1D)) {
                direction = reverseSlideDirection(terrain, direction);
                directionCode = monsterCode(direction);
                storeSlipDirection(directionCode);
                if (!retrying) {
                    creature.tile = static_cast<std::uint8_t>((creature.tile & 0xFC) | directionCode);
                    if (bottom[sourceIndex] == 0x31) top[sourceIndex] = creature.tile;
                    // MoveMonster removes a blocked creature slipper. The
                    // first SlideMovement retry recreates it at the tail;
                    // retry the same creature there, then restore the outer
                    // SlipLoop cursor index for its ordinary post-step advance.
                    requeueBlockedCreature();
                    const auto retryEntry = std::find_if(slipOrder.begin(), slipOrder.end(), [creatureIndex](const SlipperEntry& entry) {
                        return entry.kind == SlipperKind::creature && entry.entity == static_cast<int>(creatureIndex);
                    });
                    if (retryEntry != slipOrder.end()) {
                        slip = static_cast<std::size_t>(retryEntry - slipOrder.begin());
                        moveCreatureSlip(slip, true);
                    }
                    slip = originalSlipIndex;
                    return;
                }
                // The reverse transform above and its second application on
                // this retry restore/redirect the stored heading as the
                // original SlipLoop's second SlideMovement call does.
                ++slip;
            } else if (terrain == 0x0D || (terrain >= 0x12 && terrain <= 0x14)) {
                direction = terrain == 0x0D ? Direction::south
                    : terrain == 0x12 ? Direction::north
                    : terrain == 0x13 ? Direction::east : Direction::west;
                directionCode = monsterCode(direction);
                storeSlipDirection(directionCode);
                creature.tile = static_cast<std::uint8_t>((creature.tile & 0xFC) | directionCode);
                if (bottom[sourceIndex] == 0x31) top[sourceIndex] = creature.tile;
                ++slip;
            } else if (terrain == 0x32) {
                direction = randomForceDirection(randomState);
                directionCode = monsterCode(direction);
                storeSlipDirection(directionCode);
                creature.tile = static_cast<std::uint8_t>((creature.tile & 0xFC) | directionCode);
                if (bottom[sourceIndex] == 0x31) top[sourceIndex] = creature.tile;
                ++slip;
            } else if (terrain == 0x29 || terrain == 0x2B || isOneWayPanel(terrain)) {
                // SlipLoop reverses only after an ice or ice-wall failure.
                // On portals, traps, and one-way panels the follow-up
                // SlideMovement keeps the existing vector.
                requeueBlockedCreature();
                ++slip;
            } else {
                // MoveMonster deletes a blocked creature slipper, then the
                // SlipLoop's stationary SlideMovement recreates it at the
                // tail even when the underlay is ordinary terrain.
                requeueBlockedCreature();
                ++slip;
            }
        };

        if (terrain == 0x2B && trapExitBlocked(source)) {
            // MoveMonster rejects departure from a closed trap. SlipLoop's
            // blocked path preserves direction here because traps are not ice.
            requeueBlockedCreature();
            ++slip;
            return;
        }
        if (isOneWayPanel(terrain) && !panelAllows(terrain, direction, false)) {
            stopOrReverse();
            return;
        }

        const int destination = neighbor(source, direction);
        if (destination < 0) {
            stopOrReverse();
            return;
        }
        const auto targetIndex = static_cast<std::size_t>(destination);
        const bool playerContact = destination == playerPosition;
        const auto surfaceTile = surface(destination);
        const auto tile = isCreatureOverlayTile(surfaceTile)
            ? bottom[targetIndex] : surfaceTile;
        const auto moveDirection = monsterDirection(directionCode);
        // MonsterCanEnterTile rejects a CloneMachine underlay before reading
        // Teleport, panel, Chip, or creature sprites from the upper layer.
        if (bottom[targetIndex] == 0x31) {
            stopOrReverse();
            return;
        }
        if (isPanelOrIceWall(tile) && !panelAllows(tile, moveDirection, true)) {
            stopOrReverse();
            return;
        }
        bool occupied = top[targetIndex] == 0x0A;
        for (std::size_t i = 0; i < creatures.size(); ++i) {
            if (i != creatureIndex && creatures[i].alive && creatures[i].position == destination) {
                occupied = true;
                break;
            }
        }
        bool catchesPlayer = false;
        if (playerContact) {
            const auto playerFloor = floor(playerPosition);
            catchesPlayer = creatureCanCatchPlayer(kind, playerFloor, moveDirection);
            if (!catchesPlayer && playerFloor != 0x29 &&
                playerFloor != 0x03 && playerFloor != 0x04 && tile != 0x2A) {
                stopOrReverse();
                return;
            }
        }
        if (occupied) {
            stopOrReverse();
            return;
        }

        if (tile == 0x29) {
            const auto entry = std::find(teleports.begin(), teleports.end(), destination);
            bool teleported = false;
            if (entry != teleports.end() && teleports.size() > 1) {
                const auto entryIndex = static_cast<std::size_t>(entry - teleports.begin());
                std::size_t candidateIndex = (entryIndex + teleports.size() - 1) % teleports.size();
                while (candidateIndex != entryIndex) {
                    const int candidate = teleports[candidateIndex];
                    const int exitX = candidate % kBoardWidth + kMonsterDx[static_cast<std::size_t>(directionCode)];
                    const int exitY = candidate / kBoardWidth + kMonsterDy[static_cast<std::size_t>(directionCode)];
                    const int exit = exitX >= 0 && exitX < kBoardWidth && exitY >= 0 && exitY < kBoardHeight
                        ? exitY * kBoardWidth + exitX : -1;
                    const bool candidateOccupied = std::any_of(creatures.begin(), creatures.end(),
                        [candidate](const Creature& other) { return other.alive && other.position == candidate; });
                    const bool exitOccupied = std::any_of(creatures.begin(), creatures.end(),
                        [exit](const Creature& other) { return other.alive && other.position == exit; });
                    if (surface(candidate) == 0x29 && exit >= 0 && !exitOccupied &&
                        !candidateOccupied && candidate != playerPosition) {
                        const auto exitIndex = static_cast<std::size_t>(exit);
                        const auto exitSurface = surface(exit);
                        const auto exitTile = isCreatureOverlayTile(exitSurface)
                            ? bottom[exitIndex] : exitSurface;
                        const bool blockedMachine = bottom[exitIndex] == 0x31;
                        // Preserve EnterTeleport's original argument: the
                        // candidate exit check uses the entry Teleport tile
                        // (0x29) as MonsterCanEnterTile's creature value.
                        // Fire therefore passes routing for every species,
                        // then is resolved by the creature's next slip move.
                        const bool tilePass = creatureMayEnterTile(0x29, exitTile, moveDirection);
                        if (!blockedMachine && tilePass) {
                            if (bottom[sourceIndex] != 0x31) {
                                top[sourceIndex] = bottom[sourceIndex];
                                bottom[sourceIndex] = 0x00;
                            }
                            bottom[static_cast<std::size_t>(candidate)] = 0x29;
                            creature.position = candidate;
                            creature.movementDirection = static_cast<std::int8_t>(directionCode);
                            creature.teleportedThisTick = true;
                            if (kind == 0x4C)
                                creature.tankDirection = static_cast<std::int8_t>(directionCode);
                storeSlipDirection(directionCode);
                            // Keep the slip entry; SlipLoop advances from the
                            // destination teleporter on a later pass.
                            ++slip;
                            teleported = true;
                            break;
                        }
                    }
                    candidateIndex = (candidateIndex + teleports.size() - 1) % teleports.size();
                }
            }
            if (!teleported) {
                if (bottom[sourceIndex] != 0x31) {
                    top[sourceIndex] = bottom[sourceIndex];
                    bottom[sourceIndex] = 0x00;
                }
                bottom[targetIndex] = 0x29;
                creature.position = destination;
                creature.movementDirection = static_cast<std::int8_t>(directionCode);
                if (kind == 0x4C)
                    creature.tankDirection = static_cast<std::int8_t>(directionCode);
                storeSlipDirection(directionCode);
                ++slip;
                if (playerContact) {
                    playerCoveredByCreature = true;
                    kill(Outcome::dead, "A creature caught you.");
                }
            }
            return;
        }
        if (tile == 0x2A) {
            // Match MoveMonster's bomb action: the bomb becomes visible Floor,
            // while its lower-layer tile remains unchanged.
            top[targetIndex] = 0x00;
            if (bottom[sourceIndex] != 0x31) {
                top[sourceIndex] = bottom[sourceIndex];
                bottom[sourceIndex] = 0x00;
            }
            creature.position = destination;
            creature.alive = false;
            slipOrder.erase(slipOrder.begin() + static_cast<std::ptrdiff_t>(slip));
            return;
        }
        if (tile == 0x03 && kind != 0x50) {
            if (bottom[sourceIndex] != 0x31) {
                top[sourceIndex] = bottom[sourceIndex];
                bottom[sourceIndex] = 0x00;
            }
            creature.position = destination;
            creature.alive = false;
            slipOrder.erase(slipOrder.begin() + static_cast<std::ptrdiff_t>(slip));
            return;
        }
        if (tile == 0x04 && (kind == 0x40 || kind == 0x58)) {
            stopOrReverse();
            return;
        }
        if (tile == 0x04 && kind != 0x44) {
            if (bottom[sourceIndex] != 0x31) {
                top[sourceIndex] = bottom[sourceIndex];
                bottom[sourceIndex] = 0x00;
            }
            creature.position = destination;
            creature.alive = false;
            slipOrder.erase(slipOrder.begin() + static_cast<std::ptrdiff_t>(slip));
            return;
        }
        const bool passable = creatureMayEnterTile(kind, tile, direction);
        if (!passable || tile == 0x29) {
            stopOrReverse();
            return;
        }

        if (tile == 0x2B) enterLinkedTrap(destination, source);
        if (bottom[sourceIndex] != 0x31) {
            top[sourceIndex] = bottom[sourceIndex];
            bottom[sourceIndex] = 0x00;
        }
        bottom[targetIndex] = tile;
        creature.position = destination;
        creature.movementDirection = static_cast<std::int8_t>(directionCode);
        if (kind == 0x4C)
            creature.tankDirection = static_cast<std::int8_t>(directionCode);
        storeSlipDirection(directionCode);
        if (isCreatureSlipTerrain(tile)) {
            auto nextDirection = direction;
            if (tile == 0x0D) nextDirection = Direction::south;
            else if (tile == 0x12) nextDirection = Direction::north;
            else if (tile == 0x13) nextDirection = Direction::east;
            else if (tile == 0x14) nextDirection = Direction::west;
            else if (tile == 0x32) nextDirection = randomForceDirection(randomState);
            else if (tile >= 0x1A && tile <= 0x1D) nextDirection = cornerDirection(tile, direction);
            const int nextCode = monsterCode(nextDirection);
            storeSlipDirection(nextCode);
            creature.tile = static_cast<std::uint8_t>((creature.tile & 0xFC) | nextCode);
            creature.movementDirection = static_cast<std::int8_t>(nextCode);
            if (kind == 0x4C) creature.tankDirection = static_cast<std::int8_t>(nextCode);
            ++slip;
        } else {
            slipOrder.erase(slipOrder.begin() + static_cast<std::ptrdiff_t>(slip));
        }
        if (tile == 0x23 || tile == 0x24 || tile == 0x27 || tile == 0x28) {
            activateButton(tile, destination);
            // MoveMonster copies the current tank's updated Monster vector
            // back through its direction pointers after PressTankButton.
            // Other tanks keep their independent Slipper vectors unchanged.
            if (tile == 0x28 && kind == 0x4C)
                storeSlipDirection(creature.movementDirection);
        }
        if (catchesPlayer || isPlayerSpriteTile(top[targetIndex])) {
            playerCoveredByCreature = true;
            kill(Outcome::dead, "A creature caught you.");
            return;
        }
}

Direction GameState::forcedDirection(Direction current, std::uint8_t tile) {
    if ((tile == 0x0C || (tile >= 0x1A && tile <= 0x1D)) && !boots[2]) {
        return tile == 0x0C ? current : cornerDirection(tile, current);
    }
    if ((tile == 0x0D || (tile >= 0x12 && tile <= 0x14)) && !boots[3]) {
        if (tile == 0x0D) return Direction::south;
        if (tile == 0x12) return Direction::north;
        if (tile == 0x13) return Direction::east;
        return Direction::west;
    }
    if (tile == 0x32 && !boots[3]) {
        return randomForceDirection(randomState);
    }
    return current;
}

MoveResult GameState::finishMove(Direction direction, int previousPosition) {
    (void)previousPosition;
    sliding = false;
    const auto tile = floor(playerPosition);
    const bool ice = tile == 0x0C || (tile >= 0x1A && tile <= 0x1D);
    const bool force = tile == 0x0D || (tile >= 0x12 && tile <= 0x14) || tile == 0x32;
    const bool teleport = tile == 0x29;
    if (teleport || (ice && !boots[2]) || (force && !boots[3])) {
        sliding = true;
        slideDirection = forcedDirection(direction, tile);
    }
    return MoveResult::moved;
}

bool GameState::moveBlockFrom(int sourcePosition, Direction direction,
                              bool checkSlippingDirection, bool delayButton,
                              std::uint8_t& delayedButtonTile,
                              int& delayedButtonPosition,
                              std::uint8_t& vacatedTile) {
    if (sourcePosition < 0 || sourcePosition >= kBoardCells ||
        surface(sourcePosition) != 0x0A)
        return false;
    const auto source = static_cast<std::size_t>(sourcePosition);
    auto activeSlip = std::find_if(slipOrder.begin(), slipOrder.end(), [sourcePosition](const SlipperEntry& entry) {
        return entry.kind == SlipperKind::block && entry.entity == sourcePosition;
    });
    if (checkSlippingDirection && activeSlip != slipOrder.end()) {
        const int directionCode = blockDirections[source];
        if (directionCode >= 0) {
            const auto blockDirection = static_cast<Direction>(directionCode);
            const auto opposite = static_cast<Direction>((static_cast<unsigned>(direction) + 2U) % 4U);
            if (blockDirection == direction || blockDirection == opposite) return false;
        }
    }
    // MoveBlock's shared blocked path removes the block's slip-list record.
    // SlipLoop restores it afterward through SlideMovement; a push attempt
    // has no such retry phase, so keep this side effect here for callers.
    const auto blocked = [&]() {
        if (activeSlip != slipOrder.end()) slipOrder.erase(activeSlip);
        return false;
    };
    if (bottom[source] == 0x2B && trapExitBlocked(sourcePosition)) return blocked();
    if (isOneWayPanel(bottom[source]) && !panelAllows(bottom[source], direction, false)) return blocked();

    const int beyond = neighbor(sourcePosition, direction);
    if (beyond < 0) return blocked();
    // MoveBlock calls BlockCanEnterTile on the entry tile before routing. This
    // rejects a portal over a lower CloneMachine even if another portal exits.
    if (!blockCanEnter(beyond, direction)) return blocked();
    int blockDestination = beyond;
    if (blockDestination != playerPosition && surface(blockDestination) == 0x29) {
        if (const auto routed = blockTeleportDestination(blockDestination, direction))
            blockDestination = *routed;
    }
    const auto target = static_cast<std::size_t>(blockDestination);
    const auto targetSprite = blockDestination == playerPosition
        ? playerSpriteTile() : surface(blockDestination);
    const auto targetAction = targetSprite;
    if (targetAction == 0x03) {
        splashThisTick = true;
        // The source action skips the normal upper-to-lower layer copy here.
        top[target] = 0x0B;
        if (activeSlip != slipOrder.end()) slipOrder.erase(activeSlip);
    } else if (targetAction == 0x2A) {
        top[target] = 0x00;
        if (activeSlip != slipOrder.end()) slipOrder.erase(activeSlip);
    } else {
        bottom[target] = targetSprite;
        top[target] = 0x0A;
        const auto targetFloor = bottom[target];
        const bool regularSlide = targetFloor == 0x0C || (targetFloor >= 0x1A && targetFloor <= 0x1D) ||
            (targetFloor >= 0x0D && targetFloor <= 0x14) || targetFloor == 0x29 || targetFloor == 0x32;
        const bool retainedTrapSlide = targetFloor == 0x2B && activeSlip != slipOrder.end();
        const bool slides = regularSlide || retainedTrapSlide;
        const auto slideDirection = retainedTrapSlide
            ? direction : blockSlideDirection(targetFloor, direction, randomState);
        blockDirections[target] = slides ? static_cast<std::int8_t>(slideDirection) : -1;
        if (activeSlip != slipOrder.end()) {
            if (slides) activeSlip->entity = blockDestination;
            else slipOrder.erase(activeSlip);
        } else if (slides) {
            queueBlockSlip(blockDestination);
        }
        if (targetSprite == 0x2B) enterLinkedTrap(blockDestination, sourcePosition);
    }

    if (targetSprite == 0x23 || targetSprite == 0x24 || targetSprite == 0x27 || targetSprite == 0x28) {
        if (delayButton) {
            delayedButtonTile = targetSprite;
            delayedButtonPosition = blockDestination;
        } else {
            activateButton(targetSprite, blockDestination);
        }
    }
    vacatedTile = bottom[source];
    // MoveBlock leaves the source upper layer untouched when a block starts
    // on a CloneMachine. That upper tile is the machine's persistent template.
    if (bottom[source] != 0x31) {
        top[source] = vacatedTile;
        bottom[source] = 0x00;
    }
    blockDirections[source] = -1;
    if (blockDestination == playerPosition)
        kill(Outcome::dead, "A block crushed you.");
    return true;
}

bool GameState::tryMove(Direction direction) {
    if (bottom[static_cast<std::size_t>(playerPosition)] == 0x2B && trapExitBlocked(playerPosition))
        return false;
    const auto sourceTile = floor(playerPosition);
    if (isOneWayPanel(sourceTile) && !panelAllows(sourceTile, direction, false)) return false;
    const int destination = neighbor(playerPosition, direction);
    if (destination < 0) return false;
    const auto dest = static_cast<std::size_t>(destination);
    auto tile = surface(destination);
    std::uint8_t delayedButtonTile = 0;
    int delayedButtonPosition = -1;

    if (isPanelOrIceWall(tile) && !panelAllows(tile, direction, true)) return false;

    if (tile == 0x0A) {
        if (!moveBlockFrom(destination, direction, true, true,
                           delayedButtonTile, delayedButtonPosition, tile)) return false;
    }

    // ChipCanEnterTile rejects any lower-layer CloneMachine before checking
    // the visible creature tile. This check follows a possible block push so
    // that block movement can still occur before Chip's attempted entry fails.
    if (bottom[dest] == 0x31) return false;

    if (tile >= 0x40 && tile <= 0x63) {
        // ChipCanEnterTile recursively validates the lower tile for an upper
        // creature sprite, but retains that sprite's Action=2. MoveChip then
        // classifies the original upper creature as Eaten. Validate the
        // underlay first so walls, locked doors, and active toggle walls still
        // block the move before the creature collision is resolved.
        const auto underlay = bottom[dest];
        const bool blockUnderlay = underlay == 0x0A || underlay == 0x1F || underlay == 0x2C;
        const bool lockedDoor = underlay >= 0x16 && underlay <= 0x19 &&
            keys[static_cast<std::size_t>(underlay - 0x16)] == 0;
        if (blockUnderlay || lockedDoor ||
            (isPanelOrIceWall(underlay) && !panelAllows(underlay, direction, true)) ||
            !passableFloor(underlay))
            return false;
        playerPosition = destination;
        playerCoveredByCreature = false;
        kill(Outcome::dead, "A creature caught you.");
        return true;
    }

    if (tile == 0x1F || tile == 0x2C) {
        top[dest] = 0x01;
        message = "A hidden wall was revealed.";
        return false;
    }
    if (tile == 0x2E) {
        top[dest] = 0x01;
        tile = 0x00; // PopupWall rises as Chip passes onto its tile.
        message = "A popup wall rose behind you.";
    }

    if (tile == 0x29) {
        const auto sourceTeleport = std::find(teleports.begin(), teleports.end(), destination);
        if (sourceTeleport != teleports.end() && teleports.size() > 1) {
            const auto sourceIndex = static_cast<std::size_t>(sourceTeleport - teleports.begin());
            std::size_t candidateIndex = (sourceIndex + teleports.size() - 1) % teleports.size();
            while (candidateIndex != sourceIndex) {
                const int candidate = teleports[candidateIndex];
                const int exit = neighbor(candidate, direction);
                // The original teleporter list may retain a coordinate after
                // another entity covers its portal; EnterTeleport checks that
                // the candidate's current upper tile is still Teleport.
                if (surface(candidate) == 0x29 && exit >= 0) {
                    if (surface(exit) == 0x0A) {
                        std::uint8_t unusedButton = 0;
                        std::uint8_t vacatedTile = 0;
                        int unusedButtonPosition = -1;
                        // EnterTeleport tries MoveBlock at its candidate exit
                        // before validating Chip's entry to the vacated square.
                        if (!moveBlockFrom(exit, direction, false, false,
                                           unusedButton, unusedButtonPosition, vacatedTile)) {
                            candidateIndex = (candidateIndex + teleports.size() - 1) % teleports.size();
                            continue;
                        }
                    }
                    if (chipCanEnterVisibleTile(exit, direction)) {
                        playerPosition = candidate;
                        playerTeleportedThisTick = true;
                        message = "Teleported.";
                        return true;
                    }
                }
                candidateIndex = (candidateIndex + teleports.size() - 1) % teleports.size();
            }
        }
        playerPosition = destination;
        message = "No open teleport destination.";
        return true;
    }
    if (!passableFloor(tile)) return false;

    // Keys and boots are transparent in ChipCanEnterTile. Their action is
    // retained from the visible item, but the lower tile must also allow entry.
    // This prevents collecting an item through a wall, locked door, active
    // toggle wall, or lower-layer CloneMachine.
    if (tile >= 0x64 && tile <= 0x6B && !chipCanEnterVisibleTile(destination, direction))
        return false;

    if (tile >= 0x64 && tile <= 0x67) {
        playerPickupThisTick = PickupKind::key;
        ++keys[static_cast<std::size_t>(colorIndex(tile, 0x64))];
        top[dest] = bottom[dest];
        message = "Key collected.";
    } else if (tile >= 0x68 && tile <= 0x6B) {
        playerPickupThisTick = PickupKind::boot;
        boots[static_cast<std::size_t>(colorIndex(tile, 0x68))] = true;
        top[dest] = bottom[dest];
        message = "Boots collected.";
    } else if (tile >= 0x16 && tile <= 0x19) {
        auto& count = keys[static_cast<std::size_t>(colorIndex(tile, 0x16))];
        if (count == 0) return false;
        if (tile != 0x18) --count; // Green keys are reusable in the original rules.
        switchActivatedThisTick = true;
        doorOpenedThisTick = true;
        top[dest] = bottom[dest];
        message = "Door unlocked.";
    } else if (tile == 0x22) {
        // Socket shares the door's action-3 layer behavior once its chip gate
        // allows entry: Chip covers it and the underlying terrain remains.
        switchActivatedThisTick = true;
        doorOpenedThisTick = true;
        top[dest] = bottom[dest];
        message = "Socket opened.";
    } else if (tile == 0x02) {
        playerPickupThisTick = PickupKind::chip;
        if (chipsLeft > 0) --chipsLeft;
        top[dest] = bottom[dest];
        message = "Chip collected.";
    } else if (tile == 0x03 && !boots[0]) {
        playerPosition = destination;
        kill(Outcome::dead, "You drowned. Flippers protect against water.", DeathCause::water);
        return true;
    } else if (tile == 0x04 && !boots[1]) {
        playerPosition = destination;
        kill(Outcome::dead, "You burned. Fire boots protect against fire.", DeathCause::fire);
        return true;
    } else if (tile == 0x15) {
        // The reference tile table marks Exit enterable independently of the
        // remaining chip counter; sockets perform the actual chip gating.
        playerPosition = destination;
        kill(Outcome::won, "Level complete!");
        return true;
    } else if (tile == 0x0B) {
        top[dest] = bottom[dest];
    } else if (tile == 0x1E) {
        top[dest] = 0x00;
    } else if (tile == 0x2F) {
        message = level != nullptr && !level->hint.empty()
            ? "Hint: " + level->hint : "No hint is provided for this level.";
    } else if (tile == 0x21) {
        thiefActivatedThisTick = true;
        // The original thief uses ResetInventory(bootsOnly=1): it removes
        // every boot but leaves all colored keys untouched. Thief terrain is
        // persistent: Chip standing over it must not consume the map tile.
        boots.fill(false);
        message = "The thief took your boots; your keys are safe.";
    } else if (tile == 0x2A) {
        playerPosition = destination;
        kill(Outcome::dead, "You hit a bomb.");
        return true;
    } else {
        message.clear();
    }

    playerPosition = destination;
    if (tile == 0x23 || tile == 0x24 || tile == 0x27 || tile == 0x28)
        activateButton(tile, destination);
    if (delayedButtonTile != 0)
        activateButton(delayedButtonTile, delayedButtonPosition);
    return true;
}

void GameState::restart() {
    if (level != nullptr) {
        const auto tickPhase = gameTicks;
        const auto randomPhase = randomState;
        *this = start(*level, tickPhase, randomPhase);
    }
}
}
