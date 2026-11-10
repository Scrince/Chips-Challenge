#include "levelpack.hpp"
#include "replay.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
std::uint32_t parseUnsigned(std::string_view value, const std::string& context) {
    std::uint32_t parsed{};
    const auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
    if (result.ec != std::errc{} || result.ptr != value.data() + value.size())
        throw std::runtime_error("Invalid unsigned integer for " + context + ": " + std::string(value));
    return parsed;
}

chips::Direction parseDirection(const std::string& value, std::size_t line) {
    if (value == "N" || value == "n" || value == "north") return chips::Direction::north;
    if (value == "E" || value == "e" || value == "east") return chips::Direction::east;
    if (value == "S" || value == "s" || value == "south") return chips::Direction::south;
    if (value == "W" || value == "w" || value == "west") return chips::Direction::west;
    throw std::runtime_error("Invalid direction on replay line " + std::to_string(line));
}

chips::ReplayScript readReplay(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open replay script: " + path.string());
    chips::ReplayScript script;
    bool hasLevel = false;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (const auto comment = line.find('#'); comment != std::string::npos) line.erase(comment);
        std::istringstream words(line);
        std::string command;
        if (!(words >> command)) continue;
        std::string argument;
        if (!(words >> argument))
            throw std::runtime_error("Missing argument on replay line " + std::to_string(lineNumber));
        std::string extra;
        if (words >> extra)
            throw std::runtime_error("Unexpected extra token on replay line " + std::to_string(lineNumber));

        if (command == "level") {
            const auto value = parseUnsigned(argument, "level");
            if (value == 0 || value > 0xFFFFU)
                throw std::runtime_error("Replay level must be in the range 1..65535");
            script.levelNumber = static_cast<std::uint16_t>(value);
            hasLevel = true;
        } else if (command == "seed") {
            script.randomSeed = parseUnsigned(argument, "seed");
        } else if (command == "initial_ticks") {
            script.initialGameTicks = parseUnsigned(argument, "initial_ticks");
        } else if (command == "move") {
            script.commands.push_back(chips::ReplayCommand::move(parseDirection(argument, lineNumber)));
        } else if (command == "press") {
            script.commands.push_back(chips::ReplayCommand::press(parseDirection(argument, lineNumber)));
        } else if (command == "advance") {
            script.commands.push_back(chips::ReplayCommand::advance(parseUnsigned(argument, "advance")));
        } else if (command == "settle") {
            script.commands.push_back(chips::ReplayCommand::settle(parseUnsigned(argument, "settle")));
        } else if (command == "target") {
            const auto value = parseUnsigned(argument, "target");
            if (value >= chips::kBoardCells)
                throw std::runtime_error("Replay target cell must be in the range 0..1023");
            script.commands.push_back(chips::ReplayCommand::mouseTarget(static_cast<int>(value)));
        } else {
            throw std::runtime_error("Unknown replay operation on line " + std::to_string(lineNumber) +
                                     ": " + command);
        }
    }
    if (!file.eof()) throw std::runtime_error("Could not read replay script: " + path.string());
    if (!hasLevel) throw std::runtime_error("Replay script is missing its 'level' header");
    return script;
}

const char* outcomeName(chips::Outcome outcome) {
    switch (outcome) {
        case chips::Outcome::playing: return "playing";
        case chips::Outcome::won: return "won";
        case chips::Outcome::dead: return "dead";
        case chips::Outcome::timedOut: return "timed_out";
    }
    return "unknown";
}

const char* deathCauseName(chips::DeathCause cause) {
    switch (cause) {
        case chips::DeathCause::none: return "none";
        case chips::DeathCause::water: return "water";
        case chips::DeathCause::fire: return "fire";
        case chips::DeathCause::other: return "other";
        case chips::DeathCause::timeout: return "timeout";
    }
    return "unknown";
}

const char* directionName(chips::Direction direction) {
    switch (direction) {
        case chips::Direction::north: return "north";
        case chips::Direction::east: return "east";
        case chips::Direction::south: return "south";
        case chips::Direction::west: return "west";
    }
    return "unknown";
}

