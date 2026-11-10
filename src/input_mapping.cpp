#include "input_mapping.hpp"

#include <algorithm>

namespace chips {

int movementIndexForKey(std::uint16_t keyCode,
                        const std::array<std::uint16_t, 4>& bindings,
                        const std::array<std::uint16_t, 4>& fallbackKeys) {
    const auto configured = std::find(bindings.begin(), bindings.end(), keyCode);
    if (configured != bindings.end()) return static_cast<int>(configured - bindings.begin());
    const auto fallback = std::find(fallbackKeys.begin(), fallbackKeys.end(), keyCode);
    return fallback == fallbackKeys.end() ? -1 : static_cast<int>(fallback - fallbackKeys.begin());
}

bool isReservedKey(std::uint16_t keyCode, std::span<const std::uint16_t> reservedKeys) {
    return std::find(reservedKeys.begin(), reservedKeys.end(), keyCode) != reservedKeys.end();
}

BindingRejection validateMovementBinding(
    std::uint16_t keyCode, std::size_t directionIndex,
    const std::array<std::uint16_t, 4>& bindings,
    std::span<const std::uint16_t> reservedKeys,
    std::span<const std::uint16_t> fallbackKeys) {
    if (isReservedKey(keyCode, reservedKeys) || isReservedKey(keyCode, fallbackKeys))
        return BindingRejection::reserved;
    if (directionIndex >= bindings.size()) return BindingRejection::duplicate;
    for (std::size_t i = 0; i < bindings.size(); ++i)
        if (i != directionIndex && bindings[i] == keyCode)
            return BindingRejection::duplicate;
    return BindingRejection::none;
}

} // namespace chips
