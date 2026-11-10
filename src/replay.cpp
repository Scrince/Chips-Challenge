#include "replay.hpp"

#include <type_traits>
#include <stdexcept>

namespace chips {
namespace {
class StableHash {
public:
    template <typename Integer>
    void integer(Integer value) {
        using Unsigned = std::make_unsigned_t<Integer>;
        auto bytes = static_cast<Unsigned>(value);
        for (std::size_t i = 0; i < sizeof(Unsigned); ++i) {
            byte(static_cast<std::uint8_t>(bytes & 0xFFU));
            bytes >>= 8U;
        }
    }

    void byte(std::uint8_t value) {
        value_ ^= value;
        value_ *= 1099511628211ULL;
    }

    [[nodiscard]] std::uint64_t value() const { return value_; }

private:
    std::uint64_t value_{14695981039346656037ULL};
};

std::uint16_t levelNumber(const GameState& game) {
    return game.level ? game.level->number : 0;
}

} // namespace

ReplayCommand ReplayCommand::move(Direction direction) {
    ReplayCommand command;
    command.operation = ReplayOperation::queueMove;
    command.direction = direction;
    return command;
}

ReplayCommand ReplayCommand::press(Direction direction) {
    ReplayCommand command;
    command.operation = ReplayOperation::pressMove;
    command.direction = direction;
    return command;
}

ReplayCommand ReplayCommand::advance(unsigned ticks) {
    ReplayCommand command;
    command.operation = ReplayOperation::advanceTicks;
    command.ticks = ticks;
    return command;
}

ReplayCommand ReplayCommand::settle(unsigned maxTicks) {
    ReplayCommand command;
    command.operation = ReplayOperation::settleInput;
    command.ticks = maxTicks;
    return command;
}

ReplayCommand ReplayCommand::mouseTarget(int position) {
    ReplayCommand command;
    command.operation = ReplayOperation::setMouseTarget;
    command.targetPosition = position;
    return command;
}

StateSnapshot snapshot(const GameState& game) {
    StableHash hash;
    hash.integer(levelNumber(game));
    for (const auto tile : game.top) hash.byte(tile);
    for (const auto tile : game.bottom) hash.byte(tile);
    for (const auto direction : game.blockDirections) hash.integer(direction);
    for (const auto& creature : game.creatures) {
        hash.byte(creature.tile);
        hash.integer(creature.position);
        hash.byte(creature.alive ? 1 : 0);
        hash.integer(creature.tankDirection);
        hash.integer(creature.movementDirection);
        hash.byte(creature.teleportedThisTick ? 1 : 0);
    }
    hash.integer(static_cast<std::uint64_t>(game.creatures.size()));
    for (const auto& trap : game.traps) {
        hash.integer(trap.buttonPosition);
        hash.integer(trap.trapPosition);
        hash.byte(trap.closed ? 1 : 0);
    }
    hash.integer(static_cast<std::uint64_t>(game.traps.size()));
    for (const auto& clone : game.cloneLinks) {
        hash.integer(clone.buttonPosition);
        hash.integer(clone.machinePosition);
        hash.byte(clone.prototypeTile);
    }
    hash.integer(static_cast<std::uint64_t>(game.cloneLinks.size()));
    for (const auto teleport : game.teleports) hash.integer(teleport);
    hash.integer(static_cast<std::uint64_t>(game.teleports.size()));
    for (const auto& entry : game.slipOrder) {
        hash.byte(static_cast<std::uint8_t>(entry.kind));
        hash.integer(entry.entity);
        hash.byte(static_cast<std::uint8_t>(entry.direction));
    }
    hash.integer(static_cast<std::uint64_t>(game.slipOrder.size()));
    for (const auto key : game.keys) hash.integer(key);
    for (const auto boot : game.boots) hash.byte(boot ? 1 : 0);
    hash.integer(game.playerPosition);
    hash.byte(static_cast<std::uint8_t>(game.facing));
    hash.integer(game.chipsLeft);
    hash.integer(game.secondsLeft);
    hash.integer(game.secondsElapsed);
    hash.byte(game.toggleState ? 1 : 0);
    hash.byte(game.sliding ? 1 : 0);
    hash.byte(game.chipHasMoved ? 1 : 0);
    hash.integer(game.idleTickCount);
    hash.byte(static_cast<std::uint8_t>(game.slideDirection));
    hash.integer(game.randomState);
    hash.integer(game.gameTicks);
    hash.byte(game.pendingDirection ? 1 : 0);
    if (game.pendingDirection) hash.byte(static_cast<std::uint8_t>(*game.pendingDirection));
    hash.byte(game.mouseTargetPosition ? 1 : 0);
    if (game.mouseTargetPosition) hash.integer(*game.mouseTargetPosition);
    hash.byte(game.pendingAutopsy ? 1 : 0);
    hash.byte(game.playerCoveredByCreature ? 1 : 0);
    hash.byte(static_cast<std::uint8_t>(game.outcome));
    hash.byte(static_cast<std::uint8_t>(game.deathCause));

    return {hash.value(), game.gameTicks, levelNumber(game), game.playerPosition,
            game.chipsLeft, game.secondsLeft, game.outcome, game.deathCause};
}

std::vector<StateSnapshot> runReplay(const Level& level, const ReplayScript& script) {
    if (script.levelNumber != level.number)
        throw std::invalid_argument("Replay level number does not match supplied level");
    auto game = GameState::start(level, script.initialGameTicks, script.randomSeed);
    std::vector<StateSnapshot> result;
    result.reserve(script.commands.size());
    for (const auto& command : script.commands) {
        switch (command.operation) {
            case ReplayOperation::queueMove:
                game.queueMove(command.direction);
                break;
            case ReplayOperation::pressMove:
                (void)game.pressMove(command.direction);
                break;
            case ReplayOperation::advanceTicks:
                game.advanceTicks(command.ticks);
                break;
            case ReplayOperation::settleInput: {
                for (unsigned tick = 0; tick < command.ticks && game.outcome == Outcome::playing; ++tick) {
                    if (!game.sliding && !game.chipHasMoved && !game.pendingDirection) break;
                    game.advanceTicks(1);
                }
                if (game.outcome == Outcome::playing &&
                    (game.sliding || game.chipHasMoved || game.pendingDirection))
                    throw std::runtime_error("Replay input did not settle within its tick limit");
                break;
            }
            case ReplayOperation::setMouseTarget:
                game.setMouseTarget(command.targetPosition);
                break;
            default:
                throw std::invalid_argument("Replay contains an unknown operation");
        }
        result.push_back(snapshot(game));
    }
    return result;
}

} // namespace chips