void jsonString(std::ostream& out, const std::string& value) {
    out << '"';
    for (const unsigned char byte : value) {
        switch (byte) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (byte < 0x20) out << '?';
                else out << static_cast<char>(byte);
        }
    }
    out << '"';
}

void printSnapshot(std::size_t step, const chips::GameState& game) {
    const auto state = chips::snapshot(game);
    std::cout << "{\"step\":" << step << ",\"tick\":" << state.gameTicks
              << ",\"digest\":\"" << std::hex << std::setw(16) << std::setfill('0')
              << state.digest << std::dec << "\",\"player\":" << state.playerPosition
              << ",\"player_surface\":" << static_cast<unsigned>(game.surface(state.playerPosition))
              << ",\"player_floor\":" << static_cast<unsigned>(game.floor(state.playerPosition))
              << ",\"player_top\":" << static_cast<unsigned>(game.top[static_cast<std::size_t>(state.playerPosition)])
              << ",\"player_bottom\":" << static_cast<unsigned>(game.bottom[static_cast<std::size_t>(state.playerPosition)])
              << ",\"facing\":\"" << directionName(game.facing) << "\",\"chips\":"
              << state.chipsLeft << ",\"seconds\":" << state.secondsLeft
              << ",\"sliding\":" << (game.sliding ? "true" : "false")
              << ",\"slide_direction\":\"" << directionName(game.slideDirection)
              << "\",\"mouse_target\":";
    if (game.mouseTargetPosition) std::cout << *game.mouseTargetPosition;
    else std::cout << "null";
    std::cout << ",\"keys\":[";
    for (std::size_t i = 0; i < game.keys.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << game.keys[i];
    }
    std::cout << "],\"boots\":[";
    for (std::size_t i = 0; i < game.boots.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << (game.boots[i] ? "true" : "false");
    }
    std::cout << "],\"creatures\":[";
    bool first = true;
    for (const auto& creature : game.creatures) {
        if (!creature.alive) continue;
        if (!first) std::cout << ',';
        first = false;
        std::cout << "{\"tile\":" << static_cast<unsigned>(creature.tile)
                  << ",\"position\":" << creature.position << '}';
    }
    std::cout << "],\"outcome\":\"" << outcomeName(state.outcome)
              << "\",\"death_cause\":\"" << deathCauseName(state.deathCause)
              << "\",\"message\":";
    jsonString(std::cout, game.message);
    std::cout << "}\n";
}
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: chips-replay <CHIPS.DAT> <replay.txt>\n";
        return 2;
    }
    try {
        const auto levels = chips::loadLevelPack(argv[1]);
        const auto script = readReplay(argv[2]);
        const auto found = std::find_if(levels.begin(), levels.end(), [&script](const chips::Level& level) {
            return level.number == script.levelNumber;
        });
        if (found == levels.end()) throw std::runtime_error("Replay level is not present in the pack");

        auto game = chips::GameState::start(*found, script.initialGameTicks, script.randomSeed);
        printSnapshot(0, game);
        for (std::size_t i = 0; i < script.commands.size(); ++i) {
            const auto& command = script.commands[i];
            switch (command.operation) {
                case chips::ReplayOperation::queueMove: game.queueMove(command.direction); break;
                case chips::ReplayOperation::pressMove: (void)game.pressMove(command.direction); break;
                case chips::ReplayOperation::advanceTicks: game.advanceTicks(command.ticks); break;
                case chips::ReplayOperation::settleInput:
                    for (unsigned tick = 0; tick < command.ticks && game.outcome == chips::Outcome::playing; ++tick) {
                        if (!game.sliding && !game.chipHasMoved && !game.pendingDirection) break;
                        game.advanceTicks(1);
                    }
                    if (game.outcome == chips::Outcome::playing &&
                        (game.sliding || game.chipHasMoved || game.pendingDirection))
                        throw std::runtime_error("Replay input did not settle within its tick limit");
                    break;
                case chips::ReplayOperation::setMouseTarget: game.setMouseTarget(command.targetPosition); break;
                default: throw std::runtime_error("Replay contains an unknown operation");
            }
            printSnapshot(i + 1, game);
        }
        return std::cout ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "Replay failed: " << error.what() << '\n';
        return 1;
    }
}
