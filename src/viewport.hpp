#pragma once

#include <algorithm>

namespace chips_native {

struct TileViewport {
    int left{};
    int top{};
    int width{};
    int height{};
};

struct ViewportMotion {
    int fromX{};
    int fromY{};
    int toX{};
    int toY{};
};

[[nodiscard]] constexpr ViewportMotion viewportMotion(int fromMapX, int fromMapY,
                                                       int toMapX, int toMapY,
                                                       int cameraX, int cameraY) {
    return {fromMapX - cameraX, fromMapY - cameraY,
            toMapX - cameraX, toMapY - cameraY};
}

// Keep a fixed-size window around the player until it reaches an edge, then
// clamp the camera so the viewport always covers valid map coordinates.
[[nodiscard]] constexpr TileViewport playerViewport(int playerX, int playerY,
                                                     int mapWidth, int mapHeight,
                                                     int viewWidth = 9, int viewHeight = 9) {
    if (mapWidth <= 0 || mapHeight <= 0 || viewWidth <= 0 || viewHeight <= 0 ||
        mapWidth < viewWidth || mapHeight < viewHeight)
        return {};
    const int halfWidth = viewWidth / 2;
    const int halfHeight = viewHeight / 2;
    return {
        std::clamp(playerX - halfWidth, 0, mapWidth - viewWidth),
        std::clamp(playerY - halfHeight, 0, mapHeight - viewHeight),
        viewWidth,
        viewHeight};
}

} // namespace chips_native
