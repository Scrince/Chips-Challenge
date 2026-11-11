#include "render_assets.hpp"

#include <array>
#include <cstdlib>
#include <iostream>

int main() {
    unsigned failures = 0;
    const auto checkDigits = [&failures](unsigned value, std::array<unsigned, 3> expected,
                                         const char* label) {
        if (chips::scoreboardDigits(value) != expected) {
            std::cerr << "scoreboard digit order mismatch for " << label << '\n';
            ++failures;
        }
    };
    checkDigits(3, {0, 0, 3}, "Level 3");
    checkDigits(4, {0, 0, 4}, "Level 3 chip requirement");
    checkDigits(100, {1, 0, 0}, "three-digit timer");
    checkDigits(149, {1, 4, 9}, "last campaign level");
    checkDigits(1007, {0, 0, 7}, "three-digit scoreboard wrap");
    if (chips::scoreboardDigits(8, true) != std::array<unsigned, 3>{10, 10, 8} ||
        chips::scoreboardDigits(0, true) != std::array<unsigned, 3>{10, 10, 0} ||
        chips::scoreboardDigits(11, true) != std::array<unsigned, 3>{10, 1, 1} ||
        chips::scoreboardDigits(116, true) != std::array<unsigned, 3>{1, 1, 6}) {
        std::cerr << "scoreboard leading zeroes should use subdued zero glyphs\n";
        ++failures;
    }
    if (chips::scoreboardDigits(100, false) != std::array<unsigned, 3>{10, 10, 10}) {
        std::cerr << "missing scoreboard values should select subdued blank glyphs\n";
        ++failures;
    }
    if (chips::scoreboardDigitBank(true) != 0 || chips::scoreboardDigitBank(false) != 12 ||
        chips::scoreboardChipsDigitBank(1) != 0 || chips::scoreboardChipsDigitBank(0) != 12 ||
        chips::scoreboardGlyphFrame(0, 0) != 23 ||
        chips::scoreboardGlyphFrame(0, 1) != 22 ||
        chips::scoreboardGlyphFrame(0, 10) != 13 ||
        chips::scoreboardGlyphFrame(12, 11) != 0) {
        std::cerr << "scoreboard selected the wrong original digit color bank\n";
        ++failures;
    }

    if (!chips::playerCoveredByBlock(0x0A, 0x6F) ||
        !chips::playerCoveredByBlock(0x0A, 0x3F) ||
        chips::playerCoveredByBlock(0x0A, 0x02) ||
        chips::playerCoveredByBlock(0x0B, 0x6F)) {
        std::cerr << "block-over-Chip sprite occlusion does not match the original upper/lower layer order\n";
        ++failures;
    }
    constexpr std::array<std::uint8_t, 4> landTiles{0x6C, 0x6D, 0x6E, 0x6F};
    constexpr std::array<std::uint8_t, 4> swimTiles{0x3C, 0x3D, 0x3E, 0x3F};
    for (std::size_t frame = 0; frame < landTiles.size(); ++frame) {
        if (chips::polishedPlayerPoseFrame(landTiles[frame]) != static_cast<int>(frame) ||
            chips::polishedPlayerPoseIsSwimming(landTiles[frame]) ||
            !chips::usesTransparentLandPlayerSprite(landTiles[frame]) ||
            chips::polishedPlayerPoseFrame(swimTiles[frame]) != static_cast<int>(frame) ||
            chips::usesTransparentLandPlayerSprite(swimTiles[frame]) ||
            !chips::polishedPlayerPoseIsSwimming(swimTiles[frame])) {
            std::cerr << "polished Chip pose mapping does not preserve original directional tile IDs\n";
            ++failures;
        }
    }
    if (chips::polishedPlayerPoseFrame(0x6B) != -1 ||
        chips::polishedPlayerPoseFrame(0x70) != -1) {
        std::cerr << "non-player tile was mapped to a polished Chip pose\n";
        ++failures;
    }
    const auto squareFit = chips::fitSpriteRect(10.0F, 20.0F, 64.0F, 64.0F,
                                                543.0F, 724.0F);
    if (squareFit.x != 18.0F || squareFit.y != 20.0F ||
        squareFit.w != 48.0F || squareFit.h != 64.0F) {
        std::cerr << "Chip pose does not stay centered and 3:4 in a square board tile\n";
        ++failures;
    }
    const auto wideFit = chips::fitSpriteRect(0.0F, 0.0F, 100.0F, 50.0F,
                                             543.0F, 724.0F);
    if (wideFit.x != 31.25F || wideFit.y != 0.0F ||
        wideFit.w != 37.5F || wideFit.h != 50.0F) {
        std::cerr << "Chip pose does not stay centered and undistorted in a wide slot\n";
        ++failures;
    }
    const auto invalidFit = chips::fitSpriteRect(3.0F, 4.0F, 0.0F, 64.0F,
                                                 543.0F, 724.0F);
    if (invalidFit.x != 3.0F || invalidFit.y != 4.0F ||
        invalidFit.w != 0.0F || invalidFit.h != 0.0F) {
        std::cerr << "invalid sprite destination should return an empty rectangle\n";
        ++failures;
    }
    if (chips::showLossIllustration(true) || !chips::showLossIllustration(false)) {
        std::cerr << "timeout result incorrectly selects an in-world death illustration\n";
        ++failures;
    }

    if (!chips::shouldInterpolateTileMotion(4, 7, 5, 7, false) ||
        chips::shouldInterpolateTileMotion(4, 7, 5, 7, true) ||
        chips::shouldInterpolateTileMotion(4, 7, 6, 7, false) ||
        chips::shouldInterpolateTileMotion(4, 7, 5, 8, false)) {
        std::cerr << "tile motion interpolation does not distinguish a teleport from an adjacent step\n";
        ++failures;
    }

    for (unsigned upper = 0; upper <= 0xFF; ++upper) {
        for (unsigned lower = 0; lower <= 0xFF; ++lower) {
            const auto layers = chips::resolveTileDrawLayers(
                static_cast<std::uint8_t>(upper), static_cast<std::uint8_t>(lower));
            const auto expectedLower = lower;
            const bool expectedUpper = upper != 0;
            if (layers.lower != expectedLower || layers.drawUpper != expectedUpper) {
                std::cerr << "layer resolution mismatch for upper 0x" << std::hex << upper
                          << " lower 0x" << lower << std::dec << '\n';
                ++failures;
            }
        }
    }
    for (unsigned tile = 0; tile <= 0xFF; ++tile) {
        const auto byteTile = static_cast<std::uint8_t>(tile);
        const bool expectedTransparentOverlay =
            (tile >= 0x40 && tile <= 0x63) || (tile >= 0x6C && tile <= 0x6F);
        if (chips::usesTransparentSpriteOverlay(byteTile) != expectedTransparentOverlay) {
            std::cerr << "tile 0x" << std::hex << tile
                      << " selected the wrong transparent-overlay path\n" << std::dec;
            ++failures;
        }
        const bool expectedAtlas = tile <= 0x6F;
        const bool actualAtlas = chips::hasOriginalAtlasTile(static_cast<std::uint8_t>(tile));
        if (actualAtlas != expectedAtlas) {
            std::cerr << "tile 0x" << std::hex << tile << std::dec
                      << " has incorrect original-atlas availability\n";
            ++failures;
        } else if (expectedAtlas) {
            const auto cell = chips::originalAtlasCell(static_cast<std::uint8_t>(tile));
            if (cell.column != static_cast<int>(tile >> 4U) ||
                cell.row != static_cast<int>(tile & 0x0FU)) {
                std::cerr << "tile 0x" << std::hex << tile << std::dec
                          << " maps to the wrong original-atlas cell\n";
                ++failures;
            }
        }
        const int expectedFrame = tile >= 0x40 && tile <= 0x6F
            ? static_cast<int>(tile - 0x40) : -1;
        const int actualFrame = chips::originalSpriteFrame(static_cast<std::uint8_t>(tile));
        if (actualFrame != expectedFrame) {
            std::cerr << "tile 0x" << std::hex << tile << " selected original sprite frame "
                      << std::dec << actualFrame << ", expected " << expectedFrame << '\n';
            ++failures;
        }
        const int expectedCreatureFrame = tile >= 0x40 && tile <= 0x63
            ? static_cast<int>(tile - 0x40) : -1;
        const int actualCreatureFrame = chips::cleanedCreatureFrame(static_cast<std::uint8_t>(tile));
        if (actualCreatureFrame != expectedCreatureFrame) {
            std::cerr << "tile 0x" << std::hex << tile << " selected cleaned creature frame "
                      << std::dec << actualCreatureFrame << ", expected "
                      << expectedCreatureFrame << '\n';
            ++failures;
        }
        const int expectedItemFrame = tile >= 0x64 && tile <= 0x6B
            ? static_cast<int>(tile - 0x64) : -1;
        const int actualItemFrame = chips::cleanedKeyBootFrame(static_cast<std::uint8_t>(tile));
        if (actualItemFrame != expectedItemFrame) {
            std::cerr << "tile 0x" << std::hex << tile << " selected cleaned key/boot frame "
                      << std::dec << actualItemFrame << ", expected " << expectedItemFrame << '\n';
            ++failures;
        }
        const int expectedPreferredItemFrame = expectedItemFrame >= 0 ? expectedFrame : -1;
        if (chips::preferredItemSpriteFrame(static_cast<std::uint8_t>(tile)) !=
            expectedPreferredItemFrame) {
            std::cerr << "tile 0x" << std::hex << tile
                      << " did not select its canonical original key/boot frame\n" << std::dec;
            ++failures;
        }
    }
    for (unsigned tile = 0; tile <= 0xFF; ++tile) {
        const auto creatureFrame = chips::cleanedCreatureFrame(static_cast<std::uint8_t>(tile));
        if (creatureFrame >= 0) {
            const auto cell = chips::cleanedCreatureCell(static_cast<std::uint8_t>(tile));
            if (cell.column != creatureFrame % 6 || cell.row != creatureFrame / 6) {
                std::cerr << "cleaned creature frame for tile 0x" << std::hex << tile
                          << " maps to the wrong atlas cell\n";
                ++failures;
            }
        }
        const auto itemFrame = chips::cleanedKeyBootFrame(static_cast<std::uint8_t>(tile));
        if (itemFrame >= 0) {
            const auto cell = chips::cleanedKeyBootCell(static_cast<std::uint8_t>(tile));
            if (cell.column != itemFrame % 4 || cell.row != itemFrame / 4) {
                std::cerr << "cleaned item frame for tile 0x" << std::hex << tile
                          << " maps to the wrong atlas cell\n";
                ++failures;
            }
        }
    }
    for (unsigned tile = 0x40; tile <= 0x6F; ++tile) {
        const auto underIce = chips::resolveTileDrawLayers(
            static_cast<std::uint8_t>(tile), 0x0C);
        if (underIce.lower != 0x0C || !underIce.drawUpper) {
            std::cerr << "sprite does not preserve its Ice underlay for tile 0x"
                      << std::hex << tile << std::dec << '\n';
            ++failures;
        }
        const auto underWater = chips::resolveTileDrawLayers(
            static_cast<std::uint8_t>(tile), 0x03);
        if (underWater.lower != 0x03 || !underWater.drawUpper) {
            std::cerr << "sprite does not preserve its Water underlay for tile 0x"
                      << std::hex << tile << std::dec << '\n';
            ++failures;
        }
    }
    if (failures != 0) return EXIT_FAILURE;
    std::cout << "All original render sprite and item tile-selection checks passed.\n";
    return EXIT_SUCCESS;
}
