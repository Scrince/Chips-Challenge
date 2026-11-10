#include "levelpack.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace {
using Bytes = std::vector<std::uint8_t>;

void word(Bytes& out, std::uint16_t value) {
    out.push_back(static_cast<std::uint8_t>(value & 0xFF));
    out.push_back(static_cast<std::uint8_t>(value >> 8));
}

void sized(Bytes& out, const Bytes& value) {
    word(out, static_cast<std::uint16_t>(value.size()));
    out.insert(out.end(), value.begin(), value.end());
}

Bytes rleWithZeroCount() {
    Bytes encoded{0xFF, 0x00, 0x7C}; // Zero encodes 256 output tiles.
    for (int i = 0; i < 3; ++i) encoded.insert(encoded.end(), {0xFF, 0xFF, 0x42});
    encoded.insert(encoded.end(), {0xFF, 0x03, 0x42});
    return encoded;
}

Bytes makePack() {
    Bytes top = rleWithZeroCount();
    Bytes bottom;
    for (int i = 0; i < 4; ++i) bottom.insert(bottom.end(), {0xFF, 0x00, 0x00});
    const Bytes fields{
        0x01, 0x02, 0x34, 0x12, // Optional time limit overrides header.
        0x02, 0x02, 0x78, 0x56, // Optional chip requirement overrides header.
        0x06, 0x04, 0xD8, 0xDB, 0xDA, 0x00, // Encoded "ABC" password.
        0xFE, 0x03, 0x10, 0x20, 0x30 // Unknown optional field.
    };

    Bytes record;
    word(record, 1); // number
    word(record, 90); // time limit
    word(record, 7); // chips required
    word(record, 0); // map type
    sized(record, top);
    sized(record, bottom);
    sized(record, fields);

    Bytes pack;
    word(pack, 0xAAAC);
    word(pack, 2);
    word(pack, 1);
    sized(pack, record);
    return pack;
}

Bytes makeEmptyPack() {
    Bytes pack;
    word(pack, 0xAAAC);
    word(pack, 2);
    word(pack, 0);
    return pack;
}

bool writePack(const std::filesystem::path& path, const Bytes& bytes) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char*>(bytes.data()),
                static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(file);
}
}

int main() {
    const auto nonce = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto path = std::filesystem::temp_directory_path() /
        ("chips-levelpack-test-" + std::to_string(nonce) + ".dat");
    const auto cleanup = [&] {
        std::error_code error;
        std::filesystem::remove(path, error);
    };

    try {
        if (!writePack(path, makePack())) {
            std::cerr << "Could not write synthetic level pack\n";
            cleanup();
            return 1;
        }
        const auto levels = chips::loadLevelPack(path);
        cleanup();
        if (levels.size() != 1 || levels[0].number != 1 || levels[0].timeLimit != 0x1234 ||
            levels[0].chipsRequired != 0x5678 || levels[0].password != "ABC") {
            std::cerr << "Optional time/chip metadata did not override its header words\n";
            return 1;
        }
        for (std::size_t i = 0; i < 256; ++i) {
            if (levels[0].top[i] != 0x7C || levels[0].bottom[i] != 0x00) {
                std::cerr << "Zero-count RLE run did not expand to 256 tiles\n";
                return 1;
            }
        }
        for (std::size_t i = 256; i < chips::kBoardCells; ++i) {
            if (levels[0].top[i] != 0x42 || levels[0].bottom[i] != 0x00) {
                std::cerr << "RLE continuation produced an incorrect layer\n";
                return 1;
            }
        }
        if (levels[0].fields.size() != 4 || levels[0].fields[2].id != 6 ||
            levels[0].fields[2].raw != Bytes{0xD8, 0xDB, 0xDA, 0x00} ||
            levels[0].fields[3].id != 0xFE ||
            levels[0].fields[3].raw != Bytes{0x10, 0x20, 0x30}) {
            std::cerr << "Known or unknown optional field bytes were not preserved\n";
            return 1;
        }
        if (!writePack(path, makeEmptyPack())) {
            std::cerr << "Could not write empty synthetic level pack\n";
            cleanup();
            return 1;
        }
        bool emptyRejected = false;
        try {
            (void)chips::loadLevelPack(path);
        } catch (const chips::LevelPackError& error) {
            emptyRejected = std::string(error.what()).find("contains no levels") != std::string::npos;
        }
        cleanup();
        if (!emptyRejected) {
            std::cerr << "Empty level pack was not rejected with a useful error\n";
            return 1;
        }
        std::cout << "Level-pack RLE and unknown-field checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        cleanup();
        std::cerr << "Level-pack check failed: " << error.what() << '\n';
        return 1;
    }
}
