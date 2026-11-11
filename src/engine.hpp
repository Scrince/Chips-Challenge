#pragma once

#include "levelpack.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace chips {
enum class Direction : std::uint8_t { north, east, south, west };
enum class Outcome : std::uint8_t { playing, won, dead, timedOut };
enum class DeathCause : std::uint8_t { none, water, fire, other, timeout };
enum class MoveResult : std::uint8_t { moved, blocked, died, won, ignored };
enum class PickupKind : std::uint8_t { none, chip, key, boot };

constexpr std::uint8_t chipSpriteTile(Direction facing, bool swimming) noexcept {
    const std::uint8_t frame = facing == Direction::north ? 0
        : facing == Direction::west ? 1
        : facing == Direction::south ? 2 : 3;
    return static_cast<std::uint8_t>((swimming ? 0x3C : 0x6C) + frame);
}

struct Creature {
    std::uint8_t tile{};
    int position{};
    bool alive{true};
    // Tank velocity is distinct from its displayed facing: blocked tanks stop
    // without turning, and a tank button can restart them in the reverse direction.
    std::int8_t tankDirection{-1}; // -1: derive from tile; -2: stopped
    // MonsterList stores its movement vector separately from the displayed
    // tile direction; blocked AI attempts can turn the sprite without moving.
    std::int8_t movementDirection{-1}; // N/W/S/E tile encoding, -1 derives from tile
    // A teleported creature must be drawn at its destination without a
    // straight-line interpolation through unrelated board cells.
    bool teleportedThisTick{};
};

struct TrapLink {
    int buttonPosition{};
    int trapPosition{};
    bool closed{};
};

struct CloneLink {
    int buttonPosition{};
    int machinePosition{};
    std::uint8_t prototypeTile{};
};

enum class SlipperKind : std::uint8_t { block, creature };
struct SlipperEntry {
    SlipperKind kind{};
    int entity{}; // block board position or stable creature index
    // The original keeps a creature slipper's vector in Slipper.xdir/ydir,
    // separately from Monster.xdir/ydir and the displayed facing tile.
    std::int8_t direction{-1};
};

struct GameState {
    const Level* level{};
    std::array<std::uint8_t, kBoardCells> top{};
    std::array<std::uint8_t, kBoardCells> bottom{};
    // -1 means stationary; otherwise stores the last cardinal push direction.
    std::array<std::int8_t, kBoardCells> blockDirections{};
    // The original processes blocks and monsters in one insertion-ordered slip list.
    std::vector<SlipperEntry> slipOrder;
    std::vector<Creature> creatures;
    std::vector<TrapLink> traps;
    std::vector<CloneLink> cloneLinks;
    std::vector<int> teleports;
    std::vector<int> togglePositions;
    std::array<unsigned, 4> keys{};
    std::array<bool, 4> boots{};
    int playerPosition{};
    Direction facing{Direction::south};
    unsigned chipsLeft{};
    unsigned secondsLeft{};
    unsigned secondsElapsed{};
    bool toggleState{};
    bool sliding{};
    // Mirrors DoTick's one-voluntary-move-per-turn gate. Forced slide steps do not set it.
    bool chipHasMoved{};
    // DoTick turns an idle Chip south after three eligible even ticks.
    std::uint16_t idleTickCount{};
    Direction slideDirection{Direction::south};
    std::uint32_t randomState{1U};
    unsigned gameTicks{};
    std::optional<Direction> pendingDirection;
    std::optional<int> mouseTargetPosition;
    bool playerAttemptedThisTick{};
    // True if Chip traversed at least one tile during this tick. A forced
    // floor can push Chip back onto the tile where the tick began, so the
    // final position alone cannot distinguish that successful move from a wall.
    bool playerMovedThisTick{};
    bool playerTeleportedThisTick{};
    // NewMonster records Eaten in the original autopsy field when a creature
    // starts over Chip; MonsterLoop reports that death after its first pass.
    bool pendingAutopsy{};
    // The source compositor keeps the arriving creature in Upper when a
    // creature catches Chip, so that creature covers Chip's Lower sprite.
    // Conversely, MoveChip leaves Chip's death pose in Upper when Chip walks
    // into a creature. This records which sprite should be visually on top.
    bool playerCoveredByCreature{};
    // Explicit switch/gate audio event; status messages may be overwritten by
    // later actions in the same tick and are not a reliable event channel.
    bool switchActivatedThisTick{};
    bool doorOpenedThisTick{};
    bool thiefActivatedThisTick{};
    bool splashThisTick{};
    // Set only by a successful Chip, key, or boot tile action. Inventory
    // changes alone cannot distinguish pickups from thieves or door use.
    PickupKind playerPickupThisTick{PickupKind::none};
    Outcome outcome{Outcome::playing};
    DeathCause deathCause{DeathCause::none};
    std::string message;

    [[nodiscard]] static GameState start(const Level& level, std::uint32_t initialGameTicks = 0,
                           std::uint32_t initialRandomState = 1U);
    void queueMove(Direction direction);
    // Handle a physical key/controller direction event like the original
    // MoveChip call: move immediately if the turn is open, otherwise buffer
    // one direction for the next eligible tick.
    [[nodiscard]] MoveResult pressMove(Direction direction);
    void setMouseTarget(int position);
    void advanceTicks(unsigned ticks = 1);
    [[nodiscard]] MoveResult move(Direction direction);
    [[nodiscard]] MoveResult move(char direction);
    void restart();
    [[nodiscard]] std::uint8_t surface(int position) const;
    [[nodiscard]] std::uint8_t floor(int position) const;
    [[nodiscard]] bool standingOnHint() const;
    [[nodiscard]] std::uint8_t playerSpriteTile() const;
    [[nodiscard]] std::uint8_t playerDisplayTile() const;
    [[nodiscard]] bool playerHasBoot(unsigned boot) const;

private:
    [[nodiscard]] MoveResult moveInternal(Direction direction, bool voluntaryInput = false);
    [[nodiscard]] MoveResult moveSlideAfterDeath(Direction direction);
    [[nodiscard]] bool tryMove(Direction direction);
    [[nodiscard]] MoveResult finishMove(Direction direction, int previousPosition);
    [[nodiscard]] Direction forcedDirection(Direction current, std::uint8_t tile);
    [[nodiscard]] bool trapExitBlocked(int position) const;
    void enterLinkedTrap(int position, int sourcePosition);
    [[nodiscard]] bool moveBlockFrom(int sourcePosition, Direction direction,
                                     bool checkSlippingDirection, bool delayButton,
                                     std::uint8_t& delayedButtonTile,
                                     int& delayedButtonPosition,
                                     std::uint8_t& vacatedTile);
    [[nodiscard]] std::optional<int> blockTeleportDestination(int entry, Direction direction) const;
    [[nodiscard]] bool blockCanEnter(int position, Direction direction) const;
    void activateCloneButton(int buttonPosition);
    void activateButton(std::uint8_t tile, int position);
    void queueBlockSlip(int position);
    void queueCreatureSlip(std::size_t creatureIndex);
    void moveCreatures(); // One MonsterLoop pass on each even simulation tick.
    void moveBlocks();
    void moveCreatureSlip(std::size_t& slip, bool retrying = false);
    void advanceMouseTarget();
    void kill(Outcome reason, std::string why, DeathCause cause = DeathCause::other);
    [[nodiscard]] bool passableFloor(std::uint8_t tile) const;
    [[nodiscard]] bool chipCanEnterVisibleTile(int position, Direction direction);
};
}
