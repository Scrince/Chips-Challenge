#pragma once

#include <array>
#include <algorithm>
#include <cstdint>

namespace chips {

struct TileDrawLayers {
    std::uint8_t lower{};
    bool drawUpper{};
};

struct TileAtlasCell {
    int column{};
    int row{};
};

constexpr bool hasOriginalAtlasTile(std::uint8_t tile) noexcept {
    return tile <= 0x6F;
}

constexpr TileAtlasCell originalAtlasCell(std::uint8_t tile) noexcept {
    return {static_cast<int>(tile >> 4U), static_cast<int>(tile & 0x0FU)};
}

constexpr std::array<unsigned, 3> scoreboardDigits(unsigned value) noexcept {
    value %= 1000U;
    return {value / 100U, (value / 10U) % 10U, value % 10U};
}

constexpr std::array<unsigned, 3> scoreboardDigits(unsigned value, bool hasValue) noexcept {
    if (!hasValue) return {10U, 10U, 10U};
    auto digits = scoreboardDigits(value);
    for (std::size_t position = 0; position < 2; ++position) {
        if (digits[position] != 0U) break;
        digits[position] = 10U; // Use the subdued unlit-segment glyph for leading zeroes.
    }
    return digits;
}

constexpr int scoreboardDigitBank(bool hasTimeLimit) noexcept {
    // The original DIB strip is bottom-up: its green bank is at raw index 0,
    // while the top-down PNG places that bank in the lower half of the image.
    return hasTimeLimit ? 0 : 12;
}

constexpr int scoreboardGlyphFrame(int digitBank, unsigned digit) noexcept {
    return 23 - digitBank - static_cast<int>(digit);
}

constexpr bool hiddenBoardTile(std::uint8_t tile) noexcept {
    return tile == 0x05 || tile == 0x2C || tile == 0x2E;
}

constexpr bool isPlayerPoseTile(std::uint8_t tile) noexcept {
    return (tile >= 0x3C && tile <= 0x3F) || (tile >= 0x6C && tile <= 0x6F);
}

constexpr bool usesTransparentLandPlayerSprite(std::uint8_t tile) noexcept {
    return tile >= 0x6C && tile <= 0x6F;
}

constexpr bool usesTransparentSpriteOverlay(std::uint8_t tile) noexcept {
    return (tile >= 0x40 && tile <= 0x63) || usesTransparentLandPlayerSprite(tile);
}

// Polished Chip art follows the original four-direction tile order:
// north, west, south, east. Land and swimming sprites share those facings.
constexpr int polishedPlayerPoseFrame(std::uint8_t tile) noexcept {
    return isPlayerPoseTile(tile) ? static_cast<int>(tile & 0x03U) : -1;
}

constexpr bool polishedPlayerPoseIsSwimming(std::uint8_t tile) noexcept {
    return tile >= 0x3C && tile <= 0x3F;
}

// MoveBlock leaves Chip's sprite in Lower when a block lands on him. The
// original compositor draws Upper over Lower, so the block hides Chip.
constexpr bool playerCoveredByBlock(std::uint8_t upper, std::uint8_t lower) noexcept {
    return upper == 0x0A && isPlayerPoseTile(lower);
}

constexpr bool showLossIllustration(bool timedOut) noexcept {
    return !timedOut;
}

// The map gallery renders the authored map layers verbatim: lower/background
// tile first, then any non-floor upper/foreground tile. Keep this raw for map
// previews and board tiles so every cell matches the gallery.
constexpr TileDrawLayers resolveTileDrawLayers(std::uint8_t upper,
                                                std::uint8_t lower) noexcept {
    return {lower, upper != 0x00};
}

// Original transparent sprite frames are extracted using the source
// DrawTile image/mask pair and laid out row-major from tile 0x40.
constexpr int originalSpriteFrame(std::uint8_t tile) noexcept {
    if (tile >= 0x40 && tile <= 0x6F)
        return static_cast<int>(tile - 0x40);
    return -1;
}

// Checked-in cleanup sheets preserve the original tile-ID order. The
// creature sheet is a 6x6 atlas for IDs 0x40-0x63; the item sheet is a 4x2
// atlas for keys 0x64-0x67 and boots 0x68-0x6B.
constexpr int cleanedCreatureFrame(std::uint8_t tile) noexcept {
    return tile >= 0x40 && tile <= 0x63 ? static_cast<int>(tile - 0x40) : -1;
}

constexpr int cleanedKeyBootFrame(std::uint8_t tile) noexcept {
    return tile >= 0x64 && tile <= 0x6B ? static_cast<int>(tile - 0x64) : -1;
}

// The original extracted sprite atlas is the canonical source for pickups.
// Its frames are kept at the same 0x40-based tile-ID offset.
constexpr int preferredItemSpriteFrame(std::uint8_t tile) noexcept {
    return cleanedKeyBootFrame(tile) >= 0 ? originalSpriteFrame(tile) : -1;
}

struct SpriteAtlasCell {
    int column{};
    int row{};
};

struct SpriteFitRect {
    float x{};
    float y{};
    float w{};
    float h{};
};

// Fit a sprite into a tile or UI slot without changing its authored aspect
// ratio, centering any unused space. Invalid dimensions produce an empty rect.
constexpr SpriteFitRect fitSpriteRect(float x, float y, float width, float height,
                                      float sourceWidth, float sourceHeight) noexcept {
    if (width <= 0.0F || height <= 0.0F || sourceWidth <= 0.0F || sourceHeight <= 0.0F)
        return {x, y, 0.0F, 0.0F};
    const float scale = std::min(width / sourceWidth, height / sourceHeight);
    const float fittedWidth = sourceWidth * scale;
    const float fittedHeight = sourceHeight * scale;
    return {x + (width - fittedWidth) * 0.5F,
            y + (height - fittedHeight) * 0.5F,
            fittedWidth, fittedHeight};
}

constexpr SpriteAtlasCell cleanedCreatureCell(std::uint8_t tile) noexcept {
    const int frame = cleanedCreatureFrame(tile);
    return frame < 0 ? SpriteAtlasCell{-1, -1} : SpriteAtlasCell{frame % 6, frame / 6};
}

constexpr SpriteAtlasCell cleanedKeyBootCell(std::uint8_t tile) noexcept {
    const int frame = cleanedKeyBootFrame(tile);
    return frame < 0 ? SpriteAtlasCell{-1, -1} : SpriteAtlasCell{frame % 4, frame / 4};
}

constexpr bool shouldInterpolateTileMotion(int fromX, int fromY, int toX, int toY,
                                           bool teleported) noexcept {
    return !teleported && (fromX == toX ? (fromY - toY == 1 || toY - fromY == 1)
        : fromY == toY && (fromX - toX == 1 || toX - fromX == 1));
}

} // namespace chips
