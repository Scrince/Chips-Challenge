#pragma once

#include "app_state.hpp"

#include <optional>

namespace chips_native {

struct LevelGridLayout {
    float left{};
    float top{};
    float width{};
    float height{};
    float gridLeft{};
    float gridTop{};
    float cell{};
    SDL_FRect previewBounds{};
    SDL_FRect previousPageButton{};
    SDL_FRect nextPageButton{};
};

struct SettingsOverlayLayout {
    float left{};
    float top{};
    float width{};
    float height{};
    SDL_FRect effectsSlider{};
    SDL_FRect musicSlider{};
    SDL_FRect scaleSlider{};
    SDL_FRect effectsToggle{};
    SDL_FRect musicToggle{};
    SDL_FRect motionToggle{};
    SDL_FRect fullscreenToggle{};
};

struct BestTimesOverlayLayout {
    float left{};
    float top{};
    float width{};
    float height{};
    SDL_FRect previousPageButton{};
    SDL_FRect nextPageButton{};
};

struct ActionOverlayLayout {
    SDL_FRect card{};
    std::array<SDL_FRect, 3> buttons{};
    std::size_t buttonCount{};
};

[[nodiscard]] LevelGridLayout levelGridLayout(int outputWidth, int outputHeight);
[[nodiscard]] SettingsOverlayLayout settingsOverlayLayout(int outputWidth, int outputHeight);
[[nodiscard]] BestTimesOverlayLayout bestTimesOverlayLayout(int outputWidth, int outputHeight);
[[nodiscard]] ActionOverlayLayout pauseOverlayLayout(int outputWidth, int outputHeight);
[[nodiscard]] ActionOverlayLayout resultOverlayLayout(int outputWidth, int outputHeight, bool won);
[[nodiscard]] float uiScaleForOutput(int outputWidth, int outputHeight,
                                     std::uint16_t userScalePercent);
[[nodiscard]] bool hasAtlasMapping(std::uint8_t tile);
[[nodiscard]] std::optional<int> boardPositionAtOutput(const App& app, float x, float y,
                                                       int outputWidth, int outputHeight);
void draw(App& app);

} // namespace chips_native
