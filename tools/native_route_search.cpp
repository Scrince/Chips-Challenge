#include "engine.hpp"
#include "levelpack.hpp"
#include "replay.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace {
constexpr std::array<chips::Direction, 4> kDirections{
    chips::Direction::north, chips::Direction::east,
    chips::Direction::south, chips::Direction::west};
constexpr std::array<char, 4> kDirectionLetters{'N', 'E', 'S', 'W'};
constexpr std::size_t kNoParent = std::numeric_limits<std::size_t>::max();

std::uint32_t parseLimit(std::string_view text) {
    std::uint32_t result{};
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || result == 0)
        throw std::runtime_error("max-states must be a positive integer");
    return result;
}

struct Node {
    chips::GameState state;
    std::size_t parent{kNoParent};
    char move{};
    std::uint32_t depth{};
};

std::string routeTo(const std::deque<Node>& nodes, std::size_t index) {
    std::string route;
    for (auto current = index; nodes[current].parent != kNoParent;
         current = nodes[current].parent)
        route.push_back(nodes[current].move);
    std::reverse(route.begin(), route.end());
    return route;
}

std::uint64_t searchKey(const chips::GameState& state) {
    // Absolute elapsed time is not gameplay state for route reachability. Keep
    // the modulo-four creature cadence and remaining level time, while
    // normalizing counters that otherwise make every loop appear unique.
    auto normalized = state;
    normalized.gameTicks %= 4U;
    normalized.secondsElapsed = 0;
    return chips::snapshot(normalized).digest;
}
}

int main(int argc, char** argv) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: chips-route-search <CHIPS.DAT> <level> [max-states]\n";
        return 2;
    }
    try {
        const auto levels = chips::loadLevelPack(argv[1]);
        const auto number = parseLimit(argv[2]);
        if (number > std::numeric_limits<std::uint16_t>::max())
            throw std::runtime_error("level must be in the range 1..65535");
        const auto limit = argc == 4 ? parseLimit(argv[3]) : 100'000U;
        const auto found = std::find_if(levels.begin(), levels.end(), [number](const chips::Level& level) {
            return level.number == number;
        });
        if (found == levels.end()) throw std::runtime_error("level is not present in the supplied pack");

        std::deque<Node> nodes;
        nodes.push_back({chips::GameState::start(*found), kNoParent, 0, 0});
        std::unordered_map<std::uint64_t, std::size_t> seen;
        seen.emplace(searchKey(nodes.front().state), 0);
        std::size_t cursor = 0;
        std::size_t winningNode = kNoParent;

        while (cursor < nodes.size() && nodes.size() < limit) {
            const auto parent = cursor++;
            const auto depth = nodes[parent].depth + 1;
            for (std::size_t direction = 0; direction < kDirections.size(); ++direction) {
                auto next = nodes[parent].state;
                (void)next.pressMove(kDirections[direction]);
                next.advanceTicks(2); // A source-style press and one ordinary turn interval.
                if (next.outcome == chips::Outcome::won) {
                    nodes.push_back({std::move(next), parent, kDirectionLetters[direction], depth});
                    winningNode = nodes.size() - 1;
                    break;
                }
                if (next.outcome != chips::Outcome::playing) continue;
                const auto digest = searchKey(next);
                if (seen.contains(digest)) continue;
                const auto index = nodes.size();
                seen.emplace(digest, index);
                nodes.push_back({std::move(next), parent, kDirectionLetters[direction], depth});
                if (nodes.size() >= limit) break;
            }
            if (winningNode != kNoParent) break;
        }

        if (winningNode != kNoParent) {
            const auto route = routeTo(nodes, winningNode);
            std::cout << "{\"status\":\"won\",\"level\":" << number
                      << ",\"moves\":" << route.size()
                      << ",\"explored_states\":" << nodes.size()
                      << ",\"route\":\"" << route << "\"}\n";
            return 0;
        }
        std::cout << "{\"status\":\"state_limit\",\"level\":" << number
                  << ",\"explored_states\":" << nodes.size()
                  << ",\"limit\":" << limit << "}\n";
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Native route search failed: " << error.what() << '\n';
        return 1;
    }
}
