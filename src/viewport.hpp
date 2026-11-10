#pragma once

#include <algorithm>

namespace chips_native {

struct TileViewport {
    int left{};
    int top{};
    int width{};
    int height{};
};

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
