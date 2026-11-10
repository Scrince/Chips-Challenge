#include "levelpack.hpp"

#include <cstddef>
#include <fstream>
#include <iterator>
#include <utility>

namespace chips {
namespace {
class Reader {
public:
    explicit Reader(std::vector<std::uint8_t> data) : data_(std::move(data)) {}

    [[nodiscard]] std::size_t remaining() const { return data_.size() - position_; }

    [[nodiscard]] std::uint8_t byte(const std::string& context) {
        require(1, context);
        return data_[position_++];
    }

    [[nodiscard]] std::uint16_t word(const std::string& context) {
        const auto low = byte(context);
        const auto high = byte(context);
        return static_cast<std::uint16_t>(low | (static_cast<std::uint16_t>(high) << 8));
    }

    [[nodiscard]] std::vector<std::uint8_t> bytes(std::size_t count,
                                                  const std::string& context) {
        require(count, context);
        auto start = data_.begin() + static_cast<std::ptrdiff_t>(position_);
        std::vector<std::uint8_t> result(start, start + static_cast<std::ptrdiff_t>(count));
        position_ += count;
        return result;
    }

    [[nodiscard]] std::vector<std::uint8_t> sized(const std::string& context) {
        return bytes(word(context + " length"), context);
    }

private:
    void require(std::size_t count, const std::string& context) const {
        if (count > remaining()) {
            throw LevelPackError("Truncated " + context + " at byte " +
                                 std::to_string(position_));
        }
    }

    std::vector<std::uint8_t> data_;
    std::size_t position_{};
};

[[nodiscard]] std::array<std::uint8_t, kBoardCells> decodeLayer(
    const std::vector<std::uint8_t>& encoded, const std::string& context) {
    std::array<std::uint8_t, kBoardCells> result{};
    std::size_t input = 0;
    std::size_t output = 0;
    while (input < encoded.size()) {
        const auto tile = encoded[input++];
        std::size_t count = 1;
        std::uint8_t value = tile;
        if (tile == 0xFF) {
            if (input + 2 > encoded.size()) {
                throw LevelPackError("Incomplete RLE run in " + context);
            }
            count = encoded[input++];
            value = encoded[input++];
            // ExpandTilesRLE decrements an 8-bit remaining-count after
            // emitting the first tile. A stored zero therefore wraps to 255
            // and represents a 256-tile run, not an empty run.
            if (count == 0) count = 256;
        }
        if (count > result.size() - output) {
            throw LevelPackError(context + " expands beyond 1024 tiles");
        }
        for (std::size_t i = 0; i < count; ++i) {
            result[output++] = value;
        }
    }
    if (output != result.size()) {
        throw LevelPackError(context + " expands to " + std::to_string(output) +
                             " tiles; expected 1024");
    }
    return result;
}

[[nodiscard]] std::string toUtf8(const std::vector<std::uint8_t>& bytes) {
    static constexpr std::array<std::uint16_t, 32> cp1252{
        0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
        0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
        0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
        0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178};
    std::string out;
    for (const auto byte : bytes) {
        if (byte == 0) break;
        std::uint32_t codepoint = byte;
        if (byte >= 0x80 && byte <= 0x9F) codepoint = cp1252[byte - 0x80];
        if (codepoint <= 0x7F) {
            out.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7FF) {
            out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
            out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
    }
    return out;
}

void parseFields(Level& level, const std::vector<std::uint8_t>& raw) {
    std::size_t cursor = 0;
    while (cursor < raw.size()) {
        if (cursor + 2 > raw.size()) {
            throw LevelPackError("Truncated optional field header in level " +
                                 std::to_string(level.number));
        }
        OptionalField field;
        field.id = raw[cursor++];
        const auto size = raw[cursor++];
        if (size > raw.size() - cursor) {
            throw LevelPackError("Truncated optional field " + std::to_string(field.id) +
                                 " in level " + std::to_string(level.number));
        }
        field.raw.assign(raw.begin() + static_cast<std::ptrdiff_t>(cursor),
                         raw.begin() + static_cast<std::ptrdiff_t>(cursor + size));
        cursor += size;
        if (field.id == 1 || field.id == 2) {
            if (field.raw.size() < 2) {
                throw LevelPackError("Optional metadata field " + std::to_string(field.id) +
                                     " is shorter than a word in level " +
                                     std::to_string(level.number));
            }
            const auto value = static_cast<std::uint16_t>(field.raw[0] |
                (static_cast<std::uint16_t>(field.raw[1]) << 8U));
            if (field.id == 1) level.timeLimit = value;
            else level.chipsRequired = value;
        }
        if (field.id == 3) level.title = toUtf8(field.raw);
        if (field.id == 6) {
            auto decoded = field.raw;
            for (auto& byte : decoded) {
                if (byte == 0) break;
                byte ^= 0x99;
            }
            level.password = toUtf8(decoded);
        }
        if (field.id == 7) level.hint = toUtf8(field.raw);
        if (field.id == 8) level.password = toUtf8(field.raw);
        level.fields.push_back(std::move(field));
    }
}
}

std::uint8_t Level::fieldTile(int x, int y) const {
    if (x < 0 || x >= kBoardWidth || y < 0 || y >= kBoardHeight) return 1;
    const auto index = static_cast<std::size_t>(y * kBoardWidth + x);
    const auto tile = top[index];
    // Creature/player sprites are transparent overlay values in the upper layer.
    if (tile >= 0x40 && tile <= 0x6F) return bottom[index];
    return tile;
}

std::vector<Level> loadLevelPack(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw LevelPackError("Cannot open level pack: " + path.string());
    std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(file)), {});
    Reader reader(std::move(data));

    if (reader.word("level pack signature") != 0xAAAC) {
        throw LevelPackError("Not a recognized Chip's Challenge level pack");
    }
    const auto version = reader.word("level pack version");
    if (version != 2) {
        throw LevelPackError("Unsupported level pack version " + std::to_string(version));
    }
    const auto levelCount = reader.word("level count");
    if (levelCount == 0) {
        throw LevelPackError("Level pack contains no levels");
    }
    std::vector<Level> levels;
    levels.reserve(levelCount);

    for (std::uint16_t index = 0; index < levelCount; ++index) {
        const auto chunkBytes = reader.sized("level " + std::to_string(index + 1));
        Reader chunk(chunkBytes);
        Level level;
        level.number = chunk.word("level number");
        level.timeLimit = chunk.word("level time limit");
        level.chipsRequired = chunk.word("chip requirement");
        level.mapType = chunk.word("map type");
        if (level.mapType > 1) {
            throw LevelPackError("Unsupported map type " + std::to_string(level.mapType) +
                                 " in level " + std::to_string(level.number));
        }
        const auto top = chunk.sized("level top layer");
        const auto bottom = chunk.sized("level bottom layer");
        const auto fields = chunk.sized("level optional fields");
        if (chunk.remaining() != 0) {
            throw LevelPackError("Unexpected data in level " + std::to_string(level.number));
        }
        level.top = decodeLayer(top, "level " + std::to_string(level.number) + " top layer");
        level.bottom = decodeLayer(bottom, "level " + std::to_string(level.number) + " bottom layer");
        parseFields(level, fields);
        levels.push_back(std::move(level));
    }
    if (reader.remaining() != 0) {
        throw LevelPackError("Unexpected trailing data after final level");
    }
    return levels;
}
}
