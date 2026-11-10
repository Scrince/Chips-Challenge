#include "renderer.hpp"
#include "render_assets.hpp"
#include "viewport.hpp"

#include <algorithm>
#include <array>
#include <cstdarg>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <string>

namespace chips_native {
namespace {
constexpr float kPanelAspect = 154.0F / 300.0F;
constexpr float kBoardPanelGapInCells = 0.3F;
constexpr int kViewTiles = 9;
constexpr std::size_t kLevelSelectorColumns = 13;
constexpr std::size_t kLevelSelectorRows = 12;
constexpr std::size_t kLevelSelectorPageSize = kLevelSelectorColumns * kLevelSelectorRows;
constexpr SDL_Color kBackground{18, 25, 32, 255};
constexpr SDL_Color kFloor{49, 62, 70, 255};
constexpr SDL_Color kWall{27, 39, 47, 255};
constexpr SDL_Color kChip{247, 207, 76, 255};
constexpr SDL_Color kPlayer{49, 193, 224, 255};
constexpr SDL_Color kObject{177, 130, 220, 255};
constexpr SDL_Color kCard{28, 41, 52, 255};
constexpr SDL_Color kCardEdge{58, 160, 168, 255};
constexpr SDL_Color kCardCaption{20, 77, 88, 255};
constexpr SDL_Color kCardText{232, 240, 242, 255};
constexpr SDL_Color kCardMuted{164, 184, 190, 255};
constexpr SDL_Color kCardSelection{18, 105, 126, 255};

struct BoardLayout {
    int cell{};
    int width{};
    int height{};
    int originX{};
    int originY{};
    int cameraX{};
    int cameraY{};
    int playerX{};
    int playerY{};
};

BoardLayout boardLayout(const App& app, int outputWidth, int outputHeight) {
    const float widthInCells = static_cast<float>(kViewTiles) +
        static_cast<float>(kViewTiles) * kPanelAspect + kBoardPanelGapInCells;
    const int availableWidth = std::max(1, outputWidth - 48);
    const int availableHeight = std::max(1, outputHeight - 104);
    const int cell = std::max(1, std::min(
        static_cast<int>(static_cast<float>(availableWidth) / widthInCells),
        availableHeight / kViewTiles));
    const int width = cell * kViewTiles;
    const int height = cell * kViewTiles;
    const int gap = std::max(8, static_cast<int>(static_cast<float>(cell) * kBoardPanelGapInCells));
    const int panelWidth = static_cast<int>(static_cast<float>(height) * kPanelAspect);
    const int groupWidth = width + gap + panelWidth;
    const int originX = std::max(12, (outputWidth - groupWidth) / 2);
    const int originY = 54 + std::max(0, (outputHeight - 74 - height) / 2);
    const int playerX = app.game.playerPosition % chips::kBoardWidth;
    const int playerY = app.game.playerPosition / chips::kBoardWidth;
    const auto view = playerViewport(playerX, playerY, chips::kBoardWidth, chips::kBoardHeight,
                                     kViewTiles, kViewTiles);
    return {cell, width, height, originX, originY, view.left, view.top, playerX, playerY};
}

std::array<std::uint8_t, 7> bitmapGlyph(unsigned char character) {
    switch (character) {
        case 'a': return {0, 0, 14, 1, 15, 17, 15};
        case 'b': return {16, 16, 30, 17, 17, 17, 30};
        case 'c': return {0, 0, 14, 17, 16, 17, 14};
        case 'd': return {1, 1, 15, 17, 17, 17, 15};
        case 'e': return {0, 0, 14, 17, 31, 16, 14};
        case 'f': return {6, 9, 8, 28, 8, 8, 8};
        case 'g': return {0, 0, 15, 17, 15, 1, 14};
        case 'h': return {16, 16, 30, 17, 17, 17, 17};
        case 'i': return {4, 0, 12, 4, 4, 4, 14};
        case 'j': return {2, 0, 6, 2, 2, 18, 12};
        case 'k': return {16, 16, 18, 20, 24, 20, 18};
        case 'l': return {12, 4, 4, 4, 4, 4, 14};
        case 'm': return {0, 0, 26, 21, 21, 21, 21};
        case 'n': return {0, 0, 30, 17, 17, 17, 17};
        case 'o': return {0, 0, 14, 17, 17, 17, 14};
        case 'p': return {0, 0, 30, 17, 30, 16, 16};
        case 'q': return {0, 0, 15, 17, 15, 1, 1};
        case 'r': return {0, 0, 22, 25, 16, 16, 16};
        case 's': return {0, 0, 15, 16, 14, 1, 30};
        case 't': return {8, 8, 28, 8, 8, 9, 6};
        case 'u': return {0, 0, 17, 17, 17, 19, 13};
        case 'v': return {0, 0, 17, 17, 17, 10, 4};
        case 'w': return {0, 0, 17, 17, 21, 21, 10};
        case 'x': return {0, 0, 17, 10, 4, 10, 17};
        case 'y': return {0, 0, 17, 17, 15, 1, 14};
        case 'z': return {0, 0, 31, 2, 4, 8, 31};
        default: break;
    }
    switch (static_cast<unsigned char>(std::toupper(character))) {
        case 'A': return {14, 17, 17, 31, 17, 17, 17};
        case 'B': return {30, 17, 17, 30, 17, 17, 30};
        case 'C': return {14, 17, 16, 16, 16, 17, 14};
        case 'D': return {30, 17, 17, 17, 17, 17, 30};
        case 'E': return {31, 16, 16, 30, 16, 16, 31};
        case 'F': return {31, 16, 16, 30, 16, 16, 16};
        case 'G': return {14, 17, 16, 23, 17, 17, 15};
        case 'H': return {17, 17, 17, 31, 17, 17, 17};
        case 'I': return {14, 4, 4, 4, 4, 4, 14};
        case 'J': return {7, 2, 2, 2, 18, 18, 12};
        case 'K': return {17, 18, 20, 24, 20, 18, 17};
        case 'L': return {16, 16, 16, 16, 16, 16, 31};
        case 'M': return {17, 27, 21, 21, 17, 17, 17};
        case 'N': return {17, 25, 25, 21, 19, 19, 17};
        case 'O': return {14, 17, 17, 17, 17, 17, 14};
        case 'P': return {30, 17, 17, 30, 16, 16, 16};
        case 'Q': return {14, 17, 17, 17, 21, 18, 13};
        case 'R': return {30, 17, 17, 30, 20, 18, 17};
        case 'S': return {15, 16, 16, 14, 1, 1, 30};
        case 'T': return {31, 4, 4, 4, 4, 4, 4};
        case 'U': return {17, 17, 17, 17, 17, 17, 14};
        case 'V': return {17, 17, 17, 17, 17, 10, 4};
        case 'W': return {17, 17, 17, 21, 21, 21, 10};
        case 'X': return {17, 17, 10, 4, 10, 17, 17};
        case 'Y': return {17, 17, 10, 4, 4, 4, 4};
        case 'Z': return {31, 1, 2, 4, 8, 16, 31};
        case '0': return {14, 17, 19, 21, 25, 17, 14};
        case '1': return {4, 12, 4, 4, 4, 4, 14};
        case '2': return {14, 17, 1, 2, 4, 8, 31};
        case '3': return {30, 1, 1, 14, 1, 1, 30};
        case '4': return {2, 6, 10, 18, 31, 2, 2};
        case '5': return {31, 16, 16, 30, 1, 1, 30};
        case '6': return {14, 16, 16, 30, 17, 17, 14};
        case '7': return {31, 1, 2, 4, 8, 8, 8};
        case '8': return {14, 17, 17, 14, 17, 17, 14};
        case '9': return {14, 17, 17, 15, 1, 1, 14};
        case '!': return {4, 4, 4, 4, 4, 0, 4};
        case '?': return {14, 17, 1, 2, 4, 0, 4};
        case '.': return {0, 0, 0, 0, 0, 6, 6};
        case ',': return {0, 0, 0, 0, 6, 6, 4};
        case ':': return {0, 6, 6, 0, 6, 6, 0};
        case ';': return {0, 6, 6, 0, 6, 6, 4};
        case '\'': return {4, 4, 2, 0, 0, 0, 0};
        case '"': return {10, 10, 5, 0, 0, 0, 0};
        case '-': return {0, 0, 0, 31, 0, 0, 0};
        case '_': return {0, 0, 0, 0, 0, 0, 31};
        case '+': return {0, 4, 4, 31, 4, 4, 0};
        case '=': return {0, 31, 0, 31, 0, 0, 0};
        case '/': return {1, 2, 2, 4, 8, 8, 16};
        case '\\': return {16, 8, 8, 4, 2, 2, 1};
        case '%': return {17, 2, 4, 8, 16, 17, 0};
        case '[': return {14, 8, 8, 8, 8, 8, 14};
        case ']': return {14, 2, 2, 2, 2, 2, 14};
        case '(': return {2, 4, 8, 8, 8, 4, 2};
        case ')': return {8, 4, 2, 2, 2, 4, 8};
        case '<': return {2, 4, 8, 16, 8, 4, 2};
        case '>': return {8, 4, 2, 1, 2, 4, 8};
        case '*': return {0, 17, 10, 31, 10, 17, 0};
        case ' ': return {};
        default: return {31, 17, 5, 9, 4, 0, 4};
    }
}

void drawBitmapText(SDL_Renderer* renderer, float x, float y, const char* text, float pixelSize) {
    if (!text || pixelSize <= 0.0F) return;
    float cursor = x;
    std::vector<SDL_FRect> runs;
    for (const auto* character = reinterpret_cast<const unsigned char*>(text); *character; ++character) {
        const auto rows = bitmapGlyph(*character);
        for (std::size_t row = 0; row < rows.size(); ++row) {
            std::size_t column = 0;
            while (column < 5) {
                if ((rows[row] & (1U << (4U - static_cast<unsigned>(column)))) == 0) {
                    ++column;
                    continue;
                }
                const auto start = column;
                do {
                    ++column;
                } while (column < 5 &&
                         (rows[row] & (1U << (4U - static_cast<unsigned>(column)))) != 0);
                runs.push_back(SDL_FRect{
                    cursor + static_cast<float>(start) * pixelSize,
                    y + static_cast<float>(row) * pixelSize,
                    static_cast<float>(column - start) * pixelSize,
                    pixelSize,
                });
            }
        }
        cursor += 6.0F * pixelSize;
    }
    if (!runs.empty())
        SDL_RenderFillRects(renderer, runs.data(), static_cast<int>(runs.size()));
}

void setColor(SDL_Renderer* renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void drawTerrainAccent(SDL_Renderer* renderer, std::uint8_t tile,
                       const SDL_FRect& bounds, Uint64 renderTime,
                       bool reducedMotion) {
    if (reducedMotion) return;
    if (tile != 0x02 && tile != 0x03 && tile != 0x04 && tile != 0x29 &&
        !(tile >= 0x64 && tile <= 0x6B)) return;

    const auto phase = static_cast<unsigned>((renderTime / 140U) & 3U);
    const float pixel = std::max(1.0F, std::floor(bounds.w / 14.0F));
    SDL_FRect glint{};
    SDL_Color color{};
    if (tile == 0x03) {
        // A short cyan ripple travels across the original water pixels.
        color = SDL_Color{126, 236, 247, 255};
        glint = {bounds.x + bounds.w * (0.18F + 0.18F * static_cast<float>(phase)),
                 bounds.y + bounds.h * (phase % 2U == 0U ? 0.28F : 0.66F), pixel * 2.0F, pixel};
    } else if (tile == 0x04) {
        // A warm highlight shifts inside the original flame silhouette.
        color = phase == 1U ? SDL_Color{255, 236, 135, 255} : SDL_Color{255, 178, 77, 255};
        glint = {bounds.x + bounds.w * (0.34F + 0.12F * static_cast<float>(phase)),
                 bounds.y + bounds.h * (0.24F + 0.07F * static_cast<float>((phase + 1U) % 3U)),
                 pixel, pixel * 2.0F};
    } else if (tile == 0x29) {
        // A cyan/blue spark orbits the teleporter's inner edge.
        color = phase % 2U == 0U ? SDL_Color{118, 238, 243, 255}
                                 : SDL_Color{177, 157, 255, 255};
        constexpr std::array<std::array<float, 2>, 4> points{{
            {{0.25F, 0.25F}}, {{0.68F, 0.27F}}, {{0.67F, 0.67F}}, {{0.24F, 0.66F}}}};
        glint = {bounds.x + bounds.w * points[phase][0],
                 bounds.y + bounds.h * points[phase][1], pixel, pixel};
    } else {
        // A brief pale sparkle gives the original pickups a little life.
        if (phase == 0U || phase == 3U) return;
        color = SDL_Color{255, 247, 190, 255};
        glint = {bounds.x + bounds.w * (phase == 1U ? 0.30F : 0.64F),
                 bounds.y + bounds.h * (phase == 1U ? 0.26F : 0.61F), pixel, pixel};
    }
    setColor(renderer, color);
    SDL_RenderFillRect(renderer, &glint);
}

void drawUiText(SDL_Renderer* renderer, float x, float y, const char* text) {
    drawBitmapText(renderer, x, y, text, 2.0F);
}

void drawUiTextFormat(SDL_Renderer* renderer, float x, float y, const char* format, ...) {
    std::array<char, 512> buffer{};
    va_list arguments;
    va_start(arguments, format);
    std::vsnprintf(buffer.data(), buffer.size(), format, arguments);
    va_end(arguments);
    drawUiText(renderer, x, y, buffer.data());
}

void drawSmallUiText(SDL_Renderer* renderer, float x, float y, const char* text) {
    drawBitmapText(renderer, x, y, text, 1.0F);
}

void drawMediumUiText(SDL_Renderer* renderer, float x, float y, const char* text) {
    drawBitmapText(renderer, x, y, text, 1.5F);
}

void drawMediumUiTextFormat(SDL_Renderer* renderer, float x, float y, const char* format, ...) {
    std::array<char, 512> buffer{};
    va_list arguments;
    va_start(arguments, format);
    std::vsnprintf(buffer.data(), buffer.size(), format, arguments);
    va_end(arguments);
    drawMediumUiText(renderer, x, y, buffer.data());
}

void drawSmallUiTextFormat(SDL_Renderer* renderer, float x, float y, const char* format, ...) {
    std::array<char, 512> buffer{};
    va_list arguments;
    va_start(arguments, format);
    std::vsnprintf(buffer.data(), buffer.size(), format, arguments);
    va_end(arguments);
    drawSmallUiText(renderer, x, y, buffer.data());
}

void drawSettingsSlider(SDL_Renderer* renderer, const SDL_FRect& bounds, float fraction) {
    fraction = std::clamp(fraction, 0.0F, 1.0F);
    setColor(renderer, SDL_Color{10, 17, 23, 255});
    SDL_RenderFillRect(renderer, &bounds);
    SDL_FRect fill{bounds.x + 2.0F, bounds.y + 2.0F,
                   std::max(0.0F, (bounds.w - 4.0F) * fraction), bounds.h - 4.0F};
    setColor(renderer, SDL_Color{35, 146, 156, 255});
    if (fill.w > 0.0F) SDL_RenderFillRect(renderer, &fill);
    const float knobX = bounds.x + 2.0F + (bounds.w - 4.0F) * fraction;
    SDL_FRect knob{knobX - 2.0F, bounds.y - 2.0F, 4.0F, bounds.h + 4.0F};
    setColor(renderer, SDL_Color{232, 240, 242, 255});
    SDL_RenderFillRect(renderer, &knob);
}

void drawActionButton(SDL_Renderer* renderer, const SDL_FRect& bounds, const char* label,
                      bool selected = false) {
    setColor(renderer, selected ? kCardSelection : kCardCaption);
    SDL_RenderFillRect(renderer, &bounds);
    SDL_FRect top{bounds.x, bounds.y, bounds.w, 2.0F};
    SDL_FRect left{bounds.x, bounds.y, 2.0F, bounds.h};
    SDL_FRect bottom{bounds.x, bounds.y + bounds.h - 2.0F, bounds.w, 2.0F};
    SDL_FRect right{bounds.x + bounds.w - 2.0F, bounds.y, 2.0F, bounds.h};
    setColor(renderer, selected ? SDL_Color{129, 218, 220, 255} : kCardEdge);
    SDL_RenderFillRect(renderer, &top);
    SDL_RenderFillRect(renderer, &left);
    SDL_RenderFillRect(renderer, &bottom);
    SDL_RenderFillRect(renderer, &right);
    const float textWidth = static_cast<float>(std::char_traits<char>::length(label)) * 12.0F;
    setColor(renderer, kCardText);
    drawUiText(renderer, bounds.x + (bounds.w - textWidth) * 0.5F, bounds.y + 5.0F, label);
}

void drawDigitalSegments(SDL_Renderer* renderer, float x, float y, float width, float height,
                         unsigned value, bool hasValue) {
    static constexpr std::array<unsigned char, 10> segments{
        0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
        0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};
    const float digitWidth = width / 3.7F;
    const float thickness = std::max(2.0F, height * 0.13F);
    if (!hasValue) setColor(renderer, SDL_Color{32, 58, 36, 255});
    const auto numerals = chips::scoreboardDigits(value, hasValue);
    for (int digit = 0; digit < 3; ++digit) {
        const unsigned numeral = numerals[static_cast<std::size_t>(digit)];
        if (numeral > 10) continue;
        const unsigned char mask = numeral == 10 ? 0b01111111 : segments[numeral];
        const float left = x + static_cast<float>(digit) * digitWidth * 1.22F;
        const float half = height * 0.5F;
        const std::array<SDL_FRect, 7> parts{{
            {left + thickness, y, digitWidth - thickness * 2.0F, thickness},
            {left + digitWidth - thickness, y + thickness, thickness, half - thickness},
            {left + digitWidth - thickness, y + half, thickness, half - thickness},
            {left + thickness, y + height - thickness, digitWidth - thickness * 2.0F, thickness},
            {left, y + half, thickness, half - thickness},
            {left, y + thickness, thickness, half - thickness},
            {left + thickness, y + half - thickness * 0.5F, digitWidth - thickness * 2.0F, thickness}}};
        for (std::size_t i = 0; i < parts.size(); ++i)
            if (mask & (1U << i)) SDL_RenderFillRect(renderer, &parts[i]);
    }
}

void drawDigitalNumber(SDL_Renderer* renderer, SDL_Texture* digits, float x, float y,
                       float width, float height, unsigned value, int digitBank = 12,
                       bool hasValue = true) {
    if (!digits) {
        drawDigitalSegments(renderer, x, y, width, height, value, hasValue);
        return;
    }
    constexpr float sourceScale = 2.0F;
    constexpr float sourceWidth = 17.0F * sourceScale;
    constexpr float sourceHeight = 23.0F * sourceScale;
    const float digitHeight = height;
    const float digitWidth = digitHeight * 17.0F / 23.0F;
    const float gap = std::max(0.0F, (width - 3.0F * digitWidth) / 2.0F);
    const auto numerals = chips::scoreboardDigits(value, hasValue);
    for (int position = 0; position < 3; ++position) {
        const unsigned numeral = numerals[static_cast<std::size_t>(position)];
        const int frame = chips::scoreboardGlyphFrame(digitBank, numeral);
        SDL_FRect source{0.0F, static_cast<float>(frame) * sourceHeight,
                         sourceWidth, sourceHeight};
        SDL_FRect destination{x + static_cast<float>(position) * (digitWidth + gap),
                              y, digitWidth, digitHeight};
        SDL_RenderTexture(renderer, digits, &source, &destination);
    }
}

void drawWrappedUiText(SDL_Renderer* renderer, float x, float y,
                       const std::string& text, std::size_t maxChars, std::size_t maxLines) {
    std::istringstream words(text);
    std::string word;
    std::string line;
    std::size_t lineCount = 0;
    while (words >> word) {
        if (!line.empty() && line.size() + word.size() + 1 > maxChars) {
            drawUiText(renderer, x, y + static_cast<float>(lineCount * 16), line.c_str());
            if (++lineCount >= maxLines) {
                drawUiText(renderer, x + static_cast<float>(maxChars * 9 - 18),
                                    y + static_cast<float>((maxLines - 1) * 16), "...");
                return;
            }
            line.clear();
        }
        if (!line.empty()) line.push_back(' ');
        line += word;
    }
    if (!line.empty() && lineCount < maxLines)
        drawUiText(renderer, x, y + static_cast<float>(lineCount * 16), line.c_str());
}

void drawWrappedMediumText(SDL_Renderer* renderer, float x, float y,
                           const std::string& text, std::size_t maxChars, std::size_t maxLines) {
    std::istringstream words(text);
    std::string word;
    std::string line;
    std::size_t lineCount = 0;
    while (words >> word) {
        if (!line.empty() && line.size() + word.size() + 1 > maxChars) {
            drawMediumUiText(renderer, x, y + static_cast<float>(lineCount * 15), line.c_str());
            if (++lineCount >= maxLines) {
                drawMediumUiText(renderer, x + static_cast<float>(maxChars * 9 - 27),
                                 y + static_cast<float>((maxLines - 1) * 15), "...");
                return;
            }
            line.clear();
        }
        if (!line.empty()) line.push_back(' ');
        line += word;
    }
    if (!line.empty() && lineCount < maxLines)
        drawMediumUiText(renderer, x, y + static_cast<float>(lineCount * 15), line.c_str());
}

std::vector<std::string> wrapHintText(const std::string& text, std::size_t maxChars) {
    std::vector<std::string> lines;
    std::string line;
    std::size_t cursor = 0;
    while (cursor < text.size()) {
        const auto gapStart = cursor;
        while (cursor < text.size() && std::isspace(static_cast<unsigned char>(text[cursor]))) ++cursor;
        const auto gap = text.substr(gapStart, cursor - gapStart);
        const auto wordStart = cursor;
        while (cursor < text.size() && !std::isspace(static_cast<unsigned char>(text[cursor]))) ++cursor;
        if (wordStart == cursor) continue;
        const auto word = text.substr(wordStart, cursor - wordStart);

        if (line.empty()) {
            line = word;
        } else if (line.size() + gap.size() + word.size() <= maxChars) {
            line += gap + word;
        } else {
            lines.push_back(std::move(line));
            line = word;
        }
    }
    if (!line.empty()) lines.push_back(std::move(line));
    return lines;
}

void drawItalicBitmapText(SDL_Renderer* renderer, float x, float y,
                          const std::string& text, float pixelSize) {
    std::vector<SDL_FRect> runs;
    float cursor = x;
    for (const auto character : text) {
        const auto rows = bitmapGlyph(static_cast<unsigned char>(character));
        for (std::size_t row = 0; row < rows.size(); ++row) {
            const float shear = static_cast<float>(rows.size() - 1 - row) * pixelSize * 0.16F;
            std::size_t column = 0;
            while (column < 5) {
                if ((rows[row] & (1U << (4U - static_cast<unsigned>(column)))) == 0) {
                    ++column;
                    continue;
                }
                const auto start = column;
                do {
                    ++column;
                } while (column < 5 &&
                         (rows[row] & (1U << (4U - static_cast<unsigned>(column)))) != 0);
                runs.push_back({cursor + shear + static_cast<float>(start) * pixelSize,
                                y + static_cast<float>(row) * pixelSize,
                                static_cast<float>(column - start) * pixelSize, pixelSize});
            }
        }
        cursor += 6.0F * pixelSize;
    }
    if (!runs.empty())
        SDL_RenderFillRects(renderer, runs.data(), static_cast<int>(runs.size()));
}

SDL_Color tileColor(std::uint8_t tile) {
    if (tile == 0x00) return kFloor;
    if (tile == 0x01 || tile == 0x05 || tile == 0x1F ||
        (tile >= 0x06 && tile <= 0x09) || tile == 0x2C || tile == 0x2E) return kWall;
    if (tile == 0x02 || tile == 0x22) return kChip;
    if (tile == 0x03 || tile == 0x0C || (tile >= 0x1A && tile <= 0x1D))
        return SDL_Color{70, 164, 202, 255};
    if (tile == 0x04 || tile == 0x2A) return SDL_Color{222, 91, 63, 255};
    if (tile == 0x0A || (tile >= 0x0E && tile <= 0x11))
        return SDL_Color{160, 114, 76, 255};
    if (tile == 0x0B || tile == 0x1E) return SDL_Color{138, 93, 54, 255};
    if ((tile >= 0x0D && tile <= 0x14) || tile == 0x32)
        return SDL_Color{76, 177, 139, 255};
    if (tile == 0x15 || tile == 0x39 || tile == 0x3A || tile == 0x3B)
        return SDL_Color{104, 200, 111, 255};
    if (tile >= 0x16 && tile <= 0x19) return SDL_Color{205, 99, 105, 255};
    if (tile >= 0x21 && tile <= 0x31) return kObject;
    if (tile >= 0x40 && tile <= 0x63) return SDL_Color{228, 111, 108, 255};
    if (tile >= 0x64 && tile <= 0x6B) return SDL_Color{239, 192, 83, 255};
    if (tile >= 0x6C && tile <= 0x6F) return kPlayer;
    return SDL_Color{106, 112, 128, 255};
}

bool drawAtlasTile(SDL_Renderer* renderer, SDL_Texture* atlas, std::uint8_t tile,
                   const SDL_FRect& destination) {
    if (!atlas || !chips::hasOriginalAtlasTile(tile)) return false;
    // The original game indexes this 416x512 sheet by tile ID: high nibble
    // selects the X column, low nibble selects the Y row.
    const auto cell = chips::originalAtlasCell(tile);
    constexpr float sourceCell = 64.0F;
    SDL_FRect source{static_cast<float>(cell.column) * sourceCell,
                     static_cast<float>(cell.row) * sourceCell,
                     sourceCell, sourceCell};
    return SDL_RenderTexture(renderer, atlas, &source, &destination);
}

bool drawOriginalTransparentSprite(SDL_Renderer* renderer, SDL_Texture* sprites,
                                   std::uint8_t tile, const SDL_FRect& tileBounds) {
    if (!sprites) return false;
    const int frame = chips::originalSpriteFrame(tile);
    if (frame < 0) return false;
    const SDL_FRect source{static_cast<float>(frame % 6) * 256.0F,
                           static_cast<float>(frame / 6) * 256.0F,
                           256.0F, 256.0F};
    return SDL_RenderTexture(renderer, sprites, &source, &tileBounds);
}

bool drawCleanedSprite(SDL_Renderer* renderer, SDL_Texture* sprites, std::uint8_t tile,
                       const SDL_FRect& destination) {
    if (!sprites) return false;
    const int creatureFrame = chips::cleanedCreatureFrame(tile);
    if (creatureFrame >= 0) {
        constexpr float cell = 256.0F;
        const auto sourceCell = chips::cleanedCreatureCell(tile);
        const SDL_FRect source{sourceCell.column * cell, sourceCell.row * cell, cell, cell};
        return SDL_RenderTexture(renderer, sprites, &source, &destination);
    }
    const int itemFrame = chips::cleanedKeyBootFrame(tile);
    if (itemFrame >= 0) {
        // The authored 4x2 item sheet is 1774x887, so each square source cell
        // is 443.5px. SDL's floating source rectangle keeps all eight cells
        // aligned without altering the ID-to-map-position relationship.
        constexpr float cell = 443.5F;
        const auto sourceCell = chips::cleanedKeyBootCell(tile);
        const SDL_FRect source{sourceCell.column * cell, sourceCell.row * cell, cell, cell};
        return SDL_RenderTexture(renderer, sprites, &source, &destination);
    }
    return false;
}

bool drawPolishedPlayerSprite(SDL_Renderer* renderer, SDL_Texture* landPoses,
                              SDL_Texture* swimPoses, std::uint8_t tile,
                              const SDL_FRect& destination) {
    const int frame = chips::polishedPlayerPoseFrame(tile);
    if (frame < 0) return false;
    SDL_Texture* texture = chips::polishedPlayerPoseIsSwimming(tile) ? swimPoses : landPoses;
    if (!texture) return false;
    constexpr float sourceFrameWidth = 543.0F;
    constexpr float sourceFrameHeight = 724.0F;
    const SDL_FRect source{frame * sourceFrameWidth, 0.0F,
                           sourceFrameWidth, sourceFrameHeight};
    const auto fittedBounds = chips::fitSpriteRect(destination.x, destination.y,
        destination.w, destination.h, sourceFrameWidth, sourceFrameHeight);
    const SDL_FRect fitted{fittedBounds.x, fittedBounds.y,
                           fittedBounds.w, fittedBounds.h};
    return SDL_RenderTexture(renderer, texture, &source, &fitted);
}

bool drawRuntimeTile(SDL_Renderer* renderer, SDL_Texture* atlas, SDL_Texture* originalSprites,
                     SDL_Texture*, SDL_Texture*, SDL_Texture*, SDL_Texture*,
                     std::uint8_t tile, const SDL_FRect& destination) {
    // Terrain and pickups match the gallery's direct atlas crops. Creatures
    // and Chip are transparent image/mask overlays in the original renderer;
    // draw their exact extracted source frames over the actual underlay.
    if (chips::usesTransparentSpriteOverlay(tile))
        return drawOriginalTransparentSprite(renderer, originalSprites, tile, destination);
    return drawAtlasTile(renderer, atlas, tile, destination);
}

void drawInventorySlot(SDL_Renderer* renderer, SDL_Texture* atlas, SDL_Texture* originalSprites,
                       SDL_Texture* polishedPlayerLand, SDL_Texture* polishedPlayerSwim,
                       SDL_Texture* cleanedItems,
                       float x, float y, float size, std::uint8_t icon, bool occupied) {
    SDL_FRect slot{x, y, size, size};
    setColor(renderer, occupied ? SDL_Color{150, 151, 146, 255}
                               : SDL_Color{171, 173, 168, 255});
    SDL_RenderFillRect(renderer, &slot);
    setColor(renderer, SDL_Color{237, 238, 230, 255});
    const float edge = size / 32.0F;
    SDL_FRect top{slot.x, slot.y, slot.w, edge};
    SDL_FRect left{slot.x, slot.y, edge, slot.h};
    setColor(renderer, SDL_Color{80, 83, 79, 255});
    SDL_FRect bottom{slot.x, slot.y + slot.h - edge, slot.w, edge};
    SDL_FRect right{slot.x + slot.w - edge, slot.y, edge, slot.h};
    setColor(renderer, SDL_Color{237, 238, 230, 255});
    SDL_RenderFillRect(renderer, &top);
    SDL_RenderFillRect(renderer, &left);
    setColor(renderer, SDL_Color{80, 83, 79, 255});
    SDL_RenderFillRect(renderer, &bottom);
    SDL_RenderFillRect(renderer, &right);
    if (occupied) {
        SDL_FRect sprite{x + 4.0F, y + 4.0F, size - 8.0F, size - 8.0F};
        (void)drawRuntimeTile(renderer, atlas, originalSprites, polishedPlayerLand,
                              polishedPlayerSwim, nullptr, cleanedItems, icon, sprite);
    }
}

} // namespace

LevelGridLayout levelGridLayout(int outputWidth, int outputHeight) {
    LevelGridLayout layout;
    layout.width = std::max(320.0F, std::min(680.0F, static_cast<float>(outputWidth) - 32.0F));
    layout.height = std::max(300.0F, std::min(560.0F, static_cast<float>(outputHeight) - 32.0F));
    layout.left = (static_cast<float>(outputWidth) - layout.width) * 0.5F;
    layout.top = (static_cast<float>(outputHeight) - layout.height) * 0.5F;
    layout.cell = std::min(48.0F, std::min((layout.width - 48.0F) / 13.0F,
                                           (layout.height - 124.0F) / 12.0F));
    layout.gridLeft = layout.left + 24.0F;
    layout.gridTop = layout.top + 84.0F;
    layout.previewBounds = {layout.left + layout.width - 152.0F,
                            layout.top + (layout.height - 128.0F) * 0.5F + 4.0F,
                            128.0F, 128.0F};
    layout.previousPageButton = {layout.left + layout.width - 84.0F, layout.top + 16.0F,
                                 28.0F, 24.0F};
    layout.nextPageButton = {layout.left + layout.width - 48.0F, layout.top + 16.0F,
                             28.0F, 24.0F};
    return layout;
}

float uiScaleForOutput(int outputWidth, int outputHeight, std::uint16_t userScalePercent) {
    const float widthScale = static_cast<float>(outputWidth) / 1280.0F;
    const float heightScale = static_cast<float>(outputHeight) / 820.0F;
    const float userScale = static_cast<float>(userScalePercent) / 100.0F;
    return std::clamp(std::min(widthScale, heightScale) * userScale, 0.75F, 3.5F);
}

SettingsOverlayLayout settingsOverlayLayout(int outputWidth, int outputHeight) {
    SettingsOverlayLayout layout;
    layout.width = std::min(520.0F, static_cast<float>(outputWidth) - 40.0F);
    layout.height = 520.0F;
    layout.left = (static_cast<float>(outputWidth) - layout.width) * 0.5F;
    layout.top = (static_cast<float>(outputHeight) - layout.height) * 0.5F;
    layout.effectsSlider = {layout.left + 300.0F, layout.top + 108.0F, 184.0F, 12.0F};
    layout.musicSlider = {layout.left + 300.0F, layout.top + 172.0F, 184.0F, 12.0F};
    layout.scaleSlider = {layout.left + 300.0F, layout.top + 204.0F, 184.0F, 12.0F};
    layout.effectsToggle = {layout.left + 16.0F, layout.top + 70.0F,
                            layout.width - 32.0F, 28.0F};
    layout.musicToggle = {layout.left + 16.0F, layout.top + 134.0F,
                          layout.width - 32.0F, 28.0F};
    layout.motionToggle = {layout.left + 16.0F, layout.top + 220.0F,
                           layout.width - 32.0F, 28.0F};
    layout.fullscreenToggle = {layout.left + 16.0F, layout.top + 244.0F,
                               layout.width - 32.0F, 28.0F};
    return layout;
}

BestTimesOverlayLayout bestTimesOverlayLayout(int outputWidth, int outputHeight) {
    BestTimesOverlayLayout layout;
    layout.width = std::min(520.0F, static_cast<float>(outputWidth) - 40.0F);
    layout.height = 360.0F;
    layout.left = (static_cast<float>(outputWidth) - layout.width) * 0.5F;
    layout.top = (static_cast<float>(outputHeight) - layout.height) * 0.5F;
    layout.previousPageButton = {layout.left + layout.width - 84.0F, layout.top + 16.0F, 28.0F, 24.0F};
    layout.nextPageButton = {layout.left + layout.width - 48.0F, layout.top + 16.0F, 28.0F, 24.0F};
    return layout;
}

ActionOverlayLayout pauseOverlayLayout(int outputWidth, int outputHeight) {
    ActionOverlayLayout layout;
    const float width = std::min(520.0F, static_cast<float>(outputWidth) - 40.0F);
    constexpr float height = 190.0F;
    const float left = (static_cast<float>(outputWidth) - width) * 0.5F;
    const float top = (static_cast<float>(outputHeight) - height) * 0.5F;
    layout.card = {left, top, width, height};
    layout.buttonCount = 3;
    for (std::size_t i = 0; i < layout.buttonCount; ++i)
        layout.buttons[i] = {left + 28.0F, top + 68.0F + static_cast<float>(i) * 30.0F,
                             width - 56.0F, 24.0F};
    return layout;
}

ActionOverlayLayout resultOverlayLayout(int outputWidth, int outputHeight, bool won) {
    ActionOverlayLayout layout;
    const float width = std::min(won ? 420.0F : 500.0F,
                                 std::max(1.0F, static_cast<float>(outputWidth) - 32.0F));
    const float height = std::min(won ? 306.0F : 196.0F,
                                  std::max(1.0F, static_cast<float>(outputHeight) - 32.0F));
    const float left = (static_cast<float>(outputWidth) - width) * 0.5F;
    const float top = (static_cast<float>(outputHeight) - height) * 0.5F;
    layout.card = {left, top, width, height};
    layout.buttonCount = won ? 1 : 2;
    constexpr float margin = 28.0F;
    constexpr float gap = 10.0F;
    const float buttonWidth = (width - margin * 2.0F - gap * static_cast<float>(layout.buttonCount - 1)) /
                              static_cast<float>(layout.buttonCount);
    for (std::size_t i = 0; i < layout.buttonCount; ++i)
        layout.buttons[i] = {left + margin + static_cast<float>(i) * (buttonWidth + gap),
                             won ? top + height - 42.0F : top + 148.0F,
                             won ? std::min(140.0F, buttonWidth) : buttonWidth, 28.0F};
    if (won) {
        layout.buttons[0].x = left + (width - layout.buttons[0].w) * 0.5F;
    }
    return layout;
}

bool hasAtlasMapping(std::uint8_t tile) {
    return chips::hasOriginalAtlasTile(tile);
}

std::optional<int> boardPositionAtOutput(const App& app, float x, float y,
                                         int outputWidth, int outputHeight) {
    if (!app.game.level || outputWidth <= 0 || outputHeight <= 0) return std::nullopt;
    const auto layout = boardLayout(app, outputWidth, outputHeight);
    const int viewX = static_cast<int>(x) - layout.originX;
    const int viewY = static_cast<int>(y) - layout.originY;
    if (viewX < 0 || viewY < 0 || viewX >= layout.width || viewY >= layout.height)
        return std::nullopt;
    const int mapX = layout.cameraX + viewX / layout.cell;
    const int mapY = layout.cameraY + viewY / layout.cell;
    return mapY * chips::kBoardWidth + mapX;
}

void draw(App& app) {
    int physicalWidth = 0;
    int physicalHeight = 0;
    SDL_GetCurrentRenderOutputSize(app.renderer, &physicalWidth, &physicalHeight);
    const float uiScale = uiScaleForOutput(physicalWidth, physicalHeight, app.profile.uiScalePercent);
    const int outputWidth = std::max(1, static_cast<int>(static_cast<float>(physicalWidth) / uiScale));
    const int outputHeight = std::max(1, static_cast<int>(static_cast<float>(physicalHeight) / uiScale));
    setColor(app.renderer, kBackground);
    SDL_RenderClear(app.renderer);
    SDL_SetRenderScale(app.renderer, uiScale, uiScale);

    const auto board = boardLayout(app, outputWidth, outputHeight);
    const int cell = board.cell;
    const int boardWidth = board.width;
    const int boardHeight = board.height;
    const int originX = board.originX;
    const int originY = board.originY;
    const auto& level = *app.game.level;
    const int cameraX = board.cameraX;
    const int cameraY = board.cameraY;
    const int playerX = board.playerX;
    const int playerY = board.playerY;
    const Uint64 renderTime = SDL_GetTicks();

    SDL_FRect boardFrame{static_cast<float>(originX - 5), static_cast<float>(originY - 5),
                         static_cast<float>(boardWidth + 10), static_cast<float>(boardHeight + 10)};
    setColor(app.renderer, SDL_Color{16, 128, 16, 255});
    SDL_RenderFillRect(app.renderer, &boardFrame);
    SDL_Rect boardClip{originX, originY, boardWidth, boardHeight};
    SDL_SetRenderClipRect(app.renderer, &boardClip);
    for (int y = 0; y < kViewTiles; ++y) {
        for (int x = 0; x < kViewTiles; ++x) {
            const auto index = static_cast<std::size_t>((cameraY + y) * chips::kBoardWidth + cameraX + x);
            const auto lower = app.game.bottom[index];
            const auto upper = app.game.top[index];
            const auto layers = chips::resolveTileDrawLayers(upper, lower);
            SDL_FRect tile{static_cast<float>(originX + x * cell),
                           static_cast<float>(originY + y * cell),
                           static_cast<float>(cell - 1), static_cast<float>(cell - 1)};
            if (!drawRuntimeTile(app.renderer, app.atlas,
                                 app.originalTransparentSprites, app.polishedPlayerLandPoses,
                                 app.polishedPlayerSwimPoses, app.cleanedCreatureSprites,
                                 app.cleanedKeyBootIcons, layers.lower, tile)) {
                setColor(app.renderer, tileColor(layers.lower));
                SDL_RenderFillRect(app.renderer, &tile);
            }
            if (layers.drawUpper) {
                SDL_FRect mark{tile.x + cell * 0.22F, tile.y + cell * 0.22F,
                               cell * 0.56F, cell * 0.56F};
                if (!drawRuntimeTile(app.renderer, app.atlas,
                                     app.originalTransparentSprites, app.polishedPlayerLandPoses,
                                     app.polishedPlayerSwimPoses, app.cleanedCreatureSprites,
                                     app.cleanedKeyBootIcons, upper, tile)) {
                    setColor(app.renderer, tileColor(upper));
                    SDL_RenderFillRect(app.renderer, &mark);
                }
            }
            const auto surfaceTile = layers.drawUpper ? upper : layers.lower;
            drawTerrainAccent(app.renderer, surfaceTile, tile, renderTime, app.profile.reducedMotion);
        }
    }
    if (app.game.mouseTargetPosition) {
        const int targetX = *app.game.mouseTargetPosition % chips::kBoardWidth - cameraX;
        const int targetY = *app.game.mouseTargetPosition / chips::kBoardWidth - cameraY;
        if (targetX >= 0 && targetX < kViewTiles && targetY >= 0 && targetY < kViewTiles) {
            const float inset = std::max(2.0F, static_cast<float>(cell) * 0.16F);
            SDL_FRect marker{static_cast<float>(originX + targetX * cell) + inset,
                             static_cast<float>(originY + targetY * cell) + inset,
                             static_cast<float>(cell) - inset * 2.0F,
                             static_cast<float>(cell) - inset * 2.0F};
            setColor(app.renderer, SDL_Color{71, 221, 192, 235});
            SDL_RenderRect(app.renderer, &marker);
        }
    }

    constexpr float kCreatureMoveAnimationMs = 90.0F;
    for (std::size_t i = 0; i < app.game.creatures.size(); ++i) {
        const auto& creature = app.game.creatures[i];
        if (!creature.alive) continue;
        const int currentX = creature.position % chips::kBoardWidth - cameraX;
        const int currentY = creature.position / chips::kBoardWidth - cameraY;
        if (currentX < 0 || currentX >= kViewTiles || currentY < 0 || currentY >= kViewTiles) continue;
        float renderX = static_cast<float>(currentX);
        float renderY = static_cast<float>(currentY);
        if (!app.profile.reducedMotion && i < app.creatureAnimations.size()) {
            const auto& animation = app.creatureAnimations[i];
            if (animation.to == creature.position && animation.from >= 0) {
                const int fromX = animation.from % chips::kBoardWidth - cameraX;
                const int fromY = animation.from / chips::kBoardWidth - cameraY;
                const int distance = std::abs(fromX - currentX) + std::abs(fromY - currentY);
                if (chips::shouldInterpolateTileMotion(fromX, fromY, currentX, currentY,
                                                       animation.teleported) && distance == 1) {
                    const float progress = std::clamp(
                        static_cast<float>(renderTime - animation.start) / kCreatureMoveAnimationMs,
                        0.0F, 1.0F);
                    renderX = static_cast<float>(fromX) + static_cast<float>(currentX - fromX) * progress;
                    renderY = static_cast<float>(fromY) + static_cast<float>(currentY - fromY) * progress;
                }
            }
        }
        SDL_FRect sprite{static_cast<float>(originX) + std::round(renderX * static_cast<float>(cell)),
                         static_cast<float>(originY) + std::round(renderY * static_cast<float>(cell)),
                         static_cast<float>(cell - 1), static_cast<float>(cell - 1)};
        if (!drawRuntimeTile(app.renderer, app.atlas,
                             app.originalTransparentSprites, app.polishedPlayerLandPoses,
                             app.polishedPlayerSwimPoses, app.cleanedCreatureSprites,
                             app.cleanedKeyBootIcons, creature.tile, sprite)) {
            SDL_FRect mark{sprite.x + cell * 0.16F, sprite.y + cell * 0.16F,
                           cell * 0.68F, cell * 0.68F};
            setColor(app.renderer, tileColor(creature.tile));
            SDL_RenderFillRect(app.renderer, &mark);
        }
    }

    float renderX = static_cast<float>(playerX - cameraX);
    float renderY = static_cast<float>(playerY - cameraY);
    if (!app.profile.reducedMotion &&
        app.playerAnimationTo == app.game.playerPosition && app.playerAnimationFrom >= 0) {
        const int fromX = app.playerAnimationFrom % chips::kBoardWidth - cameraX;
        const int fromY = app.playerAnimationFrom / chips::kBoardWidth - cameraY;
        const int distance = std::abs(fromX - playerX) + std::abs(fromY - playerY);
        if (chips::shouldInterpolateTileMotion(fromX, fromY, playerX, playerY,
                                               app.playerAnimationTeleported) && distance == 1) {
            constexpr float kPlayerMoveAnimationMs = 90.0F;
            const float progress = std::clamp(
                static_cast<float>(SDL_GetTicks() - app.playerAnimationStart) / kPlayerMoveAnimationMs,
                0.0F, 1.0F);
            renderX = static_cast<float>(fromX) + static_cast<float>(playerX - fromX) * progress;
            renderY = static_cast<float>(fromY) + static_cast<float>(playerY - fromY) * progress;
        }
    }
    SDL_FRect player{static_cast<float>(originX) + std::round(renderX * static_cast<float>(cell)),
                     static_cast<float>(originY) + std::round(renderY * static_cast<float>(cell)),
                     static_cast<float>(cell - 1), static_cast<float>(cell - 1)};
    const auto playerTile = app.game.playerDisplayTile();
    const auto playerCell = static_cast<std::size_t>(app.game.playerPosition);
    const bool playerCovered = app.game.playerCoveredByCreature || chips::playerCoveredByBlock(
        app.game.top[playerCell], app.game.bottom[playerCell]);
    if (!playerCovered) {
        // Chip is an image/mask overlay in the original renderer. Drawing the
        // opaque atlas cell paints its baked-in floor over Ice and ForceFloor;
        // use the source transparent sprite for land poses so the actual tile
        // under Chip remains visible, as it does in the reference game.
        const bool playerDrawn = chips::usesTransparentLandPlayerSprite(playerTile)
            ? drawOriginalTransparentSprite(app.renderer, app.originalTransparentSprites,
                                            playerTile, player)
            : drawAtlasTile(app.renderer, app.atlas, playerTile, player);
        if (!playerDrawn) {
            SDL_FRect fallback{player.x + cell * 0.16F, player.y + cell * 0.16F,
                               cell * 0.68F, cell * 0.68F};
            setColor(app.renderer, kPlayer);
            SDL_RenderFillRect(app.renderer, &fallback);
        }
    }
    SDL_SetRenderClipRect(app.renderer, nullptr);

    // Compact desktop menu row styled to match the modern slate overlays.
    setColor(app.renderer, kCard);
    SDL_FRect menuStrip{0.0F, 0.0F, static_cast<float>(outputWidth), 26.0F};
    SDL_RenderFillRect(app.renderer, &menuStrip);
    setColor(app.renderer, kCardEdge);
    SDL_FRect menuRule{0.0F, 24.0F, static_cast<float>(outputWidth), 2.0F};
    SDL_RenderFillRect(app.renderer, &menuRule);
    constexpr std::array<float, 4> menuLeft{0.0F, 80.0F, 170.0F, 260.0F};
    constexpr std::array<float, 4> menuWidth{80.0F, 90.0F, 90.0F, 72.0F};
    constexpr std::array<float, 4> menuTextInset{12.0F, 12.0F, 20.0F, 20.0F};
    constexpr std::array<const char*, 4> menuLabels{"GAME", "OPTIONS", "LEVEL", "HELP"};
    for (std::size_t i = 0; i < menuLabels.size(); ++i) {
        if (app.topMenuOpen == static_cast<int>(i)) {
            SDL_FRect activeMenu{menuLeft[i], 0.0F, menuWidth[i], 24.0F};
            setColor(app.renderer, kCardSelection);
            SDL_RenderFillRect(app.renderer, &activeMenu);
        }
        setColor(app.renderer, app.topMenuOpen == static_cast<int>(i) ? kCardText : kCardMuted);
        drawUiText(app.renderer, menuLeft[i] + menuTextInset[i], 7.0F, menuLabels[i]);
    }

    const float panelScale = static_cast<float>(board.height) / 300.0F;
    const float panelWidth = static_cast<float>(board.height) * kPanelAspect;
    const float panelX = static_cast<float>(board.originX + board.width) +
                         std::max(8.0F, static_cast<float>(board.cell) * kBoardPanelGapInCells);
    const float panelY = static_cast<float>(board.originY);
    SDL_FRect panel{panelX, panelY, panelWidth, static_cast<float>(board.height)};
    if (app.scoreboardPanel) {
        SDL_RenderTexture(app.renderer, app.scoreboardPanel, nullptr, &panel);
    } else {
        setColor(app.renderer, kCard);
        SDL_RenderFillRect(app.renderer, &panel);
        setColor(app.renderer, kCardEdge);
        SDL_RenderRect(app.renderer, &panel);
    }
    setColor(app.renderer, SDL_Color{0, 255, 32, 255});
    constexpr float digitHeight = 23.0F;
    drawDigitalNumber(app.renderer, app.digitalDigits, panel.x + 47.0F * panelScale,
                      panel.y + 37.0F * panelScale,
                      51.0F * panelScale, digitHeight * panelScale,
                      level.number, chips::scoreboardDigitBank(true),
                      level.number != 0);
    const bool timedLevel = level.timeLimit != 0;
    setColor(app.renderer, timedLevel ? SDL_Color{0, 255, 32, 255}
                                      : SDL_Color{255, 255, 0, 255});
    drawDigitalNumber(app.renderer, app.digitalDigits, panel.x + 47.0F * panelScale,
                      panel.y + 99.0F * panelScale,
                      51.0F * panelScale, digitHeight * panelScale,
                      app.game.secondsLeft, chips::scoreboardDigitBank(timedLevel),
                      timedLevel);
    setColor(app.renderer, SDL_Color{0, 255, 32, 255});
    drawDigitalNumber(app.renderer, app.digitalDigits, panel.x + 47.0F * panelScale,
                      panel.y + 189.0F * panelScale,
                      51.0F * panelScale, digitHeight * panelScale,
                      app.game.chipsLeft, chips::scoreboardDigitBank(true),
                      level.chipsRequired != 0);
    setColor(app.renderer, kCardText);
    constexpr std::array<std::uint8_t, 4> keyTiles{0x64, 0x65, 0x66, 0x67};
    constexpr std::array<std::uint8_t, 4> bootTiles{0x68, 0x69, 0x6A, 0x6B};
    const float slotSize = 32.0F * panelScale;
    const float slotGap = 0.0F;
    const float inventoryX = panel.x + (154.0F - 4.0F * 32.0F) * panelScale * 0.5F;
    const float inventoryTop = panel.y + 220.0F * panelScale;
    const float inventoryRowStep = 32.0F * panelScale;
    for (std::size_t i = 0; i < 4; ++i) {
        const float x = inventoryX + static_cast<float>(i) * (slotSize + slotGap);
        drawInventorySlot(app.renderer, app.atlas, app.originalTransparentSprites,
                          app.polishedPlayerLandPoses, app.polishedPlayerSwimPoses,
                          app.cleanedKeyBootIcons, x, inventoryTop, slotSize,
                          keyTiles[i], app.game.keys[i] > 0);
        drawInventorySlot(app.renderer, app.atlas, app.originalTransparentSprites,
                          app.polishedPlayerLandPoses, app.polishedPlayerSwimPoses,
                          app.cleanedKeyBootIcons, x, inventoryTop + inventoryRowStep, slotSize,
                          bootTiles[i], app.game.boots[i]);
    }

    const bool standingOnHint = app.game.level && app.game.standingOnHint();
    if (standingOnHint) {
        // The original scoreboard replaces the chip counter and inventory with
        // a black hint window while Chip stands over a Hint tile.
        const SDL_FRect hintPanel{
            panel.x + 12.0F * panelScale,
            panel.y + 143.0F * panelScale,
            panel.w - 24.0F * panelScale,
            panel.h - 150.0F * panelScale};
        setColor(app.renderer, SDL_Color{0, 0, 0, 255});
        SDL_RenderFillRect(app.renderer, &hintPanel);

        const std::string hintText = "Hint: " + level.hint;
        const float horizontalInset = 8.0F * panelScale;
        const float verticalInset = 8.0F * panelScale;
        const float maxTextWidth = std::max(1.0F, hintPanel.w - horizontalInset * 2.0F);
        const float maxTextHeight = std::max(1.0F, hintPanel.h - verticalInset * 2.0F);
        float hintPixelSize = 1.4F * panelScale;
        std::vector<std::string> hintLines;
        float hintLineStep = 0.0F;
        float hintTextHeight = 0.0F;
        while (hintPixelSize >= 0.7F * panelScale) {
            const auto maxChars = static_cast<std::size_t>(std::max(
                1.0F, std::floor(maxTextWidth / (6.0F * hintPixelSize))));
            hintLines = wrapHintText(hintText, maxChars);
            hintLineStep = 19.0F * hintPixelSize / 1.4F;
            hintTextHeight = hintLines.empty() ? 0.0F
                : static_cast<float>(hintLines.size() - 1) * hintLineStep + 7.0F * hintPixelSize;
            if (hintTextHeight <= maxTextHeight) break;
            hintPixelSize -= 0.05F * panelScale;
        }

        setColor(app.renderer, SDL_Color{0, 242, 255, 255});
        for (std::size_t line = 0; line < hintLines.size(); ++line) {
            const auto& text = hintLines[line];
            const float lineWidth = static_cast<float>(text.size()) * 6.0F * hintPixelSize;
            drawItalicBitmapText(app.renderer,
                hintPanel.x + (hintPanel.w - lineWidth) * 0.5F,
                hintPanel.y + verticalInset + static_cast<float>(line) * hintLineStep,
                text, hintPixelSize);
        }
    }

    if (app.paused || app.helpVisible || app.debugVisible || app.bestTimesVisible || app.settingsVisible || app.levelSelect ||
        app.passwordVisible || app.mainMenuVisible) {
        SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
        setColor(app.renderer, SDL_Color{0, 0, 0, 48});
        SDL_FRect shade{0.0F, 0.0F, static_cast<float>(outputWidth), static_cast<float>(outputHeight)};
        SDL_RenderFillRect(app.renderer, &shade);
        if (app.mainMenuVisible && !app.levelSelect && !app.passwordVisible &&
            !app.settingsVisible && !app.helpVisible && !app.bestTimesVisible && app.menuBackground) {
            SDL_SetTextureColorMod(app.menuBackground, 64, 84, 64);
            SDL_SetTextureAlphaMod(app.menuBackground, 255);
            SDL_FRect backdrop{0.0F, 0.0F, static_cast<float>(outputWidth), static_cast<float>(outputHeight)};
            constexpr float sourceWidth = 474.0F;
            constexpr float sourceHeight = 392.0F;
            SDL_FRect source{0.0F, 0.0F, sourceWidth, sourceHeight};
            const float targetAspect = backdrop.w / backdrop.h;
            const float sourceAspect = sourceWidth / sourceHeight;
            if (targetAspect > sourceAspect) {
                source.h = sourceWidth / targetAspect;
                source.y = (sourceHeight - source.h) * 0.5F;
            } else {
                source.w = sourceHeight * targetAspect;
                source.x = (sourceWidth - source.w) * 0.5F;
            }
            SDL_RenderTexture(app.renderer, app.menuBackground, &source, &backdrop);
            SDL_SetTextureAlphaMod(app.menuBackground, 255);
            SDL_SetTextureColorMod(app.menuBackground, 255, 255, 255);
        }
        const auto grid = levelGridLayout(outputWidth, outputHeight);
        const auto settingsLayout = settingsOverlayLayout(outputWidth, outputHeight);
        const auto bestTimesLayout = bestTimesOverlayLayout(outputWidth, outputHeight);
        float debugPanelHeight = 190.0F;
        if (app.debugVisible) {
            const auto creatureRows = std::min<std::size_t>(4, app.game.creatures.size());
            const auto slipRows = std::min<std::size_t>(4, app.game.slipOrder.size());
            if (creatureRows != 0)
                debugPanelHeight = std::max(debugPanelHeight,
                    180.0F + static_cast<float>(creatureRows - 1) * 18.0F + 30.0F);
            if (slipRows != 0)
                debugPanelHeight = std::max(debugPanelHeight,
                    258.0F + static_cast<float>(slipRows - 1) * 18.0F + 30.0F);
            if (app.game.creatures.size() > creatureRows || app.game.slipOrder.size() > slipRows)
                debugPanelHeight = std::max(debugPanelHeight, 370.0F);
        }
        const float panelW = app.levelSelect ? grid.width
            : std::min(app.debugVisible ? 640.0F : 520.0F, static_cast<float>(outputWidth) - 40.0F);
        const float panelH = app.levelSelect ? grid.height
            : app.helpVisible ? 440.0F : app.debugVisible ? debugPanelHeight : app.bestTimesVisible ? 360.0F : app.settingsVisible ? settingsLayout.height
            : app.passwordVisible ? 220.0F : app.mainMenuVisible ? 356.0F : 190.0F;
        const float panelLeft = app.levelSelect ? grid.left
            : app.settingsVisible ? settingsLayout.left : app.bestTimesVisible ? bestTimesLayout.left
            : (static_cast<float>(outputWidth) - panelW) * 0.5F;
        const float panelTop = app.levelSelect ? grid.top
            : app.settingsVisible ? settingsLayout.top : app.bestTimesVisible ? bestTimesLayout.top
            : (static_cast<float>(outputHeight) - panelH) * 0.5F;
        SDL_FRect card{panelLeft, panelTop, panelW, panelH};
        setColor(app.renderer, kCard);
        SDL_RenderFillRect(app.renderer, &card);
        setColor(app.renderer, kCardEdge);
        SDL_FRect cardTop{card.x, card.y, card.w, 2.0F};
        SDL_FRect cardLeft{card.x, card.y, 2.0F, card.h};
        SDL_FRect cardBottom{card.x, card.y + card.h - 2.0F, card.w, 2.0F};
        SDL_FRect cardRight{card.x + card.w - 2.0F, card.y, 2.0F, card.h};
        SDL_RenderFillRect(app.renderer, &cardTop);
        SDL_RenderFillRect(app.renderer, &cardLeft);
        SDL_RenderFillRect(app.renderer, &cardBottom);
        SDL_RenderFillRect(app.renderer, &cardRight);
        if (app.mainMenuVisible && !app.levelSelect && !app.passwordVisible &&
            !app.settingsVisible && !app.helpVisible && !app.bestTimesVisible) {
            setColor(app.renderer, kCardCaption);
            SDL_FRect caption{panelLeft + 4.0F, panelTop + 4.0F, panelW - 8.0F, 28.0F};
            SDL_RenderFillRect(app.renderer, &caption);
            setColor(app.renderer, kCardText);
            drawUiText(app.renderer, panelLeft + 14.0F, panelTop + 12.0F, "NATIVE REMAKE");
        }
        setColor(app.renderer, kCardText);
        if (app.debugVisible) {
            constexpr std::array<const char*, 4> playerDirections{"NORTH", "EAST", "SOUTH", "WEST"};
            constexpr std::array<const char*, 4> creatureDirections{"N", "W", "S", "E"};
            const auto& game = app.game;
            const int player = game.playerPosition;
            const auto outcomeName = [&]() -> const char* {
                switch (game.outcome) {
                    case chips::Outcome::playing: return "PLAYING";
                    case chips::Outcome::won: return "WON";
                    case chips::Outcome::dead: return "DEAD";
                    case chips::Outcome::timedOut: return "TIMED OUT";
                }
                return "UNKNOWN";
            };
            const auto deathCauseName = [&]() -> const char* {
                switch (game.deathCause) {
                    case chips::DeathCause::none: return "NONE";
                    case chips::DeathCause::water: return "WATER";
                    case chips::DeathCause::fire: return "FIRE";
                    case chips::DeathCause::other: return "OTHER";
                    case chips::DeathCause::timeout: return "TIMEOUT";
                }
                return "UNKNOWN";
            };
            drawMediumUiText(app.renderer, panelLeft + 24.0F, panelTop + 18.0F,
                             "SIMULATION STATE  /  F8 / ESC / B TO CLOSE");
            drawMediumUiTextFormat(app.renderer, panelLeft + 24.0F, panelTop + 52.0F,
                "TICK %u   RNG %08X   OUTCOME %s   CAUSE %s", game.gameTicks,
                static_cast<unsigned>(game.randomState), outcomeName(), deathCauseName());
            drawMediumUiTextFormat(app.renderer, panelLeft + 24.0F, panelTop + 72.0F,
                "CHIP %02d,%02d CELL %04d FACE %s", player % chips::kBoardWidth,
                player / chips::kBoardWidth, player, playerDirections[static_cast<std::size_t>(game.facing)]);
            drawMediumUiTextFormat(app.renderer, panelLeft + 24.0F, panelTop + 92.0F,
                "LAYERS TOP %02X BOTTOM %02X FLOOR %02X SURFACE %02X",
                static_cast<unsigned>(game.top[static_cast<std::size_t>(player)]),
                static_cast<unsigned>(game.bottom[static_cast<std::size_t>(player)]),
                static_cast<unsigned>(game.floor(player)), static_cast<unsigned>(game.surface(player)));
            drawMediumUiTextFormat(app.renderer, panelLeft + 24.0F, panelTop + 112.0F,
                "CHIPS %u TIME %u KEYS %u,%u,%u,%u BOOTS %u%u%u%u",
                game.chipsLeft, game.secondsLeft, game.keys[0], game.keys[1], game.keys[2], game.keys[3],
                game.boots[0] ? 1U : 0U, game.boots[1] ? 1U : 0U,
                game.boots[2] ? 1U : 0U, game.boots[3] ? 1U : 0U);
            const auto slideDirection = static_cast<unsigned>(game.slideDirection);
            drawMediumUiTextFormat(app.renderer, panelLeft + 24.0F, panelTop + 132.0F,
                "SLIDING %s DIR %s   MOVED %u   IDLE TICKS %u",
                game.sliding ? "YES" : "NO", playerDirections[slideDirection],
                game.chipHasMoved ? 1U : 0U, static_cast<unsigned>(game.idleTickCount));
            const auto liveCreatures = static_cast<unsigned>(std::count_if(game.creatures.begin(), game.creatures.end(),
                [](const chips::Creature& creature) { return creature.alive; }));
            drawMediumUiTextFormat(app.renderer, panelLeft + 24.0F, panelTop + 158.0F,
                "CREATURES %u (%u LIVE)   SLIP QUEUE %u",
                static_cast<unsigned>(game.creatures.size()), liveCreatures,
                static_cast<unsigned>(game.slipOrder.size()));
            const std::size_t creatureCount = std::min<std::size_t>(4, game.creatures.size());
            for (std::size_t i = 0; i < creatureCount; ++i) {
                const auto& creature = game.creatures[i];
                const int creaturePosition = creature.position;
                const int direction = creature.movementDirection >= 0
                    ? creature.movementDirection : creature.tile & 0x03;
                drawMediumUiTextFormat(app.renderer, panelLeft + 24.0F,
                    panelTop + 180.0F + static_cast<float>(i) * 18.0F,
                    "CR %02u TILE %02X POS %02d,%02d DIR %s %s",
                    static_cast<unsigned>(i), static_cast<unsigned>(creature.tile),
                    creaturePosition % chips::kBoardWidth, creaturePosition / chips::kBoardWidth,
                    creatureDirections[static_cast<std::size_t>(direction & 3)],
                    creature.alive ? "LIVE" : "DEAD");
            }
            const std::size_t slipCount = std::min<std::size_t>(4, game.slipOrder.size());
            for (std::size_t i = 0; i < slipCount; ++i) {
                const auto& slip = game.slipOrder[i];
                const bool block = slip.kind == chips::SlipperKind::block;
                const int position = block ? slip.entity
                    : slip.entity >= 0 && static_cast<std::size_t>(slip.entity) < game.creatures.size()
                        ? game.creatures[static_cast<std::size_t>(slip.entity)].position : -1;
                const int direction = block && position >= 0
                    ? game.blockDirections[static_cast<std::size_t>(position)] : slip.direction;
                drawMediumUiTextFormat(app.renderer, panelLeft + 24.0F,
                    panelTop + 258.0F + static_cast<float>(i) * 18.0F,
                    "SLIP %02u %s POS %02d,%02d DIR %s",
                    static_cast<unsigned>(i), block ? "BLOCK" : "CREATURE",
                    position >= 0 ? position % chips::kBoardWidth : -1,
                    position >= 0 ? position / chips::kBoardWidth : -1,
                    direction >= 0 ? creatureDirections[static_cast<std::size_t>(direction & 3)] : "- ");
            }
            if (game.creatures.size() > creatureCount || game.slipOrder.size() > slipCount)
                drawMediumUiTextFormat(app.renderer, panelLeft + 24.0F, panelTop + 338.0F,
                    "MORE: %u CREATURES  %u SLIPS",
                    static_cast<unsigned>(game.creatures.size() - creatureCount),
                    static_cast<unsigned>(game.slipOrder.size() - slipCount));
        } else if (app.levelSelect) {
            const std::size_t pageCount = (app.levels.size() + kLevelSelectorPageSize - 1) /
                                          kLevelSelectorPageSize;
            const std::size_t page = app.selectedLevel / kLevelSelectorPageSize;
            if (pageCount > 1) {
                drawUiTextFormat(app.renderer, panelLeft + 24.0F, panelTop + 24.0F,
                    "PAGE %u / %u", static_cast<unsigned>(page + 1),
                    static_cast<unsigned>(pageCount));
                drawActionButton(app.renderer, grid.previousPageButton, "<");
                drawActionButton(app.renderer, grid.nextPageButton, ">");
            } else {
                drawUiTextFormat(app.renderer, panelLeft + 24.0F, panelTop + 24.0F,
                    "LEVEL PACK  PAGE %u / %u", static_cast<unsigned>(page + 1),
                    static_cast<unsigned>(pageCount));
            }
            const auto selectedTitle = app.selectedLevel < app.levels.size()
                ? app.levels[app.selectedLevel].title : std::string{};
            drawUiTextFormat(app.renderer, panelLeft + 24.0F, panelTop + 44.0F,
                "SELECTED %03u - %s", static_cast<unsigned>(app.selectedLevel + 1),
                selectedTitle.empty() ? "UNTITLED" : selectedTitle.c_str());
            drawUiText(app.renderer, panelLeft + 24.0F, panelTop + 64.0F,
                                "GREEN COMPLETE  GOLD SELECTED  DARK NEEDS PASSWORD");
            const std::size_t pageStart = page * kLevelSelectorPageSize;
            const std::size_t pageEnd = std::min(pageStart + kLevelSelectorPageSize, app.levels.size());
            for (std::size_t i = pageStart; i < pageEnd; ++i) {
                const auto local = i - pageStart;
                const auto x = grid.gridLeft + static_cast<float>(local % kLevelSelectorColumns) * grid.cell;
                const auto y = grid.gridTop + static_cast<float>(local / kLevelSelectorColumns) * grid.cell;
                SDL_FRect button{x + 1.0F, y + 1.0F, grid.cell - 3.0F, grid.cell - 3.0F};
                const bool selected = i == app.selectedLevel;
                const bool completed = app.profile.completed[i];
                const bool passwordRequired = levelRequiresPassword(app, i);
                setColor(app.renderer, passwordRequired ? SDL_Color{32, 41, 48, 255}
                    : selected ? SDL_Color{202, 156, 61, 255}
                    : completed ? SDL_Color{58, 126, 93, 255} : SDL_Color{51, 68, 79, 255});
                SDL_RenderFillRect(app.renderer, &button);
                if (selected && passwordRequired) {
                    setColor(app.renderer, SDL_Color{247, 211, 106, 255});
                    SDL_FRect selectedTop{button.x, button.y, button.w, 2.0F};
                    SDL_FRect selectedLeft{button.x, button.y, 2.0F, button.h};
                    SDL_FRect selectedBottom{button.x, button.y + button.h - 2.0F, button.w, 2.0F};
                    SDL_FRect selectedRight{button.x + button.w - 2.0F, button.y, 2.0F, button.h};
                    SDL_RenderFillRect(app.renderer, &selectedTop);
                    SDL_RenderFillRect(app.renderer, &selectedLeft);
                    SDL_RenderFillRect(app.renderer, &selectedBottom);
                    SDL_RenderFillRect(app.renderer, &selectedRight);
                }
                setColor(app.renderer, passwordRequired && !selected
                    ? SDL_Color{132, 145, 151, 255} : SDL_Color{247, 249, 250, 255});
                drawUiTextFormat(app.renderer, x + 5.0F, y + grid.cell * 0.5F - 4.0F,
                                          "%03u", static_cast<unsigned>(i + 1));
            }
            if (app.selectedLevel < app.levels.size()) {
                const auto& selectedLevel = app.levels[app.selectedLevel];
                const auto& bounds = grid.previewBounds;
                SDL_FRect previewFrame{bounds.x - 4.0F, bounds.y - 4.0F,
                                       bounds.w + 8.0F, bounds.h + 8.0F};
                setColor(app.renderer, SDL_Color{10, 17, 23, 255});
                SDL_RenderFillRect(app.renderer, &previewFrame);
                setColor(app.renderer, kCardEdge);
                SDL_FRect previewTop{previewFrame.x, previewFrame.y, previewFrame.w, 2.0F};
                SDL_FRect previewLeft{previewFrame.x, previewFrame.y, 2.0F, previewFrame.h};
                SDL_FRect previewBottom{previewFrame.x, previewFrame.y + previewFrame.h - 2.0F,
                                        previewFrame.w, 2.0F};
                SDL_FRect previewRight{previewFrame.x + previewFrame.w - 2.0F, previewFrame.y,
                                       2.0F, previewFrame.h};
                SDL_RenderFillRect(app.renderer, &previewTop);
                SDL_RenderFillRect(app.renderer, &previewLeft);
                SDL_RenderFillRect(app.renderer, &previewBottom);
                SDL_RenderFillRect(app.renderer, &previewRight);
                setColor(app.renderer, kCardText);
                drawUiText(app.renderer, bounds.x, bounds.y - 26.0F, "MAP PREVIEW");
                constexpr int previewCells = chips::kBoardWidth * chips::kBoardHeight;
                const float previewCell = bounds.w / static_cast<float>(chips::kBoardWidth);
                for (int position = 0; position < previewCells; ++position) {
                    const auto index = static_cast<std::size_t>(position);
                    const auto lower = selectedLevel.bottom[index];
                    const auto upper = selectedLevel.top[index];
                    const auto layers = chips::resolveTileDrawLayers(upper, lower);
                    SDL_FRect cell{
                        bounds.x + static_cast<float>(position % chips::kBoardWidth) * previewCell,
                        bounds.y + static_cast<float>(position / chips::kBoardWidth) * previewCell,
                        previewCell, previewCell};
                    if (!drawRuntimeTile(app.renderer, app.atlas,
                                         app.originalTransparentSprites,
                                         app.polishedPlayerLandPoses, app.polishedPlayerSwimPoses,
                                         app.cleanedCreatureSprites, app.cleanedKeyBootIcons,
                                         layers.lower, cell)) {
                        setColor(app.renderer, tileColor(layers.lower));
                        SDL_RenderFillRect(app.renderer, &cell);
                    }
                    if (layers.drawUpper &&
                        !drawRuntimeTile(app.renderer, app.atlas,
                                         app.originalTransparentSprites,
                                         app.polishedPlayerLandPoses, app.polishedPlayerSwimPoses,
                                         app.cleanedCreatureSprites, app.cleanedKeyBootIcons,
                                         upper, cell)) {
                        SDL_FRect mark{cell.x + previewCell * 0.2F, cell.y + previewCell * 0.2F,
                                       previewCell * 0.6F, previewCell * 0.6F};
                        setColor(app.renderer, tileColor(upper));
                        SDL_RenderFillRect(app.renderer, &mark);
                    }
                }
                setColor(app.renderer, kCardMuted);
                drawSmallUiTextFormat(app.renderer, bounds.x, bounds.y + bounds.h + 12.0F,
                    "LEVEL %03u", static_cast<unsigned>(app.selectedLevel + 1));
                drawSmallUiTextFormat(app.renderer, bounds.x, bounds.y + bounds.h + 28.0F,
                    "CHIPS %03u", static_cast<unsigned>(selectedLevel.chipsRequired));
                drawSmallUiTextFormat(app.renderer, bounds.x, bounds.y + bounds.h + 44.0F,
                    "LIMIT %03u", static_cast<unsigned>(selectedLevel.timeLimit));
                const auto bestTime = app.selectedLevel < app.profile.bestTimes.size()
                    ? app.profile.bestTimes[app.selectedLevel] : 0xFFFFFFFFU;
                if (bestTime == 0xFFFFFFFFU)
                    drawSmallUiText(app.renderer, bounds.x, bounds.y + bounds.h + 60.0F,
                                    "BEST --:--");
                else
                    drawSmallUiTextFormat(app.renderer, bounds.x, bounds.y + bounds.h + 60.0F,
                        "BEST %02u:%02u", bestTime / 60U, bestTime % 60U);
            }
            setColor(app.renderer, kCardMuted);
            drawSmallUiText(app.renderer, panelLeft + 24.0F, panelTop + panelH - 24.0F,
                            pageCount > 1
                                ? "CLICK A LEVEL OR PAGE ARROWS; ARROWS / PGUP / PGDN ALSO WORK"
                                : "CLICK A LEVEL TO SELECT; ARROWS MOVE; ESC CLOSES");
        } else if (app.passwordVisible) {
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 30.0F, "ENTER LEVEL PASSWORD");
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 78.0F, "PASSWORD");
            SDL_FRect inputBox{panelLeft + 28.0F, panelTop + 102.0F, panelW - 56.0F, 38.0F};
            setColor(app.renderer, SDL_Color{15, 25, 31, 255});
            SDL_RenderFillRect(app.renderer, &inputBox);
            setColor(app.renderer, SDL_Color{247, 211, 106, 255});
            drawUiText(app.renderer, panelLeft + 40.0F, panelTop + 114.0F,
                       app.passwordInput.empty() ? "TYPE PASSWORD..." : app.passwordInput.c_str());
            setColor(app.renderer, SDL_Color{240, 245, 247, 255});
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 164.0F,
                       app.passwordMessage.empty() ? "ENTER SUBMITS   ESC CANCELS" : app.passwordMessage.c_str());
        } else if (app.bestTimesVisible) {
            std::size_t totalEntries = 0;
            const auto entryLimit = std::min(app.profile.bestTimes.size(), app.levels.size());
            for (std::size_t i = 0; i < entryLimit; ++i)
                if (app.profile.bestTimes[i] != 0xFFFFFFFFU) ++totalEntries;
            const auto pageCount = std::max<std::size_t>(1,
                (totalEntries + kBestTimesPerPage - 1) / kBestTimesPerPage);
            const auto page = std::min(app.bestTimesPage, pageCount - 1);
            const auto firstEntry = page * kBestTimesPerPage;
            drawUiTextFormat(app.renderer, panelLeft + 28.0F, panelTop + 28.0F,
                "BEST TIMES   PAGE %u / %u", static_cast<unsigned>(page + 1),
                static_cast<unsigned>(pageCount));
            if (pageCount > 1) {
                drawActionButton(app.renderer, bestTimesLayout.previousPageButton, "<");
                drawActionButton(app.renderer, bestTimesLayout.nextPageButton, ">");
            }
            std::size_t entry = 0;
            std::size_t line = 0;
            for (std::size_t i = 0; i < entryLimit && line < kBestTimesPerPage; ++i) {
                if (i >= app.levels.size()) break;
                const auto seconds = app.profile.bestTimes[i];
                if (seconds == 0xFFFFFFFFU) continue;
                if (entry++ < firstEntry) continue;
                drawUiTextFormat(app.renderer, panelLeft + 28.0F,
                    panelTop + 64.0F + static_cast<float>(line) * 24.0F,
                    "%03u  %s  %02u:%02u", static_cast<unsigned>(i + 1),
                    app.levels[i].title.c_str(), seconds / 60, seconds % 60);
                ++line;
            }
            if (line == 0) drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 72.0F, "NO COMPLETED TIMES YET");
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + panelH - 30.0F,
                       pageCount > 1 ? "PGUP / PGDN OR L / R   CLICK ARROWS   ESC CLOSES"
                                     : "PGUP / PGDN OR L / R   ESC CLOSES");
        } else if (app.helpVisible) {
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 28.0F, "HOW TO PLAY  /  CONTROLS");
            constexpr std::array<const char*, 17> helpLines{
                "ARROWS / WASD / D-PAD / STICK: MOVE",
                "CLICK A TILE TO WALK TO IT",
                "[ / ]: PREVIOUS / NEXT LEVEL; L: LEVELS",
                "R: RESTART  P / START: PAUSE",
                "ENTER / A: NEXT OR RETRY LEVEL",
                "F11: FULLSCREEN  O: SETTINGS  F8: DEBUG",
                "GOAL: COLLECT THE REQUIRED CHIPS",
                "PASS THE SOCKET, THEN REACH THE EXIT",
                "MATCH EACH KEY COLOR TO ITS DOOR",
                "FLIPPERS: WATER  FIRE BOOTS: FIRE",
                "SKATES: ICE  SUCTION BOOTS: FORCE FLOORS",
                "PUSH BLOCKS; YOU CANNOT PULL THEM",
                "CREATURES ARE DANGEROUS; USE TERRAIN WISELY",
                "HINTS AND PASSWORDS CAN HELP",
                "OVERLAYS: ARROWS/WASD; ENTER/SPACE",
                "MOUSE HOVER/CLICK; D-PAD/STICK + A",
                "F1 / ESC / B: BACK  FOCUS LOSS PAUSES PLAY"};
            for (std::size_t i = 0; i < helpLines.size(); ++i)
                drawUiText(app.renderer, panelLeft + 28.0F,
                           panelTop + 64.0F + static_cast<float>(i) * 21.0F, helpLines[i]);
        } else if (app.settingsVisible) {
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 28.0F, "SETTINGS");
            drawUiTextFormat(app.renderer, panelLeft + 28.0F, panelTop + 76.0F,
                "SOUND EFFECTS   %s", app.profile.effectsMuted ? "MUTED" : "ON");
            drawUiTextFormat(app.renderer, panelLeft + 28.0F, panelTop + 108.0F,
                "EFFECT VOLUME   %u%%", static_cast<unsigned>(app.profile.effectsVolumePercent));
            drawSettingsSlider(app.renderer, settingsLayout.effectsSlider,
                               static_cast<float>(app.profile.effectsVolumePercent) / 100.0F);
            drawUiTextFormat(app.renderer, panelLeft + 28.0F, panelTop + 140.0F,
                "BACKGROUND MUSIC   %s", app.profile.musicMuted ? "MUTED" : "ON");
            drawUiTextFormat(app.renderer, panelLeft + 28.0F, panelTop + 172.0F,
                "MUSIC VOLUME    %u%%", static_cast<unsigned>(app.profile.musicVolumePercent));
            drawSettingsSlider(app.renderer, settingsLayout.musicSlider,
                               static_cast<float>(app.profile.musicVolumePercent) / 100.0F);
            drawUiTextFormat(app.renderer, panelLeft + 28.0F, panelTop + 204.0F,
                "DISPLAY SCALE   %u%%", static_cast<unsigned>(app.profile.uiScalePercent));
            drawSettingsSlider(app.renderer, settingsLayout.scaleSlider,
                               static_cast<float>(app.profile.uiScalePercent - 100U) / 100.0F);
            drawUiTextFormat(app.renderer, panelLeft + 28.0F, panelTop + 228.0F,
                "REDUCED MOTION  %s   (F6)", app.profile.reducedMotion ? "ON" : "OFF");
            drawUiTextFormat(app.renderer, panelLeft + 28.0F, panelTop + 252.0F,
                "DISPLAY MODE   %s   (F11)", app.fullscreen ? "FULLSCREEN" : "WINDOWED");
            constexpr std::array<const char*, 4> directions{"UP", "RIGHT", "DOWN", "LEFT"};
            constexpr std::array<SDL_Scancode, 4> arrows{
                SDL_SCANCODE_UP, SDL_SCANCODE_RIGHT, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT};
            for (std::size_t i = 0; i < directions.size(); ++i) {
                const auto keyName = app.waitingForBinding == static_cast<int>(i)
                    ? "PRESS A KEY" : SDL_GetScancodeName(static_cast<SDL_Scancode>(app.profile.movementBindings[i]));
                drawUiTextFormat(app.renderer, panelLeft + 28.0F, panelTop + 280.0F + static_cast<float>(i) * 24.0F,
                    "F%u  MOVE %-5s  %s   (ARROW: %s)", static_cast<unsigned>(i + 2), directions[i],
                    keyName, SDL_GetScancodeName(arrows[i]));
            }
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 400.0F, "CLICK SLIDERS OR USE PAGE UP/DOWN");
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 424.0F, "F2-F5         Rebind movement; ESC cancels");
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 448.0F, "F6            Toggle reduced motion");
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 472.0F, "F11           Toggle fullscreen");
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 496.0F, "CLICK AUDIO ROWS TO MUTE");
        } else if (app.mainMenuVisible) {
            if (app.appIcon) {
                SDL_FRect logo{panelLeft + 28.0F, panelTop + 42.0F, 48.0F, 48.0F};
                SDL_RenderTexture(app.renderer, app.appIcon, nullptr, &logo);
            }
            drawUiText(app.renderer, panelLeft + 92.0F, panelTop + 58.0F, "CHIP'S CHALLENGE");
            constexpr std::array<const char*, 7> menuItems{
                "CONTINUE LEVEL", "NEW GAME (LEVEL 1)", "CAMPAIGN LEVELS", "ENTER PASSWORD",
                "SETTINGS", "CONTROLS", "QUIT"};
            for (std::size_t i = 0; i < menuItems.size(); ++i) {
                const float rowY = panelTop + 100.0F + static_cast<float>(i) * 32.0F;
                if (i == app.menuSelection) {
                    SDL_FRect selectedRow{panelLeft + 20.0F, rowY - 5.0F, panelW - 40.0F, 28.0F};
                    setColor(app.renderer, kCardSelection);
                    SDL_RenderFillRect(app.renderer, &selectedRow);
                    setColor(app.renderer, kCardText);
                } else {
                    setColor(app.renderer, kCardText);
                }
                drawUiText(app.renderer, panelLeft + 36.0F, rowY, menuItems[i]);
            }
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 324.0F,
                       "ARROWS / D-PAD OR MOUSE; ENTER / A");
        } else {
            drawUiText(app.renderer, panelLeft + 28.0F, panelTop + 28.0F, "PAUSED");
            const auto pauseLayout = pauseOverlayLayout(outputWidth, outputHeight);
            constexpr std::array<const char*, 3> labels{"RESUME", "RESTART LEVEL", "SHOW CONTROLS"};
            for (std::size_t i = 0; i < pauseLayout.buttonCount; ++i)
                drawActionButton(app.renderer, pauseLayout.buttons[i], labels[i],
                                 static_cast<int>(i) == app.actionSelection);
        }
        SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
    }
    if (app.levelPlacardVisible && app.game.level &&
        app.game.outcome == chips::Outcome::playing && !app.mainMenuVisible &&
        !app.paused && !app.helpVisible && !app.settingsVisible && !app.levelSelect &&
        !app.passwordVisible && !app.bestTimesVisible) {
        const auto& level = *app.game.level;
        const std::string title = level.title.empty()
            ? "LEVEL " + std::to_string(level.number) : level.title;
        const std::string password = "Password: " +
            (level.password.empty() ? std::string("----") : level.password);
        const float textPixelSize = std::max(1.0F, static_cast<float>(board.cell) * 0.05F);
        const float textWidth = static_cast<float>(std::max(title.size(), password.size())) *
                                6.0F * textPixelSize;
        const float placardWidth = std::min(static_cast<float>(board.width) -
                                                static_cast<float>(board.cell) * 0.4F,
                                            std::max(static_cast<float>(board.cell) * 5.75F,
                                                     textWidth + static_cast<float>(board.cell) * 0.9F));
        const float placardHeight = static_cast<float>(board.cell) * 1.65F;
        // The original placard sits low in the visible board (around row 6.7),
        // rather than in the board's vertical center.
        constexpr float placardTopRow = 6.67F;
        const float edge = std::max(2.0F, static_cast<float>(board.cell) * 0.075F);
        const SDL_FRect placard{
            static_cast<float>(board.originX) + (static_cast<float>(board.width) - placardWidth) * 0.5F,
            static_cast<float>(board.originY) + static_cast<float>(board.cell) * placardTopRow,
            placardWidth, placardHeight};
        SDL_FRect frame{placard.x - edge, placard.y - edge,
                        placard.w + edge * 2.0F, placard.h + edge * 2.0F};
        setColor(app.renderer, SDL_Color{224, 224, 216, 255});
        SDL_RenderFillRect(app.renderer, &frame);
        setColor(app.renderer, SDL_Color{16, 16, 16, 255});
        SDL_RenderFillRect(app.renderer, &placard);
        setColor(app.renderer, SDL_Color{250, 250, 250, 255});
        SDL_FRect top{frame.x, frame.y, frame.w, edge};
        SDL_FRect left{frame.x, frame.y, edge, frame.h};
        SDL_RenderFillRect(app.renderer, &top);
        SDL_RenderFillRect(app.renderer, &left);
        setColor(app.renderer, SDL_Color{96, 96, 96, 255});
        SDL_FRect bottom{frame.x, frame.y + frame.h - edge, frame.w, edge};
        SDL_FRect right{frame.x + frame.w - edge, frame.y, edge, frame.h};
        SDL_RenderFillRect(app.renderer, &bottom);
        SDL_RenderFillRect(app.renderer, &right);
        SDL_FRect innerTop{placard.x, placard.y, placard.w,
                           std::max(1.0F, static_cast<float>(board.cell) * 0.05F)};
        setColor(app.renderer, SDL_Color{80, 80, 80, 255});
        SDL_RenderFillRect(app.renderer, &innerTop);
        setColor(app.renderer, SDL_Color{255, 245, 20, 255});
        drawBitmapText(app.renderer,
                       placard.x + (placard.w - static_cast<float>(title.size()) * 6.0F * textPixelSize) * 0.5F,
                       placard.y + placardHeight * 0.16F, title.c_str(), textPixelSize);
        drawBitmapText(app.renderer,
                       placard.x + (placard.w - static_cast<float>(password.size()) * 6.0F * textPixelSize) * 0.5F,
                       placard.y + placardHeight * 0.56F, password.c_str(), textPixelSize);
    }
    if (app.game.outcome != chips::Outcome::playing && !app.paused && !app.helpVisible && !app.bestTimesVisible &&
        !app.settingsVisible && !app.levelSelect && !app.mainMenuVisible) {
        const bool won = app.game.outcome == chips::Outcome::won;
        if (!won) {
            SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
            setColor(app.renderer, SDL_Color{0, 0, 0, 150});
            SDL_FRect shade{0.0F, 0.0F, static_cast<float>(outputWidth), static_cast<float>(outputHeight)};
            SDL_RenderFillRect(app.renderer, &shade);
        }
        const auto resultLayout = resultOverlayLayout(outputWidth, outputHeight, won);
        const auto card = resultLayout.card;
        const float cardX = card.x;
        const float cardY = card.y;
        if (won) {
            // Match the original Windows completion dialog: a compact gray
            // modal with score breakdown, time-record feedback, and Onward.
            setColor(app.renderer, SDL_Color{64, 67, 66, 255});
            SDL_FRect shadow{cardX + 4.0F, cardY + 4.0F, card.w, card.h};
            SDL_RenderFillRect(app.renderer, &shadow);
            setColor(app.renderer, SDL_Color{192, 192, 192, 255});
            SDL_RenderFillRect(app.renderer, &card);
            setColor(app.renderer, SDL_Color{255, 255, 255, 255});
            SDL_FRect topEdge{cardX, cardY, card.w, 2.0F};
            SDL_FRect leftEdge{cardX, cardY, 2.0F, card.h};
            SDL_RenderFillRect(app.renderer, &topEdge);
            SDL_RenderFillRect(app.renderer, &leftEdge);
            setColor(app.renderer, SDL_Color{78, 78, 78, 255});
            SDL_FRect bottomEdge{cardX, cardY + card.h - 2.0F, card.w, 2.0F};
            SDL_FRect rightEdge{cardX + card.w - 2.0F, cardY, 2.0F, card.h};
            SDL_RenderFillRect(app.renderer, &bottomEdge);
            SDL_RenderFillRect(app.renderer, &rightEdge);

            SDL_FRect caption{cardX + 3.0F, cardY + 3.0F, card.w - 6.0F, 25.0F};
            setColor(app.renderer, SDL_Color{174, 174, 174, 255});
            SDL_RenderFillRect(app.renderer, &caption);
            setColor(app.renderer, SDL_Color{16, 16, 16, 255});
            drawUiText(app.renderer, cardX + 11.0F, cardY + 8.0F,
                       "LEVEL COMPLETE!");
            SDL_FRect closeButton{cardX + card.w - 25.0F, cardY + 5.0F, 17.0F, 17.0F};
            setColor(app.renderer, SDL_Color{212, 212, 212, 255});
            SDL_RenderFillRect(app.renderer, &closeButton);
            setColor(app.renderer, SDL_Color{48, 48, 48, 255});
            drawUiText(app.renderer, closeButton.x + 3.0F, closeButton.y + 2.0F, "X");

            const char* completionLine = app.levelRestartCount == 0 ? "YOWSER! FIRST TRY!"
                : app.levelRestartCount < 3 ? "GO BIT BUSTER!"
                : app.levelRestartCount < 5 ? "FINISHED! GOOD WORK!"
                : "AT LAST! YOU DID IT!";
            setColor(app.renderer, SDL_Color{16, 16, 16, 255});
            drawUiText(app.renderer, cardX + 22.0F, cardY + 39.0F, completionLine);
            drawUiTextFormat(app.renderer, cardX + 22.0F, cardY + 66.0F,
                "TIME BONUS: %llu", static_cast<unsigned long long>(app.levelTimeBonus));
            drawUiTextFormat(app.renderer, cardX + 22.0F, cardY + 91.0F,
                "LEVEL BONUS: %llu", static_cast<unsigned long long>(app.levelCompletionBonus));
            drawUiTextFormat(app.renderer, cardX + 22.0F, cardY + 116.0F,
                "LEVEL SCORE: %llu", static_cast<unsigned long long>(app.levelCompletionScore));
            drawUiTextFormat(app.renderer, cardX + 22.0F, cardY + 141.0F,
                "TOTAL SCORE: %llu", static_cast<unsigned long long>(app.totalScore));
            if (app.timeRecordEstablished) {
                drawWrappedUiText(app.renderer, cardX + 22.0F, cardY + 168.0F,
                    "YOU HAVE ESTABLISHED A TIME RECORD FOR THIS LEVEL!", 30, 2);
            } else if (app.timeRecordImproved) {
                auto recordMessage = "YOU BEAT THE PREVIOUS TIME RECORD BY " +
                    std::to_string(app.timeRecordImprovementSeconds) + " SECOND";
                if (app.timeRecordImprovementSeconds != 1) recordMessage += 'S';
                recordMessage += '!';
                drawWrappedUiText(app.renderer, cardX + 22.0F, cardY + 168.0F,
                                  recordMessage, 30, 2);
            }

            const auto& onward = resultLayout.buttons[0];
            setColor(app.renderer, SDL_Color{214, 214, 214, 255});
            SDL_RenderFillRect(app.renderer, &onward);
            setColor(app.renderer, SDL_Color{255, 255, 255, 255});
            SDL_FRect buttonTop{onward.x, onward.y, onward.w, 2.0F};
            SDL_FRect buttonLeft{onward.x, onward.y, 2.0F, onward.h};
            SDL_RenderFillRect(app.renderer, &buttonTop);
            SDL_RenderFillRect(app.renderer, &buttonLeft);
            setColor(app.renderer, SDL_Color{72, 72, 72, 255});
            SDL_FRect buttonBottom{onward.x, onward.y + onward.h - 2.0F, onward.w, 2.0F};
            SDL_FRect buttonRight{onward.x + onward.w - 2.0F, onward.y, 2.0F, onward.h};
            SDL_RenderFillRect(app.renderer, &buttonBottom);
            SDL_RenderFillRect(app.renderer, &buttonRight);
            setColor(app.renderer, SDL_Color{16, 16, 16, 255});
            const char* buttonLabel = "ONWARD!";
            const float labelWidth = static_cast<float>(std::char_traits<char>::length(buttonLabel)) * 12.0F;
            drawUiText(app.renderer, onward.x + (onward.w - labelWidth) * 0.5F,
                       onward.y + 5.0F, buttonLabel);
        } else {
        setColor(app.renderer, SDL_Color{28, 42, 51, 250});
        SDL_RenderFillRect(app.renderer, &card);
        if (app.atlas &&
                   chips::showLossIllustration(app.game.outcome == chips::Outcome::timedOut)) {
            const std::uint8_t deathTile = app.game.deathCause == chips::DeathCause::water ? 0x33
                : app.game.deathCause == chips::DeathCause::fire ? 0x34 : 0x35;
            SDL_FRect illustration{cardX + card.w - 128.0F, cardY + 20.0F, 104.0F, 104.0F};
            (void)drawAtlasTile(app.renderer, app.atlas, deathTile, illustration);
        }
        setColor(app.renderer, SDL_Color{242, 133, 120, 255});
        drawUiText(app.renderer, cardX + 28.0F, cardY + 28.0F,
                   app.game.outcome == chips::Outcome::timedOut
                       ? "TIME EXPIRED" : "LEVEL FAILED");
        setColor(app.renderer, SDL_Color{240, 245, 247, 255});
        drawWrappedUiText(app.renderer, cardX + 28.0F, cardY + 72.0F,
                          app.game.message, 30, 3);
        constexpr std::array<const char*, 2> labels{"RETRY LEVEL", "MAIN MENU"};
        for (std::size_t i = 0; i < resultLayout.buttonCount; ++i)
            drawActionButton(app.renderer, resultLayout.buttons[i], labels[i],
                             static_cast<int>(i) == app.actionSelection);
        }
        SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
    }
    if (app.topMenuOpen >= 0) {
        static constexpr std::array<std::array<const char*, 4>, 4> items{{
            {{"New Game              F2", "Pause                   F3", "Best Times...", "Exit"}},
            {{"Sound and Display...", "Controls...", "", ""}},
            {{"Choose Level...", "Enter Password...", "Open Level Pack...", ""}},
            {{"Controls", "About Chip's Challenge", "", ""}}}};
        static constexpr std::array<float, 4> left{0.0F, 84.0F, 194.0F, 278.0F};
        static constexpr std::array<float, 4> widths{188.0F, 196.0F, 190.0F, 212.0F};
        const int menu = app.topMenuOpen;
        const std::size_t count = static_cast<std::size_t>(topMenuItemCount(menu));
        SDL_FRect drop{left[menu], 26.0F, widths[menu], static_cast<float>(count) * 24.0F + 4.0F};
        setColor(app.renderer, kCard);
        SDL_RenderFillRect(app.renderer, &drop);
        setColor(app.renderer, kCardEdge);
        SDL_FRect topEdge{drop.x, drop.y, drop.w, 2.0F};
        SDL_RenderFillRect(app.renderer, &topEdge);
        setColor(app.renderer, kCardEdge);
        SDL_FRect rightEdge{drop.x + drop.w - 2.0F, drop.y, 2.0F, drop.h};
        SDL_RenderFillRect(app.renderer, &rightEdge);
        for (std::size_t i = 0; i < count; ++i) {
            const float rowY = drop.y + 6.0F + static_cast<float>(i) * 24.0F;
            const bool selected = static_cast<int>(i) == app.topMenuSelection;
            if (selected) {
                SDL_FRect selected{drop.x + 3.0F, rowY - 3.0F, drop.w - 8.0F, 22.0F};
                setColor(app.renderer, kCardSelection);
                SDL_RenderFillRect(app.renderer, &selected);
            }
            if (menu == 0 && i == 3) {
                setColor(app.renderer, SDL_Color{128, 128, 128, 255});
                SDL_FRect separator{drop.x + 8.0F, rowY - 4.0F, drop.w - 18.0F, 1.0F};
                SDL_RenderFillRect(app.renderer, &separator);
            }
            setColor(app.renderer, selected ? kCardText : kCardMuted);
            drawUiText(app.renderer, drop.x + 12.0F, rowY, items[menu][i]);
        }
    }
    SDL_SetRenderScale(app.renderer, 1.0F, 1.0F);
    SDL_RenderPresent(app.renderer);
}


} // namespace chips_native
