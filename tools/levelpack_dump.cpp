#include "levelpack.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void jsonString(std::ostream& out, const std::string& value) {
    out << '"';
    for (const unsigned char byte : value) {
        switch (byte) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (byte < 0x20) {
                    constexpr char digits[] = "0123456789abcdef";
                    out << "\\u00" << digits[byte >> 4] << digits[byte & 0x0F];
                } else {
                    out << static_cast<char>(byte);
                }
        }
    }
    out << '"';
}

template <typename Range>
void hexBytes(std::ostream& out, const Range& bytes) {
    constexpr char digits[] = "0123456789abcdef";
    for (const auto byte : bytes) {
        out << digits[byte >> 4] << digits[byte & 0x0F];
    }
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: chips-levelpack-dump <CHIPS.DAT>\n";
        return 2;
    }
    try {
        const auto levels = chips::loadLevelPack(std::filesystem::path(argv[1]));
        std::cout << "[\n";
        for (std::size_t i = 0; i < levels.size(); ++i) {
            const auto& level = levels[i];
            if (i != 0) std::cout << ",\n";
            std::cout << "  {\"number\":" << level.number
                      << ",\"time_limit\":" << level.timeLimit
                      << ",\"chips_required\":" << level.chipsRequired
                      << ",\"map_type\":" << level.mapType << ",\"title\":";
            jsonString(std::cout, level.title);
            std::cout << ",\"hint\":";
            jsonString(std::cout, level.hint);
            std::cout << ",\"password\":";
            jsonString(std::cout, level.password);
            std::cout << ",\"top_hex\":\"";
            hexBytes(std::cout, level.top);
            std::cout << "\",\"bottom_hex\":\"";
            hexBytes(std::cout, level.bottom);
            std::cout << "\",\"fields\":[";
            for (std::size_t fieldIndex = 0; fieldIndex < level.fields.size(); ++fieldIndex) {
                const auto& field = level.fields[fieldIndex];
                if (fieldIndex != 0) std::cout << ',';
                std::cout << "{\"id\":" << static_cast<unsigned>(field.id) << ",\"raw_hex\":\"";
                hexBytes(std::cout, field.raw);
                std::cout << "\"}";
            }
            std::cout << "]}";
        }
        std::cout << "\n]\n";
        return std::cout ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "Level-pack dump failed: " << error.what() << '\n';
        return 1;
    }
}
